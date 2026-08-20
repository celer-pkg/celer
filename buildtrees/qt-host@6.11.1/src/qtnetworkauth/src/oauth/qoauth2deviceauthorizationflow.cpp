// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include <private/qoauth2deviceauthorizationflow_p.h>
#include <QtNetworkAuth/qoauth2deviceauthorizationflow.h>
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qvariant.h>

#include <QtNetwork/qrestaccessmanager.h>
#include <QtNetwork/qrestreply.h>

#include <functional>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;
using Error = QAbstractOAuth::Error;
using Status = QAbstractOAuth::Status;
using Stage = QAbstractOAuth::Stage;

/*!
    \class QOAuth2DeviceAuthorizationFlow
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QOAuth2DeviceAuthorizationFlow class provides an
    implementation of the
    \l {https://datatracker.ietf.org/doc/html/rfc8628}
    {Device Authorization Grant} flow.
    \since 6.9

    This class implements the
    \l {https://datatracker.ietf.org/doc/html/rfc8628}
    {Device Authorization Grant} flow, which is used to obtain
    and refresh access tokens and ID tokens, particularly on devices lacking
    a user-agent or with limited input capabilities. These devices include
    televisions, machine HMIs, appliances, and IoT devices.

    Device flow can be used on any platform and operating system
    that is capable of SSL/TLS requests. Unlike
    \l {QOAuth2AuthorizationCodeFlow}, this flow is not based on
    redirects, and therefore does not use a
    \l {QAbstractOAuthReplyHandler}{reply handler}.

    \section1 Device Flow Usage

    The following snippets illustrate the typical usage. First, we set up
    the flow similarly to \l {QOAuth2AuthorizationCodeFlow}:
    \snippet src_oauth_replyhandlers.cpp deviceflow-setup

    Then we connect to
    \l {QOAuth2DeviceAuthorizationFlow::}{authorizeWithUserCode}
    signal to handle the user authorization:
    \snippet src_oauth_replyhandlers.cpp deviceflow-handle-authorizewithusercode
    This part is crucial to the flow, and how you handle it depends on your
    specific use case. One way or another, the user needs to complete the
    authorization.

    Device flow does not define how this authorization completion
    is done, making it versatile for different use cases.
    This can be achieved by displaying the verification URI and user code
    to the user, who can then navigate to it on another device.
    Alternatively, you could present a QR code for the user to
    scan with their mobile device, send to a companion application,
    email to the user, and so on.

    While authorization is pending, \l {QOAuth2DeviceAuthorizationFlow} polls
    the server at specific intervals (typically 5 seconds) until the user
    accepts or rejects the authorization, upon which the server responds
    accordingly and the flow concludes.

    Errors can be detected as follows:
    \snippet src_oauth_replyhandlers.cpp deviceflow-handle-errors
    \l {QAbstractOAuth2::serverReportedErrorOccurred()} signal can
    be used to get information on specific RFC-defined errors.
    However, unlike \l {QAbstractOAuth::requestFailed()}, it doesn't
    cover errors such as network errors or client configuration errors.

    Flow completion is detected similarly as with
    \l {QOAuth2AuthorizationCodeFlow}, for example:
    \snippet src_oauth_replyhandlers.cpp deviceflow-handle-grant
*/

/*!
    \property QOAuth2DeviceAuthorizationFlow::userCode

    This property holds the
    \l {https://datatracker.ietf.org/doc/html/rfc8628#section-3.2}{user_code}
    received in authorization response. This code is used by the user to
    complete the authorization.

    \sa verificationUrl, completeVerificationUrl, {Device Flow Usage}
*/

/*!
    \property QOAuth2DeviceAuthorizationFlow::verificationUrl

    This property holds the URL where user should enter the user code to
    complete authorization.

    \sa userCode, completeVerificationUrl, {Device Flow Usage}
*/

