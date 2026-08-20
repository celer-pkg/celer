// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMANEUVER_H
#define QGEOMANEUVER_H

#include <QtCore/QObject>
#include <QtCore/qshareddata.h>
#include <QtLocation/qlocationglobal.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QString;
class QGeoCoordinate;
class QGeoManeuverPrivate;
QT_DECLARE_QSDP_SPECIALIZATION_DTOR_WITH_EXPORT(QGeoManeuverPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QGeoManeuver
{
    Q_GADGET
    QML_VALUE_TYPE(routeManeuver)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(QGeoCoordinate position READ position CONSTANT)
    Q_PROPERTY(QString instructionText READ instructionText CONSTANT)
    Q_PROPERTY(InstructionDirection direction READ direction CONSTANT)
    Q_PROPERTY(int timeToNextInstruction READ timeToNextInstruction CONSTANT)
    Q_PROPERTY(qreal distanceToNextInstruction READ distanceToNextInstruction CONSTANT)
    Q_PROPERTY(QGeoCoordinate waypoint READ waypoint CONSTANT)
    Q_PROPERTY(QVariantMap extendedAttributes READ extendedAttributes CONSTANT)

public:
    enum InstructionDirection {
        NoDirection,
        DirectionForward,
        DirectionBearRight,
        DirectionLightRight,
        DirectionRight,
        DirectionHardRight,
        DirectionUTurnRight,
        DirectionUTurnLeft,
        DirectionHardLeft,
        DirectionLeft,
        DirectionLightLeft,
        DirectionBearLeft
    };
    Q_ENUM(InstructionDirection)

    QGeoManeuver();
    QGeoManeuver(const QGeoManeuver &other) noexcept;
    QGeoManeuver(QGeoManeuver &&other) noexcept = default;
    ~QGeoManeuver();

    QGeoManeuver &operator= (const QGeoManeuver &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QGeoManeuver)

    void swap(QGeoManeuver &other) noexcept { d_ptr.swap(other.d_ptr); }

    friend inline bool operator==(const QGeoManeuver &lhs, const QGeoManeuver &rhs) noexcept
    { return lhs.isEqual(rhs); }
    friend inline bool operator!=(const QGeoManeuver &lhs, const QGeoManeuver &rhs) noexcept
    { return !lhs.isEqual(rhs); }

    bool isValid() const;

    void setPosition(const QGeoCoordinate &position);
    QGeoCoordinate position() const;

    void setInstructionText(const QString &instructionText);
    QString instructionText() const;

    void setDirection(InstructionDirection direction);
    InstructionDirection direction() const;

    void setTimeToNextInstruction(int secs);
    int timeToNextInstruction() const;

    void setDistanceToNextInstruction(qreal distance);
    qreal distanceToNextInstruction() const;

    void setWaypoint(const QGeoCoordinate &coordinate);
    QGeoCoordinate waypoint() const;

    void setExtendedAttributes(const QVariantMap &extendedAttributes);
    QVariantMap extendedAttributes() const;

private:
    QSharedDataPointer<QGeoManeuverPrivate> d_ptr;

    bool isEqual(const QGeoManeuver &other) const;
};

Q_DECLARE_SHARED(QGeoManeuver)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGeoManeuver)
Q_DECLARE_METATYPE(QGeoManeuver::InstructionDirection)

#endif
