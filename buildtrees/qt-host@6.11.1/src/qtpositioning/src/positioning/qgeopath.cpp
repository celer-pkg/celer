// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopath.h"
#include "qgeopolygon.h"
#include "qgeopath_p.h"

#include "qgeocoordinate.h"
#include "qnumeric.h"
#include "qlocationutils_p.h"
#include "qwebmercator_p.h"

#include "qdoublevector2d_p.h"
#include "qdoublevector3d_p.h"
QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QGeoPath)

constexpr auto kWarningString = u"The path has more elements than fit into an int. "
                                 "This can cause errors while querying elements from QML";

/*!
    \class QGeoPath
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \since 5.9

    \brief The QGeoPath class defines a geographic path.

    The path is defined by an ordered list of \l QGeoCoordinate objects.

    Each two adjacent elements in the path are intended to be connected
    together by the shortest line segment of constant bearing passing
    through both elements.
    This type of connection can cross the dateline in the longitudinal direction,
    but never crosses the poles.

    This is relevant for the calculation of the bounding box returned by
    \l QGeoShape::boundingGeoRectangle() for this shape, which will have the latitude of
    the top left corner set to the maximum latitude in the path point set.
    Similarly, the latitude of the bottom right corner will be the minimum latitude
    in the path point set.

    This class is also accessible in QML as \l[QML]{geoPath}.

    A QGeoPath is both invalid and empty if it contains no coordinate.

    \note A default constructed QGeoPath is both invalid and empty as it does not contain any coordinates.
*/

/*!
    \property QGeoPath::path
    \brief This property holds the list of coordinates for the geo path.

    \note The coordinates cannot be processed in place. To change the value
    of this property, retrieve the complete list of coordinates, process them,
    and assign the new value to the property.
*/

inline QGeoPathPrivate *QGeoPath::d_func()
{
    return static_cast<QGeoPathPrivate *>(d_ptr.data());
}

inline const QGeoPathPrivate *QGeoPath::d_func() const
{
    return static_cast<const QGeoPathPrivate *>(d_ptr.constData());
}

/*!
    Constructs a new, empty geo path.
*/
QGeoPath::QGeoPath()
:   QGeoShape(new QGeoPathPrivate())
{
}

/*!
    Constructs a new geo path from a list of coordinates
    (\a path and \a width).
*/
QGeoPath::QGeoPath(const QList<QGeoCoordinate> &path, const qreal &width)
:   QGeoShape(new QGeoPathPrivate(path, width))
{
}

/*!
    Constructs a new geo path from the contents of \a other.
*/
QGeoPath::QGeoPath(const QGeoPath &other)
:   QGeoShape(other)
{
}

/*!
    Constructs a new geo path from the contents of \a other.
*/
QGeoPath::QGeoPath(const QGeoShape &other)
:   QGeoShape(other)
{
    if (type() != QGeoShape::PathType)
        d_ptr = new QGeoPathPrivate();
}

/*!
    Destroys this path.
*/
QGeoPath::~QGeoPath() {}

/*!
    Assigns \a other to this geo path and returns a reference to this geo path.
*/
QGeoPath &QGeoPath::operator=(const QGeoPath &other)
{
    QGeoShape::operator=(other);
    return *this;
}

/*!
    Sets all the elements of the \a path.
*/
void QGeoPath::setPath(const QList<QGeoCoordinate> &path)
{
    Q_D(QGeoPath);
    return d->setPath(path);
}

/*!
    Returns all the elements of the path.
*/
const QList<QGeoCoordinate> &QGeoPath::path() const
{
    Q_D(const QGeoPath);
    return d->path();
}

/*!
    Clears the path.

    \since 5.12
*/
void QGeoPath::clearPath()
{
    Q_D(QGeoPath);
    d->clearPath();
}

/*!
    Sets all the elements of the path.

    \internal
*/
void QGeoPath::setVariantPath(const QVariantList &path)
{
    Q_D(QGeoPath);
    QList<QGeoCoordinate> p;
    for (const auto &c: path) {
        if (c.canConvert<QGeoCoordinate>())
            p << c.value<QGeoCoordinate>();
    }
    d->setPath(p);
}
/*!
    Returns all the elements of the path.

    \internal
*/
QVariantList QGeoPath::variantPath() const
{
    Q_D(const QGeoPath);
    QVariantList p;
    for (const auto &c: d->path())
        p << QVariant::fromValue(c);
    return p;
}


