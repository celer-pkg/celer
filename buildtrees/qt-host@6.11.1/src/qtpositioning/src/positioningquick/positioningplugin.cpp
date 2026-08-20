// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <private/qpositioningquickglobal_p.h>
#include <QGeoCircle>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPolygon>
#include <QGeoRectangle>
#include <QtPositioningQuick/private/qquickgeocoordinateanimation_p.h>
#include <QtCore/QVariantAnimation>
#include <QtQml/QQmlEngineExtensionPlugin>

QT_BEGIN_NAMESPACE

/*!
    \qmlvaluetype geoCoordinate
    \ingroup qmlvaluetypes
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The geoCoordinate type represents and stores a geographic position.

    This type is a QML representation of \l QGeoCoordinate and represents a geographic
    position in the form of \l {latitude}, \l longitude and \l altitude attributes.
    The \l latitude attribute specifies the number of
    decimal degrees above and below the equator.  A positive latitude indicates the Northern
    Hemisphere and a negative latitude indicates the Southern Hemisphere.  The \l longitude
    attribute specifies the number of decimal degrees east and west.  A positive longitude
    indicates the Eastern Hemisphere and a negative longitude indicates the Western Hemisphere.
    The \l altitude attribute specifies the number of meters above sea level.  Together, these
    attributes specify a 3-dimensional position anywhere on or near the Earth's surface.

    The \l isValid attribute can be used to test if a coordinate is valid.  A coordinate is
    considered valid if it has a valid latitude and longitude.  A valid altitude is not required.
    The latitude must be between -90 and 90 inclusive and the longitude must be between -180 and
    180 inclusive.

    The geoCoordinate type is used by many other types in the Qt Location module, for specifying
    the position of an object on a Map, the current position of a device and many other tasks.
    They also feature a number of important utility methods that make otherwise complex
    calculations simple to use, such as \l {atDistanceAndAzimuth}().

    \section1 Accuracy

    The latitude, longitude and altitude attributes stored in the geoCoordinate type are represented
    as doubles, giving them approximately 16 decimal digits of precision -- enough to specify
    micrometers.  The calculations performed in geoCoordinate's methods such as \l {azimuthTo}() and
    \l {distanceTo}() also use doubles for all intermediate values, but the inherent inaccuracies in
    their spherical Earth model dominate the amount of error in their output.

    \section1 Example Usage

    To create a geoCoordinate use one of the methods described below.  In all cases, specifying the
    \l altitude attribute is optional.

    To create a geoCoordinate value, use the \l{QtPositioning::coordinate}{QtPositioning.coordinate()}
    function:

    \qml
    import QtPositioning

    Location { coordinate: QtPositioning.coordinate(-27.5, 153.1) }
    \endqml

    or as separate \l latitude, \l longitude and \l altitude components:

    \qml
    Location {
        coordinate {
            latitude: -27.5
            longitude: 153.1
        }
    }
    \endqml

    When integrating with C++, note that any QGeoCoordinate value passed into QML from C++ is
    automatically converted into a geoCoordinate value, and vice-versa.
*/

/*!
    \qmlproperty real geoCoordinate::latitude

    This property holds the latitude value of the geographical position
    (decimal degrees). A positive latitude indicates the Northern Hemisphere,
    and a negative latitude indicates the Southern Hemisphere.
    If the property has not been set, its default value is NaN.

    For more details see the \l {QGeoCoordinate::latitude} property
*/

/*!
    \qmlproperty real geoCoordinate::longitude

    This property holds the longitude value of the geographical position
    (decimal degrees). A positive longitude indicates the Eastern Hemisphere,
    and a negative longitude indicates the Western Hemisphere
    If the property has not been set, its default value is NaN.

    For more details see the \l {QGeoCoordinate::longitude} property
*/

/*!
    \qmlproperty real geoCoordinate::altitude

    This property holds the altitude value (meters above sea level).
    If the property has not been set, its default value is NaN.

    For more details see the \l {QGeoCoordinate::altitude} property
*/

/*!
    \qmlproperty bool geoCoordinate::isValid

    This property holds the current validity of the coordinate. Coordinates
    are considered valid if they have been set with a valid latitude and
    longitude (altitude is not required).

    The latitude must be between -90 to 90 inclusive to be considered valid,
    and the longitude must be between -180 to 180 inclusive to be considered
    valid.

    This is a read-only property.
*/

/*!
    \qmlmethod real geoCoordinate::distanceTo(geoCoordinate other)

    Returns the distance (in meters) from this coordinate to the coordinate specified by \a other.
    Altitude is not used in the calculation.

    This calculation returns the great-circle distance between the two coordinates, with an
    assumption that the Earth is spherical for the purpose of this calculation.
*/

