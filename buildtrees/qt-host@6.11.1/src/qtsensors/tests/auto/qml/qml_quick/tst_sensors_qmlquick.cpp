// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTest>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtSensorsQuick/private/qmlsensor_p.h>
#include "../../common/test_backends.h"

class TestSetup : public QObject
{
    Q_OBJECT

public:
    TestSetup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine) {
        engine->rootContext()->setContextProperty("TestControl", this);
    }

    void registerTestBackends() {
        register_test_backends();
    }

    void unregisterTestBackends() {
        unregister_test_backends();
    }

    void setSensorReading(const QmlSensor* qmlSensor, const QVariantMap& values) {
        set_test_backend_reading(qmlSensor->sensor(), values);
    }

    void setSensorBusy(const QmlSensor* qmlSensor, bool busy) {
        set_test_backend_busy(qmlSensor->sensor(), busy);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(tst_sensors_qmlquick, TestSetup)

#include "tst_sensors_qmlquick.moc"
