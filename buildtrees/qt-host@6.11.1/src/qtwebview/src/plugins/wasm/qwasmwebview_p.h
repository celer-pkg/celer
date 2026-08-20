// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWASMWEBVIEW_P_H
#define QWASMWEBVIEW_P_H

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
#include <QtCore/qpointer.h>
#include <QtGui/qwindow.h>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/qpa/qplatformwindow_p.h>
#include <emscripten/val.h>

#include <private/qwebview_p.h>

#include <optional>

QT_BEGIN_NAMESPACE

class QWasmWebViewSettingsPrivate final : public QWebViewSettingsPrivate
{
public:
    explicit QWasmWebViewSettingsPrivate();

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
};

class QWasmWebViewPrivate final : public QWebViewPrivate
{
    Q_OBJECT
public:
    explicit QWasmWebViewPrivate(QWebView *view);
    ~QWasmWebViewPrivate() override;

    void initialize(QObject *context) override { Q_UNUSED(context); };
    QString httpUserAgent() const final;
    void setHttpUserAgent(const QString &httpUserAgent) final;
    QUrl url() const override;
    void setUrl(const QUrl &url) final;
    bool canGoBack() const final;
    bool canGoForward() const final;
    QString title() const final;
    int loadProgress() const final;
    bool isLoading() const final;

    QWindow *nativeWindow() const override { return m_window; }

    void goBack() final;
    void goForward() final;
    void reload() final;
    void stop() final;
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl()) final;
    void setCookie(const QString &domain, const QString &name, const QString &value) final;
    void deleteCookie(const QString &domain, const QString &name) final;
    void deleteAllCookies() final;

    void runJavaScript(const QString &script,
                       const std::function<void(const QVariant &)> &resultCallback) final;

protected:
    QWebViewSettingsPrivate *settings() const final;

private:
    Q_INVOKABLE void initializeIFrame();
    void updateGeometry();

    QWasmWebViewSettingsPrivate *m_settings;
    QWindow *m_window = nullptr;
    std::optional<emscripten::val> m_iframe;
    std::optional<QRect> m_geometry;
    QUrl m_currentUrl;
};

QT_END_NAMESPACE

#endif // QWASMWEBVIEW_P_H
