// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qgeopositioninfo.h"
#include "private/qgeopositioninfo_p.h"
#include <QHash>
#include <QDebug>
#include <QDataStream>
#include <QtCore/QtNumeric>

#include <algorithm>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(QGeoPositionInfo)

/*!
    \class QGeoPositionInfo
    \inmodule QtPositioning
    \ingroup QtPositioning-positioning
    \ingroup shared
    \since 5.2

    \brief The QGeoPositionInfo class contains information gathered on a global position, direction and velocity at a particular point in time.

    A QGeoPositionInfo contains, at a minimum, a geographical coordinate and
    a timestamp. It may also have heading and speed measurements as well as
    estimates of the accuracy of the provided data.

    \sa QGeoPositionInfoSource
*/

/*!
    \enum QGeoPositionInfo::Attribute
    Defines the attributes for positional information.

    \value Direction The bearing measured in degrees clockwise from true north to the direction of travel.
    \value GroundSpeed The ground speed, in meters/sec.
    \value VerticalSpeed The vertical speed, in meters/sec.
    \value MagneticVariation The angle between the horizontal component of the magnetic field and true north, in degrees. Also known as magnetic declination. A positive value indicates a clockwise direction from true north and a negative value indicates a counter-clockwise direction.
    \value HorizontalAccuracy The accuracy of the provided latitude-longitude value, in meters.
    \value VerticalAccuracy The accuracy of the provided altitude value, in meters.
    \value DirectionAccuracy The accuracy of the provided bearing, in degrees.
    This attribute is available only on Android (API level 26 or above) and
    macOS/iOS. See corresponding \l {Android getBearingAccuracyDegrees}
    {Android} and \l {iOS courseAccuracy}{Apple} documentation for more details.

    NMEA protocol also suggests another type of accuracy - PositionAccuracy,
    which is a 3D accuracy value. Qt does not provide a separate attribute for
    it. If you need this value, you can calculate it based on the following
    formula:

    \c {PositionAccuracy} \sup 2 \c {= HorizontalAccuracy} \sup 2 \c { +
    VerticalAccuracy} \sup 2
*/

/*!
    Creates an invalid QGeoPositionInfo object.

    \sa isValid()
*/
QGeoPositionInfo::QGeoPositionInfo()
        : d(new QGeoPositionInfoPrivate)
{
}

/*!
    Creates a QGeoPositionInfo for the given \a coordinate and \a timestamp.
*/
QGeoPositionInfo::QGeoPositionInfo(const QGeoCoordinate &coordinate, const QDateTime &timestamp)
        : d(new QGeoPositionInfoPrivate)
{
    d->timestamp = timestamp;
    d->coord = coordinate;
}

/*!
    Creates a QGeoPositionInfo with the values of \a other.
*/
QGeoPositionInfo::QGeoPositionInfo(const QGeoPositionInfo &other)
        : d(other.d)
{
}

/*!
    \fn QGeoPositionInfo::QGeoPositionInfo(QGeoPositionInfo &&other) noexcept
    \since 6.2

    Creates a QGeoPositionInfo object by moving from \a other.

    Note that a moved-from QGeoPositionInfo can only be destroyed or
    assigned to. The effect of calling other functions than the destructor
    or one of the assignment operators is undefined.
*/

QGeoPositionInfo::QGeoPositionInfo(QGeoPositionInfoPrivate &dd) : d(&dd)
{
}

/*!
    Destroys a QGeoPositionInfo object.
*/
QGeoPositionInfo::~QGeoPositionInfo()
{
}

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QGeoPositionInfoPrivate)

/*!
    Assigns the values from \a other to this QGeoPositionInfo.
*/
QGeoPositionInfo &QGeoPositionInfo::operator=(const QGeoPositionInfo & other)
{
    if (this == &other)
        return *this;

    d = other.d;
    return *this;
}

/*!
    \fn QGeoPositionInfo &QGeoPositionInfo::operator=(QGeoPositionInfo &&other) noexcept
    \since 6.2

    Move-assigns the values from \a other to this object.

    Note that a moved-from QGeoPositionInfo can only be destroyed or
    assigned to. The effect of calling other functions than the destructor
    or one of the assignment operators is undefined.
*/

