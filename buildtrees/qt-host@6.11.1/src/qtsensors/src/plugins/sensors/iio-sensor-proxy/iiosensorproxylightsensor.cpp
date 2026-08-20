// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxylightsensor.h"
#include "sensorproxy_interface.h"

#include <QtDBus/QDBusPendingReply>

using namespace QtSensorsPrivate;

char const * const IIOSensorProxyLightSensor::id("iio-sensor-proxy.lightsensor");

static inline QString dbusPath() { return QStringLiteral("/net/hadess/SensorProxy"); }

IIOSensorProxyLightSensor::IIOSensorProxyLightSensor(QSensor *sensor)
    : IIOSensorProxySensorBase(dbusPath(), NetHadessSensorProxyInterface::staticInterfaceName(), sensor)
{
    setReading<QLightReading>(&m_reading);
    m_sensorProxyInterface = new NetHadessSensorProxyInterface(serviceName(), dbusPath(),
                                                               QDBusConnection::systemBus(), this);
}

IIOSensorProxyLightSensor::~IIOSensorProxyLightSensor()
{
}

void IIOSensorProxyLightSensor::start()
{
    if (isServiceRunning()) {
        if (m_sensorProxyInterface->hasAmbientLight()
            && m_sensorProxyInterface->lightLevelUnit() == QLatin1String("lux")) {
            QDBusPendingReply<> reply = m_sensorProxyInterface->ClaimLight();
            reply.waitForFinished();
            if (!reply.isError()) {
                updateLightLevel(m_sensorProxyInterface->lightLevel());
                return;
            }
        }
    }
    sensorStopped();
}

void IIOSensorProxyLightSensor::stop()
{
    if (isServiceRunning()) {
        QDBusPendingReply<> reply = m_sensorProxyInterface->ReleaseLight();
        reply.waitForFinished();
    }
    sensorStopped();
}

void IIOSensorProxyLightSensor::updateProperties(const QVariantMap &changedProperties)
{
    if (changedProperties.contains("LightLevel")) {
        double lux = changedProperties.value("LightLevel").toDouble();
        updateLightLevel(lux);
    }
}

void IIOSensorProxyLightSensor::updateLightLevel(double lux)
{
    m_reading.setLux(lux);
    m_reading.setTimestamp(produceTimestamp());
    newReadingAvailable();
}
