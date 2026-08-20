// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D
import QtQuick3D.Physics
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: appWindow
    width: 800
    height: 600
    visible: true
    title: qsTr("Qt Quick 3D Physics - Material example")

    PhysicsWorld {
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            backgroundMode: SceneEnvironment.Color
            clearColor: "#f0f0f0"
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 500, 1500)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            clipFar: 10000
            clipNear: 10
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-45, 45, 0)
            castsShadow: true
            brightness: 1
            shadowFactor: 100
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
        }

        //! [material]
        PhysicsMaterial {
            id: physicsMaterial
            staticFriction: staticFrictionSlider.value
            dynamicFriction: dynamicFrictionSlider.value
            restitution: restitutionSlider.value
        }
        //! [material]

        //! [floor]
        StaticRigidBody {
            eulerRotation: Qt.vector3d(-79, -90, 0)
            scale: Qt.vector3d(20, 30, 100)
            physicsMaterial: physicsMaterial
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [floor]

        //! [box]
        DynamicRigidBody {
            id: box
            physicsMaterial: physicsMaterial
            massMode: DynamicRigidBody.CustomDensity
            density: 10
            property vector3d startPosition: Qt.vector3d(700, 300, 0)
            position: startPosition
            Model {
                source: "#Cube"
                materials: PrincipledMaterial {
                    baseColor: "red"
                }
            }
            collisionShapes: BoxShape {}
        }
        //! [box]
    }

    Frame {
        background: Rectangle {
            color: "#c0c0c0"
            border.color: "#202020"
        }
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10

        ColumnLayout {
            Label {
                text: "Static friction: " + staticFrictionSlider.value.toFixed(2)
            }
            Slider {
                id: staticFrictionSlider
                focusPolicy: Qt.NoFocus
                from: 0
                to: 1
                value: 0.1
            }
            Label {
                text: "Dynamic friction: " + dynamicFrictionSlider.value.toFixed(2)
            }
            Slider {
                id: dynamicFrictionSlider
                focusPolicy: Qt.NoFocus
                from: 0
                to: 1
                value: 0.1
            }
            Label {
                text: "Restitution: " + restitutionSlider.value.toFixed(2)
            }
            Slider {
                id: restitutionSlider
                focusPolicy: Qt.NoFocus
                from: 0
                to: 1
                value: 0.1
            }
            Button {
                id: resetButton
                Layout.alignment: Qt.AlignHCenter
                text: "Reset box"
                onClicked: box.reset(box.startPosition, Qt.vector3d(0, 0, 0))
            }
        }
    }
}
