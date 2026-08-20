// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTESEGMENT_P_H
#define QGEOROUTESEGMENT_P_H

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
#include <QtLocation/qgeomaneuver.h>
#include <QtLocation/qgeoroutesegment.h>


#include <QSharedData>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

class QGeoCoordinate;

class Q_LOCATION_EXPORT QGeoRouteSegmentPrivate : public QSharedData
{
public:
    QGeoRouteSegmentPrivate();
    static QGeoRouteSegmentPrivate *get(QGeoRouteSegment &segment);

    bool valid() const;
    void setValid(bool valid);

    bool isLegLastSegment() const;
    void setLegLastSegment(bool lastSegment);

    int travelTime() const;
    void setTravelTime(int travelTime);

    qreal distance() const;
    void setDistance(qreal distance);

    QList<QGeoCoordinate> path() const;
    void setPath(const QList<QGeoCoordinate> &path);

    QGeoManeuver maneuver() const;
    void setManeuver(const QGeoManeuver &maneuver);

    QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> nextRouteSegment() const;
    void setNextRouteSegment(const QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &next);

    QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> m_nextSegment;
    bool m_valid = false;
    bool m_legLastSegment = false;
    int m_travelTime = 0;
    qreal m_distance = 0.0;
    QList<QGeoCoordinate> m_path;
    QGeoManeuver m_maneuver;

    friend bool operator==(const QGeoRouteSegmentPrivate &lhs, const QGeoRouteSegmentPrivate &rhs);
};

QT_END_NAMESPACE

#endif
