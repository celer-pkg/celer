// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QQEMLib 1.0

Item {
    id: rootItem

    property alias outputEditorView: outputEditorView
    property alias effectPreview: effectPreview

    function showHelp() {
        tabBarViews.currentIndex = 2;
    }
    function showCodePreview() {
        tabBarViews.currentIndex = 1;
    }

    function renderToImage(filename) {
        effectPreview.renderToImage(filename);
    }

    FpsHelper {
        id: fpsHelper
        enabled: previewAnimationRunning
    }

    TabBar {
        id: tabBarViews
        currentIndex: 0
        TabButton {
            text: "Preview"
            width: 100
        }
        TabButton {
            text: "Code"
            width: 100
        }
        TabButton {
            text: "Help"
            width: 100
        }
    }
    Rectangle {
        anchors.fill: toolbarRow
        color: mainView.backgroundColor1
        opacity: 0.8
    }
    Row {
        id: toolbarRow
        anchors.right: parent.right
        anchors.verticalCenter: tabBarViews.verticalCenter
        height: tabBarViews.height * 0.8
        PlaybackTimeComponent {
            id: playbackTimeComponent
        }
        Item {
            width: 10
            height: 1
        }
        CustomIconButton {
            id: resetTimeButton
            height: parent.height
            width: height
            icon: "images/icon_restart.png"
            description: "Restart time"
            onClicked: {
                // Reset the time to 0
                previewFrameTimer.reset();
                playbackTimeComponent.reset();
            }
        }
        CustomIconButton {
            id: playPauseButton
            height: parent.height
            width: height
            icon: previewAnimationRunning ? "images/icon_pause.png" : "images/icon_play.png"
            description: "Play / Pause time"
            onClicked: {
                previewAnimationRunning = !previewAnimationRunning;
            }
        }
    }

    EffectPreview {
        id: effectPreview
        anchors.top: tabBarViews.bottom
        anchors.bottom: statusBar.top
        width: parent.width
        FrameAnimation {
            id: previewFrameTimer
            running: true
            paused: !previewAnimationRunning
        }
    }
    EditorView {
        id: outputEditorView
        anchors.top: tabBarViews.bottom
        anchors.bottom: statusBar.top
        width: parent.width
        editable: false
        visible: tabBarViews.currentIndex == 1
    }
    HelpView {
        id: helpView
        anchors.top: tabBarViews.bottom
        anchors.bottom: statusBar.top
        width: parent.width
        visible: tabBarViews.currentIndex == 2
    }

    StatusBar {
        id: statusBar
    }
}
