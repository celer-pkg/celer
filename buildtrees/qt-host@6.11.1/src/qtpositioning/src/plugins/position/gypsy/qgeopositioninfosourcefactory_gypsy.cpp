// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosourcefactory_gypsy.h"
#include "qgeosatelliteinfosource_gypsy_p.h"

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryGypsy::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryGypsy::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    QGeoSatelliteInfoSourceGypsy *src = new QGeoSatelliteInfoSourceGypsy(parent);
    if (src->init(parameters) < 0) {
        delete src;
        src = nullptr;
    }
    return src;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryGypsy::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}
