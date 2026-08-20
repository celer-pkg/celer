// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 600
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    Timer {
        interval: 100; running: true; repeat: false
        onTriggered: {
            // B) Start with sourceItem, set source
            textureB.sourceItem = null
            textureB.source = "qml/qtlogo.png"
            textureB.flipV = true
            textureB.scaleU = 1.5
            textureB.scaleV = 1.5
            textureB.tilingModeHorizontal = Texture.MirroredRepeat
            textureB.tilingModeVertical = Texture.ClampToEdge

            // C) Start with source, set sourceItem
            textureC.sourceItem = redRectangle
            textureC.source = ""

            // D) Start empty, set source
            textureD.source = "qml/qtlogo.png"
        }
    }

    Rectangle {
        width: 1
        height: 1
        color: "red"
        id: redRectangle
    }

    Rectangle {
        width: 100
        height: 100
        color: "green"
        id: hiddenGreenRectangle
        visible: false
    }

    // A) Both source and sourceItem
    Texture {
        id: textureA
        source: "qml/qtlogo.png"
        sourceItem: redRectangle
    }

    // B) With sourceItem
    Texture {
        id: textureB
        sourceItem: redRectangle
    }

    // C) With source
    Texture {
        id: textureC
        source: "qml/qtlogo.png"
    }

    // D) Empty
    Texture {
        id: textureD
    }

    // E) With hidden sourceItem
    Texture {
        id: textureE
        sourceItem: hiddenGreenRectangle
    }

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 350)
            clipFar: 5000
        }

        DirectionalLight {
        }

        Model {
            x: -100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: textureA
            }
        }

        Model {
            x: 100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: textureB
            }
        }

        Model {
            x: -100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: textureC
            }
        }

        Model {
            x: 100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: textureD
            }
        }

        Model {
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: textureE
            }
        }
    }
}
