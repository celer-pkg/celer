// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebviewfactory_p.h"
#include "qwebviewplugin_p.h"
#include <private/qfactoryloader_p.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QWebViewPluginInterface_iid, QLatin1String("/webview")))

static QString getPluginName()
{
    static const QString name = !qEnvironmentVariableIsEmpty("QT_WEBVIEW_PLUGIN")
                                ? QString::fromLatin1(qgetenv("QT_WEBVIEW_PLUGIN"))
                                : QStringLiteral("native");
    return name;
}

QWebViewPrivate *QWebViewFactory::createWebView(QWebView *view, Hint hint)
{
    QWebViewPrivate *wv = nullptr;
    QWebViewPlugin *plugin = getPlugin();
    if (plugin)
        wv = plugin->create(QStringLiteral("webview"), view, hint);

    if (!wv || !plugin) {
        qFatal("No WebView plug-in found!");
    }

    return wv;
}

bool QWebViewFactory::requiresExtraInitializationSteps()
{
    const QString pluginName = getPluginName();
    const int index = pluginName.isEmpty() ? 0 : qMax<int>(0, loader->indexOf(pluginName));

    const QList<QPluginParsedMetaData> metaDataList = loader->metaData();
    if (metaDataList.isEmpty())
        return false;

    const auto &pluginMetaData = metaDataList.at(index);
    Q_ASSERT(pluginMetaData.value(QtPluginMetaDataKeys::IID) == QLatin1String(QWebViewPluginInterface_iid));
    const auto metaDataObject = pluginMetaData.value(QtPluginMetaDataKeys::MetaData).toMap();
    return metaDataObject.value(QLatin1String("RequiresInit")).toBool();
}

QWebViewPlugin *QWebViewFactory::getPlugin()
{
    // Plugin loading logic:
    // 1. Get plugin name - plugin name is either user specified or "native"
    //    - Exception: macOS, which will default to using "webengine" until the native plugin is matured.
    // 2. If neither a user specified or "default" plugin exists, then the first available is used.
    const QString pluginName = getPluginName();
    const int index = pluginName.isEmpty() ? 0 : qMax<int>(0, loader->indexOf(pluginName));
    return qobject_cast<QWebViewPlugin *>(loader->instance(index));
}

bool QWebViewFactory::loadedPluginHasKey(const QString key)
{
    const QString &pluginName = getPluginName();
    // instead of creating multimap with QFactoryLoader::KeyMap and doing a search
    // simply check if loded and key index matches
    if (pluginName.isEmpty())
        return false;
    const int loadedIndex = qMax<int>(0, loader->indexOf(pluginName));
    const int keyIndex = loader->indexOf(key);
    return keyIndex > -1 && loadedIndex == keyIndex;
}

QT_END_NAMESPACE
