// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D

Item {
    anchors.fill: parent

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#101010"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 100, 600)
            clipFar: 2000
        }

        PointLight {
            position: Qt.vector3d(200, 600, 400)
            brightness: 40
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }

        Node {
            id: sceneRoot
            Model {
                id: rectangle
                source: "#Rectangle"
                y: -200
                scale: Qt.vector3d(30, 15, 15)
                eulerRotation.x: -90
                materials: [
                    PrincipledMaterial {
                        baseColor: Qt.rgba(1.0, 1.0, 1.0, 1.0)
                    }
                ]
            }

            Model {
                visible: showShapeBox.checked
                id: debugModel
                y: 10
                geometry: sceneShape.geometry
                materials: [
                    PrincipledMaterial {
                        baseColor: Qt.rgba(0.0, 1.0, 0.0, 1.0)
                        lighting: PrincipledMaterial.NoLighting
                    }
                ]
            }

            Node {
                id: rotatingNode
                position.y: -150
                position.z: -200
                NumberAnimation on eulerRotation.y {
                    running: true
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                    duration: 10000
                }
                Model {
                    x: 300
                    source: "#Cube"
                    scale: Qt.vector3d(1.5, 1.5, 1.5)
                    materials: PrincipledMaterial {
                        baseColor: Qt.rgba(0.9, 0.9, 0.6, 1.0)
                    }
                    eulerRotation.x: rotatingNode.eulerRotation.y
                    eulerRotation.z: 20
                }
                Model {
                    x: -300
                    source: "#Sphere"
                    scale: Qt.vector3d(2.5, 2.5, 2.5)
                    materials: PrincipledMaterial {
                        baseColor: Qt.rgba(0.9, 0.6, 0.6, 1.0)
                        opacity: 0.4
                    }
                }
            }
        }
        //! [particle system]
        ParticleSystem3D {
            id: psystem

            // Start so that the snowing is in full steam
            startTime: 0
            //! [particle system]

            //! [sprite particle]
            SpriteParticle3D {
                id: snowParticle
                sprite: Texture {
                    source: "images/snowflake.png"
                }
                maxAmount: 1500 * 100
                color: "#ffffff"
                colorVariation: Qt.vector4d(0.0, 0.0, 0.0, 0.5);
                fadeInDuration: 1000
                fadeOutDuration: 1000
            }
            //! [sprite particle]

            //! [particle emitter]
            ParticleEmitter3D {
                id: emitter
                particle: snowParticle
                position: Qt.vector3d(0, 0, 0)

                shape: ParticleSceneShape3D {
                    id: sceneShape
                    scene: sceneRoot
                    sceneExtents: Qt.vector3d(1000, 500, 1000)
                    excludedNodes: includeFloorBox.checked ? [debugModel] : [rectangle, debugModel]
                    shapeResolution: resolutionSlider.sliderValue
                }

                particleRotationVariation: Qt.vector3d(180, 180, 180)
                particleRotationVelocityVariation: Qt.vector3d(50, 50, 50);
                particleScale: 2.0
                particleScaleVariation: 0.5;
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(0, 100, 0)
                    directionVariation: Qt.vector3d(0, 0, 0)
                }
                emitRate: 1000
                lifeSpan: 15000
            }
            //! [particle emitter]

            //! [particle affectors]
            Wander3D {
                enabled: true
            }
        }
    }

    SettingsView {
        CustomLabel {
            text: "Show shape"
        }
        CustomCheckBox {
            id: showShapeBox
            checked: false
        }
        CustomLabel {
            text: "Include floor"
        }
        CustomCheckBox {
            id: includeFloorBox
            checked: false
        }
        CustomLabel {
            text: "Shape Resolution"
        }
        CustomSlider {
            id: resolutionSlider
            sliderValue: 10.0
            fromValue: 1.0
            toValue: 100.0
        }
    }

    LoggingView {
        anchors.bottom: parent.bottom
        particleSystems: [psystem]
    }
}
