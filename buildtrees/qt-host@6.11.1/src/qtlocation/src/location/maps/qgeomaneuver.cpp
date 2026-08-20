// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomaneuver.h"
#include "qgeomaneuver_p.h"

#include <QtCore/QVariant>
#include "qgeocoordinate.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QSDP_SPECIALIZATION_DTOR(QGeoManeuverPrivate)

/*!
    \class QGeoManeuver
    \inmodule QtLocation
    \ingroup QtLocation-routing
    \since 5.6

    \brief The QGeoManeuver class represents the information relevant to the
    point at which two QGeoRouteSegments meet.

    QGeoRouteSegment instances can be thought of as edges on a routing
    graph, with QGeoManeuver instances as optional labels attached to the
    vertices of the graph.

    The most interesting information help in a QGeoManeuver instance is
    normally the textual navigation to provide and the position at which to
    provide it, accessible by instructionText() and position() respectively.

    It is also possible to determine if a routing waypoint has been passed by
    checking if waypoint() returns a valid QGeoCoordinate.
*/

/*!
    \qmltype routeManeuver
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-routing
    \since QtLocation 5.5

    \brief The routeManeuver type represents the information relevant to the
    point at which two routeSegments meet.

    Instances of routeSegment can be thought of as edges on a routing
    graph, with routeManeuver instances as optional labels attached to the
    vertices of the graph.

    The most interesting information held in a routeManeuver instance is
    normally the textual navigation to provide and the position at which to
    provide it, accessible by \l instructionText and \l position respectively.

    \section1 Example

    The following QML snippet demonstrates how to print information about a
    route maneuver:

    \snippet declarative/routing.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/routing.qml routeManeuver
*/

/*!
\enum QGeoManeuver::InstructionDirection

Describes the change in direction associated with the instruction text
that is associated with a QGeoManaeuver.

\value NoDirection
There is no direction associated with the instruction text.

\value DirectionForward
The instruction indicates that the direction of travel does not need to change.

\value DirectionBearRight
The instruction indicates that the direction of travel should bear to the right.

\value DirectionLightRight
The instruction indicates that a light turn to the right is required.

\value DirectionRight
The instruction indicates that a turn to the right is required.

\value DirectionHardRight
The instruction indicates that a hard turn to the right is required.

\value DirectionUTurnRight
The instruction indicates that a u-turn to the right is required.

\value DirectionUTurnLeft
The instruction indicates that a u-turn to the left is required.

\value DirectionHardLeft
The instruction indicates that a hard turn to the left is required.

\value DirectionLeft
The instruction indicates that a turn to the left is required.

\value DirectionLightLeft
The instruction indicates that a light turn to the left is required.

\value DirectionBearLeft
The instruction indicates that the direction of travel should bear to the left.

*/

/*!
    Constructs a invalid maneuver object.

    The maneuver will remain invalid until one of
    setPosition(), setInstructionText(), setDirection(),
    setTimeToNextInstruction(), setDistanceToNextInstruction() or
    setWaypoint() is called.
*/
QGeoManeuver::QGeoManeuver()
    : d_ptr(new QGeoManeuverPrivate()) {}

/*!
    Constructs a maneuver object from the contents of \a other.
*/
QGeoManeuver::QGeoManeuver(const QGeoManeuver &other) noexcept = default;

/*!
    Destroys this maneuver object.
*/
QGeoManeuver::~QGeoManeuver() = default;

