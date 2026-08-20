// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: colorPicker
    property bool isHovered: false
    property alias color: root.color

    width: 100
    height: 26

    Rectangle {
        id: border
        color: "transparent"
        border.width: 2
        border.color: colorPicker.isHovered ? palette.dark : palette.alternateBase
        anchors.fill: parent

        Image {
            anchors.fill: parent
            anchors.margins: 4
            source: "images/grid_8x8.png"
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            color: colorPicker.color
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: colorPicker.isHovered = true
            onExited: colorPicker.isHovered = false
            onClicked: {
                colorPickerPopup.open()
            }
        }
    }

    Dialog {
        id: colorPickerPopup
        title: "Color Picker"

        anchors.centerIn: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        Pane {
            RowLayout {
                id: root
                property color color: "red"

                Item {
                    implicitWidth: 135
                    implicitHeight: 135
                    ShaderEffect {
                        id: hsvWheel
                        anchors.centerIn: parent
                        blending: true
                        width: 128
                        height: 128
                        property real value: 1.0

                        fragmentShader: "shaders/huesaturation.frag.qsb"

                        Item {
                            id: reticule

                            function reflectColor(hue, saturation) {
                                let angleDegrees = hue * 360
                                if (angleDegrees > 180)
                                    angleDegrees = angleDegrees - 360
                                let angleRadians = angleDegrees * (Math.PI / 180)
                                let vector = Qt.vector2d(Math.cos(angleRadians), Math.sin(angleRadians)).normalized().times(0.5).times(saturation)
                                vector = vector.plus(Qt.vector2d(0.5, 0.5))
                                reticule.x = vector.x * hsvWheel.width
                                reticule.y = vector.y * hsvWheel.width
                            }


                            Rectangle {
                                x: -5
                                y: -5
                                width: 10
                                height: 10
                                radius: 5
                                color: "transparent"
                                border.width: 1
                                border.color: "black"

                                Rectangle {
                                    x: 0.5
                                    y: 0.5
                                    width: 9
                                    height: 9
                                    radius: 4.5
                                    color: root.color
                                    border.width: 1
                                    border.color: "white"
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            preventStealing: true

                            function handleMouseMove(x : real, y : real, width : real) : vector2d {
                                let normalizedX = x / width - 0.5;
                                let normalizedY = y / width - 0.5;
                                let angle = Math.atan2(normalizedY, normalizedX);
                                let toCenter = Qt.vector2d(normalizedX, normalizedY);
                                let radius = toCenter.length() * 2.0;
                                let degrees = angle * (180 / Math.PI)
                                if (degrees < 0)
                                    degrees = 360 + degrees
                                let hue = degrees / 360

                                root.color = Qt.hsva(hue, radius, root.color.hsvValue, root.color.a)

                                if (radius <= 1.0)
                                    return Qt.vector2d(x, y)
                                // Limit to radius of 1.0
                                toCenter = toCenter.normalized();
                                let halfWidth = width * 0.5;
                                let newX = halfWidth * toCenter.x + halfWidth
                                let newY = halfWidth * toCenter.y + halfWidth
                                return Qt.vector2d(newX, newY)
                            }

                            onClicked: (mouse) => {
                                            let pos = handleMouseMove(mouse.x, mouse.y, hsvWheel.width)
                                            reticule.x = pos.x;
                                            reticule.y = pos.y;
                            }

                            onPositionChanged: (mouse) => {
                                                let pos = handleMouseMove(mouse.x, mouse.y, hsvWheel.width)
                                                reticule.x = pos.x;
                                                reticule.y = pos.y;
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    updateColorSections()
                    reticule.reflectColor(root.color.hsvHue, root.color.hsvSaturation)
                }

                Connections {
                    target: root
                    function onColorChanged() {
                        root.updateColorSections()
                    }
                }

                function updateColorSections() {
                    rgbSection.redValue = root.color.r * 255
                    rgbSection.greenValue = root.color.g * 255
                    rgbSection.blueValue = root.color.b * 255
                    hsvSection.hueValue = root.color.hsvHue * 360
                    hsvSection.saturationValue = root.color.hsvSaturation * 100
                    hsvSection.valueValue = root.color.hsvValue  * 100
                    alphaSection.alphaValue = root.color.a * 255
                }
                ColumnLayout {

                    SectionLayout {
                        title: "RGB"
                        id: rgbSection
                        property int redValue: 0
                        property int greenValue: 0
                        property int blueValue: 0
                        implicitWidth: 300

                        function updateRGB() {
                            root.color = Qt.rgba(redValue / 255, greenValue / 255, blueValue / 255, alphaSection.alphaValue / 255)
                            reticule.reflectColor(root.color.hsvHue, root.color.hsvSaturation)
                        }

                        onRedValueChanged: hexTextField.updateText()
                        onGreenValueChanged: hexTextField.updateText()
                        onBlueValueChanged: hexTextField.updateText()

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "R:"
                            }
                            Slider {
                                id: redSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 255
                                value: rgbSection.redValue
                                onValueChanged: {
                                    if (value !== rgbSection.redValue) {
                                        rgbSection.redValue = value
                                        if (pressed)
                                            rgbSection.updateRGB()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: rgbSection.redValue
                            onValueChanged: {
                                if (value !== rgbSection.redValue)
                                    rgbSection.redValue = value
                            }
                            onValueModified: {
                                rgbSection.updateRGB();
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "G:"
                            }
                            Slider {
                                id: greenSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 255
                                value: rgbSection.greenValue
                                onValueChanged: {
                                    if (value !== rgbSection.greenValue) {
                                        rgbSection.greenValue = value
                                        if (pressed)
                                            rgbSection.updateRGB()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: rgbSection.greenValue
                            onValueChanged: {
                                if (value !== rgbSection.greenValue)
                                    rgbSection.greenValue = value
                            }
                            onValueModified: {
                                rgbSection.updateRGB()
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "B:"
                            }
                            Slider {
                                id: blueSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 255
                                value: rgbSection.blueValue
                                onValueChanged: {
                                    if (value !== rgbSection.blueValue) {
                                        rgbSection.blueValue = value
                                        if (pressed)
                                            rgbSection.updateRGB()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: rgbSection.blueValue
                            onValueChanged: {
                                if (value !== rgbSection.blueValue)
                                    rgbSection.blueValue = value
                            }
                            onValueModified: {
                                rgbSection.updateRGB()
                            }
                        }


                        Label {
                            text: "Hex:"
                            Layout.fillWidth: true
                        }
                        TextField {
                            id: hexTextField
                            maximumLength: 6
                            Layout.alignment: Qt.AlignRight

                            function updateText() {
                                if (activeFocus)
                                    return

                                let redText = rgbSection.redValue.toString(16).toUpperCase()
                                let greenText = rgbSection.greenValue.toString(16).toUpperCase()
                                let blueText = rgbSection.blueValue.toString(16).toUpperCase()
                                if (redText.length == 1)
                                    redText = "0" + redText
                                if (greenText.length == 1)
                                    greenText = "0" + greenText
                                if (blueText.length == 1)
                                    blueText = "0" + blueText
                                text = redText + greenText + blueText;
                            }

                            function expandText(text) {
                                let newText = text.toUpperCase()
                                let expandLength = 6 - newText.length
                                for (let i = 0; i < expandLength; ++i)
                                    newText = "0" + newText
                                return newText
                            }

                            Component.onCompleted: updateText()
                            validator: RegularExpressionValidator {
                                regularExpression: /^[0-9A-Fa-f]{0,6}$/
                            }
                            onTextChanged: {
                                if (!acceptableInput)
                                    return;
                                let colorText = expandText(text)
                                rgbSection.redValue = parseInt(colorText.substr(0, 2), 16)
                                rgbSection.greenValue = parseInt(colorText.substr(2, 2), 16)
                                rgbSection.blueValue = parseInt(colorText.substr(4, 2), 16)
                                if (activeFocus)
                                    rgbSection.updateRGB()
                            }

                            onAccepted: {
                                text = expandText(text)
                            }
                        }
                    }

                    SectionLayout {
                        title: "HSV"
                        id: hsvSection
                        property int hueValue: 0
                        property int saturationValue: 0
                        property int valueValue: 0
                        implicitWidth: 300

                        function updateHSV() {
                            root.color = Qt.hsva(hueValue / 360, saturationValue / 100, valueValue / 100, alphaSection.alphaValue / 255)
                            reticule.reflectColor(root.color.hsvHue, root.color.hsvSaturation)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "H:"
                            }
                            Slider {
                                id: hueSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 360
                                value: hsvSection.hueValue
                                onValueChanged: {
                                    if (value !== hsvSection.hueValue) {
                                        hsvSection.hueValue = value
                                        if (pressed)
                                            hsvSection.updateHSV()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 360
                            value: hsvSection.hueValue
                            onValueChanged: {
                                if (value !== hsvSection.hueValue)
                                    hsvSection.hueValue = value
                            }
                            onValueModified: {
                                hsvSection.updateHSV()
                            }
                        }


                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "S:"
                            }
                            Slider {
                                id: saturationSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: hsvSection.saturationValue
                                onValueChanged: {
                                    if (value !== hsvSection.saturationValue) {
                                        hsvSection.saturationValue = value
                                        if (pressed)
                                            hsvSection.updateHSV()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: hsvSection.saturationValue
                            onValueChanged: {
                                if (value !== hsvSection.saturationValue)
                                    hsvSection.saturationValue = value
                            }
                            onValueModified: {
                                hsvSection.updateHSV()
                            }
                        }


                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "V:"
                            }
                            Slider {
                                id: valueSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: hsvSection.valueValue
                                onValueChanged: {
                                    if (value !== hsvSection.valueValue) {
                                        hsvSection.valueValue = value
                                        if (pressed)
                                            hsvSection.updateHSV()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: hsvSection.valueValue
                            onValueChanged: {
                                if (value !== hsvSection.valueValue)
                                    hsvSection.valueValue = value
                            }
                            onValueModified: {
                                hsvSection.updateHSV()
                            }
                        }

                    }

                    SectionLayout {
                        title: "Opacity / Alpha"
                        id: alphaSection
                        property int alphaValue: 0
                        implicitWidth: 300

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "V:"
                            }
                            Slider {
                                id: alphaSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 255
                                value: alphaSection.alphaValue
                                onValueChanged: {
                                    if (value !== alphaSection.alphaValue) {
                                        alphaSection.alphaValue = value
                                        if (pressed)
                                            hsvSection.updateHSV()
                                    }
                                }
                            }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: alphaSection.alphaValue
                            onValueChanged: {
                                if (value !== alphaSection.alphaValue)
                                    alphaSection.alphaValue = value
                            }
                            onValueModified: {
                                hsvSection.updateHSV()
                            }
                        }

                    }
                }
            }
        }
    }
}
