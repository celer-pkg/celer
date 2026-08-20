// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include <qoauth2authorizationcodeflow.h>
#include <private/qoauth2authorizationcodeflow_p.h>

#include <qmap.h>
#include <qurl.h>
#include <qvariant.h>
#include <qurlquery.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qauthenticator.h>
#include <qoauthhttpserverreplyhandler.h>

#include <QtCore/qcryptographichash.h>

#include <functional>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QOAuth2AuthorizationCodeFlow
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QOAuth2AuthorizationCodeFlow class provides an
    implementation of the
    \l {https://tools.ietf.org/html/rfc6749#section-4.1}
    {Authorization Code Grant} flow.
    \since 5.8

    This class implements the
    \l {https://tools.ietf.org/html/rfc6749#section-4.1}
    {Authorization Code Grant} flow, which is used both to obtain and
    to refresh access tokens. It is a redirection-based flow so the
    user will need access to a web browser.

    As a redirection-based flow this class requires a proper
    reply handler to be set. See \l {OAuth 2.0 Overview},
    QOAuthHttpServerReplyHandler, and QOAuthUriSchemeReplyHandler.
*/

#if QT_DEPRECATED_SINCE(6, 13)
/*!
    \property QOAuth2AuthorizationCodeFlow::accessTokenUrl
    \deprecated [6.9] Use QAbstractOAuth2::tokenUrl instead.

    \brief This property holds the URL used to convert the temporary
    code received during the authorization response.

    \b {See also}:
    \l {https://tools.ietf.org/html/rfc6749#section-4.1.3}{Access
    Token Request}
*/
#endif

QOAuth2AuthorizationCodeFlowPrivate::QOAuth2AuthorizationCodeFlowPrivate(
        const QUrl &authorizationUrl, const QUrl &accessTokenUrl, const QString &clientIdentifier,
        QNetworkAccessManager *manager) :
    QAbstractOAuth2Private(std::make_pair(clientIdentifier, QString()), authorizationUrl, manager)
{
    tokenUrl = accessTokenUrl;
    responseType = QStringLiteral("code");
}

static QString toUrlFormEncoding(const QString &source)
{
    // RFC 6749 Appendix B
    // https://datatracker.ietf.org/doc/html/rfc6749#appendix-B
    // Replace spaces with plus, while percent-encoding the rest
    QByteArray encoded = source.toUtf8().toPercentEncoding(" ");
    encoded.replace(" ", "+");
    return QString::fromUtf8(encoded);
}

static QString fromUrlFormEncoding(const QString &source)
{
    QByteArray decoded = source.toUtf8();
    decoded = QByteArray::fromPercentEncoding(decoded.replace("+"," "));
    return QString::fromUtf8(decoded);
}

void QOAuth2AuthorizationCodeFlowPrivate::_q_handleCallback(const QVariantMap &data)
{
    Q_Q(QOAuth2AuthorizationCodeFlow);

    if (status != QAbstractOAuth::Status::NotAuthenticated) {
        qCWarning(loggingCategory) << "Authorization stage: callback in unexpected status:"
                                   << static_cast<int>(status) << ", ignoring the callback";
        return;
    }

    Q_ASSERT(!state.isEmpty());

    if (handleRfcErrorResponseIfPresent(data))
        return;

    const QString code = data.value(QtOAuth2RfcKeywords::code).toString();
    if (code.isEmpty()) {
        qCWarning(loggingCategory, "Authorization stage: Code not received");
        emit q->requestFailed(QAbstractOAuth::Error::OAuthTokenNotFoundError);
        return;
    }
    const QString receivedState =
        fromUrlFormEncoding(data.value(QtOAuth2RfcKeywords::state).toString());
    if (receivedState.isEmpty()) {
        qCWarning(loggingCategory, "Authorization stage: State not received");
        emit q->requestFailed(QAbstractOAuth::Error::ServerError);
        return;
    }
    if (state != receivedState) {
        qCWarning(loggingCategory) << "Authorization stage: State mismatch";
        emit q->requestFailed(QAbstractOAuth::Error::ServerError);
        return;
    }

    setStatus(QAbstractOAuth::Status::TemporaryCredentialsReceived);

    QVariantMap copy(data);
    copy.remove(QtOAuth2RfcKeywords::code);
    copy.remove(QtOAuth2RfcKeywords::state);
    setExtraTokens(copy);
    q->requestAccessToken(code);
}

void QOAuth2AuthorizationCodeFlowPrivate::_q_authenticate(QNetworkReply *reply,
                                                          QAuthenticator *authenticator)
{
    if (reply == currentReply){
        const auto url = reply->url();
        if (url == tokenUrl) {
            authenticator->setUser(clientIdentifier);
            authenticator->setPassword(QString());
        }
    }
}

/*
    Creates and returns a new PKCE 'code_challenge', and stores the
    underlying 'code_verifier' that was used to compute it.

    The PKCE flow involves two parts:
    1. Authorization request: include the 'code_challenge' which
       is computed from the 'code_verifier'.
    2. Access token request: include the original 'code_verifier'.

    With these two parts the authorization server is able to verify
    that the token request came from same entity as the original
    authorization request, mitigating the risk of authorization code
    interception attacks.
*/
QByteArray QOAuth2AuthorizationCodeFlowPrivate::createPKCEChallenge()
{
    switch (pkceMethod) {
    case QOAuth2AuthorizationCodeFlow::PkceMethod::None:
        pkceCodeVerifier.clear();
        return {};
    case QOAuth2AuthorizationCodeFlow::PkceMethod::Plain:
        // RFC 7636 4.2, plain
        // code_challenge = code_verifier
        pkceCodeVerifier = generateRandomBase64String(pkceVerifierLength);
        return pkceCodeVerifier;
    case QOAuth2AuthorizationCodeFlow::PkceMethod::S256:
        // RFC 7636 4.2, S256
        // code_challenge = BASE64URL-ENCODE(SHA256(ASCII(code_verifier)))
        pkceCodeVerifier = generateRandomBase64String(pkceVerifierLength);
        // RFC 7636 3. Terminology:
        // "with all trailing '=' characters omitted"
        return QCryptographicHash::hash(pkceCodeVerifier, QCryptographicHash::Algorithm::Sha256)
                        .toBase64(QByteArray::Base64Option::Base64UrlEncoding
                                  | QByteArray::Base64Option::OmitTrailingEquals);
    };
    Q_UNREACHABLE_RETURN({});
}

/*!
    Constructs a QOAuth2AuthorizationCodeFlow object with parent
    object \a parent.
*/
QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(QObject *parent) :
    QOAuth2AuthorizationCodeFlow(nullptr,
                                 parent)
{}

/*!
    Constructs a QOAuth2AuthorizationCodeFlow object using \a parent
    as parent and sets \a manager as the network access manager.
*/
QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(QNetworkAccessManager *manager,
                                                           QObject *parent) :
    QOAuth2AuthorizationCodeFlow(QString(),
                                 manager,
                                 parent)
{}

/*!
    Constructs a QOAuth2AuthorizationCodeFlow object using \a parent
    as parent and sets \a manager as the network access manager. The
    client identifier is set to \a clientIdentifier.
*/
QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(const QString &clientIdentifier,
                                                           QNetworkAccessManager *manager,
                                                           QObject *parent) :
    QAbstractOAuth2(*new QOAuth2AuthorizationCodeFlowPrivate(QUrl(), QUrl(), clientIdentifier,
                                                             manager),
                    parent)
{
}

/*!
    Constructs a QOAuth2AuthorizationCodeFlow object using \a parent
    as parent and sets \a manager as the network access manager. The
    authenticate URL is set to \a authenticateUrl and the access
    token URL is set to \a accessTokenUrl.
*/
QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(const QUrl &authenticateUrl,
                                                           const QUrl &accessTokenUrl,
                                                           QNetworkAccessManager *manager,
                                                           QObject *parent) :
    QAbstractOAuth2(*new QOAuth2AuthorizationCodeFlowPrivate(authenticateUrl, accessTokenUrl,
                                                             QString(), manager),
                    parent)
{
}

/*!
    Constructs a QOAuth2AuthorizationCodeFlow object using \a parent
    as parent and sets \a manager as the network access manager. The
    client identifier is set to \a clientIdentifier the authenticate
    URL is set to \a authenticateUrl and the access token URL is set
    to \a accessTokenUrl.
*/
QOAuth2AuthorizationCodeFlow::QOAuth2AuthorizationCodeFlow(const QString &clientIdentifier,
                                                           const QUrl &authenticateUrl,
                                                           const QUrl &accessTokenUrl,
                                                           QNetworkAccessManager *manager,
                                                           QObject *parent) :
    QAbstractOAuth2(*new QOAuth2AuthorizationCodeFlowPrivate(authenticateUrl, accessTokenUrl,
                                                             clientIdentifier, manager),
                    parent)
{
}

/*!
    Destroys the QOAuth2AuthorizationCodeFlow instance.
*/
QOAuth2AuthorizationCodeFlow::~QOAuth2AuthorizationCodeFlow()
{}

#if QT_DEPRECATED_SINCE(6, 13)
/*!
    \deprecated [6.13] Use QAbstractOAuth2::tokenUrl() instead.

    Returns the URL used to request the access token.
    \sa setAccessTokenUrl()
*/
QUrl QOAuth2AuthorizationCodeFlow::accessTokenUrl() const
{
    Q_D(const QOAuth2AuthorizationCodeFlow);
    return d->tokenUrl;
}

/*!
    \deprecated [6.13] Use QAbstractOAuth2::setTokenUrl() instead.

    Sets the URL used to request the access token to
    \a accessTokenUrl.
*/
void QOAuth2AuthorizationCodeFlow::setAccessTokenUrl(const QUrl &accessTokenUrl)
{
    Q_D(QOAuth2AuthorizationCodeFlow);
    if (accessTokenUrl == d->tokenUrl)
        return;

    setTokenUrl(accessTokenUrl);
    QT_IGNORE_DEPRECATIONS(Q_EMIT accessTokenUrlChanged(accessTokenUrl);)
}
#endif // QT_DEPRECATED_SINCE(6, 13)

/*!
    \enum QOAuth2AuthorizationCodeFlow::PkceMethod
    \since 6.8

    List of available \l {https://datatracker.ietf.org/doc/html/rfc7636}
    {Proof Key for Code Exchange (PKCE) methods}.

    PKCE is a security measure to mitigate the risk of \l
    {https://datatracker.ietf.org/doc/html/rfc7636#section-1}{authorization
    code interception attacks}. As such it is relevant for OAuth2
    "Authorization Code" flow (grant) and in particular with
    native applications.

    PKCE inserts additional parameters into authorization
    and access token requests. With the help of these parameters the
    authorization server is able to verify that an access token request
    originates from the same entity that issued the authorization
    request.

    \value None PKCE is not used.
    \value Plain The Plain PKCE method is used. Use this only if it is not
           possible to use S256. With Plain method the
           \l {https://datatracker.ietf.org/doc/html/rfc7636#section-4.2}{code challenge}
           equals to the
           \l {https://datatracker.ietf.org/doc/html/rfc7636#section-4.1}{code verifier}.
    \value S256 The S256 PKCE method is used. This is the default and the
           recommended method for native applications. With the S256 method
           the \e {code challenge} is a base64url-encoded value of the
           SHA-256 of the \e {code verifier}.

    \sa setPkceMethod(), pkceMethod()
*/

/*!
    \since 6.8

    Sets the current PKCE method to \a method.

    Optionally, the \a length parameter can be used to set the length
    of the \c code_verifier. The value must be between 43 and 128 bytes.
    The 'code verifier' itself is random-generated by the library.

    \sa pkceMethod(), QOAuth2AuthorizationCodeFlow::PkceMethod
*/
void QOAuth2AuthorizationCodeFlow::setPkceMethod(PkceMethod method, qsizetype length)
{
    Q_D(QOAuth2AuthorizationCodeFlow);
    if (length < 43 || length > 128) {
        // RFC 7636 Section 4.1, the code_verifer should be 43..128 bytes
        qWarning("Invalid PKCE length provided, must be between 43..128. Ignoring.");
        return;
    }
    static_assert(std::is_same_v<decltype(d->pkceVerifierLength), quint8>);
    d->pkceVerifierLength = quint8(length);
    d->pkceMethod = method;
}

/*!
    \since 6.8

    Returns the current PKCE method.

    \sa setPkceMethod(), QOAuth2AuthorizationCodeFlow::PkceMethod
*/
QOAuth2AuthorizationCodeFlow::PkceMethod QOAuth2AuthorizationCodeFlow::pkceMethod() const noexcept
{
    Q_D(const QOAuth2AuthorizationCodeFlow);
    return d->pkceMethod;
}

/*!
    Starts the authentication flow as described in
    \l {https://tools.ietf.org/html/rfc6749#section-4.1}{The OAuth
    2.0 Authorization Framework}
*/
void QOAuth2AuthorizationCodeFlow::grant()
{
    Q_D(QOAuth2AuthorizationCodeFlow);
    if (d->authorizationUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "No authenticate Url set");
        return;
    }
    if (d->tokenUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "No request access token Url set");
        return;
    }

    resourceOwnerAuthorization(d->authorizationUrl);
}

