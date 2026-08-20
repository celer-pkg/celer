// Copyright (C) 2016 Jolla Ltd.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QtNumeric>
#include "qdeclarativeposition_p.h"
#include <QtQml/qqml.h>
#include <qnmeapositioninfosource.h>
#include <QFile>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Position
    //! \nativetype QDeclarativePosition
    \inqmlmodule QtPositioning
    \since 5.2

    \brief The Position type holds positional data at a particular point in time,
    such as coordinate (longitude, latitude, altitude) and speed.

    The Position type holds values related to geographic location such as
    a \l coordinate (longitude, latitude, and altitude), the \l timestamp when
    the Position was obtained, the \l speed at that time, and the accuracy of
    the data.

    Primarily, it is used in the \l{PositionSource::position}{position} property
    of a \l{PositionSource}, as the basic unit of data available from the system
    location data source.

    Not all properties of a Position object are necessarily valid or available
    (for example latitude and longitude may be valid, but speed update has not been
    received or set manually). As a result, corresponding "valid" properties
    are available (for example \l{coordinate} and \l{longitudeValid}, \l{latitudeValid}
    etc) to discern whether the data is available and valid in this position
    update.

    Position objects are read-only and can only be produced by a PositionSource.

    \section2 Example Usage

    See the example given for the \l {PositionSource} type, or the
    \l {Satellite Info} example application.

    \sa PositionSource, coordinate
*/

namespace
{

bool equalOrNaN(qreal a, qreal b)
{
    return a == b || (qIsNaN(a) && qIsNaN(b));
}

bool exclusiveNaN(qreal a, qreal b)
{
    return qIsNaN(a) != qIsNaN(b);
}

}

QDeclarativePosition::QDeclarativePosition(QObject *parent)
:   QObject(parent)
{
}

QDeclarativePosition::~QDeclarativePosition()
{
}

void QDeclarativePosition::setPosition(const QGeoPositionInfo &info)
{
    // timestamp
    const QDateTime pTimestamp = m_info.timestamp();
    const QDateTime timestamp = info.timestamp();
    const bool timestampChanged = pTimestamp != timestamp;

    // coordinate
    const QGeoCoordinate pCoordinate = m_info.coordinate();
    const QGeoCoordinate coordinate = info.coordinate();
    const bool coordinateChanged = pCoordinate != coordinate;
    const bool latitudeValidChanged = exclusiveNaN(pCoordinate.latitude(), coordinate.latitude());
    const bool longitudeValidChanged =
            exclusiveNaN(pCoordinate.longitude(), coordinate.longitude());
    const bool altitudeValidChanged = exclusiveNaN(pCoordinate.altitude(), coordinate.altitude());

    // direction
    const qreal pDirection = m_info.attribute(QGeoPositionInfo::Direction);
    const qreal direction = info.attribute(QGeoPositionInfo::Direction);
    const bool directionChanged = !equalOrNaN(pDirection, direction);
    const bool directionValidChanged = exclusiveNaN(pDirection, direction);

    // ground speed
    const qreal pSpeed = m_info.attribute(QGeoPositionInfo::GroundSpeed);
    const qreal speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    const bool speedChanged = !equalOrNaN(pSpeed, speed);
    const bool speedValidChanged = exclusiveNaN(pSpeed, speed);

    // vertical speed
    const qreal pVerticalSpeed = m_info.attribute(QGeoPositionInfo::VerticalSpeed);
    const qreal verticalSpeed = info.attribute(QGeoPositionInfo::VerticalSpeed);
    const bool verticalSpeedChanged = !equalOrNaN(pVerticalSpeed, verticalSpeed);
    const bool verticalSpeedValidChanged = exclusiveNaN(pVerticalSpeed, verticalSpeed);

    // magnetic variation
    const qreal pMagneticVariation = m_info.attribute(QGeoPositionInfo::MagneticVariation);
    const qreal magneticVariation = info.attribute(QGeoPositionInfo::MagneticVariation);
    const bool magneticVariationChanged = !equalOrNaN(pMagneticVariation, magneticVariation);
    const bool magneticVariationValidChanged = exclusiveNaN(pMagneticVariation, magneticVariation);

    // horizontal accuracy
    const qreal pHorizontalAccuracy = m_info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    const qreal horizontalAccuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    const bool horizontalAccuracyChanged = !equalOrNaN(pHorizontalAccuracy, horizontalAccuracy);
    const bool horizontalAccuracyValidChanged =
            exclusiveNaN(pHorizontalAccuracy, horizontalAccuracy);

    // vertical accuracy
    const qreal pVerticalAccuracy = m_info.attribute(QGeoPositionInfo::VerticalAccuracy);
    const qreal verticalAccuracy = info.attribute(QGeoPositionInfo::VerticalAccuracy);
    const bool verticalAccuracyChanged = !equalOrNaN(pVerticalAccuracy, verticalAccuracy);
    const bool verticalAccuracyValidChanged = exclusiveNaN(pVerticalAccuracy, verticalAccuracy);

    // direction accuracy
    const qreal pDirectionAccuracy = m_info.attribute(QGeoPositionInfo::DirectionAccuracy);
    const qreal directionAccuracy = info.attribute(QGeoPositionInfo::DirectionAccuracy);
    const bool directionAccuracyChanged = !equalOrNaN(pDirectionAccuracy, directionAccuracy);
    const bool directionAccuracyValidChanged = exclusiveNaN(pDirectionAccuracy, directionAccuracy);

    m_info = info;

    if (timestampChanged)
        m_computedTimestamp.notify();

    if (coordinateChanged)
        m_computedCoordinate.notify();
    if (latitudeValidChanged)
        m_computedLatitudeValid.notify();
    if (longitudeValidChanged)
        m_computedLongitudeValid.notify();
    if (altitudeValidChanged)
        m_computedAltitudeValid.notify();

    if (directionChanged)
        m_computedDirection.notify();
    if (directionValidChanged)
        m_computedDirectionValid.notify();

    if (speedChanged)
        m_computedSpeed.notify();
    if (speedValidChanged)
        m_computedSpeedValid.notify();

    if (verticalSpeedChanged)
        m_computedVerticalSpeed.notify();
    if (verticalSpeedValidChanged)
        m_computedVerticalSpeedValid.notify();

    if (horizontalAccuracyChanged)
        m_computedHorizontalAccuracy.notify();
    if (horizontalAccuracyValidChanged)
        m_computedHorizontalAccuracyValid.notify();

    if (verticalAccuracyChanged)
        m_computedVerticalAccuracy.notify();
    if (verticalAccuracyValidChanged)
        m_computedVerticalAccuracyValid.notify();

    if (magneticVariationChanged)
        m_computedMagneticVariation.notify();
    if (magneticVariationValidChanged)
        m_computedMagneticVariationValid.notify();

    if (directionAccuracyChanged)
        m_computedDirectionAccuracy.notify();
    if (directionAccuracyValidChanged)
        m_computedDirectionAccuracyValid.notify();
}

