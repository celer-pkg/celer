// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativecirclemapitem_p.h"
#include "qdeclarativecirclemapitem_p_p.h"

#include <QtCore/QScopedValueRollback>
#include <QPen>
#include <qgeocircle.h>

#include <QtGui/private/qtriangulator_p.h>
#include <QtLocation/private/qgeomap_p.h>
#include <QtPositioning/private/qlocationutils_p.h>

#include <qmath.h>
#include <algorithm>

#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapCircle
    \nativetype QDeclarativeCircleMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapCircle type displays a geographic circle on a Map.

    The MapCircle type displays a geographic circle on a Map, which
    consists of all points that are within a set distance from one
    central point. Depending on map projection, a geographic circle
    may not always be a perfect circle on the screen: for instance, in
    the Mercator projection, circles become ovoid in shape as they near
    the poles. To display a perfect screen circle around a point, use a
    MapQuickItem containing a relevant Qt Quick type instead.

    By default, the circle is displayed as a 1 pixel black border with
    no fill. To change its appearance, use the \l {color}, \l {border.color}
    and \l {border.width} properties.

    Internally, a MapCircle is implemented as a many-sided polygon. To
    calculate the radius points it uses a spherical model of the Earth,
    similar to the atDistanceAndAzimuth method of the \l {coordinate}
    type. These two things can occasionally have implications for the
    accuracy of the circle's shape, depending on position and map
    projection.

    \note Dragging a MapCircle (through the use of \l {MouseArea} or \l {PointHandler})
    causes new points to be generated at the same distance (in meters)
    from the center. This is in contrast to other map items which store
    their dimensions in terms of latitude and longitude differences between
    vertices.

    \section2 Example Usage

    The following snippet shows a map containing a MapCircle, centered at
    the coordinate (-27, 153) with a radius of 5km. The circle is
    filled in green, with a 3 pixel black border.

    \code
    Map {
        MapCircle {
            center {
                latitude: -27.5
                longitude: 153.0
            }
            radius: 5000.0
            color: 'green'
            border.width: 3
        }
    }
    \endcode

    \image api-mapcircle.png
*/

/*!
    \qmlproperty bool QtLocation::MapCircle::autoFadeIn

    This property holds whether the item automatically fades in when zooming into the map
    starting from very low zoom levels. By default this is \c true.
    Setting this property to \c false causes the map item to always have the opacity specified
    with the \l QtQuick::Item::opacity property, which is 1.0 by default.

    \since 5.14
*/

/*!
    \qmlproperty enum QtLocation::MapCircle::referenceSurface

    This property determines the reference surface of the circle. If it is set to
    \l QLocation::ReferenceSurface::Map the circle is drawn as a circe on the map with
    \l radius approximated to match the map scale at the center of the circle.
    If it is set to \l QLocation::ReferenceSurface::Globe the circle is mapped onto
    a sphere and the great circle path is used to determine the coverage of the circle.
    Default value is \l QLocation::ReferenceSurface::Map.

    \since 6.5
*/

struct Vertex
{
    QVector2D position;
};

QGeoMapCircleGeometry::QGeoMapCircleGeometry()
{
}

QDeclarativeCircleMapItem::QDeclarativeCircleMapItem(QQuickItem *parent)
:   QDeclarativeGeoMapItemBase(parent), m_border(this), m_color(Qt::transparent),
    m_updatingGeometry(false)
  , m_d(new QDeclarativeCircleMapItemPrivateCPU(*this))
{
    // ToDo: handle envvar, and switch implementation.
    m_itemType = QGeoMap::MapCircle;
    setFlag(ItemHasContents, true);
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::colorChanged,
                     this, &QDeclarativeCircleMapItem::onLinePropertiesChanged);
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::widthChanged,
                     this, &QDeclarativeCircleMapItem::onLinePropertiesChanged);
    QObject::connect(this, &QDeclarativeCircleMapItem::referenceSurfaceChanged, this,
                     [this]() {m_d->onGeoGeometryChanged();});
}

QDeclarativeCircleMapItem::~QDeclarativeCircleMapItem()
{
}

