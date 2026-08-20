// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Effects

Item {
    // Height, width and any other size related properties containing odd looking float or other dividers, multipliers, etc.
    // that do not seem to have any logical origin are just arbitrary and based on original design
    // and/or personal preference on what looks nice.
    property string coffeeName: ""
    id: root
    property alias confirmButton: confirmButton
    property alias rectangle: rectangle
    property alias coffeeSlider: coffeeSlider
    property alias milkSlider: milkSlider
    property alias foamSlider: foamSlider
    property alias sugarSlider: sugarSlider
    property alias box: box
    property alias sugarText: sugarText
    property alias handle: handle
    property int coffeeAmount
    property int milkAmount
    property int foamAmount
    property double sugarAmount

    states: [
        State {
            name: "portrait"
            PropertyChanges {
                target: grid
                flow: GridLayout.TopToBottom
                rows: 3
                columns: 1
                rowSpacing: 20
            }
            PropertyChanges {
                target: cup
                Layout.row: 0
                Layout.preferredWidth: height / 1.16
                Layout.preferredHeight: root.height / 3
            }
            PropertyChanges {
                target: rectangle
                Layout.row: 1
                Layout.preferredWidth: root.width / 1.12
                Layout.preferredHeight: root.height / 2.5
            }
            PropertyChanges {
                target: confirmButton
                Layout.alignment: Qt.AlignHCenter
                Layout.row: 2
                Layout.preferredWidth: root.width / 2.2
                Layout.preferredHeight: root.height / 14
            }
        },
        State {
            name: "landscape"
            PropertyChanges {
                target: grid
                flow: GridLayout.LeftToRight
                rows: 2
                columns: 3
            }
            PropertyChanges {
                target: cup
                Layout.column: 0
                Layout.preferredWidth: root.width / 4
                Layout.preferredHeight: root.height / 5
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            }

            PropertyChanges {
                target: rectangle
                Layout.preferredWidth: root.width / 2
                Layout.preferredHeight: root.height / 1.4
                Layout.leftMargin: root.width / 20
                Layout.alignment: Qt.AlignBottom
                Layout.column: 2
            }
            PropertyChanges {
                target: confirmButton
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.row: 1
                Layout.columnSpan: 3
                Layout.preferredWidth: root.width / 4
                Layout.preferredHeight: root.height / 8
            }
        }
    ]
    GridLayout {
        id: grid
        flow: GridLayout.TopToBottom
        anchors.horizontalCenter: parent.horizontalCenter

        Cup {
            id: cup
            milkAmount: milkSlider.value
            coffeeAmount: coffeeSlider.value
            foamAmount: foamSlider.value
            sugarAmount: sugarSlider.position
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        }

        Rectangle {
            id: rectangle
            radius: 8
            gradient: Colors.greyBorder
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            property int textPixelSize: 16
            Rectangle {
                id: rectangle2
                radius: 8
                color: Colors.currentTheme.cardColor
                width: parent.width - 2
                height: parent.height - 2
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            Layout.preferredWidth: parent.width / 4
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            Text {
                                id: text1
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Coffee"
                                color: Colors.currentTheme.textColor
                                font.pixelSize: rectangle.textPixelSize
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            CustomSlider {
                                id: coffeeSlider
                                from: 0
                                to: 100
                                value: root.coffeeAmount
                                liquidAmount: value
                                width: parent.width
                                height: 10
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            Layout.preferredWidth: parent.width / 4
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            Text {
                                id: text2
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Milk"
                                color: Colors.currentTheme.textColor
                                font.pixelSize: rectangle.textPixelSize
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            CustomSlider {
                                id: milkSlider
                                from: 0
                                to: 60
                                value: root.milkAmount
                                liquidAmount: value
                                width: parent.width
                                height: 10
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            Layout.preferredWidth: parent.width / 4
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            Text {
                                id: text3
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Foam"
                                color: Colors.currentTheme.textColor
                                font.pixelSize: rectangle.textPixelSize
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            CustomSlider {
                                id: foamSlider
                                from: 0
                                to: 60
                                value: root.foamAmount
                                liquidAmount: value
                                width: parent.width
                                height: 10
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            Layout.preferredWidth: parent.width / 4
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            Text {
                                id: text4
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: "Sugar"
                                color: Colors.currentTheme.textColor
                                font.pixelSize: rectangle.textPixelSize
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Colors.currentTheme.cardColor
                            Slider {
                                id: sugarSlider
                                snapMode: Slider.SnapAlways
                                stepSize: 0.25
                                value: root.sugarAmount
                                width: parent.width
                                height: 10
                                anchors.verticalCenter: parent.verticalCenter
                                transitions: Transition {
                                    NumberAnimation {
                                        properties: "scale"
                                        duration: 10
                                        easing.type: Easing.InOutQuad
                                    }
                                }
                                background: Rectangle {
                                    x: sugarSlider.leftPadding
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: sugarSlider.availableWidth
                                    height: 4
                                    radius: 2
                                    color: Colors.currentTheme.background

                                    Rectangle {
                                        id: firstBar
                                        width: parent.width / 4
                                        height: parent.height
                                        color: (sugarSlider.value > 0) ? Colors.green : Colors.grey
                                        radius: 2
                                    }

                                    Rectangle {
                                        id: secondBar
                                        anchors.left: firstBar.right
                                        width: parent.width / 4
                                        height: parent.height
                                        color: (sugarSlider.value
                                                > 0.25) ? Colors.green : Colors.grey
                                        radius: 2
                                    }

                                    Rectangle {
                                        id: thirdBar
                                        anchors.left: secondBar.right
                                        width: parent.width / 4
                                        height: parent.height
                                        color: (sugarSlider.value
                                                > 0.50) ? Colors.green : Colors.grey
                                        radius: 2
                                    }

                                    Rectangle {
                                        id: fourthBar
                                        anchors.right: parent.right
                                        width: parent.width / 4
                                        height: parent.height
                                        color: (sugarSlider.value
                                                > 0.75) ? Colors.green : Colors.grey
                                        radius: 2
                                    }
                                }
                                handle: Rectangle {
                                    id: handle
                                    x: sugarSlider.leftPadding + sugarSlider.visualPosition
                                       * (sugarSlider.availableWidth - width)
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 14
                                    height: width
                                    radius: 100
                                    color: Colors.green
                                    Image {
                                        id: sugarMark
                                        source: "./images/icons/Polygon.svg"
                                        anchors.bottom: parent.top
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottomMargin: 1
                                    }

                                    Rectangle {
                                        id: box
                                        anchors.bottom: sugarMark.top
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottomMargin: 1
                                        implicitWidth: sugarText.width + 8
                                        implicitHeight: sugarText.height + 4
                                        radius: 4
                                        color: Colors.currentTheme.background
                                        border.color: Colors.grey
                                        Text {
                                            id: sugarText
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            text: (sugarSlider.position * 4) + "p"
                                            font.pixelSize: 12
                                            clip: false
                                            color: Colors.currentTheme.textColor
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            MultiEffect {
                source: rectangle2
                anchors.fill: rectangle2
                shadowEnabled: true
                shadowColor: Colors.shadow
                shadowOpacity: 0.5
            }
        }
        CustomButton {
            id: confirmButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.minimumWidth: 150
            Layout.minimumHeight: 40
            buttonText: "Start"
            showIcon: false
        }
    }
}
