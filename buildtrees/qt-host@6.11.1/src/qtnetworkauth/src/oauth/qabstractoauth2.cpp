// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include <qabstractoauth2.h>
#include <private/qabstractoauth2_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qmessageauthenticationcode.h>

#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qhttpmultipart.h>

#ifndef QT_NO_SSL
#include <QtNetwork/qsslconfiguration.h>
#endif

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcOAuth2Validation, "qt.networkauth.oauth2.validation")

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

static constexpr auto MinimumRefreshLeadTime = 10s;
static constexpr auto FallbackRefreshInterval = 2s;

/*!
    \class QAbstractOAuth2
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QAbstractOAuth2 class is the base of all
    implementations of OAuth 2 authentication methods.
    \since 5.8

    The class defines the basic interface of the OAuth 2
    authentication classes. By inheriting this class, you
    can create custom authentication methods using the OAuth 2
    standard for different web services.

    A description of how OAuth 2 works can be found in:
    \l {https://tools.ietf.org/html/rfc6749}{The OAuth 2.0
    Authorization Framework}
*/

/*!
    \page oauth-http-method-alternatives
    \title OAuth2 HTTP method alternatives
    \brief This page provides alternatives for QtNetworkAuth
    OAuth2 HTTP methods.

    QtNetworkAuth provides HTTP Methods such as \l {QAbstractOAuth::get()}
    for issuing authenticated requests. In the case of OAuth2,
    this typically means setting the
    \l {QHttpHeaders::WellKnownHeader}{Authorization} header, as
    specified in \l {https://datatracker.ietf.org/doc/html/rfc6750#section-2.1}
    {RFC 6750}.

    Since this operation is straightforward to do, it is better to use
    the normal QtNetwork HTTP method APIs directly, and set this header
    manually. These QtNetwork APIs have less assumptions on the message
    content types and provide a broader set of APIs.

    See \l QRestAccessManager, \l QNetworkAccessManager, QNetworkRequest,
    QNetworkRequestFactory.

    \section1 QNetworkRequest

    The needed \e Authorization header can be set directly on each
    request needing authorization.

    \code
    using namespace Qt::StringLiterals;

    QOAuth2AuthorizationCodeFlow m_oauth;
    QNetworkRequest request;

    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::Authorization, u"Bearer "_s + m_oauth.token());
    request.setHeaders(headers);
    \endcode

    After setting the header, use the request normally with either
    \l QRestAccessManager or \l QNetworkAccessManager.

    \section1 QNetworkRequestFactory

    QNetworkRequestFactory is a convenience class introduced in Qt 6.7.
    It provides a suitable method for this task:
    \l {QNetworkRequestFactory::setBearerToken()}, as illustrated
    by the code below.

    \code
    QNetworkRequestFactory m_api({"https://www.example.com/v3"});
    QOAuth2AuthorizationCodeFlow m_oauth;
    // ...
    connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::granted, this, [this]{
        m_api.setBearerToken(m_oauth.token().toLatin1());
    });
    \endcode

    After setting the bearer token, use the request factory normally
    with either \l QRestAccessManager or \l QNetworkAccessManager.
*/

#ifndef QOAUTH2_NO_LEGACY_SCOPE
/*!
    \deprecated [6.13] Use requestedScopeTokens and grantedScopeTokens
    properties instead. This property will be removed in Qt 7.
    \property QAbstractOAuth2::scope
    \brief This property holds the desired scope which defines the
    permissions requested by the client.

    The scope value is updated to the scope value granted by the
    authorization server. In case of an empty scope response, the
    \l {https://datatracker.ietf.org/doc/html/rfc6749#section-5.1}
    {requested scope is assumed as granted and does not change}.

    The fact that this property serves two different roles, first
    as the requested scope and later as the granted scope, is an historical
    artefact. All new code is recommended to use
    \l QAbstractOAuth2::requestedScopeTokens and
    \l QAbstractOAuth2::grantedScopeTokens.

    \sa QAbstractOAuth2::grantedScopeTokens,
        QAbstractOAuth2::requestedScopeTokens
*/
#endif

/*!
    \since 6.9
    \property QAbstractOAuth2::grantedScopeTokens
    \brief This property holds the scope granted by the authorization
    server.

    The requested and granted scope may differ. End-user may have opted
    to grant only a subset of the scope, or server-side policies may
    change it. The application should be prepared to handle this
    scenario, and check the granted scope to see if it should impact
    the application logic.

    The server may omit indicating the granted scope altogether, as defined by
    \l {https://datatracker.ietf.org/doc/html/rfc6749#section-5.1}{RFC 6749}.
    In this case the implementation assumes the granted scope is the same as
    the requested scope.

    \sa QAbstractOAuth2::requestedScopeTokens
*/

/*!
    \since 6.9
    \property QAbstractOAuth2::requestedScopeTokens
    \brief This property holds the desired scope which defines the
    permissions requested by the client.

    \note Scope tokens are limited to a
    \l {https://datatracker.ietf.org/doc/html/rfc6749#section-3.3}{subset}
    of printable US-ASCII characters. Using characters outside this range
    is not supported.

    \sa QAbstractOAuth2::grantedScopeTokens
*/

/*!
    \since 6.9
    \property QAbstractOAuth2::nonceMode
    \brief This property holds the current nonce mode (whether or not
           nonce is used).

    \sa NonceMode, nonce
*/

/*!
    \since 6.9
    \enum QAbstractOAuth2::NonceMode

    List of available
    \l {https://openid.net/specs/openid-connect-core-1_0-final.html#IDToken}{nonce}
    modes.

    \value Automatic Nonce is sent if the
           \l {requestedScopeTokens}{requested scope} contains \c {openid}.
           This is the default mode, and sends \c {nonce} only when it's
           relevant to OIDC authentication flows.
    \value Enabled Nonce is sent during authorization stage.
    \value Disabled Nonce is not sent during authorization stage.

    \sa nonce, {OAuth 2.0 Overview}
*/

