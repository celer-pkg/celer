// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: defaultmaterial_screen

    property var blendModesModel: [
        DefaultMaterial.SourceOver,
        DefaultMaterial.Screen,
        DefaultMaterial.Multiply
    ]
    property int itemsWidth: 800
    property int itemsHeight: 400
    property int modelSeparation: itemsWidth / (blendModesModel.length - 1)

    width: 800
    height: 480
    color: "white"

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 800)
            clipFar: 5000
        }

        DirectionalLight {
        }

        Repeater3D {
            model: blendModesModel.length
            Node {
                x: -itemsWidth / 2 + index * modelSeparation
                y: -itemsHeight / 2 + index % 2 * itemsHeight
                BlendComponent {
                    id: modeComponent
                    blendMode: blendModesModel[index]
                }
                Node {
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
                        text: "blendMode:" + modeComponent.blendMode
                    }
                }
            }
        }
    }
}
