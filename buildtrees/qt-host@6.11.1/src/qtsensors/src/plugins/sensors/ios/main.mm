// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qsensorplugin.h>
#include <qsensorbackend.h>
#include <qsensormanager.h>

#include "iosmotionmanager.h"
#include "iosaccelerometer.h"
#include "iosgyroscope.h"
#include "iosmagnetometer.h"
#include "ioscompass.h"
#include "iosproximitysensor.h"
#include "iospressure.h"

#import <CoreLocation/CoreLocation.h>
#ifdef HAVE_COREMOTION
#import <CoreMotion/CoreMotion.h>
#endif

class IOSSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:
    void registerSensors() override
    {
#ifdef HAVE_COREMOTION
        QSensorManager::registerBackend(QAccelerometer::sensorType, IOSAccelerometer::id, this);
        if ([QIOSMotionManager sharedManager].gyroAvailable)
            QSensorManager::registerBackend(QGyroscope::sensorType, IOSGyroscope::id, this);
        if ([QIOSMotionManager sharedManager].magnetometerAvailable)
            QSensorManager::registerBackend(QMagnetometer::sensorType, IOSMagnetometer::id, this);
        if ([CMAltimeter isRelativeAltitudeAvailable])
            QSensorManager::registerBackend(QPressureSensor::sensorType, IOSPressure::id, this);
#endif
#ifdef HAVE_COMPASS
        if ([CLLocationManager headingAvailable])
            QSensorManager::registerBackend(QCompass::sensorType, IOSCompass::id, this);
#endif
#ifdef HAVE_UIDEVICE
        if (IOSProximitySensor::available())
            QSensorManager::registerBackend(QProximitySensor::sensorType, IOSProximitySensor::id, this);
#endif
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
#ifdef HAVE_COREMOTION
        if (sensor->identifier() == IOSAccelerometer::id)
            return new IOSAccelerometer(sensor);
        if (sensor->identifier() == IOSGyroscope::id)
            return new IOSGyroscope(sensor);
        if (sensor->identifier() == IOSMagnetometer::id)
            return new IOSMagnetometer(sensor);
        if (sensor->identifier() == IOSPressure::id)
            return new IOSPressure(sensor);
#endif
#ifdef HAVE_COMPASS
        if (sensor->identifier() == IOSCompass::id)
            return new IOSCompass(sensor);
#endif
#ifdef HAVE_UIDEVICE
        if (sensor->identifier() == IOSProximitySensor::id)
            return new IOSProximitySensor(sensor);
#endif
        return 0;
    }
};

#include "main.moc"

