// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qgeocoordinate.h"
#include "qgeocoordinate_p.h"
#include "qlocationutils_p.h"

#include <QDateTime>
#include <QHash>
#include <QDataStream>
#include <QDebug>
#include <QMetaType>
#include <qnumeric.h>
#include <qmath.h>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QGeoCoordinate)

static const double qgeocoordinate_EARTH_MEAN_RADIUS = 6371.0072;


QGeoCoordinatePrivate::QGeoCoordinatePrivate():
    lat(qQNaN()),
    lng(qQNaN()),
    alt(qQNaN())
{}

QGeoCoordinatePrivate::QGeoCoordinatePrivate(const QGeoCoordinatePrivate &other)
    : QSharedData(other),
      lat(other.lat),
      lng(other.lng),
      alt(other.alt)
{}

QGeoCoordinatePrivate::~QGeoCoordinatePrivate()
{}


QGeoMercatorCoordinatePrivate::QGeoMercatorCoordinatePrivate():
    QGeoCoordinatePrivate(),
    m_mercatorX(qQNaN()),
    m_mercatorY(qQNaN())
{}

QGeoMercatorCoordinatePrivate::QGeoMercatorCoordinatePrivate(const QGeoMercatorCoordinatePrivate &other)
    : QGeoCoordinatePrivate(other),
      m_mercatorX(other.m_mercatorX),
      m_mercatorY(other.m_mercatorY)
{}

QGeoMercatorCoordinatePrivate::~QGeoMercatorCoordinatePrivate()
{}

/*!
    \class QGeoCoordinate
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \since 5.2

    \brief The QGeoCoordinate class defines a geographical position on the surface of the Earth.

    A QGeoCoordinate is defined by latitude, longitude, and optionally, altitude.

    Use type() to determine whether a coordinate is a 2D coordinate (has
    latitude and longitude only) or 3D coordinate (has latitude, longitude
    and altitude). Use distanceTo() and azimuthTo() to calculate the distance
    and bearing between coordinates.

    The coordinate values should be specified using the WGS84 datum. For more information
    on geographical terms see this article on \l {http://en.wikipedia.org/wiki/Geographic_coordinate_system}{coordinates} and
    another on \l {http://en.wikipedia.org/wiki/Geodetic_system}{geodetic systems}
    including WGS84.

    Azimuth in this context is equivalent to a compass bearing based on true north.

    This class is also accessible in QML as \l[QML]{geoCoordinate}.
*/

/*!
    \enum QGeoCoordinate::CoordinateType
    Defines the types of a coordinate.

    \value InvalidCoordinate An invalid coordinate. A coordinate is invalid if its latitude or longitude values are invalid.
    \value Coordinate2D A coordinate with valid latitude and longitude values.
    \value Coordinate3D A coordinate with valid latitude and longitude values, and also an altitude value.
*/

/*!
    \enum QGeoCoordinate::CoordinateFormat
    Defines the possible formatting options for toString().

    \value Degrees Returns a string representation of the coordinates in decimal degrees format.
    \value DegreesWithHemisphere Returns a string representation of the coordinates in decimal degrees format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutes Returns a string representation of the coordinates in degrees-minutes format.
    \value DegreesMinutesWithHemisphere Returns a string representation of the coordinates in degrees-minutes format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutesSeconds Returns a string representation of the coordinates in degrees-minutes-seconds format.
    \value DegreesMinutesSecondsWithHemisphere Returns a string representation of the coordinates in degrees-minutes-seconds format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.

    \sa toString()
*/

