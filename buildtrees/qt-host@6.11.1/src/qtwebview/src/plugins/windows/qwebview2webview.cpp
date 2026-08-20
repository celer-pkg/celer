// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebview2webview_p.h"
#include <QtWebView/qwebviewloadinginfo.h>
#include <QtWebView/private/qwebviewfactory_p.h>
#include <QtCore/private/qfunctions_win_p.h>
#include <QtWidgets/QtWidgets>

QString WebErrorStatusToString(COREWEBVIEW2_WEB_ERROR_STATUS status)
{
    switch (status)
    {
#define STATUS_ENTRY(statusValue)                                                              \
    case statusValue:                                                                          \
        return QString(L#statusValue);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_COMMON_NAME_IS_INCORRECT);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_EXPIRED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CLIENT_CERTIFICATE_CONTAINS_ERRORS);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_REVOKED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_IS_INVALID);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_SERVER_UNREACHABLE);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_TIMEOUT);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_ERROR_HTTP_INVALID_SERVER_RESPONSE);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_RESET);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_CANNOT_CONNECT);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_HOST_NAME_NOT_RESOLVED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_REDIRECT_FAILED);
        STATUS_ENTRY(COREWEBVIEW2_WEB_ERROR_STATUS_UNEXPECTED_ERROR);

#undef STATUS_ENTRY
    case COREWEBVIEW2_WEB_ERROR_STATUS_VALID_AUTHENTICATION_CREDENTIALS_REQUIRED:
    case COREWEBVIEW2_WEB_ERROR_STATUS_VALID_PROXY_AUTHENTICATION_REQUIRED:
        break;
    }

    return QString(L"ERROR");
}

QWebview2WebViewSettingsPrivate::QWebview2WebViewSettingsPrivate()
{
}

void QWebview2WebViewSettingsPrivate::init(ICoreWebView2Controller* viewController)
{
    if (viewController != nullptr) {
        m_webviewController = viewController;
        HRESULT hr = m_webviewController->get_CoreWebView2(&m_webview);
        Q_ASSERT_SUCCEEDED(hr);
    }
}

bool QWebview2WebViewSettingsPrivate::doTestAttribute(WebAttribute attribute) const
{
    switch (attribute) {
    case QWebViewSettings::WebAttribute::LocalStorageEnabled:
        return localStorageEnabled();
    case QWebViewSettings::WebAttribute::JavaScriptEnabled:
        return javaScriptEnabled();
    case QWebViewSettings::WebAttribute::AllowFileAccess:
        return allowFileAccess();
    case QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls:
        return localContentCanAccessFileUrls();
    }
    return false;
}

void QWebview2WebViewSettingsPrivate::doSetAttribute(WebAttribute attribute, bool value)
{
    switch (attribute) {
    case QWebViewSettings::WebAttribute::LocalStorageEnabled:
        setLocalStorageEnabled(value);
        break;
    case QWebViewSettings::WebAttribute::JavaScriptEnabled:
        setJavaScriptEnabled(value);
        break;
    case QWebViewSettings::WebAttribute::AllowFileAccess:
        setAllowFileAccess(value);
        break;
    case QWebViewSettings::WebAttribute::LocalContentCanAccessFileUrls:
        setLocalContentCanAccessFileUrls(value);
        break;
    }
}

bool QWebview2WebViewSettingsPrivate::localStorageEnabled() const
{
    return true;
}

bool QWebview2WebViewSettingsPrivate::javaScriptEnabled() const
{
    return m_javaScriptEnabled;
}

bool QWebview2WebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    return m_allowFileAccess;
}

bool QWebview2WebViewSettingsPrivate::allowFileAccess() const
{
    return m_allowFileAccess;
}

void QWebview2WebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setLocalContentCanAccessFileUrls() not supported on this platform");
}

void QWebview2WebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
    m_javaScriptEnabled = enabled;
    if (!m_webview)
        return;

    ComPtr<ICoreWebView2Settings> settings;
    HRESULT hr = m_webview->get_Settings(&settings);
    Q_ASSERT_SUCCEEDED(hr);
    hr = settings->put_IsScriptEnabled(enabled);
    Q_ASSERT_SUCCEEDED(hr);
}

void QWebview2WebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    Q_UNUSED(enabled);
    qWarning("setLocalStorageEnabled() not supported on this platform");
}