/*!
    \since 6.9
    \property QAbstractOAuth2::nonce

    This property holds the string sent to the server during
    authentication. The nonce is used to associate applicable
    token responses (OpenID Connect \c {id_token} in particular)
    with the authorization stage.

    The primary purpose of the \c {nonce} is to mitigate replay attacks.
    It ensures that the token responses received are in response
    to the authentication requests initiated by the application,
    preventing attackers from reusing tokens in unauthorized contexts.
    Therefore, it's important to include nonce verification as part of
    the token validation.

    In practice, authorization server vendors may refuse the OpenID Connect
    request if \l {https://openid.net/specs/openid-connect-core-1_0-final.html#AuthRequest}
    {a nonce isn't provided in the Authorization request}.

    The token itself is an opaque string, and should contain only
    URL-safe characters for maximum compatibility. Further the
    token must provide adequate entropy
    \l {https://openid.net/specs/openid-connect-core-1_0-final.html#NonceNotes}
    {so that it's unguessable to attackers}. There are no strict size
    limits for nonce, and authorization server vendors may impose their own
    minimum and maximum sizes.

    While the \c {nonce} can be set manually, Qt classes will
    generate a 32-character nonce \l {NonceMode}{when needed} if
    one isn't set.

    \sa nonceMode, {Qt OpenID Connect Support}
*/

/*!
    \since 6.9
    \property QAbstractOAuth2::idToken

    This property holds the received
    \l {https://openid.net/specs/openid-connect-core-1_0-final.html#CodeIDToken}
    {OpenID Connect ID token}.

    \sa NonceMode, nonce, {Qt OpenID Connect Support}
*/

/*!
    \fn template<typename Functor, QAbstractOAuth2::if_compatible_callback<Functor>> void QAbstractOAuth2::setNetworkRequestModifier(
                        const ContextTypeForFunctor<Functor> *context,
                        Functor &&callback)
    \since 6.9

    Sets the network request modification function to \a callback.
    This function is used to customize the network requests sent
    to the server.

    \a callback has to implement the signature
    \c {void(QNetworkRequest&, QAbstractOAuth::Stage)}. The provided
    QNetworkRequest can be directly modified, and it is used right after
    the callback finishes. \a callback can be a function pointer, lambda,
    member function, or any callable object. The provided
    QAbstractOAuth::Stage can be used to check to which stage
    the request relates to (token request, token refresh request,
    or authorization request in case of QOAuth2DeviceAuthorizationFlow).

    \a context controls the lifetime of the calls, and prevents
    access to de-allocated resources in case \a context is destroyed.
    In other words, if the object provided as context is destroyed,
    callbacks won't be executed. \a context must point to a valid
    QObject (and in case the callback is a member function,
    it needs to actually have it). Since the callback's results
    are used immediately, \a context must reside in the same
    thread as the QAbstractOAuth2 instance.

    \sa clearNetworkRequestModifier(), QNetworkRequest
*/

/*!
    \property QAbstractOAuth2::userAgent
    This property holds the User-Agent header used to create the
    network requests.

    The default value is "QtOAuth/1.0 (+https://www.qt.io)".
*/

/*!
    \property QAbstractOAuth2::clientIdentifierSharedKey
    This property holds the client shared key used as a password if
    the server requires authentication to request the token.
*/

/*!
    \property QAbstractOAuth2::state
    This property holds the string sent to the server during
    authentication. The state is used to identify and validate the
    request when the callback is received.

    Certain characters are illegal in the state element (see
    \l {https://datatracker.ietf.org/doc/html/rfc6749#appendix-A.5}{RFC 6749}).
    The use of illegal characters could lead to an unintended state mismatch
    and a failing OAuth 2 authorization. Therefore, if you attempt to set
    a value that contains illegal characters, the state is ignored and a
    warning is logged.
*/

/*!
    \property QAbstractOAuth2::expiration
    This property holds the expiration time of the current access
    token. An invalid value means that the authorization server hasn't
    provided a valid expiration time.

    \sa QDateTime::isValid()
*/

/*!
    \fn QAbstractOAuth2::accessTokenAboutToExpire()
    \since 6.9
    \brief The signal is emitted when the access token is about
    to expire.

    Emitting this signal requires that the access token has
    a valid expiration time. An alternative for handling this
    signal manually is to use \l {autoRefresh}.

    \sa refreshLeadTime, autoRefresh, refreshTokens()
*/

/*!
    \property QAbstractOAuth2::refreshLeadTime
    \since 6.9
    \brief This property defines how far in advance
    \l {accessTokenAboutToExpire()} signal is emitted relative
    to the access token's expiration.

    This property specifies the time interval (in seconds)
    before the current access token’s expiration, when
    \l {accessTokenAboutToExpire()} signal is emitted. The value
    set for this property must be a positive duration.

    This interval allows the application to refresh the token well
    in advance, ensuring continuous authorization without interruptions.

    If this property is not explicitly set, or the provided leadTime is
    larger than the token's lifetime, the leadTime defaults to
    5% of the token's remaining lifetime, but not less than 10 seconds ahead
    of expiration (leaving time for the refresh request to complete).

    \note Expiration signal only works if the authorization server has provided
    a proper expiration time.

    \sa autoRefresh
*/

/*!
    \property QAbstractOAuth2::autoRefresh
    \since 6.9
    \brief This property enables or disables automatic refresh of
    the access token.

    This property enables or disables the automatic refresh of the
    access token. This is useful for applications that require uninterrupted
    authorization without user intervention.

    If this property is \c true, \l refreshTokens()
    will be automatically called when the token is about to expire and
    a valid \l refreshToken exists.

    \sa refreshLeadTime, accessTokenAboutToExpire()
*/

/*!
    \property QAbstractOAuth2::tokenUrl
    \since 6.9

    This property holds the token endpoint URL which is used to obtain tokens.
    Depending on the use case and authorization server support, these tokens
    can be access tokens, refresh tokens, and ID tokens.

    Tokens are typically retrieved once the authorization stage is completed,
    and the token endpoint can also be used to refresh tokens as needed.

    For example, \l QOAuth2AuthorizationCodeFlow uses this url to issue
    \l {https://tools.ietf.org/html/rfc6749#section-4.1.3}
    {an access token request},
    and \l QOAuth2DeviceAuthorizationFlow uses this url
    \l {https://datatracker.ietf.org/doc/html/rfc8628#section-3.4}
    {to poll for an access token}.
*/

