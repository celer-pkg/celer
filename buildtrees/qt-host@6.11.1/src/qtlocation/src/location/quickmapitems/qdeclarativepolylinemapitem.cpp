// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativepolylinemapitem_p.h"
#include "qdeclarativepolylinemapitem_p_p.h"

#include <QtCore/QScopedValueRollback>
#include <qnumeric.h>
#include <QPainterPath>

#include <QtGui/private/qtriangulatingstroker_p.h>
#include <QtPositioning/private/qlocationutils_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qclipperutils_p.h>
#include <QtPositioning/private/qgeopath_p.h>
#include <QtLocation/private/qgeomap_p.h>

#include <array>

QT_BEGIN_NAMESPACE


static bool get_line_intersection(const double p0_x,
                                 const double p0_y,
                                 const double p1_x,
                                 const double p1_y,
                                 const double p2_x,
                                 const double p2_y,
                                 const double p3_x,
                                 const double p3_y,
                                 double *i_x,
                                 double *i_y,
                                 double *i_t)
{
    const double s10_x = p1_x - p0_x;
    const double s10_y = p1_y - p0_y;
    const double s32_x = p3_x - p2_x;
    const double s32_y = p3_y - p2_y;

    const double denom = s10_x * s32_y - s32_x * s10_y;
    if (denom == 0.0)
        return false; // Collinear
    const bool denomPositive = denom > 0;

    const double s02_x = p0_x - p2_x;
    const double s02_y = p0_y - p2_y;
    const double s_numer = s10_x * s02_y - s10_y * s02_x;
    if ((s_numer < 0.0) == denomPositive)
        return false; // No collision

    const double t_numer = s32_x * s02_y - s32_y * s02_x;
    if ((t_numer < 0.0) == denomPositive)
        return false; // No collision

    if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
        return false; // No collision
    // Collision detected
    *i_t = t_numer / denom;
    *i_x = p0_x + (*i_t * s10_x);
    *i_y = p0_y + (*i_t * s10_y);

    return true;
}

enum SegmentType {
    NoIntersection,
    OneIntersection,
    TwoIntersections
};

static QList<QList<QDoubleVector2D> > clipLine(
        const QList<QDoubleVector2D> &l,
        const QList<QDoubleVector2D> &poly)
{
    QList<QList<QDoubleVector2D> > res;
    if (poly.size() < 2 || l.size() < 2)
        return res;

    // Step 1: build edges
    std::vector<std::array<double, 4> > edges;
    for (qsizetype i = 1; i < poly.size(); i++)
        edges.push_back({ { poly.at(i-1).x(), poly.at(i-1).y(), poly.at(i).x(), poly.at(i).y() } });
    edges.push_back({ { poly.at(poly.size()-1).x(), poly.at(poly.size()-1).y(), poly.at(0).x(), poly.at(0).y() } });

    // Step 2: check each segment against each edge
    QList<QDoubleVector2D> subLine;
    std::array<double, 4> intersections = { { 0.0, 0.0, 0.0, 0.0 } };

    for (qsizetype i = 0; i < l.size() - 1; ++i) {
        SegmentType type = NoIntersection;
        double t = -1; // valid values are in [0, 1]. Only written if intersects
        double previousT = t;
        double i_x, i_y;

        const int firstContained = QClipperUtils::pointInPolygon(l.at(i), poly);
        const int secondContained = QClipperUtils::pointInPolygon(l.at(i+1), poly);

        if (firstContained && secondContained) { // Second most common condition, test early and skip inner loop if possible
            if (!subLine.size())
                subLine.push_back(l.at(i)); // the initial element has to be pushed now.
            subLine.push_back(l.at(i+1));
            continue;
        }

        for (unsigned int j = 0; j < edges.size(); ++j) {
            const bool intersects = get_line_intersection(l.at(i).x(),
                                                         l.at(i).y(),
                                                         l.at(i+1).x(),
                                                         l.at(i+1).y(),
                                                         edges.at(j).at(0),
                                                         edges.at(j).at(1),
                                                         edges.at(j).at(2),
                                                         edges.at(j).at(3),
                                                         &i_x,
                                                         &i_y,
                                                         &t);
            if (intersects) {
                if (previousT >= 0.0) { //One intersection already hit
                    if (t < previousT) { // Reorder
                        intersections[2] = intersections[0];
                        intersections[3] = intersections[1];
                        intersections[0] = i_x;
                        intersections[1] = i_y;
                    } else {
                        intersections[2] = i_x;
                        intersections[3] = i_y;
                    }

                    type = TwoIntersections;
                    break; // no need to check anything else
                } else { // First intersection
                    intersections[0] = i_x;
                    intersections[1] = i_y;
                    type = OneIntersection;
                }
                previousT = t;
            }
        }

        if (type == NoIntersection) {
            if (!firstContained && !secondContained) { // Both outside
                subLine.clear();
            } else if (firstContained && secondContained) {
                // Handled above already.
            } else { // Mismatch between PointInPolygon and get_line_intersection. Treat it as no intersection
                if (subLine.size())
                    res.push_back(subLine);
                subLine.clear();
            }
        } else if (type == OneIntersection) { // Need to check the following cases to avoid mismatch with PointInPolygon result.
            if (firstContained <= 0 && secondContained > 0) { // subLine MUST be empty
                if (!subLine.size())
                    subLine.push_back(QDoubleVector2D(intersections[0], intersections[1]));
                subLine.push_back(l.at(i+1));
            } else if (firstContained > 0 && secondContained <= 0) { // subLine MUST NOT be empty
                if (!subLine.size())
                    subLine.push_back(l.at(i));
                subLine.push_back(QDoubleVector2D(intersections[0], intersections[1]));
                res.push_back(subLine);
                subLine.clear();
            } else {
                if (subLine.size())
                    res.push_back(subLine);
                subLine.clear();
            }
        } else { // Two
            // restart strip
            subLine.clear();
            subLine.push_back(QDoubleVector2D(intersections[0], intersections[1]));
            subLine.push_back(QDoubleVector2D(intersections[2], intersections[3]));
            res.push_back(subLine);
            subLine.clear();
        }
    }

    if (subLine.size())
        res.push_back(subLine);
    return res;
}

