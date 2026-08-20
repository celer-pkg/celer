// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import PocketDemo

Window {
    id: mainWindow
    width: Constants.width
    height: Constants.height

    visibility: Qt.platform.os === 'android' ? Window.FullScreen : Window.AutomaticVisibility

    visible: true
    title: "FX & Material Showroom"

    function downloadProgress(progress) {
        progressBar.value = progress
    }

    function downloadStart(num, total) {
        progressText.text = "Downloading asset " + num + " of " + total;
    }

    function downloadComplete() {
        assetsLoader.visible = false
        loader.sourceComponent = screen01Component
    }

    Rectangle {
        id:bg
        color: "black"
        anchors.fill: parent

        Loader {
            id: loader
            anchors.centerIn: parent
        }

        Component {
            id: screen01Component

            Screen01 {
                id: mainScreen

                transform: Scale {
                    origin.x: mainScreen.width / 2
                    origin.y: mainScreen.height / 2
                    readonly property real xRatio: bg.width / mainScreen.width
                    readonly property real yRatio: bg.height / mainScreen.height
                    xScale: Math.min(xRatio, yRatio)
                    yScale: Math.min(xRatio, yRatio)
                }
            }
        }

        Rectangle {
            id: assetsLoader

            color: "#222222"
            anchors.fill: parent

            Rectangle {
                color: "#444444"
                radius: 4
                anchors.centerIn: parent
                width: column.width + 100
                height: column.height + 100

                Column {
                    id: column
                    anchors.centerIn: parent
                    spacing: 10

                    Row {
                        spacing: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        BusyIndicator {
                            id: busyIndicator
                            palette.dark: "white"
                            width: 50
                            height: 50

                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            color: "#ffffff"
                            text: qsTr("Downloading Assets...")
                            font.pixelSize: 30
                            font.styleName: "Bold"

                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Item { width: 1; height: 30 } // spacer

                    Text {
                        id: progressText
                        color: "#ffffff"
                        font.pixelSize: 16
                        anchors.left: progressBar.left
                    }

                    ProgressBar {
                        id: progressBar

                        height: 10
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }
}
