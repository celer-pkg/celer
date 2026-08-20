// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "backend.h"
#include <QTransform>

Backend::Backend(QObject *parent) : QObject(parent)
{
    connect(&m_rotation1Angle, &AnimatedParam::valueChanged, this, &Backend::rot1AngleChanged);
    connect(&m_rotation2Angle, &AnimatedParam::valueChanged, this, &Backend::rot2AngleChanged);
    connect(&m_rotation3Angle, &AnimatedParam::valueChanged, this, &Backend::rot3AngleChanged);
    connect(&m_rotation4Angle, &AnimatedParam::valueChanged, this, &Backend::rot4AngleChanged);
    connect(&m_clawsAngle, &AnimatedParam::valueChanged, this, &Backend::clawsAngleChanged);

    m_status.setBinding([this]() {
        return m_isCollision.value() ? QString("Collision!")
                : m_rotation1Angle.isRunning() || m_rotation2Angle.isRunning() || m_rotation3Angle.isRunning()
                        || m_rotation4Angle.isRunning()
                ? QString("Busy")
                : QString("Ready");
    });

    connect(&m_rotation1Angle, &AnimatedParam::valueChanged, this, &Backend::detectCollision);
    connect(&m_rotation2Angle, &AnimatedParam::valueChanged, this, &Backend::detectCollision);
    connect(&m_rotation3Angle, &AnimatedParam::valueChanged, this, &Backend::detectCollision);
    connect(&m_rotation4Angle, &AnimatedParam::valueChanged, this, &Backend::detectCollision);
}

int Backend::rotation1Angle() const
{
    return m_rotation1Angle.value();
}

void Backend::setRot1Angle(const int angle)
{
    m_rotation1Angle.setValue(angle);
}

int Backend::rotation2Angle() const
{
    return m_rotation2Angle.value();
}

void Backend::setRot2Angle(const int angle)
{
    m_rotation2Angle.setValue(angle);
}

int Backend::rotation3Angle() const
{
    return m_rotation3Angle.value();
}

void Backend::setRot3Angle(const int angle)
{
    m_rotation3Angle.setValue(angle);
}

int Backend::rotation4Angle() const
{
    return m_rotation4Angle.value();
}

void Backend::setRot4Angle(const int angle)
{
    m_rotation4Angle.setValue(angle);
}

int Backend::clawsAngle() const
{
    return m_clawsAngle.value();
}

void Backend::setClawsAngle(const int angle)
{
    m_clawsAngle.setValue(angle);
}

QString Backend::status() const
{
    return m_status;
}

QBindable<QString> Backend::bindableStatus() const
{
    return &m_status;
}

void Backend::detectCollision()
{
    // simple aproximate collision detection, uses hardcoded model dimensions

    QPolygon pol1(QRect(-70, 0, 70, 300));

    QTransform t;

    t.rotate(8.7);
    t.translate(0, 259);

    t.rotate(-20.);
    t.rotate(rotation3Angle());

    QPolygon pol2 = t.mapToPolygon(QRect(-35, 0, 35, 233));
    t.translate(0, 233);
    t.rotate(15);
    t.rotate(rotation2Angle());

    QPolygon pol3 = t.mapToPolygon(QRect(-27, 0, 27, 212));
    t.translate(0, 212);
    t.rotate(rotation1Angle());

    QPolygon pol4 = t.mapToPolygon(QRect(-42, 0, 42, 180));

    m_isCollision.setValue(pol1.intersects(pol3) || pol1.intersects(pol4) || pol2.intersects(pol4));
}