/*!
    \qmltype MapPolyline
    \nativetype QDeclarativePolylineMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.0

    \brief The MapPolyline type displays a polyline on a map.

    The MapPolyline type displays a polyline on a map, specified in terms of an ordered list of
    \l {coordinate}{coordinates}.

    Coordinates can be added or removed at any time using the \l addCoordinate and
    \l removeCoordinate methods. They can also be modified like any other list element in QML:

    \code
    mapPolyline.path[0].latitude = 5;
    \endcode

    By default, the polyline is displayed as a 1-pixel thick black line. This
    can be changed using the \l line.width and \l line.color properties.

    \section2 Example Usage

    The following snippet shows a MapPolyline with 4 points, making a shape
    like the top part of a "question mark" (?), near Brisbane, Australia.
    The line drawn is 3 pixels in width and green in color.

    \code
    Map {
        MapPolyline {
            line.width: 3
            line.color: 'green'
            path: [
                { latitude: -27, longitude: 153.0 },
                { latitude: -27, longitude: 154.1 },
                { latitude: -28, longitude: 153.5 },
                { latitude: -29, longitude: 153.5 }
            ]
        }
    }
    \endcode

    \image api-mappolyline.png
*/

/*!
    \qmlproperty bool QtLocation::MapPolyline::autoFadeIn

    This property holds whether the item automatically fades in when zooming into the map
    starting from very low zoom levels. By default this is \c true.
    Setting this property to \c false causes the map item to always have the opacity specified
    with the \l QtQuick::Item::opacity property, which is 1.0 by default.

    \since 5.14
*/

/*!
    \qmlproperty enum QtLocation::MapPolyline::referenceSurface

    This property determines the reference surface of the polyline. If it is set to
    \l QLocation::ReferenceSurface::Map the polylines vertices are connected with straight
    lines on the map. If it is set to \l QLocation::ReferenceSurface::Globe, the vertices
    are connected following the great circle path, describing the shortest connection of
    two points on a sphere.
    Default value is \l QLocation::ReferenceSurface::Map.

    \since 6.5
*/

QDeclarativeMapLineProperties::QDeclarativeMapLineProperties(QObject *parent)
    : QObject(parent)
{
}

/*!
    \internal
*/
QColor QDeclarativeMapLineProperties::color() const
{
    return color_;
}

