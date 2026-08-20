// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects
import QtQuick.Controls.Basic

Item {
    id: rootItem

    property bool show: true
    property real showAnimation: show ? 1 : 0

    width: settings.settingsViewWidth
    x: -(width + 30) * (1 - showAnimation) + 20

    function resetPosition() {
        let lw = lightMouseArea.width
        let lh = lightMouseArea.height
        let sizeValue = sizeSlider.value - sizeSlider.from;
        let sizeRange = sizeSlider.to - sizeSlider.from;
        let lightX = lw * 0.4 - lw * 0.2 * (sizeValue) / (sizeRange)
        let lightY = lh * 0.4 - lh * 0.2 * (sizeValue) / (sizeRange)
        updatePosition(lightX, lightY)
    }

    function updatePosition(posX, posY) {
        let margin = 20;
        let halfW = (lightMouseArea.width - margin * 2) * 0.5;
        let halfH = (lightMouseArea.height - margin * 2) * 0.5;
        posX = Math.max(margin, posX);
        posY = Math.max(margin, posY);
        posX = Math.min(lightMouseArea.width - margin, posX);
        posY = Math.min(lightMouseArea.height - margin, posY);
        let dX = (posX - margin - halfW) / (halfW);
        let dY = (posY - margin - halfH) / (halfH);
        settings.offsetX = dX * 30;
        settings.offsetY = dY * 30;
        lightItem.x = posX;
        lightItem.y = posY;
    }

    Behavior on showAnimation {
        NumberAnimation {
            duration: 400
            easing.type: Easing.InOutQuad
        }
    }

    // Open/close button
    Item {
        width: 30 * dp
        height: 30 * dp
        anchors.left: parent.right
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: -10
        Rectangle {
            anchors.fill: parent
            color: Qt.lighter(mainWindow.mainColor, 0.8)
            radius: 4
        }
        Image {
            anchors.centerIn: parent
            source: "images/arrow.png"
            rotation: rootItem.showAnimation * 180
        }
        MouseArea {
            anchors.fill: parent
            anchors.margins: -30 * dp
            onClicked: {
                rootItem.show = !rootItem.show;
            }
        }
    }

    // Background
    Rectangle {
        anchors.fill: scrollView
        opacity: showAnimation ? 1 : 0
        visible: opacity
        anchors.margins: -10
        color: Qt.lighter(mainWindow.mainColor, 0.15)
        radius: 4
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.interactive: false
        clip: true
        Column {
            id: settingsArea
            anchors.fill: parent
            opacity: showAnimation
            visible: opacity
            spacing: 10 * dp
            SettingsComponentSlider {
                id: sizeSlider
                text: qsTr("Size") + ": " + value.toFixed()
                value: settings.itemSize
                from: 100
                to: 300
                onMoved: {
                    settings.itemSize = value
                    settings.radius = Math.min(settings.radius, radiusSlider.to)
                    settings.spread = Math.min(settings.spread, spreadSlider.to)
                    settings.spread = Math.max(settings.spread, spreadSlider.from)
                    settings.blur = value * 0.2
                    resetPosition();
                }
            }
            SettingsComponentSlider {
                id: radiusSlider
                text: qsTr("Radius") + ": " + value.toFixed()
                value: settings.radius
                from: 0
                to: settings.itemSize * 0.5
                onMoved: {
                    settings.radius = value
                }
            }
            SettingsComponentCheckBox {
                id: individualRadiusCheckBox
                text: qsTr("Individual corner radius")
                checked: settings.individualRadius
                onCheckedChanged: {
                    settings.individualRadius = checked;
                }
            }
            SettingsComponentSlider {
                id: radiusTRSlider
                text: qsTr("Radius Top-Right") + ": " + value.toFixed()
                enabled: individualRadiusCheckBox.checked
                value: settings.radiustr
                from: 0
                to: settings.itemSize * 0.5
                onMoved: {
                    settings.radiustr = value
                }
            }
            SettingsComponentSlider {
                id: radiusBRSlider
                text: qsTr("Radius Bottom-Right") + ": " + value.toFixed()
                enabled: individualRadiusCheckBox.checked
                value: settings.radiusbr
                from: 0
                to: settings.itemSize * 0.5
                onMoved: {
                    settings.radiusbr = value
                }
            }
            SettingsComponentSlider {
                id: radiusBLSlider
                text: qsTr("Radius Bottom-Left") + ": " + value.toFixed()
                enabled: individualRadiusCheckBox.checked
                value: settings.radiusbl
                from: 0
                to: settings.itemSize * 0.5
                onMoved: {
                    settings.radiusbl = value
                }
            }
            SettingsComponentSlider {
                id: radiusTLSlider
                text: qsTr("Radius Top-Left") + ": " + value.toFixed()
                enabled: individualRadiusCheckBox.checked
                value: settings.radiustl
                from: 0
                to: settings.itemSize * 0.5
                onMoved: {
                    settings.radiustl = value
                }
            }
            SettingsComponentSlider {
                text: qsTr("Blur") + ": " + value.toFixed()
                value: settings.blur
                from: 0
                to: 100
                onMoved: {
                    settings.blur = value;
                }
            }
            SettingsComponentSlider {
                text: qsTr("Opacity") + ": " + value.toFixed(2)
                value: settings.opacity
                from: 0.0
                to: 1.0
                onMoved: {
                    settings.opacity = value;
                }
            }
            SettingsComponentSlider {
                id: spreadSlider
                text: qsTr("Spread") + ": " + value.toFixed(2)
                value: settings.spread
                from: -settings.itemSize * 0.1
                to: settings.itemSize * 0.1
                onMoved: {
                    settings.spread = value;
                }
            }
        }
    }
    Row {
        anchors.horizontalCenter: scrollView.horizontalCenter
        anchors.bottom: resetButton.top
        anchors.bottomMargin: 20 * dp
        spacing: 20 * dp
        SettingsComponentButton {
            text: "DEBUG"
            checkable: true
            checked: settings.showDebug
            onCheckedChanged: settings.showDebug = checked
        }
        SettingsComponentButton {
            text: "CUSTOM"
            checkable: true
            checked: settings.showCustomMaterial
            onCheckedChanged: settings.showCustomMaterial = checked
        }
    }
    SettingsComponentButton {
        id: resetButton
        anchors.horizontalCenter: scrollView.horizontalCenter
        anchors.bottom: scrollView.bottom
        anchors.bottomMargin: 20 * dp
        text: "RESET"
        onClicked: settings.resetSettings()
    }
}
