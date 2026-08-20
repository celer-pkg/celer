// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include "qoauth1.h"
#include "qoauth1_p.h"
#include "qoauth1signature.h"
#include "qoauthhttpserverreplyhandler.h"

#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmessageauthenticationcode.h>

#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QOAuth1
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QOAuth1 class provides an implementation of the
    \l {https://tools.ietf.org/html/rfc5849}{OAuth 1 Protocol}.
    \since 5.8

    QOAuth1 provides a method for clients to access server resources
    on behalf of a resource owner (such as a different client or an
    end-user). It also provides a process for end-users to authorize
    third-party access to their server resources without sharing
    their credentials (typically, a username and password pair),
    using user-agent redirections.

    QOAuth1 uses tokens to represent the authorization granted to the
    client by the resource owner.  Typically, token credentials are
    issued by the server at the resource owner's request, after
    authenticating the resource owner's identity (usually using a
    username and password).

    When making the temporary credentials request, the client
    authenticates using only the client credentials. When making the
    token request, the client authenticates using the client
    credentials as well as the temporary credentials. Once the
    client receives and stores the token credentials, it can
    proceed to access protected resources on behalf of the resource
    owner by making authenticated requests using the client
    credentials together with the token credentials received.
*/

/*!
    \enum QOAuth1::SignatureMethod

    Indicates the signature method to be used to sign requests.

    \value Hmac_Sha1
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.2}
    {HMAC-SHA1} signature method.

    \value Rsa_Sha1
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.3}
    {RSA-SHA1} signature method (not supported).

    \value PlainText
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.4}
    {PLAINTEXT} signature method.
*/

using OAuth1 = QOAuth1Private::OAuth1KeyString;
const QString OAuth1::oauthCallback =           u"oauth_callback"_s;
const QString OAuth1::oauthCallbackConfirmed =  u"oauth_callback_confirmed"_s;
const QString OAuth1::oauthConsumerKey =        u"oauth_consumer_key"_s;
const QString OAuth1::oauthNonce =              u"oauth_nonce"_s;
const QString OAuth1::oauthSignature =          u"oauth_signature"_s;
const QString OAuth1::oauthSignatureMethod =    u"oauth_signature_method"_s;
const QString OAuth1::oauthTimestamp =          u"oauth_timestamp"_s;
const QString OAuth1::oauthToken =              u"oauth_token"_s;
const QString OAuth1::oauthTokenSecret =        u"oauth_token_secret"_s;
const QString OAuth1::oauthVerifier =           u"oauth_verifier"_s;
const QString OAuth1::oauthVersion =            u"oauth_version"_s;

QOAuth1Private::QOAuth1Private(const std::pair<QString, QString> &clientCredentials,
                               QNetworkAccessManager *networkAccessManager) :
    QAbstractOAuthPrivate("qt.networkauth.oauth1",
                          QUrl(),
                          clientCredentials.first,
                          networkAccessManager),
    clientIdentifierSharedKey(clientCredentials.second)
{
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    qRegisterMetaType<QOAuth1::SignatureMethod>("QOAuth1::SignatureMethod");
}

void QOAuth1Private::appendCommonHeaders(QVariantMap *headers)
{
    const auto currentDateTime = QDateTime::currentDateTimeUtc();

    headers->insert(OAuth1::oauthNonce, QOAuth1::nonce());
    headers->insert(OAuth1::oauthConsumerKey, clientIdentifier);
    headers->insert(OAuth1::oauthTimestamp, QString::number(currentDateTime.toSecsSinceEpoch()));
    headers->insert(OAuth1::oauthVersion, oauthVersion);
    headers->insert(OAuth1::oauthSignatureMethod, signatureMethodString().toUtf8());
}

void QOAuth1Private::appendSignature(QAbstractOAuth::Stage stage,
                                     QVariantMap *headers,
                                     const QUrl &url,
                                     QNetworkAccessManager::Operation operation,
                                     const QMultiMap<QString, QVariant> parameters)
{
    QByteArray signature;
    {
        QMultiMap<QString, QVariant> allParameters(*headers);
        allParameters.unite(parameters);
        if (modifyParametersFunction)
            modifyParametersFunction(stage, &allParameters);
        signature = generateSignature(allParameters, url, operation);
    }
    headers->insert(OAuth1::oauthSignature, signature);
}

