// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBENGINEWEBVIEW_P_H
#define QWEBENGINEWEBVIEW_P_H

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

#include <QtQml/qqmlcomponent.h>
#include <QtWebView/private/qwebview_p.h>
#include <QtWebEngineQuick/QQuickWebEngineProfile>
#include <QtWebEngineQuick/private/qquickwebenginesettings_p.h>

#include <QtCore/qpointer.h>
#include <QVariant>

QT_BEGIN_NAMESPACE

class QQuickItem;
class QQuickWebEngineView;
class QQuickView;
class QWebEngineLoadingInfo;
class QNetworkCookie;
class QWebEngineWebViewPrivate;

class QWebEngineWebViewSettingsPrivate : public QWebViewSettingsPrivate
{
public:
    explicit QWebEngineWebViewSettingsPrivate(QWebEngineWebViewPrivate *p);

    void init(QQuickWebEngineSettings *settings);

private:
    bool doTestAttribute(WebAttribute attribute) const final;
    void doSetAttribute(WebAttribute attribute, bool value) final;

private:
    bool localStorageEnabled() const;
    bool javaScriptEnabled() const;
    bool localContentCanAccessFileUrls() const;
    bool allowFileAccess() const;

    void setLocalContentCanAccessFileUrls(bool enabled);
    void setJavaScriptEnabled(bool enabled);
    void setLocalStorageEnabled(bool enabled);
    void setAllowFileAccess(bool enabled);

private:
    QWebEngineWebViewPrivate *parent;
    QPointer<QQuickWebEngineSettings> m_settings;
    bool m_localStorageEnabled = true;
    bool m_javaScriptEnabled = true;
    bool m_localContentCanAccessFileUrlsEnabled = true;
    bool m_allowFileAccessEnabled = true;
};

class QWebEngineWebViewPrivate : public QWebViewPrivate
{
    Q_OBJECT
public:
    explicit QWebEngineWebViewPrivate(QWebView *p);
    ~QWebEngineWebViewPrivate() override;

    void initialize(QObject *context) override;
    QString httpUserAgent() const override;
    void setHttpUserAgent(const QString &userAgent) override;
    QUrl url() const override;
    void setUrl(const QUrl &url) override;
    bool canGoBack() const override;
    bool canGoForward() const override;
    QString title() const override;
    int loadProgress() const override;
    bool isLoading() const override;

    QWebViewSettingsPrivate *settings() const override;
    QWindow *nativeWindow() const override { return nullptr; }

    void goBack() override;
    void goForward() override;
    void reload() override;
    void stop() override;
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl()) override;
    void setCookie(const QString &domain, const QString &name,
                   const QString &value) override;
    void deleteCookie(const QString &domain, const QString &name) override;
    void deleteAllCookies() override;
    void runJavaScript(const QString &script,
                       const std::function<void(const QVariant &)> &resultCallback) override;
    virtual QQuickWebEngineView *view() const = 0;

private Q_SLOTS:
    void q_urlChanged();
    void q_loadProgressChanged();
    void q_titleChanged();
    void q_loadingChanged(const QWebEngineLoadingInfo &loadRequest);
    void q_profileChanged();
    void q_httpUserAgentChanged();
    void q_cookieAdded(const QNetworkCookie &cookie);
    void q_cookieRemoved(const QNetworkCookie &cookie);

private:
    friend class QWebEngineWebViewSettingsPrivate;

    QQuickWebEngineProfile *m_profile = nullptr;
    mutable QWebEngineWebViewSettingsPrivate *m_settings = nullptr;
    QString m_httpUserAgent;
    QWebEngineCookieStore *m_cookieStore = nullptr;
};

class QQuickViewWebEngineWebViewPrivate : public QWebEngineWebViewPrivate
{
public:
    explicit QQuickViewWebEngineWebViewPrivate(QWebView *p);
    ~QQuickViewWebEngineWebViewPrivate() override;
    void initialize(QObject *context) override;
    QQuickWebEngineView *view() const override;
    std::unique_ptr<QQuickView> m_view;
    QQuickWebEngineView *m_webEngineView;
};

class QQuickItemWebEngineWebViewPrivate : public QWebEngineWebViewPrivate
{
public:
    explicit QQuickItemWebEngineWebViewPrivate(QWebView *p);
    ~QQuickItemWebEngineWebViewPrivate() override;
    void initialize(QObject *context) override;
    QQuickWebEngineView *view() const override;
    QQuickItem *m_parentItem;
    std::unique_ptr<QQuickWebEngineView> m_webEngineView;
};

QT_END_NAMESPACE

#endif // QWEBENGINEWEBVIEW_P_H
