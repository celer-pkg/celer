// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HelloItem

ApplicationWindow {
    id: w1
    visible: true
    width: 480
    height: 800

    Item {
        id: w1root
        anchors.fill: parent

        Item {
            id: content
            anchors.fill: parent

            Text {
                id: textItem
                anchors.horizontalCenter: parent.horizontalCenter
                y: parent.height * 0.02
                font.pixelSize: parent.width * 0.04
                color: "purple"
                text: "Here is our custom QCanvasPainter item"
            }

            Rectangle {
                color: "red"
                width: 100
                height: 100
                NumberAnimation on rotation { from: 0; to: 360; duration: 5000; loops: -1 }
                anchors.top: textItem.bottom

            }

            Rectangle {
                color: "transparent"
                border.color: "green"
                border.width: 4
                anchors.top: textItem.bottom
                anchors.bottom: parent.bottom
                width: parent.width

                HelloItem {
                    sampleCount: cbMsaa.checked ? 4 : 1
                    aa: aaSlider.value
                    anchors.fill: parent
                    alphaBlending: cbTransparentBackground.checked
                    fillColor: cbTransparentBackground.checked ? "transparent" : "white"
                }
            }

            Pane {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: parent.width / 2
                anchors.margins: 4

                ColumnLayout {
                    anchors.fill: parent
                    CheckBox {
                        id: cbTransparentBackground
                        text: "Transparent background"
                        checked: true
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "AA amount"
                        }
                        Text {
                            text: aaSlider.value.toFixed(1)
                            Layout.minimumWidth: 25
                        }
                        Slider {
                            Layout.fillWidth: true
                            id: aaSlider
                            from: 0.0
                            to: 10.0
                            value: 1.0
                        }
                    }
                    CheckBox {
                        id: cbMsaa
                        text: "MSAA"
                        checked: false
                    }
                    Button {
                        text: "Reparent to new window"
                        onClicked: {
                            if (w2.visible) {
                                content.parent = w1root
                                w2.visible = false
                            } else {
                                content.parent = w2root
                                w2.visible = true
                            }
                        }
                    }
                }
            }
        }
    }

    ApplicationWindow {
        id: w2
        width: 600
        height: 400
        visible: false

        Item {
            id: w2root
            anchors.fill: parent
        }
    }
}