/*!
    \deprecated [6.13] Use serverReportedErrorOccurred instead
    \fn QAbstractOAuth2::error(const QString &error, const QString &errorDescription, const QUrl &uri)

    Signal emitted when the server responds to the authorization request with
    an error as defined in \l {https://www.rfc-editor.org/rfc/rfc6749#section-5.2}
    {RFC 6749 error response}.

    \a error is the name of the error; \a errorDescription describes the error
    and \a uri is an optional URI containing more information about the error.

    \sa QAbstractOAuth::requestFailed()
    \sa QAbstractOAuth2::serverReportedErrorOccurred()
*/

/*!
    \fn QAbstractOAuth2::serverReportedErrorOccurred(const QString &error,
                                                     const QString &errorDescription,
                                                     const QUrl &uri)
    \since 6.9

    Signal emitted when the server responds to the authorization request with
    an error as defined in \l {https://www.rfc-editor.org/rfc/rfc6749#section-5.2}
    {RFC 6749 error response}.

    \a error is the name of the error; \a errorDescription describes the error
    and \a uri is an optional URI containing more information about the error.

    To catch all errors, including these RFC defined errors, with a
    single signal, use \l {QAbstractOAuth::requestFailed()}.
*/

/*!
    \fn QAbstractOAuth2::authorizationCallbackReceived(const QVariantMap &data)

    Signal emitted when the reply server receives the authorization
    callback from the server: \a data contains the values received
    from the server.
*/

QAbstractOAuth2Private::QAbstractOAuth2Private(const std::pair<QString, QString> &clientCredentials,
                                               const QUrl &authorizationUrl,
                                               QNetworkAccessManager *manager) :
    QAbstractOAuthPrivate("qt.networkauth.oauth2",
                          authorizationUrl,
                          clientCredentials.first,
                          manager),
    clientIdentifierSharedKey(clientCredentials.second)
{}

QAbstractOAuth2Private::~QAbstractOAuth2Private()
{}

void QAbstractOAuth2Private::setExpiresAt(const QDateTime &expiration)
{
    Q_ASSERT(!expiration.isValid() || expiration.timeSpec() == Qt::TimeSpec::UTC);
    if (expiresAtUtc == expiration)
        return;
    Q_Q(QAbstractOAuth2);
    expiresAtUtc = expiration;
    emit q->expirationAtChanged(expiresAtUtc.toLocalTime());
}

void QAbstractOAuth2Private::setGrantedScopeTokens(const QSet<QByteArray> &tokens)
{
    if (tokens == grantedScopeTokens)
        return;
    Q_Q(QAbstractOAuth2);
    grantedScopeTokens = tokens;
    Q_EMIT q->grantedScopeTokensChanged(grantedScopeTokens);
}

QString QAbstractOAuth2Private::joinedScope(const QSet<QByteArray> &scopeTokens)
{
    QString result;
    auto separator = ""_L1;
    for (const auto &token : scopeTokens) {
        result += separator;
        result += QLatin1StringView{token};
        separator = " "_L1;
    }
    return result;
}

QSet<QByteArray> QAbstractOAuth2Private::splitScope(QStringView scope)
{
    QSet<QByteArray> result;
    for (auto token : scope.tokenize(u' ', Qt::SkipEmptyParts)) {
        warnOnInvalidScopeToken(token);
        result.insert(token.toUtf8());
    }
    return result;
}

constexpr auto is_invalid_scope_token_char = [](char16_t ch) noexcept
{
    // https://datatracker.ietf.org/doc/html/rfc6749#section-3.3
    //   scope-token = 1*( %x21 / %x23-5B / %x5D-7E )
    //   ie. all US-ASCII, except control, SP, DQUOTE, and BACKSLASH
    return ch < 0x21 || ch == 0x22 || ch == 0x5C || ch > 0x7E;
};

bool QAbstractOAuth2Private::checkRequestedScopeTokensValid(const QSet<QByteArray> &scopeTokens)
{
    return std::all_of(scopeTokens.begin(), scopeTokens.end(),
                       [] (auto token) { return checkRequestedScopeTokenValid(token); });
}

/*!
    \internal

    Validates an RFC 6749 scope-token in octet-stream form, ie. before
    serialization. Warns also if \a token is empty (since it will then vanish
    on serialization (cause two spaces instead of one), which is probably not
    what the user wanted.
*/
bool QAbstractOAuth2Private::checkRequestedScopeTokenValid(QByteArrayView token)
{
    if (token.isEmpty()) {
        qCWarning(lcOAuth2Validation, "A scope token cannot be empty.");
        return false;
    }

    // The predicate takes char16_t, but the range is of value_type char. But
    // the signedness of `char` doesn't matter here: regardless of sign an
    // invalid char remains an invalid char after conversion to char16_t:
    const auto it = std::find_if(token.begin(), token.end(), is_invalid_scope_token_char);
    if (it != token.end()) {
        qCWarning(lcOAuth2Validation,
                  "A scope token cannot contain disallowed character '%c' (0x%02x). "
                  "Note that Qt requires scope-tokens are RFC 6749-compliant US-ASCII-only. "
                  "Please continue to use the QAbstractOAuth2::scope property for the time "
                  "being, and consider filing a bug if you need this behavior.",
                  *it, uchar(*it));
        return false;
    }

    return true;
}