/*!
    \property QGeoCoordinate::latitude
    \brief This property holds the latitude in decimal degrees.

    The property is undefined (\l {qQNaN()}) if the latitude has not been set.
    A positive latitude indicates the Northern Hemisphere, and a negative
    latitude indicates the Southern Hemisphere. When setting the latitude the
    new value should be in the
    \l {http://en.wikipedia.org/wiki/World_Geodetic_System}{WGS84} datum format.

    To be valid, the latitude must be between -90 to 90 inclusive.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    \property QGeoCoordinate::longitude
    \brief This property holds the longitude in decimal degrees.

    The property is undefined (\l {qQNaN()}) if the longitude has not been set.
    A positive longitude indicates the Eastern Hemisphere, and a negative
    longitude indicates the Western Hemisphere. When setting the longitude the
    new value should be in the
    \l {http://en.wikipedia.org/wiki/World_Geodetic_System}{WGS84} datum format.

    To be valid, the longitude must be between -180 to 180 inclusive.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    \property QGeoCoordinate::altitude
    \brief This property holds the altitude in meters above sea level.

    The property is undefined (\l {qQNaN()}) if the altitude has not been set.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    \property QGeoCoordinate::isValid
    \brief This property holds the validity of this geo coordinate.

    The geo coordinate is valid if the \l [CPP]{longitude} and \l [CPP]{latitude}
    properties have been set to valid values.

    While this property is introduced in Qt 5.5, the related accessor functions
    exist since the first version of this class.

    \since 5.5
*/

/*!
    Constructs a coordinate. The coordinate will be invalid until
    setLatitude() and setLongitude() have been called.
*/
QGeoCoordinate::QGeoCoordinate()
        : d(new QGeoCoordinatePrivate)
{
}

/*!
    Constructs a coordinate with the given \a latitude and \a longitude.

    If the latitude is not between -90 to 90 inclusive, or the longitude
    is not between -180 to 180 inclusive, none of the values are set and
    the type() will be QGeoCoordinate::InvalidCoordinate.

    \sa isValid()
*/
QGeoCoordinate::QGeoCoordinate(double latitude, double longitude)
        : d(new QGeoCoordinatePrivate)
{
    if (QLocationUtils::isValidLat(latitude) && QLocationUtils::isValidLong(longitude)) {
        d->lat = latitude;
        d->lng = longitude;
    }
}

/*!
    Constructs a coordinate with the given \a latitude, \a longitude
    and \a altitude.

    If the latitude is not between -90 to 90 inclusive, or the longitude
    is not between -180 to 180 inclusive, none of the values are set and
    the type() will be QGeoCoordinate::InvalidCoordinate.

    Note that \a altitude specifies the meters above sea level.

    \sa isValid()
*/
QGeoCoordinate::QGeoCoordinate(double latitude, double longitude, double altitude)
        : d(new QGeoCoordinatePrivate)
{
    if (QLocationUtils::isValidLat(latitude) && QLocationUtils::isValidLong(longitude)) {
        d->lat = latitude;
        d->lng = longitude;
        d->alt = altitude;
    }
}

/*!
    Constructs a coordinate from the contents of \a other.
*/
QGeoCoordinate::QGeoCoordinate(const QGeoCoordinate &other)
        : d(other.d)
{}

/*!
    \fn QGeoCoordinate::QGeoCoordinate(QGeoCoordinate &&other)
    \since 6.2

    Constructs a coordinate by moving from \a other.

    \note The moved-from QGeoCoordinate object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor
    or one of the assignment operators is undefined.
*/

/*!
    Assigns \a other to this coordinate and returns a reference to this coordinate.
*/
QGeoCoordinate &QGeoCoordinate::operator=(const QGeoCoordinate &other)
{
    if (this == &other)
        return *this;

    d = other.d;
    return (*this);
}

/*!
    \fn QGeoCoordinate &QGeoCoordinate::operator=(QGeoCoordinate &&other)
    \since 6.2

    Move-assigns \a other to this coordinate and returns a reference to this
    coordinate.

    \note The moved-from QGeoCoordinate object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor
    or one of the assignment operators is undefined.
*/

/*!
    Destroys the coordinate object.
*/
QGeoCoordinate::~QGeoCoordinate()
{
}

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QGeoCoordinatePrivate)

/*!
    \fn bool QGeoCoordinate::operator==(const QGeoCoordinate &lhs, const QGeoCoordinate &rhs)

    Returns \c true if the latitude, longitude and altitude of the \a lhs
    coordinate are the same as those of the \a rhs coordinate. Otherwise
    returns \c false.

    The longitude will be ignored if the latitude is +/- 90 degrees.
*/

