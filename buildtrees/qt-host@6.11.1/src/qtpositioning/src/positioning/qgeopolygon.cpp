// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopolygon.h"
#include "qgeopolygon_p.h"
#include "qgeopath_p.h"
#include "qgeocircle.h"

#include "qgeocoordinate.h"
#include "qnumeric.h"
#include "qlocationutils_p.h"
#include "qwebmercator_p.h"

#include "qdoublevector2d_p.h"
#include "qdoublevector3d_p.h"
#include "qwebmercator_p.h"

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QGeoPolygon)

constexpr int kMaxInt = std::numeric_limits<int>::max();
constexpr auto kTooManyHoles = u"The polygon has more holes than fit into an int. "
                                "This can cause errors while querying holes from QML";
constexpr auto kTooManyElements = u"The polygon has more elements than fit into an int. "
                                   "This can cause errors while querying elements from QML";

/*!
    \class QGeoPolygon
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \since 5.10

    \brief The QGeoPolygon class defines a geographic polygon.

    The polygon is defined by an ordered list of \l QGeoCoordinate objects
    representing its perimeter.

    Each two adjacent elements in this list are intended to be connected
    together by the shortest line segment of constant bearing passing
    through both elements.
    This type of connection can cross the date line in the longitudinal direction,
    but never crosses the poles.

    This is relevant for the calculation of the bounding box returned by
    \l QGeoShape::boundingGeoRectangle() for this shape, which will have the latitude of
    the top left corner set to the maximum latitude in the path point set.
    Similarly, the latitude of the bottom right corner will be the minimum latitude
    in the path point set.

    This class is also accessible in QML as \l[QML]{geoPolygon}.
*/

/*
    \property QGeoPolygon::path
    \brief This property holds the list of coordinates for the geo polygon.

    The polygon is both invalid and empty if it contains no coordinate.

    A default constructed QGeoPolygon is therefore invalid.
*/

inline QGeoPolygonPrivate *QGeoPolygon::d_func()
{
    return static_cast<QGeoPolygonPrivate *>(d_ptr.data());
}

inline const QGeoPolygonPrivate *QGeoPolygon::d_func() const
{
    return static_cast<const QGeoPolygonPrivate *>(d_ptr.constData());
}

/*!
    Constructs a new, empty geo polygon.
*/
QGeoPolygon::QGeoPolygon()
:   QGeoShape(new QGeoPolygonPrivate())
{
}

/*!
    Constructs a new geo polygon from the coordinates specified
    in \a path.
*/
QGeoPolygon::QGeoPolygon(const QList<QGeoCoordinate> &path)
:   QGeoShape(new QGeoPolygonPrivate(path))
{
}

/*!
    Constructs a new geo polygon from the contents of \a other.
*/
QGeoPolygon::QGeoPolygon(const QGeoPolygon &other)
:   QGeoShape(other)
{
}

static void calculatePeripheralPoints(QList<QGeoCoordinate> &path,
                                      const QGeoCircle &circle,
                                      int steps)
{
    const QGeoCoordinate &center = circle.center();
    const qreal distance = circle.radius();
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
        qreal azimuthRad = 2 * M_PI * i / steps;
        qreal resultLatRad = std::asin(sinLatRad_x_cosRatio
                                   + cosLatRad_x_sinRatio * std::cos(azimuthRad));
        qreal resultLonRad = lonRad + std::atan2(std::sin(azimuthRad) * cosLatRad_x_sinRatio,
                                       cosRatio - sinLatRad * std::sin(resultLatRad));
        qreal lat2 = QLocationUtils::degrees(resultLatRad);
        qreal lon2 = QLocationUtils::wrapLong(QLocationUtils::degrees(resultLonRad));

        path << QGeoCoordinate(lat2, lon2, center.altitude());
    }
}