/*!
    \property QOAuth2DeviceAuthorizationFlow::completeVerificationUrl

    This property holds an URL for user to complete the authorization.
    The URL itself contains the \c {user_code} and thus avoids the
    need for user to enter the code manually. Support for this
    complete URL varies between authorization servers.

    \sa verificationUrl, {Device Flow Usage}
*/

/*!
    \property QOAuth2DeviceAuthorizationFlow::polling

    This property holds whether or not the flow is actively polling
    for tokens.

    \sa startTokenPolling(), stopTokenPolling()
*/

/*!
    \property QOAuth2DeviceAuthorizationFlow::userCodeExpirationAt

    This property holds the local time the user code and
    underlying device codes expire. The codes are typically
    valid between 5 and 30 minutes.

    \sa userCode
*/

/*!
    \fn void QOAuth2DeviceAuthorizationFlow::authorizeWithUserCode(
                                const QUrl &verificationUrl,
                                const QString &userCode,
                                const QUrl &completeVerificationUrl)

    This signal is emitted when user should complete the authorization.

    If authorization server has provided \a completeVerificationUrl,
    user can navigate to that URL. The URL contains the needed \a userCode
    and any other needed parameters.

    Alternatively, the user needs to navigate to \a verificationUrl
    and enter \a userCode manually.

    \sa {Device Flow Usage}
*/

QOAuth2DeviceAuthorizationFlowPrivate::QOAuth2DeviceAuthorizationFlowPrivate(
    QNetworkAccessManager *manager)
    : QAbstractOAuth2Private(std::make_pair(QString(), QString()), QUrl(), manager)
{
}

QOAuth2DeviceAuthorizationFlowPrivate::~QOAuth2DeviceAuthorizationFlowPrivate()
{
    resetCurrentAuthorizationReply();
    resetCurrentTokenReply();
    tokenPollingTimer.stop();
}

