// Copyright (C) 2024 The Qt Company Ltd.
// Copyright (C) 2024 Florian Richer <florian.richer@protonmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxyproximitysensor.h"
#include "sensorproxy_interface.h"

#include <QtDBus/QDBusPendingReply>

using namespace QtSensorsPrivate;

char const * const IIOSensorProxyProximitySensor::id("iio-sensor-proxy.proximitysensor");

static inline QString dbusPath() { return QStringLiteral("/net/hadess/SensorProxy"); }

IIOSensorProxyProximitySensor::IIOSensorProxyProximitySensor(QSensor *sensor)
    : IIOSensorProxySensorBase(dbusPath(), NetHadessSensorProxyInterface::staticInterfaceName(), sensor)
{
    setReading<QProximityReading>(&m_reading);
    m_sensorProxyInterface = new NetHadessSensorProxyInterface(serviceName(), dbusPath(),
                                                               QDBusConnection::systemBus(), this);
}

IIOSensorProxyProximitySensor::~IIOSensorProxyProximitySensor()
{
}

void IIOSensorProxyProximitySensor::start()
{
    if (isServiceRunning()) {
        if (m_sensorProxyInterface->hasProximity()) {
            QDBusPendingReply<> reply = m_sensorProxyInterface->ClaimProximity();
            reply.waitForFinished();
            if (!reply.isError()) {
                updateProximityNear(m_sensorProxyInterface->proximityNear());
                return;
            }
        }
    }
    sensorStopped();
}

void IIOSensorProxyProximitySensor::stop()
{
    if (isServiceRunning()) {
        QDBusPendingReply<> reply = m_sensorProxyInterface->ReleaseProximity();
        reply.waitForFinished();
    }
    sensorStopped();
}

void IIOSensorProxyProximitySensor::updateProperties(const QVariantMap &changedProperties)
{
    if (changedProperties.contains("ProximityNear")) {
        bool proximityNear = changedProperties.value("ProximityNear").toBool();
        updateProximityNear(proximityNear);
    }
}

void IIOSensorProxyProximitySensor::updateProximityNear(bool proximityNear)
{
    m_reading.setClose(proximityNear);
    m_reading.setTimestamp(produceTimestamp());
    newReadingAvailable();
}