/*!
    \fn bool QGeoCoordinate::operator!=(const QGeoCoordinate &lhs, const QGeoCoordinate &rhs)

    Returns \c true if latitude, longitude, or altitude of the \a lhs
    coordinate are not identical to those of the \a rhs coordinate. Otherwise
    returns \c false.
*/

/*!
    \fn void QGeoCoordinate::swap(QGeoCoordinate &other)
    \since 6.2
    \memberswap{coordinate}
*/

/*!
    Returns \c true if the \l longitude and \l latitude are valid.
*/
bool QGeoCoordinate::isValid() const
{
    CoordinateType t = type();
    return t == Coordinate2D || t == Coordinate3D;
}

/*!
    Returns the type of this coordinate.
*/
QGeoCoordinate::CoordinateType QGeoCoordinate::type() const
{
    if (QLocationUtils::isValidLat(d->lat)
            && QLocationUtils::isValidLong(d->lng)) {
        if (qIsNaN(d->alt))
            return Coordinate2D;
        return Coordinate3D;
    }
    return InvalidCoordinate;
}


/*!
    Returns the latitude, in decimal degrees. The return value is undefined
    if the latitude has not been set.

    A positive latitude indicates the Northern Hemisphere, and a negative
    latitude indicates the Southern Hemisphere.

    \sa setLatitude(), type()
*/
double QGeoCoordinate::latitude() const
{
    return d->lat;
}

/*!
    Sets the latitude (in decimal degrees) to \a latitude. The value should
    be in the WGS84 datum.

    To be valid, the latitude must be between -90 to 90 inclusive.

    \sa latitude()
*/
void QGeoCoordinate::setLatitude(double latitude)
{
    d->lat = latitude;
}

/*!
    Returns the longitude, in decimal degrees. The return value is undefined
    if the longitude has not been set.

    A positive longitude indicates the Eastern Hemisphere, and a negative
    longitude indicates the Western Hemisphere.

    \sa setLongitude(), type()
*/
double QGeoCoordinate::longitude() const
{
    return d->lng;
}

/*!
    Sets the longitude (in decimal degrees) to \a longitude. The value should
    be in the WGS84 datum.

    To be valid, the longitude must be between -180 to 180 inclusive.

    \sa longitude()
*/
void QGeoCoordinate::setLongitude(double longitude)
{
    d->lng = longitude;
}

/*!
    Returns the altitude (meters above sea level).

    The return value is undefined if the altitude has not been set.

    \sa setAltitude(), type()
*/
double QGeoCoordinate::altitude() const
{
    return d->alt;
}

/*!
    Sets the altitude (meters above sea level) to \a altitude.

    \sa altitude()
*/
void QGeoCoordinate::setAltitude(double altitude)
{
    d->alt = altitude;
}

/*!
    Returns the distance (in meters) from this coordinate to the coordinate
    specified by \a other. Altitude is not used in the calculation.

    This calculation returns the great-circle distance between the two
    coordinates, with an assumption that the Earth is spherical for the
    purpose of this calculation.

    Returns 0 if the type of this coordinate or the type of \a other is
    QGeoCoordinate::InvalidCoordinate.
*/
qreal QGeoCoordinate::distanceTo(const QGeoCoordinate &other) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate
            || other.type() == QGeoCoordinate::InvalidCoordinate) {
        return 0;
    }

    // Haversine formula
    double dlat = qDegreesToRadians(other.d->lat - d->lat);
    double dlon = qDegreesToRadians(other.d->lng - d->lng);
    double haversine_dlat = sin(dlat / 2.0);
    haversine_dlat *= haversine_dlat;
    double haversine_dlon = sin(dlon / 2.0);
    haversine_dlon *= haversine_dlon;
    double y = haversine_dlat
             + cos(qDegreesToRadians(d->lat))
             * cos(qDegreesToRadians(other.d->lat))
             * haversine_dlon;
    double x = 2 * asin(sqrt(y));
    return qreal(x * qgeocoordinate_EARTH_MEAN_RADIUS * 1000);
}

