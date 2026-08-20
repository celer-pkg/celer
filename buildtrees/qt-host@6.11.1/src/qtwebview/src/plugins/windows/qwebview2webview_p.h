// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBVIEW2WEBVIEW_P_H
#define QWEBVIEW2WEBVIEW_P_H

#include <private/qwebview_p.h>

#include <QMap>
#include <QPointer>
#include <QtCore/qurl.h>
#include <webview2.h>
#include <wrl.h>
#include <wrl/client.h>

QT_BEGIN_NAMESPACE

using namespace Microsoft::WRL;

class QWebview2WebViewSettingsPrivate final : public QWebViewSettingsPrivate
{
public:
    explicit QWebview2WebViewSettingsPrivate();

    void init(ICoreWebView2Controller* viewController);

    bool localStorageEnabled() const;
    bool javaScriptEnabled() const;
    bool localContentCanAccessFileUrls() const;
    bool allowFileAccess() const;

    void setLocalContentCanAccessFileUrls(bool enabled);
    void setJavaScriptEnabled(bool enabled);
    void setLocalStorageEnabled(bool enabled);
    void setAllowFileAccess(bool enabled);

private:
    bool doTestAttribute(WebAttribute attribute) const final;
    void doSetAttribute(WebAttribute attribute, bool value) final;

private:
    ComPtr<ICoreWebView2Controller> m_webviewController;
    ComPtr<ICoreWebView2> m_webview;
    bool m_allowFileAccess = false;
    bool m_localContentCanAccessFileUrls = false;
    bool m_javaScriptEnabled = true;
};

// This is used to store informations before webview2 is initialized
// Because WebView2 initialization is async
struct QWebViewInitData{
    QString m_html;
    struct CookieData{
        QString domain;
        QString name;
        QString value;
    };
    QMap<QString, CookieData > m_cookies;
    QString m_httpUserAgent;
};

class QWebView2WebViewPrivate : public QWebViewPrivate
{
public:
    explicit QWebView2WebViewPrivate(QWebView *view);
    ~QWebView2WebViewPrivate() override;

    void initialize(QObject *context) override { Q_UNUSED(context); };
    QString httpUserAgent() const override;
    void setHttpUserAgent(const QString &userAgent) override;
    QUrl url() const override;
    void setUrl(const QUrl &url) override;
    bool canGoBack() const override;
    bool canGoForward() const override;
    QString title() const override;
    int loadProgress() const override;
    bool isLoading() const override;

    QWindow* nativeWindow() const override;

    void goBack() override;
    void goForward() override;
    void reload() override;
    void stop() override;
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl()) override;
    void setCookie(const QString &domain, const QString &name, const QString &value) override;
    void deleteCookie(const QString &domain, const QString &name) override;
    void deleteAllCookies() override;
    void runJavaScript(const QString &script,
                       const std::function<void(const QVariant &)> &resultCallback) override;

private:
    HRESULT onNavigationStarting(ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args);
    HRESULT onNavigationCompleted(ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args);
    HRESULT onWebResourceRequested(ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args);
    HRESULT onContentLoading(ICoreWebView2* webview, ICoreWebView2ContentLoadingEventArgs* args);
    HRESULT onNewWindowRequested(ICoreWebView2* webview, ICoreWebView2NewWindowRequestedEventArgs* args);
    void updateWindowGeometry();
    void initialize(HWND hWnd);

protected:
    QWebViewSettingsPrivate *settings() const override;

private:
    ComPtr<ICoreWebView2Controller> m_webviewController;
    ComPtr<ICoreWebView2> m_webview;
    ComPtr<ICoreWebView2CookieManager> m_cookieManager;
    QWebview2WebViewSettingsPrivate *m_settings;
    QWindow *m_window;
    bool m_isLoading;
    int m_progress;
    QUrl m_url;
    QWebViewInitData m_initData;
};

QT_END_NAMESPACE

#endif // QWEBVIEW2WEBVIEW_P_H
