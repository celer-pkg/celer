// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "test_sensorimpl.h"
#include "test_sensor2impl.h"
#include <qsensorplugin.h>
#include <qsensorbackend.h>
#include <qsensormanager.h>
#include <QFile>
#include <QDebug>
#include <QTest>

class TestSensorPlugin : public QObject,
                         public QSensorPluginInterface,
                         public QSensorChangesInterface,
                         public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0")
    Q_INTERFACES(QSensorPluginInterface QSensorChangesInterface)
public:
    void registerSensors() override
    {
        static bool recursive = false;
        QVERIFY2(!recursive, "Recursively called TestSensorPlugin::registerSensors!");
        if (recursive) return;
        recursive = true;


        // This is bad code. It caused a crash due to recursively calling
        // loadPlugins() in qsensormanager.cpp (because loadPlugins() did
        // not set the pluginsLoaded flag soon enough).
        (void)QSensor::defaultSensorForType(TestSensor::sensorType);

        QSensorManager::registerBackend(TestSensor::sensorType, testsensorimpl::id, this);
        QSensorManager::registerBackend(TestSensor::sensorType, "test sensor 2", this);
        QSensorManager::registerBackend(TestSensor2::sensorType, testsensor2impl::id, this);
    }

    void sensorsChanged() override
    {
        // Register a new type on initial load
        // This is testing the "don't emit availableSensorsChanged() too many times" functionality.
        if (!QSensorManager::isBackendRegistered(TestSensor::sensorType, "test sensor 3"))
            QSensorManager::registerBackend(TestSensor::sensorType, "test sensor 3", this);

        // When a sensor of type "a random type" is registered, register another sensor.
        // This is testing the "don't emit availableSensorsChanged() too many times" functionality.
        if (!QSensor::defaultSensorForType("a random type").isEmpty()) {
            if (!QSensorManager::isBackendRegistered("a random type 2", "random.dynamic"))
                QSensorManager::registerBackend("a random type 2", "random.dynamic", this);
        } else {
            if (QSensorManager::isBackendRegistered("a random type 2", "random.dynamic"))
                QSensorManager::unregisterBackend("a random type 2", "random.dynamic");
        }
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == testsensorimpl::id) {
            return new testsensorimpl(sensor);
        }
        if (sensor->identifier() == testsensor2impl::id) {
            return new testsensor2impl(sensor);
        }

        qWarning() << "Can't create backend" << sensor->identifier();
        return 0;
    }
};

Q_IMPORT_PLUGIN(TestSensorPlugin)

#include "test_sensorplugin.moc"
