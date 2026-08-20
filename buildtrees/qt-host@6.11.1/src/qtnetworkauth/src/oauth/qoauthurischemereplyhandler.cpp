// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include "qabstractoauthreplyhandler_p.h" // for lcReplyHandler()
#include "qoauthurischemereplyhandler.h"
#include "qoauthurischemereplyhandler_p.h"

#include <QtGui/qdesktopservices.h>

#include <private/qobject_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qurlquery.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QOAuthUriSchemeReplyHandler
    \inmodule QtNetworkAuth
    \ingroup oauth
    \since 6.8

    \brief Handles private/custom and https URI scheme redirects.

    This class serves as a reply handler for
    \l {https://datatracker.ietf.org/doc/html/rfc6749}{OAuth 2.0} authorization
    processes that use private/custom or HTTPS URI schemes for redirection.
    It manages the reception of the authorization redirection (also known as the
    callback) and the subsequent acquisition of access tokens.

    The \l {https://datatracker.ietf.org/doc/html/rfc6749#section-3.1.2}
    {redirection URI} is where the authorization server redirects the
    user-agent (typically, and preferably, the system browser) once
    the authorization part of the flow is complete.

    The use of specific URI schemes requires configuration at the
    operating system level to associate the URI with
    the correct application. The way to set up this association varies
    between operating systems. See \l {Platform Support and Dependencies}.

    This class complements QOAuthHttpServerReplyHandler,
    which handles \c http schemes by setting up a localhost server.

    The following code illustrates the usage. First, the needed variables:

    \snippet src_oauth_replyhandlers_p.h uri-variables

    Followed up by the OAuth setup (error handling omitted for brevity):

    \snippet src_oauth_replyhandlers.cpp uri-service-configuration
    \codeline
    \snippet src_oauth_replyhandlers.cpp uri-oauth-setup

    Finally, we then set up the URI scheme reply-handler:

    \snippet src_oauth_replyhandlers.cpp uri-handler-setup

    \section1 Private/Custom URI Schemes

    Custom URI schemes typically use reverse-domain notation followed
    by a path, or occasionally a host/host+path:
    \badcode
    // Example with path:
    com.example.myapp:/oauth2/callback
    // Example with host:
    com.example.myapp://oauth2.callback
    \endcode

    \section1 HTTPS URI Scheme

    With HTTPS URI schemes, the redirect URLs are regular https links:
    \badcode
    https://myapp.example.com/oauth2/callback
    \endcode

    These links are called
    \l {https://developer.apple.com/ios/universal-links/}{Universal Links}
    on iOS and
    \l {https://developer.android.com/training/app-links}{App Links on Android}.

    The use of https schemes is recommended as it provides additional security
    by forcing application developers to prove ownership of the URLs used. This
    proving is done by hosting an association file, which the operating system
    will consult as part of its internal URL dispatching.

    The content of this file associates the application and the used URLs.
    The association files must be publicly accessible without any HTTP
    redirects. In addition, the hosting site must have valid certificates
    and, at least with Android, the file must be served as
    \c application/json content-type (refer to your server's configuration
    guide).

    In addition, https links can provide some usability benefits:
    \list
        \li The https URL doubles as a regular https link. If the
            user hasn't installed the application (since the URL wasn't handled
            by any application), the https link may for example serve
            instructions to do so.
        \li The application selection dialogue to open the URL may be avoided,
            and instead your application may be opened automatically
    \endlist

    The tradeoff is that this requires extra setup as you need to set up this
    publicly-hosted association file.

    \section1 Platform Support and Dependencies

    Currently supported platforms are Android, iOS, and macOS.

    URI scheme listening is based on QDesktopServices::setUrlHandler()
    and QDesktopServices::unsetUrlHandler(). These are currently
    provided by Qt::Gui module and therefore QtNetworkAuth module
    depends on Qt::Gui. If QtNetworkAuth is built without Qt::Gui,
    QOAuthUriSchemeReplyHandler will not be included.

    \section2 Android

    On \l {Qt for Android}{Android} the URI schemes require:
    \list
        \li Setting up
            \l {configuring qdesktopservices url handler on android}{intent-filters}
            in the application manifest
        \li Optionally, for automatic verification with https schemes,
            hosting a site association file
            \l {configuring qdesktopservices url handler on android}{assetlinks.json}
    \endlist

    See also the
    \l {https://doc.qt.io/qt-6/android-manifest-file-configuration.html}
    {Qt Android Manifest File Configuration}.

    \section2 iOS and macOS

    On \l {Qt for iOS}{iOS} and \l {Qt for macOS}{macOS} the URI schemes require:
    \list
        \li Setting up site association
            \l {configuring qdesktopservices url handler on ios and macos}{entitlement}
        \li With https schemes, hosting a
            \l {configuring qdesktopservices url handler on ios and macos}{site association file}
            (\c apple-app-site-association)
    \endlist

    \section2 \l {Qt for Windows}{Windows}, \l {Qt for Linux}{Linux}

    Currently not supported. However platforms and use cases supporting
    \l {Qt WebEngine Platform Notes}{Qt WebEngine} can still use this reply
    handler - see \l {Qt OAuth2 Browser Support} for details.
*/

bool QOAuthUriSchemeReplyHandlerPrivate::hasValidRedirectUrl() const
{
    // RFC 6749 Section 3.1.2
    return redirectUrl.isValid()
           && !redirectUrl.scheme().isEmpty()
           && redirectUrl.fragment().isEmpty();
}

bool QOAuthUriSchemeReplyHandlerPrivate::_q_handleRedirectUrl(const QUrl &url)
{
    Q_Q(QOAuthUriSchemeReplyHandler);
    // Remove the query parameters from comparison, and compare them manually (the parameters
    // of interest like 'code' and 'state' are received as query parameters and comparison
    // would always fail). Fragments are removed as some servers (eg. Reddit) seem to add some,
    // possibly for some implementation consistency with other OAuth flows where fragments
    // are actually used. Some servers (eg. Microsoft) also add an extra '/' path, so treat
    // '/' and empty paths as equal
    auto normalized = [](const QUrl &url) {
        auto normalized = url.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment);
        if (normalized.path().isEmpty())
            normalized.setPath("/"_L1);
        return normalized;
    };

    bool urlMatch = normalized(url).matches(normalized(redirectUrl), QUrl::None);

    const QUrlQuery responseQuery{url};
    if (urlMatch) {
        // Verify that query parameters that are part of redirect URL are present in redirection
        const auto registeredItems = QUrlQuery{redirectUrl}.queryItems();
        for (const auto &item: registeredItems) {
            if (!responseQuery.hasQueryItem(item.first)
                || responseQuery.queryItemValue(item.first) != item.second) {
                urlMatch = false;
                break;
            }
        }
    }

    if (!urlMatch) {
        qCDebug(lcReplyHandler(), "Url ignored");
        if (forwardUnhandledUrls) {
            // The URLs received here might be unrelated. Further, in case of "https" scheme,
            // the first request issued to the authorization server comes through here
            // (if this handler is listening)
            QDesktopServices::openUrl(url);
        }
        return false;
    }

    qCDebug(lcReplyHandler(), "Url handled");
    emit q->callbackDataReceived(url.toEncoded());

    QVariantMap resultParameters;
    const auto responseItems = responseQuery.queryItems(QUrl::FullyDecoded);
    for (const auto &item : responseItems)
        resultParameters.insert(item.first, item.second);

    emit q->callbackReceived(resultParameters);
    return true;
}

/*!
    \fn QOAuthUriSchemeReplyHandler::QOAuthUriSchemeReplyHandler()

    Constructs a QOAuthUriSchemeReplyHandler object with empty callback()/
    redirectUrl() and no parent. The constructed object does not automatically
    listen.
*/

/*!
    Constructs a QOAuthUriSchemeReplyHandler object with \a parent and empty
    callback()/redirectUrl(). The constructed object does not automatically listen.
*/
QOAuthUriSchemeReplyHandler::QOAuthUriSchemeReplyHandler(QObject *parent) :
    QOAuthOobReplyHandler(*new QOAuthUriSchemeReplyHandlerPrivate(), parent)
{
}

/*!
    Constructs a QOAuthUriSchemeReplyHandler object and sets \a parent as the
    parent object and \a redirectUrl as the redirect URL. The constructed
    object attempts automatically to listen.

    \sa redirectUrl(), setRedirectUrl(), listen(), isListening()
*/
QOAuthUriSchemeReplyHandler::QOAuthUriSchemeReplyHandler(const QUrl &redirectUrl, QObject *parent)
    : QOAuthUriSchemeReplyHandler(parent)
{
    Q_D(QOAuthUriSchemeReplyHandler);
    d->redirectUrl = redirectUrl;
    listen();
}

/*!
    Destroys the QOAuthUriSchemeReplyHandler object. Closes
    this handler.

    \sa close()
*/
QOAuthUriSchemeReplyHandler::~QOAuthUriSchemeReplyHandler()
{
    close();
}

QString QOAuthUriSchemeReplyHandler::callback() const
{
    Q_D(const QOAuthUriSchemeReplyHandler);
    return d->redirectUrl.toString();
}

/*!
    \property QOAuthUriSchemeReplyHandler::redirectUrl
    \brief The URL used to receive authorization redirection/response.

    This property is used as the
    \l{https://datatracker.ietf.org/doc/html/rfc6749#section-3.1.2}
    {OAuth2 redirect_uri parameter}, which is sent as part of the
    authorization request. The \c redirect_uri is acquired by
    calling QUrl::toString() with default options.

    The URL must match the one registered at the authorization server,
    as the authorization servers likely reject any mismatching redirect_uris.

    Similarly, when this handler receives the redirection,
    the redirection URL must match the URL set here. The handler
    compares the scheme, host, port, path, and any
    query items that were part of the URL set by this method.

    The URL is handled only if all of these match. The comparison of query
    parameters excludes any additional query parameters that may have been set
    at server-side, as these contain the actual data of interest.
*/
void QOAuthUriSchemeReplyHandler::setRedirectUrl(const QUrl &url)
{
    Q_D(QOAuthUriSchemeReplyHandler);
    if (url == d->redirectUrl)
        return;

    if (d->listening) {
        close(); // close previous url listening first
        d->redirectUrl = url;
        listen();
    } else {
        d->redirectUrl = url;
    }
    emit redirectUrlChanged();
}

QUrl QOAuthUriSchemeReplyHandler::redirectUrl() const
{
    Q_D(const QOAuthUriSchemeReplyHandler);
    return d->redirectUrl;
}

/*!
    \since 6.9
    This function is used to supply the redirect URL the Authorization
    Server provides at the end of the authorization stage. The provided
    \a url undergoes the same URL matching as described in \l {redirectUrl}.

    Suppyling the URL can be useful for scenarios where this redirect URL
    is captured by some other means, for example with \l {Qt WebEngine} or
    through some other custom arrangement. This way such agent usage can be
    integrated with rest of the OAuth2 flow.

    This handler does not need to be listening, and therefore it is
    recommended to \l close() the handler to avoid unnecessary listening.

    Returns \c true if the URL matched and was handled, \c false otherwise.

    See also \l {Redirect URI Schemes}{Redirect URI Schemes with Qt WebEngine}.
*/
bool QOAuthUriSchemeReplyHandler::handleAuthorizationRedirect(const QUrl &url)
{
    Q_D(QOAuthUriSchemeReplyHandler);
    d->forwardUnhandledUrls = false;
    const bool handled = d->_q_handleRedirectUrl(url);
    d->forwardUnhandledUrls = true;
    return handled;
}

/*!
    Tells this handler to listen for incoming URLs. Returns
    \c true if listening is successful, and \c false otherwise.

    The handler will match URLs to redirectUrl().
    If the received URL does not match, it will be forwarded to
    QDesktopServices::openURL().

    Active listening is only required when performing the initial
    authorization phase, typically initiated by a
    QOAuth2AuthorizationCodeFlow::grant() call.

    It is recommended to close the listener after successful authorization.
    Listening is not needed for
    \l {QOAuth2AuthorizationCodeFlow::requestAccessToken()}{acquiring access tokens}.
*/
bool QOAuthUriSchemeReplyHandler::listen()
{
    Q_D(QOAuthUriSchemeReplyHandler);
    if (d->listening)
        return true;

    if (!d->hasValidRedirectUrl()) {
        qCWarning(lcReplyHandler(), "listen(): callback url not valid");
        return false;
    }
    qCDebug(lcReplyHandler(), "listen() URL listener");
    QDesktopServices::setUrlHandler(d->redirectUrl.scheme(), this, "_q_handleRedirectUrl");

    d->listening = true;
    return true;
}

/*!
    Tells this handler to stop listening for incoming URLs.

    \sa listen(), isListening()
*/
void QOAuthUriSchemeReplyHandler::close()
{
    Q_D(QOAuthUriSchemeReplyHandler);
    if (!d->listening)
        return;

    qCDebug(lcReplyHandler(), "close() URL listener");
    QDesktopServices::unsetUrlHandler(d->redirectUrl.scheme());
    d->listening = false;
}

/*!
    Returns \c true if this handler is currently listening,
    and \c false otherwise.

    \sa listen(), close()
*/
bool QOAuthUriSchemeReplyHandler::isListening() const noexcept
{
    Q_D(const QOAuthUriSchemeReplyHandler);
    return d->listening;
}

QT_END_NAMESPACE

#include "moc_qoauthurischemereplyhandler.cpp"
