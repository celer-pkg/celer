// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRHANDINPUT_P_H
#define QQUICK3DXRHANDINPUT_P_H

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

#include <QObject>
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuick3DXrHandInput : public QObject
{
    Q_OBJECT

public:
    enum class HandPoseSpace {
        GripPose,
        AimPose
    };
    Q_ENUM(HandPoseSpace)


    explicit QQuick3DXrHandInput(QObject *parent = nullptr);

    bool isActive() const;
    void setIsActive(bool isActive);

    void setJointPositionsAndRotations(const QList<QVector3D> &newJointPositions, const QList<QQuaternion> &newJointRotations);

    QList<QVector3D> jointPositions() const;

    QList<QQuaternion> jointRotations() const;

    QVector3D pokePosition() const;
    void setPokePosition(const QVector3D &newPokePosition);

    bool isHandTrackingActive() const;
    void setIsHandTrackingActive(bool newIsHandTracking);

Q_SIGNALS:
    void isActiveChanged();

    void poseSpaceChanged();
    void posePositionChanged();
    void poseRotationChanged();

    void jointPositionsChanged();
    void jointRotationsChanged();
    void jointDataUpdated();

    void pokePositionChanged();
    void isHandTrackingChanged();

private:
    bool m_isActive = false;

    QList<QVector3D> m_jointPositions;
    QList<QQuaternion> m_jointRotations;
    QVector3D m_pokePosition;
    bool m_isHandTracking = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRHANDINPUT_P_H