/*!
    Assigns \a other to this maneuver object and then returns
    a reference to this maneuver object.
*/
QGeoManeuver &QGeoManeuver::operator= (const QGeoManeuver & other)
{
    if (this == &other)
        return *this;

    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \fn bool QGeoManeuver::operator==(const QGeoManeuver &lhs, const QGeoManeuver &rhs) noexcept

    Returns whether the \a lhs maneuver is equal to \a rhs.
*/

/*!
    \fn bool QGeoManeuver::operator!= (const QGeoManeuver &lhs, const QGeoManeuver &rhs) noexcept

    Returns whether the \a lhs maneuver is not equal to \a rhs.
*/

bool QGeoManeuver::isEqual(const QGeoManeuver &other) const
{
    return ( (d_ptr.constData() == other.d_ptr.constData())
            ||  (d_ptr->equals(*other.d_ptr)) );
}

/*!
    \qmlproperty bool routeManeuver::valid

    This read-only property holds whether this maneuver is valid or not.

    Invalid maneuvers are used when there is no information
    that needs to be attached to the endpoint of a QGeoRouteSegment instance.
*/

/*!
    \property QGeoManeuver::valid
    \brief whether this maneuver is valid or not.

    Invalid maneuvers are used when there is no information
    that needs to be attached to the endpoint of a QGeoRouteSegment instance.
*/
bool QGeoManeuver::isValid() const
{
    return d_ptr->valid();
}

/*!
    \qmlproperty coordinate routeManeuver::position

    This read-only property holds where the \l instructionText should be displayed.

*/

/*!
    \property QGeoManeuver::position
    \brief the position where \l instructionText should be displayed.
*/
void QGeoManeuver::setPosition(const QGeoCoordinate &position)
{
    d_ptr->setValid(true);
    d_ptr->setPosition(position);
}

QGeoCoordinate QGeoManeuver::position() const
{
    return d_ptr->position();
}

/*!
    \qmlproperty string routeManeuver::instructionText

    This read-only property holds textual navigation instruction.
*/

/*!
    \property QGeoManeuver::instructionText
    \brief the textual navigation instructions.
*/
void QGeoManeuver::setInstructionText(const QString &instructionText)
{
    d_ptr->setValid(true);
    d_ptr->setText(instructionText);
}

QString QGeoManeuver::instructionText() const
{
    return d_ptr->text();
}

/*!
    \qmlproperty enumeration routeManeuver::direction

    Describes the change in direction associated with the instruction text
    that is associated with a routeManeuver.

    \list
    \li RouteManeuver.NoDirection - There is no direction associated with the instruction text
    \li RouteManeuver.DirectionForward - The instruction indicates that the direction of travel does not need to change
    \li RouteManeuver.DirectionBearRight - The instruction indicates that the direction of travel should bear to the right
    \li RouteManeuver.DirectionLightRight - The instruction indicates that a light turn to the right is required
    \li RouteManeuver.DirectionRight - The instruction indicates that a turn to the right is required
    \li RouteManeuver.DirectionHardRight - The instruction indicates that a hard turn to the right is required
    \li RouteManeuver.DirectionUTurnRight - The instruction indicates that a u-turn to the right is required
    \li RouteManeuver.DirectionUTurnLeft - The instruction indicates that a u-turn to the left is required
    \li RouteManeuver.DirectionHardLeft - The instruction indicates that a hard turn to the left is required
    \li RouteManeuver.DirectionLeft - The instruction indicates that a turn to the left is required
    \li RouteManeuver.DirectionLightLeft - The instruction indicates that a light turn to the left is required
    \li RouteManeuver.DirectionBearLeft - The instruction indicates that the direction of travel should bear to the left
    \endlist
*/

/*!
    \property QGeoManeuver::direction
    \brief the direction associated with the associated instruction.
*/
void QGeoManeuver::setDirection(QGeoManeuver::InstructionDirection direction)
{
    d_ptr->setValid(true);
    d_ptr->setDirection(direction);
}

QGeoManeuver::InstructionDirection QGeoManeuver::direction() const
{
    return d_ptr->direction();
}

/*!
    \qmlproperty int routeManeuver::timeToNextInstruction

    This read-only property holds the estimated time, in seconds, that it will take
    to travel from the point at which the associated instruction was issued
    to the point at which the next instruction should be issued, in seconds.
*/

/*!
    \property QGeoManeuver::timeToNextInstruction
    \brief the estimated time, in seconds, that it will take to travel from the
    point at which the associated instruction was issued to the point at which the
    next instruction should be issued.
*/
void QGeoManeuver::setTimeToNextInstruction(int secs)
{
    d_ptr->setValid(true);
    d_ptr->setTimeToNextInstruction(secs);
}

int QGeoManeuver::timeToNextInstruction() const
{
    return d_ptr->timeToNextInstruction();
}

/*!
    \qmlproperty real routeManeuver::distanceToNextInstruction

    This read-only property holds the distance, in meters, between the point at which
    the associated instruction was issued and the point that the next instruction should
    be issued.
*/

/*!
    \property QGeoManeuver::distanceToNextInstruction
    \brief the distance, in meters, between the point at which this instruction was
    issued, and the point at which the next instruction should be issued.
*/
void QGeoManeuver::setDistanceToNextInstruction(qreal distance)
{
    d_ptr->setValid(true);
    d_ptr->setDistanceToNextInstruction(distance);
}

qreal QGeoManeuver::distanceToNextInstruction() const
{
    return d_ptr->distanceToNextInstruction();
}

/*!
    \qmlproperty coordinate routeManeuver::waypoint

    This property holds the waypoint associated with this maneuver.
    Not all maneuvers have a waypoint associated with them.
*/

/*!
    \property QGeoManeuver::waypoint
    \brief the waypoint associated with this maneuver.

    If there is not waypoint associated with this maneuver, then this
    property holds an invalid QGeoCoordinate.
*/
void QGeoManeuver::setWaypoint(const QGeoCoordinate &coordinate)
{
    d_ptr->setValid(true);
    d_ptr->setWaypoint(coordinate);
}

QGeoCoordinate QGeoManeuver::waypoint() const
{
    return d_ptr->waypoint();
}

/*!
    \qmlproperty Object routeManeuver::extendedAttributes

    This property holds the extended attributes of the maneuver and is a map.
    These attributes are plugin specific, and can be empty.

    Consult the \l {Qt Location#Plugin References and Parameters}{plugin documentation}
    for what attributes are supported and how they should be used.

    Note, due to limitations of the QQmlPropertyMap, it is not possible
    to declaratively specify the attributes in QML, assignment of attributes keys
    and values can only be accomplished by JavaScript.

    \since QtLocation 5.11
*/

/*!
    \property QGeoManeuver::extendedAttributes
    \brief the extended attributes associated with this maneuver.
    \since QtLocation 5.11
*/
void QGeoManeuver::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    d_ptr->setValid(true);
    d_ptr->setExtendedAttributes(extendedAttributes);
}

