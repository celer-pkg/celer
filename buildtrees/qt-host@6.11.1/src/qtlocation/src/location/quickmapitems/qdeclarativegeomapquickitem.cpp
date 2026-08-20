// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapquickitem_p.h"

#include <QtCore/QScopedValueRollback>
#include <QtQml/qqmlinfo.h>
#include <QtQuick/QSGOpacityNode>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qgeoprojection_p.h>
#include <QDebug>
#include <cmath>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapQuickItem
    \nativetype QDeclarativeGeoMapQuickItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapQuickItem type displays an arbitrary Qt Quick object
           on a Map.

    The MapQuickItem type is used to place an arbitrary Qt Quick object
    on a Map at a specified location and size. Compared to floating an item
    above the Map, a MapQuickItem will follow the panning (and optionally, the
    zooming) of the Map as if it is on the Map surface.

    The \l{sourceItem} property contains the Qt Quick item to be drawn, which
    can be any kind of visible type.

    \section2 Positioning and Sizing

    The positioning of the MapQuickItem on the Map is controlled by two
    properties: \l coordinate and \l anchorPoint. If only \l coordinate is set,
    it specifies a longitude/latitude coordinate for the item to be placed at.
    The set coordinate will line up with the top-left corner of the contained
    item when shown on the screen.

    The \l anchorPoint property provides a way to line up the coordinate with
    other parts of the item than just the top-left corner, by setting a number
    of pixels the item will be offset by. A simple way to think about it is
    to note that the point given by \l anchorPoint on the item itself is the
    point that will line up with the given \l coordinate when displayed.

    In addition to being anchored to the map, the MapQuickItem can optionally
    follow the scale of the map, and change size when the Map is zoomed in or
    zoomed out. This behaviour is controlled by the \l zoomLevel property. The
    default behaviour if \l zoomLevel is not set is for the item to be drawn
    "on the screen" rather than "on the map", so that its size remains the same
    regardless of the zoom level of the Map.

    \section2 Performance

    Performance of a MapQuickItem is normally in the same ballpark as the
    contained Qt Quick item alone. Overheads added amount to a translation
    and (possibly) scaling of the original item, as well as a transformation
    from longitude and latitude to screen position.

    \section2 Limitations

    \note Due to an implementation detail, items placed inside a
    MapQuickItem will have a \c{parent} item which is not the MapQuickItem.
    Refer to the MapQuickItem by its \c{id}, and avoid the use of \c{anchor}
    in the \c{sourceItem}.

    \section2 Example Usage

    The following snippet shows a MapQuickItem containing an Image object,
    to display a Marker on the Map. This strategy is used to show the map
    markers in the MapViewer example.

    \snippet mapviewer/map/Marker.qml mqi-top
    \snippet mapviewer/map/Marker.qml mqi-anchor
    \snippet mapviewer/map/Marker.qml mqi-closeimage
    \snippet mapviewer/map/Marker.qml mqi-close

    \image api-mapquickitem.png
*/

/*!
    \qmlproperty bool QtLocation::MapQuickItem::autoFadeIn

    This property holds whether the item automatically fades in when zooming into the map
    starting from very low zoom levels. By default this is \c true.
    Setting this property to \c false causes the map item to always have the opacity specified
    with the \l QtQuick::Item::opacity property, which is 1.0 by default.

    \since 5.14
*/

QMapQuickItemMatrix4x4::QMapQuickItemMatrix4x4(QObject *parent) : QQuickTransform(parent) { }

void QMapQuickItemMatrix4x4::setMatrix(const QMatrix4x4 &matrix)
{
    if (m_matrix == matrix)
        return;
    m_matrix = matrix;
    update();
}

void QMapQuickItemMatrix4x4::applyTo(QMatrix4x4 *matrix) const
{
    *matrix *= m_matrix;
}


QDeclarativeGeoMapQuickItem::QDeclarativeGeoMapQuickItem(QQuickItem *parent)
    : QDeclarativeGeoMapItemBase(parent)
{
    m_itemType = QGeoMap::MapQuickItem;
    setFlag(ItemHasContents, true);
    opacityContainer_ = new QQuickItem(this);
    opacityContainer_->setParentItem(this);
    opacityContainer_->setFlag(ItemHasContents, true);
}

QDeclarativeGeoMapQuickItem::~QDeclarativeGeoMapQuickItem() {}

/*!
    \qmlproperty coordinate MapQuickItem::coordinate

    This property holds the anchor coordinate of the MapQuickItem. The point
    on the sourceItem that is specified by anchorPoint is kept aligned with
    this coordinate when drawn on the map.

    In the image below, there are 3 MapQuickItems that are identical except
    for the value of their anchorPoint properties. The values of anchorPoint
    for each are written on top of the item.

    \image api-mapquickitem-anchor.png
*/
void QDeclarativeGeoMapQuickItem::setCoordinate(const QGeoCoordinate &coordinate)
{
    if (coordinate_ == coordinate)
        return;

    coordinate_ = coordinate;
    geoshape_.setTopLeft(coordinate_);
    geoshape_.setBottomRight(coordinate_);
    // TODO: Handle zoomLevel != 0.0
    polishAndUpdate();
    emit coordinateChanged();
}