QNetworkReply *QOAuth1Private::requestToken(QNetworkAccessManager::Operation operation,
                                            const QUrl &url,
                                            const std::pair<QString, QString> &token,
                                            const QVariantMap &parameters)
{
    if (Q_UNLIKELY(!networkAccessManager())) {
        qCWarning(loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    if (Q_UNLIKELY(url.isEmpty())) {
        qCWarning(loggingCategory, "Request Url not set");
        return nullptr;
    }
    if (Q_UNLIKELY(operation != QNetworkAccessManager::GetOperation &&
                   operation != QNetworkAccessManager::PostOperation)) {
        qCWarning(loggingCategory, "Operation not supported");
        return nullptr;
    }

    QNetworkRequest request(url);

    QAbstractOAuth::Stage stage = QAbstractOAuth::Stage::RequestingTemporaryCredentials;
    QVariantMap headers;
    QMultiMap<QString, QVariant> remainingParameters;
    appendCommonHeaders(&headers);
    for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it) {
        const auto &key = it.key();
        const auto &value = it.value();
        if (key.startsWith(QStringLiteral("oauth_")))
            headers.insert(key, value);
        else
            remainingParameters.insert(key, value);
    }
    if (!token.first.isEmpty()) {
        headers.insert(OAuth1::oauthToken, token.first);
        stage = QAbstractOAuth::Stage::RequestingAccessToken;
    }
    appendSignature(stage, &headers, url, operation, remainingParameters);

    request.setRawHeader("Authorization", QOAuth1::generateAuthorizationHeader(headers));

    QNetworkReply *reply = nullptr;
    if (operation == QNetworkAccessManager::GetOperation) {
        if (parameters.size() > 0) {
            QUrl url = request.url();
            url.setQuery(QOAuth1Private::createQuery(remainingParameters));
            request.setUrl(url);
        }
        reply = networkAccessManager()->get(request);
    }
    else if (operation == QNetworkAccessManager::PostOperation) {
        QUrlQuery query = QOAuth1Private::createQuery(remainingParameters);
        const QByteArray data = query.toString(QUrl::FullyEncoded).toLatin1();
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/x-www-form-urlencoded"));
        reply = networkAccessManager()->post(request, data);
    }

    connect(reply, &QNetworkReply::errorOccurred,
            this, &QOAuth1Private::_q_onTokenRequestError);

    QAbstractOAuthReplyHandler *handler = replyHandler ? replyHandler.data()
                                                       : defaultReplyHandler.data();
    QObject::connect(reply, &QNetworkReply::finished, handler,
                     [handler, reply]() { handler->networkReplyFinished(reply); });
    connect(handler, &QAbstractOAuthReplyHandler::tokensReceived, this,
            &QOAuth1Private::_q_tokensReceived);

    return reply;
}

QString QOAuth1Private::signatureMethodString() const
{
    switch (signatureMethod) { // No default: intended
    case QOAuth1::SignatureMethod::PlainText:
        return QStringLiteral("PLAINTEXT");
    case QOAuth1::SignatureMethod::Hmac_Sha1:
        return QStringLiteral("HMAC-SHA1");
    case QOAuth1::SignatureMethod::Rsa_Sha1:
        qFatal("RSA-SHA1 signature method not supported");
        return QStringLiteral("RSA-SHA1");
    }
    qFatal("Invalid signature method");
    return QString();
}

QByteArray QOAuth1Private::generateSignature(const QMultiMap<QString, QVariant> &parameters,
                                             const QUrl &url,
                                             QNetworkAccessManager::Operation operation) const
{
    QOAuth1Signature signature(url,
                               clientIdentifierSharedKey,
                               tokenSecret,
                               static_cast<QOAuth1Signature::HttpRequestMethod>(operation),
                               parameters);
    return formatSignature(signature);
}

QByteArray QOAuth1Private::generateSignature(const QMultiMap<QString, QVariant> &parameters,
                                             const QUrl &url,
                                             const QByteArray &verb) const
{
    QOAuth1Signature signature(url,
                               clientIdentifierSharedKey,
                               tokenSecret,
                               QOAuth1Signature::HttpRequestMethod::Custom,
                               parameters);
    signature.setCustomMethodString(verb);
    return formatSignature(signature);
}

QByteArray QOAuth1Private::formatSignature(const QOAuth1Signature &signature) const
{
    switch (signatureMethod) {
    case QOAuth1::SignatureMethod::Hmac_Sha1:
        return signature.hmacSha1().toBase64();
    case QOAuth1::SignatureMethod::PlainText:
        return signature.plainText();
    default:
        qFatal("QOAuth1Private::generateSignature: Signature method not supported");
        return QByteArray();
    }
}

QVariantMap QOAuth1Private::createOAuthBaseParams() const
{
    QVariantMap oauthParams;

    const auto currentDateTime = QDateTime::currentDateTimeUtc();

    oauthParams.insert(OAuth1::oauthConsumerKey, clientIdentifier);
    oauthParams.insert(OAuth1::oauthVersion, QStringLiteral("1.0"));
    oauthParams.insert(OAuth1::oauthToken, token);
    oauthParams.insert(OAuth1::oauthSignatureMethod, signatureMethodString());
    oauthParams.insert(OAuth1::oauthNonce, QOAuth1::nonce());
    oauthParams.insert(OAuth1::oauthTimestamp, QString::number(currentDateTime.toSecsSinceEpoch()));

    return oauthParams;
}

/*!
    \reimp
*/
void QOAuth1::prepareRequest(QNetworkRequest *request, const QByteArray &verb,
                                 const QByteArray &body)
{
    QVariantMap signingParams;
    if (verb == "POST" &&
        request->header(QNetworkRequest::ContentTypeHeader).toByteArray()
            == "application/x-www-form-urlencoded") {
        QUrlQuery query(QString::fromUtf8(body));
        const auto items = query.queryItems(QUrl::FullyDecoded);
        for (const auto &item : items)
            signingParams.insert(item.first, item.second);
    }
    setup(request, signingParams, verb);
}

void QOAuth1Private::_q_onTokenRequestError(QNetworkReply::NetworkError error)
{
    Q_Q(QOAuth1);
    Q_UNUSED(error);
    Q_EMIT q->requestFailed(QAbstractOAuth::Error::NetworkError);
}

void QOAuth1Private::_q_tokensReceived(const QVariantMap &tokens)
{
    Q_Q(QOAuth1);

    if (!tokenRequested && status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        // We didn't request a token yet, but in the "TemporaryCredentialsReceived" state _any_
        // new tokens received will count as a successful authentication and we move to the
        // 'Granted' state. To avoid this, 'status' will be temporarily set to 'NotAuthenticated'.
        status = QAbstractOAuth::Status::NotAuthenticated;
    }
    if (tokenRequested) // 'Reset' tokenRequested now that we've gotten new tokens
        tokenRequested = false;

    const auto credential = std::make_pair(tokens.value(OAuth1::oauthToken).toString(),
                                           tokens.value(OAuth1::oauthTokenSecret).toString());
    switch (status) {
    case QAbstractOAuth::Status::NotAuthenticated:
        if (tokens.value(OAuth1::oauthCallbackConfirmed, true).toBool()) {
            q->setTokenCredentials(credential);
            setStatus(QAbstractOAuth::Status::TemporaryCredentialsReceived);
        } else {
            Q_EMIT q->requestFailed(QAbstractOAuth::Error::OAuthCallbackNotVerified);
        }
        break;
    case QAbstractOAuth::Status::TemporaryCredentialsReceived:
        q->setTokenCredentials(credential);
        setStatus(QAbstractOAuth::Status::Granted);
        break;
    case QAbstractOAuth::Status::Granted:
    case QAbstractOAuth::Status::RefreshingToken:
        break;
    }
}

/*!
    Constructs a QOAuth1 object with parent object \a parent.
*/
QOAuth1::QOAuth1(QObject *parent) :
    QOAuth1(nullptr,
            parent)
{}

/*!
    Constructs a QOAuth1 object with parent object \a parent, using
    \a manager to access the network.
*/
QOAuth1::QOAuth1(QNetworkAccessManager *manager, QObject *parent) :
    QOAuth1(QString(),
            QString(),
            manager,
            parent)
{}

/*!
    Constructs a QOAuth1 object with parent object \a parent, using
    \a manager to access the network.
    Also sets \a clientIdentifier and \a clientSharedSecret to sign
    the calls to the web server and identify the application.
*/
QOAuth1::QOAuth1(const QString &clientIdentifier,
                 const QString &clientSharedSecret,
                 QNetworkAccessManager *manager,
                 QObject *parent)
    : QAbstractOAuth(*new QOAuth1Private(std::make_pair(clientIdentifier, clientSharedSecret),
                                         manager),
                     parent)
{}

/*!
    Returns the current shared secret used to sign requests to
    the web server.

    \sa setClientSharedSecret(), clientCredentials()
*/
QString QOAuth1::clientSharedSecret() const
{
    Q_D(const QOAuth1);
    return d->clientIdentifierSharedKey;
}

/*!
    Sets \a clientSharedSecret as the string used to sign the
    requests to the web server.

    \sa clientSharedSecret(), setClientCredentials()
*/
void QOAuth1::setClientSharedSecret(const QString &clientSharedSecret)
{
    Q_D(QOAuth1);
    if (d->clientIdentifierSharedKey != clientSharedSecret) {
        d->clientIdentifierSharedKey = clientSharedSecret;
        Q_EMIT clientSharedSecretChanged(clientSharedSecret);
    }
}

/*!
    Returns the pair of QString used to identify the application and
    sign requests to the web server.

    \sa setClientCredentials()
*/
std::pair<QString, QString> QOAuth1::clientCredentials() const
{
    Q_D(const QOAuth1);
    return std::make_pair(d->clientIdentifier, d->clientIdentifierSharedKey);
}

/*!
    \fn void QOAuth1::setClientCredentials(const std::pair<QString, QString> &clientCredentials)

    Sets \a clientCredentials as the pair of QString used to identify
    the application and sign requests to the web server.

    \sa clientCredentials()
*/
void QOAuth1::setClientCredentials(const std::pair<QString, QString> &clientCredentials)
{
    setClientCredentials(clientCredentials.first, clientCredentials.second);
}

/*!
    Sets \a clientIdentifier and \a clientSharedSecret as the pair of
    QString used to identify the application and sign requests to the
    web server. \a clientIdentifier identifies the application and
    \a clientSharedSecret is used to sign requests.

    \sa clientCredentials()
*/
void QOAuth1::setClientCredentials(const QString &clientIdentifier,
                                   const QString &clientSharedSecret)
{
    setClientIdentifier(clientIdentifier);
    setClientSharedSecret(clientSharedSecret);
}

/*!
    Returns the current token secret used to sign authenticated
    requests to the web server.

    \sa setTokenSecret(), tokenCredentials()
*/
QString QOAuth1::tokenSecret() const
{
    Q_D(const QOAuth1);
    return d->tokenSecret;
}
/*!
    Sets \a tokenSecret as the current token secret used to sign
    authenticated calls to the web server.

    \sa tokenSecret(), setTokenCredentials()
*/
void QOAuth1::setTokenSecret(const QString &tokenSecret)
{
    Q_D(QOAuth1);
    if (d->tokenSecret != tokenSecret) {
        d->tokenSecret = tokenSecret;
        Q_EMIT tokenSecretChanged(tokenSecret);
    }
}

/*!
    \fn std::pair<QString, QString> QOAuth1::tokenCredentials() const

    Returns the pair of QString used to identify and sign
    authenticated requests to the web server.

    \sa setTokenCredentials()
*/
std::pair<QString, QString> QOAuth1::tokenCredentials() const
{
    Q_D(const QOAuth1);
    return std::make_pair(d->token, d->tokenSecret);
}

/*!
    \fn void QOAuth1::setTokenCredentials(const std::pair<QString, QString> &tokenCredentials)

    Sets \a tokenCredentials as the pair of QString used to identify
    and sign authenticated requests to the web server.

    \sa tokenCredentials()
*/
void QOAuth1::setTokenCredentials(const std::pair<QString, QString> &tokenCredentials)
{
    setTokenCredentials(tokenCredentials.first, tokenCredentials.second);
}

/*!
    Sets \a token and \a tokenSecret as the pair of QString used to
    identify and sign authenticated requests to the web server.
    Once the client receives and stores the token credentials, it can
    proceed to access protected resources on behalf of the resource
    owner by making authenticated requests using the client
    credentials together with the token credentials received.

    \sa tokenCredentials()
*/
void QOAuth1::setTokenCredentials(const QString &token, const QString &tokenSecret)
{
    setToken(token);
    setTokenSecret(tokenSecret);
}

/*!
    Returns the url used to request temporary credentials to
    start the authentication process.

    \sa setTemporaryCredentialsUrl()
*/
QUrl QOAuth1::temporaryCredentialsUrl() const
{
    Q_D(const QOAuth1);
    return d->temporaryCredentialsUrl;
}

/*!
    Sets \a url as the URL to request temporary credentials to
    start the authentication process.

    \sa temporaryCredentialsUrl()
*/
void QOAuth1::setTemporaryCredentialsUrl(const QUrl &url)
{
    Q_D(QOAuth1);
    if (d->temporaryCredentialsUrl != url) {
        d->temporaryCredentialsUrl = url;
        Q_EMIT temporaryCredentialsUrlChanged(url);
    }
}

/*!
    Returns the url used to request token credentials to continue
    the authentication process.

    \sa setTokenCredentialsUrl()
*/
QUrl QOAuth1::tokenCredentialsUrl() const
{
    Q_D(const QOAuth1);
    return d->tokenCredentialsUrl;
}

/*!
    Sets \a url as the URL to request the token credentials to
    continue the authentication process.

    \sa tokenCredentialsUrl()
*/
void QOAuth1::setTokenCredentialsUrl(const QUrl &url)
{
    Q_D(QOAuth1);
    if (d->tokenCredentialsUrl != url) {
        d->tokenCredentialsUrl = url;
        Q_EMIT tokenCredentialsUrlChanged(url);
    }
}

/*!
    Returns the method used to sign the request to the web server.

    \sa setSignatureMethod()
*/
QOAuth1::SignatureMethod QOAuth1::signatureMethod() const
{
    Q_D(const QOAuth1);
    return d->signatureMethod;
}

/*!
    Sets \a value as the method used to sign requests to the web
    server.

    \sa signatureMethod()
*/
void QOAuth1::setSignatureMethod(QOAuth1::SignatureMethod value)
{
    Q_D(QOAuth1);
    if (d->signatureMethod != value) {
        d->signatureMethod = value;
        Q_EMIT signatureMethodChanged(value);
    }
}

/*!
    Sends an authenticated HEAD request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.4}
    {Hypertext Transfer Protocol -- HTTP/1.1: HEAD}
*/
QNetworkReply *QOAuth1::head(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::HeadOperation);
    return d->networkAccessManager()->head(request);
}