void QOAuth2DeviceAuthorizationFlowPrivate::authorizationReplyFinished(QRestReply &reply)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);

    if (status != Status::NotAuthenticated) {
        logAuthorizationStageWarning("reply finished in unexpected flow status"_L1,
                                  static_cast<int>(status));
        return;
    }
    // HTTP status is not checked, because, while RFC states that 400 (Bad Request)
    // should be used, it also says 'unless specified otherwise'. And indeed authorization
    // servers do use different statuses (eg. 428). So we only concern ourselves
    // with network errors and what the body data says.
    // https://datatracker.ietf.org/doc/html/rfc6749#section-5.2
    if (reply.hasError()) {
        logAuthorizationStageWarning("network error"_L1);
        emit q->requestFailed(Error::NetworkError);
        return;
    }
    const auto jsonDoc = reply.readJson();
    if (!jsonDoc || !jsonDoc->isObject()) {
        logAuthorizationStageWarning("invalid response format"_L1);
        emit q->requestFailed(Error::ServerError);
        return;
    }
    const auto data = jsonDoc->object();
    if (handleRfcErrorResponseIfPresent(data.toVariantMap()))
        return;

    // https://datatracker.ietf.org/doc/html/rfc8628#section-3.2 REQUIRED parameters
    const auto receivedDeviceCode = data.value(QtOAuth2RfcKeywords::deviceCode).toString();
    auto receivedUserCode = data.value(QtOAuth2RfcKeywords::userCode).toString();
    const auto receivedExpiresIn = data.value(QtOAuth2RfcKeywords::expiresIn).toInt();
    QUrl receivedVerificationUrl;
    // The RFC keyword is 'verification_uri', but some auth servers provide 'verification_url'
    if (auto it = data.find(QtOAuth2RfcKeywords::verificationUri); it != data.end())
        receivedVerificationUrl.setUrl(it->toString());
    else if (auto it = data.find(QtOAuth2RfcKeywords::verificationUrl); it != data.end())
        receivedVerificationUrl.setUrl(it->toString());

    if (receivedDeviceCode.isEmpty() || receivedUserCode.isEmpty()
        || receivedVerificationUrl.isEmpty() || receivedExpiresIn <= 0) {
        logAuthorizationStageWarning("required data not received"_L1);
        emit q->requestFailed(Error::OAuthTokenNotFoundError);
        return;
    }

    const int receivedMinimumInterval = data.value(QtOAuth2RfcKeywords::interval).toInt();
    if (receivedMinimumInterval > 0) {
        if (useAutoTestDurations)
            tokenPollingTimer.setInterval(std::chrono::milliseconds(receivedMinimumInterval));
        else
            tokenPollingTimer.setInterval(std::chrono::seconds(receivedMinimumInterval));
    } else {
        tokenPollingTimer.setInterval(defaultPollingInterval);
    }

    // Store the expiration time
    QDateTime newCodeExpiration;
    if (useAutoTestDurations)
        newCodeExpiration = QDateTime::currentDateTimeUtc().addMSecs(receivedExpiresIn);
    else
        newCodeExpiration = QDateTime::currentDateTimeUtc().addSecs(receivedExpiresIn);
    setUserCodeExpiration(newCodeExpiration);

    if (isNextPollAfterExpiration()) {
        logAuthorizationStageWarning("code expired"_L1);
        emit q->requestFailed(Error::ExpiredError);
        return;
    }

    QUrl receivedVerificationUrlComplete;
    // The RFC keyword is 'verification_uri_complete', but some auth servers
    // use 'verification_url_complete'
    if (auto it = data.find(QtOAuth2RfcKeywords::completeVerificationUri); it != data.end())
        receivedVerificationUrlComplete.setUrl(it->toString());
    else if (auto it = data.find(QtOAuth2RfcKeywords::completeVerificationUrl); it != data.end())
        receivedVerificationUrlComplete.setUrl(it->toString());

    deviceCode = std::move(receivedDeviceCode);
    setUserCode(receivedUserCode);
    setVerificationUrl(receivedVerificationUrl);
    setVerificationUrlComplete(receivedVerificationUrlComplete);

    QVariantMap copy(data.toVariantMap());
    copy.remove(QtOAuth2RfcKeywords::deviceCode);
    copy.remove(QtOAuth2RfcKeywords::userCode);
    copy.remove(QtOAuth2RfcKeywords::verificationUrl);
    copy.remove(QtOAuth2RfcKeywords::completeVerificationUrl);
    setExtraTokens(copy);

    setStatus(Status::TemporaryCredentialsReceived);
    // Signal that user needs to authorize next, and start polling for tokens
    emit q->authorizeWithUserCode(verificationUrl, userCode, completeVerificationUrl);
    (void)startTokenPolling();
}

void QOAuth2DeviceAuthorizationFlowPrivate::tokenReplyFinished(QRestReply &reply)
{
    // HTTP status is not checked, because, while RFC states that 400 (Bad Request)
    // should be used, it also says 'unless specified otherwise'. And indeed authorization
    // servers do use different statuses (eg. 428). So we only concern ourselves
    // with network errors and the body data.
    // https://datatracker.ietf.org/doc/html/rfc6749#section-5.2
    if (reply.hasError()) {
        tokenAcquisitionFailed(Error::NetworkError, reply.errorString());
        return;
    }
    const auto jsonDoc = reply.readJson();
    if (!jsonDoc || !jsonDoc->isObject()) {
        tokenAcquisitionFailed(Error::ServerError, u"Invalid response format"_s);
        return;
    }
    const auto data = jsonDoc->object();
    if (data.contains(QtOAuth2RfcKeywords::error)) {
        // With device flow, error responses can be part of a successful flow
        handleTokenErrorResponse(data);
        return;
    }
    handleTokenSuccessResponse(data);
}

