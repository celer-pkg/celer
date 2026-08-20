// Copyright (C) 2018 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosource_geoclue2_p.h"
#include "qgeopositioninfosourcefactory_geoclue2.h"

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(lcPositioningGeoclue2, "qt.positioning.geoclue2")

QT_BEGIN_NAMESPACE

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryGeoclue2::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    return new QGeoPositionInfoSourceGeoclue2(parameters, parent);
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryGeoclue2::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryGeoclue2::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QT_END_NAMESPACE

#include "moc_qgeopositioninfosourcefactory_geoclue2.cpp"