/*!
    Sends an authenticated GET request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.3}
    {Hypertext Transfer Protocol -- HTTP/1.1: GET}
*/
QNetworkReply *QOAuth1::get(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::GetOperation);
    QNetworkReply *reply = d->networkAccessManager()->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.5}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QOAuth1::post(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::PostOperation);
    d->addContentTypeHeaders(&request);

    const QByteArray data = d->convertParameters(parameters);
    QNetworkReply *reply = d->networkAccessManager()->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QOAuth1::put(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::PutOperation);
    d->addContentTypeHeaders(&request);

    const QByteArray data = d->convertParameters(parameters);
    QNetworkReply *reply = d->networkAccessManager()->put(request, data);
    connect(reply, &QNetworkReply::finished, this, std::bind(&QAbstractOAuth::finished, this, reply));
    return reply;
}

/*!
    Sends an authenticated DELETE request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.7}
    {Hypertext Transfer Protocol -- HTTP/1.1: DELETE}
*/
QNetworkReply *QOAuth1::deleteResource(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::DeleteOperation);
    QNetworkReply *reply = d->networkAccessManager()->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Starts the a request for temporary credentials using the request
    method \a operation. The request URL is \a url and the
    \a parameters shall encoded and sent during the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2.1}
    {The OAuth 1.0 Protocol: Temporary Credentials}