void QOAuth2DeviceAuthorizationFlowPrivate::handleTokenErrorResponse(const QJsonObject &data)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    const auto errorCode = data.value(QtOAuth2RfcKeywords::error).toString();

    // https://datatracker.ietf.org/doc/html/rfc8628#section-3.5
    // RFC defines additional error codes that require specific handling
    if (errorCode == "authorization_pending"_L1) {
        // User has not yet authorized, keep polling
        //qCDebug(loggingCategory) << "Not yet authorized, polling again in:"
        //                         << tokenPollingTimer.interval();
    } else if (errorCode == "slow_down"_L1) {
        // Polling needs to slow down by (at least) 5 seconds
        if (useAutoTestDurations)
            tokenPollingTimer.setInterval(tokenPollingTimer.interval() + 50ms);
        else
            tokenPollingTimer.setInterval(tokenPollingTimer.interval() + 5s);
        qCDebug(loggingCategory) << "Slow down requested, polling again in"
             << std::chrono::duration_cast<std::chrono::milliseconds>(tokenPollingTimer.interval());
    } else {
        // Other errors are terminal
        // https://datatracker.ietf.org/doc/html/rfc8628#section-3.2
        // https://datatracker.ietf.org/doc/html/rfc6749#section-5.2
        const auto error = data.value(QtOAuth2RfcKeywords::error).toString();
        const auto description = data.value(QtOAuth2RfcKeywords::errorDescription).toString();
        const QUrl uri{data.value(QtOAuth2RfcKeywords::errorUri).toString()};
        qCDebug(loggingCategory) << "Token acquisition failed:" << error << description;
#if QT_DEPRECATED_SINCE(6, 13)
        QT_IGNORE_DEPRECATIONS(Q_EMIT q->error(error, description, uri);)
#endif
        Q_EMIT q->serverReportedErrorOccurred(error, description, uri);
        if (errorCode == "expired_token"_L1)
            tokenAcquisitionFailed(Error::ExpiredError, description);
        else
            tokenAcquisitionFailed(Error::ServerError, description);
    }
}

void QOAuth2DeviceAuthorizationFlowPrivate::tokenAcquisitionFailed(QAbstractOAuth::Error error,
                                                                   const QString &errorString)
{
    _q_tokenRequestFailed(error, errorString);
    stopTokenPolling();
}

void QOAuth2DeviceAuthorizationFlowPrivate::setUserCode(const QString &code)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (userCode == code)
        return;
    userCode = code;
    emit q->userCodeChanged(userCode);
}

void QOAuth2DeviceAuthorizationFlowPrivate::setVerificationUrl(const QUrl &url)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (verificationUrl == url)
        return;
    verificationUrl = url;
    emit q->verificationUrlChanged(verificationUrl);
}

void QOAuth2DeviceAuthorizationFlowPrivate::setVerificationUrlComplete(const QUrl &url)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (completeVerificationUrl == url)
        return;
    completeVerificationUrl = url;
    emit q->completeVerificationUrlChanged(completeVerificationUrl);
}

void QOAuth2DeviceAuthorizationFlowPrivate::setUserCodeExpiration(const QDateTime &expiration)
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    Q_ASSERT(!expiration.isValid() || expiration.timeSpec() == Qt::TimeSpec::UTC);
    if (userCodeExpirationUtc == expiration)
        return;
    userCodeExpirationUtc = expiration;
    emit q->userCodeExpirationAtChanged(userCodeExpirationUtc.toLocalTime());
}

bool QOAuth2DeviceAuthorizationFlowPrivate::startTokenPolling()
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);

    if (q->isPolling()) {
        qCDebug(loggingCategory, "Token stage: polling already active");
        return true;
    }
    if (deviceCode.isEmpty()) {
        logTokenStageWarning("missing device code for polling"_L1);
        emit q->requestFailed(Error::ClientError);
        return false;
    }
    if (tokenUrl.isEmpty()) {
        logTokenStageWarning("token URL is empty"_L1);
        emit q->requestFailed(Error::ClientError);
        return false;
    }
    if (isNextPollAfterExpiration()) {
        logTokenStageWarning("code expired"_L1);
        emit q->requestFailed(Error::ExpiredError);
        return false;
    }

    qCDebug(loggingCategory) << "Token stage: starting polling with interval:"
             << std::chrono::duration_cast<std::chrono::milliseconds>(tokenPollingTimer.interval());
    tokenPollingTimer.start();
    emit q->pollingChanged(true);
    return true;
}

