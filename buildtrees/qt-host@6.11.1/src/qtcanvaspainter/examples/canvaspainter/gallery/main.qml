// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import GalleryExample

Window {
    id: mainView

    readonly property real dp: height / 600
    property real animationTime: 0
    property real animationSine: 0
    property bool animatePainting: true

    width: 1280
    height: 720
    visible: true
    color: "#222831"

    NumberAnimation on animationTime {
        id: animationTimeAnimation
        running: mainView.animatePainting
        from: 0
        to: 360
        duration: 1000*360
        loops: Animation.Infinite
    }
    SequentialAnimation on animationSine {
        running: mainView.animatePainting
        loops: Animation.Infinite
        NumberAnimation {
            from: 0
            to: 1
            duration: 1000
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            from: 1
            to: 0
            duration: 1000
            easing.type: Easing.InOutSine
        }
    }

    TopBar {
        id: topBar
        width: parent.width
        height: Math.floor(60 * mainView.dp)
        currentIndex: listView.currentIndex
        itemCount: listView.count
        visibilityState: (listView.visibleArea.xPosition * (listView.delegateWidth * listView.count + listView.width)) / listView.delegateWidth
    }

/*
    // This can be used for autoscrolling gallery to see memory usage
    // while (un)loading items
    SequentialAnimation {
        running: true
        loops: Animation.Infinite
        ScriptAction {
            script: listView.currentIndex = listView.count-1;
        }
        PauseAnimation {
            duration: 5000
        }
        ScriptAction {
            script: listView.currentIndex = 0;
        }
        PauseAnimation {
            duration: 5000
        }
    }
*/

    ListView {
        id: listView
        readonly property real delegateWidth: height * 0.75
        anchors.top: topBar.bottom
        anchors.topMargin: Math.floor(20 * mainView.dp)
        anchors.bottom: parent.bottom
        width: parent.width
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        maximumFlickVelocity: 10000
        highlightMoveDuration: 2500
        model: 15
        // Disable this to not preload all views.
        cacheBuffer: 10000
        preferredHighlightBegin: width * 0.5 - delegateWidth * 0.5
        preferredHighlightEnd: preferredHighlightBegin + 1
        Component.onCompleted: currentIndex = 0;
        delegate: GalleryItem {
            id: galleryItemDelegate
            required property int index
            readonly property bool animationsOn: index >= listView.currentIndex - 1 &&
                                                 index <= listView.currentIndex + 1
            width: listView.delegateWidth
            height: listView.height
            animationTime: animationsOn ? mainView.animationTime : 0
            animationSine: animationsOn ? mainView.animationSine : 0
            animState: Math.max(0, (1.0 - Math.abs(topBar.visibilityState - index)))
            galleryView: index
            fillColor: "transparent"
            alphaBlending: true
        }
    }
}
