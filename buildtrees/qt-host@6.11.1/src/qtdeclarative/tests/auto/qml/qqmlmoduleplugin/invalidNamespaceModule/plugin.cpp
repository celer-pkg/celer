// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QDebug>

class MyPluginTypeInvalidNamespace : public QObject
{
    Q_OBJECT
};

class MyPluginInvalidNamespace : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.qtproject.InvalidNamespaceModule");
        qmlRegisterType<MyPluginTypeInvalidNamespace>("org.qtproject.SomeOtherModule", 1, 0,
                                                      "MyPluginType");
    }
};

#include "plugin.moc"
