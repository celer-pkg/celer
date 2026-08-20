// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Item {
    Component.onCompleted: {
        console.log("Keys:\n[L] - toggle lighting\n[N] - add a new point light\n[R] - remove a point light\n[T] - toggle directional light");
    }

    id: root

    Text {
        color: "#ffffff"
        style: Text.Outline
        styleColor: "#606060"
        font.pixelSize: 28
        property int api: GraphicsInfo.api
        text: {
            if (GraphicsInfo.api === GraphicsInfo.OpenGL)
                "OpenGL";
            else if (GraphicsInfo.api === GraphicsInfo.Software)
                "Software";
            else if (GraphicsInfo.api === GraphicsInfo.Direct3D12)
                "D3D12";
            else if (GraphicsInfo.api === GraphicsInfo.OpenVG)
                "OpenVG";
            else if (GraphicsInfo.api === GraphicsInfo.OpenGLRhi)
                "OpenGL via QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.Direct3D11Rhi)
                "D3D11 via QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.VulkanRhi)
                "Vulkan via QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.MetalRhi)
                "Metal via QRhi";
            else if (GraphicsInfo.api === GraphicsInfo.Null)
                "Null via QRhi";
            else
                "Unknown API";
        }
    }

    Image {
        source: "qt_logo.png"
        x: 50
        NumberAnimation on y { from: 50; to: 400; duration: 3000; loops: -1 }
    }

    property bool useLighting: false;
    property var lights: []

    focus: true
    Keys.onPressed: {
        if (event.key === Qt.Key_L) {
            useLighting = !useLighting;
        } else if (event.key === Qt.Key_N && lights.length + 2 <= 7) {
            var colors = [ "yellow", "red", "green", "blue", "purple", "magenta" ];
            var lightColor = colors[lights.length];
            console.log("Adding new point light: " + (lights.length + 1) + " color: " + lightColor);
            var component = Qt.createComponent("qrc:/SomePointLight.qml");
            var newLight = component.createObject(pointLightContainer, { color: lightColor });
            lights.push(newLight);
        } else if (event.key === Qt.Key_R && lights.length > 0) {
            console.log("Removing last point light. Remaining: " + (lights.length - 1));
            var dyingLight = lights.pop();
            dyingLight.destroy();
        } else if (event.key === Qt.Key_T) {
            console.log("Toggling visibility of directional light: " + !dirLight.visible);
            dirLight.visible = !dirLight.visible
        }
    }

    Rectangle {
        color: "transparent"
        border.color: "lightGray"
        border.width: 2
        anchors.fill: parent

        transform: [
            Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: v3d.width / 2; origin.y: v3d.height / 2; },
            Translate { id: txOut; x: -v3d.width / 2; y: -v3d.height / 2 },
            Scale { id: scale; },
            Translate { id: txIn; x: v3d.width / 2; y: v3d.height / 2 }
        ]

        SequentialAnimation {
            PauseAnimation { duration: 2000 }
            ParallelAnimation {
                NumberAnimation { target: scale; property: "xScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
                NumberAnimation { target: scale; property: "yScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
            }
            NumberAnimation { target: rotation; property: "angle"; to: 80; duration: 1000; easing.type: Easing.InOutCubic }
            NumberAnimation { target: rotation; property: "angle"; to: -80; duration: 1000; easing.type: Easing.InOutCubic }
            NumberAnimation { target: rotation; property: "angle"; to: 0; duration: 1000; easing.type: Easing.InOutCubic }
            PauseAnimation { duration: 1000 }
            ParallelAnimation {
                NumberAnimation { target: scale; property: "xScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
                NumberAnimation { target: scale; property: "yScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
            }
            running: true
            loops: Animation.Infinite
        }

        View3D {
            id: v3d
            anchors.fill: parent
            camera: camera
            renderMode: View3D.Offscreen

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 200, 300)
                eulerRotation: Qt.vector3d(-30, 0, 0)
            }
            Model {
                source: "#Cube"
                materials: [ DefaultMaterial {
                        lighting: root.useLighting ? DefaultMaterial.FragmentLighting : DefaultMaterial.NoLighting
                    } ]
                eulerRotation: Qt.vector3d(0, -90, 0)
                PropertyAnimation on eulerRotation.y { from: 0; to: -360; duration: 5000; loops: -1 }
            }
            Model {
                source: "teapot.mesh"
                x: 100
                y: 100
                scale: Qt.vector3d(15, 15, 15)
                materials: [ DefaultMaterial {
                        lighting: root.useLighting ? DefaultMaterial.FragmentLighting : DefaultMaterial.NoLighting
                        diffuseColor: "salmon"
                    } ]
                PropertyAnimation on eulerRotation.y { from: 0; to: 360; duration: 2000; loops: -1 }
                PropertyAnimation on eulerRotation.z { from: 0; to: 360; duration: 2000; loops: -1 }
            }
            DirectionalLight {
                id: dirLight
                ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0);
            }
            Node {
                id: pointLightContainer
            }
        }
    }
}