/*!
    Constructs a new geo polygon from the contents of \a other.
*/
QGeoPolygon::QGeoPolygon(const QGeoShape &other)
:   QGeoShape(other)
{
    if (type() != QGeoShape::PolygonType) {
        QGeoPolygonPrivate *poly = new QGeoPolygonPrivate();
        if (type() == QGeoShape::CircleType) {
            const QGeoCircle &circle = static_cast<const QGeoCircle &>(other);
            QList<QGeoCoordinate> perimeter;
            calculatePeripheralPoints(perimeter, circle, 128);
            poly->setPath(perimeter);
        } else if (type() == QGeoShape::RectangleType) {
            const QGeoRectangle &rect = static_cast<const QGeoRectangle &>(other);
            QList<QGeoCoordinate> perimeter;
            perimeter << rect.topLeft() << rect.topRight()
                      << rect.bottomRight() << rect.bottomLeft();
            poly->setPath(perimeter);
        }
        d_ptr = poly;
    }
}

/*!
    Destroys this polygon.
*/
QGeoPolygon::~QGeoPolygon() {}

/*!
    Assigns \a other to this geo polygon and returns a reference to this geo polygon.
*/
QGeoPolygon &QGeoPolygon::operator=(const QGeoPolygon &other)
{
    QGeoShape::operator=(other);
    return *this;
}

/*!
    Sets the perimeter of the polygon based on a list of coordinates \a path.

    \since QtPositioning 5.12
*/
void QGeoPolygon::setPerimeter(const QList<QGeoCoordinate> &path)
{
    Q_D(QGeoPolygon);
    return d->setPath(path);
}

/*!
    \property QGeoPolygon::perimeter
    \brief the coordinates of the polygon's perimeter.
*/

/*!
    Returns all the elements of the polygon's perimeter.

    \since QtPositioning 5.12
*/
const QList<QGeoCoordinate> &QGeoPolygon::perimeter() const
{
    Q_D(const QGeoPolygon);
    return d->path();
}

/*!
    Translates this geo polygon by \a degreesLatitude northwards and \a degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.
*/
void QGeoPolygon::translate(double degreesLatitude, double degreesLongitude)
{
    Q_D(QGeoPolygon);
    d->translate(degreesLatitude, degreesLongitude);
}

/*!
    Returns a copy of this geo polygon translated by \a degreesLatitude northwards and
    \a degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.

    \sa translate()
*/
QGeoPolygon QGeoPolygon::translated(double degreesLatitude, double degreesLongitude) const
{
    QGeoPolygon result(*this);
    result.translate(degreesLatitude, degreesLongitude);
    return result;
}

/*!
    Returns the length of the polygon's perimeter, in meters, from the element \a indexFrom to the element \a indexTo.
    The length is intended to be the sum of the shortest distances for each pair of adjacent points.
*/
double QGeoPolygon::length(qsizetype indexFrom, qsizetype indexTo) const
{
    Q_D(const QGeoPolygon);
    return d->length(indexFrom, indexTo);
}

/*!
    Returns the number of elements in the polygon.

    \since 5.10
*/
qsizetype QGeoPolygon::size() const
{
    Q_D(const QGeoPolygon);
    const qsizetype result = d->size();
    if (result > kMaxInt)
        qWarning() << kTooManyElements;
    return result;
}

/*!
    Appends \a coordinate to the polygon.
*/
void QGeoPolygon::addCoordinate(const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPolygon);
    d->addCoordinate(coordinate);
    if (d->size() > kMaxInt)
        qWarning() << kTooManyElements;
}

/*!
    Inserts \a coordinate at the specified \a index.
*/
void QGeoPolygon::insertCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPolygon);
    d->insertCoordinate(index, coordinate);
}

/*!
    Replaces the path element at the specified \a index with \a coordinate.
*/
void QGeoPolygon::replaceCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPolygon);
    d->replaceCoordinate(index, coordinate);
}

/*!
    Returns the coordinate at \a index .
*/
QGeoCoordinate QGeoPolygon::coordinateAt(qsizetype index) const
{
    Q_D(const QGeoPolygon);
    return d->coordinateAt(index);
}

/*!
    Returns true if the polygon's perimeter contains \a coordinate as one of the elements.
*/
bool QGeoPolygon::containsCoordinate(const QGeoCoordinate &coordinate) const
{
    Q_D(const QGeoPolygon);
    return d->containsCoordinate(coordinate);
}

