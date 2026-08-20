// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapitembase_p.h"
#include "qgeocameradata_p.h"

#include <QtQml/QQmlInfo>
#include <QtQuick/QSGOpacityNode>

#include <QtQuick/private/qquickmousearea_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qgeoprojection_p.h>

#include <QtQuickShapes/private/qquickshape_p_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeGeoMapItemBase::QDeclarativeGeoMapItemBase(QQuickItem *parent)
    : QQuickItem(parent)
{
    connect(this, &QDeclarativeGeoMapItemBase::childrenChanged,
            this, &QDeclarativeGeoMapItemBase::afterChildrenChanged);
    // Changing opacity on a mapItemGroup should affect also the opacity on the children.
    // This must be notified to plugins, if they are to render the item.
    connect(this, &QQuickItem::opacityChanged, this, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged);
}

QDeclarativeGeoMapItemBase::~QDeclarativeGeoMapItemBase()
{
    disconnect(this, &QDeclarativeGeoMapItemBase::childrenChanged,
               this, &QDeclarativeGeoMapItemBase::afterChildrenChanged);
    if (quickMap_)
        quickMap_->removeMapItem(this);
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::afterChildrenChanged()
{
    const QList<QQuickItem *> kids = childItems();
    if (kids.size() > 0) {
        bool printedWarning = false;
        for (auto *i : kids) {
            if (i->flags() & QQuickItem::ItemHasContents
                    && !qobject_cast<QQuickMouseArea *>(i)
                    && i->objectName() != QStringLiteral("_qt_map_item_shape"))
            {
                if (!printedWarning) {
                    qmlWarning(this) << "Geographic map items do not support child items";
                    printedWarning = true;
                }

                qmlWarning(i) << "deleting this child";
                i->deleteLater();
            }
        }
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    if (quickMap == quickMap_)
        return;
    if (quickMap && quickMap_)
        return; // don't allow association to more than one map

    quickMap_ = quickMap;
    map_ = map;

    if (map_ && quickMap_) {
        // For performance reasons we're not connecting map_'s and quickMap_'s signals to this.
        // Rather, the handling of cameraDataChanged, visibleAreaChanged, heightChanged and widthChanged is done explicitly in QDeclarativeGeoMap by directly calling methods on the items.
        // See QTBUG-76950
        lastMapSize_ = QSizeF(quickMap_->width(), quickMap_->height());
        lastCameraData_ = map_->cameraData();
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::baseCameraDataChanged(const QGeoCameraData &cameraData)
{
    QGeoMapViewportChangeEvent evt;
    evt.cameraData = cameraData;
    evt.mapSize = QSizeF(quickMap_->width(), quickMap_->height());

    if (evt.mapSize != lastMapSize_)
        evt.mapSizeChanged = true;

    if (cameraData.bearing() != lastCameraData_.bearing())
        evt.bearingChanged = true;
    if (cameraData.center() != lastCameraData_.center())
        evt.centerChanged = true;
    if (cameraData.roll() != lastCameraData_.roll())
        evt.rollChanged = true;
    if (cameraData.tilt() != lastCameraData_.tilt())
        evt.tiltChanged = true;
    if (cameraData.zoomLevel() != lastCameraData_.zoomLevel())
        evt.zoomLevelChanged = true;

    lastMapSize_ = evt.mapSize;
    lastCameraData_ = cameraData;

    afterViewportChanged(evt);
}

void QDeclarativeGeoMapItemBase::visibleAreaChanged()
{
    QGeoMapViewportChangeEvent evt;
    evt.mapSize = QSizeF(quickMap_->width(), quickMap_->height());
    afterViewportChanged(evt);
}

/*!
    \internal
*/
void QDeclarativeGeoMapItemBase::setPositionOnMap(const QGeoCoordinate &coordinate, const QPointF &offset)
{
    if (!map_ || !quickMap_)
        return;

    QDoubleVector2D pos;
    if (map()->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) {
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
        QDoubleVector2D wrappedProjection = p.geoToWrappedMapProjection(coordinate);
        if (!p.isProjectable(wrappedProjection))
            return;
        pos = p.wrappedMapProjectionToItemPosition(wrappedProjection);
    } else {
        pos = map()->geoProjection().coordinateToItemPosition(coordinate, false);
        if (qIsNaN(pos.x()))
            return;
    }

    QPointF topLeft = pos.toPointF() - offset;

    setPosition(topLeft);
}

bool QDeclarativeGeoMapItemBase::autoFadeIn() const
{
    return m_autoFadeIn;
}

static const double opacityRampMin = 1.5;
static const double opacityRampMax = 2.5;

void QDeclarativeGeoMapItemBase::setAutoFadeIn(bool fadeIn)
{
    if (fadeIn == m_autoFadeIn)
        return;
    m_autoFadeIn = fadeIn;
    if (quickMap_ && quickMap_->zoomLevel() < opacityRampMax)
        polishAndUpdate();
}

QLocation::ReferenceSurface QDeclarativeGeoMapItemBase::referenceSurface() const
{
    return m_referenceSurface;
}

void QDeclarativeGeoMapItemBase::setReferenceSurface(QLocation::ReferenceSurface referenceSurface)
{
    if (referenceSurface == m_referenceSurface)
        return;
    m_referenceSurface = referenceSurface;
    emit referenceSurfaceChanged();
    updatePolish();
}

int QDeclarativeGeoMapItemBase::lodThreshold() const
{
    return m_lodThreshold;
}

void QDeclarativeGeoMapItemBase::setLodThreshold(int lt)
{
    if (lt == m_lodThreshold)
        return;
    m_lodThreshold = lt;
    update();
}

/*!
    \internal

    This returns the zoom level to be used when requesting the LOD.
    Essentially it clamps to m_lodThreshold, and if above, it selects
    a ZL higher than the maximum LODable level.
*/
unsigned int QDeclarativeGeoMapItemBase::zoomForLOD(int zoom) const
{
    if (zoom >= m_lodThreshold)
        return 30; // some arbitrarily large zoom
    return uint(zoom);
}

/*!
    \internal
*/
float QDeclarativeGeoMapItemBase::zoomLevelOpacity() const
{
    if (!m_autoFadeIn) // Consider skipping the opacity node instead.
        return 1.0;
    else if (quickMap_->zoomLevel() > opacityRampMax)
        return 1.0;
    else if (quickMap_->zoomLevel() > opacityRampMin)
        return quickMap_->zoomLevel() - opacityRampMin;
    else
        return 0.0;
}

void QDeclarativeGeoMapItemBase::setShapeTriangulationScale(QQuickShape *shape, qreal maxCoord) const
{
    const qreal zoom = qMax(0.01, quickMap_->zoomLevel());
    qreal scale = 1 / zoom;

    // cater also for QTriangulator's 65536 coordinate limit due to the fixed point math
    qint64 coord = qint64(maxCoord);
    const qint64 COORD_LIMIT = (1 << 21) / 32; // 65536 (where 32 is Q_FIXED_POINT_SCALE)
    while (coord > COORD_LIMIT) {
        coord /= COORD_LIMIT;
        scale /= COORD_LIMIT;
    }

    QQuickShapePrivate::get(shape)->triangulationScale = scale;
}

/*!
    \internal
*/
QSGNode *QDeclarativeGeoMapItemBase::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *pd)
{
    if (!map_ || !quickMap_ || map_->supportedMapItemTypes() & itemType()) {
        if (oldNode)
            delete oldNode;
        oldNode = nullptr;
        return nullptr;
    }

    QSGOpacityNode *opn = static_cast<QSGOpacityNode *>(oldNode);
    if (!opn)
        opn = new QSGOpacityNode();

    opn->setOpacity(zoomLevelOpacity());

    QSGNode *oldN = opn->childCount() ? opn->firstChild() : 0;
    opn->removeAllChildNodes();
    if (opn->opacity() > 0.0) {
        QSGNode *n = this->updateMapItemPaintNode(oldN, pd);
        if (n)
            opn->appendChildNode(n);
    } else {
        delete oldN;
    }

    return opn;
}

/*!
    \internal
*/
QSGNode *QDeclarativeGeoMapItemBase::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    delete oldNode;
    return nullptr;
}

/*!
    \internal

    The actual combined opacity of the item. Needed by custom renderer to look like
    the scene-graph one.
*/
qreal QDeclarativeGeoMapItemBase::mapItemOpacity() const
{
    if (parentGroup_)
        return parentGroup_->mapItemOpacity() * opacity();
    return opacity();
}

void QDeclarativeGeoMapItemBase::setParentGroup(QDeclarativeGeoMapItemGroup &parentGroup)
{
    parentGroup_ = &parentGroup;
    if (parentGroup_) {
        connect(parentGroup_, &QDeclarativeGeoMapItemGroup::mapItemOpacityChanged,
                this, &QDeclarativeGeoMapItemBase::mapItemOpacityChanged);
    }
}

bool QDeclarativeGeoMapItemBase::isPolishScheduled() const
{
    return QQuickItemPrivate::get(this)->polishScheduled;
}

void QDeclarativeGeoMapItemBase::polishAndUpdate()
{
    polish();
    update();
}

QT_END_NAMESPACE