/*!
    \internal

    Validates an RFC 6749 scope-token in UTF-16 encoding, ie. while splitting a
    scope into scope-tokens. This function assumes that \a token is not empty,
    because the cardinality of spaces separating scope-tokens is not
    significant, so Qt::SkipEmptyParts should have been used by caller).
*/
void QAbstractOAuth2Private::warnOnInvalidScopeToken(QStringView token)
{
    if (!lcOAuth2Validation().isWarningEnabled())
        return;

    Q_ASSERT(!token.isEmpty()); // we only operate in UTF-16 space while splitting,
                                // which we do along " +", so this cannot happen.

    const auto b = token.utf16(); // want char16_t, not QChar
    const auto e = b + token.size();
    const auto it = std::find_if(b, e, is_invalid_scope_token_char);
    if (it != e) {
        qCWarning(lcOAuth2Validation,
                  "Scope token contains disallowed character '%s' (0x%04x). "
                  "This may cause interoperability issues",
                  qUtf8Printable(QChar(*it)), *it);
    }
}

QString QAbstractOAuth2Private::generateRandomState()
{
    return QString::fromLatin1(QAbstractOAuthPrivate::generateRandomBase64String(8));
}

QString QAbstractOAuth2Private::generateNonce()
{
    // There is no strict minimum or maximum size for nonce, but
    // generating a 32-character base64 URL string provides
    // ~192 bits of entropy (32 characters * 6 bits per character).
    return QString::fromLatin1(QAbstractOAuthPrivate::generateRandomBase64String(32));
}

QNetworkRequest QAbstractOAuth2Private::createRequest(QUrl url, const QVariantMap *parameters)
{
    QUrlQuery query(url.query());

    QNetworkRequest request;
    if (parameters) {
        for (auto it = parameters->begin(), end = parameters->end(); it != end; ++it)
            query.addQueryItem(it.key(), it.value().toString());
        url.setQuery(query);
    } else { // POST, PUT request
        addContentTypeHeaders(&request);
    }

    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    const QString bearer = bearerFormat.arg(token);
    request.setRawHeader("Authorization", bearer.toUtf8());
    return request;
}

void QAbstractOAuth2Private::initializeRefreshHandling()
{
    Q_Q(QAbstractOAuth2);
    refreshTimer.setSingleShot(true);
    QObject::connect(q, &QAbstractOAuth2::expirationAtChanged, q, [this]() {
        updateRefreshTimer(false);
    });
    QObject::connect(&refreshTimer, &QChronoTimer::timeout, q,
                     &QAbstractOAuth2::accessTokenAboutToExpire);
    QObject::connect(q, &QAbstractOAuth2::accessTokenAboutToExpire, q, [q] {
        if (q->autoRefresh() && !q->refreshToken().isEmpty())
            q->refreshTokens();
    });
}

void QAbstractOAuth2Private::updateRefreshTimer(bool clientSideUpdate)
{
    Q_Q(QAbstractOAuth2);
    qCDebug(loggingCategory, "Updating refresh timer");

    refreshTimer.stop();

    if (!q->expirationAt().isValid()) {
        qCDebug(loggingCategory, "Expiration time not valid");
        return;
    }

    auto leadTime = q->refreshLeadTime();
    std::chrono::seconds untilNextExpiration = std::chrono::seconds(
            QDateTime::currentDateTime().secsTo(q->expirationAt()));

    // If leadTime is zero, or larger than token's lifetime, estimate a decent leadTime
    if (leadTime == 0s || leadTime.count() >= tokenLifetime) {
        leadTime = qMax(MinimumRefreshLeadTime, untilNextExpiration / 20);
        qCDebug(loggingCategory, "Adjusted expiration leadTime to %lld seconds",
                static_cast<long long>(leadTime.count()));
    }

    std::chrono::seconds interval = untilNextExpiration - leadTime;

    if (interval < MinimumRefreshLeadTime) {
        if (clientSideUpdate) {
            // Refresh timer update was triggered by the application, and the pre-existing
            // token is near expiration => emit expiration immediately
            qCDebug(loggingCategory, "Token expiration immediate");
            emit q->accessTokenAboutToExpire();
            return;
        }
        // Refresh update was triggered by a response from authorization server,
        // and the token is already near expiration. Use a miminum update interval to avoid
        // intense refresh request looping (treat as a misconfigured authorization server)
        interval = qMax(interval, FallbackRefreshInterval);
    }

    qCDebug(loggingCategory, "Token refresh timer will expire in %lld seconds",
            static_cast<long long>(interval.count()));
    refreshTimer.setInterval(interval);
    refreshTimer.start();
}

bool QAbstractOAuth2Private::authorizationShouldIncludeNonce() const
{
    switch (nonceMode) {
    case QAbstractOAuth2::NonceMode::Enabled:
        return true;
    case QAbstractOAuth2::NonceMode::Disabled:
        return false;
    case QAbstractOAuth2::NonceMode::Automatic:
        return requestedScopeTokens.contains("openid");
    };
    return false;
}

void QAbstractOAuth2Private::setIdToken(const QString &token)
{
    Q_Q(QAbstractOAuth2);
    if (idToken == token)
        return;
    idToken = token;
    emit q->idTokenChanged(idToken);
}

void QAbstractOAuth2Private::_q_tokenRequestFailed(QAbstractOAuth::Error error,
                                                const QString& errorString)
{
    Q_Q(QAbstractOAuth);
    qCWarning(loggingCategory) << "Token request failed:" << errorString;
    // If we were refreshing, reset status to Granted if we have an access token.
    // The access token might still be valid, and even if it wouldn't be,
    // refreshing can be attempted again.
    if (q->status() == QAbstractOAuth::Status::RefreshingToken) {
        if (!q->token().isEmpty())
            setStatus(QAbstractOAuth::Status::Granted);
        else
            setStatus(QAbstractOAuth::Status::NotAuthenticated);
    }
    emit q->requestFailed(error);
}