QVariantMap QGeoManeuver::extendedAttributes() const
{
    return d_ptr->extendedAttributes();
}

/*******************************************************************************
*******************************************************************************/

bool QGeoManeuverPrivate::equals(const QGeoManeuverPrivate &other) const
{
    return ((valid() == other.valid())
            && (position() == other.position())
            && (text() == other.text())
            && (direction() == other.direction())
            && (timeToNextInstruction() == other.timeToNextInstruction())
            && (distanceToNextInstruction() == other.distanceToNextInstruction())
            && (waypoint() == other.waypoint()));
}

/*******************************************************************************
*******************************************************************************/

bool QGeoManeuverPrivate::valid() const
{
    return m_valid;
}

void QGeoManeuverPrivate::setValid(bool valid)
{
    m_valid = valid;
}

QString QGeoManeuverPrivate::id() const
{
    return m_id;
}

void QGeoManeuverPrivate::setId(const QString &id)
{
    m_id = id;
}

QGeoCoordinate QGeoManeuverPrivate::position() const
{
    return m_position;
}

void QGeoManeuverPrivate::setPosition(const QGeoCoordinate &position)
{
    m_position = position;
}

QString QGeoManeuverPrivate::text() const
{
    return m_text;
}

void QGeoManeuverPrivate::setText(const QString &text)
{
    m_text = text;
}

QGeoManeuver::InstructionDirection QGeoManeuverPrivate::direction() const
{
    return m_direction;
}

void QGeoManeuverPrivate::setDirection(QGeoManeuver::InstructionDirection direction)
{
    m_direction = direction;
}

int QGeoManeuverPrivate::timeToNextInstruction() const
{
    return m_timeToNextInstruction;
}

void QGeoManeuverPrivate::setTimeToNextInstruction(int timeToNextInstruction)
{
    m_timeToNextInstruction = timeToNextInstruction;
}

qreal QGeoManeuverPrivate::distanceToNextInstruction() const
{
    return m_distanceToNextInstruction;
}

void QGeoManeuverPrivate::setDistanceToNextInstruction(qreal distanceToNextInstruction)
{
    m_distanceToNextInstruction = distanceToNextInstruction;
}

QGeoCoordinate QGeoManeuverPrivate::waypoint() const
{
    return m_waypoint;
}

void QGeoManeuverPrivate::setWaypoint(const QGeoCoordinate &waypoint)
{
    m_waypoint = waypoint;
}

QVariantMap QGeoManeuverPrivate::extendedAttributes() const
{
    return m_extendedAttributes;
}

void QGeoManeuverPrivate::setExtendedAttributes(const QVariantMap &extendedAttributes)
{
    m_extendedAttributes = extendedAttributes;
}

QT_END_NAMESPACE

#include "moc_qgeomaneuver.cpp"
