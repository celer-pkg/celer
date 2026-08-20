// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMANEUVER_P_H
#define QGEOMANEUVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/qgeomaneuver.h>
#include <QtPositioning/qgeocoordinate.h>

#include <QSharedData>
#include <QString>

QT_BEGIN_NAMESPACE

class QGeoManeuverPrivate : public QSharedData
{
public:
    bool equals(const QGeoManeuverPrivate &other) const;

    bool valid() const;
    void setValid(bool valid);

    QString id() const;
    void setId(const QString &id);

    QGeoCoordinate position() const;
    void setPosition(const QGeoCoordinate &position);

    QString text() const;
    void setText(const QString &text);

    QGeoManeuver::InstructionDirection direction() const;
    void setDirection(QGeoManeuver::InstructionDirection direction);

    int timeToNextInstruction() const;
    void setTimeToNextInstruction(int timeToNextInstruction);

    qreal distanceToNextInstruction() const;
    void setDistanceToNextInstruction(qreal distanceToNextInstruction);

    QGeoCoordinate waypoint() const;
    void setWaypoint(const QGeoCoordinate &waypoint);

    QVariantMap extendedAttributes() const;
    void setExtendedAttributes(const QVariantMap &extendedAttributes);

    QString m_id;
    QGeoCoordinate m_position;
    QString m_text;
    QGeoCoordinate m_waypoint;
    QVariantMap m_extendedAttributes;
    qreal m_distanceToNextInstruction = 0.0;
    QGeoManeuver::InstructionDirection m_direction = QGeoManeuver::NoDirection;
    int m_timeToNextInstruction = 0;
    bool m_valid = false;
};

QT_END_NAMESPACE

#endif
