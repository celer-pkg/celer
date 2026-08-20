// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTE_H
#define QGEOROUTE_H

#include <QtLocation/QGeoRouteRequest>

#include <QtCore/QObject>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QList>

#include "QtQml/qqml.h"

QT_BEGIN_NAMESPACE

class QGeoCoordinate;
class QGeoRectangle;
class QGeoRouteSegment;

class QGeoRoutePrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoRoutePrivate, Q_LOCATION_EXPORT)
class Q_LOCATION_EXPORT QGeoRoute
{
    Q_GADGET
    QML_VALUE_TYPE(route)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(QString routeId READ routeId CONSTANT)
    Q_PROPERTY(QGeoRectangle bounds READ bounds CONSTANT)
    Q_PROPERTY(int travelTime READ travelTime CONSTANT)
    Q_PROPERTY(qreal distance READ distance CONSTANT)
    Q_PROPERTY(QList<QGeoCoordinate> path READ path WRITE setPath)
    Q_PROPERTY(QList<QGeoRoute> routeLegs READ routeLegs CONSTANT)
    Q_PROPERTY(QVariantMap extendedAttributes READ extendedAttributes CONSTANT)
    Q_PROPERTY(int legIndex READ legIndex CONSTANT)
    Q_PROPERTY(QGeoRoute overallRoute READ overallRoute CONSTANT)
    Q_PROPERTY(qsizetype segmentsCount READ segmentsCount CONSTANT)
    Q_PROPERTY(QList<QGeoRouteSegment> segments READ segments CONSTANT)

public:
    QGeoRoute();
    QGeoRoute(const QGeoRoute &other) noexcept;
    QGeoRoute(QGeoRoute &&other) noexcept = default;
    ~QGeoRoute();

    QGeoRoute &operator=(const QGeoRoute &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoRoute)

    void swap(QGeoRoute &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QGeoRoute &lhs, const QGeoRoute &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QGeoRoute &lhs, const QGeoRoute &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    void setRouteId(const QString &id);
    QString routeId() const;

    void setRequest(const QGeoRouteRequest &request);
    QGeoRouteRequest request() const;

    void setBounds(const QGeoRectangle &bounds);
    QGeoRectangle bounds() const;

    void setFirstRouteSegment(const QGeoRouteSegment &routeSegment);
    QGeoRouteSegment firstRouteSegment() const;
    qsizetype segmentsCount() const;
    QList<QGeoRouteSegment> segments() const;

    void setTravelTime(int secs);
    int travelTime() const;

    void setDistance(qreal distance);
    qreal distance() const;

    void setTravelMode(QGeoRouteRequest::TravelMode mode);
    QGeoRouteRequest::TravelMode travelMode() const;

    void setPath(const QList<QGeoCoordinate> &path);
    QList<QGeoCoordinate> path() const;

    void setRouteLegs(const QList<QGeoRoute> &legs);
    QList<QGeoRoute> routeLegs() const;

    void setExtendedAttributes(const QVariantMap &extendedAttributes);
    QVariantMap extendedAttributes() const;

    void setLegIndex(int idx);
    int legIndex() const;

    void setOverallRoute(const QGeoRoute &route);
    QGeoRoute overallRoute() const;

protected:
    QExplicitlySharedDataPointer<QGeoRoutePrivate> &d();
    const QExplicitlySharedDataPointer<QGeoRoutePrivate> &const_d() const;

private:
    QExplicitlySharedDataPointer<QGeoRoutePrivate> d_ptr;
    bool isEqual(const QGeoRoute &other) const noexcept;

    friend class QDeclarativeGeoRoute;
    friend class QGeoRoutePrivate;
};

QT_END_NAMESPACE

#endif
