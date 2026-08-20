// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosource_wasm_p.h"
#include "qgeopositioninfosourcefactory_wasm.h"

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryWasm::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    if (QGeoPositionInfoSourceWasm::geolocation().isUndefined())
        return nullptr;
    return new QGeoPositionInfoSourceWasm(parent);
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryWasm::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryWasm::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

