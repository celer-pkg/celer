// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPITEMUTILS_P_H
#define QDECLARATIVEGEOMAPITEMUTILS_P_H

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
#include <QtLocation/private/qgeoprojection_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>


QT_BEGIN_NAMESPACE

namespace QDeclarativeGeoMapItemUtils
{
    enum greaterCirclePathForm {
        OpenPath,
        ClosedPath
    };

    struct vec2 {
        float x;
        float y;
        vec2(const QDoubleVector2D &p)
        {
            x = float(p.x());
            y = float(p.y());
        }
        vec2() = default;
        vec2 &operator=(const QPointF &p)
        {
            x = float(p.x());
            y = float(p.y());
            return *this;
        }
        vec2 &operator=(const QDoubleVector2D &p)
        {
            x = float(p.x());
            y = float(p.y());
            return *this;
        }
        QDoubleVector2D toDoubleVector2D() const
        {
            return QDoubleVector2D(double(x), double(y));
        }
    };


    double distanceSqrPointLine(double p0_x
                              , double p0_y
                              , double p1_x
                              , double p1_y
                              , double p2_x
                              , double p2_y);

    void wrapPath(const QList<QGeoCoordinate> &perimeter
                , const QGeoCoordinate &geoLeftBound
                , const QGeoProjectionWebMercator &p
                , QList<QDoubleVector2D> &wrappedPath
                , QList<QDoubleVector2D> &wrappedPathMinus1
                , QList<QDoubleVector2D> &wrappedPathPlus1
                , QDoubleVector2D *leftBoundWrapped = nullptr);

    void wrapPath(const QList<QGeoCoordinate> &perimeter
                , const QGeoCoordinate &geoLeftBound
                , const QGeoProjectionWebMercator &p
                , QList<QDoubleVector2D> &wrappedPath
                , QDoubleVector2D *leftBoundWrapped = nullptr);

    void wrapPath(const QList<QDoubleVector2D> &path
                , const QDoubleVector2D &geoLeftBound
                , QList<QDoubleVector2D> &wrappedPath);

    void clipPolygon(const QList<QDoubleVector2D> &wrappedPath
                   , const QGeoProjectionWebMercator &p
                   , QList<QList<QDoubleVector2D> > &clippedPaths
                   , QDoubleVector2D *leftBoundWrapped = nullptr
                   , bool closed = true);

    void projectBbox(const QList<QDoubleVector2D> &clippedBbox
                   , const QGeoProjectionWebMercator &p
                   , QPainterPath &projectedBbox);

    QRectF boundingRectangleFromList(const QList<QDoubleVector2D> &list);

    QList<QGeoCoordinate> greaterCirclePath(const QList<QGeoCoordinate> &cornerPoints,
                                            greaterCirclePathForm form = QDeclarativeGeoMapItemUtils::OpenPath,
                                            int N=360);
};

QT_END_NAMESPACE

#endif // QDECLARATIVEGEOMAPITEMUTILS_P_H