const QGeoPositionInfo &QDeclarativePosition::position() const
{
    return m_info;
}

QBindable<bool> QDeclarativePosition::bindableLatitudeValid() const
{
    return QBindable<bool>(&m_computedLatitudeValid);
}

QBindable<bool> QDeclarativePosition::bindableLongitudeValid() const
{
    return QBindable<bool>(&m_computedLongitudeValid);
}

QBindable<bool> QDeclarativePosition::bindableAltitudeValid() const
{
    return QBindable<bool>(&m_computedAltitudeValid);
}

QBindable<QGeoCoordinate> QDeclarativePosition::bindableCoordinate() const
{
    return QBindable<QGeoCoordinate>(&m_computedCoordinate);
}

QBindable<QDateTime> QDeclarativePosition::bindableTimestamp() const
{
    return QBindable<QDateTime>(&m_computedTimestamp);
}

QBindable<double> QDeclarativePosition::bindableSpeed() const
{
    return QBindable<double>(&m_computedSpeed);
}

QBindable<bool> QDeclarativePosition::bindableSpeedValid() const
{
    return QBindable<bool>(&m_computedSpeedValid);
}

QBindable<qreal> QDeclarativePosition::bindableHorizontalAccuracy() const
{
    return QBindable<qreal>(&m_computedHorizontalAccuracy);
}

QBindable<qreal> QDeclarativePosition::binableVerticalAccuracy() const
{
    return QBindable<qreal>(&m_computedVerticalAccuracy);
}

QBindable<bool> QDeclarativePosition::bindableHorizontalAccuracyValid() const
{
    return QBindable<bool>(&m_computedHorizontalAccuracyValid);
}

QBindable<bool> QDeclarativePosition::bindableVerticalAccuracyValid() const
{
    return QBindable<bool>(&m_computedVerticalAccuracyValid);
}

QBindable<bool> QDeclarativePosition::bindableDirectionValid() const
{
    return QBindable<bool>(&m_computedDirectionValid);
}

QBindable<double> QDeclarativePosition::bindableDirection() const
{
    return QBindable<double>(&m_computedDirection);
}

