// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    property real time: time.sliderValue
    property real amplitude: amplitude.sliderValue
    property real alpha: alpha.sliderValue
    property bool animateRotation: animControl.checkBoxSet
    property bool texturing: texControl.checkBoxSet
    property bool textureFromItem : texControl2.checkBoxSet

    color: "#6b7080"
    width: parent.width
    height: 75

    component PropertySlider : RowLayout {
        id: sliderRoot
        required property string name
        required property real sliderValue
        required property real fromValue
        required property real toValue
        Label {
            id: propText
            text: sliderRoot.name
            color: "#222840"
            font.pointSize: 12
        }
        Slider {
            id: slider
            from: sliderRoot.fromValue
            to: sliderRoot.toValue
            value: sliderRoot.sliderValue
            stepSize: 0.01
            onValueChanged: sliderRoot.sliderValue = value
            Layout.minimumWidth: 100
            Layout.maximumWidth: 200
            background: Rectangle {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 120
                implicitHeight: 4
                width: slider.availableWidth
                height: implicitHeight
                radius: 1
                color: "#222840"

                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    color: "#848895"
                    radius: 1
                }
            }
            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 20
                implicitHeight: 20
                radius: 5
                color: slider.pressed ? "#222840" : "#6b7080"
                border.color: "#848895"
            }
        }
        Label {
            id: valueText
            text: slider.value.toFixed(2)
            color: "#222840"
            font.pointSize: 12
            font.bold: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: 40
        }
    }

    component PropertyCheckBox : RowLayout {
        id: checkBoxRoot
        required property string checkBoxText
        required property bool checkBoxSet

        Label {
            text: checkBoxRoot.checkBoxText
            font.pointSize: 12
            font.bold: true
        }
        CheckBox {
            checked: false
            onCheckedChanged: checkBoxRoot.checkBoxSet = checked
        }
    }

    ScrollBar {
        id: hbar
        policy: parent.width < columnLayout.width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        orientation: Qt.Horizontal
        size: parent.width / columnLayout.width
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    ColumnLayout {
        id: columnLayout
        readonly property int leftMargin: 25
        x: leftMargin - hbar.position * width

        RowLayout {
            spacing: 10
            PropertySlider {
                id: time
                sliderValue: 0.0
                name: "Time"
                fromValue: 0.0
                toValue: 1.0
            }
            PropertySlider {
                id: amplitude
                sliderValue: 5.0
                name: "Amplitude"
                fromValue: 1.0
                toValue: 20.0
            }
            PropertySlider {
                id: alpha
                sliderValue: 1.0
                name: "Alpha"
                fromValue: 0.0
                toValue: 1.0
            }
            PropertyCheckBox {
                id: animControl
                checkBoxText: "Rotate"
                checkBoxSet: false
            }
            PropertyCheckBox {
                id: texControl
                checkBoxText: "Texture"
                checkBoxSet: false
            }
            Layout.rightMargin: texControl.checkBoxSet ? 0 : texControl2.width + 10
            PropertyCheckBox {
                id: texControl2
                visible: texControl.checkBoxSet
                checkBoxText: "Texture with Item"
                checkBoxSet: false
            }
        }
    }
}
