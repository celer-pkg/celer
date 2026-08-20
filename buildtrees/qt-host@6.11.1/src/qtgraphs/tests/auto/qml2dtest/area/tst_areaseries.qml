// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Shapes
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    AreaSeries {
        id: initial
    }

    AreaSeries {
        id: initialized

        color: "#ff00ff"
        selectedColor: "#00ff00"
        borderColor: "#ff00ff"
        selectedBorderColor: "#00ff00"
        borderWidth: 2.0
        selected: true
        upperSeries: upperSeries
        lowerSeries: lowerSeries

        name: "AreaSeries"
        visible: false
        selectable: true
        hoverable: true
        opacity: 0.75
        valuesMultiplier: 0.75

        gradient: customGradient
        selectedGradient: customGradient
    }

    LineSeries {
        id: upperSeries
        XYPoint { x: 0; y: 1 }
        XYPoint { x: 1; y: 2 }
        XYPoint { x: 2; y: 3 }
    }

    LineSeries {
        id: lowerSeries
        XYPoint { x: 0; y: 0 }
        XYPoint { x: 1; y: 1 }
        XYPoint { x: 2; y: 2 }
    }

    LineSeries {
        id: upperSeries2
        XYPoint { x: 0; y: 1 }
        XYPoint { x: 1; y: 2 }
        XYPoint { x: 2; y: 3 }
    }

    LineSeries {
        id: lowerSeries2
        XYPoint { x: 0; y: 0 }
        XYPoint { x: 1; y: 1 }
        XYPoint { x: 2; y: 2 }
    }

    LinearGradient {
        id: customGradient
    }

    LinearGradient {
        id: customGradient2
    }

    TestCase {
        name: "AreaSeries Initial"

        function test_1_initial() {
            compare(initial.color, "#00000000")
            compare(initial.selectedColor, "#00000000")
            compare(initial.borderColor, "#00000000")
            compare(initial.selectedBorderColor, "#00000000")
            compare(initial.borderWidth, -1.0)
            compare(initial.selected, false)
            compare(initial.upperSeries, null)
            compare(initial.lowerSeries, null)
            compare(initial.gradient, null)
            compare(initial.selectedGradient, null)
        }

        function test_2_initial_common() {
            // Properties from QAbstractSeries
            compare(initial.name, "")
            compare(initial.visible, true)
            compare(initial.selectable, false)
            compare(initial.hoverable, false)
            compare(initial.opacity, 1.0)
            compare(initial.valuesMultiplier, 1.0)
        }

        function test_3_initial_change() {
            initial.color = "#ff00ff"
            initial.selectedColor = "#00ff00"
            initial.borderColor = "#ff00ff"
            initial.selectedBorderColor = "#00ff00"
            initial.borderWidth = 2.0
            initial.selected = true
            initial.upperSeries = upperSeries;
            initial.lowerSeries = lowerSeries;
            initial.gradient = customGradient;
            initial.selectedGradient = customGradient;

            initial.name = "Area"
            initial.visible = false
            initial.selectable = true
            initial.hoverable = true
            initial.opacity = 0.5
            initial.valuesMultiplier = 0.5

            compare(initial.color, "#ff00ff")
            compare(initial.selectedColor, "#00ff00")
            compare(initial.borderColor, "#ff00ff")
            compare(initial.selectedBorderColor, "#00ff00")
            compare(initial.borderWidth, 2.0);
            compare(initial.selected, true);
            compare(initial.upperSeries, upperSeries);
            compare(initial.lowerSeries, lowerSeries);
            compare(initial.gradient, customGradient);
            compare(initial.selectedGradient, customGradient);

            compare(initial.name, "Area")
            compare(initial.visible, false)
            compare(initial.selectable, true)
            compare(initial.hoverable, true)
            compare(initial.opacity, 0.5)
            compare(initial.valuesMultiplier, 0.5)
        }
    }

    TestCase {
        name: "AreaSeries Initialized"

        function test_1_initialized() {
            compare(initialized.color, "#ff00ff")
            compare(initialized.selectedColor, "#00ff00")
            compare(initialized.borderColor, "#ff00ff")
            compare(initialized.selectedBorderColor, "#00ff00")
            compare(initialized.borderWidth, 2.0)
            compare(initialized.selected, true)
            compare(initialized.upperSeries, upperSeries);
            compare(initialized.lowerSeries, lowerSeries);
            compare(initialized.gradient, customGradient);
            compare(initialized.selectedGradient, customGradient);

            compare(initialized.name, "AreaSeries")
            compare(initialized.visible, false)
            compare(initialized.selectable, true)
            compare(initialized.hoverable, true)
            compare(initialized.opacity, 0.75)
            compare(initialized.valuesMultiplier, 0.75)
        }

        function test_2_initialized_change() {
            initialized.color = "#0000ff"
            initialized.selectedColor = "#ff0000"
            initialized.borderColor = "#0000ff"
            initialized.selectedBorderColor = "#ff0000"
            initialized.borderWidth = 3.0;
            initialized.selected = false;
            initialized.upperSeries = upperSeries2;
            initialized.lowerSeries = lowerSeries2;
            initialized.gradient = customGradient2;
            initialized.selectedGradient = customGradient2;

            initialized.name = "Area"
            initialized.visible = true
            initialized.selectable = false
            initialized.hoverable = false
            initialized.opacity = 0.5
            initialized.valuesMultiplier = 0.25

            compare(initialized.color, "#0000ff")
            compare(initialized.selectedColor, "#ff0000")
            compare(initialized.borderColor, "#0000ff")
            compare(initialized.selectedBorderColor, "#ff0000")
            compare(initialized.borderWidth, 3.0)
            compare(initialized.selected, false)
            compare(initialized.upperSeries, upperSeries2);
            compare(initialized.lowerSeries, lowerSeries2);
            compare(initialized.gradient, customGradient2);
            compare(initialized.selectedGradient, customGradient2);

            compare(initialized.name, "Area")
            compare(initialized.visible, true)
            compare(initialized.selectable, false)
            compare(initialized.hoverable, false)
            compare(initialized.opacity, 0.5)
            compare(initialized.valuesMultiplier, 0.25)

            // Signals
            compare(colorSpy.count, 1)
            compare(selectedColorSpy.count, 1)
            compare(borderColorSpy.count, 1)
            compare(selectedBorderColorSpy.count, 1)
            compare(widthSpy.count, 1)
            compare(selectedSpy.count, 1)
            compare(upperSpy.count, 2)
            compare(lowerSpy.count, 2)
            compare(gradientSpy.count, 2)
            compare(selectedGradientSpy.count, 2)

            // Common Signals
            compare(nameSpy.count, 1)
            compare(visibleSpy.count, 1)
            compare(selectableSpy.count, 1)
            compare(hoveringSpy.count, 1)
            compare(opacitySpy.count, 1)
            compare(valuemultiplierSpy.count, 1)
        }

        function test_3_initialized_change_to_null() {
            initialized.upperSeries = null
            initialized.lowerSeries = null
            initialized.gradient = null
            initialized.selectedGradient = null

            verify(!initialized.upperSeries)
            verify(!initialized.lowerSeries)
            verify(!initialized.gradient)
            verify(!initialized.selectedGradient)
        }

        function test_4_initialized_change_to_invalid() {
            initialized.valuesMultiplier = 2.0 // range 0...1
            compare(initialized.valuesMultiplier, 1.0)

            initialized.valuesMultiplier = -1.0 // range 0...1
            compare(initialized.valuesMultiplier, 0.0)
        }
    }

    SignalSpy {
        id: colorSpy
        target: initialized
        signalName: "colorChanged"
    }

    SignalSpy {
        id: selectedColorSpy
        target: initialized
        signalName: "selectedColorChanged"
    }

    SignalSpy {
        id: borderColorSpy
        target: initialized
        signalName: "borderColorChanged"
    }

    SignalSpy {
        id: selectedBorderColorSpy
        target: initialized
        signalName: "selectedBorderColorChanged"
    }

    SignalSpy {
        id: widthSpy
        target: initialized
        signalName: "borderWidthChanged"
    }

    SignalSpy {
        id: selectedSpy
        target: initialized
        signalName: "selectedChanged"
    }

    SignalSpy {
        id: upperSpy
        target: initialized
        signalName: "upperSeriesChanged"
    }

    SignalSpy {
        id: lowerSpy
        target: initialized
        signalName: "lowerSeriesChanged"
    }

    SignalSpy {
        id: gradientSpy
        target: initialized
        signalName: "gradientChanged"
    }

    SignalSpy {
        id: selectedGradientSpy
        target: initialized
        signalName: "selectedGradientChanged"
    }

    //signals from QAbstractseries
    SignalSpy {
        id: nameSpy
        target: initialized
        signalName: "nameChanged"
    }

    SignalSpy {
        id: visibleSpy
        target: initialized
        signalName: "visibleChanged"
    }

    SignalSpy {
        id: selectableSpy
        target: initialized
        signalName: "selectableChanged"
    }

    SignalSpy {
        id: hoveringSpy
        target: initialized
        signalName: "hoverableChanged"
    }

    SignalSpy {
        id: opacitySpy
        target: initialized
        signalName: "opacityChanged"
    }

    SignalSpy {
        id: valuemultiplierSpy
        target: initialized
        signalName: "valuesMultiplierChanged"
    }
}