/*!
    \internal
*/
void QDeclarativeMapLineProperties::setColor(const QColor &color)
{
    if (color_ == color)
        return;

    color_ = color;
    emit colorChanged(color_);
}

/*!
    \internal
*/
qreal QDeclarativeMapLineProperties::width() const
{
    return width_;
}

/*!
    \internal
*/
void QDeclarativeMapLineProperties::setWidth(qreal width)
{
    if (width_ == width)
        return;

    width_ = width;
    emit widthChanged(width_);
}

/*!
    \internal
*/
void QGeoMapPolylineGeometry::updateSourcePoints(const QGeoMap &map,
                                                 const QList<QDoubleVector2D> &basePath)
{
    // A polygon consists of mutliple paths. This is usually a perimeter and multiple holes
    // We move all paths into a single QPainterPath. The filling rule EvenOdd will then ensure that the paths are shown correctly
    if (!sourceDirty_)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map.geoProjection());
    srcPath_ = QPainterPath();
    srcOrigin_ = p.mapProjectionToGeo(QDoubleVector2D(0, 0)); //avoid warning of NaN values if function is returned early

    //0 Wrap the points around the globe if the path makes more sense that way.
    //  Ultimately, this is done if it is closer to walk around the day-border than the other direction
    QVarLengthArray<QList<QDoubleVector2D>, 3> wrappedPaths;
    wrappedPaths << QList<QDoubleVector2D>({basePath[0]});
    wrappedPaths.last().reserve(basePath.size());
    for (int i = 1; i < basePath.size(); i++) {
        if (basePath[i].x() > wrappedPaths.last().last().x() + 0.5)
            wrappedPaths.last() << basePath[i] - QDoubleVector2D(1.0, 0.0);
        else if (basePath[i].x() < wrappedPaths.last().last().x() - 0.5)
            wrappedPaths.last() << basePath[i] + QDoubleVector2D(1.0, 0.0);
        else
            wrappedPaths.last() << basePath[i];
    }

    //1 The bounding rectangle of the polygon and camera view are compared to determine if the polygon is visible
    //  The viewport is periodic in x-direction in the interval [-1; 1].
    //  The polygon (maybe) has to be ploted periodically too by shifting it by -1 or +1;
    const QRectF cameraRect = QDeclarativeGeoMapItemUtils::boundingRectangleFromList(p.visibleGeometry());
    QRectF itemRect;
    for (const auto &path : wrappedPaths)
        itemRect |= QDeclarativeGeoMapItemUtils::boundingRectangleFromList(path).adjusted(-1e-6, -1e-6, 2e-6, 2e-6); //TODO: Maybe use linewidth?
    for (double xoffset : {-1.0, 1.0}) {
        if (!cameraRect.intersects(itemRect.translated(QPointF(xoffset,0))))
            continue;
        wrappedPaths.append(QList<QDoubleVector2D>());
        QList<QDoubleVector2D> &wP = wrappedPaths.last();
        wP.reserve(wrappedPaths.first().size());
        for (const QDoubleVector2D &coord : wrappedPaths.first())
            wP.append(coord + QDoubleVector2D(xoffset, 0.0));
    }
    if (wrappedPaths.isEmpty()) // the polygon boundary rectangle does not overlap with the viewport rectangle
        return;

    //2 The polygons that are at least partially in the viewport are cliped to reduce their size
    QList<QList<QDoubleVector2D>> clippedPaths;
    const QList<QDoubleVector2D> &visibleRegion = p.visibleGeometryExpanded();
    for (const auto &path : wrappedPaths) {
        if (visibleRegion.size()) {
            clippedPaths << clipLine(path, visibleRegion);
            //TODO: Replace clipping with Clipper lib, similar to QPolygonMapItem
        } else {
            clippedPaths.append(path); //Do we really need this if there are no visible regions??
        }
    }
    if (clippedPaths.isEmpty()) //the polygon is entirely outside visibleRegion
        return;

    QRectF bb;
    for (const auto &path: clippedPaths)
        bb |= QDeclarativeGeoMapItemUtils::boundingRectangleFromList(path);
    //Offset by origin, find the maximum coordinate
    maxCoord_ = 0.0;
    srcOrigin_ = p.mapProjectionToGeo(QDoubleVector2D(bb.left(), bb.top()));
    QDoubleVector2D origin = p.wrappedMapProjectionToItemPosition(p.geoToWrappedMapProjection(srcOrigin_)); //save way: redo all projections
    for (const auto &path: clippedPaths) {
        QDoubleVector2D lastAddedPoint;
        for (qsizetype i = 0; i < path.size(); ++i) {
            QDoubleVector2D point = p.wrappedMapProjectionToItemPosition(path.at(i));
            point = point - origin; // (0,0) if point == origin

            if (qMax(point.x(), point.y()) > maxCoord_)
                maxCoord_ = qMax(point.x(), point.y());

            if (i == 0) {
                srcPath_.moveTo(point.toPointF());
                lastAddedPoint = point;
            } else {
                if ((point - lastAddedPoint).manhattanLength() > 3 ||
                        i == path.size() - 1) {
                    srcPath_.lineTo(point.toPointF());
                    lastAddedPoint = point;
                }
            }
        }
    }

    sourceBounds_ = srcPath_.boundingRect();
}

