// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapitemutils_p.h"
#include "qdeclarativepolygonmapitem_p.h"
#include "qdeclarativepolygonmapitem_p_p.h"

#include <QtCore/QScopedValueRollback>
#include <qnumeric.h>
#include <QPainterPath>

#include <QtLocation/private/qgeomap_p.h>
#include <QtPositioning/private/qlocationutils_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qclipperutils_p.h>
#include <QtPositioning/private/qgeopolygon_p.h>
#include <QtPositioning/private/qwebmercator_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapPolygon
    \nativetype QDeclarativePolygonMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.5

    \brief The MapPolygon type displays a polygon on a Map.

    The MapPolygon type displays a polygon on a Map, specified in terms of an ordered list of
    \l {QtPositioning::coordinate}{coordinates}. For best appearance and results, polygons should be
    simple (not self-intersecting).

    Coordinates can be added or removed at any time using the \l addCoordinate and
    \l removeCoordinate methods. They can also be modified like any other list element in QML:

    \code
    mapPolygon.path[0].latitude = 5;
    \endcode

    For drawing rectangles with "straight" edges (same latitude across one
    edge, same latitude across the other), the \l MapRectangle type provides
    a simpler, two-point API.

    By default, the polygon is displayed as a 1 pixel black border with no
    fill. To change its appearance, use the \l color, \l border.color and
    \l border.width properties.

    \note Since MapPolygons are geographic items, dragging a MapPolygon
    (through the use of \l MouseArea or \l PointHandler) causes its vertices to be
    recalculated in the geographic coordinate space. The edges retain the
    same geographic lengths (latitude and longitude differences between the
    vertices), but they remain straight. Apparent stretching of the item occurs
    when dragged to a different latitude.

    \section2 Example Usage

    The following snippet shows a MapPolygon being used to display a triangle,
    with three vertices near Brisbane, Australia. The triangle is filled in
    green, with a 1 pixel black border.

    \code
    Map {
        MapPolygon {
            color: 'green'
            path: [
                { latitude: -27, longitude: 153.0 },
                { latitude: -27, longitude: 154.1 },
                { latitude: -28, longitude: 153.5 }
            ]
        }
    }
    \endcode

    \image api-mappolygon.png
*/

/*!
    \qmlproperty bool QtLocation::MapPolygon::autoFadeIn

    This property holds whether the item automatically fades in when zooming into the map
    starting from very low zoom levels. By default this is \c true.
    Setting this property to \c false causes the map item to always have the opacity specified
    with the \l QtQuick::Item::opacity property, which is 1.0 by default.

    \since 5.14
*/

/*!
    \qmlproperty enum QtLocation::MapPolygon::referenceSurface

    This property determines the reference surface of the polygon. If it is set to
    \l QLocation::ReferenceSurface::Map the polygons vertices are connected with straight
    lines on the map. If it is set to \l QLocation::ReferenceSurface::Globe, the vertices
    are connected following the great circle path, describing the shortest connection of
    two points on a sphere.
    Default value is \l QLocation::ReferenceSurface::Map.

    \since 6.5
*/

QGeoMapPolygonGeometry::QGeoMapPolygonGeometry() = default;

