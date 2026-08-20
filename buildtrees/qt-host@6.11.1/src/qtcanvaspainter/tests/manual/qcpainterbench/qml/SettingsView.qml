// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: root

    property bool isOpen: false

    width: parent.width
    height: topBar.height + contentArea.height

    Rectangle {
        id: topBar
        width: parent.width
        height: fpsItem.height
        color: "#000000"
    }

    Item {
        id: contentArea
        anchors.right: parent.right
        anchors.rightMargin: 8 * dp
        anchors.top: topBar.bottom
        anchors.topMargin: 8 * dp
        width: Math.min(parent.width - 16*dp, 450*dp)
        height: tabView.height + backgroundItem.height
        visible: root.isOpen
        Rectangle {
            id: backgroundItem
            anchors.fill: tabView.currentIndex == 0 ? contentItemTab1 : tabView.currentIndex == 1 ? contentItemTab2 : contentItemTab3
            color: "#000000"
            opacity: 0.75
        }
        TabView {
            id: tabView
            anchors.top: parent.top
            texts: ["TESTS", "VIEW", "ADVANCED"]
        }
        Column {
            id: contentItemTab1
            anchors.top: tabView.bottom
            width: parent.width
            visible: tabView.currentIndex == 0
            Switch {
                text: "Render ruler"
                checked: (enabledTests & 1)
                onUserChecked: {
                    enabledTests ^= 1;
                }
            }
            Switch {
                text: "Render circles"
                checked: (enabledTests & 2)
                onUserChecked: {
                    enabledTests ^= 2;
                }
            }
            Switch {
                text: "Render bezier lines"
                checked: (enabledTests & 4)
                onUserChecked: {
                    enabledTests ^= 4;
                }
            }
            Switch {
                text: "Render bars"
                checked: (enabledTests & 8)
                onUserChecked: {
                    enabledTests ^= 8;
                }
            }
            Switch {
                text: "Render icons&text"
                checked: (enabledTests & 16)
                onUserChecked: {
                    enabledTests ^= 16;
                }
            }
            Switch {
                text: "Render flower"
                checked: (enabledTests & 32)
                onUserChecked: {
                    enabledTests ^= 32;
                }
            }
        }
        Column {
            id: contentItemTab2
            anchors.top: tabView.bottom
            width: parent.width
            visible: tabView.currentIndex == 1
            Switch {
                text: "Use 256x256px item"
                checked: false
                onCheckedChanged: {
                    mainWindow.fullScreen = !checked;
                    if (sizeAnimation.running) {
                        sizeAnimation.restart();
                    }
                }
            }
            Switch {
                text: "Animate item size"
                checked: mainWindow.settingAnimateSize
                onCheckedChanged: {
                    mainWindow.settingAnimateSize = checked;
                }
            }
            SliderSelector {
                id: itemCountSelector
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 40 * dp
                title: "Item count"
                texts: ["1", "2", "4", "8", "16", "32", "64", "128", "256", "512"]
                selectedIndex: 0
                onSelectedIndexChanged: {
                    mainWindow.itemCount = itemCountSelector.texts[selectedIndex];
                }
            }
            SliderSelector {
                id: testCountSelector
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 40 * dp
                title: "Rendering count (per item)"
                texts: ["1", "2", "4", "8", "16", "32", "64", "128", "256", "512"]
                selectedIndex: 0
                onSelectedIndexChanged: {
                    mainWindow.testCount = testCountSelector.texts[selectedIndex];
                }
            }
        }
        Column {
            id: contentItemTab3
            anchors.top: tabView.bottom
            width: parent.width
            visible: tabView.currentIndex == 2
            Switch {
                text: "Antialiasing"
                checked: mainWindow.settingAntialiasing
                onCheckedChanged: {
                    mainWindow.settingAntialiasing = checked;
                }
            }
            Switch {
                text: "High quality rendering (QCPainter)"
                checked: mainWindow.settingHighQualityRendering
                onCheckedChanged: {
                    mainWindow.settingHighQualityRendering = checked;
                }
            }
            Switch {
                id: showDebugSwitch
                text: "Show Debug (QCPainter)"
                checked: mainWindow.settingShowDebug
                onCheckedChanged: {
                    mainWindow.settingShowDebug = checked;
                }
            }
            Switch {
                text: "OpenGL FBO renderTarget (QPainter)"
                enabled: GraphicsInfo.api === GraphicsInfo.OpenGL
                checked: mainWindow.settingFBORendering
                onCheckedChanged: {
                    mainWindow.settingFBORendering = checked;
                }
            }
            Text {
                anchors.right: parent.right
                anchors.rightMargin: 32 * dp
                font.pixelSize: 16 * dp
                color: "#ffffff"
                text: "QCPainter backend: <b>" + mainWindow.settingQCBackendName + "</b>"
            }
            Item {
                width: 1
                height: 16 * dp
            }
        }
    }

    Image {
        anchors.horizontalCenter: parent.right
        anchors.horizontalCenterOffset: -30 * dp
        anchors.top: parent.top
        anchors.topMargin: 8 * dp
        width: 32 * dp
        height: 32 * dp
        source: root.isOpen ? "images/ic_cancel_white_48dp.png" : "images/ic_settings_white_48dp.png"
        smooth: true
        MouseArea {
            anchors.fill: parent
            anchors.margins: -16 * dp
            onClicked: {
                root.isOpen = !root.isOpen;
            }
        }
    }

}

