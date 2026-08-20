// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root

    width: 44
    height: 44
    radius: 14
    color: "#41CD52"

    signal tapped()
    property alias iconSource: icon.source

    Rectangle {
        width: root.width / 2
        height: root.width / 2
        anchors.bottom: root.bottom
        anchors.left: root.left
        color: root.color
        radius: 6
    }

    Rectangle {
        width: root.width / 2
        height: root.width / 2
        anchors.top: root.top
        anchors.right: root.right
        color: root.color
        radius: 6
    }

    Image {
        id: icon

        anchors.centerIn: root
    }

    TapHandler {
        id: tapHandler

        gesturePolicy: TapHandler.WithinBounds
        onTapped: root.tapped()
    }
}