void QAbstractOAuth2Private::_q_tokenRequestFinished(const QVariantMap &values)
{
    Q_Q(QAbstractOAuth2);

    if (values.contains(QtOAuth2RfcKeywords::error)) {
        _q_tokenRequestFailed(QAbstractOAuth::Error::ServerError,
                                    values.value(QtOAuth2RfcKeywords::error).toString());
        return;
    }

    bool ok;
    const QString accessToken = values.value(QtOAuth2RfcKeywords::accessToken).toString();
    tokenType = values.value(QtOAuth2RfcKeywords::tokenType).toString();
    tokenLifetime = values.value(QtOAuth2RfcKeywords::expiresIn).toLongLong(&ok);
    if (!ok)
        tokenLifetime = 0;
    if (values.value(QtOAuth2RfcKeywords::refreshToken).isValid())
        q->setRefreshToken(values.value(QtOAuth2RfcKeywords::refreshToken).toString());

    if (accessToken.isEmpty()) {
        _q_tokenRequestFailed(QAbstractOAuth::Error::OAuthTokenNotFoundError,
                                    "Access token not received"_L1);
        return;
    }
    q->setToken(accessToken);

    // RFC 6749 section 5.1 https://datatracker.ietf.org/doc/html/rfc6749#section-5.1
    // If the requested scope and granted scopes differ, server is REQUIRED to return
    // the scope. If OTOH the scopes match, the server MAY omit the scope in the response,
    // in which case we assume that the granted scope matches the requested scope.
    //
    // Note: 'scope' variable has two roles: requested scope, and later granted scope.
    // Therefore 'scope' needs to be set if the granted scope differs from 'scope'.
    QString receivedGrantedScope = values.value(QtOAuth2RfcKeywords::scope).toString();
    const QSet<QByteArray> splitGrantedScope = splitScope(receivedGrantedScope);
    if (splitGrantedScope.isEmpty()) {
        setGrantedScopeTokens(requestedScopeTokens);
    } else {
        setGrantedScopeTokens(splitGrantedScope);
#ifndef QOAUTH2_NO_LEGACY_SCOPE
        if (receivedGrantedScope != legacyScope) {
            legacyScope = std::move(receivedGrantedScope);
            QT_IGNORE_DEPRECATIONS(Q_EMIT q->scopeChanged(legacyScope);)
        }
#endif
    }

    // An id_token must be included if this was an OIDC request and the server granted it.
    // https://openid.net/specs/openid-connect-core-1_0-final.html#AuthRequest (cf. 'scope')
    // https://openid.net/specs/openid-connect-core-1_0-final.html#TokenResponse
    //
    // However token refresh responses 'might not' contain an id_token
    // https://openid.net/specs/openid-connect-core-1_0-final.html#RefreshTokenResponse
    const QString receivedIdToken = values.value(QtOAuth2RfcKeywords::idToken).toString();
    if (requestedScopeTokens.contains("openid") && grantedScopeTokens.contains("openid")
        && receivedIdToken.isEmpty() && status != QAbstractOAuth::Status::RefreshingToken) {
        setIdToken({});
        _q_tokenRequestFailed(QAbstractOAuth::Error::OAuthTokenNotFoundError,
                              "'openid' scope requested and granted but ID token not received"_L1);
        return;
    }

    // Update the id_token. However if we were refreshing tokens and didn't receive a new
    // id_token, keep the original
    if (status != QAbstractOAuth::Status::RefreshingToken || !receivedIdToken.isEmpty())
        setIdToken(receivedIdToken);

    if (tokenLifetime > 0)
        setExpiresAt(QDateTime::currentDateTimeUtc().addSecs(tokenLifetime));
    else
        setExpiresAt(QDateTime());

    QVariantMap copy(values);
    copy.remove(QtOAuth2RfcKeywords::accessToken);
    copy.remove(QtOAuth2RfcKeywords::expiresIn);
    copy.remove(QtOAuth2RfcKeywords::refreshToken);
    copy.remove(QtOAuth2RfcKeywords::scope);
    copy.remove(QtOAuth2RfcKeywords::tokenType);
    copy.remove(QtOAuth2RfcKeywords::idToken);
    QVariantMap newExtraTokens = extraTokens;
    newExtraTokens.insert(copy);
    setExtraTokens(newExtraTokens);

    setStatus(QAbstractOAuth::Status::Granted);
}

bool QAbstractOAuth2Private::handleRfcErrorResponseIfPresent(const QVariantMap &data)
{
    Q_Q(QAbstractOAuth2);
    const QString error = data.value(QtOAuth2RfcKeywords::error).toString();

    if (error.size()) {
        // RFC 6749, Section 5.2 Error Response
        const QString uri = data.value(QtOAuth2RfcKeywords::errorUri).toString();
        const QString description = data.value(QtOAuth2RfcKeywords::errorDescription).toString();
        qCWarning(loggingCategory, "Authorization stage: AuthenticationError: %ls(%ls): %ls",
                  qUtf16Printable(error), qUtf16Printable(uri), qUtf16Printable(description));

        const QUrl url = QUrl{uri};
#if QT_DEPRECATED_SINCE(6, 13)
        QT_IGNORE_DEPRECATIONS(Q_EMIT q->error(error, description, url);)
#endif
        Q_EMIT q->serverReportedErrorOccurred(error, description, url);

        // Emit also requestFailed() so that it is a signal for all errors
        emit q->requestFailed(QAbstractOAuth::Error::ServerError);
        return true;
    }
    return false;
}

QAbstractOAuth2Private::RequestAndBody QAbstractOAuth2Private::createRefreshRequestAndBody(
    const QUrl &url)
{
    RequestAndBody result;
    result.request.setUrl(url);

    QMultiMap<QString, QVariant> parameters;
#ifndef QT_NO_SSL
    if (sslConfiguration && !sslConfiguration->isNull())
        result.request.setSslConfiguration(*sslConfiguration);
#endif
    QUrlQuery query;
    parameters.insert(QtOAuth2RfcKeywords::grantType, QStringLiteral("refresh_token"));
    parameters.insert(QtOAuth2RfcKeywords::refreshToken, refreshToken);
    parameters.insert(QtOAuth2RfcKeywords::clientIdentifier, clientIdentifier);
    parameters.insert(QtOAuth2RfcKeywords::clientSharedSecret, clientIdentifierSharedKey);
    if (modifyParametersFunction)
        modifyParametersFunction(QAbstractOAuth::Stage::RefreshingAccessToken, &parameters);
    query = QAbstractOAuthPrivate::createQuery(parameters);
    result.request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));

    callNetworkRequestModifier(result.request, QAbstractOAuth::Stage::RefreshingAccessToken);
    result.body = query.toString(QUrl::FullyEncoded).toLatin1();

    return result;
}