/*!
    \qmlmethod real geoCoordinate::azimuthTo(geoCoordinate other)

    Returns the azimuth (or bearing) in degrees from this coordinate to the coordinate specified by
    \a other.  Altitude is not used in the calculation.

    There is an assumption that the Earth is spherical for the purpose of this calculation.
*/

/*!
    \qmlmethod geoCoordinate geoCoordinate::atDistanceAndAzimuth(real distance, real azimuth)

    Returns the coordinate that is reached by traveling \a distance metres from this coordinate at
    \a azimuth degrees along a great-circle.

    There is an assumption that the Earth is spherical for the purpose of this calculation.
*/

/*!
    \qmlvaluetype geoShape
    \ingroup qmlvaluetypes
    \inqmlmodule QtPositioning
    \since 5.2

    \brief A geoShape type represents an abstract geographic area.

    This type is a QML representation of \l QGeoShape which is an abstract geographic area.
    It includes attributes and methods common to all geographic areas. To create objects
    that represent a valid geographic area use \l {geoRectangle} or \l {geoCircle}.

    The \l isValid attribute can be used to test if the shape represents a valid geographic
    area.

    The \l isEmpty attribute can be used to test if the shape represents a region with a
    geometrical area of 0.

    The \l {contains}() method can be used to test if a \l geoCoordinate is
    within the shape.

    \section1 Example Usage

    To create a geoShape value, use the
    \l {QtPositioning::shape}{QtPositioning.shape()} function:

    \qml
    import QtPositioning

    Item {
        property geoShape region: QtPositioning.shape()
    }
    \endqml

    When integrating with C++, note that any QGeoShape value passed into QML from C++ is
    automatically converted into a geoShape value, and vice-versa.
*/

/*!
    \qmlproperty bool geoShape::isEmpty

    Returns whether this shape is empty. An empty shape is a region which has
    a geometrical area of 0.
*/

/*!
    \qmlproperty bool geoShape::isValid

    Returns whether this shape is valid.

    A shape is considered to be invalid if some of the data that is required to
    unambiguously describe the shape has not been set or has been set to an
    unsuitable value.
*/

/*!
    \qmlproperty ShapeType geoShape::type

    Returns the current type of the shape.

    \list
        \li \c GeoShape.UnknownType - The shape's type is not known.
        \li \c GeoShape.RectangleType - The shape is a \l geoRectangle.
        \li \c GeoShape.CircleType - The shape is a \l geoCircle.
        \li \c GeoShape.PathType - The shape is a \l geoPath. (Since Qt 5.9)
        \li \c GeoShape.PolygonType - The shape is a \l geoPolygon. (Since Qt 5.10)
    \endlist

    This QML property was introduced by Qt 5.5.
*/

/*!
    \qmlmethod bool geoShape::contains(geoCoordinate coord)

    Returns true if the \l {QtPositioning::geoCoordinate}{coordinate} specified by \a coord is within
    this shape; otherwise returns false.
*/

/*!
    \qmlvaluetype geoRectangle
    \ingroup qmlvaluetypes
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The geoRectangle type represents a rectangular geographic area.

    The geoRectangle type is a \l {geoShape} that represents a
    rectangular geographic area. The type is direct representation of a \l QGeoRectangle.
    It is defined by a pair of \l {geoCoordinate}{coordinates} which represent the top-left
    and bottom-right corners of the rectangle.  The coordinates are accessible
    from the \l topLeft and \l bottomRight attributes.

    A rectangle is considered invalid if the top-left or bottom-right coordinates are invalid
    or if the top-left coordinate is south of the bottom-right coordinate.

    The coordinates of the four corners of the rectangle can be accessed with the
    \l {topLeft}, \l {topRight}, \l {bottomLeft} and \l {bottomRight} attributes.  The \l center
    attribute can be used to get the coordinate of the center of the rectangle.  The \l width
    and \l height attributes can be used to get the width and height of the rectangle in
    degrees.  Setting one of these attributes will cause the other attributes to be adjusted
    accordingly.

    \section1 Limitations

    A geoRectangle can never cross the poles.

    If the height or center of a geoRectangle is adjusted such that it would cross one of the
    poles the height is modified such that the rectangle touches but does not cross the pole
    and that the center coordinate is still in the center of the rectangle.

    \section1 Example Usage

    To create a geoRectangle value, use the \l {QtPositioning::rectangle}{QtPositioning.rectangle()}
    function:

    \qml
    import QtPositioning

    Item {
        property geoRectangle region: QtPositioning.rectangle(QtPositioning.coordinate(-27.5, 153.1),
                                                              QtPositioning.coordinate(-27.6, 153.2))
    }
    \endqml

    When integrating with C++, note that any QGeoRectangle value passed into QML from C++ is
    automatically converted into a geoRectangle value, and vice-versa.
*/