/*!
    Removes the last occurrence of \a coordinate from the polygon.
*/
void QGeoPolygon::removeCoordinate(const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPolygon);
    d->removeCoordinate(coordinate);
}

/*!
    Removes element at position \a index from the polygon.
*/
void QGeoPolygon::removeCoordinate(qsizetype index)
{
    Q_D(QGeoPolygon);
    d->removeCoordinate(index);
}

/*!
    Returns the geo polygon properties as a string.
*/
QString QGeoPolygon::toString() const
{
    if (type() != QGeoShape::PolygonType) {
        qWarning("Not a polygon");
        return QStringLiteral("QGeoPolygon(not a polygon)");
    }

    QString pathString;
    for (const auto &p : perimeter())
        pathString += p.toString() + QLatin1StringView("; ");
    pathString.chop(2);

    return QStringLiteral("QGeoPolygon([ %1 ])").arg(pathString);
}

/*!
   Sets the \a holePath for a hole inside the polygon. The hole is a
   QVariant containing a QList<QGeoCoordinate>.

   \since 5.12
*/
void QGeoPolygon::addHole(const QVariant &holePath)
{
    QList<QGeoCoordinate> qgcHolePath;
    if (holePath.canConvert<QVariantList>()) {
        const QVariantList qvlHolePath = holePath.toList();
        for (const QVariant &vertex : qvlHolePath) {
            if (vertex.canConvert<QGeoCoordinate>())
                qgcHolePath << vertex.value<QGeoCoordinate>();
        }
    }
    //ToDo: add QGeoShape support
    addHole(qgcHolePath);
}

/*!
   Overloaded method. Sets the \a holePath for a hole inside the polygon. The hole is a QList<QGeoCoordinate>.

   \since 5.12
*/
void QGeoPolygon::addHole(const QList<QGeoCoordinate> &holePath)
{
    Q_D(QGeoPolygon);
    d->addHole(holePath);
    if (d->holesCount() > kMaxInt)
        qDebug() << kTooManyHoles;
}

/*!
    Returns a QVariant containing a QList<QGeoCoordinate>
    which represents the hole at \a index.

    \since 5.12
*/
const QVariantList QGeoPolygon::hole(qsizetype index) const
{
    Q_D(const QGeoPolygon);
    QVariantList holeCoordinates;
    for (const QGeoCoordinate &coords: d->holePath(index))
        holeCoordinates << QVariant::fromValue(coords);
    return holeCoordinates;
}

/*!
    Returns a QList<QGeoCoordinate> which represents the hole at \a index.

    \since 5.12
*/
const QList<QGeoCoordinate> QGeoPolygon::holePath(qsizetype index) const
{
    Q_D(const QGeoPolygon);
    return d->holePath(index);
}

/*!
    Removes element at position \a index from the list of holes.

    \since 5.12
*/
void QGeoPolygon::removeHole(qsizetype index)
{
    Q_D(QGeoPolygon);
    return d->removeHole(index);
}

/*!
    Returns the number of holes.

    \since 5.12
*/
qsizetype QGeoPolygon::holesCount() const
{
    Q_D(const QGeoPolygon);
    const qsizetype result = d->holesCount();
    if (result > kMaxInt)
        qWarning() << kTooManyHoles;
    return result;
}

/*******************************************************************************
 *
 * QGeoPathPrivate & friends
 *
*******************************************************************************/

QGeoPolygonPrivate::QGeoPolygonPrivate()
:   QGeoPathPrivateBase()
{
    type = QGeoShape::PolygonType;
}

QGeoPolygonPrivate::QGeoPolygonPrivate(const QList<QGeoCoordinate> &path)
:   QGeoPathPrivateBase(path)
{
    type = QGeoShape::PolygonType;
}

QGeoPolygonPrivate::~QGeoPolygonPrivate() {}

QGeoShapePrivate *QGeoPolygonPrivate::clone() const
{
    return new QGeoPolygonPrivate(*this);
}

bool QGeoPolygonPrivate::isValid() const
{
    return path().size() > 2;
}