void QOAuth2DeviceAuthorizationFlowPrivate::stopTokenPolling()
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (!q->isPolling())
        return;

    qCDebug(loggingCategory, "Token stage: Stopping token polling");
    resetCurrentTokenReply();
    tokenPollingTimer.stop();
    emit q->pollingChanged(false);
}

void QOAuth2DeviceAuthorizationFlowPrivate::pollTokens()
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (currentTokenReply) {
        logTokenStageWarning("poll request already in progress"_L1);
        return;
    }
    if (tokenUrl.isEmpty()) {
        tokenAcquisitionFailed(Error::ClientError, u"token URL is empty"_s);
        return;
    }
    if (QDateTime::currentDateTimeUtc() >= userCodeExpirationUtc) {
        tokenAcquisitionFailed(Error::ExpiredError, u"code expired"_s);
        return;
    }

    QMultiMap<QString, QVariant> parameters;
    // https://datatracker.ietf.org/doc/html/rfc8628#section-3.4
    static constexpr auto grantType = "urn:ietf:params:oauth:grant-type:device_code"_L1;
    parameters.insert(QtOAuth2RfcKeywords::grantType, QUrl::toPercentEncoding(grantType));
    parameters.insert(QtOAuth2RfcKeywords::deviceCode, QUrl::toPercentEncoding(deviceCode));
    parameters.insert(QtOAuth2RfcKeywords::clientIdentifier,
                      QUrl::toPercentEncoding(clientIdentifier));
    if (!clientIdentifierSharedKey.isEmpty())
        parameters.insert(QtOAuth2RfcKeywords::clientSharedSecret, clientIdentifierSharedKey);
    if (modifyParametersFunction)
        modifyParametersFunction(QAbstractOAuth2::Stage::RequestingAccessToken, &parameters);

    QUrlQuery query;
    for (const auto &[key, value] : std::as_const(parameters).asKeyValueRange())
        query.addQueryItem(key, value.toString());

    QNetworkRequest request(tokenUrl);
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType,
                   "application/x-www-form-urlencoded"_L1);
    request.setHeaders(headers);
#ifndef QT_NO_SSL
    if (sslConfiguration && !sslConfiguration->isNull())
        request.setSslConfiguration(*sslConfiguration);
#endif
    callNetworkRequestModifier(request, Stage::RequestingAccessToken);

    const QByteArray data = query.toString(QUrl::FullyEncoded).toLatin1();
    currentTokenReply = network()->post(request, data, q, [this](QRestReply &reply) {
        if (reply.networkReply() != currentTokenReply) {
            logTokenStageWarning("unexpected token reply"_L1);
            return;
        }
        qCDebug(loggingCategory, "Token stage: token reply finished");
        currentTokenReply->deleteLater();
        currentTokenReply.clear();
        tokenReplyFinished(reply);
    });
}

void QOAuth2DeviceAuthorizationFlowPrivate::reset()
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    resetCurrentAuthorizationReply();
    resetCurrentTokenReply();
    setUserCode({});
    setVerificationUrl({});
    setVerificationUrlComplete({});
    setUserCodeExpiration(QDateTime());
    setExtraTokens({});
    setExpiresAt(QDateTime());
    deviceCode.clear();
    if (q->isPolling()) {
        tokenPollingTimer.stop();
        emit q->pollingChanged(false);
    }
    tokenPollingTimer.setInterval(defaultPollingInterval);
    setStatus(Status::NotAuthenticated);
}