/*!
    \fn bool QGeoPositionInfo::operator==(const QGeoPositionInfo &lhs, const QGeoPositionInfo &rhs)

    Returns \c true if all of the \a lhs object's values are the same as those
    of \a rhs. Otherwise returns \c false.
*/

/*!
    \fn bool QGeoPositionInfo::operator!=(const QGeoPositionInfo &lhs, const QGeoPositionInfo &rhs)

    Returns \c true if any of the \a lhs object's values are not the same as
    those of \a rhs. Otherwise returns \c false.
*/

/*!
    \fn void QGeoPositionInfo::swap(QGeoPositionInfo &other)
    \since 6.2
    \memberswap{position information}
*/

/*!
    Returns true if the timestamp() and coordinate() values are both valid.

    \sa QGeoCoordinate::isValid(), QDateTime::isValid()
*/
bool QGeoPositionInfo::isValid() const
{
    return d->timestamp.isValid() && d->coord.isValid();
}

/*!
    Sets the date and time at which this position was reported to \a timestamp.

    The \a timestamp must be in UTC time.

    \sa timestamp()
*/
void QGeoPositionInfo::setTimestamp(const QDateTime &timestamp)
{
    d.detach();
    d->timestamp = timestamp;
}

/*!
    Returns the date and time at which this position was reported, in UTC time.

    Returns an invalid QDateTime if no date/time value has been set.

    \sa setTimestamp()
*/
QDateTime QGeoPositionInfo::timestamp() const
{
    return d->timestamp;
}

/*!
    Sets the coordinate for this position to \a coordinate.

    \sa coordinate()
*/
void QGeoPositionInfo::setCoordinate(const QGeoCoordinate &coordinate)
{
    d.detach();
    d->coord = coordinate;
}

/*!
    Returns the coordinate for this position.

    Returns an invalid coordinate if no coordinate has been set.

    \sa setCoordinate()
*/
QGeoCoordinate QGeoPositionInfo::coordinate() const
{
    return d->coord;
}

/*!
    Sets the value for \a attribute to \a value.

    \sa attribute()
*/
void QGeoPositionInfo::setAttribute(Attribute attribute, qreal value)
{
    d.detach();
    d->doubleAttribs[attribute] = value;
}

/*!
    Returns the value of the specified \a attribute as a qreal value.

    Returns NaN if the value has not been set.

    The function hasAttribute() should be used to determine whether or
    not a value has been set for an attribute.

    \sa hasAttribute(), setAttribute()
*/
qreal QGeoPositionInfo::attribute(Attribute attribute) const
{
    return d->doubleAttribs.value(attribute, qQNaN());
}

/*!
    Removes the specified \a attribute and its value.
*/
void QGeoPositionInfo::removeAttribute(Attribute attribute)
{
    const auto it = d->doubleAttribs.constFind(attribute);
    if (it != d->doubleAttribs.cend()) {
        d.detach();
        d->doubleAttribs.erase(it);
    }
}

/*!
    Returns true if the specified \a attribute is present for this
    QGeoPositionInfo object.
*/
bool QGeoPositionInfo::hasAttribute(Attribute attribute) const
{
    return d->doubleAttribs.contains(attribute);
}

/*!
    \internal
*/
void QGeoPositionInfo::detach()
{
    if (d)
        d.detach();
    else
        d = new QGeoPositionInfoPrivate;
}