*/
QNetworkReply *QOAuth1::requestTemporaryCredentials(QNetworkAccessManager::Operation operation,
                                                    const QUrl &url,
                                                    const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    d->token.clear();
    d->tokenSecret.clear();
    QVariantMap allParameters(parameters);
    allParameters.insert(OAuth1::oauthCallback, callback());
    return d->requestToken(operation, url, std::make_pair(d->token, d->tokenSecret), allParameters);
}

/*!
    \fn QNetworkReply *QOAuth1::requestTokenCredentials(QNetworkAccessManager::Operation operation,
                                                const QUrl &url,
                                                const std::pair<QString, QString> &temporaryToken,
                                                const QVariantMap &parameters)

    Starts a request for token credentials using the request
    method \a operation. The request URL is \a url and the
    \a parameters shall be encoded and sent during the
    request. The \a temporaryToken pair of string is used to identify
    and sign the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2.3}
    {The OAuth 1.0 Protocol: Token Credentials}
*/
QNetworkReply *QOAuth1::requestTokenCredentials(QNetworkAccessManager::Operation operation,
                                                const QUrl &url,
                                                const std::pair<QString, QString> &temporaryToken,
                                                const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    d->tokenRequested = true;
    return d->requestToken(operation, url, temporaryToken, parameters);
}

