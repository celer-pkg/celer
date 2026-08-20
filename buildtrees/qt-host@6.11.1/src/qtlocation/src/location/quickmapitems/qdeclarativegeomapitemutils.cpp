// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapitemutils_p.h"

#include <QPointF>
#include <QMatrix4x4>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QtPositioning/QGeoCoordinate>

#include <QtPositioning/private/qclipperutils_p.h>

QT_BEGIN_NAMESPACE

namespace QDeclarativeGeoMapItemUtils {

double distanceSqrPointLine(double p0_x
                          , double p0_y
                          , double p1_x
                          , double p1_y
                          , double p2_x
                          , double p2_y)
{
    const double t_x = p2_x - p1_x;
    const double t_y = p2_y - p1_y;
    const double p_x = p0_x - p1_x;
    const double p_y = p0_y - p1_y;
    const double tsqr = t_x * t_x + t_y * t_y;

    if (tsqr == 0)
        return qInf();

    double alpha = (p_x * t_x + p_y * t_y) / tsqr;
    alpha = qBound<double>(0, alpha, 1);

    const double dx = p_x - t_x * alpha;
    const double dy = p_y - t_y * alpha;

    return dx * dx + dy * dy;
}

void wrapPath(const QList<QGeoCoordinate> &perimeter,
              const QGeoCoordinate &geoLeftBound,
              const QGeoProjectionWebMercator &p,
              QList<QDoubleVector2D> &wrappedPath,
              QList<QDoubleVector2D> &wrappedPathMinus1,
              QList<QDoubleVector2D> &wrappedPathPlus1,
              QDoubleVector2D *leftBoundWrapped)
{
    QList<QDoubleVector2D> path;
    for (const QGeoCoordinate &c : perimeter)
        path << p.geoToMapProjection(c);
    const QDoubleVector2D leftBound = p.geoToMapProjection(geoLeftBound);
    wrappedPath.clear();
    wrappedPathPlus1.clear();
    wrappedPathMinus1.clear();
    // compute 3 sets of "wrapped" coordinates: one w regular mercator, one w regular mercator +- 1.0
    for (QDoubleVector2D coord : path) {
        // We can get NaN if the map isn't set up correctly, or the projection
        // is faulty -- probably best thing to do is abort
        if (!qIsFinite(coord.x()) || !qIsFinite(coord.y()))
            return;

        const bool isPointLessThanUnwrapBelowX = (coord.x() < leftBound.x());
        // unwrap x to preserve geometry if moved to border of map
        if (isPointLessThanUnwrapBelowX)
            coord.setX(coord.x() + 1.0);

        QDoubleVector2D coordP1(coord.x() + 1.0, coord.y());
        QDoubleVector2D coordM1(coord.x() - 1.0, coord.y());

        wrappedPath.append(coord);
        wrappedPathPlus1.append(coordP1);
        wrappedPathMinus1.append(coordM1);
    }
    if (leftBoundWrapped)
        *leftBoundWrapped = leftBound;
}

void wrapPath(const QList<QGeoCoordinate> &perimeter,
              const QGeoCoordinate &geoLeftBound,
              const QGeoProjectionWebMercator &p,
              QList<QDoubleVector2D> &wrappedPath,
              QDoubleVector2D *leftBoundWrapped)
{
    QList<QDoubleVector2D> path;
    for (const QGeoCoordinate &c : perimeter)
        path << p.geoToMapProjection(c);
    const QDoubleVector2D leftBound = p.geoToMapProjection(geoLeftBound);
    wrapPath(path, leftBound,wrappedPath);
    if (leftBoundWrapped)
        *leftBoundWrapped = leftBound;
}

void wrapPath(const QList<QDoubleVector2D> &path,
              const QDoubleVector2D &geoLeftBound,
              QList<QDoubleVector2D> &wrappedPath)
{
    wrappedPath.clear();
    // compute 3 sets of "wrapped" coordinates: one w regular mercator, one w regular mercator +- 1.0
    for (QDoubleVector2D coord : path) {
        // We can get NaN if the map isn't set up correctly, or the projection
        // is faulty -- probably best thing to do is abort
        if (!qIsFinite(coord.x()) || !qIsFinite(coord.y()))
            return;

        const bool isPointLessThanUnwrapBelowX = (coord.x() < geoLeftBound.x());
        // unwrap x to preserve geometry if moved to border of map
        if (isPointLessThanUnwrapBelowX)
            coord.setX(coord.x() + 1.0);

        wrappedPath.append(coord);
    }
}

void clipPolygon(const QList<QDoubleVector2D> &wrappedPath,
                 const QGeoProjectionWebMercator &p,
                 QList<QList<QDoubleVector2D>> &clippedPaths,
                 QDoubleVector2D *leftBoundWrapped,
                 bool closed)
{
    // 2) Clip bounding box
    clippedPaths.clear();
    const QList<QDoubleVector2D> &visibleRegion = p.projectableGeometry();
    if (visibleRegion.size()) {
        QClipperUtils clipper;
        clipper.addSubjectPath(wrappedPath, closed);
        clipper.addClipPolygon(visibleRegion);
        clippedPaths = clipper.execute(QClipperUtils::Intersection, QClipperUtils::pftEvenOdd,
                                       QClipperUtils::pftEvenOdd);

        if (leftBoundWrapped) {
            // 2.1) update srcOrigin_ and leftBoundWrapped with the point with minimum X
            QDoubleVector2D lb(qInf(), qInf());
            for (const QList<QDoubleVector2D> &path : clippedPaths) {
                for (const QDoubleVector2D &p : path) {
                    if (p.x() < lb.x() || (p.x() == lb.x() && p.y() < lb.y()))
                        // y-minimization needed to find the same point on polygon and border
                        lb = p;
                }
            }

            if (qIsInf(lb.x())) // e.g., when the polygon is clipped entirely
                return;

            // 2.2) Prevent the conversion to and from clipper from introducing tiny negative offsets which,
            //      in turn will make the geometry wrap around.
            lb.setX(qMax(leftBoundWrapped->x(), lb.x()));

            *leftBoundWrapped = lb;
            // srcOrigin_ = p.mapProjectionToGeo(p.unwrapMapProjection(lb));
        }
    } else {
        clippedPaths.append(wrappedPath);
    }
}

void projectBbox(const QList<QDoubleVector2D> &clippedBbox,
                 const QGeoProjectionWebMercator &p,
                 QPainterPath &projectedBbox)
{
    projectedBbox.clear();
    bool first = true;
    for (const auto &coord : clippedBbox) {
        QDoubleVector2D point = p.wrappedMapProjectionToItemPosition(coord);
        if (first) {
            first = false;
            projectedBbox.moveTo(point.toPointF());
        } else {
            projectedBbox.lineTo(point.toPointF());
        }
    }
    projectedBbox.closeSubpath();
}


QRectF boundingRectangleFromList(const QList<QDoubleVector2D> &list)
{
    double xMin = qInf();
    double xMax = -qInf();
    double yMin = qInf();
    double yMax = -qInf();
    for (const auto &coord : list) {
        xMin = qMin(xMin, coord.x());
        xMax = qMax(xMax, coord.x());
        yMin = qMin(yMin, coord.y());
        yMax = qMax(yMax, coord.y());
    }
    return QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

QList<QGeoCoordinate> greaterCirclePath(const QList<QGeoCoordinate> &cornerPoints,
                                        greaterCirclePathForm form, int N)
{
    QList<QGeoCoordinate> path;
    if (cornerPoints.empty())
        return path;
    path.reserve(N);
    //If the path has to be closed we include the first coordinate again at the end of the procedure
    qsizetype lineCount = cornerPoints.size() - ((form == OpenPath) ? 1 : 0);
    for (qsizetype i = 0; i < lineCount; i++) {
        path.append(cornerPoints.at(i));
        const double p_lat = cornerPoints.at(i).latitude() / 180.0 * M_PI; //latitude point p in rad
        const double p_lon = cornerPoints.at(i).longitude() / 180.0 * M_PI; //longitude point p in rad
        const double q_lat = cornerPoints.at((i + 1) % cornerPoints.size()).latitude() / 180.0 * M_PI; //latitude point q in rad
        const double q_lon = cornerPoints.at((i + 1) % cornerPoints.size()).longitude() / 180.0 * M_PI;//longitude point q in rad

        const double c_p_lat = cos(p_lat);
        const double c_p_lon = cos(p_lon);
        const double s_p_lat = sin(p_lat);
        const double s_p_lon = sin(p_lon);
        const double c_q_lat = cos(q_lat);
        const double c_q_lon = cos(q_lon);
        const double s_q_lat = sin(q_lat);
        const double s_q_lon = sin(q_lon);

        const QDoubleVector3D p(c_p_lat * c_p_lon,
                                c_p_lat * s_p_lon,
                                s_p_lat);
        const QDoubleVector3D q(c_q_lat * c_q_lon,
                                c_q_lat * s_q_lon,
                                s_q_lat);
        const double qp = QDoubleVector3D::dotProduct(q, p); //scalar product of q p

        if (qp <= -1.0 || qp >= 1.0)
            continue;

        // the path from q to p will be parametrized as a combination of points p and q:
        // x = s*p + t*q.
        // We will then directly calculate the latitude and longitude of the interpolated point x
        // from the latitude and longitude of cornerpoints q and p.
        // The parameters s and t depend on each other to ensure that x is located on the
        // surface of the earth: s*s + 2*s*t*qp + t*t = 1
        // Their minimum value is 0, as negative values indicate a path that goes around earth
        // long way. Their maximum value follows as
        const double paramMax = sqrt(1 / (1 - qp * qp));

        double t = 0.0;
        for (int sign = 1; sign > -2.0; sign -= 2.0) {
            for (; t <= paramMax && t >= 0.0; t += sign * paramMax / N / 2) {
                // dependence between s and t requires a plus/minus
                // therefore we solve this equation two times with sign = -1/+1
                const double s = - t * qp + sign * sqrt(t * t * (qp * qp - 1.0) + 1.0);
                if (s < 0.0) //s > paramMax will never happen. If s < 0 we are done.
                    break;
                const double lat = asin(s * s_p_lat + t * s_q_lat);
                const double lon = atan2(s * c_p_lat * s_p_lon + t * c_q_lat * s_q_lon,
                                         s * c_p_lat * c_p_lon + t * c_q_lat * c_q_lon);
                path.append(QGeoCoordinate(lat * 180.0 / M_PI, lon * 180.0 / M_PI));
            }
            t -= paramMax / N / 2;
        }
    }
    if (form == OpenPath)
        path.append(cornerPoints.last());
    path.squeeze();
    return path;
}

} // namespace QDeclarativeGeoMapItemUtils

QT_END_NAMESPACE
