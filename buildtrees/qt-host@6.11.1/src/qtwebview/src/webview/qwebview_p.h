// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QABSTRACTWEBVIEW_P_H
#define QABSTRACTWEBVIEW_P_H

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

#include "qwebview_global.h"
#include "qwebview.h"
#include "qwebviewsettings.h"

#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QWindow;
class QWebViewLoadingInfo;

class Q_WEBVIEW_EXPORT QWebViewSettingsPrivate : public QWebViewSettings
{
public:
    ~QWebViewSettingsPrivate() override;

protected:
    explicit QWebViewSettingsPrivate() { }
};

class Q_WEBVIEW_EXPORT QWebViewPrivate : public QObject
{
public:
    virtual void initialize(QObject *context) = 0;
    virtual QWebViewSettingsPrivate *settings() const = 0;
    virtual QString httpUserAgent() const = 0;
    virtual void setHttpUserAgent(const QString &httpUserAgent) = 0;
    virtual void setUrl(const QUrl &url) = 0;
    virtual bool canGoBack() const = 0;
    virtual bool canGoForward() const = 0;
    virtual QString title() const = 0;
    virtual QUrl url() const = 0;
    virtual int loadProgress() const = 0;
    virtual bool isLoading() const = 0;
    virtual void goBack() = 0;
    virtual void goForward() = 0;
    virtual void stop() = 0;
    virtual void reload() = 0;
    virtual void loadHtml(const QString &html, const QUrl &baseUrl) = 0;
    virtual void setCookie(const QString &domain, const QString &name, const QString &value) = 0;
    virtual void deleteCookie(const QString &domain, const QString &name) = 0;
    virtual void deleteAllCookies() = 0;
    virtual QWindow *nativeWindow() const = 0;
    virtual void runJavaScript(const QString &script,
                               const std::function<void(const QVariant &)> &resultCallback) = 0;

protected:
    explicit QWebViewPrivate(QWebView *view) : q_ptr(view) { };

public:
    std::unique_ptr<QWebViewSettingsPrivate> m_settings;
    QWebView *q_ptr;
};

QT_END_NAMESPACE

#endif // QABSTRACTWEBVIEW_P_H