/*!
    Returns the azimuth (or bearing) in degrees from this coordinate to the
    coordinate specified by \a other. Altitude is not used in the calculation.

    The bearing returned is the bearing from the origin to \a other along the
    great-circle between the two coordinates. There is an assumption that the
    Earth is spherical for the purpose of this calculation.

    Returns 0 if the type of this coordinate or the type of \a other is
    QGeoCoordinate::InvalidCoordinate.
*/
qreal QGeoCoordinate::azimuthTo(const QGeoCoordinate &other) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate
            || other.type() == QGeoCoordinate::InvalidCoordinate) {
        return 0;
    }

    double dlon = qDegreesToRadians(other.d->lng - d->lng);
    double lat1Rad = qDegreesToRadians(d->lat);
    double lat2Rad = qDegreesToRadians(other.d->lat);

    double y = sin(dlon) * cos(lat2Rad);
    double x = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(dlon);

    double azimuth = qRadiansToDegrees(atan2(y, x)) + 360.0;
    double whole;
    double fraction = modf(azimuth, &whole);
    return qreal((int(whole + 360) % 360) + fraction);
}

void QGeoCoordinatePrivate::atDistanceAndAzimuth(const QGeoCoordinate &coord,
                                                 qreal distance, qreal azimuth,
                                                 double *lon, double *lat)
{
    double latRad = qDegreesToRadians(coord.d->lat);
    double lonRad = qDegreesToRadians(coord.d->lng);
    double cosLatRad = cos(latRad);
    double sinLatRad = sin(latRad);

    double azimuthRad = qDegreesToRadians(azimuth);

    double ratio = (distance / (qgeocoordinate_EARTH_MEAN_RADIUS * 1000.0));
    double cosRatio = cos(ratio);
    double sinRatio = sin(ratio);

    double resultLatRad = asin(sinLatRad * cosRatio
                               + cosLatRad * sinRatio * cos(azimuthRad));
    double resultLonRad = lonRad + atan2(sin(azimuthRad) * sinRatio * cosLatRad,
                                   cosRatio - sinLatRad * sin(resultLatRad));

    *lat = qRadiansToDegrees(resultLatRad);
    *lon = qRadiansToDegrees(resultLonRad);
}

/*!
    Returns the coordinate that is reached by traveling \a distance meters
    from the current coordinate at \a azimuth (or bearing) along a great-circle.
    There is an assumption that the Earth is spherical for the purpose of this
    calculation.

    The altitude will have \a distanceUp added to it.

    Returns an invalid coordinate if this coordinate is invalid.
*/
QGeoCoordinate QGeoCoordinate::atDistanceAndAzimuth(qreal distance, qreal azimuth, qreal distanceUp) const
{
    if (!isValid())
        return QGeoCoordinate();

    double resultLon, resultLat;
    QGeoCoordinatePrivate::atDistanceAndAzimuth(*this, distance, azimuth,
                                                &resultLon, &resultLat);
    double resultAlt = d->alt + distanceUp;
    return QGeoCoordinate(resultLat, QLocationUtils::wrapLong(resultLon), resultAlt);
}

