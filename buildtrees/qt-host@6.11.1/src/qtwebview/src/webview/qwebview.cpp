// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebview.h"

#include "qwebviewsettings.h"
#include "qwebviewplugin_p.h"
#include "qwebviewloadinginfo.h"
#include "qwebviewfactory_p.h"

QT_BEGIN_NAMESPACE

QWebViewSettingsPrivate::~QWebViewSettingsPrivate()
    = default;

/*!
    \class QWebView
    \brief The QWebView class provides a window that is used to view web content.
    \since 6.11
    \ingroup webview
    \inmodule QtWebView

    A \e {web view} is a window for displaying web content that is implemented
    using native APIs on the platforms where they are available. Therefore, it does not
    necessarily require including a full web browser stack as part of the application.

    A \e {web site} can be loaded to a web view with the setUrl() function.

    Alternatively, if you have the HTML content readily available, you can use setHtml()
    instead.

    The loadingChanged() signal is emitted when the view begins loading and when the
    view has been loaded completely.
    Its argument QWebViewLoadRequest object indicates whether loading was
    successful or failed.

    The title of an HTML document can be accessed with the title() property.
*/

/*!
    \fn void QWebView::loadingChanged(const QWebViewLoadingInfo &loadingInfo)

    This signal is emitted when the state of loading the web content changes.
    By handling this signal it's possible, for example, to react to page load
    errors.

    The \a loadingInfo parameter holds the \e url and \e status of the request,
    as well as an \e errorString containing an error message for a failed
    request.

    \sa loadProgressChanged, QWebViewLoadingInfo
*/

/*!
    \fn void QWebView::loadProgressChanged(int loadProgress)

    This signal is continuously emitted during the loading of a web page.
    The \a loadProgress parameter is a value between 0 and 100, indicating
    what percentage of the web page has been loaded. The intended use
    for this is to display a progress bar to the user.

    \note Some backends do not support fractional load progress changes,
    and will only emit this signal at the start and end of a load, with
    values of 0 and 100, respecively.

    \sa loadingChanged
*/

QWebView::QWebView(QWebViewFactory::Hint hint) : d(QWebViewFactory::createWebView(this, hint))
{
    //note this is called only from qquickwebview
    Q_ASSERT(d);
    d->m_settings.reset(d->settings());
    qRegisterMetaType<QWebViewLoadingInfo>();
}

QWebView::QWebView(QWindow *parent) : QWindow(parent), d(QWebViewFactory::createWebView(this))
{
    Q_ASSERT(d);
    d->m_settings.reset(d->settings());
    qRegisterMetaType<QWebViewLoadingInfo>();
    d->initialize(nullptr);
}

QWebView::QWebView(QScreen *screen) : QWindow(screen), d(QWebViewFactory::createWebView(this))
{
    Q_ASSERT(d);
    d->m_settings.reset(d->settings());
    qRegisterMetaType<QWebViewLoadingInfo>();
    d->initialize(nullptr);
}

QWebView::~QWebView() { }

/*!
    \property QWebView::httpUserAgentString
    \brief The user agent in use.
    The user-agent string sent with HTTP to identify the browser.
*/

/*!
    \fn void QWebView::httpUserAgentStringChanged(const QString &userAgent)
    This signal is emitted whenever the \a userAgent of the view changes.

    \sa httpUserAgent()
*/

QString QWebView::httpUserAgentString() const
{
    return d->httpUserAgent();
}

void QWebView::setHttpUserAgentString(const QString &userAgent)
{
    return d->setHttpUserAgent(userAgent);
}

/*!
    \property QWebView::url
    \brief The URL of the web page currently viewed.

    Setting this property clears the view and loads the URL.
    By default, this property contains an empty, invalid URL.
    URLs that originate from user input should be parsed with QUrl::fromUserInput().
    \note QWebView does not support loading content through the Qt Resource system.

    \sa loadingChanged()
*/

/*!
    \fn void QWebView::urlChanged(const QUrl &url)
    This signal is emitted whenever the \a url of the view changes.

    \sa url()
*/

QUrl QWebView::url() const
{
    return d->url();
}

void QWebView::setUrl(const QUrl &url)
{
    d->setUrl(url);
}

/*!
    Returns \c true if it's currently possible to navigate back in the web history.
*/

bool QWebView::canGoBack() const
{
    return d->canGoBack();
}

/*!
    Navigates back in the web history.
*/

void QWebView::goBack()
{
    d->goBack();
}

