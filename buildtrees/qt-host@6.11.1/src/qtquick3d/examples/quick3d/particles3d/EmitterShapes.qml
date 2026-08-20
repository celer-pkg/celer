// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    id: mainWindow

    property bool fill: checkBoxFill.checked

    anchors.fill: parent

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#202020"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
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
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: camera
                    property: "cameraAnim"
                    to: 0
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
            position: Qt.vector3d(0, 400 + cameraAnim * 400, 800 - cameraAnim * 800)
            eulerRotation: Qt.vector3d(-30 - cameraAnim * 60, 0, 0)
        }

        PointLight {
            position: Qt.vector3d(0, 400, 0)
            brightness: 10
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        ParticleSystem3D {
            id: psystem

            NumberAnimation on eulerRotation.y {
                running: true
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: 12000
            }

            // Particles
            SpriteParticle3D {
                id: particleWhite
                sprite: Texture {
                    source: "images/dot.png"
                }
                maxAmount: 4000 * 3
                color: "#ffffff"
                billboard: true
            }

            // Emitters, one per particle
            ParticleEmitter3D {
                particle: particleWhite
                position: Qt.vector3d(-300, 0, 0)
                scale: Qt.vector3d(2.0, 2.0, 3.0)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Cube
                    fill: mainWindow.fill
                }
                emitRate: sliderEmitRate.sliderValue
                lifeSpan: 2000
                depthBias: depthBias.sliderValue
                Model {
                    source: "#Cube"
                    opacity: 0.4
                    materials: PrincipledMaterial {
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleWhite
                position: Qt.vector3d(0, 0, 0)
                scale: Qt.vector3d(2.0, 2.0, 3.0)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Sphere
                    fill: mainWindow.fill
                }
                emitRate: sliderEmitRate.sliderValue
                lifeSpan: 2000
                depthBias: depthBias.sliderValue
                Model {
                    source: "#Sphere"
                    opacity: 0.4
                    materials: PrincipledMaterial {
                    }
                }
            }
            ParticleEmitter3D {
                particle: particleWhite
                position: Qt.vector3d(300, 0, 0)
                scale: Qt.vector3d(2.0, 2.0, 3.0)
                shape: ParticleShape3D {
                    type: ParticleShape3D.Cylinder
                    fill: mainWindow.fill
                }
                emitRate: sliderEmitRate.sliderValue
                lifeSpan: 2000
                depthBias: depthBias.sliderValue
                Model {
                    source: "#Cylinder"
                    opacity: 0.4
                    materials: PrincipledMaterial {
                    }
                }
            }
        }
    }

    SettingsView {
        CustomCheckBox {
            id: checkBoxFill
            text: "Fill"
            checked: false
        }
        CustomLabel {
            text: "Particles emitRate"
        }
        CustomSlider {
            id: sliderEmitRate
            sliderValue: 1000
            fromValue: 0
            toValue: 2000
        }
        CustomLabel {
            text: "Particle depthBias"
        }
        CustomSlider {
            id: depthBias
            sliderValue: -20
            fromValue: -20
            toValue: 20
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
