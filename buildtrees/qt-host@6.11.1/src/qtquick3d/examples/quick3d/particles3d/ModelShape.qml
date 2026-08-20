// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow
    anchors.fill: parent
    property real fontSize: width * 0.012

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            property real cameraAnim: 0
            SequentialAnimation {
                running: true
                loops: Animation.Infinite
                NumberAnimation {
                    target: camera
                    property: "cameraAnim"
                    to: 1
                    duration: 4000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: camera
                    property: "cameraAnim"
                    to: 0
                    duration: 4000
                    easing.type: Easing.InOutQuad
                }
            }
            position: Qt.vector3d(500 * Math.sin(cameraAnim * Math.PI * 2), 0, 500 * Math.cos(cameraAnim * Math.PI * 2))
            eulerRotation: Qt.vector3d(0, cameraAnim * 360, 0)
        }

        Timer {
            running: true
            repeat: true
            interval: 4000
            onTriggered: {
                if (shape1.delegate === cube) {
                    shape1.delegate = suzanne;
                    shape2.delegate = suzanne;
                } else if (shape1.delegate === suzanne) {
                    shape1.delegate = cube;
                    shape2.delegate = cube;
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            SpriteParticle3D {
                id: particleFire
                sprite: Texture {
                    source: "images/sphere.png"
                }
                colorTable: Texture {
                    source: "images/colorTable.png"
                }
                maxAmount: 6000
                color: "#ffffff"
                billboard: true
                blendMode: SpriteParticle3D.Screen
            }

            Component {
                id: suzanne
                Model {
                    source: "meshes/suzanne.mesh"
                    scale: Qt.vector3d(100, 100, 100)
                    materials: PrincipledMaterial { baseColor: "red" }
                }
            }

            Component {
                id: cube
                Model {
                    source: "#Cube"
                    scale: Qt.vector3d(2, 2, 2)
                    materials: PrincipledMaterial { baseColor: "red" }
                }
            }

            ParticleEmitter3D {
                particle: particleFire
                position: Qt.vector3d(-150, 0, 0)
                particleScale: 1
                particleScaleVariation: 1
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 60, 0)
                    directionVariation: Qt.vector3d(6, 6, 6)
                }
                emitRate: 3000
                lifeSpan: 1000
                shape: ParticleModelShape3D {
                    id: shape1
                    delegate: suzanne
                }
                Node {
                    x: -30
                    y: 150
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Filled"
                        font.pointSize: mainWindow.fontSize
                        color: "#ffffff"
                    }
                }
            }

            ParticleEmitter3D {
                particle: particleFire
                position: Qt.vector3d(150, 0, 0)
                particleScale: 1
                particleScaleVariation: 1
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 60, 0)
                    directionVariation: Qt.vector3d(6, 6, 6)
                }
                emitRate: 3000
                lifeSpan: 1000
                shape: ParticleModelShape3D {
                    id: shape2
                    delegate: suzanne
                    fill: false
                }
                Node {
                    x: -30
                    y: 150
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Surface"
                        font.pointSize: mainWindow.fontSize
                        color: "#ffffff"
                    }
                }
            }
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
