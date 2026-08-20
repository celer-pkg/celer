// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "winrtaccelerometer.h"
#include "winrtcompass.h"
#include "winrtgyroscope.h"
#include "winrtrotationsensor.h"
#include "winrtambientlightsensor.h"
#include "winrtorientationsensor.h"
#include <QtSensors/QAccelerometer>
#include <QtSensors/QAmbientLightSensor>
#include <QtSensors/QCompass>
#include <QtSensors/QGyroscope>
#include <QtSensors/QOrientationSensor>
#include <QtSensors/QRotationSensor>
#include <QtSensors/QSensorBackendFactory>
#include <QtSensors/QSensorManager>
#include <QtSensors/QSensorPluginInterface>
#include <wrl.h>

QT_BEGIN_NAMESPACE

class WinRtSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:

    ~WinRtSensorPlugin()
    {
        CoUninitialize();
    }

    void registerSensors() override
    {
        if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
            qErrnoWarning("CoInitializeEx() failed.");
            return;
        }
        QSensorManager::registerBackend(QAccelerometer::sensorType, QByteArrayLiteral("WinRtAccelerometer"), this);
        QSensorManager::registerBackend(QCompass::sensorType, QByteArrayLiteral("WinRtCompass"), this);
        QSensorManager::registerBackend(QGyroscope::sensorType, QByteArrayLiteral("WinRtGyroscope"), this);
        QSensorManager::registerBackend(QRotationSensor::sensorType, QByteArrayLiteral("WinRtRotationSensor"), this);
        QSensorManager::registerBackend(QAmbientLightSensor::sensorType, QByteArrayLiteral("WinRtAmbientLightSensor"), this);
        QSensorManager::registerBackend(QOrientationSensor::sensorType, QByteArrayLiteral("WinRtOrientationSensor"), this);
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == QByteArrayLiteral("WinRtAccelerometer"))
            return new WinRtAccelerometer(sensor);

        if (sensor->identifier() == QByteArrayLiteral("WinRtCompass"))
            return new WinRtCompass(sensor);

        if (sensor->identifier() == QByteArrayLiteral("WinRtGyroscope"))
            return new WinRtGyroscope(sensor);

        if (sensor->identifier() == QByteArrayLiteral("WinRtRotationSensor"))
            return new WinRtRotationSensor(sensor);

        if (sensor->identifier() == QByteArrayLiteral("WinRtAmbientLightSensor"))
            return new WinRtAmbientLightSensor(sensor);

        if (sensor->identifier() == QByteArrayLiteral("WinRtOrientationSensor"))
            return new WinRtOrientationSensor(sensor);

        return 0;
    }
};

QT_END_NAMESPACE

#include "main.moc"