void QWebview2WebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    m_allowFileAccess = enabled;
}

QWebView2WebViewPrivate::QWebView2WebViewPrivate(QWebView *view)
    : QWebViewPrivate(view),
      m_settings(new QWebview2WebViewSettingsPrivate()),
      m_window(view),
      m_isLoading(false),
      m_progress(0)
{
    // Create a QWindow without a parent
    // This window is used for initializing the WebView2

    m_window->setFlag(Qt::Tool);
    m_window->setFlag(Qt::FramelessWindowHint); // No border
    m_window->setFlag(Qt::WindowDoesNotAcceptFocus); // No focus

    // create platform window
    HWND hWnd = (HWND)m_window->winId();

    QTimer::singleShot(0, this, [this, hWnd]() { emit initialize(hWnd); });
};

void QWebView2WebViewPrivate::initialize(HWND hWnd)
{
    connect(m_window, &QWindow::widthChanged, this,
            &QWebView2WebViewPrivate::updateWindowGeometry, Qt::QueuedConnection);
    connect(m_window, &QWindow::heightChanged, this,
            &QWebView2WebViewPrivate::updateWindowGeometry, Qt::QueuedConnection);
    connect(m_window, &QWindow::screenChanged, this,
            &QWebView2WebViewPrivate::updateWindowGeometry, Qt::QueuedConnection);

    QPointer<QWebView2WebViewPrivate> thisPtr = this;
    const QString userDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) % QDir::separator() % QLatin1StringView("WebView2");
    using W2ControllerCallback = ICoreWebView2CreateCoreWebView2ControllerCompletedHandler;
    auto controllerCallback = Microsoft::WRL::Callback<W2ControllerCallback>(
            [thisPtr, this](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
                if (thisPtr.isNull())
                    return S_FALSE;

                if (!controller)
                    return S_FALSE;
                HRESULT hr;
                m_webviewController = controller;
                hr = m_webviewController->get_CoreWebView2(&m_webview);
                Q_ASSERT_SUCCEEDED(hr);

                ComPtr<ICoreWebView2_2> webview2;
                hr = m_webview->QueryInterface(IID_PPV_ARGS(&webview2));
                Q_ASSERT_SUCCEEDED(hr);
                hr = webview2->get_CookieManager(&m_cookieManager);
                Q_ASSERT_SUCCEEDED(hr);

                m_settings->init(m_webviewController.Get());

                // Add a few settings for the webview
                ComPtr<ICoreWebView2Settings> settings;
                hr = m_webview->get_Settings(&settings);
                Q_ASSERT_SUCCEEDED(hr);
                hr = settings->put_IsScriptEnabled(m_settings->javaScriptEnabled());
                Q_ASSERT_SUCCEEDED(hr);
                hr = settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                Q_ASSERT_SUCCEEDED(hr);
                hr = settings->put_IsWebMessageEnabled(TRUE);
                Q_ASSERT_SUCCEEDED(hr);

                // Schedule an async task to navigate to the url
                // Because this is a callback and it might be triggered with a delay
                if (!m_url.isEmpty() && m_url.isValid() && !m_url.scheme().isEmpty()) {
                    hr = m_webview->Navigate((wchar_t *)m_url.toString().utf16());
                    Q_ASSERT_SUCCEEDED(hr);
                } else if (!m_initData.m_html.isEmpty()) {
                    hr = m_webview->NavigateToString((wchar_t *)m_initData.m_html.utf16());
                    Q_ASSERT_SUCCEEDED(hr);
                }
                if (m_initData.m_cookies.size() > 0) {
                    for (auto it = m_initData.m_cookies.constBegin();
                         it != m_initData.m_cookies.constEnd(); ++it)
                        setCookie(it->domain, it->name, it.value().value);
                }
                if (!m_initData.m_httpUserAgent.isEmpty()) {
                    ComPtr<ICoreWebView2Settings2> settings2;
                    hr = settings->QueryInterface(IID_PPV_ARGS(&settings2));
                    if (settings2) {
                        hr = settings2->put_UserAgent(
                                (wchar_t *)m_initData.m_httpUserAgent.utf16());
                        if (SUCCEEDED(hr))
                            QTimer::singleShot(0, thisPtr, [thisPtr] {
                                if (!thisPtr.isNull())
                                    emit thisPtr->q_ptr->httpUserAgentStringChanged(
                                            thisPtr->m_initData.m_httpUserAgent);
                            });
                    }
                }

                EventRegistrationToken token;
                hr = m_webview->add_NavigationStarting(
                        Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
                                [this](ICoreWebView2 *webview,
                                       ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
                                    return this->onNavigationStarting(webview, args);
                                })
                                .Get(),
                        &token);
                Q_ASSERT_SUCCEEDED(hr);

                hr = m_webview->add_NavigationCompleted(
                        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                [this](ICoreWebView2 *webview,
                                       ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT {
                                    return this->onNavigationCompleted(webview, args);
                                })
                                .Get(),
                        &token);
                Q_ASSERT_SUCCEEDED(hr);

                m_webview->add_WebResourceRequested(
                        Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
                                [this](ICoreWebView2 *webview,
                                       ICoreWebView2WebResourceRequestedEventArgs *args)
                                        -> HRESULT {
                                    return this->onWebResourceRequested(webview, args);
                                })
                                .Get(),
                        &token);

                hr = m_webview->add_ContentLoading(
                        Microsoft::WRL::Callback<ICoreWebView2ContentLoadingEventHandler>(
                                [this](ICoreWebView2 *webview,
                                       ICoreWebView2ContentLoadingEventArgs *args) -> HRESULT {
                                    return this->onContentLoading(webview, args);
                                })
                                .Get(),
                        &token);
                Q_ASSERT_SUCCEEDED(hr);

                hr = m_webview->add_NewWindowRequested(
                        Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                                [this](ICoreWebView2 *webview,
                                       ICoreWebView2NewWindowRequestedEventArgs *args) -> HRESULT {
                                    return this->onNewWindowRequested(webview, args);
                                })
                                .Get(),
                        &token);
                Q_ASSERT_SUCCEEDED(hr);

                ComPtr<ICoreWebView2_22> webview22;
                hr = m_webview->QueryInterface(IID_PPV_ARGS(&webview22));
                Q_ASSERT_SUCCEEDED(hr);
                hr = webview22->AddWebResourceRequestedFilterWithRequestSourceKinds(
                        L"file://*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL,
                        COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL);
                Q_ASSERT_SUCCEEDED(hr);
                this->updateWindowGeometry();
                return S_OK;
            });
    using W2EnvironmentCallback = ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler;
    auto environmentCallback = Microsoft::WRL::Callback<W2EnvironmentCallback>(
            [hWnd, thisPtr, controllerCallback, this](HRESULT result,
                                                      ICoreWebView2Environment *env) -> HRESULT {
                env->CreateCoreWebView2Controller(hWnd, controllerCallback.Get());
                return S_OK;
            });
    CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.toStdWString().c_str(),
                                             nullptr, environmentCallback.Get());
}

