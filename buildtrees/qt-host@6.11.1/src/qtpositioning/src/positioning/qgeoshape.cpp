// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoshape.h"
#include "qgeoshape_p.h"
#include "qgeorectangle.h"
#include "qgeocircle.h"
#include "qgeopath.h"
#include "qgeopolygon.h"


#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/QDebug>
#endif

#ifndef QT_NO_DATASTREAM
#include <QtCore/QDataStream>
#endif

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QGeoShape)

QGeoShapePrivate::QGeoShapePrivate(QGeoShape::ShapeType type)
:   type(type)
{
}

QGeoShapePrivate::~QGeoShapePrivate()
{
}

bool QGeoShapePrivate::operator==(const QGeoShapePrivate &other) const
{
    return type == other.type;
}

/*!
    \class QGeoShape
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \since 5.2

    \brief The QGeoShape class defines a geographic area.

    This class is the base class for classes which specify a geographic
    area.

    For the sake of consistency, subclasses should describe the specific
    details of the associated areas in terms of QGeoCoordinate instances
    and distances in meters.

    This class is also accessible in QML as \l[QML]{geoShape}.
*/

/*!
    \enum QGeoShape::ShapeType

    Describes the type of the shape.

    \value UnknownType      A shape of unknown type
    \value RectangleType    A rectangular shape
    \value CircleType       A circular shape
    \value PathType         A path type
    \value PolygonType      A polygon type
*/

/*!
    \property QGeoShape::type
    \brief This property holds the type of this geo shape.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    \property QGeoShape::isValid
    \brief This property holds the validity of the geo shape.

    A geo shape is considered to be invalid if some of the data that is required to
    unambiguously describe the geo shape has not been set or has been set to an
    unsuitable value depending on the subclass of this object. The default constructed
    objects of this type are invalid.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    \property QGeoShape::isEmpty
    \brief This property defines whether this geo shape is empty.

    An empty geo shape is a region which has a geometrical area of 0.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/
inline QGeoShapePrivate *QGeoShape::d_func()
{
    return static_cast<QGeoShapePrivate *>(d_ptr.data());
}

inline const QGeoShapePrivate *QGeoShape::d_func() const
{
    return static_cast<const QGeoShapePrivate *>(d_ptr.constData());
}

/*!
    Constructs a new invalid geo shape of \l UnknownType.
*/
QGeoShape::QGeoShape()
{
}

/*!
    Constructs a new geo shape which is a copy of \a other.
*/
QGeoShape::QGeoShape(const QGeoShape &other)
:   d_ptr(other.d_ptr)
{
}

/*!
    \internal
*/
QGeoShape::QGeoShape(QGeoShapePrivate *d)
:   d_ptr(d)
{
}

bool QGeoShape::equals(const QGeoShape &lhs, const QGeoShape &rhs)
{
    if (lhs.d_func() == rhs.d_func())
        return true;

    if (!lhs.d_func() || !rhs.d_func())
        return false;

    return *lhs.d_func() == *rhs.d_func();
}

/*!
    Destroys this geo shape.
*/
QGeoShape::~QGeoShape()
{
}

/*!
    Returns the type of this geo shape.
*/
QGeoShape::ShapeType QGeoShape::type() const
{
    Q_D(const QGeoShape);

    if (d)
        return d->type;
    else
        return UnknownType;
}

/*!
    Returns whether this geo shape is valid.

*/
bool QGeoShape::isValid() const
{
    Q_D(const QGeoShape);

    if (d)
        return d->isValid();
    else
        return false;
}

/*!
    Returns whether this geo shape is empty.

    An empty geo shape is a region which has a geometrical area of 0.
*/
bool QGeoShape::isEmpty() const
{
    Q_D(const QGeoShape);

    if (d)
        return d->isEmpty();
    else
        return true;
}

/*!
    Returns whether the coordinate \a coordinate is contained within this geo shape.
*/
bool QGeoShape::contains(const QGeoCoordinate &coordinate) const
{
    Q_D(const QGeoShape);

    if (d)
        return d->contains(coordinate);
    else
        return false;
}

/*!
    Returns a QGeoRectangle representing the geographical bounding rectangle of the
    geo shape, that defines the latitudinal/longitudinal bounds of the geo shape.

    \since 5.9
*/
QGeoRectangle QGeoShape::boundingGeoRectangle() const
{
    Q_D(const QGeoShape);

    if (d)
        return d->boundingGeoRectangle();
    else
        return QGeoRectangle();
}

/*!
    \property QGeoShape::center
    \brief the coordinate at the geometric center of the shape.
*/

/*!
    Returns the coordinate located at the geometric center of the geo shape.

    \since 5.5
*/
QGeoCoordinate QGeoShape::center() const
{
    Q_D(const QGeoShape);

    if (d)
        return d->center();
    else
        return QGeoCoordinate();
}

