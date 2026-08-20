// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Item {
    id: control

    property alias text: label.text
    property alias wrapMode: label.wrapMode
    property alias color: label.color
    property alias font: label.font
    property alias textStyle: label.textStyle
    property int lineHeightMode: Text.ProportionalHeight
    property real lineHeight: 1.65

    visible: text !== ""
    implicitHeight: visible ? (label.lineHeight * (label.lineCount - 1) + label.textHeight) : 0
    implicitWidth: label.implicitWidth

    Label {
        id: label

        property int textStyle: ApplicationConfig.TextStyle.Body_L
        readonly property real textHeight: textMetrics.tightBoundingRect.height

        y: textMetrics.boundingRect.y - textMetrics.tightBoundingRect.y
        anchors {
            left: parent.left
            right: parent.right
        }

        color: "#162655"
        font {
            pixelSize: ApplicationConfig.responsiveFontSize(textStyle)
            family: ApplicationConfig.fontFamily()
            bold: ApplicationConfig.isBoldText(textStyle)
        }

        // In Proportional mode, lineHeight depends on the font height,
        // which doesn't match the actual rendered text height.
        // Instead, label.lineHeightMode is set to Text.FixedHeight and
        // label.lineHeight is calculated from the true rendered height
        // (label.textHeight), scaled by the control's lineHeight and mode.
        lineHeightMode: Text.FixedHeight
        lineHeight: {
            switch (control.lineHeightMode) {
            case Text.ProportionalHeight:
                return control.lineHeight * textHeight
            case Text.FixedHeight:
                return control.lineHeight
            }
        }
    }

    TextMetrics {
        id: textMetrics
        font: label.font
        text: label.text
    }
}