/*!
    \qmlproperty geoCoordinate geoRectangle::bottomLeft

    This property holds the bottom left coordinate of this geoRectangle.
*/

/*!
    \qmlproperty geoCoordinate geoRectangle::bottomRight

    This property holds the bottom right coordinate of this geoRectangle.
*/

/*!
    \qmlproperty geoCoordinate geoRectangle::center

    This property holds the center coordinate of this geoRectangle. For more details
    see \l {QGeoRectangle::setCenter()}.
*/

/*!
    \qmlproperty double geoRectangle::height

    This property holds the height of this geoRectangle (in degrees). For more details
    see \l {QGeoRectangle::setHeight()}.

    \note If the geoRectangle is invalid, it is not possible to set the height. QtPositioning
    releases prior to Qt 5.5 permitted the setting of the height even on invalid geoRectangles.
*/

/*!
    \qmlproperty geoCoordinate geoRectangle::topLeft

    This property holds the top left coordinate of this geoRectangle.
*/

/*!
    \qmlproperty geoCoordinate geoRectangle::topRight

    This property holds the top right coordinate of this geoRectangle.
*/

/*!
    \qmlproperty double geoRectangle::width

    This property holds the width of this geoRectangle (in degrees). For more details
    see \l {QGeoRectangle::setWidth()}.

    \note If the geoRectangle is invalid, it is not possible to set the width. QtPositioning
    releases prior to Qt 5.5 permitted the setting of the width even on invalid geoRectangles.
*/

/*!
    \qmlvaluetype geoCircle
    \ingroup qmlvaluetypes
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The geoCircle type represents a circular geographic area.

    The geoCircle type is a \l {geoShape} that represents a circular
    geographic area. It is a direct representation of a \l QGeoCircle and is defined
    in terms of a \l {geoCoordinate}{coordinate} which specifies the \l center of the circle and
    a qreal which specifies the \l radius of the circle in meters.

    The circle is considered invalid if the \l center coordinate is invalid or if
    the \l radius is less than zero.

    \section1 Example Usage

    To create a geoCircle value, use the \l {QtPositioning::circle}{QtPositioning.circle()}
    function:

    \qml
    import QtPositioning

    Item {
        property geoCircle region: QtPositioning.circle(QtPositioning.coordinate(-27.5, 153.1), 1000)
    }
    \endqml

    When integrating with C++, any QGeoCircle value passed into QML from C++ is
    automatically converted into a geoCircle value. Similarly, geoCircle
    values are converted to QGeoCircle values when passed from QML to C++.
*/

/*!
    \qmlproperty geoCoordinate geoCircle::center

    This property holds the coordinate of the center of the geoCircle.
*/

/*!
    \qmlproperty real geoCircle::radius

    This property holds the radius of the geoCircle in meters.

    The default value for the radius is -1 indicating an invalid geoCircle area.
*/

/*!
    \qmlvaluetype geoPath
    \ingroup qmlvaluetypes
    \inqmlmodule QtPositioning
    \since 5.9

    \brief The geoPath type represents a geographic path.

    The geoPath type is a \l {geoShape} that represents a geographic
    path. It is a direct representation of a \l QGeoPath and is defined
    in terms of a \l {path} which holds the list of geoCoordinates in the
    path.

    The path is considered invalid if it is empty.

    When integrating with C++, note that any QGeoPath value passed into QML from C++ is
    automatically converted into a geoPath value, and vice versa.
*/

/*!
    \qmlproperty list<geoCoordinate> geoPath::path

    This property holds the list of coordinates defining the path.
*/

/*!
    \qmlproperty real geoPath::width

    This property holds the width of the path in meters. This is currently only used
    when calling the \l {geoShape::}{contains()} method.

    The default value for the width is 0.
*/

/*!
   \qmlvaluetype geoPolygon
   \ingroup qmlvaluetypes
   \inqmlmodule QtPositioning
   \since 5.10

   \brief The geoPolygon type represents a geographic polygon.

   The geoPolygon type is a \l [QML] geoShape that represents a geographic
   polygon. It is a direct representation of QGeoPolygon and is defined in
   terms of a \l path which holds a list of geoCoordinates in the polygon.

   The polygon is considered invalid if its path holds less than three
   coordinates.

   When integrating with C++, note that any QGeoPolygon value passed into QML
   is automatically converted into a geoPolygon, and vice versa.
*/

