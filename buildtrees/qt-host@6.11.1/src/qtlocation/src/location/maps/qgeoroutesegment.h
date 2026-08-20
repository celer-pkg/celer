// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTESEGMENT_H
#define QGEOROUTESEGMENT_H

#include <QtCore/QObject>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QList>
#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QGeoCoordinate;
class QGeoManeuver;
class QGeoRouteSegmentPrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoRouteSegmentPrivate, Q_LOCATION_EXPORT)
class Q_LOCATION_EXPORT QGeoRouteSegment
{
    Q_GADGET
    QML_VALUE_TYPE(routeSegment)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(int travelTime READ travelTime CONSTANT)
    Q_PROPERTY(qreal distance READ distance CONSTANT)
    Q_PROPERTY(QList<QGeoCoordinate> path READ path CONSTANT)
    Q_PROPERTY(QGeoManeuver maneuver READ maneuver CONSTANT)
public:
    QGeoRouteSegment();
    QGeoRouteSegment(const QGeoRouteSegment &other) noexcept;
    QGeoRouteSegment(QGeoRouteSegment &&other) noexcept = default;
    ~QGeoRouteSegment();

    QGeoRouteSegment &operator=(const QGeoRouteSegment &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoRouteSegment)

    void swap(QGeoRouteSegment &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator ==(const QGeoRouteSegment &lhs, const QGeoRouteSegment &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator !=(const QGeoRouteSegment &lhs, const QGeoRouteSegment &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    bool isValid() const;
    bool isLegLastSegment() const;

    void setNextRouteSegment(const QGeoRouteSegment &routeSegment);
    QGeoRouteSegment nextRouteSegment() const;

    void setTravelTime(int secs);
    int travelTime() const;

    void setDistance(qreal distance);
    qreal distance() const;

    void setPath(const QList<QGeoCoordinate> &path);
    QList<QGeoCoordinate> path() const;

    void setManeuver(const QGeoManeuver &maneuver);
    QGeoManeuver maneuver() const;

private:
    QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> d_ptr;
    QGeoRouteSegment(QExplicitlySharedDataPointer<QGeoRouteSegmentPrivate> &&dd);

    bool isEqual(const QGeoRouteSegment &other) const noexcept;

    friend class QGeoRouteSegmentPrivate;
};

QT_END_NAMESPACE

#endif