/*!
    \internal
*/
void QDeclarativeGeoMapQuickItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map && quickMap) {
        connect(map, &QGeoMap::cameraDataChanged,
                this, &QDeclarativeGeoMapQuickItem::polishAndUpdate);
        polishAndUpdate();
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapQuickItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (!mapAndSourceItemSet_ || updatingGeometry_ ||
        newGeometry.topLeft() == oldGeometry.topLeft()) {
        QDeclarativeGeoMapItemBase::geometryChange(newGeometry, oldGeometry);
        return;
    }

    QGeoCoordinate newCoordinate = map()->geoProjection().
            itemPositionToCoordinate(QDoubleVector2D(x(), y()) + QDoubleVector2D(anchorPoint_), false);

    if (newCoordinate.isValid())
        setCoordinate(newCoordinate);

    // Not calling QDeclarativeGeoMapItemBase::geometryChange() as it will be called from a nested
    // call to this function.
}

/*!
    \internal
*/
QGeoCoordinate QDeclarativeGeoMapQuickItem::coordinate()
{
    return coordinate_;
}

/*!
    \qmlproperty object MapQuickItem::sourceItem

    This property holds the source item that will be drawn on the map.
*/
void QDeclarativeGeoMapQuickItem::setSourceItem(QQuickItem *sourceItem)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(sourceItem); // Workaround for QTBUG-72930
    if (sourceItem_.data() == item)
        return;
    sourceItem_ = item;
    polishAndUpdate();
    emit sourceItemChanged();
}

QQuickItem *QDeclarativeGeoMapQuickItem::sourceItem()
{
    return sourceItem_.data();
}

/*!
    \internal
*/
void QDeclarativeGeoMapQuickItem::afterChildrenChanged()
{
    const QList<QQuickItem *> kids = childItems();
    if (kids.size() > 0) {
        bool printedWarning = false;
        for (auto *i : kids) {
            if (i->flags() & QQuickItem::ItemHasContents
                    && !qobject_cast<QQuickMouseArea *>(i)
                    && sourceItem_.data() != i
                    && opacityContainer_ != i) {
                if (!printedWarning) {
                    qmlWarning(this) << "Use the sourceItem property for the contained item, direct children are not supported";
                    printedWarning = true;
                }

                qmlWarning(i) << "deleting this child";
                i->deleteLater();
            }
        }
    }
}

/*!
    \qmlproperty point MapQuickItem::anchorPoint

    This property determines which point on the sourceItem that will be lined
    up with the coordinate on the map.
*/
void QDeclarativeGeoMapQuickItem::setAnchorPoint(const QPointF &anchorPoint)
{
    if (anchorPoint == anchorPoint_)
        return;
    anchorPoint_ = anchorPoint;
    polishAndUpdate();
    emit anchorPointChanged();
}

QPointF QDeclarativeGeoMapQuickItem::anchorPoint() const
{
    return anchorPoint_;
}

/*!
    \qmlproperty real MapQuickItem::zoomLevel

    This property controls the scaling behaviour of the contents of the
    MapQuickItem. In particular, by setting this property it is possible
    to choose between objects that are drawn on the screen (and sized in
    screen pixels), and those drawn on the map surface (which change size
    with the zoom level of the map).

    The default value for this property is 0.0, which corresponds to drawing
    the object on the screen surface. If set to another value, the object will
    be drawn on the map surface instead. The value (if not zero) specifies the
    zoomLevel at which the object will be visible at a scale of 1:1 (ie, where
    object pixels and screen pixels are the same). At zoom levels lower than
    this, the object will appear smaller, and at higher zoom levels, appear
    larger. This is in contrast to when this property is set to zero, where
    the object remains the same size on the screen at all zoom levels.
*/
void QDeclarativeGeoMapQuickItem::setZoomLevel(qreal zoomLevel)
{
    if (zoomLevel == zoomLevel_)
        return;
    zoomLevel_ = zoomLevel;
    // TODO: update geoshape_!
    polishAndUpdate();
    emit zoomLevelChanged();
}

qreal QDeclarativeGeoMapQuickItem::zoomLevel() const
{
    return zoomLevel_;
}

const QGeoShape &QDeclarativeGeoMapQuickItem::geoShape() const
{
    // TODO: return a QGeoRectangle representing the bounding geo rectangle of the quick item
    // when zoomLevel_ is != 0.0
    return geoshape_;
}

