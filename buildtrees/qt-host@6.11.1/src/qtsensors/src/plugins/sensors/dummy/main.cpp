// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "dummyaccelerometer.h"
#include "dummylightsensor.h"
#include <qsensorplugin.h>
#include <qsensorbackend.h>
#include <qsensormanager.h>
#include <QFile>
#include <QDebug>

class dummySensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:
    void registerSensors() override
    {
        QSensorManager::registerBackend(QAccelerometer::sensorType, dummyaccelerometer::id, this);
        QSensorManager::registerBackend(QAmbientLightSensor::sensorType, dummylightsensor::id, this);
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == dummyaccelerometer::id) {
            return new dummyaccelerometer(sensor);
        }

        if (sensor->identifier() == dummylightsensor::id) {
            return new dummylightsensor(sensor);
        }

        return 0;
    }
};

#include "main.moc"