void QAbstractOAuth2Private::logAuthorizationStageWarning(QLatin1StringView message)
{
    static constexpr auto base = "Authorization stage: %s";
    qCWarning(loggingCategory, base, message.latin1());
}

void QAbstractOAuth2Private::logAuthorizationStageWarning(QLatin1StringView message, int detail)
{
    static constexpr auto base = "Authorization stage: %s: %d";
    qCWarning(loggingCategory, base, message.latin1(), detail);
}

void QAbstractOAuth2Private::logTokenStageWarning(QLatin1StringView message)
{
    static constexpr auto base = "Token stage: %s";
    qCWarning(loggingCategory, base, message.latin1());
}

bool QAbstractOAuth2Private::verifyThreadAffinity(const QObject *contextObject)
{
    Q_Q(QAbstractOAuth2);
    if (contextObject && (contextObject->thread() != q->thread())) {
        qCWarning(loggingCategory, "Context object must reside in the same thread");
        return false;
    }
    return true;
}

void QAbstractOAuth2Private::callNetworkRequestModifier(QNetworkRequest &request,
                                                       QAbstractOAuth::Stage stage)
{
    if (networkRequestModifier.contextObject && networkRequestModifier.slot) {
        if (!verifyThreadAffinity(networkRequestModifier.contextObject)) {
            Q_Q(QAbstractOAuth2);
            q->clearNetworkRequestModifier();
            return;
        }
        void *argv[] = { nullptr, &request, &stage};
        networkRequestModifier.slot->call(
            const_cast<QObject*>(networkRequestModifier.contextObject.get()), argv);
    }
}

/*!
    \reimp
*/
void QAbstractOAuth2::prepareRequest(QNetworkRequest *request, const QByteArray &verb,
                                     const QByteArray &body)
{
    Q_D(QAbstractOAuth2);
    Q_UNUSED(verb);
    Q_UNUSED(body);
    request->setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);
    const QString bearer = d->bearerFormat.arg(d->token);
    request->setRawHeader("Authorization", bearer.toUtf8());
}

/*!
    Constructs a QAbstractOAuth2 object using \a parent as parent.
*/
QAbstractOAuth2::QAbstractOAuth2(QObject *parent) :
    QAbstractOAuth2(nullptr, parent)
{}

/*!
    Constructs a QAbstractOAuth2 object using \a parent as parent and
    sets \a manager as the network access manager.
*/
QAbstractOAuth2::QAbstractOAuth2(QNetworkAccessManager *manager, QObject *parent) :
    QAbstractOAuth(*new QAbstractOAuth2Private(std::make_pair(QString(), QString()),
                                               QUrl(),
                                               manager),
                   parent)
{
    Q_D(QAbstractOAuth2);
    d->initializeRefreshHandling();
}

QAbstractOAuth2::QAbstractOAuth2(QAbstractOAuth2Private &dd, QObject *parent) :
    QAbstractOAuth(dd, parent)
{
    Q_D(QAbstractOAuth2);
    d->initializeRefreshHandling();
}

void QAbstractOAuth2::setResponseType(const QString &responseType)
{
    Q_D(QAbstractOAuth2);
    if (d->responseType != responseType) {
        d->responseType = responseType;
        Q_EMIT responseTypeChanged(responseType);
    }
}

void QAbstractOAuth2::setNetworkRequestModifierImpl(const QObject* context,
                                                   QtPrivate::QSlotObjectBase *slot)
{
    Q_D(QAbstractOAuth2);

    if (!context) {
        qCWarning(d->loggingCategory, "Context object must not be null, ignoring");
        return;
    }
    if (!d->verifyThreadAffinity(context))
        return;

    d->networkRequestModifier.contextObject = context;
    d->networkRequestModifier.slot.reset(slot);
}

/*!
    Clears the network request modifier.

    \sa setNetworkRequestModifier()
*/
void QAbstractOAuth2::clearNetworkRequestModifier()
{
    Q_D(QAbstractOAuth2);
    d->networkRequestModifier = {nullptr, nullptr};
}

/*!
    \since 6.9

    Call this function to refresh the tokens. The function calls
    \l {refreshTokensImplementation()} to perform the actual refresh.

    \sa refreshTokensImplementation(), autoRefresh
*/
void QAbstractOAuth2::refreshTokens()
{
#if QT_VERSION < QT_VERSION_CHECK(7, 0, 0)
    // Due to binary compatibility reasons we can't add new virtuals to this baseclass,
    // but instead we mimic the virtual by invoking the implementation as a slot
    QMetaObject::invokeMethod(this, "refreshTokensImplementation", Qt::DirectConnection);
#else
    refreshTokensImplementation();
#endif
}

/*!
    \fn void QAbstractOAuth2::refreshTokensImplementation()
    \since 6.9

    This slot is called by \l refreshTokens() to send the token
    refresh request.

\if defined(qt7)
    The derived classes \e must reimplement this slot to support token
    refreshing:
\else
    The derived classes \e should reimplement this slot to support token
    refreshing:
\endif
    \snippet src_oauth_replyhandlers.cpp custom-class-def-start
    \dots
    \snippet src_oauth_replyhandlers.cpp custom-class-def-end
    \codeline
    \snippet src_oauth_replyhandlers.cpp custom-class-impl

    \sa autoRefresh, accessTokenAboutToExpire()
*/
#if QT_VERSION < QT_VERSION_CHECK(7, 0, 0)
void QAbstractOAuth2::refreshTokensImplementation()
{
    Q_D(QAbstractOAuth2);
    qCDebug(d->loggingCategory, "%s class does not support refreshing", metaObject()->className());
}
#endif // QT_VERSION < QT_VERSION_CHECK(7, 0, 0)

/*!
    Destroys the QAbstractOAuth2 instance.
*/
QAbstractOAuth2::~QAbstractOAuth2()
{}

