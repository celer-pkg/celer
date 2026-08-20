// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Rectangle
{
    id: button
    signal clicked()
    property string buttonText
    width: powerOn.width + 10
    height: powerOn.height + 10
    border.color: "black"
    border.width: 2
    radius: 4


    Text {
        id: powerOn
        text: buttonText
        font.pointSize: 16
        anchors.centerIn: parent
    }
    MouseArea {
        anchors.fill: parent
        onClicked: button.clicked()
    }
}
