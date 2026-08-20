// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativerectanglemapitem_p.h"
#include "qdeclarativerectanglemapitem_p_p.h"

#include <QtCore/QScopedValueRollback>
#include <QPainterPath>
#include <qnumeric.h>
#include <QRectF>
#include <QPointF>

#include <QtLocation/private/qgeomap_p.h>
#include <QtPositioning/private/qlocationutils_p.h>
#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapRectangle
    \nativetype QDeclarativeRectangleMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapRectangle type displays a rectangle on a Map.

    The MapRectangle type displays a rectangle on a Map. Rectangles are a
    special case of Polygon with exactly 4 vertices and 4 "straight" edges. In
    this case, "straight" means that the top-left point has the same latitude
    as the top-right point (the top edge), and the bottom-left point has the
    same latitude as the bottom-right point (the bottom edge). Similarly, the
    points on the left side have the same longitude, and the points on the
    right side have the same longitude.

    To specify the rectangle, it requires a \l topLeft and \l bottomRight point,
    both given by a \l {coordinate}.

    By default, the rectangle is displayed with transparent fill and a 1-pixel
    thick black border. This can be changed using the \l color, \l border.color
    and \l border.width properties.

    \note Similar to the \l MapPolygon type, MapRectangles are geographic
    items, thus dragging a MapRectangle causes its vertices to be recalculated
    in the geographic coordinate space. Apparent stretching of the item
    occurs when dragged to the a different latitude, however, its edges
    remain straight.

    \section2 Example Usage

    The following snippet shows a map containing a MapRectangle, spanning
    from (-27, 153) to (-28, 153.5), near Brisbane, Australia. The rectangle
    is filled in green, with a 2 pixel black border.

    \code
    Map {
        MapRectangle {
            color: 'green'
            border.width: 2
            topLeft {
                latitude: -27
                longitude: 153
            }
            bottomRight {
                latitude: -28
                longitude: 153.5
            }
        }
    }
    \endcode

    \image api-maprectangle.png
*/

/*!
    \qmlproperty bool QtLocation::MapRectangle::autoFadeIn

    This property holds whether the item automatically fades in when zooming into the map
    starting from very low zoom levels. By default this is \c true.
    Setting this property to \c false causes the map item to always have the opacity specified
    with the \l QtQuick::Item::opacity property, which is 1.0 by default.

    \since 5.14
*/

/*!
    \qmlproperty enum QtLocation::MapRectangle::referenceSurface

    This property determines the reference surface of the rectangle. If it is set to
    \l QLocation::ReferenceSurface::Map the edge points are connected with straight lines
    on the map. If it is set to \l QLocation::ReferenceSurface::Globe, the edge points
    are connected following the great circle path, describing the shortest connection of
    two points on a sphere.
    Default value is \l QLocation::ReferenceSurface::Map.

    \since 6.5
*/

QDeclarativeRectangleMapItem::QDeclarativeRectangleMapItem(QQuickItem *parent)
    : QDeclarativeGeoMapItemBase(parent), m_border(this),
      m_d(new QDeclarativeRectangleMapItemPrivateCPU(*this))
{
    // ToDo: handle envvar, and switch implementation.
    m_itemType = QGeoMap::MapRectangle;
    setFlag(ItemHasContents, true);
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::colorChanged,
                     this, &QDeclarativeRectangleMapItem::onLinePropertiesChanged);
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::widthChanged,
                     this, &QDeclarativeRectangleMapItem::onLinePropertiesChanged);
}

QDeclarativeRectangleMapItem::~QDeclarativeRectangleMapItem()
{
}

/*!
    \internal
*/
void QDeclarativeRectangleMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (!map)
        return;
    m_d->onMapSet();
}

/*!
    \qmlpropertygroup Location::MapRectangle::border
    \qmlproperty int MapRectangle::border.width
    \qmlproperty color MapRectangle::border.color

    This property is part of the border property group. The border property group
    holds the width and color used to draw the border of the rectangle.
    The width is in pixels and is independent of the zoom level of the map.

    The default values correspond to a black border with a width of 1 pixel.
    For no line, use a width of 0 or a transparent color.
*/
QDeclarativeMapLineProperties *QDeclarativeRectangleMapItem::border()
{
    return &m_border;
}

