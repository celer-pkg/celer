// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LIGHTNINGITEMDATA_H
#define LIGHTNINGITEMDATA_H

#include <QGeoCoordinate>

struct LightningItemData
{
    std::chrono::duration<int> timestamp;
    double latitude;
    double longitude;

    double getDistanceTo(const LightningItemData &other) const;
    double getDistanceTo(const QGeoCoordinate &coordinate) const;
    double getDirectionTo(const QGeoCoordinate &coordinate) const;
    double getDirectionFrom(const QGeoCoordinate &coordinate) const;
};

#endif // LIGHTNINGITEMDATA_H
