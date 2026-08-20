// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BACKEND_H
#define BACKEND_H

#include "animatedparam.h"

#include <QObject>
#include <qqmlregistration.h>

//! [class definition]
class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int rotation1Angle READ rotation1Angle WRITE setRot1Angle NOTIFY rot1AngleChanged)
    Q_PROPERTY(int rotation2Angle READ rotation2Angle WRITE setRot2Angle NOTIFY rot2AngleChanged)
    Q_PROPERTY(int rotation3Angle READ rotation3Angle WRITE setRot3Angle NOTIFY rot3AngleChanged)
    Q_PROPERTY(int rotation4Angle READ rotation4Angle WRITE setRot4Angle NOTIFY rot4AngleChanged)
    Q_PROPERTY(int clawsAngle READ clawsAngle WRITE setClawsAngle NOTIFY clawsAngleChanged)
    Q_PROPERTY(QString status READ status BINDABLE bindableStatus NOTIFY statusChanged)
    //! [class definition]

public:
    explicit Backend(QObject *parent = nullptr);

    int rotation1Angle() const;
    void setRot1Angle(const int angle);

    int rotation2Angle() const;
    void setRot2Angle(const int angle);

    int rotation3Angle() const;
    void setRot3Angle(const int angle);

    int rotation4Angle() const;
    void setRot4Angle(const int angle);

    int clawsAngle() const;
    void setClawsAngle(const int angle);

    QString status() const;
    QBindable<QString> bindableStatus() const;

signals:
    void rot1AngleChanged();
    void rot2AngleChanged();
    void rot3AngleChanged();
    void rot4AngleChanged();
    void clawsAngleChanged();
    void statusChanged();

private:
    AnimatedParam m_rotation1Angle;
    AnimatedParam m_rotation2Angle;
    AnimatedParam m_rotation3Angle;
    AnimatedParam m_rotation4Angle;
    AnimatedParam m_clawsAngle;

    Q_OBJECT_BINDABLE_PROPERTY(Backend, QString, m_status, &Backend::statusChanged)
    QProperty<bool> m_isCollision;

    void detectCollision();
};

#endif // BACKEND_H
