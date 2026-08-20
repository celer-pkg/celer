// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEREQUEST_P_H
#define QGEOROUTEREQUEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qgeorouterequest.h"

#include <QList>
#include <QSharedData>
#include <QDateTime>
#include <QMap>
#include <QVariantMap>

QT_BEGIN_NAMESPACE

class QGeoRouteRequestPrivate : public QSharedData
{
public:
    bool operator==(const QGeoRouteRequestPrivate &other) const;

    QList<QGeoCoordinate> waypoints;
    QList<QGeoRectangle> excludeAreas;
    int numberAlternativeRoutes = 0;
    QGeoRouteRequest::TravelModes travelModes = QGeoRouteRequest::CarTravel;
    QMap <QGeoRouteRequest::FeatureType,
          QGeoRouteRequest::FeatureWeight> featureWeights;
    QGeoRouteRequest::RouteOptimizations routeOptimization = QGeoRouteRequest::FastestRoute;
    QGeoRouteRequest::SegmentDetail segmentDetail = QGeoRouteRequest::BasicSegmentData;
    QGeoRouteRequest::ManeuverDetail maneuverDetail = QGeoRouteRequest::BasicManeuvers;
    QDateTime departureTime;
};

QT_END_NAMESPACE

#endif
