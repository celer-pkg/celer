// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickgeocoordinateanimation_p.h"
#include "qquickgeocoordinateanimation_p_p.h"
#include <QtQuick/private/qquickanimation_p_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qgeocoordinate_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype CoordinateAnimation
    \inherits PropertyAnimation
    \inqmlmodule QtPositioning
    \since 5.3

    \brief A PropertyAnimation for geo coordinate properties.

    A specialized \l{PropertyAnimation} that defines an animation
    between two \l{coordinate}{coordinates}.

    By default, a \l{latitude} of the \l{coordinate} is animated in the direction of shortest
    (geodesic) distance between those coordinates. Since CoordinateAnimation uses Mercator
    map projection, the \l{latitude} animation is always between -90 and 90 degrees.
    The \l{longitude} animation path is not limited and can go over 180 degrees
    in both west and east directions.

    The \l{direction} property can be set to specify the direction in which the \l{longitude}
    animation should occur.

    \sa {Animation and Transitions in Qt Quick}
*/

QVariant q_coordinateInterpolator(const QGeoCoordinate &from, const QGeoCoordinate &to, qreal progress)
{
    if (from == to) {
        if (progress < 0.5) {
            return QVariant::fromValue(from);
        } else {
            return QVariant::fromValue(to);
        }
    }

    QGeoCoordinate result = QWebMercator::coordinateInterpolation(from, to, progress);

    return QVariant::fromValue(result);
}

QVariant q_coordinateShortestInterpolator(const QGeoCoordinate &from, const QGeoCoordinate &to, qreal progress)
{
    const QGeoMercatorCoordinatePrivate* fromMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&from));
    const QGeoMercatorCoordinatePrivate* toMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&to));

    double toX = toMercator->m_mercatorX;
    double toY = toMercator->m_mercatorY;
    double fromX = fromMercator->m_mercatorX;
    double fromY = fromMercator->m_mercatorY;
    double x;
    if (0.5 < qAbs(toX - fromX)) {
        // handle dateline crossing
        double ex = toX;
        double sx = fromX;
        if (ex < sx)
            sx -= 1.0;
        else if (sx < ex)
            ex -= 1.0;

        x = sx + (ex - sx) * progress;

        if (x < 0.0)
            x += 1.0;

    } else {
        x = fromX + (toX - fromX) * progress;
    }

    double y = fromY + (toY - fromY) * progress;

    QGeoCoordinate result = QWebMercator::mercatorToCoord(QDoubleVector2D(x, y));
    result.setAltitude(from.altitude() + (to.altitude() - from.altitude()) * progress);
    return QVariant::fromValue(result);
}

QVariant q_coordinateEastInterpolator(const QGeoCoordinate &from, const QGeoCoordinate &to, qreal progress)
{
    const QGeoMercatorCoordinatePrivate* fromMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&from));
    const QGeoMercatorCoordinatePrivate* toMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&to));

    double toX = toMercator->m_mercatorX;
    double toY = toMercator->m_mercatorY;
    double fromX = fromMercator->m_mercatorX;
    double fromY = fromMercator->m_mercatorY;
    double diff = toX - fromX;

    while (diff < 0.0) {
        toX += 1.0;
        diff += 1.0;
    }

    double x = fromX + (toX - fromX) * progress;
    double y = fromY + (toY - fromY) * progress;

    while (x > 1.0)
        x -= 1.0;

    QGeoCoordinate result = QWebMercator::mercatorToCoord(QDoubleVector2D(x, y));
    result.setAltitude(from.altitude() + (to.altitude() - from.altitude()) * progress);

    return QVariant::fromValue(result);
}

QVariant q_coordinateWestInterpolator(const QGeoCoordinate &from, const QGeoCoordinate &to, qreal progress)
{
    const QGeoMercatorCoordinatePrivate* fromMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&from));
    const QGeoMercatorCoordinatePrivate* toMercator =
            static_cast<const QGeoMercatorCoordinatePrivate*>(QGeoCoordinatePrivate::get(&to));

    double toX = toMercator->m_mercatorX;
    double toY = toMercator->m_mercatorY;
    double fromX = fromMercator->m_mercatorX;
    double fromY = fromMercator->m_mercatorY;
    double diff = toX - fromX;

    while (diff > 0.0) {
        toX -= 1.0;
        diff -= 1.0;
    }

    double x = fromX + (toX - fromX) * progress;
    double y = fromY + (toY - fromY) * progress;

    while (x < 0.0)
        x += 1.0;

    QGeoCoordinate result = QWebMercator::mercatorToCoord(QDoubleVector2D(x, y));
    result.setAltitude(from.altitude() + (to.altitude() - from.altitude()) * progress);

    return QVariant::fromValue(result);
}

