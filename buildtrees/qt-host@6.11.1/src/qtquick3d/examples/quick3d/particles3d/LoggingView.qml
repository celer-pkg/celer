// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick3D.Particles3D

Frame {
    id: root
    required property list<ParticleSystem3D> particleSystems
    readonly property bool loggingEnabled: AppSettings.showLoggingView
    property bool intervalInstant: false
    property real itemWidth: (width - loggingButton.width - intervalButton.width) / 7

    width: parent.width
    height: tableContent.height + 30

    Component.onCompleted: {
        for (const psystem of particleSystems)
            psystem.logging = AppSettings.showLoggingView;
    }

    // Background
    background: Rectangle {
        color: "#80000000"
        visible: root.loggingEnabled
    }

    Button {
        id: loggingButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        opacity: root.loggingEnabled ? 1.0 : 0.4
        icon.source: "qrc:/images/icon_logging.png"
        icon.width: 32
        icon.height: 32
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            AppSettings.showLoggingView = !AppSettings.showLoggingView
            for (const psystem of root.particleSystems) {
                psystem.logging = AppSettings.showLoggingView;
            }
        }
    }

    Button {
        id: intervalButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: loggingButton.left
        anchors.rightMargin: 0
        visible: root.loggingEnabled
        opacity: root.intervalInstant ? 1.0 : 0.2
        icon.source: "qrc:/images/icon_interval.png"
        icon.width: 32
        icon.height: 32
        icon.color: "transparent"
        background: Rectangle {
            color: "transparent"
        }
        onClicked: {
            root.intervalInstant = !root.intervalInstant;
            var interval = root.intervalInstant ? 0 : 1000;
            for (const psystem of root.particleSystems)
                psystem.loggingData.loggingInterval = interval;
        }
    }

    Component {
        id: systemComponent
        Row {
            id: systemItem
            required property ParticleSystem3D modelData
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.seed
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.updates
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.particlesMax
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.particlesUsed
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.time.toFixed(4)
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.timeAverage.toFixed(4)
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: systemItem.modelData.loggingData.timeDeviation.toFixed(4)
            }
        }
    }

    Column {
        id: tableContent
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        visible: root.loggingEnabled
        Row {
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("SEED")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("UPDATES")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("P. MAX")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("P. USED")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("TIME")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("TIME AVG.")
            }
            Text {
                width: root.itemWidth
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
                font.pointSize: AppSettings.fontSizeSmall
                text: qsTr("TIME DEV.")
            }
        }
        Repeater {
            model: root.particleSystems
            delegate: systemComponent
        }
    }
}
