// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    property alias text: label.text
    readonly property int animTime: 3000

    function restart() : void { anim.restart() }

    z: 2
    height: label.contentHeight + 10
    radius: 5
    color: "transparent"
    visible: anim.running

    ColorAnimation {
        id: anim
        to: "transparent"
        duration: root.animTime
        easing.type: Easing.InQuint
        target: root
        properties: "color"
        onStarted: {
            label.color = "black"
            textAnim.restart()
        }
    }

    ColorAnimation {
        id: textAnim
        to: "transparent"
        duration: root.animTime
        easing.type: Easing.InQuint
        target: label
        properties: "color"
    }

    Text {
        id: label
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: "OK"
        font.pixelSize: 24
        clip: true
        wrapMode: Text.WordWrap
    }
}

