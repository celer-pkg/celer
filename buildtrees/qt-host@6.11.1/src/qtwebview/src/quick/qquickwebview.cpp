// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickwebview_p.h"
#include "qquickwebviewloadrequest_p.h"
#include "qquickwebviewsettings_p.h"
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWebView/private/qwebviewfactory_p.h>
#include <QtWebView/private/qwebview_p.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qmutex.h>

namespace {

class CallbackStorage
{
public:
    int insertCallback(const QJSValue &callback)
    {
        QMutexLocker locker(&m_mtx);
        const int nextId = qMax(++m_counter, 0);
        if (nextId == 0)
            m_counter = 1;

        m_callbacks.insert(nextId, callback);
        return nextId;
    }

    QJSValue takeCallback(int callbackId)
    {
        QMutexLocker lock(&m_mtx);
        return m_callbacks.take(callbackId);
    }

private:
    QMutex m_mtx;
    int m_counter;
    QHash<int, QJSValue> m_callbacks;
};

} // namespace

Q_GLOBAL_STATIC(CallbackStorage, callbacks)

/*!
    \qmltype WebView
    \inqmlmodule QtWebView
    \ingroup qtwebview
    \brief A component for displaying web content.

    WebView is a component for displaying web content which is implemented using native
    APIs on the platforms where this is available, thus it does not necessarily require
    including a full web browser stack as part of the application.

    To make the Qt WebView module function correctly across all platforms, it is necessary
    to call \l {qtwebview-initialize}{QtWebView::initialize}() right after creating the
    QGuiApplication instance.

    \note Due to platform limitations overlapping the WebView and other QML components
    is not supported.
*/

QQuickWebView::QQuickWebView(QQuickItem *parent)
    : QQuickWindowContainer(parent),
      m_webView(new QWebView(QWebViewFactory::Hint::QuickInitialization)),
      m_settings(new QQuickWebViewSettings(m_webView->settings()))
{
    if (QWindow *nativeWindow = m_webView->d->nativeWindow())
        setContainedWindow(nativeWindow);

    connect(m_webView, &QWebView::titleChanged, this, &QQuickWebView::titleChanged);
    connect(m_webView, &QWebView::urlChanged, this, &QQuickWebView::urlChanged);
    connect(m_webView, &QWebView::loadProgressChanged, this, &QQuickWebView::loadProgressChanged);
    connect(m_webView, &QWebView::loadingChanged, this, &QQuickWebView::onLoadingChanged);
    connect(m_webView, &QWebView::httpUserAgentStringChanged,
            this, &QQuickWebView::httpUserAgentChanged);
    connect(m_webView, &QWebView::cookieAdded, this, &QQuickWebView::cookieAdded);
    connect(m_webView, &QWebView::cookieRemoved, this, &QQuickWebView::cookieRemoved);

    m_webView->QObject::setParent(this);
    m_settings->QObject::setParent(this);
}

QQuickWebView::~QQuickWebView() { }

/*!
  \qmlproperty url QtWebView::WebView::httpUserAgent
  \since QtWebView 1.14
  The user agent in use.
*/
/*!
  \qmlsignal QtWebView::WebView::httpUserAgentChanged(string userAgent)
  This signal is emitted whenever the \a userAgent of the view changes.

  \sa httpUserAgent
*/

void QQuickWebView::setHttpUserAgent(const QString &userAgent)
{
    m_webView->setHttpUserAgentString(userAgent);
}

QString QQuickWebView::httpUserAgent() const
{
    return m_webView->httpUserAgentString();
}

/*!
  \qmlproperty url QtWebView::WebView::url

  The URL of currently loaded web page. Changing this will trigger
  loading new content.

  The URL is used as-is. URLs that originate from user input should
  be parsed with QUrl::fromUserInput().

  \note The WebView does not support loading content through the Qt Resource system.
*/
/*!
    \qmlsignal QtWebView::WebView::urlChanged(url url)
    This signal is emitted whenever the \a url of the view changes.

    \sa url
*/

void QQuickWebView::setUrl(const QUrl &url)
{
    m_webView->setUrl(url);
}

/*!
  \qmlproperty string QtWebView::WebView::title
  \readonly

  The title of the currently loaded web page.
*/
/*!
  \qmlsignal QtWebView::WebView::titleChanged(string title)
  This signal is emitted whenever the \a title of the view changes.

  \sa title
*/
QString QQuickWebView::title() const
{
    return m_webView->title();
}

QUrl QQuickWebView::url() const
{
    return m_webView->url();
}

/*!
  \qmlproperty bool QtWebView::WebView::canGoBack
  \readonly

  Holds \c true if it's currently possible to navigate back in the web history.
*/

bool QQuickWebView::canGoBack() const
{
    return m_webView->canGoBack();
}

/*!
  \qmlproperty bool QtWebView::WebView::canGoForward
  \readonly

  Holds \c true if it's currently possible to navigate forward in the web history.
*/

bool QQuickWebView::canGoForward() const
{
    return m_webView->canGoForward();
}

/*!
  \qmlproperty int QtWebView::WebView::loadProgress
  \readonly

  The current load progress of the web content, represented as
  an integer between 0 and 100.
*/
/*!
  \qmlsignal QtWebView::WebView::loadProgressChanged(int loadProgress)

  This signal is continuously emitted during the loading of a web page.
  The \a loadProgress parameter is a value between 0 and 100, indicating
  what percentage of the web page has been loaded. The intended use
  for this is to display a progress bar to the user.

  \note Some backends do not support fractional load progress changes,
  and will only emit this signal at the start and end of a load, with
  values of 0 and 100, respecively.

  \sa loadingChanged
*/
int QQuickWebView::loadProgress() const
{
    return m_webView->loadProgress();
}

