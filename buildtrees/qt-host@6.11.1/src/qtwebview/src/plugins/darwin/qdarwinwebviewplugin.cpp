// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdarwinwebview_p.h"
#include <QtWebView/private/qwebviewplugin_p.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QDarwinWebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "darwin.json")

public:
    QWebViewPrivate *create(const QString &key, QWebView *view,
                            QWebViewFactory::Hint hint) const override
    {
        Q_UNUSED(hint);
        return (key == QLatin1String("webview")) ? new QDarwinWebViewPrivate(view) : nullptr;
    }

    void prepare() const override
    {
    }
};

QT_END_NAMESPACE

#include "qdarwinwebviewplugin.moc"
