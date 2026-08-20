// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOCAMERATILES_P_P_H
#define QGEOCAMERATILES_P_P_H

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

#include "qgeocameratiles_p.h"
#include "qgeomaptype_p.h"
#include "qgeocameradata_p.h"
#include "qgeotilespec_p.h"

#include <QtCore/qlist.h>
#include <QtCore/qset.h>

#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qdoublevector3d_p.h>

QT_BEGIN_NAMESPACE

struct Q_LOCATION_EXPORT Frustum
{
    QDoubleVector3D apex;
    QDoubleVector3D topLeftNear;
    QDoubleVector3D topLeftFar;
    QDoubleVector3D topRightNear;
    QDoubleVector3D topRightFar;
    QDoubleVector3D bottomLeftNear;
    QDoubleVector3D bottomLeftFar;
    QDoubleVector3D bottomRightNear;
    QDoubleVector3D bottomRightFar;
};

typedef QList<QDoubleVector3D> PolygonVector;

class Q_LOCATION_EXPORT QGeoCameraTilesPrivate
{
public:
    struct ClippedFootprint
    {
        ClippedFootprint()
        {}
        ClippedFootprint(const PolygonVector &left_, const PolygonVector &mid_, const PolygonVector &right_)
            : left(left_), mid(mid_), right(right_)
        {}
        PolygonVector left;
        PolygonVector mid;
        PolygonVector right;
    };

    struct TileMap
    {
        TileMap();

        void add(int tileX, int tileY);

        QMap<int, QPair<int, int> > data;
    };

    void updateMetadata();
    void updateGeometry();

    Frustum createFrustum(double viewExpansion) const;
    PolygonVector frustumFootprint(const Frustum &frustum) const;

    QPair<PolygonVector, PolygonVector> splitPolygonAtAxisValue(const PolygonVector &polygon, int axis, double value) const;
    ClippedFootprint clipFootprintToMap(const PolygonVector &footprint) const;

    QList<QPair<double, int> > tileIntersections(double p1, int t1, double p2, int t2) const;
    QSet<QGeoTileSpec> tilesFromPolygon(const PolygonVector &polygon) const;

    static QGeoCameraTilesPrivate *get(QGeoCameraTiles *o) {
        return o->d_ptr.get();
    }

public:
    QString m_pluginString;
    QGeoMapType m_mapType;
    int m_mapVersion = -1;
    QGeoCameraData m_camera;
    QSize m_screenSize;
    QRectF m_visibleArea;
    int m_tileSize = 0;
    QSet<QGeoTileSpec> m_tiles;

    int m_intZoomLevel = 0;
    int m_sideLength = 1;
    bool m_dirtyGeometry = false;
    bool m_dirtyMetadata = false;
    double m_viewExpansion = 1.0;

#ifdef QT_LOCATION_DEBUG
    // updateGeometry
    ClippedFootprint m_clippedFootprint;
    PolygonVector m_frustumFootprint;
    Frustum m_frustum;

    // createFrustum
    mutable QDoubleVector3D m_createFrustum_center;
    mutable QDoubleVector3D m_createFrustum_eye;
#endif
};

QT_END_NAMESPACE

#endif // QGEOCAMERATILES_P_P_H
