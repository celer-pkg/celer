// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IIOSENSORPROXY_SENSORBASE_H
#define IIOSENSORPROXY_SENSORBASE_H

#include <qsensorbackend.h>

namespace QtSensorsPrivate {

class OrgFreedesktopDBusPropertiesInterface;

} // namespace QtSensorsPrivate

class IIOSensorProxySensorBase : public QSensorBackend
{
    Q_OBJECT
public:
    static char const * const id;

    IIOSensorProxySensorBase(const QString &dbusPath, const QString dbusIface, QSensor *sensor);
    ~IIOSensorProxySensorBase();

    bool isServiceRunning() const { return m_serviceRunning; }
    QString serviceName() const;

protected:
    static quint64 produceTimestamp();
    virtual void updateProperties(const QVariantMap &changedProperties) = 0;

private slots:
    void serviceRegistered();
    void serviceUnregistered();
    void propertiesChanged(const QString &interface, const QVariantMap &changedProperties,
                           const QStringList &invalidatedProperties);

private:
    bool m_serviceRunning;
    QtSensorsPrivate::OrgFreedesktopDBusPropertiesInterface *m_propertiesInterface;
    QString m_dbusInterface;
};

#endif // IIOSENSORPROXY_SENSORBASE_H
