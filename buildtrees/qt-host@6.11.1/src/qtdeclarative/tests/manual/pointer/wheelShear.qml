// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Rectangle {
    width: 400
    height: 400
    color: palette.window

    Row {
        CheckBox {
            id: smoothCB
            text: "smooth"
        }
        CheckBox {
            id: aaCB
            text: "antialiasing"
            checked: true
        }
        CheckBox {
            id: angleCB
            text: "by angle"
        }
    }

    Rectangle {
        color: "orange"
        width: 300; height: 200
        anchors.centerIn: parent
        transform: Shear {
            id: barber
            origin.x: 150
            origin.y: 100
            xAngle: angleCB.checked ? hwheel.rotation : 0
            yAngle: angleCB.checked ? vwheel.rotation : 0
            xFactor: !angleCB.checked ? hwheel.rotation / 120 : 0
            yFactor: !angleCB.checked ? vwheel.rotation / 120 : 0
        }
        smooth: smoothCB.checked
        antialiasing: aaCB.checked

        Text {
            text: "use mouse wheels / touchpad gestures to shear\nclick to set shear center"
            horizontalAlignment: Text.AlignHCenter
            anchors.centerIn: parent
        }

        Rectangle {
            x: barber.origin.x - width / 2
            y: barber.origin.y - width / 2
            width: 5; height: 5; radius: 2.5
            color: "red"
        }

        WheelHandler {
            id: vwheel
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            target: null
        }

        WheelHandler {
            id: hwheel
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            orientation: Qt.Horizontal
            target: null
        }

        TapHandler {
            onTapped: barber.origin = point.position
        }
    }

    Text {
        color: palette.windowText
        text: angleCB.checked ?
                  "angles " + barber.xAngle.toFixed(3) + ", " + barber.yAngle.toFixed(3)
                  + "\ntangents " + Math.tan(barber.xAngle * Math.PI / 180).toFixed(2) + ", "
                  + Math.tan(barber.yAngle * Math.PI / 180).toFixed(2)
                : "factors " + barber.xFactor.toFixed(3) + ", " + barber.yFactor.toFixed(3)
        anchors.bottom: parent.bottom
    }
}
