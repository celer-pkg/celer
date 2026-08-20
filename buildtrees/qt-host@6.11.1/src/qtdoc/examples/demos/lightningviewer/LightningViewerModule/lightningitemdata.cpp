// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "lightningitemdata.h"

double LightningItemData::getDistanceTo(const LightningItemData &other) const
{
    return other.getDistanceTo(QGeoCoordinate{latitude, longitude});
}

double LightningItemData::getDistanceTo(const QGeoCoordinate &coordinate) const
{
    return coordinate.distanceTo(QGeoCoordinate{latitude, longitude});
}

double LightningItemData::getDirectionTo(const QGeoCoordinate &coordinate) const
{
    return QGeoCoordinate{latitude, longitude}.azimuthTo(coordinate);
}

double LightningItemData::getDirectionFrom(const QGeoCoordinate &coordinate) const
{
    return coordinate.azimuthTo(QGeoCoordinate{latitude, longitude});
}