/*!
    Signs the \a request using \a signingParameters and \a operation.

    \overload
*/
void QOAuth1::setup(QNetworkRequest *request,
                    const QVariantMap &signingParameters,
                    QNetworkAccessManager::Operation operation)
{
    Q_D(const QOAuth1);

    auto oauthParams = d->createOAuthBaseParams();

    // Add signature parameter
    {
        QMultiMap<QString, QVariant> parameters(oauthParams);
        parameters.unite(QMultiMap<QString, QVariant>(signingParameters));
        const auto signature = d->generateSignature(parameters, request->url(), operation);
        oauthParams.insert(OAuth1::oauthSignature, signature);
    }

    if (operation == QNetworkAccessManager::GetOperation) {
        if (signingParameters.size()) {
            QUrl url = request->url();
            QUrlQuery query = QUrlQuery(url.query());
            for (auto it = signingParameters.begin(), end = signingParameters.end(); it != end;
                 ++it)
                query.addQueryItem(it.key(), it.value().toString());
            url.setQuery(query);
            request->setUrl(url);
        }
    }

    request->setRawHeader("Authorization", generateAuthorizationHeader(oauthParams));

    if (operation == QNetworkAccessManager::PostOperation
            || operation == QNetworkAccessManager::PutOperation)
        request->setHeader(QNetworkRequest::ContentTypeHeader,
                           QStringLiteral("application/x-www-form-urlencoded"));
}