QWebView2WebViewPrivate::~QWebView2WebViewPrivate()
{
    m_webviewController = nullptr;
    m_webview = nullptr;
}

QString QWebView2WebViewPrivate::httpUserAgent() const
{
    if (m_webviewController) {
        ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_webview->get_Settings(&settings);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<ICoreWebView2Settings2> settings2;
        hr = settings->QueryInterface(IID_PPV_ARGS(&settings2));

        if (settings2) {
            wchar_t *userAgent;
            hr = settings2->get_UserAgent(&userAgent);
            Q_ASSERT_SUCCEEDED(hr);
            QString userAgentString(userAgent);
            CoTaskMemFree(userAgent);
            return userAgentString;
        }
        qWarning() << "No http user agent available.";
        return "";
    }
    return m_initData.m_httpUserAgent;
}

void QWebView2WebViewPrivate::setHttpUserAgent(const QString &userAgent)
{
    if (m_webviewController) {
        ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_webview->get_Settings(&settings);
        Q_ASSERT_SUCCEEDED(hr);

        ComPtr<ICoreWebView2Settings2> settings2;
        hr = settings->QueryInterface(IID_PPV_ARGS(&settings2));
        if (settings2) {
            hr = settings2->put_UserAgent((wchar_t*)userAgent.utf16());
            if (SUCCEEDED(hr))
                emit q_ptr->httpUserAgentStringChanged(userAgent);
            return;
        } else {
            qWarning() << "No http user agent setting available.";
        }
    } else {
        m_initData.m_httpUserAgent = userAgent;
    }
}

