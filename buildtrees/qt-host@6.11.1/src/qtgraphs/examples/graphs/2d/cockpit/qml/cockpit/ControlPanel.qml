// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Item {
    id: root
    anchors.fill: parent

    property LineSeries vibrationGraph

    Rectangle {
        color: "black"
        height: root.height * 0.1
        anchors.left: parent.left
        anchors.right: parent.right
        border.color: "white"
        border.width: 2
        z: 1

        Text {
            color: "white"
            antialiasing: true
            font.bold: true
            font.family: "Akshar"
            font.pointSize: 16
            anchors.centerIn: parent
            text: "VIBRATION GRAPH PANEL"
        }
    }

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Dial {
                anchors.fill: parent
                value: 0.5
                onMoved: {
                    vibrationGraph.amplitude = value
                }
            }

            Text {
                color: "black"
                antialiasing: true
                font.bold: false
                font.family: "Akshar"
                font.pointSize: 6
                anchors.centerIn: parent
                text: "AMPLITUDE"
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Dial {
                anchors.fill: parent
                value: 0.5
                onMoved: {
                    vibrationGraph.resolution = value
                }
            }

            Text {
                color: "black"
                antialiasing: true
                font.bold: false
                font.family: "Akshar"
                font.pointSize: 6
                anchors.centerIn: parent
                text: "FREQUENCY"
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Dial {
                anchors.fill: parent
                value: 500
                from: 1
                to: 500
                stepSize: 1
                onMoved: {
                    vibrationGraph.change(value)
                }
            }

            Text {
                color: "black"
                antialiasing: true
                font.bold: false
                font.family: "Akshar"
                font.pointSize: 6
                anchors.centerIn: parent
                text: "RESOLUTION"
            }
        }
    }
}
