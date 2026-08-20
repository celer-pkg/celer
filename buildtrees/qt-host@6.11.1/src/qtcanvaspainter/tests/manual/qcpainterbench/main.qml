// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import "qml"

Window {
    id: mainWindow

    property real animationTime: 0
    // Cutting corners a bit and not using enum
    // 1=ruler, 2=circles, 4=lines, 8=bars, 16=icons, 32=flower
    property int enabledTests: 31
    // Amount of items (QQuickCPainterItem / QQuickPaintedItem) to render
    property int itemCount: 1
    // How many times selected tests are rendered per item
    property int testCount: 1
    property bool fullScreen: true
    // 0 = QCPainter, 1 = QQuickPaintedItem
    property int renderType: 0
    // Resolution-independent dp
    readonly property real dp: Screen.pixelDensity * 25.4/160

    property bool settingAntialiasing: true
    property bool settingHighQualityRendering: false
    property bool settingFBORendering: false
    property bool settingAnimateSize: false
    property bool settingShowDebug: false
    property string settingQCBackendName: "(unknown)"

    property real itemHeight: graphContainer.height
    property var qnItem: null

    function updateDebugItem() {
        // Debug will be shown for the first item.
        // Enable QCPAINTER_DEBUG_COLLECT to see this.
        qnItem = itemRepeater.itemAt(0).children[0];
    }

    function getRhiApiName(api) {
        if (api === GraphicsInfo.OpenGL)
            return "OpenGL on QRhi";
        else if (api === GraphicsInfo.Direct3D11)
            return "D3D11 on QRhi";
        else if (api === GraphicsInfo.Direct3D12)
            return "D3D12 on QRhi";
        else if (api === GraphicsInfo.Vulkan)
            return "Vulkan on QRhi";
        else if (api === GraphicsInfo.Metal)
            return "Metal on QRhi";
        else if (api === GraphicsInfo.Null)
            return "Null on QRhi";
        else
            return "Unknown API";
    }

    Component.onCompleted: {
        updateDebugItem();
    }

    width: 375
    height: 667
    visible: true
    minimumWidth: 400*dp

    color: "#404040"

    NumberAnimation on animationTime {
        id: animationTimeAnimation
        from: 0
        to: 360
        duration: 1000*360
        loops: Animation.Infinite
    }

    Item {
        id: graphContainer
        width: mainWindow.fullScreen ? parent.width : 256
        height: mainWindow.fullScreen ? parent.height - fpsItem.height : 256
        y: mainWindow.fullScreen ? fpsItem.height : (parent.height - height)/2
        x: mainWindow.fullScreen ? 0 : (parent.width - width)/2

        Component.onCompleted: {
            // Note: GraphicsInfo is only available inside items.
            mainWindow.settingQCBackendName = getRhiApiName(GraphicsInfo.api);
        }

        Repeater {
            id: itemRepeater
            model: mainWindow.itemCount
            onModelChanged: {
                if (itemRepeater.count > 0)
                    updateDebugItem();
            }

            Item {
                id: sizeAnimatedContainer
                readonly property real itemMargin: (mainWindow.width / (40 + mainWindow.itemCount*2))
                readonly property real itemPos: (index * itemMargin)
                readonly property real itemPosCentered: (index * itemMargin) - (itemRepeater.count/2 * itemMargin)
                // Move each item slightly to see the item amount
                x: mainWindow.fullScreen ? itemPos : itemPosCentered
                y: mainWindow.fullScreen ? itemPos : itemPosCentered
                width: parent.width
                height: mainWindow.itemHeight

                DemoQCPainterItem {
                    id: qnItem
                    width: parent.width
                    height: visible ? parent.height : 0
                    enabledTests: mainWindow.enabledTests
                    testCount: mainWindow.testCount
                    animationTime: visible ? mainWindow.animationTime : 0
                    visible: renderType === 0
                    fillColor: "transparent"
                    alphaBlending: true
                    antialiasing: mainWindow.settingAntialiasing
                    highQualityRendering: mainWindow.settingHighQualityRendering
                }

                DemoQPItem {
                    id: qpItem
                    width: parent.width
                    height: visible ? parent.height : 0
                    enabledTests: mainWindow.enabledTests
                    testCount: mainWindow.testCount
                    animationTime: visible ? mainWindow.animationTime : 0
                    visible: renderType === 1
                    qpAntialiasing: mainWindow.settingAntialiasing
                    qpRenderTargetFBO: mainWindow.settingFBORendering
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            animationTimeAnimation.paused = !animationTimeAnimation.paused
        }
    }

    DebugView {
        id: debugView
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10 * dp
        anchors.left: parent.left
        anchors.leftMargin: 10 * dp
        visible: settingShowDebug
    }

    SettingsView {
        id: settigsView
    }

    FpsItem {
        id: fpsItem
    }

    Button {
        anchors.verticalCenter: fpsItem.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 60 * dp
        model: ["Canvas Painter", "QPainter"]
        onCurrentIndexChanged: {
            renderType = currentIndex;
        }
    }

    SequentialAnimation {
        id: sizeAnimation
        loops: Animation.Infinite
        running: mainWindow.settingAnimateSize
        NumberAnimation  {
            target: mainWindow
            property: "itemHeight"
            from: graphContainer.height
            to: graphContainer.height * 0.5
            duration: 1000
            easing.type: Easing.InOutQuad
        }
        NumberAnimation  {
            target: mainWindow
            property: "itemHeight"
            from: graphContainer.height * 0.5
            to: graphContainer.height
            duration: 1000
            easing.type: Easing.InOutQuad
        }
    }
}
