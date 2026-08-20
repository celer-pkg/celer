// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QDebug>

class MyChildPluginTypeV2_1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int valueOnlyIn2 READ value WRITE setValue)

public:
    MyChildPluginTypeV2_1(QObject *parent = nullptr) : QObject(parent)
    {
        qWarning("child import2.1 worked");
    }

    int value() const { return v; }
    void setValue(int i) { v = i; }

private:
    int v;
};

class MyChildPluginV2_1 : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    MyChildPluginV2_1() { qWarning("child plugin2.1 created"); }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.qtproject.AutoTestQmlPluginType.ChildPlugin");
        qmlRegisterType<MyChildPluginTypeV2_1>(uri, 2, 1, "MyChildPluginType");
    }
};

#include "childplugin.moc"
