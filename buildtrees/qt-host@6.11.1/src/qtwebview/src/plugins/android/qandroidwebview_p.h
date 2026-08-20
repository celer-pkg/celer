// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QANDROIDWEBVIEW_P_H
#define QANDROIDWEBVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtGui/qwindow.h>
#include <QtCore/qjniobject.h>
#include <QtCore/qjnitypes.h>

#include <private/qwebview_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_JNI_CLASS(WebViewController, "org/qtproject/qt/android/view/QtAndroidWebViewController");
Q_DECLARE_JNI_CLASS(WebView, "android/webkit/WebView");

class QAndroidWebViewSettingsPrivate : public QWebViewSettingsPrivate
{
public:
    explicit QAndroidWebViewSettingsPrivate(const QtJniTypes::WebViewController &viewController);

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
    QtJniTypes::WebViewController m_viewController;
};

class QAndroidWebViewPrivate : public QWebViewPrivate
{
    Q_OBJECT
public:
    explicit QAndroidWebViewPrivate(QWebView *view);
    ~QAndroidWebViewPrivate() override;

    void initialize(QObject *context) override { Q_UNUSED(context); };
    QString httpUserAgent() const override;
    void setHttpUserAgent(const QString &httpUserAgent) override;
    QUrl url() const override;
    void setUrl(const QUrl &url) override;
    bool canGoBack() const override;
    bool canGoForward() const override;
    QString title() const override;
    int loadProgress() const override;
    bool isLoading() const override;

    QWindow *nativeWindow() const override { return m_window; }

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

    void javaScriptResult(int id, const QVariant &result);

protected:
    QWebViewSettingsPrivate *settings() const override;

private Q_SLOTS:
    void onApplicationStateChanged(Qt::ApplicationState state);

private:
    int m_callbackId;
    QMap<int, std::function<void(const QVariant &)>> m_callbacks;

    QWindow *m_window;
    QtJniTypes::WebViewController m_viewController;
    QtJniTypes::WebView m_webView;
    QAndroidWebViewSettingsPrivate *m_settings;
};

QT_END_NAMESPACE

#endif // QANDROIDWEBVIEW_P_H
