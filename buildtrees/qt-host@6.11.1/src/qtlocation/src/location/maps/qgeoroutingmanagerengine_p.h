// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTINGMANAGERENGINE_P_H
#define QGEOROUTINGMANAGERENGINE_P_H

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

#include <QMap>
#include <QLocale>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QGeoRoutingManagerEnginePrivate
{
public:
    QString managerName;
    int managerVersion = -1;
    QGeoRouteRequest::TravelModes supportedTravelModes;
    QGeoRouteRequest::FeatureTypes supportedFeatureTypes;
    QGeoRouteRequest::FeatureWeights supportedFeatureWeights;
    QGeoRouteRequest::RouteOptimizations supportedRouteOptimizations;
    QGeoRouteRequest::SegmentDetails supportedSegmentDetails;
    QGeoRouteRequest::ManeuverDetails supportedManeuverDetails;

    QLocale locale;
    QLocale::MeasurementSystem measurementSystem = locale.measurementSystem();
};

QT_END_NAMESPACE

#endif