QUrl QWebView2WebViewPrivate::url() const
{
    return m_url;
}

void QWebView2WebViewPrivate::setUrl(const QUrl &url)
{
    m_url = url;
    if (m_webview) {
        HRESULT hr = m_webview->Navigate((wchar_t*)url.toString().utf16());
        if (FAILED(hr)) {
            m_isLoading = false;
            m_progress = 100;
            emit q_ptr->loadProgressChanged(100);
            emit q_ptr->loadingChanged(QWebViewFactory::LoadingInfo::create(
                    url, QWebViewLoadingInfo::LoadStatus::Failed, QString()));
        }
    }
}

bool QWebView2WebViewPrivate::canGoBack() const
{
    BOOL canGoBack = false;
    if (m_webviewController) {
        HRESULT hr = m_webview->get_CanGoBack(&canGoBack);
        Q_ASSERT_SUCCEEDED(hr);
    }
    return canGoBack;
}

bool QWebView2WebViewPrivate::canGoForward() const
{
    BOOL canGoForward = false;
    if (m_webviewController) {
        HRESULT hr = m_webview->get_CanGoForward(&canGoForward);
        Q_ASSERT_SUCCEEDED(hr);
    }
    return canGoForward;
}

QString QWebView2WebViewPrivate::title() const
{
    if (m_webviewController) {
        wchar_t *title;
        HRESULT hr = m_webview->get_DocumentTitle(&title);
        Q_ASSERT_SUCCEEDED(hr);
        QString titleString(title);
        CoTaskMemFree(title);
        return titleString;
    }
    return QString();
}

int QWebView2WebViewPrivate::loadProgress() const
{
    return m_progress;
}

bool QWebView2WebViewPrivate::isLoading() const
{
    return m_isLoading;
}


QWindow* QWebView2WebViewPrivate::nativeWindow() const
{
    return m_window;
}

void QWebView2WebViewPrivate::goBack()
{
    if (m_webview) {
        const HRESULT hr = m_webview->GoBack();
        Q_ASSERT_SUCCEEDED(hr);
    }
}

void QWebView2WebViewPrivate::goForward()
{
    if (m_webview) {
        const HRESULT hr = m_webview->GoForward();
        Q_ASSERT_SUCCEEDED(hr);
    }
}

void QWebView2WebViewPrivate::reload()
{
    if (m_webview) {
        const HRESULT hr = m_webview->Reload();
        Q_ASSERT_SUCCEEDED(hr);
    }
}

void QWebView2WebViewPrivate::stop()
{
    if (m_webview) {
        const HRESULT hr = m_webview->Stop();
        Q_ASSERT_SUCCEEDED(hr);
    }
}

void QWebView2WebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    if (!baseUrl.isEmpty())
        qWarning("Base URLs for loadHtml() are not supported on this platform. The document will be treated as coming from 'about:blank'.");

    QByteArray encoded("data:text/html;charset=UTF-8,");
    encoded.append(html.toUtf8().toPercentEncoding());
    m_url = QUrl(encoded);

    if (m_webview) {
        const HRESULT hr = m_webview->NavigateToString((wchar_t *)html.utf16());
        Q_ASSERT_SUCCEEDED(hr);
    } else {
        m_initData.m_html = html;
    }
}

void QWebView2WebViewPrivate::setCookie(const QString &domain,
                                        const QString &name,
                                        const QString &value)
{
    if (m_webview) {
        if (m_cookieManager) {
            ComPtr<ICoreWebView2Cookie> cookie;
            HRESULT hr = m_cookieManager->CreateCookie((wchar_t*)name.utf16(),
                                             (wchar_t*)value.utf16(),
                                             (wchar_t*)domain.utf16(),
                                             L"",
                                             &cookie);
            Q_ASSERT_SUCCEEDED(hr);
            hr = m_cookieManager->AddOrUpdateCookie(cookie.Get());
            Q_ASSERT_SUCCEEDED(hr);
            emit q_ptr->cookieAdded(domain, name);
        }
    } else {
        m_initData.m_cookies.insert(domain + "/" + name, {domain, name, value});
    }
}

