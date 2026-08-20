// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Rectangle {
    width: 1024
    height: 512
    color: "white"

    Rectangle {
        id: rectangle
        x: 0
        y: 0
        width: 256
        height: 512
        color: "red"
    }
    Text {
        anchors.fill: parent
        text: "QML"
        font.pixelSize: 200
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
