// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Item {
    id: root

    implicitWidth: 256
    implicitHeight: 256

    property bool isHourClock: true
    property int hour: 12
    property int min: 0
    property real hourClockRotation: 0
    property real minutesClockRotation: 0

    property alias clockHand: clockHand
    property alias clockFace: clockFace
    property alias mouseArea: mouseArea
    property alias mouseX: mouseArea.mouseX
    property alias mouseY: mouseArea.mouseY
    property alias angle: clockHand.rotation
    property alias centerPoint: mouseArea.centerPoint

    Image {
        id: clockHand

        source: "images/Clock_Hand.svg"
        anchors.horizontalCenter: clockFace.horizontalCenter
        anchors.bottom: clockFace.verticalCenter

        transformOrigin: Item.Bottom
        rotation: root.angle
        antialiasing: true
    }

    Image {
        id: clockFace

        source: "images/Clock.svg"
        sourceSize.width: parent.width
        fillMode: Image.PreserveAspectFit


        MouseArea {
            id: mouseArea

            property int centerX: clockHand.x + clockHand.width / 2
            property int centerY: root.height / 2
            property vector2d centerPoint: Qt.vector2d(centerX, centerY)

            anchors.fill: clockFace
        }
    }
}
