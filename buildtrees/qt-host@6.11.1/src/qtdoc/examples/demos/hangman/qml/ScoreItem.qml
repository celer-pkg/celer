// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Rectangle {
    id: borderRect
    color: "transparent"
    border.color: "white"
    width: scoreText.contentWidth + (topLevel.globalMargin * 2)
    radius: 10
    Label {
        id: scoreText
        anchors.fill: parent
        anchors.topMargin: topLevel.globalMargin
        anchors.bottomMargin: topLevel.globalMargin
        anchors.rightMargin: topLevel.globalMargin
        anchors.leftMargin: topLevel.globalMargin
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: parent.height
        font.family: "Helvetica"
        font.weight: Font.Light
        text: applicationData.score
        color: "white"
    }
}