QBindable<bool> QDeclarativePosition::bindableVerticalSpeedValid() const
{
    return QBindable<bool>(&m_computedVerticalSpeedValid);
}

QBindable<double> QDeclarativePosition::bindableVerticalSpeed() const
{
    return QBindable<double>(&m_computedVerticalSpeed);
}

QBindable<double> QDeclarativePosition::bindableMagneticVariation() const
{
    return QBindable<double>(&m_computedMagneticVariation);
}

QBindable<bool> QDeclarativePosition::bindableMagneticVariationValid() const
{
    return QBindable<bool>(&m_computedMagneticVariationValid);
}

QBindable<double> QDeclarativePosition::bindableDirectionAccuracy() const
{
    return QBindable<double>(&m_computedDirectionAccuracy);
}

QBindable<bool> QDeclarativePosition::bindableDirectionAccuracyValid() const
{
    return QBindable<bool>(&m_computedDirectionAccuracyValid);
}

/*!
    \qmlproperty coordinate Position::coordinate

    This property holds the latitude, longitude, and altitude value of the Position.

    It is a read-only property.

    \sa longitudeValid, latitudeValid, altitudeValid
*/
QGeoCoordinate QDeclarativePosition::coordinate() const
{
    return m_computedCoordinate.value();
}

QGeoCoordinate QDeclarativePosition::coordinateActualCalculation() const
{
    return m_info.coordinate();
}

/*!
    \qmlproperty bool Position::latitudeValid

    This property is true if coordinate's latitude has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa coordinate
*/
bool QDeclarativePosition::isLatitudeValid() const
{
    return m_computedLatitudeValid.value();
}

bool QDeclarativePosition::isLatitudeValidActualCalculation() const
{
    return !qIsNaN(m_info.coordinate().latitude());
}

/*!
    \qmlproperty bool Position::longitudeValid

    This property is true if coordinate's longitude has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa coordinate
*/
bool QDeclarativePosition::isLongitudeValid() const
{
    return m_computedLongitudeValid.value();
}

bool QDeclarativePosition::isLongitudeValidActualCalculation() const
{
    return !qIsNaN(m_info.coordinate().longitude());
}

/*!
    \qmlproperty bool Position::speedValid

    This property is true if \l speed has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa speed
*/
bool QDeclarativePosition::isSpeedValid() const
{
    return m_computedSpeedValid.value();
}

bool QDeclarativePosition::isSpeedValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::GroundSpeed));
}

/*!
    \qmlproperty bool Position::altitudeValid

    This property is true if coordinate's altitude has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa coordinate
*/
bool QDeclarativePosition::isAltitudeValid() const
{
    return m_computedAltitudeValid.value();
}

bool QDeclarativePosition::isAltitudeValidActualCalculation() const
{
    return !qIsNaN(m_info.coordinate().altitude());
}

/*!
    \qmlproperty double Position::speed

    This property holds the value of speed (groundspeed, meters / second).

    It is a read-only property.

    \sa speedValid, coordinate
*/
double QDeclarativePosition::speed() const
{
    return m_computedSpeed.value();
}

double QDeclarativePosition::speedActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::GroundSpeed);
}

/*!
    \qmlproperty real Position::horizontalAccuracy

    This property holds the horizontal accuracy of the coordinate (in meters).

    \sa horizontalAccuracyValid, coordinate
*/
qreal QDeclarativePosition::horizontalAccuracy() const
{
    return m_computedHorizontalAccuracy.value();
}

qreal QDeclarativePosition::horizontalAccuracyActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::HorizontalAccuracy);
}

/*!
    \qmlproperty bool Position::horizontalAccuracyValid

    This property is true if \l horizontalAccuracy has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa horizontalAccuracy
*/
bool QDeclarativePosition::isHorizontalAccuracyValid() const
{
    return m_computedHorizontalAccuracyValid.value();
}

bool QDeclarativePosition::isHorizontalAccuracyValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::HorizontalAccuracy));
}

/*!
    \qmlproperty real Position::verticalAccuracy

    This property holds the vertical accuracy of the coordinate (in meters).

    \sa verticalAccuracyValid, coordinate
*/
qreal QDeclarativePosition::verticalAccuracy() const
{
    return m_computedVerticalAccuracy.value();
}

qreal QDeclarativePosition::verticalAccuracyActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::VerticalAccuracy);
}

