// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBVIEWSETTINGS_H
#define QWEBVIEWSETTINGS_H

#include <QtWebView/qwebview_global.h>

#include <QtCore/qobject.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QWebViewSettingsPrivate;
class QWindow;

class Q_WEBVIEW_EXPORT QWebViewSettings
{
    Q_GADGET
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_DISABLE_COPY_MOVE(QWebViewSettings)
public:
    enum class WebAttribute : quint32 {
        LocalStorageEnabled,
        JavaScriptEnabled,
        AllowFileAccess,
        LocalContentCanAccessFileUrls,
    };
    Q_ENUM(WebAttribute);

    bool testAttribute(WebAttribute attribute) const;
    void setAttribute(WebAttribute attribute, bool value);

private:
    virtual void doSetAttribute(WebAttribute a, bool v) = 0;
    virtual bool doTestAttribute(WebAttribute a) const = 0;

    friend class QWebViewSettingsPrivate;
    explicit QWebViewSettings();
    virtual ~QWebViewSettings();
};

QT_END_NAMESPACE

#endif // QWEBVIEWSETTINGS_H
