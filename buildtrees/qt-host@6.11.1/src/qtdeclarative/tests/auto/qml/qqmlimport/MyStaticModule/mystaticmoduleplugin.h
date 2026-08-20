// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MYSTATICMODULEPLUGIN_H
#define MYSTATICMODULEPLUGIN_H

#include <QtQml/qqmlextensionplugin.h>
#include <QtCore/qdebug.h>
#include <QtQml/qqml.h>

class MyCppComponent : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int myP READ myP WRITE setMyP NOTIFY myPChanged)

public:
    MyCppComponent() : QObject(nullptr) { }

    int myP() const { return m_myP; }
    void setMyP(int newP)
    {
        m_myP = newP;
        emit myPChanged();
    }

signals:
    void myPChanged();

private:
    int m_myP = 0;
};

class MyStaticModulePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    MyStaticModulePlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent) { }

    void registerTypes(const char *uri) override
    {
        qmlRegisterType<MyCppComponent>(uri, 1, 0, "MyCppComponent");
        qmlRegisterModule(uri, 1, 0);
        s_myStaticModulePluginRegistered = true;
    }
    void unregisterTypes() override { s_myStaticModulePluginRegistered = false; }
    static inline bool s_myStaticModulePluginRegistered = false;
};

#endif // MYSTATICMODULEPLUGIN_H
