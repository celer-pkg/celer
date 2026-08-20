// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxyorientationsensor.h"
#include "sensorproxy_interface.h"

#include <QtDBus/QDBusPendingReply>

using namespace QtSensorsPrivate;

char const * const IIOSensorProxyOrientationSensor::id("iio-sensor-proxy.orientationsensor");

static inline QString dbusPath() { return QStringLiteral("/net/hadess/SensorProxy"); }

IIOSensorProxyOrientationSensor::IIOSensorProxyOrientationSensor(QSensor *sensor)
    : IIOSensorProxySensorBase(dbusPath(), NetHadessSensorProxyInterface::staticInterfaceName(), sensor)
{
    setReading<QOrientationReading>(&m_reading);
    m_sensorProxyInterface = new NetHadessSensorProxyInterface(serviceName(), dbusPath(),
                                                               QDBusConnection::systemBus(), this);
}

IIOSensorProxyOrientationSensor::~IIOSensorProxyOrientationSensor()
{
}

void IIOSensorProxyOrientationSensor::start()
{
    if (isServiceRunning()) {
        if (m_sensorProxyInterface->hasAccelerometer()) {
            QDBusPendingReply<> reply = m_sensorProxyInterface->ClaimAccelerometer();
            reply.waitForFinished();
            if (!reply.isError()) {
                QString orientation = m_sensorProxyInterface->accelerometerOrientation();
                updateOrientation(orientation);
                return;
            }
        }
    }
    sensorStopped();
}

void IIOSensorProxyOrientationSensor::stop()
{
    if (isServiceRunning()) {
        QDBusPendingReply<> reply = m_sensorProxyInterface->ReleaseAccelerometer();
        reply.waitForFinished();
    }
    sensorStopped();
}

void IIOSensorProxyOrientationSensor::updateProperties(const QVariantMap &changedProperties)
{
    if (changedProperties.contains("AccelerometerOrientation")) {
        QString orientation = changedProperties.value("AccelerometerOrientation").toString();
        updateOrientation(orientation);
    }
}

void IIOSensorProxyOrientationSensor::updateOrientation(const QString &orientation)
{
    QOrientationReading::Orientation o = QOrientationReading::Undefined;
    if (orientation == QLatin1String("normal"))
        o = QOrientationReading::TopUp;
    else if (orientation == QLatin1String("bottom-up"))
        o = QOrientationReading::TopDown;
    else if (orientation == QLatin1String("left-up"))
        o = QOrientationReading::LeftUp;
    else if (orientation == QLatin1String("right-up"))
        o = QOrientationReading::RightUp;

    m_reading.setOrientation(o);
    m_reading.setTimestamp(produceTimestamp());
    newReadingAvailable();
}