/*!
    \internal
*/
void QGeoMapPolygonGeometry::updateSourcePoints(const QGeoMap &map,
                                                const QList<QList <QDoubleVector2D>> &basePaths,
                                                MapBorderBehaviour wrapping)
{
    // A polygon consists of mutliple paths. This is usually a perimeter and multiple holes
    // We move all paths into a single QPainterPath. The filling rule EvenOdd will then ensure that the paths are shown correctly
    if (!sourceDirty_)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map.geoProjection());
    srcPath_ = QPainterPath();
    srcOrigin_ = p.mapProjectionToGeo(QDoubleVector2D(0.0, 0.0)); //avoid warning of NaN values if function is returned early
    const QRectF cameraRect = QDeclarativeGeoMapItemUtils::boundingRectangleFromList(p.visibleGeometry());

    QList<QList<QDoubleVector2D>> paths;

    if (wrapping == WrapAround) {
        // 0.1 Wrap the points around the globe if the path makes more sense that way.
        //    Ultimately, this is done if it is closer to walk around the day-border than the other direction
        paths.reserve(basePaths.size());
        for (qsizetype j = 0; j< basePaths.size(); j++) {
            const QList<QDoubleVector2D> &bp = basePaths[j];
            if (bp.isEmpty())
                continue;
            paths << QList<QDoubleVector2D>({bp[0]});
            QList<QDoubleVector2D> &pp = paths[j];
            pp.reserve(bp.size());
            for (qsizetype i = 1; i < bp.size(); i++) {
                if (bp[i].x() > pp.last().x() + 0.5)
                    pp << bp[i] - QDoubleVector2D(1.0, 0.0);
                else if (bp[i].x() < pp.last().x() - 0.5)
                    pp << bp[i] + QDoubleVector2D(1.0, 0.0);
                else
                    pp << bp[i];
            }
        }

        // 0.2 Check and include one of the poles if necessary to make sense out of the polygon
        for (qsizetype j = 0; j < paths.size(); j++) {
            QList<QDoubleVector2D> &pp = paths[j];

            if (pp.last().x() - pp.first().x() < -0.5) {
                for (qsizetype i = 0; i < floor(pp.length()/2.); i++)
                    pp.swapItemsAt(i, pp.length() - i - 1);
            }
            if (pp.last().x() - pp.first().x() > 0.5) {

                const double leftBorder = cameraRect.left();
                const double rightBorder = cameraRect.right();

                qsizetype originalPathLength = pp.length();

                if (pp.last().x() < rightBorder) {
                    for (qsizetype i = 0; i < originalPathLength; i++)
                        pp.append(pp[i] + QDoubleVector2D(1.0, 0.0));
                }
                if (pp.first().x() > leftBorder) {
                    for (qsizetype i = 0; i < originalPathLength; i++)
                        pp.insert(i, pp[2*i] - QDoubleVector2D(1.0, 0.0));
                }
                const double newPoleLat = (pp.first().y() + pp.last().y() < 1.0) ? 0.0 : 1.0; //mean of y < 0.5?
                const QDoubleVector2D P1 = pp.first();
                const QDoubleVector2D P2 = pp.last();
                pp.push_front(QDoubleVector2D(P1.x(), newPoleLat));
                pp.append(QDoubleVector2D(P2.x(), newPoleLat));

                wrapping = DrawOnce;
            }
        }
    } else {
        paths = basePaths;
    }

    //1 The bounding rectangle of the polygon and camera view are compared to determine if the polygon is visible
    //  The viewport is periodic in x-direction in the interval [-1; 1].
    //  The polygon (maybe) has to be ploted periodically too by shifting it by -1 or +1;
    QList<QList<QDoubleVector2D>> wrappedPaths;

    if (wrapping == Duplicate || wrapping == WrapAround) {
        QRectF itemRect;
        for (const auto &path : paths)
            itemRect |= QDeclarativeGeoMapItemUtils::boundingRectangleFromList(path);

        for (double xoffset : {-1.0, 0.0, 1.0}) {
            if (!cameraRect.intersects(itemRect.translated(QPointF(xoffset, 0.0))))
                continue;
            for (const auto &path : paths) {
                wrappedPaths.append(QList<QDoubleVector2D>());
                QList<QDoubleVector2D> &wP = wrappedPaths.last();
                wP.reserve(path.size());
                for (const QDoubleVector2D &coord : path)
                    wP.append(coord+QDoubleVector2D(xoffset, 0.0));
            }
        }
    } else {
        wrappedPaths = paths;
    }

    if (wrappedPaths.isEmpty()) // the polygon boundary rectangle does not overlap with the viewport rectangle
        return;


    //2 The polygons that are at least partially in the viewport are cliped to reduce their size
    QList<QList<QDoubleVector2D>> clippedPaths;
    const QList<QDoubleVector2D> &visibleRegion = p.visibleGeometryExpanded();
    for (const auto &path : wrappedPaths) {
        if (visibleRegion.size()) {
            QClipperUtils clipper;
            clipper.addSubjectPath(path, true);
            clipper.addClipPolygon(visibleRegion);
            clippedPaths << clipper.execute(QClipperUtils::Intersection, QClipperUtils::pftEvenOdd,
                                           QClipperUtils::pftEvenOdd);
        }
        else {
            clippedPaths.append(path); //Do we really need this if there are no visible regions??
        }
    }
    if (clippedPaths.isEmpty()) //the polygon is entirely outside visibleRegion
        return;

    QRectF bb;
    for (const auto &path: clippedPaths)
        bb |= QDeclarativeGeoMapItemUtils::boundingRectangleFromList(path);
    //Offset by origin, find the maximum coordinate
    srcOrigin_ = p.mapProjectionToGeo(QDoubleVector2D(bb.left(), bb.top()));
    QDoubleVector2D origin = p.wrappedMapProjectionToItemPosition(p.geoToWrappedMapProjection(srcOrigin_)); //save way: redo all projections
    maxCoord_ = 0.0;
    for (const auto &path: clippedPaths) {
        QDoubleVector2D prevPoint = p.wrappedMapProjectionToItemPosition(path.at(0)) - origin;
        QDoubleVector2D nextPoint = p.wrappedMapProjectionToItemPosition(path.at(1)) - origin;
        srcPath_.moveTo(prevPoint.toPointF());
        maxCoord_ = qMax(maxCoord_, qMax(prevPoint.x(), prevPoint.y()));
        qsizetype pointsAdded = 1;
        for (qsizetype i = 1; i < path.size(); ++i) {
            const QDoubleVector2D point = nextPoint;

            if (qMax(point.x(), point.y()) > maxCoord_)
                maxCoord_ = qMax(point.x(), point.y());

            if (i == path.size() - 1) {
                srcPath_.lineTo(point.toPointF()); //close the path
            } else {
                nextPoint = p.wrappedMapProjectionToItemPosition(path.at(i+1)) - origin;

                bool addPoint = ( i > pointsAdded * 10 || //make sure that at least every 10th point is drawn
                                  path.size() < 10 );     //draw small paths completely

                const double tolerance = 0.1;
                if (!addPoint) { //add the point to the shape if it deflects the boundary by more than the tolerance
                    const double dsqr = QDeclarativeGeoMapItemUtils::distanceSqrPointLine(
                                            point.x(), point.y(),
                                            nextPoint.x(), nextPoint.y(),
                                            prevPoint.x(), prevPoint.y());
                    addPoint = addPoint || (dsqr > (tolerance*tolerance));
                }

                if (addPoint) {
                    srcPath_.lineTo(point.toPointF());
                    pointsAdded++;
                    prevPoint = point;
                }

            }
        }
        srcPath_.closeSubpath();
    }

    if (!assumeSimple_)
        srcPath_ = srcPath_.simplified();

    sourceBounds_ = srcPath_.boundingRect();
}

