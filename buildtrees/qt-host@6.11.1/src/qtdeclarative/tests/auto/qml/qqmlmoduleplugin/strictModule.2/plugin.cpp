// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QDebug>

class MyPluginTypeStrictV2 : public QObject
{
    Q_OBJECT
};

class MyPluginStrictV2 : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.qtproject.StrictModule");
        qmlRegisterType<MyPluginTypeStrictV2>(uri, 2, 0, "MyPluginType");
    }
};

#include "plugin.moc"