#if QT_DEPRECATED_SINCE(6, 13)
/*!
    \deprecated [6.13] Use QAbstractOAuth2::refreshTokens() instead.

    Call this function to refresh the token.

    This function calls \l {refreshTokensImplementation()}.
*/
void QOAuth2AuthorizationCodeFlow::refreshAccessToken()
{
    refreshTokensImplementation();
}
#endif // QT_DEPRECATED_SINCE(6, 13)

/*!
    \since 6.9

    This function sends a token refresh request.

    If the refresh request was initiated successfully, the status is set to
    \l QAbstractOAuth::Status::RefreshingToken; otherwise the \l requestFailed()
    signal is emitted and the status is not changed.

    This function has no effect if the token refresh process is already in
    progress.

    If refreshing the token fails and an access token exists, the status is
    set to \l QAbstractOAuth::Status::Granted, and to
    \l QAbstractOAuth::Status::NotAuthenticated if an access token
    does not exist.

    \sa QAbstractOAuth::requestFailed(), QAbstractOAuth2::refreshTokens()
*/
void QOAuth2AuthorizationCodeFlow::refreshTokensImplementation()
{
    Q_D(QOAuth2AuthorizationCodeFlow);

    if (d->refreshToken.isEmpty()) {
        qCWarning(d->loggingCategory, "Cannot refresh access token. Empty refresh token");
        return;
    }
    if (d->status == Status::RefreshingToken) {
        qCWarning(d->loggingCategory, "Cannot refresh access token. "
                                      "Refresh Access Token is already in progress");
        return;
    }

    const auto [request, body] = d->createRefreshRequestAndBody(d->tokenUrl);
    d->currentReply = d->networkAccessManager()->post(request, body);
    setStatus(Status::RefreshingToken);

    QNetworkReply *reply = d->currentReply.data();
    QAbstractOAuthReplyHandler *handler = replyHandler();
    connect(reply, &QNetworkReply::finished, handler,
            [handler, reply]() { handler->networkReplyFinished(reply); });
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    QObjectPrivate::connect(handler, &QAbstractOAuthReplyHandler::tokensReceived, d,
                            &QOAuth2AuthorizationCodeFlowPrivate::_q_tokenRequestFinished,
                            Qt::UniqueConnection);
    QObjectPrivate::connect(d->networkAccessManager(),
                            &QNetworkAccessManager::authenticationRequired,
                            d, &QOAuth2AuthorizationCodeFlowPrivate::_q_authenticate,
                            Qt::UniqueConnection);
    QObjectPrivate::connect(handler, &QAbstractOAuthReplyHandler::tokenRequestErrorOccurred,
                            d, &QOAuth2AuthorizationCodeFlowPrivate::_q_tokenRequestFailed,
                            Qt::UniqueConnection);
}