/*
 * QDeclarativePolygonMapItem Private Implementations
 */

QDeclarativePolylineMapItemPrivate::~QDeclarativePolylineMapItemPrivate()
{
}

QDeclarativePolylineMapItemPrivateCPU::QDeclarativePolylineMapItemPrivateCPU(QDeclarativePolylineMapItem &poly)
    : QDeclarativePolylineMapItemPrivate(poly)
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

QDeclarativePolylineMapItemPrivateCPU::~QDeclarativePolylineMapItemPrivateCPU()
{
    delete m_shape;
}

void QDeclarativePolylineMapItemPrivateCPU::regenerateCache()
{
    if (!m_poly.map() || m_poly.map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_poly.map()->geoProjection());
    m_geopathProjected.clear();
    if (m_poly.referenceSurface() == QLocation::ReferenceSurface::Globe) {
        const QList<QGeoCoordinate> realPath = QDeclarativeGeoMapItemUtils::greaterCirclePath(m_poly.m_geopath.path());
        m_geopathProjected.reserve(realPath.size());
        for (const QGeoCoordinate &c : realPath)
            m_geopathProjected << p.geoToMapProjection(c);
    } else {
        m_geopathProjected.reserve(m_poly.m_geopath.path().size());
        const QList<QGeoCoordinate> path = m_poly.m_geopath.path();
        for (const QGeoCoordinate &c : path)
            m_geopathProjected << p.geoToMapProjection(c);
    }
}

void QDeclarativePolylineMapItemPrivateCPU::updateCache()
{
    if (!m_poly.map() || m_poly.map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_poly.map()->geoProjection());
    m_geopathProjected << p.geoToMapProjection(m_poly.m_geopath.path().last());
}

void QDeclarativePolylineMapItemPrivateCPU::updatePolish()
{
    if (m_poly.m_geopath.path().length() < 2) { // Possibly cleared
        m_geometry.clear();
        m_poly.setWidth(0);
        m_poly.setHeight(0);
        m_shape->setVisible(false);
        return;
    }
    QScopedValueRollback<bool> rollback(m_poly.m_updatingGeometry);
    m_poly.m_updatingGeometry = true;

    const QGeoMap *map = m_poly.map();
    const qreal borderWidth = m_poly.m_line.width();

    m_geometry.updateSourcePoints(*map, m_geopathProjected);

    const QRectF bb = m_geometry.sourceBoundingBox();
    m_poly.setSize(bb.size() + QSizeF(borderWidth, borderWidth));
    // it has to be shifted so that the center of the line is on the correct geocoord
    m_poly.setPositionOnMap(m_geometry.origin(), -1 * bb.topLeft() + QPointF(borderWidth, borderWidth) * 0.5);


    m_poly.setShapeTriangulationScale(m_shape, m_geometry.maxCoord_);

    m_shapePath->setStrokeColor(m_poly.m_line.color());
    m_shapePath->setStrokeWidth(borderWidth);
    m_shapePath->setFillColor(Qt::transparent);

    QPainterPath path = m_geometry.srcPath();
    path.translate(-bb.left() + borderWidth * 0.5, -bb.top() + borderWidth * 0.5);
    m_painterPath->setPath(path);

    m_shape->setSize(m_poly.size());
    m_shape->setOpacity(m_poly.zoomLevelOpacity());
    m_shape->setVisible(true);
}