/*!
    Returns this coordinate as a string in the specified \a format.

    For example, if this coordinate has a latitude of -27.46758, a longitude
    of 153.027892 and an altitude of 28.1, these are the strings
    returned depending on \a format:

    \table
    \header
        \li \a format value
        \li Returned string
    \row
        \li \l Degrees
        \li -27.46758\unicode{0xB0}, 153.02789\unicode{0xB0}, 28.1m
    \row
        \li \l DegreesWithHemisphere
        \li 27.46758\unicode{0xB0} S, 153.02789\unicode{0xB0} E, 28.1m
    \row
        \li \l DegreesMinutes
        \li -27\unicode{0xB0} 28.054', 153\unicode{0xB0} 1.673', 28.1m
    \row
        \li \l DegreesMinutesWithHemisphere
        \li 27\unicode{0xB0} 28.054 S', 153\unicode{0xB0} 1.673' E, 28.1m
    \row
        \li \l DegreesMinutesSeconds
        \li -27\unicode{0xB0} 28' 3.2", 153\unicode{0xB0} 1' 40.4", 28.1m
    \row
        \li \l DegreesMinutesSecondsWithHemisphere
        \li 27\unicode{0xB0} 28' 3.2" S, 153\unicode{0xB0} 1' 40.4" E, 28.1m
    \endtable

    The altitude field is omitted if no altitude is set.

    If the coordinate is invalid, an empty string is returned.
*/
QString QGeoCoordinate::toString(CoordinateFormat format) const
{
    if (type() == QGeoCoordinate::InvalidCoordinate)
        return QString();

    QString latStr;
    QString longStr;

    double absLat = qAbs(d->lat);
    double absLng = qAbs(d->lng);

    switch (format) {
        case Degrees:
        case DegreesWithHemisphere: {
            latStr = QString::asprintf("%.5f°", absLat);
            longStr = QString::asprintf("%.5f°", absLng);
            break;
        }
        case DegreesMinutes:
        case DegreesMinutesWithHemisphere: {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;

            // We use QString::number(val, 'f', 3) to represent minutes.
            // It rounds up to the next integer in case the fraction > 0.9995.
            // Such behavior should be handled specifically when the rounded
            // value is 60, so that we overflow to degrees correctly.
            // If we overflow, the minutes should unconditionally be 0.0.
            if (latMin > 59.9995) {
                absLat++;
                latMin = 0.0f;
            }
            if (lngMin > 59.9995) {
                absLng++;
                lngMin = 0.0f;
            }

            latStr = QString::asprintf("%d° %.3f'", int(absLat), latMin);
            longStr = QString::asprintf("%d° %.3f'", int(absLng), lngMin);
            break;
        }
        case DegreesMinutesSeconds:
        case DegreesMinutesSecondsWithHemisphere: {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;
            double latSec = (latMin - int(latMin)) * 60;
            double lngSec = (lngMin - int(lngMin)) * 60;

            // We use QString::number(val, 'f', 1) to represent seconds.
            // It rounds up to the next integer in case the fraction >= 0.95.
            // Such behavior should be handled specifically when the rounded
            // value is 60, so that we overflow to minutes correctly.
            // If we overflow, the seconds should unconditionally be 0.0.
            if (latSec >= 59.95) {
                latMin++;
                latSec = 0.0f;
                // We cast to int to represent minutes, so we can use qRound()
                // to determine if we need to overflow to full degrees.
                // If we overflow, the minutes will unconditionally be 0.0.
                if (qRound(latMin) >= 60) {
                    absLat++;
                    latMin = 0.0f;
                }
            }
            if (lngSec >= 59.95) {
                lngMin++;
                lngSec = 0.0f;
                if (qRound(lngMin) >= 60) {
                    absLng++;
                    lngMin = 0.0f;
                }
            }

            latStr = QString::asprintf("%d° %d' %.1f\"", int(absLat), int(latMin), latSec);
            longStr = QString::asprintf("%d° %d' %.1f\"", int(absLng), int(lngMin), lngSec);
            break;
        }
    }

    // now add the "-" to the start, or append the hemisphere char
    switch (format) {
        case Degrees:
        case DegreesMinutes:
        case DegreesMinutesSeconds: {
            if (d->lat < 0)
                latStr.insert(0, QStringLiteral("-"));
            if (d->lng < 0)
                longStr.insert(0, QStringLiteral("-"));
            break;
        }
        case DegreesWithHemisphere:
        case DegreesMinutesWithHemisphere:
        case DegreesMinutesSecondsWithHemisphere: {
            if (d->lat < 0)
                latStr.append(QString::fromLatin1(" S"));
            else if (d->lat > 0)
                latStr.append(QString::fromLatin1(" N"));
            if (d->lng < 0)
                longStr.append(QString::fromLatin1(" W"));
            else if (d->lng > 0)
                longStr.append(QString::fromLatin1(" E"));
            break;
        }
    }

    if (qIsNaN(d->alt))
        return QString::fromLatin1("%1, %2").arg(latStr, longStr);
    return QString::fromLatin1("%1, %2, %3m").arg(latStr, longStr, QString::number(d->alt));
}

