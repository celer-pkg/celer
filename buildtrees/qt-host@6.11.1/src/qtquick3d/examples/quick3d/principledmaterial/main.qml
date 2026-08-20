// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Helpers
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "Principled Materials Example"
    //color: "black"
    color: "#848895"

    Image {
        anchors.fill: parent
        source: "maps/grid.png"
        fillMode: Image.Tile
        horizontalAlignment: Image.AlignLeft
        verticalAlignment: Image.AlignTop
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        Page {
            id: toolPage
            SplitView.fillHeight: true
            SplitView.preferredWidth: 450
            SplitView.minimumWidth: 400
            header: TabBar {
               id: tabBar
               TabButton {
                   text: "Basics"
               }
               TabButton {
                   text: "Alpha"
               }
               TabButton {
                   text: "Details"
               }
               TabButton {
                   text: "Clearcoat"
               }
               TabButton {
                   text: "Refraction"
               }
               TabButton {
                   text: "Special"
               }
               TabButton {
                   text: "Vertex Color"
               }
            }

            StackLayout {
                id: toolPageSwipeView
                anchors.fill: parent
                anchors.margins: 10
                currentIndex: tabBar.currentIndex

                BasicsPane {
                    id: basicsPane
                    principledMaterial: basicMaterial
                    specularGlossyMaterial: specularGlossyMaterial
                }

                AlphaPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                    specularGlossyMode: viewport.specularGlossyMode
                }

                DetailsPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                    view3D: viewport
                }

                ClearcoatPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                }

                RefractionPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                    specularGlossyMode: viewport.specularGlossyMode
                }

                SpecialPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                    linesModel: linesLogo
                    pointsModel: pointsLogo
                    specularGlossyMode: viewport.specularGlossyMode
                }

                VertexColorPane {
                    targetMaterial: viewport.specularGlossyMode ? specularGlossyMaterial : basicMaterial
                    specularGlossyMode: viewport.specularGlossyMode
                }
            }
        }

        View3D {
            id: viewport
            SplitView.fillHeight: true
            SplitView.fillWidth: true
            SplitView.minimumWidth: splitView.width * 0.5
            environment: SceneEnvironment {
                property bool enableEffects: false
                antialiasingMode: SceneEnvironment.MSAA
                antialiasingQuality: SceneEnvironment.High
                lightProbe: Texture {
                    source: "maps/OpenfootageNET_garage-1024.hdr"
                }
                effects: enableEffects ? [bloom, scurveTonemap] : []
                backgroundMode: SceneEnvironment.SkyBox

                SCurveTonemap {
                    id: scurveTonemap
                }
                HDRBloomTonemap {
                    id: bloom
                }
            }
            Node {
                id: originNode
                PerspectiveCamera {
                    id: cameraNode
                    z: 600
                    clipNear: 1
                    clipFar: 10000
                }
            }

            property bool specularGlossyMode: basicsPane.specularGlossyMode

            PrincipledMaterial {
                id: basicMaterial
                baseColor: "white"
            }

            SpecularGlossyMaterial {
                id: specularGlossyMaterial
                property alias baseColor: specularGlossyMaterial.albedoColor
                property real specularAmount: 1.0
                property real specularTint: 1.0
                specularColor: Qt.rgba(0.22, 0.22, 0.22, 1.0)
                albedoColor: "white"
            }

            Model {
                id: cube
                source: "#Cube"
                materials: viewport.specularGlossyMode ? [ specularGlossyMaterial ] : [ basicMaterial ]
                pickable: true
            }

            Model {
                id: sphereModel
                x: -200
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                source: "#Sphere"
                materials: viewport.specularGlossyMode ? [ specularGlossyMaterial ] : [ basicMaterial ]
                pickable: true
            }

            Model {
                id: monkeyMesh
                x: 250
                source: "meshes/suzanne.mesh"
                materials: viewport.specularGlossyMode ? [ specularGlossyMaterial ] : [ basicMaterial ]
                pickable: true
                scale: Qt.vector3d(100, 100, 100)
            }

            Model {
                id: linesLogo
                y: 200
                x: -100
                source: "meshes/logo_lines.mesh"
                materials: viewport.specularGlossyMode ? [ specularGlossyMaterial ] : [ basicMaterial ]
                pickable: true
                visible: false
            }

            Model {
                id: pointsLogo
                y: 200
                x: 100
                source: "meshes/logo_points.mesh"
                materials: viewport.specularGlossyMode ? [ specularGlossyMaterial ] : [ basicMaterial ]
                pickable: true
                visible: false
            }
            //! [resource loader]
            ResourceLoader {
                meshSources: [
                    "meshes/logo_lines.mesh",
                    "meshes/logo_points.mesh"
                ]
            }
            //! [resource loader]

            BackgroundCurtain {
                visible: curtainToggleButton.checked
                y: -150
            }

            OrbitCameraController {
                origin: originNode
                camera: cameraNode
            }

            MouseArea {
                id: pickController
                anchors.fill: parent
                onClicked: function(mouse) {
                    let pickResult = viewport.pick(mouse.x, mouse.y);
                    if (pickResult.objectHit) {
                        let pickedObject = pickResult.objectHit;
                        // Move the camera orbit origin to be the clicked object
                        originNode.position = pickedObject.position
                    }
                }
            }

            Button {
                id: curtainToggleButton
                text: checked ? "Hide Curtain" : "Show Curtain"
                checkable: true
                checked: true
                anchors.top: parent.top
                anchors.right: parent.right
            }

            RowLayout {
                anchors.bottom: parent.bottom
                Label {
                    text: "Drag Background to Orbit Camera, Touch/Click a Model to Center Camera"
                    color: "#dddddd"
                }
            }
        }
    }
}
