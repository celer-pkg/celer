// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick.Window
import "qml"

Rectangle {
    width: 600
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

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

        Component {
            id: sourceItemComponent
            RedFill { }
        }
        Component {
            id: sourceItemComponent2
            AnimatedItem { }
        }

        // Model with dynamically created sourceItem
        Model {
            x: -100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    id: myTexture
                    Component.onCompleted: {
                        const item = sourceItemComponent.createObject(sourceItemContainer);
                        myTexture.sourceItem = item;
                    }
                }
            }
        }

        // Model with dynamically removed sourceItem
        Model {
            x: 100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    id: myTexture2
                    sourceItem: RedFill { }
                    Component.onCompleted: {
                        myTexture2.sourceItem.destroy();
                    }
                }
            }
        }

        // Model with dynamically switched sourceItem
        Model {
            x: -100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    id: myTexture3
                    sourceItem: RedFill { }
                    Component.onCompleted: {
                        const item = sourceItemComponent2.createObject(sourceItemContainer);
                        myTexture3.sourceItem = item;
                    }
                }
            }
        }

        // Model with Loader sourceItem
        Model {
            x: 100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    id: myTexture4
                    sourceItem: Loader {
                        width: 100
                        height: 100
                        source: "qml/AnimatedItem.qml"
                    }
                }
            }
        }
    }

    Item {
        id: sourceItemContainer
        visible: false
    }
}