/*!
    Generates an authentication URL to be used in the
    \l {https://tools.ietf.org/html/rfc6749#section-4.1.1}
    {Authorization Request} using \a parameters.
*/
QUrl QOAuth2AuthorizationCodeFlow::buildAuthenticateUrl(const QMultiMap<QString, QVariant> &parameters)
{
    Q_D(QOAuth2AuthorizationCodeFlow);

    if (d->state.isEmpty())
        setState(QAbstractOAuth2Private::generateRandomState());
    Q_ASSERT(!d->state.isEmpty());
    const QString state = d->state;

    QMultiMap<QString, QVariant> p(parameters);
    QUrl url(d->authorizationUrl);
    p.insert(QtOAuth2RfcKeywords::responseType, responseType());
    p.insert(QtOAuth2RfcKeywords::clientIdentifier, d->clientIdentifier);
    p.insert(QtOAuth2RfcKeywords::redirectUri, callback());
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    if (d->legacyScopeWasSetByUser) {
        if (!d->legacyScope.isEmpty())
            p.insert(QtOAuth2RfcKeywords::scope, d->legacyScope);
    } else
#endif
    if (!d->requestedScopeTokens.isEmpty())
        p.insert(QtOAuth2RfcKeywords::scope, d->joinedScope(d->requestedScopeTokens));
    p.insert(QtOAuth2RfcKeywords::state, toUrlFormEncoding(state));
    if (d->pkceMethod != PkceMethod::None) {
        p.insert(QtOAuth2RfcKeywords::codeChallenge, d->createPKCEChallenge());
        p.insert(QtOAuth2RfcKeywords::codeChallengeMethod,
                 d->pkceMethod == PkceMethod::Plain ? u"plain"_s : u"S256"_s);
    }
    if (d->authorizationShouldIncludeNonce()) {
        if (d->nonce.isEmpty())
            setNonce(QAbstractOAuth2Private::generateNonce());
        p.insert(QtOAuth2RfcKeywords::nonce, d->nonce);
    }
    if (d->modifyParametersFunction)
        d->modifyParametersFunction(Stage::RequestingAuthorization, &p);
    url.setQuery(d->createQuery(p));
    connect(replyHandler(), &QAbstractOAuthReplyHandler::callbackReceived, this,
            &QOAuth2AuthorizationCodeFlow::authorizationCallbackReceived, Qt::UniqueConnection);
    setStatus(QAbstractOAuth::Status::NotAuthenticated);
    qCDebug(d->loggingCategory, "Authorization URL generated");
    return url;
}

