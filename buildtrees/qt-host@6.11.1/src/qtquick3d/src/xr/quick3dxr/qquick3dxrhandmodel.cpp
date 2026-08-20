// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrhandinput_p.h"
#include "qquick3dxrinputmanager_p.h"
#include "qquick3dxrhandmodel_p.h"

#if defined(Q_OS_VISIONOS)
# include "visionos/qquick3dxrinputmanager_visionos_p.h"
#else
# include "openxr/qopenxrinputmanager_p.h"
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrHandModel
    \inherits Model
    \inqmlmodule QtQuick3D.Xr
    \brief Represents a 3D model for a hand.

    Contains an animated 3D model that tracks the user's hands.

    XrHandModel is only visible when hand tracking is active.

    \note XrHandModel depends on hand-tracking data from the underlying
    system and is, therefore, not available on all platforms.

    \section1 Platform Notes

    \section2 Apple Vision Pro

    The Apple Vision Pro will overlay video of the user's hands directly,
    and the XrHandModel will not have any content.

    \section2 Meta devices

    This required the following feature and permission lines to be added
    in the AndroidManifest.qml file of your project.

    \badcode
        <uses-permission android:name="com.oculus.permission.HAND_TRACKING" />
        <uses-feature android:name="oculus.software.handtracking" android:required="false" />
    \endcode

    Use \c true instead of \c false in the \c uses-feature tag if your app's interaction
    is implemented only for hands.
*/

QQuick3DXrHandModel::QQuick3DXrHandModel(QQuick3DNode *parent)
    : QQuick3DModel(parent)
{
}

void QQuick3DXrHandModel::updatePose()
{
    if (auto *skin = QQuick3DModel::skin()) {
        auto jointListProp = skin->joints();
        int count = jointListProp.count(&jointListProp);
        const auto positions = m_handTracker->jointPositions();
        const auto rotations = m_handTracker->jointRotations();
        for (int i = 0; i < count; ++i) {
            auto *joint = jointListProp.at(&jointListProp, i);
            joint->setPosition(positions.at(i));
            joint->setRotation(rotations.at(i));
        }
    } else {
        static bool warned = false;
        if (!warned) {
            qWarning() << "No skin available for hand model";
            warned = true;
        }
    }
}

void QQuick3DXrHandModel::setupModel()
{
    if (m_initialized) {
        qWarning() << "XrHandModel does not support changing hand";
        return;
    }
    QQuick3DXrInputManager *inputMan = QQuick3DXrInputManager::instance();
    if (m_hand == RightHand)
        m_handTracker = inputMan->rightHandInput();
    else if (m_hand == LeftHand)
        m_handTracker = inputMan->leftHandInput();
    if (!m_handTracker)
        return;

    QQuick3DXrInputManagerPrivate::get(inputMan)->setupHandModel(this);

    connect(m_handTracker, &QQuick3DXrHandInput::jointDataUpdated, this, &QQuick3DXrHandModel::updatePose);
    connect(m_handTracker, &QQuick3DXrHandInput::isHandTrackingChanged, this, [this](){
        setVisible(m_handTracker->isHandTrackingActive());
    });
    setVisible(m_handTracker->isActive());
    m_initialized = true;
}

void QQuick3DXrHandModel::componentComplete()
{
    setupModel();
    QQuick3DModel::componentComplete();
}

/*!
    \qmlproperty enumeration XrHandModel::hand
    \brief Specifies which hand the model is showing

    \value XrHandModel.LeftHand The left hand.
    \value XrHandModel.RightHand The right hand.
    \value XrHandModel.Unknown No hand is shown.

    \default XrHandModel.Unknown

    The value of this property is compatible with \l{XrController::controller}{XrController.controller}.

    \warning This property must be set when the XrHandModel is constructed.
    Changing hands later is not currently supported.
*/

QQuick3DXrHandModel::Hand QQuick3DXrHandModel::hand() const
{
    return m_hand;
}

void QQuick3DXrHandModel::setHand(Hand newHand)
{
    if (m_hand == newHand)
        return;
    m_hand = newHand;
    emit handChanged();
}
QT_END_NAMESPACE
