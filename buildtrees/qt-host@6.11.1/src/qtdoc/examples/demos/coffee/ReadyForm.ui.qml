// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Effects
import QtQuick.Layouts

Item {
    // Height, width and any other size related properties containing odd looking float or other dividers
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    id: root
    property alias grid: grid
    property alias caption: caption
    property int coffeeAmount
    property int milkAmount
    property int foamAmount
    property double sugarAmount

    states: [
        State {
            name: "portrait"
            PropertyChanges {
                target: cup
                Layout.preferredWidth: height / 1.16
                Layout.preferredHeight: root.height / 3
            }
            PropertyChanges {
                target: dialog
                Layout.preferredWidth: root.width / 1.12
                Layout.preferredHeight: root.height / 7
            }
        },
        State {
            name: "landscape"
            PropertyChanges {
                target: cup
                Layout.alignment: Qt.AlignCenter | Qt.AlignTop
                Layout.preferredWidth: root.height / 3
                Layout.preferredHeight: root.height / 2.5
            }
            PropertyChanges {
                target: dialog
                Layout.preferredWidth: root.width / 2.5
                Layout.preferredHeight: root.height / 5
            }
        }
    ]
    GridLayout {
        id: grid
        flow: GridLayout.TopToBottom
        anchors.horizontalCenter: root.horizontalCenter
        rowSpacing: 20
        Cup {
            id: cup
            coffeeAmount: root.coffeeAmount
            milkAmount: root.milkAmount
            foamAmount: root.foamAmount
            sugarAmount: root.sugarAmount
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        Rectangle {
            id: dialog
            gradient: Colors.greyBorder
            radius: 8
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.minimumHeight: 30
            Layout.minimumWidth: 200
            Rectangle {
                id: rectangle
                width: parent.width - 2
                height: parent.height - 2
                radius: 8
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: Colors.currentTheme.cardColor
                Text {
                    id: caption
                    text: "Your coffee is ready."
                    color: Colors.currentTheme.textColor
                    font.pixelSize: 18
                    font.weight: 600
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            MultiEffect {
                source: rectangle
                anchors.fill: rectangle
                shadowEnabled: true
                shadowColor: Colors.shadow
                shadowOpacity: 0.5
            }
        }
        Image {
            id: circle
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            source: (Colors.currentTheme == Colors.dark) ? "./images/icons/ellipse_dark.svg" : "./images/icons/ellipse_light.svg"
            Image {
                id: check
                source: "./images/icons/check.svg"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
