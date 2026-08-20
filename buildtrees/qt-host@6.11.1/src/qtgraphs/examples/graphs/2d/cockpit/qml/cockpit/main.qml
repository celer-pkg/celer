// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtGraphs

Item {
    id: mainWindow
    width: 1280
    height: 720

    RowLayout {
        id: rowTop
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 275

        Rectangle {
            Layout.margins: 5
            color: "black"
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 2
            border.color: "white"
            clip: true

            Speedometer {}
        }

        Rectangle {
            Layout.margins: 5
            color: "black"
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 2
            border.color: "white"
            clip: true

            VibrationSensor { id: vibrationSensor }
        }

        Rectangle {
            Layout.margins: 5
            color: "black"
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 2
            border.color: "white"
            clip: true

            ControlPanel { vibrationGraph: vibrationSensor.lineGraph }
        }
    }

    RowLayout {
        id: rowBottom
        anchors.top: rowTop.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Rectangle {
            Layout.margins: 5
            color: "green"
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 2
            border.color: "white"
            clip: true

            Map {}
        }

        Rectangle {
            Layout.margins: 5
            color: "black"
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.width: 2
            border.color: "white"
            clip: true

            ArtificialHorizon {}
        }
    }
}