/*!
    \qmlproperty coordinate MapRectangle::topLeft

    This property holds the top-left coordinate of the MapRectangle which
    can be used to retrieve its longitude, latitude and altitude.
*/
void QDeclarativeRectangleMapItem::setTopLeft(const QGeoCoordinate &topLeft)
{
    if (m_rectangle.topLeft() == topLeft)
        return;

    m_rectangle.setTopLeft(topLeft);
    m_d->onGeoGeometryChanged();
    emit topLeftChanged(topLeft);
}

QGeoCoordinate QDeclarativeRectangleMapItem::topLeft()
{
    return m_rectangle.topLeft();
}

/*!
    \internal
*/
void QDeclarativeRectangleMapItem::markSourceDirtyAndUpdate()
{
    m_d->markSourceDirtyAndUpdate();
}

void QDeclarativeRectangleMapItem::onLinePropertiesChanged()
{
    m_d->onLinePropertiesChanged();
}

/*!
    \qmlproperty coordinate MapRectangle::bottomRight

    This property holds the bottom-right coordinate of the MapRectangle which
    can be used to retrieve its longitude, latitude and altitude.
*/
void QDeclarativeRectangleMapItem::setBottomRight(const QGeoCoordinate &bottomRight)
{
    if (m_rectangle.bottomRight() == bottomRight)
        return;

    m_rectangle.setBottomRight(bottomRight);
    m_d->onGeoGeometryChanged();
    emit bottomRightChanged(bottomRight);
}

QGeoCoordinate QDeclarativeRectangleMapItem::bottomRight()
{
    return m_rectangle.bottomRight();
}

/*!
    \qmlproperty color MapRectangle::color

    This property holds the fill color of the rectangle. For no fill, use
    a transparent color.
*/
QColor QDeclarativeRectangleMapItem::color() const
{
    return m_color;
}

void QDeclarativeRectangleMapItem::setColor(const QColor &color)
{
    if (m_color == color)
        return;
    m_color = color;
    polishAndUpdate();
    emit colorChanged(m_color);
}

/*!
  \qmlproperty real MapRectangle::opacity

  This property holds the opacity of the item.  Opacity is specified as a
  number between 0 (fully transparent) and 1 (fully opaque).  The default is 1.

  An item with 0 opacity will still receive mouse events. To stop mouse events, set the
  visible property of the item to false.
*/

/*!
    \internal
*/
QSGNode *QDeclarativeRectangleMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    return m_d->updateMapItemPaintNode(oldNode, data);
}

/*!
    \internal
*/
void QDeclarativeRectangleMapItem::updatePolish()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    m_d->updatePolish();
}

/*!
    \internal
*/
void QDeclarativeRectangleMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.width() <= 0 || event.mapSize.height() <= 0)
        return;
    m_d->afterViewportChanged();
}

/*!
    \internal
*/
bool QDeclarativeRectangleMapItem::contains(const QPointF &point) const
{
    return m_d->contains(point);
}

const QGeoShape &QDeclarativeRectangleMapItem::geoShape() const
{
    return m_rectangle;
}

void QDeclarativeRectangleMapItem::setGeoShape(const QGeoShape &shape)
{
    if (shape == m_rectangle)
        return;

    const QGeoRectangle rectangle = m_rectangle.boundingGeoRectangle();
    const bool tlHasChanged = rectangle.topLeft() != m_rectangle.topLeft();
    const bool brHasChanged = rectangle.bottomRight() != m_rectangle.bottomRight();
    m_rectangle = rectangle;

    m_d->onGeoGeometryChanged();
    if (tlHasChanged)
        emit topLeftChanged(m_rectangle.topLeft());
    if (brHasChanged)
        emit bottomRightChanged(m_rectangle.bottomRight());
}

/*!
    \internal
*/
void QDeclarativeRectangleMapItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (!map() || !m_rectangle.isValid() || m_updatingGeometry || newGeometry.topLeft() == oldGeometry.topLeft()) {
        QDeclarativeGeoMapItemBase::geometryChange(newGeometry, oldGeometry);
        return;
    }
    // TODO: change the algorithm to preserve the distances and size
    QGeoCoordinate newCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(newGeometry.center()), false);
    QGeoCoordinate oldCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(oldGeometry.center()), false);
    if (!newCenter.isValid() || !oldCenter.isValid())
        return;
    double offsetLongi = newCenter.longitude() - oldCenter.longitude();
    double offsetLati = newCenter.latitude() - oldCenter.latitude();
    if (offsetLati == 0.0 && offsetLongi == 0.0)
        return;

    m_rectangle.translate(offsetLati, offsetLongi);
    m_d->onItemGeometryChanged();
    emit topLeftChanged(m_rectangle.topLeft());
    emit bottomRightChanged(m_rectangle.bottomRight());

    // Not calling QDeclarativeGeoMapItemBase::geometryChange() as it will be called from a nested
    // call to this function.
}

