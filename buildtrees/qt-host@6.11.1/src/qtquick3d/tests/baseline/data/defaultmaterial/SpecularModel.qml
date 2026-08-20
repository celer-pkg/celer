// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    id: defaultmaterial_screen

    color: "white"

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 800)
            clipFar: 5000
        }

        DirectionalLight {
        }

        Node {
            x: -250
            y: 0
            Model {
                id: model
                position: Qt.vector3d(0, 0, 0)
                scale: Qt.vector3d(4,4,4)
                source: "#Sphere"
                materials: DefaultMaterial {
                    specularModel: DefaultMaterial.Default
                    diffuseColor: Qt.rgba(1.0, 0.0, 0.0, 1)
                    specularTint: Qt.rgba(1.0, 0.0, 0.0, 1)
                    specularAmount: 1.0
                    fresnelPower: 5.0
                    specularRoughness: 1.0
                }
            }
            Node {
                y: 200
                z: 200
                Rectangle {
                    color: "white"
                    width: textItem.width + 20
                    height: textItem.height + 10
                }
                Text {
                    id: textItem
                    font.pixelSize: 20
                    color: "black"
                    text: "specularModel: Default"
                }
            }
        }
        Node {
            x: 250
            y: 0
            Model {
                position: Qt.vector3d(0, 0, 0)
                scale: Qt.vector3d(4,4,4)
                source: "#Sphere"
                materials: DefaultMaterial {
                    specularModel: DefaultMaterial.KGGX
                    diffuseColor: Qt.rgba(1.0, 0.0, 0.0, 1)
                    specularTint: Qt.rgba(1.0, 0.0, 0.0, 1)
                    specularAmount: 1.0
                    fresnelPower: 5.0
                    specularRoughness: 1.0
                }
            }
            Node {
                y: 200
                z: 200
                Rectangle {
                    color: "white"
                    width: textItem.width + 20
                    height: textItem.height + 10
                }
                Text {
                    id: textItem2
                    font.pixelSize: 20
                    color: "black"
                    text: "specularModel: KGGX"
                }
            }
        }
    }
}
