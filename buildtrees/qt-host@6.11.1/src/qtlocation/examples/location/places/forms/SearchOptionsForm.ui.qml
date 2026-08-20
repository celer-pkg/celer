// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    property alias clearButton: clearButton
    property alias setButton: setButton
    property alias cancelButton: cancelButton
    property alias tabTitle: tabTitle
    property alias orderGroup: orderGroup
    property alias distanceOrderButton: distanceOrderButton
    property alias nameOrderButton: nameOrderButton
    property alias unspecifiedButton: unspecifiedButton
    property alias favoritesButton: favoritesButton
    property alias locales: locales

    Rectangle {
        id: tabRectangle
        y: 20
        height: tabTitle.height * 2
        color: "#46a2da"
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        anchors.left: parent.left
        anchors.right: parent.right

        Label {
            id: tabTitle
            color: "#ffffff"
            text: qsTr("Search Options")
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Item {
        id: item2
        anchors.rightMargin: 20
        anchors.leftMargin: 20
        anchors.bottomMargin: 20
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: tabRectangle.bottom

        GridLayout {
            id: gridLayout3
            anchors.rightMargin: 0
            anchors.bottomMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 0
            rowSpacing: 10
            rows: 1
            columns: 2
            anchors.fill: parent

            Label {
                id: label
                text: qsTr("Locale(s)")
                visible: locales.visible
            }

            TextField {
                id: locales
                Layout.fillWidth: true
                placeholderText: qsTr("")
            }

            RadioButton {
                id: favoritesButton
                text: qsTr("Enable favorites")
                Layout.columnSpan: 2
            }

            ButtonGroup { id: orderGroup }

            RadioButton {
                id: distanceOrderButton
                text: qsTr("Order by distance")
                ButtonGroup.group: orderGroup
                Layout.columnSpan: 2
            }

            RadioButton {
                id: nameOrderButton
                text: qsTr("Order by name")
                ButtonGroup.group: orderGroup
                Layout.columnSpan: 2
            }

            RadioButton {
                id: unspecifiedButton
                text: qsTr("Unspecified")
                ButtonGroup.group: orderGroup
                Layout.columnSpan: 2
            }

            RowLayout {
                id: rowLayout1
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignRight

                Button {
                    id: setButton
                    text: qsTr("Set")
                }

                Button {
                    id: clearButton
                    text: qsTr("Clear")
                }

                Button {
                    id: cancelButton
                    text: qsTr("Cancel")
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.columnSpan: 2
            }
        }
    }
}
