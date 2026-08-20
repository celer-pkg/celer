// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwasmwebview_p.h"
#include <QtWebView/private/qwebviewplugin_p.h>

QT_BEGIN_NAMESPACE

class QWasmWebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "wasm.json")

public:
    QWebViewPrivate *create(const QString &key, QWebView *view,
                            QWebViewFactory::Hint hint) const override
    {
        Q_UNUSED(hint);
        return key == QLatin1String("webview") ? new QWasmWebViewPrivate(view) : nullptr;
    }
};

QT_END_NAMESPACE

#include "qwasmwebviewplugin.moc"
