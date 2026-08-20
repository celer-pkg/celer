// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qwebmercator_p.h"

#include "qgeocoordinate.h"

#include <qnumeric.h>
#include <qmath.h>

#include "qdoublevector2d_p.h"

QT_BEGIN_NAMESPACE
// We reverse the y-direction to make North be its decreasing direction, which
// is "up" in computer graphics. We offset it by 0.5 to put the range -85 deg to
// +85 deg latitude into the unit interval. The x-coordinate maps the range from
// -180 deg to +180 deg longitude into its unit interval, with The Prime
// Meridian mapped to x = 0.5. The x-coordinate is formally periodic, with
// period 1, so adding or subtracting any whole number gets a representation of
// the same longitude.

// As latitudes approach ±90, the Mercator y-Coordinate aproaches ∓infinity. We
// limit that to avoid numerical problems. Here the limit, -4 <= y <= 5, is
// chosen such that only points beyond ±89.9999999999 deg latituide are cut off,
// which matches the accuracy of qFuzzyCompare()
const static double yCutOff = 4.0;

QDoubleVector2D QWebMercator::coordToMercator(const QGeoCoordinate &coord)
{
    const double x = coord.longitude() / 360.0 + 0.5;
    const double lat = coord.latitude();
    const double y = (1.0 - std::log(std::tan(qDegreesToRadians((90.0 + lat) / 2.0))) / M_PI) / 2.0;
    return QDoubleVector2D(x, qBound(-yCutOff, y, 1.0 + yCutOff));
}

double QWebMercator::realmod(const double a, const double b)
{
    quint64 div = static_cast<quint64>(a / b);
    return a - static_cast<double>(div) * b;
}

QGeoCoordinate QWebMercator::mercatorToCoord(const QDoubleVector2D &mercator)
{
    double fx = mercator.x();
    const double fy = mercator.y();

    double lat;
    if (fy <= -yCutOff)
        lat = 90.0;
    else if (fy >= 1.0 + yCutOff)
        lat = -90.0;
    else
        lat = qRadiansToDegrees(2.0 * std::atan(std::exp(M_PI * (1.0 - 2.0 * fy)))) - 90.0;

    if (fx < 0) // Map to the +ve unit interval:
        fx = 1.0 - realmod(-fx, 1.0);
    const double lng = realmod(fx, 1.0) * 360.0 - 180.0;

    return QGeoCoordinate(lat, lng, 0.0);
}

QGeoCoordinate QWebMercator::coordinateInterpolation(const QGeoCoordinate &from,
                                                     const QGeoCoordinate &to, qreal progress)
{
    QDoubleVector2D s = QWebMercator::coordToMercator(from);
    QDoubleVector2D e = QWebMercator::coordToMercator(to);

    double x;

    if (0.5 < qAbs(e.x() - s.x())) {
        // handle dateline crossing
        double ex = e.x();
        double sx = s.x();
        if (ex < sx)
            sx -= 1.0;
        else if (sx < ex)
            ex -= 1.0;

        x = (1.0 - progress) * sx + progress * ex;

        if (!qFuzzyIsNull(x) && (x < 0.0))
            x += 1.0;

    } else {
        x = (1.0 - progress) * s.x() + progress * e.x();
    }

    double y = (1.0 - progress) * s.y() + progress * e.y();

    QGeoCoordinate result = QWebMercator::mercatorToCoord(QDoubleVector2D(x, y));
    result.setAltitude((1.0 - progress) * from.altitude() + progress * to.altitude());

    return result;
}

QT_END_NAMESPACE
