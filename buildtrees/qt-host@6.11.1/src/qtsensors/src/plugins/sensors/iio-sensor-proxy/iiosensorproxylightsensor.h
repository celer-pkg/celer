// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IIOSENSORPROXY_LIGHTSENSOR_H
#define IIOSENSORPROXY_LIGHTSENSOR_H

#include "iiosensorproxysensorbase.h"

#include <qlightsensor.h>

namespace QtSensorsPrivate {

class NetHadessSensorProxyInterface;

} // namespace QtSensorsPrivate

class IIOSensorProxyLightSensor : public IIOSensorProxySensorBase
{
    Q_OBJECT
public:
    static char const * const id;

    IIOSensorProxyLightSensor(QSensor *sensor);
    ~IIOSensorProxyLightSensor();

    void start() override;
    void stop() override;

protected:
    void updateProperties(const QVariantMap &changedProperties) override;

private:
    void updateLightLevel(double lux);

    QLightReading m_reading;
    QtSensorsPrivate::NetHadessSensorProxyInterface *m_sensorProxyInterface;
};

#endif // IIOSENSORPROXY_LIGHTSENSOR_H