/*!
    \since 5.13

    Signs the \a request using \a signingParameters and \a operationVerb.

    \overload
*/
void QOAuth1::setup(QNetworkRequest *request, const QVariantMap &signingParameters, const QByteArray &operationVerb)
{
    Q_D(const QOAuth1);

    auto oauthParams = d->createOAuthBaseParams();

    // Add signature parameter
    {
        QMultiMap<QString, QVariant> parameters(oauthParams);
        parameters.unite(QMultiMap<QString, QVariant>(signingParameters));
        const auto signature = d->generateSignature(parameters, request->url(), operationVerb);
        oauthParams.insert(OAuth1::oauthSignature, signature);
    }

    request->setRawHeader("Authorization", generateAuthorizationHeader(oauthParams));
}

/*!
    Generates a nonce.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-3.3}
    {The OAuth 1.0 Protocol: Nonce and Timestamp}
*/
QByteArray QOAuth1::nonce()
{
    return QAbstractOAuth::generateRandomString(8);
}

/*!
    Generates an authorization header using \a oauthParams.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-3.5.1}
    {The OAuth 1.0 Protocol: Authorization Header}
*/
QByteArray QOAuth1::generateAuthorizationHeader(const QVariantMap &oauthParams)
{
    // TODO Add realm parameter support
    bool first = true;
    QString ret(QStringLiteral("OAuth "));
    QVariantMap headers(oauthParams);
    for (auto it = headers.begin(), end = headers.end(); it != end; ++it) {
        if (first)
            first = false;
        else
            ret += QLatin1String(",");
        ret += it.key() +
               QLatin1String("=\"") +
               QString::fromUtf8(QUrl::toPercentEncoding(it.value().toString())) +
               QLatin1Char('\"');
    }
    return ret.toUtf8();
}

