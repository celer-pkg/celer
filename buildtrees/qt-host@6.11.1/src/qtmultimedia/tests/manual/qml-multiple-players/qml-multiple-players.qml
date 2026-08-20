// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Window {
    id: root
    height: 1000
    width: 960
    color: "#000000"
    visible: true

    property var sources: ["qrc:/testvideo1.mp4", "qrc:/testvideo2.mp4"]
    // property var sources: ["file:///path/to/file1.mp4", "file:///path/to/file2.mp4"]

    property int videoOutputHeight: 40
    property int videoOutputWidth: 300

    property int symbolPointSize: 22
    property int textPointSize: 16
    property int buttonHeight: 40

    property bool playing: true
    property bool looping: true
    property bool audioMuted: false
    property bool videoVisible: true

    ListView {
        id: list
        width: parent.width
        height: parent.height - globalControls.height
        clip: true
        spacing: 5
        ScrollBar.vertical: ScrollBar { active: true }

        model: playerModel
        delegate: playerDelegate
    }

    ListModel {
        id: playerModel

        ListElement {
            index: 0
        }
    }

    Component {
        id: playerDelegate

        RowLayout {
            id: listItem
            required property int index

            Component.onCompleted: {
                playerModel.set(listItem.index, { "player": player })
                player.play()
                audioOutput.muted = root.audioMuted
            }

            Label {
                Layout.preferredHeight: 40
                Layout.preferredWidth: 80
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: textPointSize
                color: "#FFFFFF"

                text: "Player " + (listItem.index + 1)
            }

            Button {
                implicitHeight: buttonHeight
                font.pointSize: textPointSize
                implicitWidth: 90

                text: audioOutput.muted ? "Unmute" : "Mute"
                onClicked: {
                    audioOutput.muted = !audioOutput.muted
                }
            }

            Button {
                implicitHeight: buttonHeight
                font.pointSize: textPointSize
                implicitWidth: 80

                text: videoOutput.visible ? "Hide" : "Show"
                onClicked: videoOutput.visible = videoOutput.visible ? false : true
            }

            Button {
                implicitHeight: buttonHeight
                font.pointSize: symbolPointSize
                implicitWidth: 60

                text: player.playing ? "\u23f8" : "\u23f5" // pause / play
                onClicked: player.playing ? player.pause() : player.play()
            }

            Button {
                implicitHeight: buttonHeight
                font.pointSize: symbolPointSize

                text: "\u23f9" // stop
                onClicked: player.stop()
            }

            Button {
                implicitHeight: buttonHeight
                font.pointSize: symbolPointSize

                text: "\u23ed" // skip to next source
                onClicked: {
                    player.sourceIndex += 1
                    player.play()
                }
            }

            VideoOutput {
                id: videoOutput
                Layout.preferredWidth: videoOutputWidth
                Layout.preferredHeight: videoOutputHeight
                visible: true
            }

            AudioOutput {
                id: audioOutput
            }

            MediaPlayer {
                id: player
                videoOutput: videoOutput
                audioOutput: audioOutput
                loops: root.looping ? MediaPlayer.Infinite : 1
                property int sourceIndex: 0
                source: root.sources[sourceIndex % root.sources.length]

                onErrorOccurred: {
                    console.log("Error occurred, status:", player.errorString);
                }
                onSourceChanged: {
                    console.log("Source changed to " + source)
                }
            }
        }
    }

    RowLayout {
        id: globalControls
        height: 60
        anchors.bottom: parent.bottom

        Label {
            Layout.preferredHeight: 40
            Layout.preferredWidth: 80
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: textPointSize
            color: "#FFFFFF"

            text: "Global"
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: textPointSize
            implicitWidth: 90

            text: root.audioMuted ? "Unmute" : "Mute"
            onClicked: {
                root.audioMuted = !root.audioMuted
                for (var i = 0; i < playerModel.count; i++) {
                    playerModel.get(i).player.audioOutput.muted = root.audioMuted
                 }

            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: textPointSize
            implicitWidth: 80

            text: root.videoVisible ? "Hide" : "Show"
            onClicked: {
                root.videoVisible = !root.videoVisible
                for (var i = 0; i < playerModel.count; i++) {
                    playerModel.get(i).player.videoOutput.visible = root.videoVisible
                }
            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: root.symbolPointSize
            implicitWidth: 60

            text: root.playing ? "\u23f8" : "\u23f5" // pause / play
            onClicked: {
                root.playing = !root.playing
                for (var i = 0; i < playerModel.count; i++) {
                    if (root.playing) {
                        playerModel.get(i).player.play()
                    } else {
                        playerModel.get(i).player.pause()
                    }

                }
            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: root.symbolPointSize

            text: "\u23f9" // stop
            onClicked: {
                for (var i = 0; i < playerModel.count; i++) {
                    playerModel.get(i).player.stop()
                }
            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: root.symbolPointSize

            text: "\u23ed" // skip to next source
            onClicked: {
                for (var i = 0; i < playerModel.count; i++) {
                    var p = playerModel.get(i).player
                    p.sourceIndex += 1
                    p.play()
                }
            }
        }

        Button {
            implicitHeight: buttonHeight
            implicitWidth: 70
            font.pointSize: root.symbolPointSize

            text: root.looping ? "!\u221E" : "\u221E" // looping: !infinite / infinite
            onClicked: {
                root.looping = !root.looping
            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: textPointSize

            text: "Add player " + (playerModel.count + 1)
            onClicked: {
                playerModel.append({"index": playerModel.count})
            }
        }

        Button {
            implicitHeight: buttonHeight
            font.pointSize: textPointSize

            text: "Remove player"
            onClicked: {
                if (playerModel.count > 0) {
                    playerModel.remove(playerModel.count - 1)
                }
            }
        }
    }
}
