// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: colorPicker
    property string description
    property vector3d colorVector: Qt.vector3d(0.0, 0.0, 0.0)
    width: 200
    height: 35

    color: Qt.rgba(colorVector.x, colorVector.y, colorVector.z, 1.0)

    function close()
    {
        colorSliders.visible = false
    }

    TapHandler {
        onTapped: {
            if (!colorSliders.visible) {
                rSlider.value = colorVector.x
                gSlider.value = colorVector.y
                bSlider.value = colorVector.z
            }
            colorSliders.visible = !colorSliders.visible
        }
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        x: 20
        text: parent.description
        style: Text.Outline
        styleColor: "white"
        font.pixelSize: 20
        font.bold: true
    }
    Rectangle {
        id: colorSliders
        visible: false
        anchors.verticalCenter: colorPicker.verticalCenter
        anchors.left: colorPicker.right
        width: col.width
        height: col.height
        z: 10
        color: "#cccccc"
        Column {
            id: col
            Slider {
                id: rSlider
                onValueChanged: colorVector.x = value
                Rectangle {
                    anchors.fill: parent
                    z: -1
                    color: "red"
                }
            }
            Slider {
                id: gSlider
                onValueChanged: colorVector.y = value
                Rectangle {
                    anchors.fill: parent
                    z: -1
                    color: "green"
                }
            }
            Slider {
                id: bSlider
                onValueChanged: colorVector.z = value
                Rectangle {
                    anchors.fill: parent
                    z: -1
                    color: "blue"
                }
            }
        }
        // Very quick-and-dirty implicit mouse grabbing.
        MouseArea {
            id: implicitMouseGrabber
            x: -3000
            y: -3000
            z: -1
            width: 6000
            height: 6000

            onClicked: colorSliders.visible = false
        }
        onVisibleChanged: {
            if (visible) {
                colorPicker.z = 10
                colorPicker.parent.z = 10
            } else {
                colorPicker.z = 0
                colorPicker.parent.z = 0
            }
        }
    }
}
