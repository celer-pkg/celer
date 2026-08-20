// Copyright (C) 2016 Alexander Volkov <a.volkov@rusbitech.ru>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef IIOSENSORPROXY_COMPASS_H
#define IIOSENSORPROXY_COMPASS_H

#include "iiosensorproxysensorbase.h"

#include <qcompass.h>

namespace QtSensorsPrivate {

class NetHadessSensorProxyCompassInterface;

} // namespace QtSensorsPrivate

class IIOSensorProxyCompass : public IIOSensorProxySensorBase
{
    Q_OBJECT
public:
    static char const * const id;

    IIOSensorProxyCompass(QSensor *sensor);
    ~IIOSensorProxyCompass();

    void start() override;
    void stop() override;

protected:
    void updateProperties(const QVariantMap &changedProperties) override;

private:
    void updateAzimuth(double azimuth);

    QCompassReading m_reading;
    QtSensorsPrivate::NetHadessSensorProxyCompassInterface *m_sensorProxyInterface;
};

#endif // IIOSENSORPROXY_COMPASS_H
