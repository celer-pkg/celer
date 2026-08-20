// Copyright (C) 2024 The Qt Company Ltd.
// Copyright (C) 2024 Florian Richer <florian.richer@protonmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IIOSENSORPROXY_PROXIMITYSENSOR_H
#define IIOSENSORPROXY_PROXIMITYSENSOR_H

#include "iiosensorproxysensorbase.h"

#include <qproximitysensor.h>

namespace QtSensorsPrivate {

class NetHadessSensorProxyInterface;

} // namespace QtSensorsPrivate

class IIOSensorProxyProximitySensor : public IIOSensorProxySensorBase
{
    Q_OBJECT
public:
    static char const * const id;

    IIOSensorProxyProximitySensor(QSensor *sensor);
    ~IIOSensorProxyProximitySensor();

    void start() override;
    void stop() override;

protected:
    void updateProperties(const QVariantMap &changedProperties) override;

private:
    void updateProximityNear(bool proximityNear);

    QProximityReading m_reading;
    QtSensorsPrivate::NetHadessSensorProxyInterface *m_sensorProxyInterface;
};

#endif // IIOSENSORPROXY_PROXIMITYSENSOR_H