/*!
    \property QGeoPath::width

    \brief the width of the path in meters.
*/
void QGeoPath::setWidth(const qreal &width)
{
    Q_D(QGeoPath);
    d->setWidth(width);
}

/*!
    Returns the width of the path, in meters. This information is used in the \l contains method.
    The default value is 0.
*/
qreal QGeoPath::width() const
{
    Q_D(const QGeoPath);
    return d->width();
}

/*!
    Translates this geo path by \a degreesLatitude northwards and \a degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.
*/
void QGeoPath::translate(double degreesLatitude, double degreesLongitude)
{
    Q_D(QGeoPath);
    d->translate(degreesLatitude, degreesLongitude);
}

/*!
    Returns a copy of this geo path translated by \a degreesLatitude northwards and
    \a degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.

    \sa translate()
*/
QGeoPath QGeoPath::translated(double degreesLatitude, double degreesLongitude) const
{
    QGeoPath result(*this);
    result.translate(degreesLatitude, degreesLongitude);
    return result;
}

/*!
    Returns the length of the path, in meters, from the element \a indexFrom to the element \a indexTo.
    The length is intended to be the sum of the shortest distances for each pair of adjacent points.

    If \a indexTo is -1 (the default value), the length will be including the distance between last coordinate
    and the first (closed loop).
    To retrieve the length for the path, use 0 for \a indexFrom and \l QGeoPath::size() - 1 for \a indexTo.
*/
double QGeoPath::length(qsizetype indexFrom, qsizetype indexTo) const
{
    Q_D(const QGeoPath);
    return d->length(indexFrom, indexTo);
}

/*!
    Returns the number of elements in the path.

    \since 5.10
*/
qsizetype QGeoPath::size() const
{
    Q_D(const QGeoPath);
    const qsizetype result = d->size();
    if (result > std::numeric_limits<int>::max())
        qWarning() << kWarningString;
    return result;
}

/*!
    Appends \a coordinate to the path.
*/
void QGeoPath::addCoordinate(const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPath);
    d->addCoordinate(coordinate);
    if (d->size() > std::numeric_limits<int>::max())
        qWarning() << kWarningString;
}

/*!
    Inserts \a coordinate at the specified \a index.
*/
void QGeoPath::insertCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPath);
    d->insertCoordinate(index, coordinate);
}

/*!
    Replaces the path element at the specified \a index with \a coordinate.
*/
void QGeoPath::replaceCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPath);
    d->replaceCoordinate(index, coordinate);
}

/*!
    Returns the coordinate at \a index .
*/
QGeoCoordinate QGeoPath::coordinateAt(qsizetype index) const
{
    Q_D(const QGeoPath);
    return d->coordinateAt(index);
}

/*!
    Returns true if the path contains \a coordinate as one of the elements.
*/
bool QGeoPath::containsCoordinate(const QGeoCoordinate &coordinate) const
{
    Q_D(const QGeoPath);
    return d->containsCoordinate(coordinate);
}

/*!
    Removes the last occurrence of \a coordinate from the path.
*/
void QGeoPath::removeCoordinate(const QGeoCoordinate &coordinate)
{
    Q_D(QGeoPath);
    d->removeCoordinate(coordinate);
}

/*!
    Removes element at position \a index from the path.
*/
void QGeoPath::removeCoordinate(qsizetype index)
{
    Q_D(QGeoPath);
    d->removeCoordinate(index);
}

/*!
    Returns the geo path properties as a string.
*/
QString QGeoPath::toString() const
{
    if (type() != QGeoShape::PathType) {
        qWarning("Not a path");
        return QStringLiteral("QGeoPath(not a path)");
    }

    QString pathString;
    for (const auto &p : path())
        pathString += p.toString() + QLatin1Char(',');

    return QStringLiteral("QGeoPath([ %1 ])").arg(pathString);
}

/*******************************************************************************
 *
 * QGeoPathPrivateBase & friends
 *
*******************************************************************************/

QGeoPathPrivateBase::QGeoPathPrivateBase()
    : QGeoShapePrivate(QGeoShape::PathType)
{
}

QGeoPathPrivateBase::QGeoPathPrivateBase(const QList<QGeoCoordinate> &path)
    : QGeoPathPrivateBase()
{
    setPath(path);
}

QGeoPathPrivateBase::~QGeoPathPrivateBase()
{
}

bool QGeoPathPrivateBase::isValid() const
{
    return !isEmpty();
}

