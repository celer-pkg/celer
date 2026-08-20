// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebview2webview_p.h"
#include <QtWebView/private/qwebviewplugin_p.h>

QT_BEGIN_NAMESPACE

class QWebView2WebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "windows.json")

public:
    QWebViewPrivate *create(const QString &key, QWebView *view,
                            QWebViewFactory::Hint hint) const override
    {
        Q_UNUSED(hint);
        return (key == QLatin1String("webview")) ? new QWebView2WebViewPrivate(view) : nullptr;
    }
};

QT_END_NAMESPACE

#include "qwebview2webviewplugin.moc"
