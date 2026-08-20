// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Particles3D
import QtQuick3D.Helpers
import QtQuick.Controls

Window {
    id: window
    width: 1024
    height: 768
    visible: true
    title: qsTr("Reflection Probes")

    View3D {
        id: view
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: window.color
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                source: "res/OpenfootageNET_lowerAustria01-1024.hdr"
            }
            probeOrientation: Qt.vector3d(0, -90, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-50, 700, 475)
            eulerRotation.x: -50
        }

        DirectionalLight { }

        Model {
            property real angle
            source: "#Sphere"
            x: Math.cos(angle) * 100
            z: Math.sin(angle) * 100
            y: 300
            receivesReflections: settingsPanel.sphereReceivesReflection
            materials: PrincipledMaterial {
                metalness: 1.0
                roughness: settingsPanel.sphereRoughness
            }

            NumberAnimation on angle {
                from: 0
                to: Math.PI * 2
                duration: 8000
                loops: Animation.Infinite
            }

            ReflectionProbe {
                objectName: "sphereReflProbe"
                timeSlicing: {
                    if (settingsPanel.timeSlicingIndex == 0) ReflectionProbe.None
                    else if (settingsPanel.timeSlicingIndex == 1) ReflectionProbe.AllFacesAtOnce
                    else ReflectionProbe.IndividualFaces
                }
                refreshMode: {
                    if (settingsPanel.refreshModeIndex == 0) ReflectionProbe.EveryFrame
                    else ReflectionProbe.FirstFrame
                }
                quality: {
                    if (settingsPanel.qualityIndex == 0) ReflectionProbe.VeryLow
                    else if (settingsPanel.qualityIndex == 1) ReflectionProbe.Low
                    else if (settingsPanel.qualityIndex == 2) ReflectionProbe.Medium
                    else if (settingsPanel.qualityIndex == 3) ReflectionProbe.High
                    else ReflectionProbe.VeryHigh
                }
                boxSize: Qt.vector3d(300, 300, 300)
            }
        }

        Model {
            position: Qt.vector3d(200, 300, 0)
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "green"
            }
        }

        Model {
            position: Qt.vector3d(-200, 300, 0)
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "green"
            }
        }

        Model {
            position: Qt.vector3d(0, 200, -285)
            scale: Qt.vector3d(6, 4, 0.3)
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "yellow"
                roughness: 0.1
                metalness: 1.0
            }
        }

        Model {
            scale: Qt.vector3d(6, 0.1, 6)
            source: "#Cube"
            receivesReflections: settingsPanel.floorReceivesReflection
            materials: PrincipledMaterial {
                baseColor: "lightBlue"
                roughness: 0.1
                metalness: 1.0
            }
        }

        ReflectionProbe {
            objectName: "reflProbe"
            position: settingsPanel.probePosition
            timeSlicing: {
                if (settingsPanel.timeSlicingIndex == 0) ReflectionProbe.None
                else if (settingsPanel.timeSlicingIndex == 1) ReflectionProbe.AllFacesAtOnce
                else ReflectionProbe.IndividualFaces
            }
            refreshMode: {
                if (settingsPanel.refreshModeIndex == 0) ReflectionProbe.EveryFrame
                else ReflectionProbe.FirstFrame
            }
            quality: {
                if (settingsPanel.qualityIndex == 0) ReflectionProbe.VeryLow
                else if (settingsPanel.qualityIndex == 1) ReflectionProbe.Low
                else if (settingsPanel.qualityIndex == 2) ReflectionProbe.Medium
                else if (settingsPanel.qualityIndex == 3) ReflectionProbe.High
                else ReflectionProbe.VeryHigh
            }
            parallaxCorrection: settingsPanel.probeParallaxCorrection
            boxSize: settingsPanel.probeSize
        }

        ParticleSystem3D {
            position: Qt.vector3d(0, 300, 0)

            SpriteParticle3D {
                id: snowFlakeParticle
                sprite: Texture {
                    source: "res/snowflake.png"
                }
                maxAmount: 100
                particleScale: 20.0
                fadeOutDuration: 500
                billboard: true
            }

            ParticleEmitter3D {
                enabled: settingsPanel.spriteParticlesEnabled
                particle: snowFlakeParticle
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(100, 200, 0)
                    directionVariation: Qt.vector3d(20, 20, 20)
                }
                particleScaleVariation: 0.4
                emitRate: 50
                lifeSpan: 4000
            }

            Gravity3D {
                magnitude: 100
            }
        }

        ParticleSystem3D {
            position: Qt.vector3d(0, 300, 0)

            Component {
                id: particleComponent
                Model {
                    source: "#Sphere"
                    scale: Qt.vector3d(0.8, 0.8, 0.8)
                    receivesReflections: true
                    materials: PrincipledMaterial {
                        baseColor: "red"
                        roughness: 0.1
                        metalness: 1.0
                    }
                }
            }

            ModelParticle3D {
                id: particleRed
                delegate: particleComponent
                maxAmount: 10
            }

            ParticleEmitter3D {
                enabled: settingsPanel.modelParticlesEnabled
                particle: particleRed
                velocity: VectorDirection3D {
                    direction: Qt.vector3d(-100, 200, 0)
                }
                emitRate: 1
                lifeSpan: 4000
            }

            Gravity3D {
                magnitude: 100
            }
        }
    }

    SettingsPanel {
        id: settingsPanel
        iconSize: 16 + Math.max(window.width, window.height) * 0.05
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            color: "white"
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: view
                id: dbg
                visible: false
            }
        }
    }
}
