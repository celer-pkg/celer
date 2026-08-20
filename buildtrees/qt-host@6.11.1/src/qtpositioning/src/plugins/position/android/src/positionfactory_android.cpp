// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "positionfactory_android.h"
#include "qgeopositioninfosource_android_p.h"
#include "qgeosatelliteinfosource_android_p.h"

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryAndroid::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    QGeoPositionInfoSourceAndroid *src = new QGeoPositionInfoSourceAndroid(parameters, parent);
    return src;
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryAndroid::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    QGeoSatelliteInfoSourceAndroid *src = new QGeoSatelliteInfoSourceAndroid(parent);
    return src;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryAndroid::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}
