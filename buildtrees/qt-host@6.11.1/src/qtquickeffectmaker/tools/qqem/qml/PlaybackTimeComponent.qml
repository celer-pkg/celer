// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Item {
    id: rootItem

    property real showFpsAnimated: fpsHelper.enabled

    property bool scrubbing: false
    property real scrubTime: 0
    property int scrubFrame: 0
    property int scrubStartFrame: 0
    property int scrubStartX: 0

    function reset() {
        scrubTime = 0;
        scrubFrame = 0;
    }

    anchors.verticalCenter: parent.verticalCenter
    height: parent.height
    width: Math.max(fpsTextItem.width + 20, timeTextItem.width + 20)

    Behavior on showFpsAnimated {
        NumberAnimation {
            duration: 400
            easing.type: Easing.InOutQuad
        }
    }
    Column {
        id: fpsTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: -60 * rootItem.showFpsAnimated
        height: parent.height
        spacing: 0
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: mainView.foregroundColor2
            font.pixelSize: 14
            text: fpsHelper.fps.toFixed(1)
            opacity: rootItem.showFpsAnimated
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: mainView.foregroundColor1
            font.pixelSize: 14
            font.bold: true
            text: "FPS"
            opacity: rootItem.showFpsAnimated
        }
    }
    Text {
        id: timeTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -10
        color: mainView.foregroundColor2
        font.pixelSize: 14
        text: effectPreview.animatedTime >= 100 ? effectPreview.animatedTime.toFixed(1) + " s" : effectPreview.animatedTime.toFixed(3) + " s"
    }
    Text {
        id: frameTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 10
        color: mainView.foregroundColor1
        font.pixelSize: 14
        font.bold: true
        text: (effectPreview.animatedFrame).toString().padStart(6, '0');
    }
    Rectangle {
        z: -1
        anchors.fill: parent
        anchors.margins: -2
        color: "#000000"
        border.width: 2
        border.color: mainView.highlightColor
        antialiasing: true
        radius: 5
        opacity: rootItem.scrubbing * 0.5
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            scrubStartX = mouseX;
            scrubStartFrame = scrubFrame;
            rootItem.scrubbing = true;
            previewAnimationRunning = false;
        }
        onReleased: {
            rootItem.scrubbing = false;
        }
        onPositionChanged: {
            let pannedFrame = scrubStartFrame + (mouseX - scrubStartX);
            let pannedTime = (1 / 60) * pannedFrame;
            // Don't scrub below time < 0 or frame < 0
            pannedFrame = Math.max(pannedFrame, -previewFrameTimer.currentFrame);
            pannedTime = Math.max(pannedTime, -previewFrameTimer.elapsedTime);
            rootItem.scrubFrame = pannedFrame;
            rootItem.scrubTime = pannedTime;
        }
    }
}