bool QOAuth2DeviceAuthorizationFlowPrivate::isNextPollAfterExpiration() const
{
    if (!userCodeExpirationUtc.isValid())
        return true;

    const QDateTime nextPoll = QDateTime::currentDateTimeUtc().addDuration(
        std::chrono::duration_cast<std::chrono::milliseconds>(tokenPollingTimer.interval()));

    return nextPoll > userCodeExpirationUtc;
}

void QOAuth2DeviceAuthorizationFlowPrivate::resetCurrentTokenReply()
{
    if (currentTokenReply) {
        const auto reply = currentTokenReply.get();
        // Clear current reply before abort to avoid handling the finished signal
        currentTokenReply.clear();
        reply->abort();
        reply->deleteLater();
    }
}

void QOAuth2DeviceAuthorizationFlowPrivate::resetCurrentAuthorizationReply()
{
    if (currentAuthorizationReply) {
        const auto reply = currentAuthorizationReply.get();
        // Clear current reply before abort to avoid handling the finished signal
        currentAuthorizationReply.clear();
        reply->abort();
        reply->deleteLater();
    }
}

QRestAccessManager *QOAuth2DeviceAuthorizationFlowPrivate::network()
{
    Q_Q(QOAuth2DeviceAuthorizationFlow);
    if (!restAccessManager) {
        // First usage, create
        restAccessManager = new QRestAccessManager(networkAccessManager(), q);
    } else if (restAccessManager->networkAccessManager() != networkAccessManager()) {
        // QNetworkAccessManager has changed, re-create
        resetCurrentAuthorizationReply();
        resetCurrentTokenReply();
        delete restAccessManager;
        restAccessManager = new QRestAccessManager(networkAccessManager(), q);
    }
    return restAccessManager;
}

void QOAuth2DeviceAuthorizationFlowPrivate::handleTokenSuccessResponse(const QJsonObject &data)
{
    _q_tokenRequestFinished(data.toVariantMap());
    stopTokenPolling();
}

/*!
    Constructs a QOAuth2DeviceAuthorizationFlow object.
*/
QOAuth2DeviceAuthorizationFlow::QOAuth2DeviceAuthorizationFlow()
    : QOAuth2DeviceAuthorizationFlow(static_cast<QObject*>(nullptr))
{
}

/*!
    Constructs a QOAuth2DeviceAuthorizationFlow object with parent
    object \a parent.
*/
QOAuth2DeviceAuthorizationFlow::QOAuth2DeviceAuthorizationFlow(QObject *parent)
    : QOAuth2DeviceAuthorizationFlow(nullptr, parent)
{
}

/*!
    Constructs a QOAuth2DeviceAuthorizationFlow object using \a parent
    as parent and sets \a manager as the network access manager.
*/
QOAuth2DeviceAuthorizationFlow::QOAuth2DeviceAuthorizationFlow(QNetworkAccessManager *manager,
                                                               QObject *parent)
    : QAbstractOAuth2(*new QOAuth2DeviceAuthorizationFlowPrivate(manager), parent)
{
    Q_D(QOAuth2DeviceAuthorizationFlow);
    d->tokenPollingTimer.setInterval(d->defaultPollingInterval);
    d->tokenPollingTimer.setSingleShot(false);
    connect(&d->tokenPollingTimer, &QChronoTimer::timeout, this, [d]() {
        d->pollTokens();
    });
}

/*!
    Destroys the QOAuth2DeviceAuthorizationFlow instance.
*/
QOAuth2DeviceAuthorizationFlow::~QOAuth2DeviceAuthorizationFlow()
    = default;

QString QOAuth2DeviceAuthorizationFlow::userCode() const
{
    Q_D(const QOAuth2DeviceAuthorizationFlow);
    return d->userCode;
}

QUrl QOAuth2DeviceAuthorizationFlow::verificationUrl() const
{
    Q_D(const QOAuth2DeviceAuthorizationFlow);
    return d->verificationUrl;
}