QQuickGeoCoordinateAnimation::QQuickGeoCoordinateAnimation(QObject *parent)
    : QQuickPropertyAnimation(*(new QQuickGeoCoordinateAnimationPrivate), parent)

{
    Q_D(QQuickGeoCoordinateAnimation);
    d->interpolatorType = qMetaTypeId<QGeoCoordinate>();
    d->defaultToInterpolatorType = true;
    d->interpolator = QVariantAnimationPrivate::getInterpolator(d->interpolatorType);
}

QQuickGeoCoordinateAnimation::~QQuickGeoCoordinateAnimation()
{
}

/*!
    \qmlproperty coordinate CoordinateAnimation::from
    This property holds the coordinate where the animation should begin.
*/
QGeoCoordinate QQuickGeoCoordinateAnimation::from() const
{
    Q_D(const QQuickGeoCoordinateAnimation);
    return d->from.value<QGeoCoordinate>();
}

void QQuickGeoCoordinateAnimation::setFrom(const QGeoCoordinate &f)
{
    QGeoMercatorCoordinatePrivate *mercator = new QGeoMercatorCoordinatePrivate();
    QDoubleVector2D fromVector = QWebMercator::coordToMercator(f);
    mercator->lat = f.latitude();
    mercator->lng = f.longitude();
    mercator->alt = f.altitude();
    mercator->m_mercatorX = fromVector.x();
    mercator->m_mercatorY = fromVector.y();
    QGeoCoordinate from(*mercator);
    QQuickPropertyAnimation::setFrom(QVariant::fromValue(from));
}

/*!
    \qmlproperty coordinate CoordinateAnimation::to
    This property holds the coordinate where the animation should end.
*/
QGeoCoordinate QQuickGeoCoordinateAnimation::to() const
{
    Q_D(const QQuickGeoCoordinateAnimation);
    return d->to.value<QGeoCoordinate>();
}

void QQuickGeoCoordinateAnimation::setTo(const QGeoCoordinate &t)
{
    QGeoMercatorCoordinatePrivate *mercator = new QGeoMercatorCoordinatePrivate();
    QDoubleVector2D toVector = QWebMercator::coordToMercator(t);
    mercator->lat = t.latitude();
    mercator->lng = t.longitude();
    mercator->alt = t.altitude();
    mercator->m_mercatorX = toVector.x();
    mercator->m_mercatorY = toVector.y();
    QGeoCoordinate to(*mercator);
    QQuickPropertyAnimation::setTo(QVariant::fromValue(to));
}

/*!
    \qmlproperty enumeration CoordinateAnimation::direction

    This property holds the direction of the \l{longitude} animation of the \l{coordinate}.

    \value CoordinateAnimation.Shortest
        The longitude animation goes in the direction
        that produces the shortest animation path. This is the default.
    \value CoordinateAnimation.West
        The longitude animation always goes into western direction
        and may cross the date line.
    \value CoordinateAnimation.East
        The longitude animation always goes into eastern direction
        and may cross the date line.

    \since 5.5
*/


QQuickGeoCoordinateAnimation::Direction QQuickGeoCoordinateAnimation::direction() const
{
    Q_D(const QQuickGeoCoordinateAnimation);
    return d->m_direction.value();
}

void QQuickGeoCoordinateAnimation::setDirection(QQuickGeoCoordinateAnimation::Direction direction)
{
    Q_D( QQuickGeoCoordinateAnimation);
    d->m_direction.removeBindingUnlessInWrapper();
    if (d->m_direction.valueBypassingBindings() == direction)
        return;

    d->m_direction.setValueBypassingBindings(direction);
    switch (direction) {
    case West:
        d->interpolator = reinterpret_cast<QVariantAnimation::Interpolator>(reinterpret_cast<void *>(&q_coordinateWestInterpolator));
        break;
    case East:
        d->interpolator = reinterpret_cast<QVariantAnimation::Interpolator>(reinterpret_cast<void *>(&q_coordinateEastInterpolator));
        break;
    case Shortest:
    default:
        d->interpolator = reinterpret_cast<QVariantAnimation::Interpolator>(reinterpret_cast<void *>(&q_coordinateShortestInterpolator));
        break;
    }
    d->m_direction.notify();
}

QBindable<QQuickGeoCoordinateAnimation::Direction> QQuickGeoCoordinateAnimation::bindableDirection()
{
    Q_D(QQuickGeoCoordinateAnimation);
    return QBindable<Direction>(&d->m_direction);
}

QT_END_NAMESPACE

#include "moc_qquickgeocoordinateanimation_p.cpp"
