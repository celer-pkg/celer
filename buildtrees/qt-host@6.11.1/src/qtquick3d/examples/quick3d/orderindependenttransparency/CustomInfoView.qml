// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: root
    property int selection: 0
    property list<string> titles
    property list<string> contents

    Rectangle {
        anchors.fill: parent
        color: "#80404040"
        border.color: "#000000"
        border.width: 1
        opacity: 0.8
    }

    Label {
        id: title
        font.pixelSize: 18
        text: root.titles[root.selection]
        color: "white"
    }

    Text {
        id: content
        color: "white"
        font.pixelSize: 14
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: root.contents[root.selection]
    }
}
