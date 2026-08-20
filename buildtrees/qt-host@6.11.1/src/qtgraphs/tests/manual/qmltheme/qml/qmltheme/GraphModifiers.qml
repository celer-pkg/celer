// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Fusion
import QtGraphs

ColumnLayout {
    spacing: 10

    property bool barsVisible: barsMode.checked
    property bool valueColoring: valueColoringChange.checked
    property bool shaded: shadingChange.checked
    property int transparencyTechnique: transparencyTechniqueChange.currentValue

    Label {
        text: "Bars3D Graph"
        color: "gray"
    }
    CheckBox {
        id: barsMode
        onCheckedChanged: {
            customTheme = (checked ? customBarsTheme : customSurfaceTheme)
            currentGraph = (checked ? bars : surface)
        }
    }

    Label {
        text: "Ambient Light Strength"
        color: "gray"
    }
    Slider {
        from: 0.0
        to: 1.0
        value: currentGraph.ambientLightStrength
        onValueChanged: currentGraph.ambientLightStrength = value
    }

    Label {
        text: "Light Strength"
        color: "gray"
    }
    Slider {
        from: 0.0
        to: 10.0
        value: currentGraph.lightStrength
        onValueChanged: currentGraph.lightStrength = value
    }

    Label {
        visible: !colorStyle.checked
        text: testgradientchange.checked ? "Gradient Color, Red" : "Light Color; Red"
        color: "gray"
    }
    Slider {
        visible: !colorStyle.checked
        from: 0.0
        to: 1.0
        value: testgradientchange.checked ? 1.0 : currentGraph.lightColor.r
        onValueChanged: testgradientchange.checked ? (redstop.color.r = value)
                                                   : (currentGraph.lightColor.r = value)
    }

    Label {
        visible: !colorStyle.checked
        text: testgradientchange.checked ? "Gradient Color, Green" : "Light Color; Green"
        color: "gray"
    }
    Slider {
        visible: !colorStyle.checked
        from: 0.0
        to: 1.0
        value: testgradientchange.checked ? 0.5 : currentGraph.lightColor.g
        onValueChanged: testgradientchange.checked ? (greenstop.color.g = value)
                                                   : (currentGraph.lightColor.g = value)
    }

    Label {
        visible: colorStyle.checked && barsVisible
        text: testgradientchange.checked ? "Bar Color, Blue" : "Light Color; Blue"
        color: "gray"
    }
    Slider {
        visible: colorStyle.checked && barsVisible
        from: 0.0
        to: 1.0
        value: testgradientchange.checked ? barColor.color.b
                                          : currentGraph.lightColor.b
        onValueChanged: testgradientchange.checked ? barColor.color.b  = value
                                                   : currentGraph.lightColor.b = value
    }

    Label {
        visible: colorStyle.checked && !barsVisible
        text: testgradientchange.checked ? "Surface Color, Blue" : "Light Color; Blue"
        color: "gray"
    }
    Slider {
        visible: colorStyle.checked && !barsVisible
        from: 0.0
        to: 1.0
        value: testgradientchange.checked ? surfaceColor.color.b
                                          : currentGraph.lightColor.b
        onValueChanged: testgradientchange.checked ? surfaceColor.color.b  = value
                                                   : currentGraph.lightColor.b = value
    }

    Label {
        visible: testgradientchange.checked && !colorStyle.checked
        text: "Gradient green alpha"
        color: "gray"
    }
    Slider {
        visible: testgradientchange.checked && !colorStyle.checked
        from: 0.0
        to: 1.0
        value: 1.0
        onValueChanged: greenstop.color.a = value
    }

    Label {
        visible: testgradientchange.checked && !colorStyle.checked
        text: "Gradient red alpha"
        color: "gray"
    }
    Slider {
        visible: testgradientchange.checked && !colorStyle.checked
        from: 0.0
        to: 1.0
        value: 1.0
        onValueChanged: redstop.color.a = value
    }

    Label {
        visible: testgradientchange.checked && colorStyle.checked && barsVisible
        text: "Bar Color alpha"
        color: "gray"
    }
    Slider {
        visible: testgradientchange.checked && colorStyle.checked && barsVisible
        from: 0.0
        to: 1.0
        value: 1.0
        onValueChanged: barColor.color.a = value
    }

    Label {
        text: "Color Style Uniform"
        color: "gray"
    }
    CheckBox {
        id: colorStyle
        checked: (customTheme.colorStyle === GraphsTheme.ColorStyle.Uniform)
        onCheckedChanged: {
            if (checked)
                customTheme.colorStyle = GraphsTheme.ColorStyle.Uniform
            else
                customTheme.colorStyle = GraphsTheme.ColorStyle.RangeGradient
        }
    }
    Label {
        text: "Test Theme Color /\nGradient Change"
        color: "gray"
    }
    CheckBox {
        id: testgradientchange
        checked: false
    }

    Label {
        text: "Value Coloring"
        color: "gray"
        visible: bars.visible && !colorStyle.checked;
    }
    CheckBox {
        id: valueColoringChange
        checked: false
        visible: bars.visible && !colorStyle.checked
    }

    Label {
        text: "Shaded lighting"
        color: "gray"
    }
    CheckBox {
        id: shadingChange
        checked: true
    }

    Label {
        text: "Transparency technique"
        color: "gray"
    }
    ComboBox {
        id: transparencyTechniqueChange
        textRole: "text"
        valueRole: "value"

        model : [
            { value: Graphs3D.TransparencyTechnique.Default, text: qsTr("Default") },
            { value: Graphs3D.TransparencyTechnique.Approximate, text: qsTr("Approximate") },
            { value: Graphs3D.TransparencyTechnique.Accurate, text: qsTr("Accurate") }
        ]
    }
}
