// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "Principled Materials Example"
    color: "#848895"

    MaterialControl {
        id: materialCtrl
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Underlay

        //! [rotating light]
        // Rotate the light direction
        DirectionalLight {
            eulerRotation.y: -100
            SequentialAnimation on eulerRotation.y {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    to: 360
                    from: 0
                }
            }
        }
        //! [rotating light]

        //! [environment]
        environment: SceneEnvironment {
            probeExposure: 2.5
            clearColor: window.color

            backgroundMode: SceneEnvironment.Color
            lightProbe: Texture {
                source: "maps/OpenfootageNET_garage-1024.hdr"
            }
        }
        //! [environment]

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        //! [basic principled]
        Model {
            position: Qt.vector3d(-250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ PrincipledMaterial {
                    baseColor: "#41cd52"
                    metalness: materialCtrl.metalness
                    roughness: materialCtrl.roughness
                    specularAmount: materialCtrl.specular
                    specularTint: materialCtrl.specularTint
                    opacity: materialCtrl.opacityValue
                }
            ]
        }
        //! [basic principled]

        //! [textured principled]
        Model {
            position: Qt.vector3d(250, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: [ PrincipledMaterial {
                    metalness: materialCtrl.metalness
                    roughness: materialCtrl.roughness
                    specularAmount: materialCtrl.specular
                    opacity: materialCtrl.opacityValue
                    Texture {
                        id: basemetal
                        source: "maps/metallic/basemetal.astc"
                    }
                    Texture {
                        id: normalrough
                        source: "maps/metallic/normalrough.astc"
                    }
                    baseColorMap: basemetal
                    metalnessMap: basemetal
                    roughnessMap: normalrough
                    normalMap: normalrough
                    metalnessChannel: Material.A
                    roughnessChannel: Material.A
                }
            ]
            //! [textured principled]

            SequentialAnimation on eulerRotation {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 360, 360)
                }
            }
        }
    }
}
