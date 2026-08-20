// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root
    property alias iconSource: icon.source
    property alias text: label.text
    property bool checked: false
    property bool checkable: true
    property bool toggable: false
    property int iconSize: 36
    property var conflictingButtons: []

    width: 110
    height: 110
    radius: 10

    // Always semi-transparent; green tint when checked
    color: checked ? "#8000414a" : "#80222222"

    // The content container is *exactly* centered; no extra paddings
    Column {
        id: content
        anchors.centerIn: parent // true center
        spacing: 6
        width: parent.width - 16 // small horizontal margin for long labels

        Image {
            id: icon
            source: ""
            width: root.iconSize
            height: root.iconSize
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
            smooth: true
            mipmap: true
        }

        Text {
            id: label
            text: "Unset"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            elide: Text.ElideRight
            font.pixelSize: 16
            color: "white"
            wrapMode: Text.NoWrap
        }
    }

    // Click handling (toggle if checkable)
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (root.toggable) {
                root.checked = !root.checked
            } else if (root.checkable) {
                root.checked = true
                for (let i = 0; i < root.conflictingButtons.length; i++) {
                    root.conflictingButtons[i].checked = false
                }
            }

            root.clicked();
        }
        onPressed: root.opacity = 0.85
        onReleased: root.opacity = 1.0
    }

    signal clicked()
}
