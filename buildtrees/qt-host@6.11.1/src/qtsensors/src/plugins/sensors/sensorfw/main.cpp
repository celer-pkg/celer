// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "sensorfwaccelerometer.h"
#include "sensorfwals.h"
#include "sensorfwcompass.h"
#include "sensorfwmagnetometer.h"
#include "sensorfworientationsensor.h"
#include "sensorfwproximitysensor.h"
#include "sensorfwirproximitysensor.h"
#include "sensorfwrotationsensor.h"
#include "sensorfwtapsensor.h"
#include "sensorfwgyroscope.h"
#include "sensorfwlightsensor.h"
#include "sensorfwlidsensor.h"

#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QDebug>
#include <QSettings>

class sensorfwSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)

public:

    void registerSensors() override
    {
        // if no default - no support either, uses Sensors.conf
        QSettings settings(QSettings::SystemScope, QLatin1String("QtProject"), QLatin1String("Sensors"));
        settings.beginGroup(QLatin1String("Default"));
        QStringList keys = settings.allKeys();
        for (int i=0,l=keys.size(); i<l; i++) {
            QString type = keys.at(i);
            if (settings.value(type).toString().contains(QStringLiteral("sensorfw")))//register only ones we know
                QSensorManager::registerBackend(type.toLocal8Bit(), settings.value(type).toByteArray(), this);
        }
    }


    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == sensorfwaccelerometer::id)
            return new sensorfwaccelerometer(sensor);
        if (sensor->identifier() == Sensorfwals::id)
            return new Sensorfwals(sensor);
        if (sensor->identifier() == SensorfwCompass::id)
            return new SensorfwCompass(sensor);
        if (sensor->identifier() == SensorfwMagnetometer::id)
            return new SensorfwMagnetometer(sensor);
        if (sensor->identifier() == SensorfwOrientationSensor::id)
            return new SensorfwOrientationSensor(sensor);
        if (sensor->identifier() == SensorfwProximitySensor::id)
            return new SensorfwProximitySensor(sensor);
        if (sensor->identifier() == SensorfwRotationSensor::id)
            return new SensorfwRotationSensor(sensor);
        if (sensor->identifier() == SensorfwTapSensor::id)
            return new SensorfwTapSensor(sensor);
        if (sensor->identifier() == SensorfwGyroscope::id)
            return new SensorfwGyroscope(sensor);
        if (sensor->identifier() == SensorfwLidSensor::id)
            return new SensorfwLidSensor(sensor);
        if (sensor->identifier() == SensorfwLightSensor::id)
            return new SensorfwLightSensor(sensor);
        if (sensor->identifier() == SensorfwIrProximitySensor::id)
            return new SensorfwIrProximitySensor(sensor);
        return 0;
    }
};

#include "main.moc"
