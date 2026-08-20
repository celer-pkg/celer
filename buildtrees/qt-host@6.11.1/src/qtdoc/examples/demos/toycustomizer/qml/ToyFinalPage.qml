// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    readonly property bool ready: (bearImage.status === Image.Ready) ||
                                  (dialogBackgroundImage.status === Image.Ready)

    signal orderReviewRequested
    signal newOrderRequested

    ColumnLayout {
        id: layout
        spacing: 32
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: ApplicationConfig.responsiveSize(80)
        }

        state: root.width < layout.width ? "narrow" : ""
        states: State {
            name: "narrow"
            AnchorChanges {
                target: layout
                anchors {
                    left: root.left
                    horizontalCenter: undefined
                }
            }
        }

        ToyImage {
            sourceSize.height: 128
            source: "images/appLogo.svg"
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        ToyImage {
            sourceSize.height: 32
            source: "images/builtWithQt.svg"
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        ToyLabel {
            color: "#2269EE"
            text: qsTr("Thanks for your order!")
            font {
                bold: true
                family: "DynaPuff"
                pixelSize: ApplicationConfig.responsiveSize(120)
            }
            Layout.alignment: Qt.AlignCenter
        }

        Item {
            id: bearDialogItem
            implicitWidth: bearImage.implicitWidth + bearImage.leftMargin
            visible: root.ready
            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                id: bearImage
                readonly property int leftMargin: 96
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                source: "images/teddyBear.png"
                sourceClipRect: Qt.rect(90, 110, 540, 650)
                scale: 0.8
                anchors {
                    left: parent.left
                    leftMargin: bearImage.leftMargin
                }
            }

            Item {
                implicitHeight: dialogBackgroundImage.implicitHeight
                implicitWidth: dialogBackgroundImage.implicitWidth
                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: 56
                }

                ToyImage {
                    id: dialogBackgroundImage
                    asynchronous: true
                    showBusyIndicator: false
                    source: "images/dialogBackground.svg"
                    sourceSize {
                        height: 256
                        width: 384
                    }
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 16
                    rowSpacing: 24
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        topMargin: 80
                    }

                    ToyLabel {
                        color: "#162655"
                        text: qsTr("Your order has been confirmed!")
                        textStyle: ApplicationConfig.TextStyle.H2
                        font.bold: false
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignCenter
                    }

                    ToyButton {
                        type: ToyButton.Type.Secondary
                        text: qsTr("View order")
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        onClicked: root.orderReviewRequested()
                    }

                    ToyButton {
                        text: qsTr("New order")
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        onClicked: root.newOrderRequested()
                    }
                }
            }
        }

        Item {
            implicitWidth: 2
            visible: !root.ready
            Layout.fillHeight: true
        }
    }
}