/*!
    Returns \c true if it's currently possible to navigate forward in the web history.
*/

bool QWebView::canGoForward() const
{
    return d->canGoForward();
}

/*!
    Navigates forward in the web history.
*/
void QWebView::goForward()
{
    d->goForward();
}

/*!
    Reloads the current \l url.
*/
void QWebView::reload()
{
    d->reload();
}

/*!
    Stops loading the current \l url.
*/
void QWebView::stop()
{
    d->stop();
}

/*!
    \property QWebView::title
    \brief The title of the currently loaded web page.

    The title of the page as defined by the HTML \c <title> element.

    \sa titleChanged()
*/

/*!
    \fn void QWebView::titleChanged(const QString &title)
    This signal is emitted whenever the \a title of the view changes.

    \sa title()
*/

QString QWebView::title() const
{
    return d->title();
}

/*!
    \property QWebView::loadProgress
    The current load progress of the web content, represented as
    an integer between 0 and 100.
*/

int QWebView::loadProgress() const
{
    return d->loadProgress();
}

/*!
    \property QWebView::loading
    Holds \c true if the WebView is currently in the process of loading
    new content, \c false otherwise.
*/

bool QWebView::isLoading() const
{
    return d->isLoading();
}

/*!
    \property QWebView::settings
    \brief Settings object for the QWebView.
    The settings for a web view.
    \sa WebViewSettings
*/

QWebViewSettings *QWebView::settings() const
{
    return d->m_settings.get();
}

/*!
    \fn void QWebView::loadHtml(const QString &html, const QUrl &baseUrl)
    Loads the specified \a html content to the web view.

    This method offers a lower-level alternative to the \l url property,
    which references HTML pages via URL.

    External objects such as stylesheets or images referenced in the HTML
    document should be located relative to \a baseUrl. For example, if \a html
    is retrieved from \c http://www.example.com/documents/overview.html, which
    is the base URL, then an image referenced with the relative URL \c diagram.png,
    should be at \c{http://www.example.com/documents/diagram.png}.

    \note The QWebView does not support loading content through the Qt Resource system.
*/

void QWebView::loadHtml(const QString &html, const QUrl &baseUrl)
{
    d->loadHtml(html, baseUrl);
}

/*!
    \fn void runJavaScript(const QString &script,
                           const std::function<void(const QVariant &)> &resultCallback)
    Runs the specified JavaScript.
    In case a \a resultCallback function is provided, it will be invoked after the \a script
    finishes running.

    \code
    view.runJavaScript("document.title", [](const QVariant &v) { qDebug() << v.toString(); });
    \endcode
*/

void QWebView::runJavaScript(const QString &script,
                             const std::function<void(const QVariant &)> &resultCallback)
{
    return d->runJavaScript(script, resultCallback);
}

/*!
    \fn void cookieAdded(const QString &domain, const QString &name)

    This signal is emitted when a cookie is added.

    The parameters provide information about the \a domain and the \a name of the added cookie.

    \note When the Qt WebEngine module is used as a backend, the cookieAdded signal will
    be emitted for any cookie added to the underlying QWebEngineCookieStore, including
    those added by websites. In other cases cookieAdded signal is only emitted for cookies
    explicitly added with \l setCookie().
*/
/*!
    \fn void setCookie(const QString &domain, const QString &name, const QString &value)
    Adds a cookie with the specified \a domain, \a name and \a value.
    The cookieAdded() signal will be emitted when the cookie is added.
*/
void QWebView::setCookie(const QString &domain, const QString &name, const QString &value)
{
    d->setCookie(domain, name, value);
}

/*!
    \fn void cookieRemoved(const QString &domain, const QString &name)
    This signal is emitted when a cookie is deleted.
    The parameters provide information about the \a domain and the \a name of the deleted cookie.
*/
/*!
    \fn void deleteCookie(const QString &domain, const QString &name)
    Deletes a cookie with the specified \a domain and \a name.

    The cookieRemoved() signal will be emitted when the cookie is deleted.
*/

void QWebView::deleteCookie(const QString &domain, const QString &name)
{
    d->deleteCookie(domain, name);
}

/*!
    Deletes all the cookies.
*/

void QWebView::deleteAllCookies()
{
    d->deleteAllCookies();
}

/*! \reimp */
bool QWebView::event(QEvent *e)
{
    return QWindow::event(e);
}

QT_END_NAMESPACE

#include "moc_qwebview.cpp"
