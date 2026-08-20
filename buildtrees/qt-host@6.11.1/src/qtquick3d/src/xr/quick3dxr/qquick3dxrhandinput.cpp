// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrhandinput_p.h"
#include "qquick3dxrinputmanager_p.h"

#if defined(Q_OS_VISIONOS)
#include "visionos/qquick3dxrinputmanager_visionos_p.h"
#else
#include "openxr/qopenxrinputmanager_p.h"
#endif

QT_BEGIN_NAMESPACE

QQuick3DXrHandInput::QQuick3DXrHandInput(QObject *parent)
    : QObject(parent)
{

}

bool QQuick3DXrHandInput::isActive() const
{
    return m_isActive;
}

void QQuick3DXrHandInput::setIsActive(bool isActive)
{
    if (m_isActive == isActive)
        return;

    m_isActive = isActive;
    emit isActiveChanged();
}

void QQuick3DXrHandInput::setJointPositionsAndRotations(const QList<QVector3D> &newJointPositions, const QList<QQuaternion> &newJointRotations)
{
    m_jointPositions = newJointPositions;
    emit jointPositionsChanged();
    m_jointRotations = newJointRotations;
    emit jointRotationsChanged();
    emit jointDataUpdated();

    QQuick3DXrInputManager *inputMan = QQuick3DXrInputManager::instance();
    const auto pokeIndex = QQuick3DXrInputManagerPrivate::get(inputMan)->getPokeJointIndex();

    if (pokeIndex >= 0 && pokeIndex < m_jointPositions.size())
        setPokePosition(m_jointPositions[pokeIndex]);
}

QList<QVector3D> QQuick3DXrHandInput::jointPositions() const
{
    return m_jointPositions;
}

QList<QQuaternion> QQuick3DXrHandInput::jointRotations() const
{
    return m_jointRotations;
}

QVector3D QQuick3DXrHandInput::pokePosition() const
{
    return m_pokePosition;
}

void QQuick3DXrHandInput::setPokePosition(const QVector3D &newPokePosition)
{
    if (m_pokePosition == newPokePosition)
        return;
    m_pokePosition = newPokePosition;
    emit pokePositionChanged();
}

bool QQuick3DXrHandInput::isHandTrackingActive() const
{
    return m_isHandTracking;
}

void QQuick3DXrHandInput::setIsHandTrackingActive(bool newIsHandTracking)
{
    if (m_isHandTracking == newIsHandTracking)
        return;
    m_isHandTracking = newIsHandTracking;
    emit isHandTrackingChanged();
}

QT_END_NAMESPACE