QUrl QOAuth2DeviceAuthorizationFlow::completeVerificationUrl() const
{
    Q_D(const QOAuth2DeviceAuthorizationFlow);
    return d->completeVerificationUrl;
}

bool QOAuth2DeviceAuthorizationFlow::isPolling() const
{
    Q_D(const QOAuth2DeviceAuthorizationFlow);
    return d->tokenPollingTimer.isActive();
}

QDateTime QOAuth2DeviceAuthorizationFlow::userCodeExpirationAt() const
{
    Q_D(const QOAuth2DeviceAuthorizationFlow);
    return d->userCodeExpirationUtc.toLocalTime();
}

/*!
    Starts the authorization flow as described in
    \l {https://datatracker.ietf.org/doc/html/rfc8628#section-3}{Device Grant RFC}.

    The flow consists of following steps:
    \list
        \li Authorization request to the authorization server
        \li User authorizing the access, see \l {authorizeWithUserCode()}
        \li Polling the authorization server until user has accepted
            or rejected the authorization (or codes expire)
        \li Indicating the result to the application (see granted() and
            QAbstractOAuth::requestFailed())
    \endlist
    The flow progresses automatically from authorization to token polling.

    Calling this function will reset any previous authorization data.

    \sa authorizeWithUserCode(), granted(), QAbstractOAuth::requestFailed(),
        polling, startTokenPolling(), stopTokenPolling(), {Device Flow Usage}
*/
void QOAuth2DeviceAuthorizationFlow::grant()
{
    Q_D(QOAuth2DeviceAuthorizationFlow);
    d->reset();

    if (d->authorizationUrl.isEmpty()) {
        d->logAuthorizationStageWarning("No authorization URL set"_L1);
        emit requestFailed(Error::ClientError);
        return;
    }
    if (d->tokenUrl.isEmpty()) {
        d->logAuthorizationStageWarning("No token URL set"_L1);
        emit requestFailed(Error::ClientError);
        return;
    }

    QMultiMap<QString, QVariant> parameters;
    parameters.insert(QtOAuth2RfcKeywords::clientIdentifier, d->clientIdentifier);
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    if (d->legacyScopeWasSetByUser) {
        if (!d->legacyScope.isEmpty())
            parameters.insert(QtOAuth2RfcKeywords::scope, d->legacyScope);
    } else
#endif
    if (!d->requestedScopeTokens.isEmpty())
        parameters.insert(QtOAuth2RfcKeywords::scope, d->joinedScope(d->requestedScopeTokens));
    if (d->authorizationShouldIncludeNonce()) {
        if (d->nonce.isEmpty())
            setNonce(QAbstractOAuth2Private::generateNonce());
        parameters.insert(QtOAuth2RfcKeywords::nonce, d->nonce);
    }
    if (d->modifyParametersFunction)
        d->modifyParametersFunction(Stage::RequestingAuthorization, &parameters);

    QUrlQuery query;
    for (const auto &[key, value] : std::as_const(parameters).asKeyValueRange())
        query.addQueryItem(key, value.toString());

    QNetworkRequest request(d->authorizationUrl);
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType,
                   "application/x-www-form-urlencoded"_L1);
    request.setHeaders(headers);

#ifndef QT_NO_SSL
    if (d->sslConfiguration && !d->sslConfiguration->isNull())
        request.setSslConfiguration(*d->sslConfiguration);
#endif
    d->callNetworkRequestModifier(request, Stage::RequestingAuthorization);

    const QByteArray data = query.toString(QUrl::FullyEncoded).toLatin1();
    d->currentAuthorizationReply =
        d->network()->post(request, data, this, [d](QRestReply &reply) {
            if (reply.networkReply() != d->currentAuthorizationReply) {
                d->logAuthorizationStageWarning("unexpected reply"_L1);
                return;
            }
            qCDebug(d->loggingCategory, "Authorization stage: reply finished");
            d->currentAuthorizationReply->deleteLater();
            d->currentAuthorizationReply.clear();
            d->authorizationReplyFinished(reply);
    });
}

