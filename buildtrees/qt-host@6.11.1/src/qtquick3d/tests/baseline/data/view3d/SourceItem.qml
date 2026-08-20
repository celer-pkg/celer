// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    width: 460
    height: 460
    Node {
        id: sceneRootRed
        Model {
            source: "#Cube"
            materials: [ PrincipledMaterial {
                    baseColor: "red"
                    lighting: PrincipledMaterial.NoLighting
                }
            ]

            // NumberAnimation on eulerRotation.y {
            //     from: 0
            //     to: 360
            //     running: true
            //     duration: 5000
            //     loops: -1
            // }
        }
    }

    Node {
        id: sceneRootBlue
        Model {
            source: "#Cube"
            materials: [ PrincipledMaterial {
                    baseColor: "blue"
                    lighting: PrincipledMaterial.NoLighting
                }
            ]

            // NumberAnimation on eulerRotation.y {
            //     from: 0
            //     to: 360
            //     running: true
            //     duration: 5000
            //     loops: -1
            // }
        }
    }

    View3D {
        id: rootView
        anchors.fill: parent
        renderMode: View3D.Underlay
        camera: PerspectiveCamera {
            z: 300
        }
        View3D {
            id: sourceViewLeft
            visible: false // The texture should be available as a sourceItem even if the view's visibility is false
            width: 512
            height: 512
            anchors.centerIn: parent
            importScene: sceneRootBlue
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Color
                clearColor: "red"
            }
            camera: PerspectiveCamera {
                z: 300
            }
        }

        Node { // Check that that this works when the view under a node
            View3D {
                id: sourceViewRight
                visible: false
                width: 512
                height: 512
                anchors.centerIn: parent
                importScene: sceneRootRed
                environment: SceneEnvironment {
                    backgroundMode: SceneEnvironment.Color
                    clearColor: "blue"
                }
                camera: PerspectiveCamera {
                    z: 300
                }
            }
        }

        Model {
            id: leftRect
            position: Qt.vector3d(-50, 0, 0)
            source: "#Rectangle"
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: Texture {
                        sourceItem: sourceViewLeft
                    }
                }
            ]
        }

        Model {
            id: rightRect
            position: Qt.vector3d(50, 0, 0)
            source: "#Rectangle"
            materials: [ PrincipledMaterial {
                    lighting: PrincipledMaterial.NoLighting
                    baseColorMap: Texture {
                        sourceItem: sourceViewRight
                    }
                }
            ]
        }
    }
}
