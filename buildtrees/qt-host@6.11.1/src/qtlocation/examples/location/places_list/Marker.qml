// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    width: image.width
    Image {
        id: image
        anchors.centerIn: parent
        source: "marker.png"
        Text{
            y: parent.height/10
            width: parent.width
            color: "white"
            font.bold: true
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            text: index
        }
    }
}
