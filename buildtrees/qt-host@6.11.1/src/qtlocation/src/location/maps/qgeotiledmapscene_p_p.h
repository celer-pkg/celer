// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOTILEDMAPSCENE_P_P_H
#define QGEOTILEDMAPSCENE_P_P_H

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

#include "qgeotiledmapscene_p.h"
#include "qgeocameradata_p.h"
#include "qgeotilespec_p.h"

#include <QtQuick/QSGImageNode>
#include <QtQuick/QQuickWindow>

#include <QtCore/private/qobject_p.h>
#include <QtPositioning/private/qdoublevector3d_p.h>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QGeoTiledMapTileContainerNode : public QSGTransformNode
{
public:
    void addChild(const QGeoTileSpec &spec, QSGImageNode *node)
    {
        tiles.insert(spec, node);
        appendChildNode(node);
    }
    QHash<QGeoTileSpec, QSGImageNode *> tiles;
};

class Q_LOCATION_EXPORT QGeoTiledMapRootNode : public QSGClipNode
{
public:
    QGeoTiledMapRootNode()
        : isTextureLinear(false)
        , geometry(QSGGeometry::defaultAttributes_Point2D(), 4)
        , root(new QSGTransformNode())
        , tiles(new QGeoTiledMapTileContainerNode())
        , wrapLeft(new QGeoTiledMapTileContainerNode())
        , wrapRight(new QGeoTiledMapTileContainerNode())
    {
        setIsRectangular(true);
        setGeometry(&geometry);
        root->appendChildNode(tiles);
        root->appendChildNode(wrapLeft);
        root->appendChildNode(wrapRight);
        appendChildNode(root);
    }

    ~QGeoTiledMapRootNode()
    {
        qDeleteAll(textures);
    }

    void setClipRect(const QRect &rect)
    {
        if (rect != clipRect) {
            QSGGeometry::updateRectGeometry(&geometry, rect);
            QSGClipNode::setClipRect(rect);
            clipRect = rect;
            markDirty(DirtyGeometry);
        }
    }

    void updateTiles(QGeoTiledMapTileContainerNode *root,
                     QGeoTiledMapScenePrivate *d,
                     double camAdjust,
                     QQuickWindow *window);

    bool isTextureLinear;

    QSGGeometry geometry;
    QRect clipRect;

    QSGTransformNode *root;

    QGeoTiledMapTileContainerNode *tiles;        // The majority of the tiles
    QGeoTiledMapTileContainerNode *wrapLeft;     // When zoomed out, the tiles that wrap around on the left.
    QGeoTiledMapTileContainerNode *wrapRight;    // When zoomed out, the tiles that wrap around on the right

    QHash<QGeoTileSpec, QSGTexture *> textures;

#ifdef QT_LOCATION_DEBUG
    double m_sideLengthPixel;
    QMap<double, QList<QGeoTileSpec>> m_droppedTiles;
#endif
};

class Q_LOCATION_EXPORT QGeoTiledMapScenePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGeoTiledMapScene)
public:
    QGeoTiledMapScenePrivate();
    ~QGeoTiledMapScenePrivate();

    void addTile(const QGeoTileSpec &spec, QSharedPointer<QGeoTileTexture> texture);

    void setVisibleTiles(const QSet<QGeoTileSpec> &visibleTiles);
    void removeTiles(const QSet<QGeoTileSpec> &oldTiles);
    bool buildGeometry(const QGeoTileSpec &spec, QSGImageNode *imageNode, bool &overzooming);
    void updateTileBounds(const QSet<QGeoTileSpec> &tiles);
    void setupCamera();
    inline bool isTiltedOrRotated() const { return (m_cameraData.tilt() > 0.0) || (m_cameraData.bearing() > 0.0); }

public:

    QSize m_screenSize; // in pixels
    int m_tileSize = 0; // the pixel resolution for each tile
    QGeoCameraData m_cameraData;
    QRectF m_visibleArea;
    QSet<QGeoTileSpec> m_visibleTiles;

    QDoubleVector3D m_cameraUp;
    QDoubleVector3D m_cameraEye;
    QDoubleVector3D m_cameraCenter;
    QMatrix4x4 m_projectionMatrix;

    // scales up the tile geometry and the camera altitude, resulting in no visible effect
    // other than to control the accuracy of the render by keeping the values in a sensible range
    double m_scaleFactor =
#ifdef QT_LOCATION_DEBUG
        1.0;
#else
        10.0;
#endif

    // rounded down, positive zoom is zooming in, corresponding to reduced altitude
    int m_intZoomLevel = 0;

    // mercatorToGrid transform
    // the number of tiles in each direction for the whole map (earth) at the current zoom level.
    // it is 1<<zoomLevel
    int m_sideLength = 0;

    QHash<QGeoTileSpec, QSharedPointer<QGeoTileTexture> > m_textures;
    QList<QGeoTileSpec> m_updatedTextures;

    // tilesToGrid transform
    int m_minTileX = -1; // the minimum tile index, i.e. 0 to sideLength which is 1<< zoomLevel
    int m_minTileY = -1;
    int m_maxTileX = -1;
    int m_maxTileY = -1;
    int m_tileXWrapsBelow = 0; // the wrap point as a tile index
    bool m_linearScaling = false;
    bool m_dropTextures = false;

#ifdef QT_LOCATION_DEBUG
    double m_sideLengthPixel;
    QGeoTiledMapRootNode *m_mapRoot = nullptr;
#endif
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPSCENE_P_P_H
