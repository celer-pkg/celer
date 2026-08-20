// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtMultimedia

Item {
    id: root

    required property int selectedAudioTrack
    required property int selectedVideoTrack
    required property int selectedSubtitleTrack
    required property MediaPlayer mediaPlayer

    Flickable {
        anchors.fill: parent
        contentWidth: column.implicitWidth
        contentHeight: column.implicitHeight
        boundsBehavior: Flickable.DragAndOvershootBounds
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Column {
            id: column
            anchors.fill: parent
            clip: true
            padding: 15
            spacing: 20

            TracksOptions {
                id: audioTracks
                headerText: qsTr("Audio Tracks")
                selectedTrack: root.selectedAudioTrack
                metaData: root.mediaPlayer.audioTracks
                onSelectedTrackChanged: root.mediaPlayer.activeAudioTrack = audioTracks.selectedTrack
            }

            TracksOptions {
                id: videoTracks
                headerText: qsTr("Video Tracks")
                selectedTrack: root.selectedVideoTrack
                metaData: root.mediaPlayer.videoTracks
                onSelectedTrackChanged: root.mediaPlayer.activeVideoTrack = videoTracks.selectedTrack
            }

            TracksOptions {
                id: subtitleTracks
                headerText: qsTr("Subtitle Tracks")
                selectedTrack: root.selectedSubtitleTrack
                metaData: root.mediaPlayer.subtitleTracks
                onSelectedTrackChanged: root.mediaPlayer.activeSubtitleTrack = subtitleTracks.selectedTrack
            }
        }
    }
}