/*!
    \fn bool QGeoShape::operator==(const QGeoShape &lhs, const QGeoShape &rhs)

    Returns \c true if the \a lhs geo shape is equivalent to the \a rhs geo
    shape, otherwise returns \c false.
*/

/*!
    \fn bool QGeoShape::operator!=(const QGeoShape &lhs, const QGeoShape &rhs)

    Returns \c true if the \a lhs geo shape is not equivalent to the \a rhs geo
    shape, otherwise returns \c false.
*/

/*!
    Assigns \a other to this geo shape and returns a reference to this geo shape.
*/
QGeoShape &QGeoShape::operator=(const QGeoShape &other)
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns a string representation of this geo shape.

    \since 5.5
*/
QString QGeoShape::toString() const
{
    return QStringLiteral("QGeoShape(%1)").arg(type());
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QGeoShape::debugStreaming(QDebug dbg, const QGeoShape &shape)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QGeoShape(";
    switch (shape.type()) {
    case QGeoShape::UnknownType:
        dbg << "Unknown";
        break;
    case QGeoShape::RectangleType:
        dbg << "Rectangle";
        break;
    case QGeoShape::PathType:
        dbg << "Path";
        break;
    case QGeoShape::PolygonType:
        dbg << "Polygon";
        break;
    case QGeoShape::CircleType:
        dbg << "Circle";
    }

    dbg << ')';

    return dbg;
}
#endif

#ifndef QT_NO_DATASTREAM
QDataStream &QGeoShape::dataStreamOut(QDataStream &stream, const QGeoShape &shape)
{
    stream << quint32(shape.type());
    switch (shape.type()) {
    case QGeoShape::UnknownType:
        break;
    case QGeoShape::RectangleType: {
        QGeoRectangle r = shape;
        stream << r.topLeft() << r.bottomRight();
        break;
    }
    case QGeoShape::CircleType: {
        QGeoCircle c = shape;
        stream << c.center() << c.radius();
        break;
    }
    case QGeoShape::PathType: {
        QGeoPath p = shape;
        stream << p.width();
        stream << p.path().size();
        for (const auto &c: p.path())
            stream << c;
        break;
    }
    case QGeoShape::PolygonType: {
        QGeoPolygon p = shape;
        stream << p.perimeter().size();
        for (const auto &c: p.perimeter())
            stream << c;
        if (stream.version() >= QDataStream::Qt_6_10) {
            // serialize holes
            const qsizetype holesCount = p.holesCount();
            stream << holesCount;
            for (qsizetype i = 0; i < holesCount; ++i)
                stream << p.holePath(i);
        }
        break;
    }
    }

    return stream;
}

QDataStream &QGeoShape::dataStreamIn(QDataStream &stream, QGeoShape &shape)
{
    quint32 type;
    stream >> type;

    switch (type) {
    case QGeoShape::UnknownType:
        shape = QGeoShape();
        break;
    case QGeoShape::RectangleType: {
        QGeoCoordinate tl;
        QGeoCoordinate br;
        stream >> tl >> br;
        shape = QGeoRectangle(tl, br);
        break;
    }
    case QGeoShape::CircleType: {
        QGeoCoordinate c;
        qreal r;
        stream >> c >> r;
        shape = QGeoCircle(c, r);
        break;
    }
    case QGeoShape::PathType: {
        QList<QGeoCoordinate> l;
        QGeoCoordinate c;
        qreal width;
        stream >> width;
        qsizetype sz;
        stream >> sz;
        for (qsizetype i = 0; i < sz; i++) {
            stream >> c;
            l.append(c);
        }
        shape = QGeoPath(l, width);
        break;
    }
    case QGeoShape::PolygonType: {
        QList<QGeoCoordinate> l;
        QGeoCoordinate c;
        qsizetype sz;
        stream >> sz;
        for (qsizetype i = 0; i < sz; i++) {
            stream >> c;
            l.append(c);
        }
        QGeoPolygon p(l);
        if (stream.version() >= QDataStream::Qt_6_10) {
            // deserialize holes
            qsizetype holesCount = 0;
            stream >> holesCount;
            for (qsizetype i = 0; i < holesCount; ++i) {
                QList<QGeoCoordinate> holePath;
                stream >> holePath;
                p.addHole(holePath);
            }
        }
        shape = p;
        break;
    }
    }

    return stream;
}
#endif

/*!
    \relates QGeoShape

    Returns the hash value for the \a shape, using \a seed for the
    calculation.
*/
size_t qHash(const QGeoShape &shape, size_t seed) noexcept
{
    if (shape.d_ptr)
        return shape.d_ptr->hash(seed);
    else
        return qHashMulti(seed, shape.type());
}

QT_END_NAMESPACE

#include "moc_qgeoshape.cpp"