void QDeclarativeGeoMapQuickItem::setGeoShape(const QGeoShape &shape)
{
    if (shape == geoshape_)
        return;

    const QGeoRectangle rect = shape.boundingGeoRectangle();
    geoshape_ = rect;
    coordinate_ = rect.center();

    // TODO: Handle zoomLevel != 0.0
    polishAndUpdate();
    emit coordinateChanged();

}

bool QDeclarativeGeoMapQuickItem::contains(const QPointF &point) const
{
    if (zoomLevel_ != 0) {
        // This follows the mathematical principle of
        // QDeclarativeGeoMap::fitViewportToMapItemsRefine but in a shorter way:
        const QTransform t = matrix_->m_matrix.toTransform();
        const QPointF transformedPoint = t.inverted().map(point);
        return QQuickItem::contains(transformedPoint);
    }
    return QQuickItem::contains(point);
}

/*!
    \internal
*/
void QDeclarativeGeoMapQuickItem::updatePolish()
{
    if (!quickMap() && sourceItem_) {
        mapAndSourceItemSet_ = false;
        sourceItem_.data()->setParentItem(0);
        return;
    }

    if (!quickMap() || !map() || !sourceItem_) {
        mapAndSourceItemSet_ = false;
        return;
    }

    if (!mapAndSourceItemSet_ && quickMap() && map() && sourceItem_) {
        mapAndSourceItemSet_ = true;
        sourceItem_.data()->setParentItem(opacityContainer_);
        sourceItem_.data()->setTransformOrigin(QQuickItem::TopLeft);
        connect(sourceItem_.data(), &QQuickItem::xChanged,
                this, &QDeclarativeGeoMapQuickItem::polishAndUpdate);
        connect(sourceItem_.data(), &QQuickItem::yChanged,
                this, &QDeclarativeGeoMapQuickItem::polishAndUpdate);
        connect(sourceItem_.data(), &QQuickItem::widthChanged,
                this, &QDeclarativeGeoMapQuickItem::polishAndUpdate);
        connect(sourceItem_.data(), &QQuickItem::heightChanged,
                this, &QDeclarativeGeoMapQuickItem::polishAndUpdate);
    }

    if (!coordinate_.isValid()) {
        opacityContainer_->setVisible(false);
        return;
    } else {
        opacityContainer_->setVisible(true);
    }

    QScopedValueRollback<bool> rollback(updatingGeometry_);
    updatingGeometry_ = true;

    opacityContainer_->setOpacity(zoomLevelOpacity());

    setWidth(sourceItem_.data()->width());
    setHeight(sourceItem_.data()->height());
    if (zoomLevel_ != 0.0 // zoom level initialized to 0.0. If it's different, it has been set explicitly.
            && map()->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) { // Currently unsupported on any other projection
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());

        if (!matrix_) {
            matrix_ = new QMapQuickItemMatrix4x4(this);
            matrix_->appendToItem(opacityContainer_);
        }
        matrix_->setMatrix(p.quickItemTransformation(coordinate(), anchorPoint_, zoomLevel_));
        setPosition(QPointF(0,0));
    } else {
        if (map()->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) {
            const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
            if (map()->cameraData().tilt() > 0.0
                    && !p.isProjectable(p.geoToWrappedMapProjection(coordinate()))) {
                // if the coordinate is behind the camera, we use the transformation to get the item out of the way
                if (!matrix_) {
                    matrix_ = new QMapQuickItemMatrix4x4(this);
                    matrix_->appendToItem(opacityContainer_);
                }
                matrix_->setMatrix(p.quickItemTransformation(coordinate(), anchorPoint_, map()->cameraData().zoomLevel()));
                setPosition(QPointF(0,0));
            } else { // All good, rendering screen-aligned
                if (matrix_)
                    matrix_->setMatrix(QMatrix4x4());
                setPositionOnMap(coordinate(), anchorPoint_);
            }
        } else { // On other projections we can only currently test if coordinateToItemPosition returns a valid position
            if (map()->cameraData().tilt() > 0.0
                    && qIsNaN(map()->geoProjection().coordinateToItemPosition(coordinate(), false).x())) {
                opacityContainer_->setVisible(false);
            } else {
                if (matrix_)
                    matrix_->setMatrix(QMatrix4x4());
                setPositionOnMap(coordinate(), anchorPoint_);
            }
        }
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMapQuickItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    Q_UNUSED(event);
    if (event.mapSize.width() <= 0 || event.mapSize.height() <= 0)
        return;

    polishAndUpdate();
}

/*!
    \internal
*/
qreal QDeclarativeGeoMapQuickItem::scaleFactor()
{
    qreal scale = 1.0;
    // use 1+x to avoid fuzzy compare against zero
    if (!qFuzzyCompare(1.0 + zoomLevel_, 1.0))
        scale = std::pow(0.5, zoomLevel_ - map()->cameraData().zoomLevel());
    return scale;
}

QT_END_NAMESPACE
