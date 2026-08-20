// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebenginewebview_p.h"
#include <QtWebView/private/qwebviewplugin_p.h>

QT_BEGIN_NAMESPACE

class QWebEngineWebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "webengine.json")

public:
    QWebViewPrivate *create(const QString &key, QWebView *view,
                            QWebViewFactory::Hint hint) const override
    {
        if (key != QLatin1String("webview"))
            return nullptr;
        if (hint == QWebViewFactory::Hint::QuickInitialization) {
            return new QQuickItemWebEngineWebViewPrivate(view);
        } else {
            return new QQuickViewWebEngineWebViewPrivate(view);
        }
    }

    void prepare() const override { }
};

QT_END_NAMESPACE

#include "qwebenginewebviewplugin.moc"
