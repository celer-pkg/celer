// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl

RoundButton {
    id: control

    enum Type {
        Primary,
        Secondary
    }
    property int type: ToyButton.Type.Primary

    verticalPadding: Math.round(ApplicationConfig.responsiveSize(48))
    horizontalPadding: Math.round(ApplicationConfig.responsiveSize(80))

    property int textStyle: ApplicationConfig.TextStyle.Button_L
    font {
        family: ApplicationConfig.fontFamily()
        bold: ApplicationConfig.isBoldText(textStyle)
        pixelSize: ApplicationConfig.responsiveFontSize(textStyle)
    }

    property color color: {
        if (type === ToyButton.Type.Primary)
            return "#EFF7FF"
        else
            return pressed ? "#162655" : hovered ? "#162655" : "#162655"
    }
    icon.color: control.color
    icon.height: ApplicationConfig.responsiveSize(48)
    icon.width: ApplicationConfig.responsiveSize(48)

    background: Rectangle {
        visible: !control.flat
        implicitHeight: control.contentItem.implicitHeight + topPadding + bottomPadding
        radius: control.radius
        color: "transparent"
        border {
            width: {
                let width_ = 0
                if (control.type === ToyButton.Type.Primary)
                    width_ = control.pressed ? 4 : control.hovered ? 8 : 0
                else
                    width_ = control.pressed ? 4 : control.hovered ? 8 : 4
                return Math.round(ApplicationConfig.responsiveSize(width_))
            }
            color: {
                if (control.type === ToyButton.Type.Primary)
                    return control.pressed ? "#1C44B1" : control.hovered ? "#162655" : "#1C44B1"
                else
                    return control.pressed ? "#5EAAFC" : control.hovered ? "#162655" : "#D7D6E1"
            }
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            color: {
                if (control.type === ToyButton.Type.Primary)
                    return control.pressed ? "#1C44B1" : control.hovered ? "#2269EE" : "#1C44B1"
                else
                    return control.pressed ? "#5EAAFC" : control.hovered ? "#BCDCFF" : "#FFFFFF"
            }
        }

        ShaderEffectSource {
            id: theSource
            sourceItem: backgroundRect
        }

        ShaderEffect {
            readonly property Item iSource: theSource
            readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
            readonly property point rectSize: Qt.point(width, height)
            readonly property real radius: backgroundRect.radius
            readonly property real blur: 0.2
            readonly property color shadowColor: "#60000000"
            readonly property real thickness: 0.65

            anchors.fill: backgroundRect
            visible: control.pressed
            vertexShader: "shaders/buttonpressedshadow.vert.qsb"
            fragmentShader: "shaders/buttonpressedshadow.frag.qsb"
        }
    }

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: Qt.AlignCenter

        icon: control.icon
        text: control.text
        font: control.font
        color: control.color
    }
}
