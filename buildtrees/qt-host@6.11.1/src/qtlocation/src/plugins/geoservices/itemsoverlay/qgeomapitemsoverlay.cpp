// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomapitemsoverlay.h"
#include "qgeomappingmanagerengineitemsoverlay.h"
#include <QtLocation/private/qgeomap_p_p.h>
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgrectanglenode.h>
#include <QtQuick/qquickwindow.h>

QT_BEGIN_NAMESPACE

class QGeoMapItemsOverlayPrivate : public QGeoMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoMapItemsOverlay)
public:
    QGeoMapItemsOverlayPrivate(QGeoMappingManagerEngineItemsOverlay *engine, QGeoMapItemsOverlay *map);
    virtual ~QGeoMapItemsOverlayPrivate();

    void updateObjectsGeometry();

    void setVisibleArea(const QRectF &visibleArea) override;
    QRectF visibleArea() const override;

protected:
    void changeViewportSize(const QSize &size) override;
    void changeCameraData(const QGeoCameraData &oldCameraData) override;
    void changeActiveMapType(const QGeoMapType &mapType) override;

    QRectF m_visibleArea;
};

QGeoMapItemsOverlay::QGeoMapItemsOverlay(QGeoMappingManagerEngineItemsOverlay *engine, QObject *parent)
    : QGeoMap(*(new QGeoMapItemsOverlayPrivate(engine, this)), parent)
{

}

QGeoMapItemsOverlay::~QGeoMapItemsOverlay()
{
}

QGeoMap::Capabilities QGeoMapItemsOverlay::capabilities() const
{
    return Capabilities(SupportsVisibleRegion
                        | SupportsSetBearing
                        | SupportsAnchoringCoordinate);
}

void QGeoMapItemsOverlayPrivate::setVisibleArea(const QRectF &visibleArea)
{
    Q_Q(QGeoMapItemsOverlay);
    const QRectF va = clampVisibleArea(visibleArea);
    if (va == m_visibleArea)
        return;

    m_visibleArea = va;
    m_geoProjection->setVisibleArea(va);

    q->sgNodeChanged();
}

QRectF QGeoMapItemsOverlayPrivate::visibleArea() const
{
    return m_visibleArea;
}

QSGNode *QGeoMapItemsOverlay::updateSceneGraph(QSGNode *node, QQuickWindow *)
{
    return node;
}

QGeoMapItemsOverlayPrivate::QGeoMapItemsOverlayPrivate(QGeoMappingManagerEngineItemsOverlay *engine, QGeoMapItemsOverlay *map)
    : QGeoMapPrivate(engine, new QGeoProjectionWebMercator)
{
    Q_UNUSED(map);
}

QGeoMapItemsOverlayPrivate::~QGeoMapItemsOverlayPrivate()
{
}

void QGeoMapItemsOverlayPrivate::updateObjectsGeometry()
{
}

void QGeoMapItemsOverlayPrivate::changeViewportSize(const QSize &/*size*/)
{
    updateObjectsGeometry();
}

void QGeoMapItemsOverlayPrivate::changeCameraData(const QGeoCameraData &/*oldCameraData*/)
{
    updateObjectsGeometry();
}

void QGeoMapItemsOverlayPrivate::changeActiveMapType(const QGeoMapType &/*mapType*/)
{
    updateObjectsGeometry();
}

QT_END_NAMESPACE