/*!
    \qmlpropertygroup Location::MapCircle::border
    \qmlproperty int MapCircle::border.width
    \qmlproperty color MapCircle::border.color

    This property is part of the border group property.
    The border property holds the width and color used to draw the border of the circle.
    The width is in pixels and is independent of the zoom level of the map.

    The default values correspond to a black border with a width of 1 pixel.
    For no line, use a width of 0 or a transparent color.
*/
QDeclarativeMapLineProperties *QDeclarativeCircleMapItem::border()
{
    return &m_border;
}

void QDeclarativeCircleMapItem::markSourceDirtyAndUpdate()
{
    m_d->markSourceDirtyAndUpdate();
}

void QDeclarativeCircleMapItem::onLinePropertiesChanged()
{
    m_d->onLinePropertiesChanged();
}

void QDeclarativeCircleMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map)
        m_d->onMapSet();
}

/*!
    \qmlproperty coordinate MapCircle::center

    This property holds the central point about which the circle is defined.

    \sa radius
*/
void QDeclarativeCircleMapItem::setCenter(const QGeoCoordinate &center)
{
    if (m_circle.center() == center)
        return;

    m_circle.setCenter(center);
    m_d->onGeoGeometryChanged();
    emit centerChanged(center);
}

QGeoCoordinate QDeclarativeCircleMapItem::center()
{
    return m_circle.center();
}

/*!
    \qmlproperty color MapCircle::color

    This property holds the fill color of the circle when drawn. For no fill,
    use a transparent color.
*/
void QDeclarativeCircleMapItem::setColor(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;
    polishAndUpdate(); // in case color was transparent and now is not or vice versa
    emit colorChanged(m_color);
}

QColor QDeclarativeCircleMapItem::color() const
{
    return m_color;
}

/*!
    \qmlproperty real MapCircle::radius

    This property holds the radius of the circle, in meters on the ground.

    \sa center
*/
void QDeclarativeCircleMapItem::setRadius(qreal radius)
{
    if (m_circle.radius() == radius)
        return;

    m_circle.setRadius(radius);
    m_d->onGeoGeometryChanged();
    emit radiusChanged(radius);
}

qreal QDeclarativeCircleMapItem::radius() const
{
    return m_circle.radius();
}

/*!
  \qmlproperty real MapCircle::opacity

  This property holds the opacity of the item.  Opacity is specified as a
  number between 0 (fully transparent) and 1 (fully opaque).  The default is 1.

  An item with 0 opacity will still receive mouse events. To stop mouse events, set the
  visible property of the item to false.
*/

/*!
    \internal
*/
QSGNode *QDeclarativeCircleMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    return m_d->updateMapItemPaintNode(oldNode, data);
}

/*!
    \internal
*/
void QDeclarativeCircleMapItem::updatePolish()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    m_d->updatePolish();
}

/*!
    \internal
*/
void QDeclarativeCircleMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.isEmpty())
        return;

    m_d->afterViewportChanged();
}

/*!
    \internal
*/
bool QDeclarativeCircleMapItem::contains(const QPointF &point) const
{
    return m_d->contains(point);
}

const QGeoShape &QDeclarativeCircleMapItem::geoShape() const
{
    return m_circle;
}

void QDeclarativeCircleMapItem::setGeoShape(const QGeoShape &shape)
{
    if (shape == m_circle)
        return;

    const QGeoCircle circle(shape); // if shape isn't a circle, circle will be created as a default-constructed circle
    const bool centerHasChanged = circle.center() != m_circle.center();
    const bool radiusHasChanged = circle.radius() != m_circle.radius();
    m_circle = circle;

    m_d->onGeoGeometryChanged();
    if (centerHasChanged)
        emit centerChanged(m_circle.center());
    if (radiusHasChanged)
        emit radiusChanged(m_circle.radius());
}

