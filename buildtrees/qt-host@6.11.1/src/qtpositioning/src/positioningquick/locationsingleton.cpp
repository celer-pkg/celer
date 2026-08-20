// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "locationsingleton_p.h"
#include <QtPositioning/private/qwebmercator_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

static QGeoCoordinate parseCoordinate(const QJSValue &value, bool *ok)
{
    QGeoCoordinate c;

    if (value.isObject()) {
        if (value.hasProperty(QStringLiteral("latitude")))
            c.setLatitude(value.property(QStringLiteral("latitude")).toNumber());
        if (value.hasProperty(QStringLiteral("longitude")))
            c.setLongitude(value.property(QStringLiteral("longitude")).toNumber());
        if (value.hasProperty(QStringLiteral("altitude")))
            c.setAltitude(value.property(QStringLiteral("altitude")).toNumber());

        if (ok)
            *ok = true;
    } else if (ok) {
        *ok = false;
    }

    return c;
}


/*!
    \qmltype QtPositioning
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The QtPositioning global object provides useful functions for working with location-based
           types in QML.

    \qml
    import QtPositioning

    Item {
        property var coordinate: QtPositioning.coordinate(-27.5, 153.1)
    }
    \endqml
*/

LocationSingleton::LocationSingleton(QObject *parent)
:   QObject(parent)
{
}

/*!
    \qmlmethod coordinate QtPositioning::coordinate()

    Constructs an invalid coordinate.

*/
QGeoCoordinate LocationSingleton::coordinate() const
{
    return QGeoCoordinate();
}

/*!
    \qmlmethod coordinate QtPositioning::coordinate(real latitude, real longitude, real altitude) const

    Constructs a coordinate with the specified \a latitude, \a longitude and optional \a altitude.
    Both \a latitude and \a longitude must be valid, otherwise an invalid coordinate is returned.

    \sa {coordinate}
*/
QGeoCoordinate LocationSingleton::coordinate(double latitude, double longitude, double altitude) const
{
    return QGeoCoordinate(latitude, longitude, altitude);
}

/*!
    \qmlmethod geoShape QtPositioning::shape() const

    Constructs an invalid geoShape.

    \sa {geoShape}
*/
QGeoShape LocationSingleton::shape() const
{
    return QGeoShape();
}

/*!
    \qmlmethod geoRectangle QtPositioning::rectangle() const

    Constructs an invalid geoRectangle.

    \sa {geoRectangle}
*/
QGeoRectangle LocationSingleton::rectangle() const
{
    return QGeoRectangle();
}

/*!
    \qmlmethod geoRectangle QtPositioning::rectangle(coordinate center, real width, real height) const

    Constructs a geoRectangle centered at \a center with a width of \a width degrees and a hight of
    \a height degrees.

    \sa {geoRectangle}
*/
QGeoRectangle LocationSingleton::rectangle(const QGeoCoordinate &center,
                                           double width, double height) const
{
    return QGeoRectangle(center, width, height);
}

/*!
    \qmlmethod geoRectangle QtPositioning::rectangle(coordinate topLeft, coordinate bottomRight) const

    Constructs a geoRectangle with its top left corner positioned at \a topLeft and its bottom
    right corner positioned at \a {bottomRight}.

    \sa {geoRectangle}
*/
QGeoRectangle LocationSingleton::rectangle(const QGeoCoordinate &topLeft,
                                           const QGeoCoordinate &bottomRight) const
{
    return QGeoRectangle(topLeft, bottomRight);
}

/*!
    \qmlmethod geoRectangle QtPositioning::rectangle(list<coordinate> coordinates) const

    Constructs a geoRectangle from the list of coordinates, the returned list is the smallest possible
    containing all the coordinates.

    \sa {geoRectangle}
*/
QGeoRectangle LocationSingleton::rectangle(const QVariantList &coordinates) const
{
    QList<QGeoCoordinate> internalCoordinates;
    for (const auto &coordinate : coordinates) {
        if (coordinate.canConvert<QGeoCoordinate>())
            internalCoordinates << coordinate.value<QGeoCoordinate>();
    }

    return QGeoRectangle(internalCoordinates);
}

/*!
    \qmlmethod geoCircle QtPositioning::circle() const

    Constructs an invalid geoCircle.

    \sa {geoCircle}
*/
QGeoCircle LocationSingleton::circle() const
{
    return QGeoCircle();
}

/*!
    \qmlmethod geoCircle QtPositioning::circle(coordinate center, real radius) const

    Constructs a geoCircle centered at \a center with a radius of \a radius meters.
*/
QGeoCircle LocationSingleton::circle(const QGeoCoordinate &center, qreal radius) const
{
    return QGeoCircle(center, radius);
}

/*!
    \qmlmethod geoPath QtPositioning::path() const

    Constructs an empty geoPath.

    \sa {geoPath}
    \since 5.9
*/
QGeoPath LocationSingleton::path() const
{
    return QGeoPath();
}