/*!
    Starts the Redirection-Based Authorization flow.

    \note For an out-of-band reply handler, a verifier string is
    received after the call to this function; pass that to
    continueGrantWithVerifier() to continue the grant process.

    \sa continueGrantWithVerifier()

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2}
    {The OAuth 1.0 Protocol: Redirection-Based Authorization}
*/
void QOAuth1::grant()
{
    Q_D(QOAuth1);
    if (d->temporaryCredentialsUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "requestTokenUrl is empty");
        return;
    }
    if (d->tokenCredentialsUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "authorizationGrantUrl is empty");
        return;
    }
    if (!d->token.isEmpty() && status() == Status::Granted) {
        qCWarning(d->loggingCategory, "Already authenticated");
        return;
    }

    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = connect(this, &QAbstractOAuth::statusChanged, this,
                          [this, connection](Status status) {
        Q_D(QOAuth1);

        if (status == Status::TemporaryCredentialsReceived) {
            if (d->authorizationUrl.isEmpty()) {
                // try upgrading token without verifier
                auto reply = requestTokenCredentials(QNetworkAccessManager::PostOperation,
                                                     d->tokenCredentialsUrl,
                                                     std::make_pair(d->token, d->tokenSecret));
                connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
            } else {
                QMultiMap<QString, QVariant> parameters;
                parameters.insert(OAuth1::oauthToken, d->token);
                if (d->modifyParametersFunction)
                    d->modifyParametersFunction(Stage::RequestingAuthorization, &parameters);

                // https://tools.ietf.org/html/rfc5849#section-2.2
                resourceOwnerAuthorization(d->authorizationUrl, parameters);
            }
        } else if (status == Status::NotAuthenticated) {
            // Inherit class called QAbstractOAuth::setStatus(Status::NotAuthenticated);
            setTokenCredentials(QString(), QString());
            disconnect(*connection);
        }
    });

    auto httpReplyHandler = qobject_cast<QOAuthHttpServerReplyHandler*>(replyHandler());
    if (httpReplyHandler) {
        auto func = [this](const QVariantMap &values) {
            Q_D(QOAuth1);
            QString verifier = values.value(OAuth1::oauthVerifier).toString();
            if (verifier.isEmpty()) {
                qCWarning(d->loggingCategory, "%s not found in the callback",
                          qPrintable(OAuth1::oauthVerifier));
                return;
            }
            continueGrantWithVerifier(verifier);
        };
        connect(httpReplyHandler, &QOAuthHttpServerReplyHandler::callbackReceived, this, func);
    }

    // requesting temporary credentials
    auto reply = requestTemporaryCredentials(QNetworkAccessManager::PostOperation,
                                             d->temporaryCredentialsUrl);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

/*!
    Continues the Redirection-Based Authorization flow using
    \a verifier. Call this function when using an Out-of-band reply
    handler to supply the verifier provided by the web server.
*/
void QOAuth1::continueGrantWithVerifier(const QString &verifier)
{
    // https://tools.ietf.org/html/rfc5849#section-2.3
    Q_D(QOAuth1);

    QVariantMap parameters;
    parameters.insert(OAuth1::oauthVerifier, verifier);
    auto reply = requestTokenCredentials(QNetworkAccessManager::PostOperation,
                                         d->tokenCredentialsUrl,
                                         std::make_pair(d->token, d->tokenSecret),
                                         parameters);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

QT_END_NAMESPACE
