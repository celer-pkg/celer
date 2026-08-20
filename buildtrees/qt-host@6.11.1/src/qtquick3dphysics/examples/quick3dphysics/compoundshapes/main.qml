// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("Qt Quick 3D Physics - Compound Shapes")

    //! [world]
    PhysicsWorld {
        id: physicsWorld
        enableCCD: true
        maximumTimestep: 20
        scene: viewport.scene
    }
    //! [world]

    View3D {
        id: viewport
        property real ringY: 900
        property real ringDistance: 165
        anchors.fill: parent

        //! [environment]
        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            backgroundMode: SceneEnvironment.Color
            clearColor: "lightblue"
        }

        focus: true

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 900, 1500)
            eulerRotation: Qt.vector3d(-10, 0, 0)
            clipFar: 15500
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1.5
            shadowMapQuality: Light.ShadowMapQualityHigh
        }
        //! [environment]

        //! [plane]
        StaticRigidBody {
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(500, 500, 1)
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [plane]

        //! [link]
        MeshLink {
            id: leftLink
            isKinematic: true
            property vector3d startPos: Qt.vector3d(-6 * viewport.ringDistance,
                                                    viewport.ringY,
                                                    0)
            property vector3d startRot: Qt.vector3d(90, 0, 0)
            kinematicPosition: startPos
            position: startPos
            kinematicEulerRotation: startRot
            eulerRotation: startRot
            color: "red"
        }

        CapsuleLink {
            position: Qt.vector3d(-5 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-4 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-3 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(-2 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(-1 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        CapsuleLink {
            position: Qt.vector3d(0, viewport.ringY, 0)
        }

        MeshLink {
            position: Qt.vector3d(1 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(2 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            position: Qt.vector3d(3 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(0, 90, 0)
        }

        MeshLink {
            position: Qt.vector3d(4 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        CapsuleLink {
            position: Qt.vector3d(5 * viewport.ringDistance, viewport.ringY, 0)
            eulerRotation: Qt.vector3d(90, 0, 0)
        }

        MeshLink {
            id: rightLink
            isKinematic: true
            property vector3d startPos: Qt.vector3d(6 * viewport.ringDistance,
                                                    viewport.ringY,
                                                    0)
            property vector3d startRot: Qt.vector3d(90, 0, 0)
            kinematicPosition: startPos
            position: startPos
            kinematicEulerRotation: startRot
            eulerRotation: startRot
            color: "red"
        }
        //! [link]

        //! [animation]
        Connections {
            target: physicsWorld
            property real totalAnimationTime: 12000
            function onFrameDone(timeStep) {
                let progressStep = timeStep / totalAnimationTime
                animationController.progress += progressStep
                if (animationController.progress >= 1) {
                    animationController.completeToEnd()
                    animationController.reload()
                    animationController.progress = 0
                }
            }
        }

        AnimationController {
            id: animationController
            animation: SequentialAnimation {
                NumberAnimation {
                    target: leftLink
                    property: "kinematicPosition.x"
                    to: 3 * viewport.ringDistance
                    from: -6 * viewport.ringDistance
                    easing.type: Easing.InOutCubic
                    duration: 1000
                }
                NumberAnimation {
                    target: leftLink
                    property: "kinematicPosition.x"
                    from: 3 * viewport.ringDistance
                    to: -6 * viewport.ringDistance
                    easing.type: Easing.InOutCubic
                    duration: 1000
                }
                NumberAnimation {
                    target: rightLink
                    property: "kinematicPosition.x"
                    to: -3 * viewport.ringDistance
                    from: 6 * viewport.ringDistance
                    easing.type: Easing.InOutCubic
                    duration: 1000
                }
                NumberAnimation {
                    target: rightLink
                    property: "kinematicPosition.x"
                    from: -3 * viewport.ringDistance
                    to: 6 * viewport.ringDistance
                    easing.type: Easing.InOutCubic
                    duration: 1000
                }
            }
        }
        //! [animation]
    }
}
