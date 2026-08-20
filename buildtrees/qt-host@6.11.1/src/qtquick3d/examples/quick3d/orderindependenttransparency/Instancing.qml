// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: mainWindow
    anchors.fill: parent

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#000000"
            antialiasingMode: AppSettings.antialiasingMode
            antialiasingQuality: AppSettings.antialiasingQuality
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData{}
            }
            oitMethod: SceneEnvironment.OITWeightedBlended
        }

        Node {
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 100, 300)
            }


        }

        Node {
            id: sceneRoot

            Model {
                id: pillar
                source: "#Cylinder"
                scale: Qt.vector3d(0.3, 100, 0.3)
                position: Qt.vector3d(100, 0, 100)
                materials: [
                    PrincipledMaterial {
                        id: pillarMaterial
                        normalMap: Texture {
                            source: "images/pillar_normal.png"
                            scaleU: 20.0
                        }
                        roughness: 0.2
                    }
                ]
            }

            Model {
                id: pillar2
                source: "#Cylinder"
                scale: Qt.vector3d(0.3, 100, 0.3)
                position: Qt.vector3d(-100, 0, 100)
                materials: [
                    PrincipledMaterial {
                        id: pillarMaterial2
                        normalMap: Texture {
                            source: "images/pillar_normal.png"
                            scaleU: 20.0
                        }

                        roughness: 0.2
                    }
                ]
            }

            RandomInstancing {
                id: randomInstancing
                instanceCount: 80

                position: InstanceRange {
                    from: Qt.vector3d(-200, -200, -200)
                    to: Qt.vector3d(200, 200, 200)
                }
                scale: InstanceRange {
                    from: Qt.vector3d(5, 5, 5)
                    to: Qt.vector3d(20, 20, 20)
                    proportional: true
                }
                rotation: InstanceRange {
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 360, 360)
                }
                color: InstanceRange {
                    from: "grey"
                    to: "white"
                    proportional: true
                }

                randomSeed: 1423
            }
            DirectionalLight {
                position: Qt.vector3d(0, -300, 0)
                eulerRotation: Qt.vector3d(-90, 0, 0)
                brightness: 4
                color: Qt.rgba(0.8, 0.8, 1.0, 1.0)
            }

            Node {
                NumberAnimation on eulerRotation.y {
                    duration: 30000
                    from: 0
                    to: 360
                    loops: Animation.Infinite
                    running: true
                }
                PointLight {
                    id: pointLight
                    position: Qt.vector3d(0, 100, 300)
                    brightness: 1
                    ambientColor: Qt.rgba(0.5, 0.3, 0.1, 1.0)
                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: true
                        NumberAnimation {
                            target: pointLight
                            property: "brightness"
                            to: 1
                            duration: 2000
                            easing.type: Easing.OutElastic
                        }
                        NumberAnimation {
                            target: pointLight
                            property: "brightness"
                            to: 2
                            duration: 6000
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                PointLight {
                    id: pointLight2
                    position: Qt.vector3d(100, 100, 300)
                    brightness: 1
                    ambientColor: Qt.rgba(0.5, 0.3, 0.1, 1.0)
                    SequentialAnimation {
                        loops: Animation.Infinite
                        running: true
                        NumberAnimation {
                            target: pointLight
                            property: "brightness"
                            to: 1
                            duration: 2000
                            easing.type: Easing.OutElastic
                        }
                        NumberAnimation {
                            target: pointLight
                            property: "brightness"
                            to: 2
                            duration: 6000
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                Model {
                    instancing: randomInstancing
                    source: "meshes/asteroid.mesh"
                    opacity: 0.5

                    materials: [
                        PrincipledMaterial {
                            baseColor: "#c0e0cc"
                            roughness: 0.1
                            cullMode: Material.NoCulling
                        }
                    ]
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