bool QGeoPositionInfo::equals(const QGeoPositionInfo &lhs, const QGeoPositionInfo &rhs)
{
    return *lhs.d == *rhs.d;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QGeoPositionInfo::debugStreaming(QDebug dbg, const QGeoPositionInfo &info)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QGeoPositionInfo(" << info.d->timestamp;
    dbg.nospace() << ", "; // timestamp force dbg.space() -> reverting here
    dbg << info.d->coord;

    QList<QGeoPositionInfo::Attribute> attribs = info.d->doubleAttribs.keys();
    std::stable_sort(attribs.begin(), attribs.end()); // Output a sorted list from an unsorted hash.
    for (int i = 0; i < attribs.size(); ++i) {
        dbg << ", ";
        switch (attribs[i]) {
            case QGeoPositionInfo::Direction:
                dbg << "Direction=";
                break;
            case QGeoPositionInfo::GroundSpeed:
                dbg << "GroundSpeed=";
                break;
            case QGeoPositionInfo::VerticalSpeed:
                dbg << "VerticalSpeed=";
                break;
            case QGeoPositionInfo::MagneticVariation:
                dbg << "MagneticVariation=";
                break;
            case QGeoPositionInfo::HorizontalAccuracy:
                dbg << "HorizontalAccuracy=";
                break;
            case QGeoPositionInfo::VerticalAccuracy:
                dbg << "VerticalAccuracy=";
                break;
            case QGeoPositionInfo::DirectionAccuracy:
                dbg << "DirectionAccuracy=";
                break;
        }
        dbg << info.d->doubleAttribs[attribs[i]];
    }
    dbg << ')';
    return dbg;
}
#endif


#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &QGeoPositionInfo::operator<<(QDataStream &stream, QGeoPositionInfo::Attribute attr)

    Writes the given \a attr enumeration to the specified \a stream.

    \sa {Serializing Qt Data Types}
*/
QDataStream &QGeoPositionInfo::dataStreamOut(QDataStream &stream, QGeoPositionInfo::Attribute attr)
{
    return stream << qint32(attr);
}

/*!
    \fn QDataStream &QGeoPositionInfo::operator>>(QDataStream &stream, QGeoPositionInfo::Attribute &attr)

    Reads an attribute enumeration from the specified \a stream info the given \a attr.

    \sa {Serializing Qt Data Types}
*/
QDataStream &QGeoPositionInfo::dataStreamIn(QDataStream &stream, QGeoPositionInfo::Attribute &attr)
{
    qint32 a;
    stream >> a;
    attr = static_cast<QGeoPositionInfo::Attribute>(a);
    return stream;
}

/*!
    \fn QDataStream &QGeoPositionInfo::operator<<(QDataStream &stream, const QGeoPositionInfo &info)

    Writes the given \a info to the specified \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &QGeoPositionInfo::dataStreamOut(QDataStream &stream, const QGeoPositionInfo &info)
{
    stream << info.d->timestamp;
    stream << info.d->coord;
    stream << info.d->doubleAttribs;
    return stream;
}

/*!
    \fn QDataStream &QGeoPositionInfo::operator>>(QDataStream &stream, QGeoPositionInfo &info)

    Reads a coordinate from the specified \a stream into the given
    \a info.

    \sa {Serializing Qt Data Types}
*/

QDataStream &QGeoPositionInfo::dataStreamIn(QDataStream &stream, QGeoPositionInfo &info)
{
    stream >> info.d->timestamp;
    stream >> info.d->coord;
    stream >> info.d->doubleAttribs;
    return stream;
}
#endif

QGeoPositionInfoPrivate::QGeoPositionInfoPrivate() : QSharedData()
{
}

QGeoPositionInfoPrivate::QGeoPositionInfoPrivate(const QGeoPositionInfoPrivate &other)
    : QSharedData(other),
      timestamp(other.timestamp),
      coord(other.coord),
      doubleAttribs(other.doubleAttribs)
{
}

QGeoPositionInfoPrivate::~QGeoPositionInfoPrivate()
{

}

bool QGeoPositionInfoPrivate::operator==(const QGeoPositionInfoPrivate &other) const
{
    return timestamp == other.timestamp
           && coord == other.coord
            && doubleAttribs == other.doubleAttribs;
}

QGeoPositionInfoPrivate *QGeoPositionInfoPrivate::get(const QGeoPositionInfo &info)
{
    return info.d.data();
}

size_t qHash(const QGeoPositionInfo &key, size_t seed) noexcept
{
    return qHashMulti(seed, key.d->coord);
}

namespace QTest
{

char *toString(const QGeoPositionInfo &info)
{
    QString result;
    QDebug dbg(&result);
    dbg << info;

    return qstrdup(qPrintable(result));
}

} // namespace QTest

QT_END_NAMESPACE