QSGNode *QDeclarativePolylineMapItemPrivateCPU::updateMapItemPaintNode(QSGNode *oldNode,
                                                        QQuickItem::UpdatePaintNodeData * /*data*/)
{
    delete oldNode;

    if (m_geometry.isScreenDirty() || !oldNode) {
        m_geometry.markClean();
    }
    return nullptr;
}

bool QDeclarativePolylineMapItemPrivateCPU::contains(const QPointF &point) const
{
    // With Shapes, do not just call
    // m_shape->contains(m_poly.mapToItem(m_shape, point)) because that can
    // only do FillContains at best, whereas the polyline relies on stroking.

    const QPainterPath &path = m_shapePath->path();
    const double &lineWidth = m_poly.m_line.width();
    const QPointF p = m_poly.mapToItem(m_shape, point);

    for (int i = 1; i < path.elementCount(); i++) {
        if (path.elementAt(i).type == QPainterPath::MoveToElement)
            continue;
        const double dsqr = QDeclarativeGeoMapItemUtils::distanceSqrPointLine(p.x(), p.y(),
                                path.elementAt(i - 1).x, path.elementAt(i - 1).y,
                                path.elementAt(i).x, path.elementAt(i).y);
        if (dsqr < 0.25 * lineWidth * lineWidth)
            return true;
    }
    return false;
}

/*
 * QDeclarativePolygonMapItem Implementation
 */

QDeclarativePolylineMapItem::QDeclarativePolylineMapItem(QQuickItem *parent)
    : QDeclarativeGeoMapItemBase(parent), m_line(this),
      m_d(new QDeclarativePolylineMapItemPrivateCPU(*this))
{
    m_itemType = QGeoMap::MapPolyline;
    m_geopath = QGeoPathEager();
    setFlag(ItemHasContents, true);
    QObject::connect(&m_line, &QDeclarativeMapLineProperties::colorChanged,
                     this, &QDeclarativePolylineMapItem::updateAfterLinePropertiesChanged);
    QObject::connect(&m_line, &QDeclarativeMapLineProperties::widthChanged,
                     this, &QDeclarativePolylineMapItem::updateAfterLinePropertiesChanged);
    QObject::connect(this, &QDeclarativePolylineMapItem::referenceSurfaceChanged,
                     this, [this]() { m_d->onGeoGeometryChanged(); });
}

QDeclarativePolylineMapItem::~QDeclarativePolylineMapItem()
{
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::updateAfterLinePropertiesChanged()
{
    m_d->onLinePropertiesChanged();
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map)
        m_d->onMapSet();
}

/*!
    \qmlproperty list<coordinate> MapPolyline::path

    This property holds the ordered list of coordinates which
    define the polyline.
*/

QList<QGeoCoordinate> QDeclarativePolylineMapItem::path() const
{
    return m_geopath.path();
}

void QDeclarativePolylineMapItem::setPath(const QList<QGeoCoordinate> &value)
{
    setPathFromGeoList(value);
}