/*!
    \internal
*/
void QDeclarativeCircleMapItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (!map() || !m_circle.isValid() || m_updatingGeometry || newGeometry == oldGeometry) {
        QDeclarativeGeoMapItemBase::geometryChange(newGeometry, oldGeometry);
        return;
    }

    QDoubleVector2D newPoint = QDoubleVector2D(x(),y()) + QDoubleVector2D(width(), height()) * 0.5;
    QGeoCoordinate newCoordinate = map()->geoProjection().itemPositionToCoordinate(newPoint, false);
    if (newCoordinate.isValid())
        setCenter(newCoordinate); // ToDo: this is incorrect. setting such center might yield to another geometry changed.

    // Not calling QDeclarativeGeoMapItemBase::geometryChange() as it will be called from a nested
    // call to this function.
}

QDeclarativeCircleMapItemPrivate::~QDeclarativeCircleMapItemPrivate()
{
}

QDeclarativeCircleMapItemPrivateCPU::QDeclarativeCircleMapItemPrivateCPU(QDeclarativeCircleMapItem &circle)
    : QDeclarativeCircleMapItemPrivate(circle)
{
    m_shape = new QQuickShape(&m_circle);
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

QDeclarativeCircleMapItemPrivateCPU::~QDeclarativeCircleMapItemPrivateCPU()
{
    delete m_shape;
}

/*
 * A workaround for circle path to be drawn correctly using a polygon geometry
 * This method generates a polygon like
 *  ______________
 *  |    ____    |
 *   \__/    \__/
 */
void QDeclarativeCircleMapItemPrivate::includeOnePoleInPath(QList<QDoubleVector2D> &path,
                                                             const QGeoCoordinate &center,
                                                             qreal distance, const QGeoProjectionWebMercator &p)
{
    const qreal poleLat = 90;
    const qreal distanceToNorthPole = center.distanceTo(QGeoCoordinate(poleLat, 0));
    const qreal distanceToSouthPole = center.distanceTo(QGeoCoordinate(-poleLat, 0));
    const bool crossNorthPole = distanceToNorthPole < distance;
    const bool crossSouthPole = distanceToSouthPole < distance;

    if (!crossNorthPole && !crossSouthPole)
        return;

    if (crossNorthPole && crossSouthPole)
        return;

    const QRectF cameraRect = QDeclarativeGeoMapItemUtils::boundingRectangleFromList(p.visibleGeometry());
    const qreal xAtBorder = cameraRect.left();

    // The strategy is to order the points from left to right as they appear on the screen.
    // Then add the 3 missing sides that form the box for painting at the front and at the end of the list.
    // We ensure that the box aligns with the cameraRect in order to avoid rendering it twice (wrap around).
    // Notably, this leads to outlines at the right side of the map.
    // Set xAtBorder to 0.0 to avoid this, however, for an increased rendering cost.
    for (auto &c : path) {
        c.setX(c.x());
        while (c.x() - xAtBorder > 1.0)
            c.setX(c.x() - 1.0);
        while (c.x() - xAtBorder < 0.0)
            c.setX(c.x() + 1.0);
    }

    std::sort(path.begin(), path.end(),
              [](const QDoubleVector2D &a, const QDoubleVector2D &b) -> bool
                {return a.x() < b.x();});

    const qreal newPoleLat = crossNorthPole ? -0.1 : 1.1;
    const QDoubleVector2D P1 = path.first() + QDoubleVector2D(1.0, 0.0);
    const QDoubleVector2D P2 = path.last() - QDoubleVector2D(1.0, 0.0);
    path.push_front(P2);
    path.push_front(QDoubleVector2D(P2.x(), newPoleLat));
    path.append(P1);
    path.append(QDoubleVector2D(P1.x(), newPoleLat));
}

int QDeclarativeCircleMapItemPrivate::crossEarthPole(const QGeoCoordinate &center, qreal distance)
{
    qreal poleLat = 90;
    QGeoCoordinate northPole = QGeoCoordinate(poleLat, center.longitude());
    QGeoCoordinate southPole = QGeoCoordinate(-poleLat, center.longitude());
    // approximate using great circle distance
    qreal distanceToNorthPole = center.distanceTo(northPole);
    qreal distanceToSouthPole = center.distanceTo(southPole);
    return (distanceToNorthPole < distance? 1 : 0) +
           (distanceToSouthPole < distance? 1 : 0);
}

void QDeclarativeCircleMapItemPrivate::calculatePeripheralPointsSimple(QList<QDoubleVector2D> &path,
                                      const QGeoCoordinate &center,
                                      qreal distance,
                                      const QGeoProjectionWebMercator &p,
                                      int steps)
{
    const double lambda = 0.0001;
    const QDoubleVector2D c = p.geoToMapProjection(center);
    const double lambda_geo = center.distanceTo(p.mapProjectionToGeo(c + QDoubleVector2D(lambda, 0)));
    const qreal mapDistance = distance * lambda / lambda_geo;

    for (int i = 0; i < steps; ++i) {
        const qreal rad = 2 * M_PI * i / steps;
        path << c + QDoubleVector2D(cos(rad), sin(rad)) * mapDistance;
    }
}

void QDeclarativeCircleMapItemPrivate::calculatePeripheralPointsGreatCircle(QList<QDoubleVector2D> &path,
                                      const QGeoCoordinate &center,
                                      qreal distance,
                                      const QGeoProjectionWebMercator &p,
                                      int steps)
{
    // Calculate points based on great-circle distance
    // Calculation is the same as GeoCoordinate's atDistanceAndAzimuth function
    // but tweaked here for computing multiple points

    // pre-calculations
    steps = qMax(steps, 3);
    qreal centerLon = center.longitude();
    qreal latRad = QLocationUtils::radians(center.latitude());
    qreal lonRad = QLocationUtils::radians(centerLon);
    qreal cosLatRad = std::cos(latRad);
    qreal sinLatRad = std::sin(latRad);
    qreal ratio = (distance / QLocationUtils::earthMeanRadius());
    qreal cosRatio = std::cos(ratio);
    qreal sinRatio = std::sin(ratio);
    qreal sinLatRad_x_cosRatio = sinLatRad * cosRatio;
    qreal cosLatRad_x_sinRatio = cosLatRad * sinRatio;
    for (int i = 0; i < steps; ++i) {
        const qreal azimuthRad = 2 * M_PI * i / steps;
        const qreal resultLatRad = std::asin(sinLatRad_x_cosRatio
                                 + cosLatRad_x_sinRatio * std::cos(azimuthRad));
        const qreal resultLonRad = lonRad + std::atan2(std::sin(azimuthRad) * cosLatRad_x_sinRatio,
                                   cosRatio - sinLatRad * std::sin(resultLatRad));
        const qreal lat2 = QLocationUtils::degrees(resultLatRad);
        qreal lon2 =  QLocationUtils::degrees(resultLonRad);

        //Workaround as QGeoCoordinate does not take Longitudes outside [-180,180]
        qreal offset = 0.0;
        while (lon2 > 180.0) {
            offset += 1.0;
            lon2 -= 360.0;
        }
        while (lon2 < -180.0) {
            offset -= 1.0;
            lon2 += 360.0;
        }
        path << p.geoToMapProjection(QGeoCoordinate(lat2, lon2, center.altitude())) + QDoubleVector2D(offset, 0.0);
    }
}

//////////////////////////////////////////////////////////////////////

void QDeclarativeCircleMapItemPrivateCPU::updatePolish()
{
    if (!m_circle.m_circle.isValid()) {
        m_geometry.clear();
        m_circle.setWidth(0);
        m_circle.setHeight(0);
        m_shape->setVisible(false);
        return;
    }

    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_circle.map()->geoProjection());
    QScopedValueRollback<bool> rollback(m_circle.m_updatingGeometry);
    m_circle.m_updatingGeometry = true;

    QList<QDoubleVector2D> circlePath = m_circlePath;

    const QGeoCoordinate &center = m_circle.m_circle.center();
    const qreal &radius = m_circle.m_circle.radius();

    // if circle crosses north/south pole, then don't preserve circular shape,
    int crossingPoles = m_circle.referenceSurface() == QLocation::ReferenceSurface::Globe ? crossEarthPole(center, radius) : 0;
    if (crossingPoles == 1) { // If the circle crosses both poles, we will remove it from a rectangle
        includeOnePoleInPath(circlePath, center, radius, p);
        m_geometry.updateSourcePoints(*m_circle.map(), QList<QList<QDoubleVector2D>>{circlePath}, QGeoMapPolygonGeometry::DrawOnce);
    }
    else if (crossingPoles == 2) { // If the circle crosses both poles, we will remove it from a rectangle
        // The circle covers both poles. This appears on the map as a total fill with a hole on the opposite side of the planet
        // This can be represented by a rectangle that spans the entire planet with a hole defined by the calculated points.
        // The points on one side have to be wraped around the globe
        const qreal centerX = p.geoToMapProjection(center).x();
        for (int i = 0; i < circlePath.count(); i++) {
            if (circlePath.at(i).x() > centerX)
                circlePath[i].setX(circlePath.at(i).x() - 1.0);
        }
        QRectF cameraRect = QDeclarativeGeoMapItemUtils::boundingRectangleFromList(p.visibleGeometry());
        const QRectF circleRect = QDeclarativeGeoMapItemUtils::boundingRectangleFromList(circlePath);
        QGeoMapPolygonGeometry::MapBorderBehaviour wrappingMode = QGeoMapPolygonGeometry::DrawOnce;
        QList<QDoubleVector2D> surroundingRect;
        if (cameraRect.contains(circleRect)){
            cameraRect = cameraRect.adjusted(-0.1, -0.1, 0.2, 0.2);
            surroundingRect = {{cameraRect.left(), cameraRect.top()}, {cameraRect.right(), cameraRect.top()},
                               {cameraRect.right(), cameraRect.bottom()}, {cameraRect.left() , cameraRect.bottom()}};
        } else {
            const qreal anchorRect = centerX;

            surroundingRect = {{anchorRect, -0.1}, {anchorRect + 1.0, -0.1},
                               {anchorRect + 1.0, 1.1}, {anchorRect, 1.1}};
            wrappingMode = QGeoMapPolygonGeometry::Duplicate;
        }
        m_geometry.updateSourcePoints(*m_circle.map(), {surroundingRect, circlePath}, wrappingMode);
    } else {
        m_geometry.updateSourcePoints(*m_circle.map(), QList<QList<QDoubleVector2D>>{circlePath});
    }

    m_circle.setShapeTriangulationScale(m_shape, m_geometry.maxCoord());

    const bool hasBorder = m_circle.m_border.color().alpha() != 0 && m_circle.m_border.width() > 0;
    const float borderWidth = hasBorder ? m_circle.m_border.width() : 0.0f;
    m_shapePath->setStrokeColor(hasBorder ? m_circle.m_border.color() : Qt::transparent);
    m_shapePath->setStrokeWidth(hasBorder ? borderWidth : -1.0f);
    m_shapePath->setFillColor(m_circle.color());

    const QRectF bb = m_geometry.sourceBoundingBox();
    QPainterPath path = m_geometry.srcPath();
    path.translate(-bb.left() + borderWidth, -bb.top() + borderWidth);
    path.closeSubpath();
    m_painterPath->setPath(path);

    m_circle.setSize(bb.size());
    m_shape->setSize(m_circle.size());
    m_shape->setOpacity(m_circle.zoomLevelOpacity());
    m_shape->setVisible(true);

    m_circle.setPositionOnMap(m_geometry.origin(), -1 * bb.topLeft() + QPointF(borderWidth, borderWidth));
}

QSGNode *QDeclarativeCircleMapItemPrivateCPU::updateMapItemPaintNode(QSGNode *oldNode,
                                                            QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    delete oldNode;
    if (m_geometry.isScreenDirty()) {
        m_geometry.markClean();
    }
    return nullptr;
}

bool QDeclarativeCircleMapItemPrivateCPU::contains(const QPointF &point) const
{
    return m_shape->contains(m_circle.mapToItem(m_shape, point));
}

QT_END_NAMESPACE
