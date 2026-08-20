// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Physics

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Qt Quick 3D Physics - Scaling test")

    PhysicsWorld {
        id: physicsWorld
        gravity: Qt.vector3d(0, -9.81, 0)
        running: true
        forceDebugDraw: true

        typicalLength: 1
        typicalSpeed: 10
        scene: viewport.scene
    }

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: scene

            PerspectiveCamera {
                id: camera1
                position: Qt.vector3d(-2, 1, 5)
                eulerRotation: Qt.vector3d(-20, -20, 0)
                clipFar: 50
                clipNear: 0.01
            }

            DirectionalLight {
                eulerRotation.x: -45
                eulerRotation.y: 45
                castsShadow: true
                brightness: 1
                shadowFactor: 100
                shadowBias: 0.05
                pcfFactor: 0.005
            }

            StaticRigidBody {
                id: floor
                position: Qt.vector3d(0, -1, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                collisionShapes: PlaneShape {}
                Model {
                    source: "#Rectangle"
                    scale: Qt.vector3d(5, 5, 1)
                    materials: PrincipledMaterial {
                        baseColor: "green"
                    }
                    castsShadows: false
                    receivesShadows: true
                }
            }


            Component {
                id: spawnComponent

                DynamicRigidBody {
                    id: thisBody
                    y: 2

                    collisionShapes: SphereShape {
                        id: sphereShape
                        diameter: 1
                    }
                    Model {
                        source: "#Sphere"
                        materials: PrincipledMaterial {
                            baseColor: "orange"
                        }
                        scale: Qt.vector3d(0.01, 0.01, 0.01).times(sphereShape.diameter)
                    }
                    Connections {
                        target: spawner
                        function onKill() {
                            thisBody.destroy()
                        }
                    }
                }
            }

            Timer {
                id: spawner
                interval: 2000
                repeat: true
                running: physicsWorld.running

                onTriggered: {
                    doSpawn()
                }

                function doSpawn() {
                    kill()
                    spawnComponent.createObject(scene)
                }
                signal kill()
            }



            Node {
                id: bodies
                property real sf: scaleSlider.value
                scale: Qt.vector3d(sf, sf, sf)

                DynamicRigidBody {
                    id: box
                    massMode: DynamicRigidBody.CustomDensity
                    density: 10
                    property vector3d startPos: Qt.vector3d(-1, 1, 0)
                    position: box.startPos

                    property real xf: xSlider.value
                    property real yf: ySlider.value
                    scale: Qt.vector3d(xf, yf, 1)


                    collisionShapes: BoxShape {
                        id: boxShape
                        property real xtent: xtentSlider.value
                        extents: Qt.vector3d(xtent, 1, 1)
                    }
                    Model {
                        source: "#Cube"
                        materials: PrincipledMaterial {
                            baseColor: "yellow"
                        }
                        scale: boxShape.extents.times(0.01)
                    }
                }

                DynamicRigidBody {
                    id: sphere
                    massMode: DynamicRigidBody.CustomDensity
                    density: 10
                    property vector3d startPos: Qt.vector3d(0, 1, 0)
                    position: sphere.startPos

                    collisionShapes: SphereShape {
                        id: sphereShape
                        diameter: 1
                    }
                    Model {
                        source: "#Sphere"
                        materials: PrincipledMaterial {
                            baseColor: "blue"
                        }
                        scale: Qt.vector3d(0.01, 0.01, 0.01).times(sphereShape.diameter)
                    }
                }

                DynamicRigidBody {
                    id: capsule
                    property vector3d startPos: Qt.vector3d(1, 1, 0)
                    position: capsule.startPos
                    massMode: DynamicRigidBody.CustomDensity
                    density: 10

                    property real xf: xSlider.value
                    property real yf: ySlider.value
                    scale: Qt.vector3d(xf, yf, yf)

                    collisionShapes: CapsuleShape {
                        id: capsuleShape
                        property real cHeight: heightSlider.value
                        diameter: 1
                        height: cHeight
                    }

                    Model {
                        geometry: CapsuleGeometry {
                            diameter: capsuleShape.diameter
                            height: capsuleShape.height
                        }
                        materials: PrincipledMaterial {
                            baseColor: "red"
                        }
                    }
                }
            } // bodies
        }
    }

 ColumnLayout {

        Label {
            text: "Uniform scale: " + scaleSlider.value
        }
        Slider {
            id: scaleSlider
            focusPolicy: Qt.NoFocus
            from: 0.2
            to: 3
            value: 1
        }
        Label {
            text: "Local x scale: " + xSlider.value
        }
        Slider {
            id: xSlider
            focusPolicy: Qt.NoFocus
            from: 0.2
            to: 3
            value: 1
        }
        Label {
            text: "Local y scale: " + ySlider.value
        }
        Slider {
            id: ySlider
            focusPolicy: Qt.NoFocus
            from: 0.2
            to: 3
            value: 1
        }
        Label {
            text: "Box x extent: " + xtentSlider.value
        }
        Slider {
            id: xtentSlider
            focusPolicy: Qt.NoFocus
            from: 0.2
            to: 5
            value: 1
        }
        Label {
            text: "Capsule height: " + heightSlider.value
        }
        Slider {
            id: heightSlider
            focusPolicy: Qt.NoFocus
            from: 0.2
            to: 5
            value: 1
        }
        Button {
            id: resetButton
            Layout.alignment: Qt.AlignHCenter
            text: "Reset scene"
            onClicked: {
                box.reset(box.startPos, Qt.vector3d(0, 0, 0))
                sphere.reset(sphere.startPos, Qt.vector3d(0, 0, 0))
                capsule.reset(capsule.startPos, Qt.vector3d(0, 0, 0))
            }
        }

    }

}