bool QGeoPolygonPrivate::contains(const QGeoCoordinate &coordinate) const
{
    return polygonContains(coordinate);
}

inline static void translatePoly(   QList<QGeoCoordinate> &m_path,
                                    QList<QList<QGeoCoordinate>> &m_holesList,
                                    QGeoRectangle &m_bbox,
                                    double degreesLatitude,
                                    double degreesLongitude,
                                    double m_maxLati,
                                    double m_minLati)
{
    if (degreesLatitude > 0.0)
        degreesLatitude = qMin(degreesLatitude, 90.0 - m_maxLati);
    else
        degreesLatitude = qMax(degreesLatitude, -90.0 - m_minLati);
    for (QGeoCoordinate &p: m_path) {
        p.setLatitude(p.latitude() + degreesLatitude);
        p.setLongitude(QLocationUtils::wrapLong(p.longitude() + degreesLongitude));
    }
    if (!m_holesList.isEmpty()){
        for (QList<QGeoCoordinate> &hole: m_holesList){
            for (QGeoCoordinate &holeVertex: hole){
                holeVertex.setLatitude(holeVertex.latitude() + degreesLatitude);
                holeVertex.setLongitude(QLocationUtils::wrapLong(holeVertex.longitude() + degreesLongitude));
            }
        }
    }
    m_bbox.translate(degreesLatitude, degreesLongitude);
}

void QGeoPolygonPrivate::translate(double degreesLatitude, double degreesLongitude)
{
    // Need min/maxLati, so update bbox
    QList<double> m_deltaXs;
    double m_minX, m_maxX, m_minLati, m_maxLati;
    m_bboxDirty = false; // Updated in translatePoly
    computeBBox(m_path, m_deltaXs, m_minX, m_maxX, m_minLati, m_maxLati, m_bbox);
    translatePoly(m_path, m_holesList, m_bbox, degreesLatitude, degreesLongitude, m_maxLati, m_minLati);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
    m_clipperDirty = true;
}

bool QGeoPolygonPrivate::operator==(const QGeoShapePrivate &other) const
{
    if (!QGeoPathPrivateBase::operator==(other)) // checks type
        return false;

    const QGeoPolygonPrivate &otherPolygon = static_cast<const QGeoPolygonPrivate &>(other);
    return  m_holesList == otherPolygon.m_holesList;
}

size_t QGeoPolygonPrivate::hash(size_t seed) const
{
    const size_t pointsHash = qHashRange(m_path.cbegin(), m_path.cend(), seed);
    const size_t holesHash = qHashRange(m_holesList.cbegin(), m_holesList.cend(), seed);
    return qHashMulti(seed, pointsHash, holesHash);
}

void QGeoPolygonPrivate::addHole(const QList<QGeoCoordinate> &holePath)
{
    for (const QGeoCoordinate &holeVertex: holePath)
        if (!holeVertex.isValid())
            return;

    m_holesList << holePath;
    // ToDo: mark clipper dirty when hole caching gets added
}

const QList<QGeoCoordinate> QGeoPolygonPrivate::holePath(qsizetype index) const
{
    return m_holesList.at(index);
}

void QGeoPolygonPrivate::removeHole(qsizetype index)
{
    if (index < 0 || index >= m_holesList.size())
        return;

    m_holesList.removeAt(index);
    // ToDo: mark clipper dirty when hole caching gets added
}

qsizetype QGeoPolygonPrivate::holesCount() const
{
    return m_holesList.size();
}

bool QGeoPolygonPrivate::polygonContains(const QGeoCoordinate &coordinate) const
{
    if (m_clipperDirty)
        const_cast<QGeoPolygonPrivate *>(this)->updateClipperPath(); // this one updates bbox too if needed

    QDoubleVector2D coord = QWebMercator::coordToMercator(coordinate);

    if (coord.x() < m_leftBoundWrapped)
        coord.setX(coord.x() + 1.0);

    if (!m_clipperWrapper.pointInPolygon(coord))
        return false;

    // else iterates the holes List checking whether the point is contained inside the holes
    for (const QList<QGeoCoordinate> &holePath : std::as_const(m_holesList)) {
        // ToDo: cache these
        QGeoPolygon holePolygon;
        holePolygon.setPerimeter(holePath);
        if (holePolygon.contains(coordinate))
            return false;
    }
    return true;
}

