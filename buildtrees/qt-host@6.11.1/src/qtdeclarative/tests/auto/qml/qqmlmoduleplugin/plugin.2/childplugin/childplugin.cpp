// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QDebug>

class MyChildPluginTypeV2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int valueOnlyIn2 READ value WRITE setValue)

public:
    MyChildPluginTypeV2(QObject *parent = nullptr) : QObject(parent)
    {
        qWarning("child import2 worked");
    }

    int value() const { return v; }
    void setValue(int i) { v = i; }

private:
    int v;
};

class MyChildPluginV2 : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    MyChildPluginV2() { qWarning("child plugin2 created"); }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.qtproject.AutoTestQmlPluginType.ChildPlugin");
        qmlRegisterType<MyChildPluginTypeV2>(uri, 2, 0, "MyChildPluginType");
    }
};

#include "childplugin.moc"
