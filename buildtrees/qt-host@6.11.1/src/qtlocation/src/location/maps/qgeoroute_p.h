// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTE_P_H
#define QGEOROUTE_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include "qgeoroute.h"
#include "qgeorouterequest.h"
#include "qgeorectangle.h"
#include "qgeoroutesegment.h"

#include <QSharedData>
#include <QVariantMap>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QGeoCoordinate;

class Q_LOCATION_EXPORT QGeoRoutePrivate : public QSharedData
{
public:
    bool operator==(const QGeoRoutePrivate &other) const;
    bool equals(const QGeoRoutePrivate &other) const;

    void setId(const QString &id);
    QString id() const;

    void setRequest(const QGeoRouteRequest &request);
    QGeoRouteRequest request() const;

    void setBounds(const QGeoRectangle &bounds);
    QGeoRectangle bounds() const;

    void setTravelTime(int travelTime);
    int travelTime() const;

    void setDistance(qreal distance);
    qreal distance() const;

    void setTravelMode(QGeoRouteRequest::TravelMode mode);
    QGeoRouteRequest::TravelMode travelMode() const;

    void setPath(const QList<QGeoCoordinate> &path);
    QList<QGeoCoordinate> path() const;

    void setFirstSegment(const QGeoRouteSegment &firstSegment);
    QGeoRouteSegment firstSegment() const;

    int segmentsCount() const;
    QList<QGeoRouteSegment> segments() const;

    void setRouteLegs(const QList<QGeoRoute> &legs);
    QList<QGeoRoute> routeLegs() const;

    void setExtendedAttributes(const QVariantMap &extendedAttributes);
    QVariantMap extendedAttributes() const;

    // QGeoRouteLeg API
    void setLegIndex(int idx);
    int legIndex() const;
    void setContainingRoute(const QGeoRoute &route);
    QGeoRoute containingRoute() const;

private:
    template<typename Functor>
    inline void forEachSegment(Functor &&functor) const {
        QGeoRouteSegment segment = m_firstSegment;
        while (segment.isValid()) {
            functor(segment);
            // if containing route, this is a leg
            if (segment.isLegLastSegment() && m_containingRoute)
                break;
            segment = segment.nextRouteSegment();
        }
    }
    QString m_id;
    QGeoRouteRequest m_request;

    QGeoRectangle m_bounds;
    mutable QList<QGeoRouteSegment> m_routeSegments;

    int m_travelTime = 0;
    qreal m_distance = 0.0;

    QGeoRouteRequest::TravelMode m_travelMode;

    QList<QGeoCoordinate> m_path;
    QList<QGeoRoute> m_legs;
    QGeoRouteSegment m_firstSegment;
    mutable int m_numSegments = -1;
    std::unique_ptr<QGeoRoute> m_containingRoute;
    QVariantMap m_extendedAttributes;
    int m_legIndex = -1;
};

QT_END_NAMESPACE

#endif