/*!
    \since 6.9

    This function sends a token refresh request.

    If the refresh request was initiated successfully, the status is set to
    \l QAbstractOAuth::Status::RefreshingToken; otherwise the \l requestFailed()
    signal is emitted and the status is not changed. Tokens cannot be refreshed
    while \l {isPolling} is \c {true}.

    This function has no effect if the token refresh process is already in
    progress.

    If refreshing the token fails and an access token exists, the status is
    set to \l QAbstractOAuth::Status::Granted, and to
    \l QAbstractOAuth::Status::NotAuthenticated if an access token
    does not exist.

    \sa QAbstractOAuth::requestFailed(), QAbstractOAuth2::refreshTokens()
*/
void QOAuth2DeviceAuthorizationFlow::refreshTokensImplementation()
{
    Q_D(QOAuth2DeviceAuthorizationFlow);
    if (d->status == Status::RefreshingToken && d->currentTokenReply) {
        qCDebug(d->loggingCategory, "refresh already in progress");
        return;
    }
    if (isPolling()) {
        d->logTokenStageWarning("polling in progress, cannot refresh"_L1);
        emit requestFailed(Error::ClientError);
        return;
    }
    if (d->refreshToken.isEmpty()) {
        d->logTokenStageWarning("empty refresh token"_L1);
        emit requestFailed(Error::ClientError);
        return;
    }
    if (d->tokenUrl.isEmpty()) {
        d->logTokenStageWarning("No token URL set"_L1);
        emit requestFailed(Error::ClientError);
        return;
    }

    d->resetCurrentTokenReply();

    const auto [request, body] = d->createRefreshRequestAndBody(d->tokenUrl);
    d->currentTokenReply = d->network()->post(request, body, this, [d](QRestReply &reply) {
        if (reply.networkReply() != d->currentTokenReply) {
            d->logTokenStageWarning("unexpected refresh reply"_L1);
            return;
        }
        qCDebug(d->loggingCategory, "Token stage: refresh reply finished");
        d->currentTokenReply->deleteLater();
        d->currentTokenReply.clear();
        d->tokenReplyFinished(reply);
    });
    setStatus(Status::RefreshingToken);
}

/*!
    Starts token polling. Returns \c {true} if the start
    was successful (or was already active), and \c {false} otherwise.

    Calling this function is not necessary in a typical use case.
    Once the authorization request has completed,
    as a result of \l {grant()} call, the polling is started automatically.

    This function can be useful in cases where resuming (retrying)
    the token polling a bit later is needed, without restarting
    the whole authorization flow. For example in case of a transient
    network connectivity loss.

    Polling interval is defined by the authorization server, and is
    typically 5 seconds. First poll request is sent once the first interval
    has elapsed.

    \sa polling, stopTokenPolling(), {Device Flow Usage}
 */
bool QOAuth2DeviceAuthorizationFlow::startTokenPolling()
{
    Q_D(QOAuth2DeviceAuthorizationFlow);
    return d->startTokenPolling();
}

/*!
    Stops token polling. Any potential outstanding poll requests
    are silently discarded.

    \sa polling, startTokenPolling()
 */
void QOAuth2DeviceAuthorizationFlow::stopTokenPolling()
{
    Q_D(QOAuth2DeviceAuthorizationFlow);
    d->stopTokenPolling();
}

bool QOAuth2DeviceAuthorizationFlow::event(QEvent* event)
{
    // https://wiki.qt.io/Things_To_Look_Out_For_In_Reviews#New_Classes
    return QAbstractOAuth2::event(event);
}

QT_END_NAMESPACE

#include "moc_qoauth2deviceauthorizationflow.cpp"
