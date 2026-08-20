// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers

Item {
    id: mainWindow
    anchors.fill: parent

    property color fireColor: "#fdb597"

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
            oitMethod: SceneEnvironment.OITLinkedList
        }

        Node {
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 100, 300)
            }
        }

        Node {
            id: sceneRoot
            PointLight {
                id: pointLight
                position: Qt.vector3d(0, 100, 0)
                color: mainWindow.fireColor
                brightness: 60
                castsShadow: true
                SequentialAnimation {
                    loops: NumberAnimation.Infinite
                    running: true
                    NumberAnimation {
                        target: pointLight
                        property: "brightness"
                        from: 60
                        to: 80
                        duration: 400
                    }
                    NumberAnimation {
                        target: pointLight
                        property: "brightness"
                        from: 80
                        to: 60
                        duration: 400
                    }
                }
                SequentialAnimation {
                    loops: NumberAnimation.Infinite
                    running: true
                    NumberAnimation {
                        target: pointLight
                        property: "position.x"
                        from: -1
                        to: 1
                        duration: 100
                    }
                    NumberAnimation {
                        target: pointLight
                        property: "position.x"
                        from: 1
                        to: -1
                        duration: 100
                    }
                }
            }

            Model {
                source: "#Rectangle"
                scale: Qt.vector3d(10, 10, 1)
                position: Qt.vector3d(0,-10, -200)
                eulerRotation.x: -89

                materials: DefaultMaterial {
                    diffuseColor: "gray"
                }
            }
            Model {
                source: "#Sphere"
                scale: Qt.vector3d(3, 0.01, 3)
                position: Qt.vector3d(0, -9, -30)
                materials: DefaultMaterial {
                    diffuseColor: "#979797"
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        source: "images/firestone.png"
                    }
                }
            }

            FireStone {
                stoneSize: 3
                position: Qt.vector3d(-0, 0, -200)
            }
            FireStone {
                stoneSize: 3
                position: Qt.vector3d(-100, 20, -175)
                stoneRotationZ: 45
                stoneRotationY: 160
            }
            FireStone {
                stoneSize: 3
                position: Qt.vector3d(-150, 0, -10)
                stoneRotationZ: 90
            }
            FireStone {
                stoneSize: 3
                position: Qt.vector3d(-100, -20, 80)
                stoneRotationZ: 45
            }
            FireStone {
                stoneSize: 3
                position: Qt.vector3d(80, -20, 80)
                stoneRotationZ: -35
            }
            FireStone {
                stoneSize: 3
                position: Qt.vector3d(150, 0, -40)
                stoneRotationZ: 90
            }

            FireStick {
                id: fs1
                stickSize: 3
                stickRotationZ: 20
                stickRotationX: 10
                position: Qt.vector3d(-50, 0, 0)
            }
            FireStick {
                id: fs2
                stickSize: 2.5
                stickRotationZ: 170
                stickRotationX: 10
                position: Qt.vector3d(50, 0, 0)
                glowFactor: 1.0 + Math.random() * 1.5
            }
            FireStick {
                id: fs3
                stickSize: 3
                stickRotationZ: 0
                stickRotationY: 90
                position: Qt.vector3d(0, 0, -50)
                glowFactor: 1.0 + Math.random() * 1.5
            }
            FireStick {
                stickSize: 2.3
                stickRotationZ: 0
                stickRotationY: 85
                position: Qt.vector3d(0, 0, 50)
                glowFactor: 1.0 + Math.random() * 1.5
            }
            FireStick {
                stickSize: 2
                stickRotationZ: -20
                stickRotationY: 145
                position: Qt.vector3d(25, 0, 25)
                glowFactor: 1.0 + Math.random() * 1.5
            }
            FireStick {
                id: fs4
                stickSize: 2
                stickRotationZ: 40
                stickRotationY: 65
                position: Qt.vector3d(-25, 10, 25)
                glowFactor: 1.0 + Math.random() * 1.5
            }
            FireStick {
                stickSize: 1
                stickRotationZ: 0
                stickRotationY: 65
                position: Qt.vector3d(-15, 0, 55)
                glowFactor: 1.0 + Math.random() * 1.5
            }
        }
        Model {
            id: rm
            source: "#Sphere"
            position: test.position
            scale: Qt.vector3d(test.outerRadius / 100, test.outerRadius / 100, test.outerRadius / 100)
            materials : [
                DefaultMaterial {
                }
            ]
        }

        ParticleSystem3D {
            id: system
            running: true
            startTime: 1000
            Repeller3D {
                id: rp1
                position: Qt.vector3d(-10, 15, 5)
                radius: 5
                outerRadius: 10
                strength: 20
            }
            Repeller3D {
                id: rp2
                position: Qt.vector3d(-15, 28, 5)
                radius: 5
                outerRadius: 10
                strength: 20
            }
            Repeller3D {
                id: rp3
                position: Qt.vector3d(5, 5, 0)
                radius: 5
                outerRadius: 10
                strength: 20
            }
            Repeller3D {
                id: rp4
                position: Qt.vector3d(3, 0, 20)
                radius: 5
                outerRadius: 10
                strength: 20
            }
            Repeller3D {
                id: rp5
                position: Qt.vector3d(20, 5, 0)
                radius: 5
                outerRadius: 10
                strength: 20
            }
            Repeller3D {
                id: test
                position: Qt.vector3d(35, 2, 0)
                radius: 5
                outerRadius: 10
                strength: 20
            }

            SpriteParticle3D {
                id: sparkle
                particleScale: 1
                maxAmount: 20
                fadeInEffect: Particle3D.FadeNone
                fadeOutEffect: Particle3D.FadeNone
                color: mainWindow.fireColor
            }
            RandomEmitter {
                particle: sparkle
                lifeSpan: 500
                minTime: 500
                timeVariance: 2500
                running: true
                positionVariance: Qt.vector3d(40, 0, 40)
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 200, 0)
                    directionVariation: Qt.vector3d(40, 20, 40)
                }
                position: Qt.vector3d(0, 0, 0)
            }

            SpriteParticle3D {
                id: fire
                sprite: Texture {
                    source: "images/flame.png"
                }
                colorTable: Texture {
                    source: "images/color_table3.png"
                }
                maxAmount: 7000
                particleScale: 3
                color: mainWindow.fireColor
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.4)
            }

            function firePos(idx)
            {
                var r = idx * 0.4;
                var a = 2.0 * Math.PI * (idx % 10) / 10.0
                var x = r * Math.cos(a)
                var z = r * Math.sin(a)
                return Qt.vector3d(x, 0, z)
            }
            function fireSize(idx, total)
            {
                var r = idx * 0.4;
                var maxR = total * 0.4
                return 1.0 - (r / maxR) + 0.5
            }

            Repeater3D {
                model: 120

                FireParticles {
                    required property int index
                    ps: system
                    particle: fire
                    size: system.fireSize(index, 120)
                    position: system.firePos(index)
                }
            }
        }
    }
    Pane {
        anchors.top: view3D.top
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: dbg.visible ? "Hide DebugView" : "Show DebugView"
            anchors.right: parent.right
            anchors.top: parent.top
            MouseArea {
                anchors.fill: parent
                onClicked: dbg.visible = !dbg.visible
                DebugView {
                    y: debugViewToggleText.height * 2
                    anchors.right: parent.right
                    source: view3D
                    id: dbg
                    visible: false
                }
            }
        }
    }
}