/*!
    The returned URL is based on \a url, combining it with the given
    \a parameters and the access token.
*/
QUrl QAbstractOAuth2::createAuthenticatedUrl(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(const QAbstractOAuth2);
    if (Q_UNLIKELY(d->token.isEmpty())) {
        qCWarning(d->loggingCategory, "Empty access token");
        return QUrl();
    }
    QUrl ret = url;
    QUrlQuery query(ret.query());
    query.addQueryItem(QtOAuth2RfcKeywords::accessToken, d->token);
    for (auto it = parameters.begin(), end = parameters.end(); it != end ;++it)
        query.addQueryItem(it.key(), it.value().toString());
    ret.setQuery(query);
    return ret;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    Sends an authenticated HEAD request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.4}
    {Hypertext Transfer Protocol -- HTTP/1.1: HEAD}
*/
QNetworkReply *QAbstractOAuth2::head(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->head(d->createRequest(url, &parameters));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    Sends an authenticated GET request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.3}
    {Hypertext Transfer Protocol -- HTTP/1.1: GET}
*/
QNetworkReply *QAbstractOAuth2::get(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->get(d->createRequest(url, &parameters));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.5}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    const auto data = d->convertParameters(parameters);
    QT_IGNORE_DEPRECATIONS(return post(url, data);)
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    \since 5.10

    \overload

    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a data are used to create
    the request.

    \sa post(), {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, const QByteArray &data)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->post(d->createRequest(url), data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    \since 5.10

    \overload

    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a multiPart are used to create
    the request.

    \sa post(), QHttpMultiPart, {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, QHttpMultiPart *multiPart)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->post(d->createRequest(url), multiPart);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    const auto data = d->convertParameters(parameters);
    QT_IGNORE_DEPRECATIONS(return put(url, data);)
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    \since 5.10

    \overload

    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a data are used to create
    the request.

    \sa put(), {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, const QByteArray &data)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->put(d->createRequest(url), data);
    connect(reply, &QNetworkReply::finished, this, std::bind(&QAbstractOAuth::finished, this, reply));
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    \since 5.10

    \overload

    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a multiPart are used to create
    the request.

    \sa put(), QHttpMultiPart, {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, QHttpMultiPart *multiPart)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->put(d->createRequest(url), multiPart);
    connect(reply, &QNetworkReply::finished, this, std::bind(&QAbstractOAuth::finished, this, reply));
    return reply;
}

/*!
    \deprecated [6.11] Please use QtNetwork classes directly instead, see
    \l {OAuth2 HTTP method alternatives}{HTTP method alternatives}.

    Sends an authenticated DELETE request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.7}
    {Hypertext Transfer Protocol -- HTTP/1.1: DELETE}
*/
QNetworkReply *QAbstractOAuth2::deleteResource(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->deleteResource(
                d->createRequest(url, &parameters));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { emit finished(reply); });
    return reply;
}

#ifndef QOAUTH2_NO_LEGACY_SCOPE
QString QAbstractOAuth2::scope() const
{
    Q_D(const QAbstractOAuth2);
    return d->legacyScope;
}
#endif

QSet<QByteArray> QAbstractOAuth2::grantedScopeTokens() const
{
    Q_D(const QAbstractOAuth2);
    return d->grantedScopeTokens;
}

#ifndef QOAUTH2_NO_LEGACY_SCOPE
void QAbstractOAuth2::setScope(const QString &scope)
{
    Q_D(QAbstractOAuth2);
    d->legacyScopeWasSetByUser = true;
    if (d->legacyScope != scope) {
        d->legacyScope = scope;
        QT_IGNORE_DEPRECATIONS(Q_EMIT scopeChanged(d->legacyScope);)
    }
    const QSet<QByteArray> splitScope = d->splitScope(d->legacyScope);
    if (d->requestedScopeTokens != splitScope) {
        d->requestedScopeTokens = splitScope;
        Q_EMIT requestedScopeTokensChanged(splitScope);
    }
}
#endif

QSet<QByteArray> QAbstractOAuth2::requestedScopeTokens() const
{
    Q_D(const QAbstractOAuth2);
    return d->requestedScopeTokens;
}

void QAbstractOAuth2::setRequestedScopeTokens(const QSet<QByteArray> &tokens)
{
    Q_D(QAbstractOAuth2);
    if (!d->checkRequestedScopeTokensValid(tokens))
        return;
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    d->legacyScopeWasSetByUser = false;
#endif
    if (tokens != d->requestedScopeTokens) {
        d->requestedScopeTokens = tokens;
        Q_EMIT requestedScopeTokensChanged(tokens);
    }
#ifndef QOAUTH2_NO_LEGACY_SCOPE
    QString joinedScope = d->joinedScope(tokens);
    if (joinedScope != d->legacyScope) {
        d->legacyScope = std::move(joinedScope);
        QT_IGNORE_DEPRECATIONS(Q_EMIT scopeChanged(d->legacyScope);)
    }
#endif
}

QString QAbstractOAuth2::userAgent() const
{
    Q_D(const QAbstractOAuth2);
    return d->userAgent;
}

void QAbstractOAuth2::setUserAgent(const QString &userAgent)
{
    Q_D(QAbstractOAuth2);
    if (d->userAgent != userAgent) {
        d->userAgent = userAgent;
        Q_EMIT userAgentChanged(userAgent);
    }
}

/*!
    Returns the \l {https://tools.ietf.org/html/rfc6749#section-3.1.1}
    {response_type} used.
*/
QString QAbstractOAuth2::responseType() const
{
    Q_D(const QAbstractOAuth2);
    return d->responseType;
}

QString QAbstractOAuth2::clientIdentifierSharedKey() const
{
    Q_D(const QAbstractOAuth2);
    return d->clientIdentifierSharedKey;
}

void QAbstractOAuth2::setClientIdentifierSharedKey(const QString &clientIdentifierSharedKey)
{
    Q_D(QAbstractOAuth2);
    if (d->clientIdentifierSharedKey != clientIdentifierSharedKey) {
        d->clientIdentifierSharedKey = clientIdentifierSharedKey;
        Q_EMIT clientIdentifierSharedKeyChanged(clientIdentifierSharedKey);
    }
}

