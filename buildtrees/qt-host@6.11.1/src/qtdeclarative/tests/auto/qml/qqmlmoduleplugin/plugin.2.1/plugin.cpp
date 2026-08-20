// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QDebug>

class MyPluginTypeV2_1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int valueOnlyIn2 READ value WRITE setValue)

public:
    MyPluginTypeV2_1(QObject *parent = nullptr) : QObject(parent) { qWarning("import2.1 worked"); }

    int value() const { return v; }
    void setValue(int i) { v = i; }

private:
    int v;
};

class MyPluginV2_1 : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    MyPluginV2_1() { qWarning("plugin2.1 created"); }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.qtproject.AutoTestQmlPluginType");
        qmlRegisterType<MyPluginTypeV2_1>(uri, 2, 1, "MyPluginType");
    }
};

#include "plugin.moc"