void QWebView2WebViewPrivate::deleteCookie(const QString &domainName, const QString &cookieName)
{
    if (m_webview) {
        if (m_cookieManager) {
            QString uri = domainName;
            if (!uri.startsWith("http"))
                uri = "https://" + uri;
            HRESULT hr = m_cookieManager->GetCookies((wchar_t*)uri.utf16(),
            Microsoft::WRL::Callback<ICoreWebView2GetCookiesCompletedHandler>(
            [cookieName, domainName, this]
            (HRESULT result, ICoreWebView2CookieList* cookieList)->HRESULT
            {
                UINT count = 0;
                cookieList->get_Count(&count);
                for (UINT i = 0; i < count; ++i) {
                    ComPtr<ICoreWebView2Cookie> cookie;
                    if (SUCCEEDED(cookieList->GetValueAtIndex(i, &cookie))) {
                        wchar_t *domainPtr;
                        wchar_t *namePtr;
                        if (SUCCEEDED(cookie->get_Domain(&domainPtr)) &&
                            SUCCEEDED(cookie->get_Name(&namePtr))) {
                            QString domain(domainPtr);
                            QString name(namePtr);
                            CoTaskMemFree(namePtr);
                            CoTaskMemFree(domainPtr);
                            if (domainName == domain && cookieName == name) {
                                emit q_ptr->cookieRemoved(domain, cookieName);
                                return S_OK;
                            }
                        }
                    }
                }
              return S_OK;
            }).Get());
            Q_ASSERT_SUCCEEDED(hr);
            hr = m_cookieManager->DeleteCookiesWithDomainAndPath((wchar_t*)cookieName.utf16(),
                                                                 (wchar_t*)domainName.utf16(),
                                                                 L"");
            Q_ASSERT_SUCCEEDED(hr);


        }
    } else {
        m_initData.m_cookies.remove(domainName + "/" + cookieName);
    }
}

void QWebView2WebViewPrivate::deleteAllCookies()
{
    if (m_webview) {
        if (m_cookieManager) {
            HRESULT hr = m_cookieManager->GetCookies(L"",
            Microsoft::WRL::Callback<ICoreWebView2GetCookiesCompletedHandler>(
            [this](HRESULT result, ICoreWebView2CookieList* cookieList) -> HRESULT
            {
                UINT count = 0;
                cookieList->get_Count(&count);
                for (UINT i = 0; i < count; ++i) {
                    ComPtr<ICoreWebView2Cookie> cookie;
                    if (SUCCEEDED(cookieList->GetValueAtIndex(i, &cookie))) {
                        wchar_t *domain;
                        wchar_t *name;
                        if (SUCCEEDED(cookie->get_Domain(&domain))) {
                            if (SUCCEEDED(cookie->get_Name(&name))) {
                                emit q_ptr->cookieRemoved(QString(domain), QString(name));
                                CoTaskMemFree(name);
                            }
                            CoTaskMemFree(domain);
                        }
                    }
                }
              return S_OK;
            }).Get());
            Q_ASSERT_SUCCEEDED(hr);
            hr = m_cookieManager->DeleteAllCookies();
            Q_ASSERT_SUCCEEDED(hr);
        }
    } else {
        m_initData.m_cookies.clear();
    }
}

HRESULT QWebView2WebViewPrivate::onNavigationStarting(ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args)
{
    wchar_t *uri;
    HRESULT hr = args->get_Uri(&uri);
    Q_ASSERT_SUCCEEDED(hr);
    std::wstring_view source(uri);
    m_url.setUrl(QString(source));
    emit q_ptr->urlChanged(m_url);
    CoTaskMemFree(uri);
    return S_OK;
}


HRESULT QWebView2WebViewPrivate::onNavigationCompleted(ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args)
{
    m_isLoading = false;
    m_progress = 100;

    BOOL isSuccess;
    HRESULT hr = args->get_IsSuccess(&isSuccess);
    Q_ASSERT_SUCCEEDED(hr);
    const QWebViewLoadingInfo::LoadStatus status = isSuccess
            ? QWebViewLoadingInfo::LoadStatus::Succeeded
            : QWebViewLoadingInfo::LoadStatus::Failed;

    COREWEBVIEW2_WEB_ERROR_STATUS errorStatus;
    hr = args->get_WebErrorStatus(&errorStatus);
    Q_ASSERT_SUCCEEDED(hr);
    if (errorStatus != COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED) {
        const QString errorStr = isSuccess ? "" : WebErrorStatusToString(errorStatus);
        emit q_ptr->titleChanged(title());
        emit q_ptr->loadProgressChanged(100);
        emit q_ptr->loadingChanged(QWebViewFactory::LoadingInfo::create(
            m_url, status, errorStr));
    } else {
        emit q_ptr->loadingChanged(QWebViewFactory::LoadingInfo::create(
                m_url, QWebViewLoadingInfo::LoadStatus::Stopped, QString()));
    }
    return S_OK;
}

