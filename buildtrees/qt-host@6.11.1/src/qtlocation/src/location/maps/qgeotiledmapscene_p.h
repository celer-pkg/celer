// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOTILEDMAPSCENE_P_H
#define QGEOTILEDMAPSCENE_P_H

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

#include <QObject>
#include <QtLocation/private/qlocationglobal_p.h>

QT_BEGIN_NAMESPACE

class QGeoCameraData;
class QGeoTileSpec;
class QDoubleVector2D;
struct QGeoTileTexture;
class QSGNode;
class QQuickWindow;
class QGeoTiledMapScenePrivate;

class Q_LOCATION_EXPORT QGeoTiledMapScene : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoTiledMapScene)
public:
    explicit QGeoTiledMapScene(QObject *parent = nullptr);
    virtual ~QGeoTiledMapScene();

    void setScreenSize(const QSize &size);
    void setTileSize(int tileSize);
    void setCameraData(const QGeoCameraData &cameraData);
    void setVisibleArea(const QRectF &visibleArea);

    void setVisibleTiles(const QSet<QGeoTileSpec> &tiles);
    const QSet<QGeoTileSpec> &visibleTiles() const;

    void addTile(const QGeoTileSpec &spec, QSharedPointer<QGeoTileTexture> texture);

    QSGNode *updateSceneGraph(QSGNode *oldNode, QQuickWindow *window);

    QSet<QGeoTileSpec> texturedTiles();

    void clearTexturedTiles();

Q_SIGNALS:
    void newTilesVisible(const QSet<QGeoTileSpec> &newTiles);

private:
    void updateSceneParameters();

    Q_DISABLE_COPY(QGeoTiledMapScene)
};

QT_END_NAMESPACE

#endif // QGEOTILEDMAPSCENE_P_H
