// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxyorientationsensor.h"
#include "iiosensorproxylightsensor.h"
#include "iiosensorproxycompass.h"
#include "iiosensorproxyproximitysensor.h"

#include <qsensorplugin.h>
#include <qsensorbackend.h>
#include <qsensormanager.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include <QtCore/QFile>
#include <QtCore/QDebug>

class IIOSensorProxySensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)
public:
    void registerSensors() override
    {
        if (QDBusConnection::systemBus().interface()->isServiceRegistered("net.hadess.SensorProxy")) {
            if (!QSensorManager::isBackendRegistered(QOrientationSensor::sensorType, IIOSensorProxyOrientationSensor::id))
                QSensorManager::registerBackend(QOrientationSensor::sensorType, IIOSensorProxyOrientationSensor::id, this);
            if (!QSensorManager::isBackendRegistered(QLightSensor::sensorType, IIOSensorProxyLightSensor::id))
                QSensorManager::registerBackend(QLightSensor::sensorType, IIOSensorProxyLightSensor::id, this);
            if (!QSensorManager::isBackendRegistered(QCompass::sensorType, IIOSensorProxyCompass::id))
                QSensorManager::registerBackend(QCompass::sensorType, IIOSensorProxyCompass::id, this);
            if (!QSensorManager::isBackendRegistered(QProximitySensor::sensorType, IIOSensorProxyProximitySensor::id))
                QSensorManager::registerBackend(QProximitySensor::sensorType, IIOSensorProxyProximitySensor::id, this);
        }
    }

    QSensorBackend *createBackend(QSensor *sensor) override
    {
        if (sensor->identifier() == IIOSensorProxyOrientationSensor::id)
            return new IIOSensorProxyOrientationSensor(sensor);
        else if (sensor->identifier() == IIOSensorProxyLightSensor::id)
            return new IIOSensorProxyLightSensor(sensor);
        else if (sensor->identifier() == IIOSensorProxyCompass::id)
            return new IIOSensorProxyCompass(sensor);
        else if (sensor->identifier() == IIOSensorProxyProximitySensor::id)
            return new IIOSensorProxyProximitySensor(sensor);

        return 0;
    }
};

#include "main.moc"
