// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt.labs.animation

Item {
    id: root
    property real value: 50
    property real maximumValue: 99
    width: 100
    height: 240

    DragHandler {
        id: dragHandler
        objectName: root.objectName + " draghandler"
        target: knob
        xAxis.enabled: false
        yAxis.minimum: slot.y
        yAxis.maximum: slot.height + slot.y - knob.height
    }

    WheelHandler {
        id: wheelHandler
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        invertible: false
        rotationScale: -0.5
        target: knob
        property: "y"
    }

    Rectangle {
        id: slot
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        anchors.topMargin: 30
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        width: 10
        color: "black"
        radius: width / 2
        smooth: true
    }

    Rectangle {
        id: knob
        objectName: "Slider Knob"
        width: parent.width - 2
        height: 30
        radius: 5
        color: "beige"
        border.width: 3
        border.color: hover.hovered ? "orange" : "black"
        property bool programmatic: false
        property real multiplier: root.maximumValue / (dragHandler.yAxis.maximum - dragHandler.yAxis.minimum)
        onYChanged: if (!programmatic) root.value = root.maximumValue - (knob.y - dragHandler.yAxis.minimum) * multiplier
        transformOrigin: Item.Center
        function setValue(value) { knob.y = dragHandler.yAxis.maximum - value / knob.multiplier }
        HoverHandler {
            id: hover
            objectName: "Slider"
        }
        BoundaryRule on y {
            id: ybr
            minimum: slot.y
            maximum: slot.height + slot.y - knob.height
        }
    }

    Component.onCompleted: {
        knob.programmatic = true
        knob.setValue(root.value)
        knob.programmatic = false
    }
}
