// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    property real metalness: metalness.sliderValue
    property real roughness: roughness.sliderValue
    property real specular: specular.sliderValue
    property real specularTint: specularTint.sliderValue
    property real opacityValue: opacityValue.sliderValue

    color: "#6b7080"
    width: parent.width
    height: 75

    Component {
        id: propertySlider
        RowLayout {
            Label {
                id: propText
                text: name
                color: "#222840"
                font.pointSize: 12
                Layout.minimumWidth: 150
                Layout.maximumWidth: 150
            }
            Slider {
                id: slider
                from: fromValue
                to: toValue
                value: sliderValue
                stepSize: 0.01
                onValueChanged: sliderValue = value
                Layout.minimumWidth: 200
                Layout.maximumWidth: 200
                background: Rectangle {
                    x: slider.leftPadding
                    y: slider.topPadding + slider.availableHeight / 2 - height / 2
                    implicitWidth: 200
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
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        RowLayout {
            spacing: 20
            Loader {
                id: metalness
                property real sliderValue: 1.0
                property string name: "Metalness"
                property real fromValue: 0.0
                property real toValue: 1.0
                sourceComponent:  propertySlider
            }
            Loader {
                id: roughness
                property real sliderValue: 0.2
                property string name: "Roughness"
                property real fromValue: 0.0
                property real toValue: 1.0
                sourceComponent:  propertySlider
            }
        }
        RowLayout {
            spacing: 20
            Loader {
                id: specular
                property real sliderValue: 0.0
                property string name: "Specular Power"
                property real fromValue: 0.0
                property real toValue: 1.0
                sourceComponent:  propertySlider
            }
            Loader {
                id: specularTint
                property real sliderValue: 0.0
                property string name: "Specular Tint"
                property real fromValue: 0.0
                property real toValue: 1.0
                sourceComponent: propertySlider
            }
            Loader {
                id: opacityValue
                property real sliderValue: 1.0
                property string name: "Opacity"
                property real fromValue: 0.0
                property real toValue: 1.0
                sourceComponent: propertySlider
            }
        }
    }
}