/*
 * QDeclarativePolygonMapItem Private Implementations
 */

QDeclarativePolygonMapItemPrivate::~QDeclarativePolygonMapItemPrivate()
{
}

QDeclarativePolygonMapItemPrivateCPU::QDeclarativePolygonMapItemPrivateCPU(QDeclarativePolygonMapItem &polygon)
    : QDeclarativePolygonMapItemPrivate(polygon)
{
    m_shape = new QQuickShape(&m_poly);
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

QDeclarativePolygonMapItemPrivateCPU::~QDeclarativePolygonMapItemPrivateCPU()
{
    delete m_shape;
}

void QDeclarativePolygonMapItemPrivateCPU::updatePolish()
{
    if (m_poly.m_geopoly.perimeter().length() == 0) { // Possibly cleared
        m_geometry.clear();
        m_poly.setWidth(0);
        m_poly.setHeight(0);
        m_shape->setVisible(false);
        return;
    }
    const QGeoMap *map = m_poly.map();
    const qreal borderWidth = m_poly.m_border.width();
    QScopedValueRollback<bool> rollback(m_poly.m_updatingGeometry);
    m_poly.m_updatingGeometry = true;

    m_geometry.updateSourcePoints(*map, m_geopathProjected,
                                  m_poly.referenceSurface() == QLocation::ReferenceSurface::Globe ?
                                    QGeoMapPolygonGeometry::WrapAround :
                                    QGeoMapPolygonGeometry::Duplicate);

    const QRectF bb = m_geometry.sourceBoundingBox();

    m_poly.setShapeTriangulationScale(m_shape, m_geometry.maxCoord());

    const bool hasBorder = m_poly.m_border.color().alpha() != 0 && m_poly.m_border.width() > 0;
    m_shapePath->setStrokeColor(hasBorder ? m_poly.m_border.color() : Qt::transparent);
    m_shapePath->setStrokeWidth(hasBorder ? borderWidth : -1.0f);
    m_shapePath->setFillColor(m_poly.color());

    QPainterPath path = m_geometry.srcPath();
    path.translate(-bb.left() + borderWidth, -bb.top() + borderWidth);
    path.closeSubpath();
    m_painterPath->setPath(path);

    m_poly.setSize(bb.size() + QSize(2 * borderWidth, 2 * borderWidth));
    m_shape->setSize(m_poly.size());
    m_shape->setOpacity(m_poly.zoomLevelOpacity());
    m_shape->setVisible(true);

    m_poly.setPositionOnMap(m_geometry.origin(), -1 * bb.topLeft() + QPointF(borderWidth, borderWidth));
}

QSGNode *QDeclarativePolygonMapItemPrivateCPU::updateMapItemPaintNode(QSGNode *oldNode,
                                                            QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    delete oldNode;
    if (m_geometry.isScreenDirty()) {
        m_geometry.markClean();
    }
    return nullptr;
}

bool QDeclarativePolygonMapItemPrivateCPU::contains(const QPointF &point) const
{
    return m_shape->contains(m_poly.mapToItem(m_shape, point));
}

/*
 * QDeclarativePolygonMapItem Implementation
 */

QDeclarativePolygonMapItem::QDeclarativePolygonMapItem(QQuickItem *parent)
:   QDeclarativeGeoMapItemBase(parent), m_border(this), m_color(Qt::transparent),
    m_updatingGeometry(false)
  , m_d(new QDeclarativePolygonMapItemPrivateCPU(*this))

{
    // ToDo: handle envvar, and switch implementation.
    m_itemType = QGeoMap::MapPolygon;
    m_geopoly = QGeoPolygonEager();
    setFlag(ItemHasContents, true);
    // ToDo: fix this, only flag material?
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::colorChanged,
                     this, &QDeclarativePolygonMapItem::onLinePropertiesChanged);
    QObject::connect(&m_border, &QDeclarativeMapLineProperties::widthChanged,
                     this, &QDeclarativePolygonMapItem::onLinePropertiesChanged);
    QObject::connect(this, &QDeclarativePolygonMapItem::referenceSurfaceChanged,
                     this, [this]() { m_d->onGeoGeometryChanged(); });
}

