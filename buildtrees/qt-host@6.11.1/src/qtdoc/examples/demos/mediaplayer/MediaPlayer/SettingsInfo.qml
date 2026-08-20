// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Fusion
import QtMultimedia
import Config

Rectangle {
    id: root
    implicitWidth: 380
    color: Config.mainColor
    border.color: "lightgrey"
    radius: 10

    property alias tracksInfo: tracksInfo
    property alias metadataInfo: metadataInfo
    required property MediaPlayer mediaPlayer
    required property int selectedAudioTrack
    required property int selectedVideoTrack
    required property int selectedSubtitleTrack

    MouseArea {
        anchors.fill: root
        preventStealing: true
    }

    TabBar {
        id: bar
        width: root.width
        contentHeight: 60

        Repeater {
            model: [qsTr("Metadata"), qsTr("Tracks"), qsTr("Theme")]

            TabButton {
                id: tab
                required property int index
                required property string modelData
                property color shadowColor:  bar.currentIndex === index ? Config.highlightColor : "black"
                property color textColor:  bar.currentIndex === index ? Config.highlightColor : Config.secondaryColor

                background: Rectangle {
                    opacity: 0.15
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 0.5; color: "transparent" }
                        GradientStop { position: 1.0; color: tab.shadowColor }
                    }
                }

                contentItem: Label {
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: tab.modelData
                    font.pixelSize: 20
                    color: tab.textColor
                }
            }
        }
    }

    StackLayout {
        width: root.width
        anchors.top: bar.bottom
        anchors.bottom: root.bottom
        currentIndex: bar.currentIndex

        MetadataInfo { id: metadataInfo }

        TracksInfo {
            id: tracksInfo
            mediaPlayer: root.mediaPlayer
            selectedAudioTrack: root.selectedAudioTrack
            selectedVideoTrack: root.selectedVideoTrack
            selectedSubtitleTrack: root.selectedSubtitleTrack
        }

        ThemeInfo { id: themeInfo }
    }
}