QDeclarativeRectangleMapItemPrivate::QDeclarativeRectangleMapItemPrivate(QDeclarativeRectangleMapItem &rect)
    : m_rect(rect)
{
}

QDeclarativeRectangleMapItemPrivate::~QDeclarativeRectangleMapItemPrivate()
{
}

QDeclarativeRectangleMapItemPrivateCPU::QDeclarativeRectangleMapItemPrivateCPU(QDeclarativeRectangleMapItem &rect)
    : QDeclarativeRectangleMapItemPrivate(rect)
{
    m_shape = new QQuickShape(&m_rect);
    m_shape->setObjectName("_qt_map_item_shape");
    m_shape->setZ(-1);
    m_shape->setContainsMode(QQuickShape::FillContains);

    m_shapePath = new QQuickShapePath(m_shape);
    m_painterPath = new QDeclarativeGeoMapPainterPath(m_shapePath);

    auto pathElements = m_shapePath->pathElements();
    pathElements.append(&pathElements, m_painterPath);

    auto shapePaths = m_shape->data();
    shapePaths.append(&shapePaths, m_shapePath);
}

QDeclarativeRectangleMapItemPrivateCPU::~QDeclarativeRectangleMapItemPrivateCPU()
{
    delete m_shape;
}

void QDeclarativeRectangleMapItemPrivateCPU::updatePolish()
{
    if (!m_rect.topLeft().isValid() || !m_rect.bottomRight().isValid()) {
        m_geometry.clear();
        m_rect.setWidth(0);
        m_rect.setHeight(0);
        m_shape->setVisible(false);
        return;
    }

    QScopedValueRollback<bool> rollback(m_rect.m_updatingGeometry);
    m_rect.m_updatingGeometry = true;

    QList<QGeoCoordinate> perimeter = QGeoMapItemGeometry::path(m_rect.m_rectangle);

    if (m_rect.referenceSurface() == QLocation::ReferenceSurface::Globe) {
        perimeter = QDeclarativeGeoMapItemUtils::greaterCirclePath(perimeter,
                                                                      QDeclarativeGeoMapItemUtils::ClosedPath);
    }

    const QList<QDoubleVector2D> pathMercator = QGeoMapItemGeometry::pathMercator(perimeter);

    m_geometry.updateSourcePoints(*m_rect.map(), QList<QList<QDoubleVector2D>>{pathMercator},
                                  m_rect.referenceSurface() == QLocation::ReferenceSurface::Globe ? QGeoMapPolygonGeometry::WrapAround :
                                                         QGeoMapPolygonGeometry::Duplicate);
    m_rect.setShapeTriangulationScale(m_shape, m_geometry.maxCoord());

    const bool hasBorder = m_rect.m_border.color().alpha() != 0 && m_rect.m_border.width() > 0;
    m_shapePath->setStrokeColor(hasBorder ? m_rect.m_border.color() : Qt::transparent);
    const float borderWidth = hasBorder ? m_rect.m_border.width() : 0.0f;
    m_shapePath->setStrokeWidth(hasBorder ? borderWidth : -1.0f);
    m_shapePath->setFillColor(m_rect.color());

    const QRectF bb = m_geometry.sourceBoundingBox();
    QPainterPath path = m_geometry.srcPath();
    path.translate(-bb.left() + borderWidth, -bb.top() + borderWidth);
    path.closeSubpath();
    m_painterPath->setPath(path);

    m_rect.setSize(bb.size() + QSize(2 * borderWidth, 2 * borderWidth));
    m_shape->setSize(m_rect.size());
    m_shape->setOpacity(m_rect.zoomLevelOpacity());
    m_shape->setVisible(true);

    m_rect.setPositionOnMap(m_geometry.origin(), -1 * bb.topLeft() + QPointF(borderWidth, borderWidth));
}

QSGNode *QDeclarativeRectangleMapItemPrivateCPU::updateMapItemPaintNode(QSGNode *oldNode,
                                                            QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    delete oldNode;
    if (m_geometry.isScreenDirty()) {
        m_geometry.markClean();
    }
    return nullptr;
}

bool QDeclarativeRectangleMapItemPrivateCPU::contains(const QPointF &point) const
{
    return m_shape->contains(m_rect.mapToItem(m_shape, point));
}

QT_END_NAMESPACE