bool QGeoPathPrivateBase::isEmpty() const
{
    return path().isEmpty(); // this should perhaps return geometric emptiness, less than 2 points for line, or empty polygon for polygons
}

QGeoCoordinate QGeoPathPrivateBase::center() const
{
    return boundingGeoRectangle().center();
}

bool QGeoPathPrivateBase::operator==(const QGeoShapePrivate &other) const
{
    if (!QGeoShapePrivate::operator==(other)) // checks type
        return false;

    const QGeoPathPrivateBase &otherBase = static_cast<const QGeoPathPrivateBase &>(other);
    return m_path == otherBase.m_path;
}

QGeoRectangle QGeoPathPrivateBase::boundingGeoRectangle() const
{
    if (m_bboxDirty)
        const_cast<QGeoPathPrivateBase *>(this)->computeBoundingBox();
    return m_bbox;
}

size_t QGeoPathPrivateBase::hash(size_t seed) const
{
    return qHashRange(m_path.cbegin(), m_path.cend(), seed);
}

const QList<QGeoCoordinate> &QGeoPathPrivateBase::path() const
{
    return m_path;
}

double QGeoPathPrivateBase::length(qsizetype indexFrom, qsizetype indexTo) const
{
    if (path().isEmpty())
        return 0.0;

    bool wrap = indexTo == -1;
    if (indexTo < 0 || indexTo >= path().size())
        indexTo = path().size() - 1;
    double len = 0.0;
    // TODO: consider calculating the length of the actual rhumb line segments
    // instead of the shortest path from A to B.
    for (qsizetype i = indexFrom; i < indexTo; i++)
        len += m_path[i].distanceTo(m_path[i + 1]);
    if (wrap)
        len += m_path.last().distanceTo(m_path.first());
    return len;
}

qsizetype QGeoPathPrivateBase::size() const
{
    return m_path.size();
}

QGeoCoordinate QGeoPathPrivateBase::coordinateAt(qsizetype index) const
{
    if (index < 0 || index >= m_path.size())
        return QGeoCoordinate();

    return m_path.at(index);
}

bool QGeoPathPrivateBase::containsCoordinate(const QGeoCoordinate &coordinate) const
{
    return m_path.indexOf(coordinate) > -1;
}

void QGeoPathPrivateBase::translate(double degreesLatitude, double degreesLongitude)
{
    // Need min/maxLati, so update bbox
    QList<double> deltaXs;
    double minX, maxX, minLati, maxLati;
    m_bboxDirty = false;
    computeBBox(m_path, deltaXs, minX, maxX, minLati, maxLati, m_bbox);

    if (degreesLatitude > 0.0)
        degreesLatitude = qMin(degreesLatitude, 90.0 - maxLati);
    else
        degreesLatitude = qMax(degreesLatitude, -90.0 - minLati);
    for (QGeoCoordinate &p: m_path) {
        p.setLatitude(p.latitude() + degreesLatitude);
        p.setLongitude(QLocationUtils::wrapLong(p.longitude() + degreesLongitude));
    }
    m_bbox.translate(degreesLatitude, degreesLongitude);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

void QGeoPathPrivateBase::setPath(const QList<QGeoCoordinate> &path)
{
    for (const QGeoCoordinate &c: path) {
        if (!c.isValid())
            return;
    }
    m_path = path;
    markDirty();
}

void QGeoPathPrivateBase::clearPath()
{
    m_path.clear();
    markDirty();
}

void QGeoPathPrivateBase::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;
    m_path.append(coordinate);
    markDirty();
}

void QGeoPathPrivateBase::insertCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index > m_path.size() || !coordinate.isValid())
        return;
    m_path.insert(index, coordinate);
    markDirty();
}

void QGeoPathPrivateBase::replaceCoordinate(qsizetype index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index >= m_path.size() || !coordinate.isValid())
        return;
    m_path[index] = coordinate;
    markDirty();
}

void QGeoPathPrivateBase::removeCoordinate(const QGeoCoordinate &coordinate)
{
    qsizetype index = m_path.lastIndexOf(coordinate);
    removeCoordinate(index);
}

void QGeoPathPrivateBase::removeCoordinate(qsizetype index)
{
    if (index < 0 || index >= m_path.size())
        return;
    m_path.removeAt(index);
    markDirty();
}