QString QAbstractOAuth2::state() const
{
    Q_D(const QAbstractOAuth2);
    return d->state;
}

void QAbstractOAuth2::setState(const QString &state)
{
    Q_D(QAbstractOAuth2);
    // Allowed characters are defined in
    // https://datatracker.ietf.org/doc/html/rfc6749#appendix-A.5
    // state      = 1*VSCHAR
    // Where
    // VSCHAR     = %x20-7E
    for (QChar c : state) {
        if (c < u'\x20' || c > u'\x7E') {
            qCWarning(lcOAuth2Validation, "setState() contains illegal character(s), ignoring");
            return;
        }
    }
    if (state != d->state) {
        d->state = state;
        Q_EMIT stateChanged(state);
    }
}

QDateTime QAbstractOAuth2::expirationAt() const
{
    Q_D(const QAbstractOAuth2);
    return d->expiresAtUtc.toLocalTime();
}

/*!
    \property QAbstractOAuth2::refreshToken

    This property holds the refresh token used to obtain new access tokens.

    Refresh tokens usually have longer lifespans than access tokens,
    so it makes sense to save them for later use.

    \sa setRefreshToken()
*/
/*!
    \brief Gets the current refresh token.

    Refresh tokens usually have longer lifespans than access tokens,
    so it makes sense to save them for later use.

    Returns the current refresh token or an empty string, if
    there is no refresh token available.
*/
QString QAbstractOAuth2::refreshToken() const
{
    Q_D(const QAbstractOAuth2);
    return  d->refreshToken;
}

/*!
   \brief Sets the new refresh token \a refreshToken to be used.

    A custom refresh token can be used to refresh the access token via this method and then
    the access token can be refreshed via \l refreshTokens().

*/
void QAbstractOAuth2::setRefreshToken(const QString &refreshToken)
{
    Q_D(QAbstractOAuth2);
    if (d->refreshToken != refreshToken) {
        d->refreshToken = refreshToken;
        Q_EMIT refreshTokenChanged(refreshToken);
    }
}

std::chrono::seconds QAbstractOAuth2::refreshLeadTime() const
{
    Q_D(const QAbstractOAuth2);
    return d->refreshLeadTime;
}

void QAbstractOAuth2::setRefreshLeadTime(std::chrono::seconds leadTime)
{
    Q_D(QAbstractOAuth2);
    if (leadTime < 0s) {
        qCWarning(d->loggingCategory, "Invalid refresh leadTime");
        return;
    }
    if (d->refreshLeadTime == leadTime)
        return;
    d->refreshLeadTime = leadTime;
    d->updateRefreshTimer(true);
    Q_EMIT refreshLeadTimeChanged(leadTime);
}

bool QAbstractOAuth2::autoRefresh() const
{
    Q_D(const QAbstractOAuth2);
    return d->autoRefresh;
}

void QAbstractOAuth2::setAutoRefresh(bool enabled)
{
    Q_D(QAbstractOAuth2);
    if (d->autoRefresh == enabled)
        return;
    d->autoRefresh = enabled;
    Q_EMIT autoRefreshChanged(enabled);
}

QAbstractOAuth2::NonceMode QAbstractOAuth2::nonceMode() const
{
    Q_D(const QAbstractOAuth2);
    return d->nonceMode;
}

void QAbstractOAuth2::setNonceMode(NonceMode mode)
{
    Q_D(QAbstractOAuth2);
    if (mode == d->nonceMode)
        return;
    d->nonceMode = mode;
    emit nonceModeChanged(d->nonceMode);
}

QString QAbstractOAuth2::nonce() const
{
    Q_D(const QAbstractOAuth2);
    return d->nonce;
}

void QAbstractOAuth2::setNonce(const QString &nonce)
{
    Q_D(QAbstractOAuth2);
    if (nonce == d->nonce)
        return;
    d->nonce = nonce;
    emit nonceChanged(d->nonce);
}

QString QAbstractOAuth2::idToken() const
{
    Q_D(const QAbstractOAuth2);
    return d->idToken;
}

QUrl QAbstractOAuth2::tokenUrl() const
{
    Q_D(const QAbstractOAuth2);
    return d->tokenUrl;
}

void QAbstractOAuth2::setTokenUrl(const QUrl &tokenUrl)
{
    Q_D(QAbstractOAuth2);
    if (d->tokenUrl == tokenUrl)
        return;

    d->tokenUrl = tokenUrl;
    emit tokenUrlChanged(d->tokenUrl);
}

#ifndef QT_NO_SSL
/*!
    \since 6.5

    Returns the TLS configuration to be used when establishing a mutual TLS
    connection between the client and the Authorization Server.

    \sa setSslConfiguration(), sslConfigurationChanged()
*/
QSslConfiguration QAbstractOAuth2::sslConfiguration() const
{
    Q_D(const QAbstractOAuth2);
    return d->sslConfiguration.value_or(QSslConfiguration());
}

/*!
    \since 6.5

    Sets the TLS \a configuration to be used when establishing
    a mutual TLS connection between the client and the Authorization Server.

    \sa sslConfiguration(), sslConfigurationChanged()
*/
void QAbstractOAuth2::setSslConfiguration(const QSslConfiguration &configuration)
{
    Q_D(QAbstractOAuth2);
    const bool configChanged = !d->sslConfiguration || (*d->sslConfiguration != configuration);
    if (configChanged) {
        d->sslConfiguration = configuration;
        Q_EMIT sslConfigurationChanged(configuration);
    }
}

/*!
    \fn void QAbstractOAuth2::sslConfigurationChanged(const QSslConfiguration &configuration)
    \since 6.5

    The signal is emitted when the TLS configuration has changed.
    The \a configuration parameter contains the new TLS configuration.

    \sa sslConfiguration(), setSslConfiguration()
*/
#endif // !QT_NO_SSL

QT_END_NAMESPACE

#include "moc_qabstractoauth2.cpp"