HRESULT QWebView2WebViewPrivate::onWebResourceRequested(ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args)
{
    ComPtr<ICoreWebView2WebResourceRequest> request;
    ComPtr<ICoreWebView2WebResourceResponse> response;
    HRESULT hr = args->get_Request(&request);
    Q_ASSERT_SUCCEEDED(hr);
    wchar_t *uri;
    hr = request->get_Uri(&uri);
    std::wstring_view source(uri);

    if (!m_settings->allowFileAccess()) {
        ComPtr<ICoreWebView2Environment> environment;
        ComPtr<ICoreWebView2_2> webview2;
        m_webview->QueryInterface(IID_PPV_ARGS(&webview2));
        webview2->get_Environment(&environment);

        hr = environment->CreateWebResourceResponse(nullptr, 403, L"Access Denied", L"", &response);
        Q_ASSERT_SUCCEEDED(hr)
        hr = args->put_Response(response.Get());
        Q_ASSERT_SUCCEEDED(hr)
    }

    CoTaskMemFree(uri);
    return S_OK;
}

HRESULT QWebView2WebViewPrivate::onContentLoading(ICoreWebView2* webview, ICoreWebView2ContentLoadingEventArgs* args)
{
    m_isLoading = true;
    m_progress = 0;
    emit q_ptr->loadingChanged(QWebViewFactory::LoadingInfo::create(
            m_url, QWebViewLoadingInfo::LoadStatus::Started, QString()));
    emit q_ptr->loadProgressChanged(0);
    return S_OK;
}

HRESULT QWebView2WebViewPrivate::onNewWindowRequested(ICoreWebView2* webview, ICoreWebView2NewWindowRequestedEventArgs* args)
{
    Q_UNUSED(webview);
    // This blocks the spawning of new windows we don't control
    // FIXME actually handle new windows when QWebView has the API for them
    args->put_Handled(TRUE);
    return S_OK;
}

void QWebView2WebViewPrivate::updateWindowGeometry()
{
    if (m_webviewController) {
        RECT bounds;
        GetClientRect((HWND)m_window->winId(), &bounds);
        const HRESULT hr = m_webviewController->put_Bounds(bounds);
        Q_ASSERT_SUCCEEDED(hr);
    }
}

void QWebView2WebViewPrivate::runJavaScript(
        const QString &script, const std::function<void(const QVariant &)> &resultCallback)
{
    if (m_webview) {
        m_webview->ExecuteScript(
                (wchar_t *)script.utf16(),
                Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                        [this, resultCallback](HRESULT errorCode,
                                               LPCWSTR resultObjectAsJson) -> HRESULT {
                            QString resultStr = QString::fromWCharArray(resultObjectAsJson);

                            QJsonParseError parseError;
                            QJsonDocument jsonDoc =
                                    QJsonDocument::fromJson(resultStr.toUtf8(), &parseError);

                            QVariant resultVariant;
                            if (parseError.error == QJsonParseError::NoError) {
                                resultVariant = jsonDoc.toVariant();
                            } else {
                                QString wrapped = QString("{\"value\":%1}").arg(resultStr);
                                jsonDoc = QJsonDocument::fromJson(wrapped.toUtf8(), &parseError);
                                if (parseError.error == QJsonParseError::NoError) {
                                    resultVariant = jsonDoc.object().value("value").toVariant();
                                } else {
                                    QJsonValue val = QJsonValue::fromVariant(resultStr);
                                    resultVariant = val.toVariant();
                                }
                            }
                            QVariant r =
                                    errorCode == S_OK ? resultVariant : qt_error_string(errorCode);
                            if (resultCallback)
                                resultCallback(r);
                            return errorCode;
                        })
                        .Get());
    } else {
        if (resultCallback)
            resultCallback(QVariant());
    }
}

QWebViewSettingsPrivate *QWebView2WebViewPrivate::settings() const
{
    return m_settings;
}