/*!
    \qmlmethod void MapPolyline::setPath(geoPath path)

    Sets the \a path using a geoPath type.

    \since 5.10

    \sa path
*/
void QDeclarativePolylineMapItem::setPath(const QGeoPath &path)
{
    if (m_geopath.path() == path.path())
        return;

    m_geopath = QGeoPathEager(path);
    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::setPathFromGeoList(const QList<QGeoCoordinate> &path)
{
    if (m_geopath.path() == path)
        return;

    m_geopath.setPath(path);

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlmethod int MapPolyline::pathLength()

    Returns the number of coordinates of the polyline.

    \since QtLocation 5.6

    \sa path
*/
int QDeclarativePolylineMapItem::pathLength() const
{
    return m_geopath.path().length();
}

/*!
    \qmlmethod void MapPolyline::addCoordinate(coordinate)

    Adds the specified \a coordinate to the end of the path.

    \sa insertCoordinate, removeCoordinate, path
*/
void QDeclarativePolylineMapItem::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;

    m_geopath.addCoordinate(coordinate);

    m_d->onGeoGeometryUpdated();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolyline::insertCoordinate(index, coordinate)

    Inserts a \a coordinate to the path at the given \a index.

    \since QtLocation 5.6

    \sa addCoordinate, removeCoordinate, path
*/
void QDeclarativePolylineMapItem::insertCoordinate(int index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index > m_geopath.path().length())
        return;

    m_geopath.insertCoordinate(index, coordinate);

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolyline::replaceCoordinate(index, coordinate)

    Replaces the coordinate in the current path at the given \a index
    with the new \a coordinate.

    \since QtLocation 5.6

    \sa addCoordinate, insertCoordinate, removeCoordinate, path
*/
void QDeclarativePolylineMapItem::replaceCoordinate(int index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index >= m_geopath.path().length())
        return;

    m_geopath.replaceCoordinate(index, coordinate);

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlmethod coordinate MapPolyline::coordinateAt(index)

    Gets the coordinate of the polyline at the given \a index.
    If the index is outside the path's bounds then an invalid
    coordinate is returned.

    \since QtLocation 5.6
*/
QGeoCoordinate QDeclarativePolylineMapItem::coordinateAt(int index) const
{
    if (index < 0 || index >= m_geopath.path().length())
        return QGeoCoordinate();

    return m_geopath.coordinateAt(index);
}

/*!
    \qmlmethod coordinate MapPolyline::containsCoordinate(coordinate)

    Returns true if the given \a coordinate is part of the path.

    \since QtLocation 5.6
*/
bool QDeclarativePolylineMapItem::containsCoordinate(const QGeoCoordinate &coordinate)
{
    return m_geopath.containsCoordinate(coordinate);
}

/*!
    \qmlmethod void MapPolyline::removeCoordinate(coordinate)

    Removes \a coordinate from the path. If there are multiple instances of the
    same coordinate, the one added last is removed.

    If \a coordinate is not in the path this method does nothing.

    \sa addCoordinate, insertCoordinate, path
*/
void QDeclarativePolylineMapItem::removeCoordinate(const QGeoCoordinate &coordinate)
{
    int length = m_geopath.path().length();
    m_geopath.removeCoordinate(coordinate);
    if (m_geopath.path().length() == length)
        return;

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlmethod void MapPolyline::removeCoordinate(index)

    Removes a coordinate from the path at the given \a index.

    If \a index is invalid then this method does nothing.

    \since QtLocation 5.6

    \sa addCoordinate, insertCoordinate, path
*/
void QDeclarativePolylineMapItem::removeCoordinate(int index)
{
    if (index < 0 || index >= m_geopath.path().length())
        return;

    m_geopath.removeCoordinate(index);

    m_d->onGeoGeometryChanged();
    emit pathChanged();
}

/*!
    \qmlpropertygroup Location::MapPolyline::line
    \qmlproperty int MapPolyline::line.width
    \qmlproperty color MapPolyline::line.color

    This property is part of the line property group. The line
    property group holds the width and color used to draw the line.

    The width is in pixels and is independent of the zoom level of the map.
    The default values correspond to a black border with a width of 1 pixel.

    For no line, use a width of 0 or a transparent color.
*/

QDeclarativeMapLineProperties *QDeclarativePolylineMapItem::line()
{
    return &m_line;
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.topLeft() == oldGeometry.topLeft() || !map() || !m_geopath.isValid() || m_updatingGeometry) {
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

    m_geopath.translate(offsetLati, offsetLongi);
    m_d->onGeoGeometryChanged();
    emit pathChanged();

    // Not calling QDeclarativeGeoMapItemBase::geometryChange() as it will be called from a nested
    // call to this function.
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.isEmpty())
        return;

    m_d->afterViewportChanged();
}

/*!
    \internal
*/
void QDeclarativePolylineMapItem::updatePolish()
{
    if (!map() || map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
        return;
    m_d->updatePolish();
}

/*!
    \internal
*/
QSGNode *QDeclarativePolylineMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    return m_d->updateMapItemPaintNode(oldNode, data);
}

bool QDeclarativePolylineMapItem::contains(const QPointF &point) const
{
    return m_d->contains(point);
}

const QGeoShape &QDeclarativePolylineMapItem::geoShape() const
{
    return m_geopath;
}

void QDeclarativePolylineMapItem::setGeoShape(const QGeoShape &shape)
{
    const QGeoPath geopath(shape); // if shape isn't a path, path will be created as a default-constructed path
    setPath(geopath);
}

//////////////////////////////////////////////////////////////////////

QT_END_NAMESPACE
