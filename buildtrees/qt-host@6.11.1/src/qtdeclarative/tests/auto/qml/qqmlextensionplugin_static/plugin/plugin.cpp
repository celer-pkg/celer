// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QtCore/qdebug.h>

class TestObject : public QObject
{
    Q_OBJECT

public:
    explicit TestObject(QObject *parent = nullptr) : QObject(parent)
    {
        qWarning() << "TestObject created";
    }
};

class MockExtensionPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    MockExtensionPlugin() { qWarning() << "MockExtensionPlugin instantiated"; }

    void registerTypes(const char *uri) override
    {
        qWarning() << "MockExtension::registerTypes called with URI:" << uri;
        qmlRegisterType<TestObject>(uri, 1, 0, "TestObject");
    }

    void initializeEngine(QQmlEngine *, const char *uri) override
    {
        qWarning() << "MockExtension::initializeEngine called with URI:" << uri;
    }
};

#include "plugin.moc"