/*!
   \qmlproperty list<geoCoordinate> geoPolygon::path

   This property holds the list of coordinates defining the polygon.
*/

class QtPositioningDeclarativeModule: public QQmlEngineExtensionPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtPositioningDeclarativeModule(QObject *parent = 0) : QQmlEngineExtensionPlugin(parent)
    {
        volatile auto registration = &qml_register_types_QtPositioning;
        Q_UNUSED(registration)
    }
};

namespace {

bool parseCoordinate(const QVariantMap &map, QGeoCoordinate &c)
{
    if (const auto it = map.find(QStringLiteral("latitude")); it != map.end())
        c.setLatitude(it.value().toDouble());
    else
        c.setLatitude(qQNaN());
    if (const auto it = map.find(QStringLiteral("longitude")); it != map.end())
        c.setLongitude(it.value().toDouble());
    else
        c.setLongitude(qQNaN());
    if (const auto it = map.find(QStringLiteral("altitude")); it != map.end())
        c.setAltitude(it.value().toDouble());
    else
        c.setAltitude(qQNaN());

    // Not considering the case where the map is valid but containing NaNs.
    return c.isValid();
}

bool parseRectangle(const QVariantMap &map, QGeoRectangle &rect)
{
    if (const auto it = map.find(QStringLiteral("topLeft")); it != map.end())
        rect.setTopLeft(it.value().value<QGeoCoordinate>());
    if (const auto it = map.find(QStringLiteral("bottomLeft")); it != map.end())
        rect.setBottomLeft(it.value().value<QGeoCoordinate>());
    if (const auto it = map.find(QStringLiteral("topRight")); it != map.end())
        rect.setTopRight(it.value().value<QGeoCoordinate>());
    if (const auto it = map.find(QStringLiteral("bottomRight")); it != map.end())
        rect.setBottomRight(it.value().value<QGeoCoordinate>());
    if (const auto it = map.find(QStringLiteral("center")); it != map.end())
        rect.setCenter(it.value().value<QGeoCoordinate>());
    if (const auto it = map.find(QStringLiteral("width")); it != map.end())
        rect.setWidth(it.value().toDouble());
    if (const auto it = map.find(QStringLiteral("height")); it != map.end())
        rect.setHeight(it.value().toDouble());

    // Not considering the case where the map is valid but containing NaNs.
    return rect.isValid();
}
}

void QtPositioningDeclarative_initializeModule()
{
    QMetaType::registerConverter<QGeoRectangle, QGeoShape>();
    QMetaType::registerConverter<QGeoShape, QGeoRectangle>();
    QMetaType::registerConverter<QGeoShape, QGeoCircle>();
    QMetaType::registerConverter<QGeoCircle, QGeoShape>();
    QMetaType::registerConverter<QGeoShape, QGeoPath>();
    QMetaType::registerConverter<QGeoPath, QGeoShape>();
    QMetaType::registerConverter<QGeoShape, QGeoPolygon>();
    QMetaType::registerConverter<QGeoPolygon, QGeoShape>();

    if (!QMetaType::registerConverterFunction([](const void *src, void *target) -> bool {
        const QVariantMap &map = *static_cast<const QVariantMap *>(src);
        QGeoCoordinate &coord = *static_cast<QGeoCoordinate *>(target);
        return parseCoordinate(map, coord);
    }, QMetaType::fromType<QVariantMap>(), QMetaType::fromType<QGeoCoordinate>())) {
        qWarning("Failed to register conversion function from QVariantMap to QGeoCoordinate");
    }

    if (!QMetaType::registerConverterFunction([](const void *src, void *target) -> bool {
        const QVariantMap &map = *static_cast<const QVariantMap *>(src);
        QGeoRectangle &rect = *static_cast<QGeoRectangle *>(target);
        return parseRectangle(map, rect);
    }, QMetaType::fromType<QVariantMap>(), QMetaType::fromType<QGeoRectangle>())) {
        qWarning("Failed to register conversion function from QVariantMap to QGeoRectangle");
    }

    qRegisterAnimationInterpolator<QGeoCoordinate>(q_coordinateInterpolator);
}

Q_CONSTRUCTOR_FUNCTION(QtPositioningDeclarative_initializeModule)

QT_END_NAMESPACE

#include "positioningplugin.moc"
