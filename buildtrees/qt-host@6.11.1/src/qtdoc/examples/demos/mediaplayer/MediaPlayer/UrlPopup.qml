// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import MediaControls
import Config

Popup {
    id: urlPopup
    anchors.centerIn: Overlay.overlay
    padding: 30
    width: 500
    height: column.height + 60

    property url path: ""
    readonly property color borderColor: urlText.text ? (!errorMsg.visible ? Config.highlightColor : "red") : Config.secondaryColor

    background: Rectangle {
        color: Config.mainColor
        opacity: 0.9
        radius: 15
        border.color: "grey"
    }

    function setUrl(urlPath: url) {
        path = urlPath
        urlPopup.close()
    }

    function validateUrl(urlText: string) : bool {
        const urlPattern = /^([a-z]+:){1,2}\/\/.+/
        return urlPattern.test(urlText)
    }

    Column {
        id: column
        spacing: 20

        Label {
            text: qsTr("Load from URL")
            font.pixelSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
            color: Config.secondaryColor
        }

        ColumnLayout {
            spacing: 0
            TextField {
                id: urlText
                leftPadding: 15
                verticalAlignment: TextInput.AlignVCenter
                font.pixelSize: 16
                placeholderText: qsTr("URL:")
                placeholderTextColor: Config.secondaryColor
                color: Config.secondaryColor
                text: "https://download.qt.io/learning/videos/media-player-example/Qt_LogoMergeEffect.mp4"

                Layout.preferredHeight: 40
                Layout.preferredWidth: 440

                background: Rectangle {
                    color: Config.mainColor
                    border.color: urlPopup.borderColor
                }
            }

            Rectangle {
                id: errorMsg
                visible: false
                color: "#FF3A3A"

                Layout.minimumHeight: 40
                Layout.minimumWidth: 130
                Layout.alignment: Qt.AlignLeft

                Row {
                    anchors.centerIn: parent
                    spacing: 10

                    Image {
                        source: Images.iconSource("Warning_Icon", false)
                    }

                    Label {
                        text: qsTr("Wrong URL")
                        font.pixelSize: 16
                        color: "white"
                    }
                }

                onVisibleChanged: showError.start()

                NumberAnimation {
                    id: showError
                    target: errorMsg
                    properties: "opacity"
                    from: 0
                    to: 1
                    duration: 1000
                }
            }
        }


        RowLayout {
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter

            CustomButton {
                icon.source: ControlImages.iconSource("Cancel_Button", false)
                onClicked: {
                    urlText.text = ""
                    urlPopup.close()
                }
            }

            CustomButton {
                icon.source: ControlImages.iconSource("Load_Button", false)
                enabled: urlText.text
                opacity: urlText.text ? 1 : 0.5
                onClicked: {
                    if (urlPopup.validateUrl(urlText.text)) {
                        urlPopup.setUrl(new URL(urlText.text))
                    } else {
                        errorMsg.visible = true
                    }
                }
            }
        }
    }
    onOpened: urlPopup.forceActiveFocus()
    onClosed: {
        urlText.text = ""
        errorMsg.visible = false
    }
}
