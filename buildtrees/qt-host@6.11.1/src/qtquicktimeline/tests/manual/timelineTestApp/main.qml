// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Timeline 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Loader {
        id: loader
        anchors.fill: parent
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        spacing: 4
        Text {
            text: "Test 01"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test01.qml"
            }
        }

        Text {
            text: "Test 02"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test02.qml"
            }
        }

        Text {
            text: "Test 03"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test03.qml"
            }
        }

        Text {
            text: "Test 04"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test04.qml"
            }
        }

        Text {
            text: "Test 05"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test05.qml"
            }
        }

        Text {
            text: "Test 06"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test06.qml"
            }
        }

        Text {
            text: "Test 07"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test07.qml"
            }
        }

        Text {
            text: "Test 08"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test08.qml"
            }
        }

        Text {
            text: "Test 09"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test09.qml"
            }
        }

        Text {
            text: "Test 10"
            font.pixelSize: 12
            MouseArea {
                anchors.fill: parent
                onClicked: loader.source = "test10.qml"
            }
        }
    }
}
