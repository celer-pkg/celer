// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOTILEDMAP_P_P_H
#define QGEOTILEDMAP_P_P_H

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

#include <QtCore/QPointer>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qgeomap_p_p.h>
#include <QtLocation/private/qgeocameradata_p.h>
#include <QtLocation/private/qgeomaptype_p.h>

#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

class QGeoCameraTiles;
class QGeoTiledMapScene;
class QAbstractGeoTileCache;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMap;
class QGeoTileRequestManager;
class QGeoTileSpec;
class QSGNode;
class QQuickWindow;
class QGeoCameraCapabilities;

class Q_LOCATION_EXPORT QGeoTiledMapPrivate : public QGeoMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoTiledMap)
public:
    QGeoTiledMapPrivate(QGeoTiledMappingManagerEngine *engine);
    ~QGeoTiledMapPrivate();

    QSGNode *updateSceneGraph(QSGNode *node, QQuickWindow *window);

    void updateTile(const QGeoTileSpec &spec);
    void prefetchTiles();
    QGeoMapType activeMapType() const;
    void onCameraCapabilitiesChanged(const QGeoCameraCapabilities &oldCameraCapabilities);

protected:
    void changeViewportSize(const QSize& size) override;
    void changeCameraData(const QGeoCameraData &cameraData) override;
    void changeActiveMapType(const QGeoMapType &mapType) override;
    void changeTileVersion(int version);
    void clearScene();

    void updateScene();

    void setVisibleArea(const QRectF &visibleArea) override;
    QRectF visibleArea() const override;

#ifdef QT_LOCATION_DEBUG
public:
#else
protected:
#endif
    QAbstractGeoTileCache *m_cache;
    QGeoCameraTiles *m_visibleTiles;
    QGeoCameraTiles *m_prefetchTiles;
    QGeoTiledMapScene *m_mapScene;
    QGeoTileRequestManager *m_tileRequests;
    QRectF m_visibleArea;
    int m_maxZoomLevel;
    int m_minZoomLevel;
    QGeoTiledMap::PrefetchStyle m_prefetchStyle;
    Q_DISABLE_COPY(QGeoTiledMapPrivate)
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAP_P_P_H
