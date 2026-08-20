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
    title: qsTr("Qt Quick 3D Physics - Cannon")

    PhysicsWorld {
        scene: viewport.scene
    }

    View3D {
        id: viewport
        width: parent.width
        height: parent.height
        focus: true

        environment: SceneEnvironment {
            antialiasingMode: SceneEnvironment.MSAA
            backgroundMode: SceneEnvironment.Color
            clearColor: "#f0f0f0"
        }

        //! [camera]
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-4000, 5000, 10000)
            eulerRotation: Qt.vector3d(-20, -20, 0)
            clipFar: 200000
            clipNear: 100
        }
        //! [camera]

        //! [light]
        DirectionalLight {
            eulerRotation: Qt.vector3d(-45, 45, 0)
            castsShadow: true
            brightness: 1
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            shadowMapFar: camera.clipFar
            shadowFactor: 50
            csmNumSplits: 2
            csmSplit1: 0.1
            csmSplit2: 0.3
            softShadowQuality: Light.PCF4
        }
        //! [light]

        //! [floor]
        StaticRigidBody {
            eulerRotation: Qt.vector3d(-90, 0, 0)
            collisionShapes: PlaneShape {}
            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(2000, 2000, 1)
                materials: PrincipledMaterial {
                    baseColor: "green"
                }
                castsShadows: false
                receivesShadows: true
            }
        }
        //! [floor]

        //! [spawner]
        Node {
            id: shapeSpawner
            property var instancesBoxes: []
            property var instancesSpheres: []
            property int stackCount: 0
            property var boxComponent: Qt.createComponent("Box.qml")
            property var sphereComponent: Qt.createComponent("Sphere.qml")

            function createStack(stackZ, numStacks) {
                let size = 10
                let extents = 400

                for (var i = 0; i < size; i++) {
                    for (var j = 0; j < size - i; j++) {
                        let x = j * 2 - size + i
                        let y = i * 2 + 1
                        let z = 5 * (stackZ - numStacks)
                        let center = Qt.vector3d(x, y, z).times(0.5 * extents)
                        let box = boxComponent.incubateObject(shapeSpawner, {
                                                                  "position": center,
                                                                  "xyzExtents": extents
                                                              })
                        instancesBoxes.push(box)
                    }
                }
            }

            function createBall(position, forward) {
                var diameter = 600
                var speed = 20000
                let settings = {
                    "position": position,
                    "sphereDiameter": diameter
                }
                let sphere = sphereComponent.createObject(shapeSpawner, settings)
                sphere.setLinearVelocity(forward.times(speed))
                instancesSpheres.push(sphere)

                if (sphere === null) {
                    console.log("Error creating object")
                }
            }

            function reset() {
                // Only run method if previous stack has been created fully
                for (var i = 0; i < instancesBoxes.length; i++)
                    if (!instancesBoxes[i].object)
                        return

                instancesSpheres.forEach(sphere => {
                                             sphere.collisionShapes = []
                                             sphere.destroy()
                                         })
                instancesBoxes.forEach(box => {
                                           box.object.collisionShapes = []
                                           box.object.destroy()
                                       })
                instancesSpheres = []
                instancesBoxes = []

                for (var stackI = 0; stackI < stackSlider.value; stackI++) {
                    shapeSpawner.createStack(stackI, stackSlider.value)
                }
            }
        }
        //! [spawner]
        Crosshair {
            id: crossHair
            anchors.centerIn: parent
        }
    }

    Component.onCompleted: {
        shapeSpawner.reset()
    }

    WasdController {
        speed: 100
        controlledObject: camera
        Keys.onPressed: event => {
                            handleKeyPress(event)
                            if (event.key === Qt.Key_Space) {
                                shapeSpawner.createBall(camera.position,
                                                        camera.forward)
                            }
                        }
        Keys.onReleased: event => {
                             handleKeyRelease(event)
                         }
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
                text: "No. Stacks: " + stackSlider.value.toFixed(0)
            }
            Slider {
                id: stackSlider
                focusPolicy: Qt.NoFocus
                from: 1
                to: 9
                value: 4
                stepSize: 1
                snapMode: Slider.SnapOnRelease
            }
            Button {
                id: resetButton
                Layout.alignment: Qt.AlignHCenter
                text: "Reset scene"
                onClicked: shapeSpawner.reset()
            }
            Button {
                id: fireButton
                Layout.alignment: Qt.AlignHCenter
                text: "Fire!"
                onClicked: shapeSpawner.createBall(camera.position,
                                                   camera.forward)
            }
        }
    }
}