/*!
    \qmlproperty bool Position::verticalAccuracyValid

    This property is true if \l verticalAccuracy has been set
    (to indicate whether that data has been received or not, as every update
    does not necessarily contain all data).

    \sa verticalAccuracy
*/
bool QDeclarativePosition::isVerticalAccuracyValid() const
{
    return m_computedVerticalAccuracyValid.value();
}

bool QDeclarativePosition::isVerticalAccuracyValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::VerticalAccuracy));
}

/*!
    \qmlproperty date Position::timestamp

    This property holds the timestamp when this position
    was received. If the property has not been set, it is invalid.

    It is a read-only property.
*/
QDateTime QDeclarativePosition::timestamp() const
{
    return m_computedTimestamp.value();
}

QDateTime QDeclarativePosition::timestampActualCalculation() const
{
    return m_info.timestamp();
}

/*!
    \qmlproperty bool Position::directionValid
    \since Qt Positioning 5.3

    This property is true if \l direction has been set (to indicate whether that data has been
    received or not, as every update does not necessarily contain all data).

    \sa direction
*/
bool QDeclarativePosition::isDirectionValid() const
{
    return m_computedDirectionValid.value();
}

bool QDeclarativePosition::isDirectionValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::Direction));
}

/*!
    \qmlproperty double Position::direction
    \since Qt Positioning 5.3

    This property holds the value of the direction of travel in degrees from true north.

    It is a read-only property.

    \sa directionValid
*/
double QDeclarativePosition::direction() const
{
    return m_computedDirection.value();
}

double QDeclarativePosition::directionActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::Direction);
}

/*!
    \qmlproperty bool Position::verticalSpeedValid
    \since Qt Positioning 5.3

    This property is true if \l verticalSpeed has been set (to indicate whether that data has been
    received or not, as every update does not necessarily contain all data).

    \sa verticalSpeed
*/
bool QDeclarativePosition::isVerticalSpeedValid() const
{
    return m_computedVerticalSpeedValid.value();
}

bool QDeclarativePosition::isVerticalSpeedValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::VerticalSpeed));
}

/*!
    \qmlproperty double Position::verticalSpeed
    \since Qt Positioning 5.3

    This property holds the value of the vertical speed in meters per second.

    It is a read-only property.

    \sa verticalSpeedValid
*/
double QDeclarativePosition::verticalSpeed() const
{
    return m_computedVerticalSpeed.value();
}

double QDeclarativePosition::verticalSpeedActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::VerticalSpeed);
}

/*!
    \qmlproperty bool Position::magneticVariationValid
    \since Qt Positioning 5.4

    This property is true if \l magneticVariation has been set (to indicate whether that data has been
    received or not, as every update does not necessarily contain all data).

    \sa magneticVariation
*/
bool QDeclarativePosition::isMagneticVariationValid() const
{
    return m_computedMagneticVariationValid.value();
}

bool QDeclarativePosition::isMagneticVariationValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::MagneticVariation));
}

/*!
    \qmlproperty double Position::magneticVariation
    \since Qt Positioning 5.4

    This property holds the angle between the horizontal component of the
    magnetic field and true north, in degrees. Also known as magnetic
    declination. A positive value indicates a clockwise direction from
    true north and a negative value indicates a counter-clockwise direction.

    It is a read-only property.

    \sa magneticVariationValid
*/
double QDeclarativePosition::magneticVariation() const
{
    return m_computedMagneticVariation.value();
}

double QDeclarativePosition::magneticVariationActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::MagneticVariation);
}

/*!
    \qmlproperty bool Position::directionAccuracyValid
    \since Qt Positioning 6.3

    This property is \c true if \l directionAccuracy has been set.

    \sa directionAccuracy
*/
bool QDeclarativePosition::isDirectionAccuracyValid() const
{
    return m_computedDirectionAccuracyValid.value();
}

bool QDeclarativePosition::isDirectionAccuracyValidActualCalculation() const
{
    return !qIsNaN(m_info.attribute(QGeoPositionInfo::DirectionAccuracy));
}

/*!
    \qmlproperty double Position::directionAccuracy
    \since Qt Positioning 6.3

    This property holds the accuracy of the provided \l direction in degrees.
    This property is valid for Android and macOS/iOS only. See
    \l {QGeoPositionInfo::Attribute} documentation for more details.

    \sa direction, directionAccuracyValid
*/
double QDeclarativePosition::directionAccuracy() const
{
    return m_computedDirectionAccuracy.value();
}

double QDeclarativePosition::directionAccuracyActualCalculation() const
{
    return m_info.attribute(QGeoPositionInfo::DirectionAccuracy);
}

QT_END_NAMESPACE

#include "moc_qdeclarativeposition_p.cpp"
