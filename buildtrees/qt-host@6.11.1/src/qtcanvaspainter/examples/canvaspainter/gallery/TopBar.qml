// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2015 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick 2.0

Item {
    id: root

    property real dp: height / 60
    property int currentIndex: 0
    property real visibilityState: 0
    property int itemCount: 0
    property string groupTitle
    property string title

    onCurrentIndexChanged: {
        switch (root.currentIndex) {
        case 0:
            groupTitle = "Brushes";
            title = "Gradients and Image Patterns";
            break;
        case 1:
            groupTitle = "Brushes";
            title = "Images";
            break;
        case 2:
            groupTitle = "Brushes";
            title = "Grid Patterns";
            break;
        case 3:
            groupTitle = "Brushes";
            title = "Shadows";
            break;
        case 4:
            groupTitle = "Brushes";
            title = "Custom Brushes";
            break;
        case 5:
            groupTitle = "Painting";
            title = "Paths, Caps & Joins";
            break;
        case 6:
            groupTitle = "Painting";
            title = "Painter Paths";
            break;
        case 7:
            groupTitle = "Painting";
            title = "States, Transitions and Clipping";
            break;
        case 8:
            groupTitle = "Painting";
            title = "Antialiasing and Line Width";
            break;
        case 9:
            groupTitle = "Painting";
            title = "Composite Modes";
            break;
        case 10:
            groupTitle = "Painting";
            title = "Color Effects";
            break;
        case 11:
            groupTitle = "Text";
            title = "Fonts and Styles";
            break;
        case 12:
            groupTitle = "Text";
            title = "Brushes";
            break;
        case 13:
            groupTitle = "Text";
            title = "Alignment";
            break;
        case 14:
            groupTitle = "Text";
            title = "Wrapping";
            break;
        }
    }

    Text {
        id: groupTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: titleTextItem.top
        font.pixelSize: 12 * root.dp
        color: "#DFD0B8"
        text: root.groupTitle
    }
    Text {
        id: titleTextItem
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 8 * root.dp
        font.pixelSize: 20 * root.dp
        color: "#DFD0B8"
        text: root.title
    }
    Row {
        anchors.top: titleTextItem.bottom
        anchors.topMargin: 6 * root.dp
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 4 * root.dp
        Repeater {
            model: root.itemCount
            Rectangle {
                // Between 0..1 when the page indicator is highligted
                required property int index
                readonly property real animState: Math.max(0, (1.0 - Math.abs(root.visibilityState - index)))
                width: height + 16 * root.dp * animState
                height: 8 * root.dp
                radius: width/2
                color: "#948979"
                border.width: 1
                border.color: "#DFD0B8"
                opacity: 0.2 + 0.8 * animState
            }
        }
    }
}

