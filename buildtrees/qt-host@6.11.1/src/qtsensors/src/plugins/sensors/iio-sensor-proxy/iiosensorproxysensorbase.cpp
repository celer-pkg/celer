// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iiosensorproxysensorbase.h"
#include "sensorproxy_interface.h"
#include "properties_interface.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusConnectionInterface>

#include <time.h>

using namespace QtSensorsPrivate;

quint64 IIOSensorProxySensorBase::produceTimestamp()
{
    struct timespec tv;
    int ok;

#ifdef CLOCK_MONOTONIC_RAW
    ok = clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
    if (ok != 0)
#endif
    ok = clock_gettime(CLOCK_MONOTONIC, &tv);
    Q_ASSERT(ok == 0);

    quint64 result = (tv.tv_sec * 1000000ULL) + (tv.tv_nsec * 0.001); // scale to microseconds
    return result;
}

IIOSensorProxySensorBase::IIOSensorProxySensorBase(const QString& dbusPath, const QString dbusIface, QSensor *sensor)
    : QSensorBackend(sensor)
    , m_dbusInterface(dbusIface)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(serviceName(), QDBusConnection::systemBus(),
                                                           QDBusServiceWatcher::WatchForRegistration |
                                                           QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(serviceRegistered()));
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(serviceUnregistered()));

    m_serviceRunning = QDBusConnection::systemBus().interface()->isServiceRegistered(serviceName());

    m_propertiesInterface = new OrgFreedesktopDBusPropertiesInterface(serviceName(), dbusPath,
                                                                      QDBusConnection::systemBus(), this);
    connect(m_propertiesInterface, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
            this, SLOT(propertiesChanged(QString,QVariantMap,QStringList)));
}

IIOSensorProxySensorBase::~IIOSensorProxySensorBase()
{
}

QString IIOSensorProxySensorBase::serviceName() const
{
    return QLatin1String("net.hadess.SensorProxy");
}

void IIOSensorProxySensorBase::serviceRegistered()
{
    m_serviceRunning = true;
}

void IIOSensorProxySensorBase::serviceUnregistered()
{
    m_serviceRunning = false;
    sensorStopped();
}

void IIOSensorProxySensorBase::propertiesChanged(const QString &interface,
                                                 const QVariantMap &changedProperties,
                                                 const QStringList &/*invalidatedProperties*/)
{
    if (interface == m_dbusInterface)
        updateProperties(changedProperties);
}
