// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3D.Xr

Window {
    id: root
    width: 1280
    height: 720
    visible: true

    Text {
        property string desktopApiName:
            (GraphicsInfo.api === GraphicsInfo.OpenGL) ? "OpenGL" :
            (GraphicsInfo.api === GraphicsInfo.Direct3D11) ? "D3D11" :
            (GraphicsInfo.api === GraphicsInfo.Direct3D12) ? "D3D12" :
            (GraphicsInfo.api === GraphicsInfo.Vulkan) ? "Vulkan" :
            (GraphicsInfo.api === GraphicsInfo.Metal) ? "Metal" :
                "Unknown API"

        text: "This is an ordinary QQuickWindow rendering with " + desktopApiName + ". Click the button to instantiate an XrView as well."
    }

    ColumnLayout {
        x: 100
        y: 100
        Rectangle {
            color: "red"
            width: 200
            height: 200
            NumberAnimation on rotation { from: 0; to: 360; duration: 5000; loops: -1 }
        }
        Some3DScene {
            id: v3d
        }
    }

    DebugView {
        source: v3d
        anchors.right: parent.right
    }

    property XrView xrView: null
    property string errorString: ""

    Connections {
        target: root.xrView
        function onInitializeFailed(errorString) {
            root.errorString = errorString;
            root.xrView = null;
        }
        function onSessionEnded() {
            root.xrView.destroy();
            root.xrView = null;
        }
        function onSessionEndRequested() {
            root.xrView.destroy();
            root.xrView = null;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        Text {
            text: root.xrView !== null ?
                "XrView initialized; running on " + root.xrView.runtimeInfo.runtimeName + " " + root.xrView.runtimeInfo.runtimeVersion + " with " + root.xrView.runtimeInfo.graphicsApiName
                : root.errorString
        }

        Button {
            id: btnStart
            visible: root.xrView === null
            text: "Start VR session"
            onClicked: {
                var component = Qt.createComponent("VrScene.qml");
                root.xrView = component.createObject();
            }
        }

        Button {
            id: btnStop
            visible: root.xrView !== null
            text: "Stop VR session"
            onClicked: {
                root.xrView.destroy();
                root.xrView = null;
            }
        }
    }
}
