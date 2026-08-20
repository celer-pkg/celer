// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRCONTROLLER_H
#define QQUICK3DXRCONTROLLER_H

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

#include <QtQuick3DXr/qtquick3dxrglobal.h>
#include "qquick3dxrhandinput_p.h"
#include "qtquick3dxrglobal_p.h"
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class QQuaternion;
class QQuick3DXrInputManager;

class Q_QUICK3DXR_EXPORT QQuick3DXrController : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(Controller controller READ controller WRITE setController NOTIFY controllerChanged FINAL)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged FINAL)
    Q_PROPERTY(HandPoseSpace poseSpace READ poseSpace WRITE setPoseSpace NOTIFY poseSpaceChanged FINAL)

    Q_PROPERTY(QVector3D pokePosition READ pokePosition NOTIFY pokePositionChanged FINAL)
    Q_PROPERTY(QList<QVector3D> jointPositions READ jointPositions NOTIFY jointPositionsChanged FINAL)
    Q_PROPERTY(QList<QQuaternion> jointRotations READ jointRotations NOTIFY jointRotationsChanged FINAL)

    QML_NAMED_ELEMENT(XrController)
    QML_ADDED_IN_VERSION(6, 8)
public:
    enum Controller {
        ControllerLeft = 0,
        ControllerRight = 1,
        ControllerNone = 2,
        LeftController = ControllerLeft,
        RightController = ControllerRight,
        LeftHand = ControllerLeft,
        RightHand = ControllerRight,
        UnknownController = ControllerNone,
    };
    Q_ENUM(Controller)

    enum class HandPoseSpace {
        GripPose,
        AimPose
    };
    Q_ENUM(HandPoseSpace)

    QQuick3DXrController();

    QQuick3DXrController::Controller controller() const;
    void setController(QQuick3DXrController::Controller newController);

    QQuick3DXrHandInput *handInput() const;

    HandPoseSpace poseSpace() const;
    void setPoseSpace(HandPoseSpace newPoseSpace);

    QVector3D pokePosition() const;

    QList<QVector3D> jointPositions() const;
    QList<QQuaternion> jointRotations() const;

    bool isActive() const;

Q_SIGNALS:
    void controllerChanged();
    void actionMapperChanged();

    void poseSpaceChanged();

    void pokePositionChanged();

    void jointPositionsChanged();
    void jointRotationsChanged();
    void jointDataUpdated();

    void isActiveChanged();

private:
    QPointer<QQuick3DXrInputManager> m_inputManager;
    Controller m_controller = ControllerNone;
    QMetaObject::Connection m_isActiveConnection;
    HandPoseSpace m_poseSpace = HandPoseSpace::AimPose;
    QVector3D m_pokePosition;
    bool m_isActive;
};

namespace QtQuick3DXr
{
QtQuick3DXr::Hand handForController(QQuick3DXrController::Controller controller);
QtQuick3DXr::HandPoseSpace pose_cast(QQuick3DXrController::HandPoseSpace poseSpace);
}

QT_END_NAMESPACE

#endif // QQUICK3DXRCONTROLLER_H
