// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Window {
    id: mainWindow
width: 1280
height: 720
visible: true

    property real fontSize: width * 0.012

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 100)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        // Models shared between particles
        Component {
            id: particleComponent
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.05, 0.05, 0.05)
                materials: DefaultMaterial {
                }
            }
        }

        ParticleSystem3D {
            id: psystem

            property vector4d cVar: Qt.vector4d(0.5,0.5,0.5,0.5)
            // Particles
            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: 250000
                color: "#ff0000"
                colorVariation: psystem.cVar
            }


            // Emitters, one per particle
            ParticleEmitter3D {
                particle: particleRed
                position: Qt.vector3d(400, -50, 0)
                particleScaleVariation: 0.8
                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(200, 200, 200);
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-50, 100, 0)
                    directionVariation: Qt.vector3d(30, 30, 30)
                }
                emitRate: 10000
                lifeSpan: 20000
            }
            Gravity3D {
                // Enable to affect only some of the particles
                //particles: [particleRed, particleBlue, particleGreen]
                direction: Qt.vector3d(0, 1, 0)
                magnitude: -20
            }
        }
    }

    Rectangle {
        anchors.fill: settingsArea
        anchors.margins: -10
        color: "#e0e0e0"
        border.color: "#000000"
        border.width: 1
        opacity: 0.8
    }

    Column {
        id: settingsArea
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
        Text {
            color: "#222840"
            font.pointSize: 12
            text: "Red Variation"
        }

    }
}