QDeclarativePolygonMapItem::~QDeclarativePolygonMapItem()
{
}

/*!
    \qmlpropertygroup Location::MapPolygon::border
    \qmlproperty int MapPolygon::border.width
    \qmlproperty color MapPolygon::border.color

    This property is part of the border property group. The border property
    group holds the width and color used to draw the border of the polygon.

    The width is in pixels and is independent of the zoom level of the map.

    The default values correspond to a black border with a width of 1 pixel.
    For no line, use a width of 0 or a transparent color.
*/

QDeclarativeMapLineProperties *QDeclarativePolygonMapItem::border()
{
    return &m_border;
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map)
        m_d->onMapSet();
}

/*!
    \qmlproperty list<coordinate> MapPolygon::path

    This property holds the ordered list of coordinates which
    define the polygon.
    Having less than 3 different coordinates in the path results in undefined behavior.

    \sa addCoordinate, removeCoordinate
*/
QList<QGeoCoordinate> QDeclarativePolygonMapItem::path() const
{
    return m_geopoly.perimeter();
}

void QDeclarativePolygonMapItem::setPath(const QList<QGeoCoordinate> &path)
{
    // Equivalent to QDeclarativePolylineMapItem::setPathFromGeoList
    if (m_geopoly.perimeter() == path)
        return;

    m_geopoly.setPerimeter(path);

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolygon::addCoordinate(coordinate)

    Adds the specified \a coordinate to the path.

    \sa removeCoordinate, path
*/

void QDeclarativePolygonMapItem::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;

    m_geopoly.addCoordinate(coordinate);
    m_d->onGeoGeometryUpdated();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolygon::removeCoordinate(coordinate)

    Removes \a coordinate from the path. If there are multiple instances of the
    same coordinate, the one added last is removed.

    If \a coordinate is not in the path this method does nothing.

    \sa addCoordinate, path
*/
void QDeclarativePolygonMapItem::removeCoordinate(const QGeoCoordinate &coordinate)
{
    int length = m_geopoly.perimeter().length();
    m_geopoly.removeCoordinate(coordinate);
    if (m_geopoly.perimeter().length() == length)
        return;

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlproperty color MapPolygon::color

    This property holds the color used to fill the polygon.

    The default value is transparent.
*/

QColor QDeclarativePolygonMapItem::color() const
{
    return m_color;
}

void QDeclarativePolygonMapItem::setColor(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;
    polishAndUpdate(); // in case color was transparent and now is not or vice versa
    emit colorChanged(m_color);
}

/*!
    \internal
*/
QSGNode *QDeclarativePolygonMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    return m_d->updateMapItemPaintNode(oldNode, data);
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::updatePolish()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    m_d->updatePolish();
}

void QDeclarativePolygonMapItem::markSourceDirtyAndUpdate()
{
    m_d->markSourceDirtyAndUpdate();
}

void QDeclarativePolygonMapItem::onLinePropertiesChanged()
{
    m_d->onLinePropertiesChanged();
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.isEmpty())
        return;

    m_d->afterViewportChanged();
}

