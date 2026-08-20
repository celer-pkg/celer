// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Component.onCompleted: {
            // At the end there should be 5 red and 5 blue items
            for (var i = 0; i < 8; ++i)
                shapeSpawner.addItem("TEXT " + i, "red");
            for (var i = 0; i < 3; ++i)
                shapeSpawner.removeItem();
            for (var i = 0; i < 15; ++i)
                shapeSpawner.addItem("TEXT " + i, "blue");
            for (var i = 0; i < 10; ++i)
                shapeSpawner.removeItem();
        }

        Node {
            id: shapeSpawner
            property real range: 300
            property var instances: []

            function addItem(textLabel, textColor) {
                var yPos = range - (2 * (0.1 * instances.length) * range);
                var itemComponent = Qt.createComponent("Component.qml");
                let instance = itemComponent.createObject(
                        shapeSpawner, { "y": yPos, "textLabel": textLabel, "textColor": textColor});
                instances.push(instance);
            }
            function removeItem() {
                let instance = instances.pop();
                instance.destroy();
            }
        }

        Node {
            y: 200
            x: -200
            Loader {
                anchors.fill: parent
                sourceComponent: rect
            }
        }
        Node {
            y: 0
            x: -200
            Loader {
                id: loaderItem
            }
            Component.onCompleted: {
                loaderItem.sourceComponent = rect;
            }
        }
        Node {
            y: -200
            x: -200
            Loader {
                id: loaderItem2
                sourceComponent: rect
            }
            Component.onCompleted: {
                // Remove component, so shouldn't be visible
                loaderItem2.sourceComponent = undefined;
            }
        }

        Component {
            id: rect
            Rectangle {
                width: 200
                height: 150
                color: "green"
                border.color: "white"
                border.width: 5
                Text {
                    anchors.centerIn: parent
                    color: "white"
                    font.pixelSize: 20
                    text: "Loader Component"
                }
            }
        }
    }
}
