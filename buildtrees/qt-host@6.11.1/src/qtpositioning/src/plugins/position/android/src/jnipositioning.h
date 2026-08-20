// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JNIPOSITIONING_H
#define JNIPOSITIONING_H

#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>

namespace AndroidPositioning
{
    int registerPositionInfoSource(QObject *obj);
    void unregisterPositionInfoSource(int key);

    QGeoPositionInfoSource::PositioningMethods availableProviders();
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly,
                                       bool useAltitudeConverter);

    QGeoPositionInfoSource::Error startUpdates(int androidClassKey);
    void stopUpdates(int androidClassKey);
    QGeoPositionInfoSource::Error requestUpdate(int androidClassKey, int timeout);

    QGeoSatelliteInfoSource::Error startSatelliteUpdates(int androidClassKey,
                                                         bool isSingleRequest,
                                                         int updateRequestTimeout);

    // This basically mimics QGeoPositionInfoSource::PositioningMethods,
    // but we introduce a separate enum, because it's also used when accessing
    // QGeoSatelliteInfoSouce.
    enum class AccuracyType : quint8
    {
        None = 0x00,
        Precise = 0x01,
        Approximate = 0x02,
        Any = 0xFF,
    };
    Q_DECLARE_FLAGS(AccuracyTypes, AccuracyType)

    bool hasPositioningPermissions(AccuracyTypes accuracy);
}

#endif // JNIPOSITIONING_H