/*!
    \qmlmethod geoPath QtPositioning::path(list<coordinate> coordinates, real width) const

    Constructs a geoPath from coordinates and width.

    \sa {geoPath}
    \since 5.9
*/
QGeoPath LocationSingleton::path(const QJSValue &value, qreal width) const
{
    QList<QGeoCoordinate> pathList;

    if (value.isArray()) {
        quint32 length = value.property(QStringLiteral("length")).toUInt();
        for (quint32 i = 0; i < length; ++i) {
            bool ok = false;
            QGeoCoordinate c = parseCoordinate(value.property(i), &ok);

            if (!ok || !c.isValid()) {
                pathList.clear(); // aborting
                break;
            }

            pathList.append(c);
        }
    }

    return QGeoPath(pathList, width);
}

/*!
    \qmlmethod geoPolygon QtPositioning::polygon() const

    Constructs an empty polygon.

    \sa {geoPolygon}
    \since 5.10
*/
QGeoPolygon LocationSingleton::polygon() const
{
    return QGeoPolygon();
}

/*!
    \qmlmethod geoPolygon QtPositioning::polygon(list<coordinate> coordinates) const

    Constructs a polygon from coordinates.

    \sa {geoPolygon}
    \since 5.10
*/
QGeoPolygon LocationSingleton::polygon(const QVariantList &coordinates) const
{
    QList<QGeoCoordinate> internalCoordinates;
    for (const auto &coordinate : coordinates) {
        if (coordinate.canConvert<QGeoCoordinate>())
            internalCoordinates << coordinate.value<QGeoCoordinate>();
    }

    return QGeoPolygon(internalCoordinates);
}

/*!
    \qmlmethod geoPolygon QtPositioning::polygon(list<coordinate> perimeter, list<list<coordinate>> holes) const

    Constructs a polygon from coordinates for perimeter and inner holes.

    \sa {geoPolygon}
    \since 5.12
*/
QGeoPolygon LocationSingleton::polygon(const QVariantList &perimeter, const QVariantList &holes) const
{
    QList<QGeoCoordinate> internalCoordinates;
    for (const auto &coordinate : perimeter) {
        if (coordinate.canConvert<QGeoCoordinate>())
            internalCoordinates << coordinate.value<QGeoCoordinate>();
    }
    QGeoPolygon poly(internalCoordinates);

    for (const auto &h : holes) {
        if (h.metaType().id() == QMetaType::QVariantList) {
            QList<QGeoCoordinate> hole;
            const QVariantList &holeData = h.toList();
            for (const auto &coord : holeData) {
                if (coord.canConvert<QGeoCoordinate>())
                    hole << coord.value<QGeoCoordinate>();
            }
            if (!hole.isEmpty())
                poly.addHole(hole);
        }
    }

    return poly;
}

/*!
    \qmlmethod geoCircle QtPositioning::shapeToCircle(geoShape shape) const

    Converts \a shape to a geoCircle.

    \sa {geoCircle}
    \since 5.5
*/
QGeoCircle LocationSingleton::shapeToCircle(const QGeoShape &shape) const
{
    return QGeoCircle(shape);
}

/*!
    \qmlmethod geoRectangle QtPositioning::shapeToRectangle(geoShape shape) const

    Converts \a shape to a geoRectangle.

    \sa {geoRectangle}
    \since 5.5
*/
QGeoRectangle LocationSingleton::shapeToRectangle(const QGeoShape &shape) const
{
    return QGeoRectangle(shape);
}

/*!
    \qmlmethod geoPath QtPositioning::shapeToPath(geoShape shape) const

    Converts \a shape to a geoPath.

    \sa {geoPath}
    \since 5.9
*/
QGeoPath LocationSingleton::shapeToPath(const QGeoShape &shape) const
{
    return QGeoPath(shape);
}

/*!
    \qmlmethod geoPolygon QtPositioning::shapeToPolygon(geoShape shape) const

    Converts \a shape to a polygon.

    \sa {geoPolygon}
    \since 5.10
*/
QGeoPolygon LocationSingleton::shapeToPolygon(const QGeoShape &shape) const
{
    return QGeoPolygon(shape);
}

/*!
    \qmlmethod coordinate QtPositioning::mercatorToCoord(point mercator) const

    Converts a \a mercator coordinate into a latitude-longitude coordinate.

    \sa {coordToMercator}
    \since 5.12
*/
QGeoCoordinate LocationSingleton::mercatorToCoord(const QPointF &mercator) const
{
    return QWebMercator::mercatorToCoord(QDoubleVector2D(mercator.x(), mercator.y()));
}

/*!
    \qmlmethod point QtPositioning::coordToMercator(coordinate coord) const

    Converts a coordinate \a coord into a mercator coordinate and returns it.
    \sa {mercatorToCoord}
    \since 5.12
*/
QPointF LocationSingleton::coordToMercator(const QGeoCoordinate &coord) const
{
    return QWebMercator::coordToMercator(coord).toPointF();
}

QT_END_NAMESPACE

#include "moc_locationsingleton_p.cpp"