bool QGeoCoordinate::equals(const QGeoCoordinate &lhs, const QGeoCoordinate &rhs)
{
    bool latEqual = (qIsNaN(lhs.d->lat) && qIsNaN(rhs.d->lat))
                        || QtPrivate::fuzzyCompare(lhs.d->lat, rhs.d->lat);
    bool lngEqual = (qIsNaN(lhs.d->lng) && qIsNaN(rhs.d->lng))
                        || QtPrivate::fuzzyCompare(lhs.d->lng, rhs.d->lng);
    bool altEqual = (qIsNaN(lhs.d->alt) && qIsNaN(rhs.d->alt))
                        || QtPrivate::fuzzyCompare(lhs.d->alt, rhs.d->alt);

    if (!qIsNaN(lhs.d->lat) && ((lhs.d->lat == 90.0) || (lhs.d->lat == -90.0)))
        lngEqual = true;

    return (latEqual && lngEqual && altEqual);
}

QGeoCoordinate::QGeoCoordinate(QGeoCoordinatePrivate &dd):
    d(&dd)
{
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QGeoCoordinate::debugStreaming(QDebug dbg, const QGeoCoordinate &coord)
{
    QDebugStateSaver saver(dbg);
    double lat = coord.latitude();
    double lng = coord.longitude();

    QTextStreamManipulator tsm = qSetRealNumberPrecision(11);
    dbg << tsm;
    dbg.nospace() << "QGeoCoordinate(";
    if (qIsNaN(lat))
        dbg << '?';
    else
        dbg << lat;
    dbg << ", ";
    if (qIsNaN(lng))
        dbg << '?';
    else
        dbg << lng;
    if (coord.type() == QGeoCoordinate::Coordinate3D) {
        dbg << ", ";
        dbg << coord.altitude();
    }
    dbg << ')';
    return dbg;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &QGeoCoordinate::operator<<(QDataStream &stream, const QGeoCoordinate &coordinate)

    Writes the given \a coordinate to the specified \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &QGeoCoordinate::dataStreamOut(QDataStream &stream, const QGeoCoordinate &coordinate)
{
    stream << coordinate.latitude();
    stream << coordinate.longitude();
    stream << coordinate.altitude();
    return stream;
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \fn  QDataStream &QGeoCoordinate::operator>>(QDataStream &stream, QGeoCoordinate &coordinate)

    Reads a coordinate from the specified \a stream into the given
    \a coordinate.

    \sa {Serializing Qt Data Types}
*/

QDataStream &QGeoCoordinate::dataStreamIn(QDataStream &stream, QGeoCoordinate &coordinate)
{
    double value;
    stream >> value;
    coordinate.setLatitude(value);
    stream >> value;
    coordinate.setLongitude(value);
    stream >> value;
    coordinate.setAltitude(value);
    return stream;
}
#endif

/*! \fn size_t qHash(const QGeoCoordinate &coordinate, size_t seed = 0)
    \relates QHash

    Returns a hash value for \a coordinate, using \a seed to seed the calculation.
*/
size_t qHash(const QGeoCoordinate &coordinate, size_t seed)
{
    QtPrivate::QHashCombine hash(seed);
    // north and south pole are geographically equivalent (no matter the longitude)
    if (coordinate.latitude() != 90.0 && coordinate.latitude() != -90.0)
        seed = hash(seed, coordinate.longitude());
    seed = hash(seed, coordinate.latitude());
    seed = hash(seed, coordinate.altitude());
    return seed;
}

QT_END_NAMESPACE

#include "moc_qgeocoordinate.cpp"