/*!
    \internal
*/
bool QDeclarativePolygonMapItem::contains(const QPointF &point) const
{
    return m_d->contains(point);
}

const QGeoShape &QDeclarativePolygonMapItem::geoShape() const
{
    return m_geopoly;
}

void QDeclarativePolygonMapItem::setGeoShape(const QGeoShape &shape)
{
    if (shape == m_geopoly)
        return;

    m_geopoly = QGeoPolygonEager(shape);
    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \internal
*/
void QDeclarativePolygonMapItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.topLeft() == oldGeometry.topLeft() || !map() || !m_geopoly.isValid() || m_updatingGeometry) {
        QDeclarativeGeoMapItemBase::geometryChange(newGeometry, oldGeometry);
        return;
    }
    // TODO: change the algorithm to preserve the distances and size!
    QGeoCoordinate newCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(newGeometry.center()), false);
    QGeoCoordinate oldCenter = map()->geoProjection().itemPositionToCoordinate(QDoubleVector2D(oldGeometry.center()), false);
    if (!newCenter.isValid() || !oldCenter.isValid())
        return;
    double offsetLongi = newCenter.longitude() - oldCenter.longitude();
    double offsetLati = newCenter.latitude() - oldCenter.latitude();
    if (offsetLati == 0.0 && offsetLongi == 0.0)
        return;

    m_geopoly.translate(offsetLati, offsetLongi);
    m_d->onGeoGeometryChanged();
    emit pathChanged();

    // Not calling QDeclarativeGeoMapItemBase::geometryChange() as it will be called from a nested
    // call to this function.
}

//////////////////////////////////////////////////////////////////////

QT_END_NAMESPACE
