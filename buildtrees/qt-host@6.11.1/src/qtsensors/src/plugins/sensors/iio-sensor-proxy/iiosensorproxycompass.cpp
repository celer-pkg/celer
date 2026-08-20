// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxycompass.h"
#include "compass_interface.h"

#include <QtDBus/QDBusPendingReply>

using namespace QtSensorsPrivate;

char const * const IIOSensorProxyCompass::id("iio-sensor-proxy.compass");

static inline QString dbusPath() { return QStringLiteral("/net/hadess/SensorProxy/Compass"); }

IIOSensorProxyCompass::IIOSensorProxyCompass(QSensor *sensor)
    : IIOSensorProxySensorBase(dbusPath(), NetHadessSensorProxyCompassInterface::staticInterfaceName(), sensor)
{
    setReading<QCompassReading>(&m_reading);
    m_sensorProxyInterface = new NetHadessSensorProxyCompassInterface(serviceName(), dbusPath(),
                                                                      QDBusConnection::systemBus(), this);
}

IIOSensorProxyCompass::~IIOSensorProxyCompass()
{
}

void IIOSensorProxyCompass::start()
{
    if (isServiceRunning()) {
        if (m_sensorProxyInterface->hasCompass()) {
            QDBusPendingReply<> reply = m_sensorProxyInterface->ClaimCompass();
            reply.waitForFinished();
            if (!reply.isError()) {
                double azimuth = m_sensorProxyInterface->compassHeading();
                updateAzimuth(azimuth);
                return;
            }
        }
    }
    sensorStopped();
}

void IIOSensorProxyCompass::stop()
{
    if (isServiceRunning()) {
        QDBusPendingReply<> reply = m_sensorProxyInterface->ReleaseCompass();
        reply.waitForFinished();
    }
    sensorStopped();
}

void IIOSensorProxyCompass::updateProperties(const QVariantMap &changedProperties)
{
    if (changedProperties.contains("CompassHeading")) {
        double azimuth = changedProperties.value("CompassHeading").toDouble();
        updateAzimuth(azimuth);
    }
}

void IIOSensorProxyCompass::updateAzimuth(double azimuth)
{
    m_reading.setAzimuth(azimuth);
    m_reading.setTimestamp(produceTimestamp());
    newReadingAvailable();
}