/*!
    Requests an access token from the received \a code. The \a code
    is received as a response when the user completes a successful
    authentication in the browser.
*/
void QOAuth2AuthorizationCodeFlow::requestAccessToken(const QString &code)
{
    Q_D(QOAuth2AuthorizationCodeFlow);

    QMultiMap<QString, QVariant> parameters;
    QNetworkRequest request(d->tokenUrl);
#ifndef QT_NO_SSL
    if (d->sslConfiguration && !d->sslConfiguration->isNull())
        request.setSslConfiguration(*d->sslConfiguration);
#endif
    QUrlQuery query;
    parameters.insert(QtOAuth2RfcKeywords::grantType, QStringLiteral("authorization_code"));

    if (code.contains(QLatin1Char('%')))
        parameters.insert(QtOAuth2RfcKeywords::code, code);
    else
        parameters.insert(QtOAuth2RfcKeywords::code, QUrl::toPercentEncoding(code));

    parameters.insert(QtOAuth2RfcKeywords::redirectUri, QUrl::toPercentEncoding(callback()));
    parameters.insert(QtOAuth2RfcKeywords::clientIdentifier,
                      QUrl::toPercentEncoding(d->clientIdentifier));

    if (d->pkceMethod != PkceMethod::None)
        parameters.insert(QtOAuth2RfcKeywords::codeVerifier, d->pkceCodeVerifier);
    if (!d->clientIdentifierSharedKey.isEmpty())
        parameters.insert(QtOAuth2RfcKeywords::clientSharedSecret, d->clientIdentifierSharedKey);
    if (d->modifyParametersFunction)
        d->modifyParametersFunction(Stage::RequestingAccessToken, &parameters);
    query = QAbstractOAuthPrivate::createQuery(parameters);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));

    const QByteArray data = query.toString(QUrl::FullyEncoded).toLatin1();

    d->callNetworkRequestModifier(request, QAbstractOAuth::Stage::RequestingAccessToken);

    QNetworkReply *reply = d->networkAccessManager()->post(request, data);
    d->currentReply = reply;
    QAbstractOAuthReplyHandler *handler = replyHandler();
    QObject::connect(reply, &QNetworkReply::finished, handler,
                     [handler, reply] { handler->networkReplyFinished(reply); });
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    QObjectPrivate::connect(handler, &QAbstractOAuthReplyHandler::tokensReceived, d,
                            &QOAuth2AuthorizationCodeFlowPrivate::_q_tokenRequestFinished,
                            Qt::UniqueConnection);
    QObjectPrivate::connect(d->networkAccessManager(),
                            &QNetworkAccessManager::authenticationRequired,
                            d, &QOAuth2AuthorizationCodeFlowPrivate::_q_authenticate,
                            Qt::UniqueConnection);
    QObjectPrivate::connect(handler,
                            &QAbstractOAuthReplyHandler::tokenRequestErrorOccurred,
                            d, &QOAuth2AuthorizationCodeFlowPrivate::_q_tokenRequestFailed,
                            Qt::UniqueConnection);
}

/*!
    Builds an authentication URL using \a url and \a parameters. This
    function emits an authorizeWithBrowser() signal to require user
    interaction.
*/
void QOAuth2AuthorizationCodeFlow::resourceOwnerAuthorization(const QUrl &url,
                                                              const QMultiMap<QString, QVariant> &parameters)
{
    Q_D(QOAuth2AuthorizationCodeFlow);
    if (Q_UNLIKELY(url != d->authorizationUrl)) {
        qCWarning(d->loggingCategory, "Invalid URL: %s", qPrintable(url.toString()));
        return;
    }
    const QUrl u = buildAuthenticateUrl(parameters);
    QObjectPrivate::connect(this, &QOAuth2AuthorizationCodeFlow::authorizationCallbackReceived, d,
                            &QOAuth2AuthorizationCodeFlowPrivate::_q_handleCallback,
                            Qt::UniqueConnection);
    Q_EMIT authorizeWithBrowser(u);
}

QT_END_NAMESPACE

#include "moc_qoauth2authorizationcodeflow.cpp"