void QGeoPolygonPrivate::markDirty()
{
    m_bboxDirty = m_clipperDirty = true;
}

void QGeoPolygonPrivate::updateClipperPath()
{
    if (m_bboxDirty)
        computeBoundingBox();
    m_clipperDirty = false;

    QList<QDoubleVector2D> preservedPath;
    preservedPath.reserve(m_path.size());
    for (const QGeoCoordinate &c : std::as_const(m_path)) {
        QDoubleVector2D crd = QWebMercator::coordToMercator(c);
        if (crd.x() < m_leftBoundWrapped)
            crd.setX(crd.x() + 1.0);
        preservedPath << crd;
    }
    m_clipperWrapper.setPolygon(preservedPath);
}

QGeoPolygonPrivateEager::QGeoPolygonPrivateEager() : QGeoPolygonPrivate()
{
    m_bboxDirty = false; // never dirty on the eager version
}

QGeoPolygonPrivateEager::QGeoPolygonPrivateEager(const QList<QGeoCoordinate> &path) : QGeoPolygonPrivate(path)
{
    m_bboxDirty = false; // never dirty on the eager version
    markDirty(); // calculate the cached values
}

QGeoPolygonPrivateEager::~QGeoPolygonPrivateEager()
{

}

QGeoShapePrivate *QGeoPolygonPrivateEager::clone() const
{
    return new QGeoPolygonPrivateEager(*this);
}

void QGeoPolygonPrivateEager::translate(double degreesLatitude, double degreesLongitude)
{
    translatePoly(m_path, m_holesList, m_bbox, degreesLatitude, degreesLongitude, m_maxLati, m_minLati);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
    m_clipperDirty = true;
}

void QGeoPolygonPrivateEager::markDirty()
{
    m_clipperDirty = true;
    // do the calculations directly
    computeBBox(m_path, m_deltaXs, m_minX, m_maxX, m_minLati, m_maxLati, m_bbox);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

void QGeoPolygonPrivateEager::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;
    m_path.append(coordinate);
    m_clipperDirty = true;
    updateBoundingBox(); // do not markDirty as it uses computeBoundingBox instead
}

void QGeoPolygonPrivateEager::computeBoundingBox()
{
    Q_UNREACHABLE();
}

void QGeoPolygonPrivateEager::updateBoundingBox()
{
    updateBBox(m_path, m_deltaXs, m_minX, m_maxX, m_minLati, m_maxLati, m_bbox);
}

QGeoPolygonEager::QGeoPolygonEager() : QGeoPolygon()
{
    d_ptr = new QGeoPolygonPrivateEager;
}

QGeoPolygonEager::QGeoPolygonEager(const QList<QGeoCoordinate> &path) : QGeoPolygon()
{
    d_ptr = new QGeoPolygonPrivateEager(path);
}

QGeoPolygonEager::QGeoPolygonEager(const QGeoPolygon &other) : QGeoPolygon()
{
    // without being able to dynamic_cast the d_ptr, only way to be sure is to reconstruct a new QGeoPolygonPrivateEager
    d_ptr = new QGeoPolygonPrivateEager;
    setPerimeter(other.perimeter());
    for (qsizetype i = 0; i < other.holesCount(); i++)
        addHole(other.holePath(i));
}

QGeoPolygonEager::QGeoPolygonEager(const QGeoShape &other) : QGeoPolygon()
{
    if (other.type() == QGeoShape::PolygonType)
        *this = QGeoPolygonEager(QGeoPolygon(other));
    else
        d_ptr = new QGeoPolygonPrivateEager;
}

QGeoPolygonEager::~QGeoPolygonEager()
{

}

QT_END_NAMESPACE

#include "moc_qgeopolygon_p.cpp"
#include "moc_qgeopolygon.cpp"