void QGeoPathPrivateBase::computeBoundingBox()
{
    QList<double> deltaXs;
    double minX, maxX, minLati, maxLati;
    m_bboxDirty = false;
    computeBBox(m_path, deltaXs, minX, maxX, minLati, maxLati, m_bbox);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

void QGeoPathPrivateBase::markDirty()
{
    m_bboxDirty = true;
}



QGeoPathPrivate::QGeoPathPrivate()
    : QGeoPathPrivateBase()
{
}

QGeoPathPrivate::QGeoPathPrivate(const QList<QGeoCoordinate> &path, const qreal width)
    : QGeoPathPrivateBase(path)
{
    setWidth(width);
}

QGeoPathPrivate::~QGeoPathPrivate()
{
}

QGeoShapePrivate *QGeoPathPrivate::clone() const
{
    return new QGeoPathPrivate(*this);
}

bool QGeoPathPrivate::operator==(const QGeoShapePrivate &other) const
{
    if (!QGeoPathPrivateBase::operator==(other)) // checks type
        return false;

    const QGeoPathPrivate &otherPath = static_cast<const QGeoPathPrivate &>(other);
    return m_width == otherPath.m_width;
}

bool QGeoPathPrivate::lineContains(const QGeoCoordinate &coordinate) const
{
    // Unoptimized approach:
    // - consider each segment of the path (a rhumb line)
    // - project it into Mercator space (in which it is straight)
    // - find Mercator-closest point to coordinate
    // - unproject the closest point
    // - calculate coordinate to closest point distance with distanceTo()
    // - if not within lineRadius, advance to next segment.
    //
    // When considering each segment, wrap its start-point to x-within 0.5 of
    // its end-point and (initially) the target point to x-within the mid-point
    // of the thus-narrowed segment. That position will usually find the actual
    // Mercator-closest point and makes it easy to test for the obscure case
    // where a different wrap-position for it might work better; when that is
    // possible, try that other wrap-position for it in a second pass.

    // Should be redundant now, unless other functions rely on this one's side-effect:
    if (m_bboxDirty)
        const_cast<QGeoPathPrivate &>(*this).computeBoundingBox();

    double lineRadius = qMax(width() * 0.5, 0.2); // minimum radius: 20cm

    if (m_path.isEmpty())
        return false;
    else if (m_path.size() == 1)
        return (m_path[0].distanceTo(coordinate) <= lineRadius);

    Q_ASSERT(m_path.size() > 1);
    const QDoubleVector2D pt = QWebMercator::coordToMercator(coordinate);
    QDoubleVector2D last = QWebMercator::coordToMercator(m_path[0]);
    for (qsizetype i = 1; i < m_path.size(); i++) {
        const QDoubleVector2D here = QWebMercator::coordToMercator(m_path[i]);
        // Wrap last to gets its x() within ±0.5 of that of here:
        if (here.x() > last.x() + 0.5)
            last.setX(last.x() + 1.0);
        else if (here.x() < last.x() - 0.5)
            last.setX(last.x() - 1.0);

        if (here == last) {
            // The whole line segment is one point, so easy to test.
            if (m_path[i].distanceTo(coordinate) <= lineRadius)
                return true;
            continue;
        }

        QDoubleVector2D p = pt;
        {
            QDoubleVector2D mid = (last + here) / 2.0;
            // Wrap p to gets its x() within ±0.5 of that of mid:
            if (p.x() > mid.x() + 0.5)
                p.setX(p.x() - 1.0);
            else if (p.x() < mid.x() - 0.5)
                p.setX(p.x() + 1.0);
        }
        // See comment on updating p after j == 0; loop to catch the corner case.
        for (int j = 0; j < 2; ++j) {
            const QDoubleVector2D pml = p - last, hml = here - last;
            const double u = (pml.x() * hml.x() + pml.y() * hml.y()) / hml.lengthSquared();
            // Interpolate between 0 < u < 1, use nearer end otherwise:
            const QDoubleVector2D candidate = u > 0 ? u < 1 ? last + u * hml : here : last;
            const double distance = coordinate.distanceTo(QWebMercator::mercatorToCoord(candidate));
            if (distance <= lineRadius)
                return true;

            if (j == 0) {
                /* Our initial p's .x() is within 0.5 of mid.x(), hence of at
                   least one of last.x() and here.x(). If it's within 0.5 of
                   both of these, the candidate we just tried is definitely our
                   best bet. Otherwise - though it's very obscure and can't
                   matter unless lineRadius is comparable with the rhumb line's
                   distance from the planet's spin axis - there are technically
                   two parabolic segments p could be in (y-closer to one end but
                   x-closer to the other), that would put p further from
                   last:here than from its more x()-distant end's wrapped
                   version the other side of p. So wrap p to its version that
                   would be closer to that distant end, were this to arise.
                */
                if (p.x() > qMin(last.x(), here.x()) + 0.5)
                    p.setX(p.x() - 1.0);
                else if (p.x() < qMax(last.x(), here.x()) - 0.5)
                    p.setX(p.x() + 1.0);
                else
                    break;
            }
        }
        // swap
        last = here;
    }

    return false;
}

bool QGeoPathPrivate::contains(const QGeoCoordinate &coordinate) const
{
    return lineContains(coordinate);
}

qreal QGeoPathPrivate::width() const
{
    return m_width;
}

void QGeoPathPrivate::setWidth(const qreal &width)
{
    if (qIsNaN(width) || width < 0.0)
        return;
    m_width = width;
}

size_t QGeoPathPrivate::hash(size_t seed) const
{
    const size_t res = QGeoPathPrivateBase::hash(seed);
    return qHashMulti(seed, res, m_width);
}

QGeoPathPrivateEager::QGeoPathPrivateEager()
:   QGeoPathPrivate()
{
    m_bboxDirty = false; // never dirty on the eager version
}

QGeoPathPrivateEager::QGeoPathPrivateEager(const QList<QGeoCoordinate> &path, const qreal width)
:   QGeoPathPrivate(path, width)
{
    m_bboxDirty = false; // never dirty on the eager version
    markDirty(); // calculate the cached values
}

QGeoPathPrivateEager::~QGeoPathPrivateEager()
{

}

QGeoShapePrivate *QGeoPathPrivateEager::clone() const
{
    return new QGeoPathPrivateEager(*this);
}

void QGeoPathPrivateEager::markDirty()
{
    // do the calculations directly
    computeBBox(m_path, m_deltaXs, m_minX, m_maxX, m_minLati, m_maxLati, m_bbox);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

void QGeoPathPrivateEager::translate(double degreesLatitude, double degreesLongitude)
{
    if (degreesLatitude > 0.0)
        degreesLatitude = qMin(degreesLatitude, 90.0 - m_maxLati);
    else
        degreesLatitude = qMax(degreesLatitude, -90.0 - m_minLati);
    for (QGeoCoordinate &p: m_path) {
        p.setLatitude(p.latitude() + degreesLatitude);
        p.setLongitude(QLocationUtils::wrapLong(p.longitude() + degreesLongitude));
    }
    m_bbox.translate(degreesLatitude, degreesLongitude);
    m_minLati += degreesLatitude;
    m_maxLati += degreesLatitude;
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

void QGeoPathPrivateEager::addCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid())
        return;
    m_path.append(coordinate);
    //m_clipperDirty = true; // clipper not used in polylines
    updateBoundingBox();
}

void QGeoPathPrivateEager::QGeoPathPrivateEager::computeBoundingBox()
{
    Q_UNREACHABLE();
}

void QGeoPathPrivateEager::QGeoPathPrivateEager::updateBoundingBox()
{
    updateBBox(m_path, m_deltaXs, m_minX, m_maxX, m_minLati, m_maxLati, m_bbox);
    m_leftBoundWrapped = QWebMercator::coordToMercator(m_bbox.topLeft()).x();
}

QGeoPathEager::QGeoPathEager() : QGeoPath()
{
    d_ptr = new QGeoPathPrivateEager;
}

QGeoPathEager::QGeoPathEager(const QList<QGeoCoordinate> &path, const qreal &width) : QGeoPath()
{
    d_ptr = new QGeoPathPrivateEager(path, width);
}

QGeoPathEager::QGeoPathEager(const QGeoPath &other) : QGeoPath()
{
    d_ptr = new QGeoPathPrivateEager;
    setPath(other.path());
    setWidth(other.width());
}

QGeoPathEager::QGeoPathEager(const QGeoShape &other) : QGeoPath()
{
    if (other.type() == QGeoShape::PathType)
        *this = QGeoPathEager(QGeoPath(other));
    else
        d_ptr = new QGeoPathPrivateEager;
}

QGeoPathEager::~QGeoPathEager() {}

QT_END_NAMESPACE

#include "moc_qgeopath_p.cpp"
#include "moc_qgeopath.cpp"