/*!
  \qmlproperty bool QtWebView::WebView::loading
  \readonly

  Holds \c true if the WebView is currently in the process of loading
  new content, \c false otherwise.

  \sa loadingChanged()
*/

/*!
  \qmlsignal QtWebView::WebView::loadingChanged(WebViewLoadRequest loadRequest)

  This signal is emitted when the state of loading the web content changes.
  By handling this signal it's possible, for example, to react to page load
  errors.

  The \a loadRequest parameter holds the \e url and \e status of the request,
  as well as an \e errorString containing an error message for a failed
  request.

  \sa WebViewLoadRequest
*/
bool QQuickWebView::isLoading() const
{
    return m_webView->isLoading();
}

/*!
    \qmlmethod void QtWebView::WebView::goBack()

    Navigates back in the web history.
*/
void QQuickWebView::goBack()
{
    m_webView->goBack();
}

/*!
    \qmlmethod void QtWebView::WebView::goForward()

    Navigates forward in the web history.
*/
void QQuickWebView::goForward()
{
    m_webView->goForward();
}

/*!
    \qmlmethod void QtWebView::WebView::reload()

    Reloads the current \l url.
*/
void QQuickWebView::reload()
{
    m_webView->reload();
}

/*!
    \qmlmethod void QtWebView::WebView::stop()

    Stops loading the current \l url.
*/
void QQuickWebView::stop()
{
    m_webView->stop();
}

/*!
    \qmlmethod void QtWebView::WebView::loadHtml(string html, url baseUrl)

    Loads the specified \a html content to the web view.

    This method offers a lower-level alternative to the \l url property,
    which references HTML pages via URL.

    External objects such as stylesheets or images referenced in the HTML
    document should be located relative to \a baseUrl. For example, if \a html
    is retrieved from \c http://www.example.com/documents/overview.html, which
    is the base URL, then an image referenced with the relative url, \c diagram.png,
    should be at \c{http://www.example.com/documents/diagram.png}.

    \note The WebView does not support loading content through the Qt Resource system.

    \sa url
*/
void QQuickWebView::loadHtml(const QString &html, const QUrl &baseUrl)
{
    m_webView->loadHtml(html, baseUrl);
}

/*!
    \qmlmethod void QtWebView::WebView::runJavaScript(string script, variant callback)

    Runs the specified JavaScript.
    In case a \a callback function is provided, it will be invoked after the \a script
    finished running.

    \badcode
    runJavaScript("document.title", function(result) { console.log(result); });
    \endcode
*/
void QQuickWebView::runJavaScript(const QString &script, const QJSValue &callback)
{
    if (callback.isCallable()) {
        int callbackId = callbacks->insertCallback(callback);
        QPointer<QQmlEngine> weakEngine = qmlEngine(this);
        m_webView->runJavaScript(script, [callbackId, weakEngine](const QVariant &result) {
            QJSValue callback = callbacks->takeCallback(callbackId);
            if (!weakEngine) {
                qWarning("No JavaScript engine, unable to handle JavaScript callback!");
                return;
            }

            Q_ASSERT(callback.isCallable());
            QJSValueList args;
            args.append(weakEngine->toScriptValue(result));
            callback.call(args);
        });
    } else {
        m_webView->runJavaScript(script);
    }
}

/*!
    \qmlmethod void QtWebView::WebView::setCookie(string domain, string name, string value)
    \since QtWebView 6.3
    Adds a cookie with the specified \a domain, \a name and \a value.

    The \l cookieAdded signal will be emitted when the cookie is added.
*/
/*!
    \qmlsignal QtWebView::WebView::cookieAdded(string domain, string name)

    This signal is emitted when a cookie is added.

    The parameters provide information about the \a domain and the \a name of the added cookie.

    \note When Qt WebEngine module is used as backend, cookieAdded signal will be emitted for any
    cookie added to the underlying QWebEngineCookieStore, including those added by websites.
    In other cases cookieAdded signal is only emitted for cookies explicitly added with \l setCookie().
*/
void QQuickWebView::setCookie(const QString &domain, const QString &name, const QString &value)
{
    m_webView->setCookie(domain, name, value);
}

/*!
    \qmlmethod void QtWebView::WebView::deleteCookie(string domain, string name)
    \since QtWebView 6.3
    Deletes a cookie with the specified \a domain and \a name.

    The \l cookieRemoved signal will be emitted when the cookie is deleted.
*/
/*!
    \qmlsignal QtWebView::WebView::cookieRemoved(string domain, string name)

    This signal is emitted when a cookie is deleted.

    The parameters provide information about the \a domain and the \a name of the deleted cookie.
*/
void QQuickWebView::deleteCookie(const QString &domain, const QString &name)
{
    m_webView->deleteCookie(domain, name);
}

/*!
    \qmlmethod void QtWebView::WebView::deleteAllCookies()
    \since QtWebView 6.3
    Deletes all the cookies.
*/
void QQuickWebView::deleteAllCookies()
{
    m_webView->deleteAllCookies();
}

void QQuickWebView::onLoadingChanged(const QWebViewLoadingInfo &loadRequest)
{
    QQuickWebViewLoadRequest qqLoadRequest(loadRequest);
    Q_EMIT loadingChanged(&qqLoadRequest);
}

/*!
    \qmlproperty WebViewSettings WebView::settings
    \readonly
    \since QtWebView 6.5

    Settings object for the WebView.

    \sa WebViewSettings
*/

QQuickWebViewSettings *QQuickWebView::settings() const
{
    return m_settings;
}

void QQuickWebView::classBegin()
{
    m_webView->d->initialize(this);
    QQuickWindowContainer::classBegin();
}
