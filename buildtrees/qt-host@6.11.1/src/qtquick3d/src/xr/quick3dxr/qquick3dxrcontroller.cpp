// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrcontroller_p.h"

#include "qquick3dxrinputmanager_p.h"

#if !defined(Q_OS_VISIONOS)
# include "openxr/qopenxrinputmanager_p.h"
#endif

#include "qquick3dxrview_p.h"
#include "qquick3dxractionmapper_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrController
    \inherits Node
    \inqmlmodule QtQuick3D.Xr
    \brief A tracked spatial node that tracks the position and orientation of an input controller.

    The XrController is a tracked spatial node that tracks the position and orientation of an input controller.

    Since this is a tracked node, its spatial properties should be considered read-only.

    \sa XrInputAction
*/

QQuick3DXrController::QQuick3DXrController()
{
    m_inputManager = QQuick3DXrInputManager::instance();
}

/*!
    \qmlproperty enumeration QtQuick3D.Xr::XrController::controller

    Specifies the controller to track.

    It can be one of:
    \value XrController.LeftController
    \value XrController.RightController
    \value XrController.UnknownController
    \value XrController.LeftHand (alias for \c LeftController)
    \value XrController.RightHand (alias for \c RightController)
*/

QQuick3DXrController::Controller QQuick3DXrController::controller() const
{
    return m_controller;
}

void QQuick3DXrController::setController(QQuick3DXrController::Controller newController)
{
    if (m_controller == newController)
        return;
    m_controller = newController;
    emit controllerChanged();

    disconnect(m_isActiveConnection);

    QQuick3DXrInputManager::instance()->registerController(this);
    auto *input = handInput();
    if (input) {
        setVisible(input->isActive()); // ### position not set yet, so might show up briefly in wrong location

        m_isActiveConnection = connect(input, &QQuick3DXrHandInput::isActiveChanged, this, [this, input]{
            setVisible(input->isActive());
        });

        connect(input, &QQuick3DXrHandInput::pokePositionChanged, this, &QQuick3DXrController::pokePositionChanged);
        connect(input, &QQuick3DXrHandInput::jointPositionsChanged, this, &QQuick3DXrController::jointPositionsChanged);
        connect(input, &QQuick3DXrHandInput::jointRotationsChanged, this, &QQuick3DXrController::jointRotationsChanged);
        connect(input, &QQuick3DXrHandInput::jointDataUpdated, this, &QQuick3DXrController::jointDataUpdated);
    } else {
        setVisible(false);
    }
}

QQuick3DXrHandInput *QQuick3DXrController::handInput() const
{
    if (m_inputManager && m_inputManager->isValid()) {
        if (m_controller == ControllerRight)
            return m_inputManager->rightHandInput();
        else if (m_controller == ControllerLeft)
            return m_inputManager->leftHandInput();
    }

    return nullptr;
}

/*!
    \qmlproperty enumeration XrController::poseSpace

    Specifies the pose of the controller to track, that is, the orientation and
    position relative to the physical controller.

    It can be one of:
    \value XrController.AimPose Used when aiming at something, such as with \l XrVirtualMouse.
    \value XrController.GripPose Used when grabbing something, such as when holding an object in the hand.
    \default XrController.AimPose
*/

QQuick3DXrController::HandPoseSpace QQuick3DXrController::poseSpace() const
{
    return m_poseSpace;
}

void QQuick3DXrController::setPoseSpace(HandPoseSpace newPoseSpace)
{
    if (m_poseSpace == newPoseSpace)
        return;
    m_poseSpace = newPoseSpace;
    QQuick3DXrInputManager::instance()->registerController(this);
    emit poseSpaceChanged();
}

/*!
    \qmlproperty vector3d XrController::pokePosition

    \readonly
    This property holds the position to be used for touch interactions.
    Typically, it will be the tip of the index finger when tracking a hand.

    \sa XrView::processTouch, XrView::setTouchpoint
*/

QVector3D QQuick3DXrController::pokePosition() const
{
    auto *input = handInput();
    if (input)
        return input->pokePosition();
    return {};
}

/*!
    \qmlproperty list<vector3d> XrController::jointPositions

    \readonly
    When using hand tracking, this property holds the positions of all the bones in the hand.

   \sa jointRotations, XrHandModel
*/
QList<QVector3D> QQuick3DXrController::jointPositions() const
{
    auto *input = handInput();
    if (input)
        return input->jointPositions();
    return {};
}

/*!
    \qmlproperty list<quaternion> XrController::jointRotations

    \readonly
    When using hand tracking, this property holds the orientation of all the bones in the hand.

    \sa jointPositions, XrHandModel
*/
QList<QQuaternion> QQuick3DXrController::jointRotations() const
{
    auto *input = handInput();
    if (input)
        return input->jointRotations();
    return {};
}

/*!
    \qmlproperty bool XrController::isActive
    \brief Indicates whether the controller is providing input.

    \readonly
    This property is true if the corresponding physical controller is present
    and tracking.
*/

bool QQuick3DXrController::isActive() const
{
    return m_isActive;
}

QtQuick3DXr::Hand QtQuick3DXr::handForController(QQuick3DXrController::Controller controller)
{
    QSSG_ASSERT(controller != QQuick3DXrController::ControllerNone, return QQuick3DXrInputManager::Hand::RightHand);
    switch (controller) {
    case QQuick3DXrController::ControllerLeft:
        return QQuick3DXrInputManager::Hand::LeftHand;
    case QQuick3DXrController::ControllerRight:
        return QQuick3DXrInputManager::Hand::RightHand;
    default:
        Q_UNREACHABLE();
    }
}

QtQuick3DXr::HandPoseSpace QtQuick3DXr::pose_cast(QQuick3DXrController::HandPoseSpace poseSpace)
{
    return static_cast<QtQuick3DXr::HandPoseSpace>(poseSpace);
}

QT_END_NAMESPACE


