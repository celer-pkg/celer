// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdeclarativepinchgenerator_p.h"
#include "qdeclarativelocationtestmodel_p.h"
#include "testhelper.h"

#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/qqml.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

static QObject *helper_factory(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    TestHelper *helper = new TestHelper();
    return helper;
}

class QLocationDeclarativeTestModule: public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char* uri) override
    {
        if (QLatin1String(uri) == QLatin1String("QtLocation.Test")) {
            qmlRegisterType<QDeclarativePinchGenerator>(uri, 5, 5, "PinchGenerator");
            qmlRegisterType<QDeclarativeLocationTestModel>(uri, 5, 5, "TestModel");
            qmlRegisterSingletonType<TestHelper>(uri, 5, 6, "LocationTestHelper", helper_factory);
        } else {
            qWarning() << "Unsupported URI given to load location test QML plugin: " << QLatin1String(uri);
        }
    }
};

QT_END_NAMESPACE

#include "locationtest.moc"
