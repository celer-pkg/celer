// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    BarSet {
        id: initial
    }

    BarSet {
        id: initialized
        label: "Set1"
        labelColor: "green"
        color: "red"
        borderColor: "black"
        borderWidth: 3
        values: [1, 2, 3, 4, 5, 6]
    }

    TestCase {
        name: "BarSet Initial"

        function test_1_initial() {
            compare(initial.label, "")
            compare(initial.labelColor, "#00000000")
            compare(initial.color, "#00000000")
            compare(initial.borderColor, "#00000000")
            compare(initial.borderWidth, -1)
            compare(initial.values, [])
            compare(initial.count, 0)
            compare(initial.selectedBars, [])
        }

        function test_2_initial_change() {
            var values = [1, 2, 3]

            initial.label = "BarSet"
            initial.labelColor = "red"
            initial.color = "blue"
            initial.borderColor = "yellow"
            initial.borderWidth = 2
            initial.values = values

            compare(initial.label, "BarSet")
            compare(initial.labelColor, "#ff0000")
            compare(initial.color, "#0000ff")
            compare(initial.borderColor, "#ffff00")
            compare(initial.borderWidth, 2)
            compare(initial.values, values)
            compare(initial.count, 3)
            compare(initial.selectedBars, [])
        }
        function test_3_initial_selections() {
            initial.deselectAllBars();
            initial.clear();
            compare(initial.selectedBars, [])
            initial.append(0)
            initial.append(1)
            initial.append(2)
            initial.append(3)
            initial.append(4)
            compare(initial.isBarSelected(0), false)
            initial.selectBar(0);
            compare(initial.isBarSelected(0), true)
            compare(initial.selectedBars, [0])
            initial.selectBar(2);
            // Note: Checking just length as the order of the elements may differ
            compare(initial.selectedBars.length, 2) // [0, 2]
            initial.toggleSelection([3])
            compare(initial.selectedBars.length, 3) // [0, 2, 3]
            initial.toggleSelection([3])
            compare(initial.selectedBars.length, 2) // [0, 2]
            initial.setBarSelected(0, false)
            compare(initial.selectedBars.length, 1) // [0]
            initial.selectAllBars()
            compare(initial.selectedBars.length, 5) // [0, 1, 2, 3, 4]
            initial.deselectAllBars()
            compare(initial.selectedBars.length, 0)
            initial.selectBars([1, 2, 3])
            compare(initial.selectedBars.length, 3) // [1, 2, 3]
            initial.deselectBars([2, 3, 4])
            compare(initial.selectedBars.length, 1) // [1]
        }
    }

    TestCase {
        name: "BarSet Initialized"

        function test_1_initialized() {
            compare(initialized.label, "Set1")
            compare(initialized.labelColor, "#008000")
            compare(initialized.color, "#ff0000")
            compare(initialized.borderColor, "#000000")
            compare(initialized.borderWidth, 3)
            compare(initialized.values, [1, 2, 3, 4, 5, 6])
            compare(initialized.count, 6)
            compare(initialized.selectedBars, [])
        }

        function test_2_initialized_change() {
            var values = [10, 20, 30]

            initialized.label = "BarSet"
            initialized.labelColor = "red"
            initialized.color = "blue"
            initialized.borderColor = "yellow"
            initialized.borderWidth = 2
            initialized.values = values

            compare(initialized.label, "BarSet")
            compare(initialized.labelColor, "#ff0000")
            compare(initialized.color, "#0000ff")
            compare(initialized.borderColor, "#ffff00")
            compare(initialized.borderWidth, 2)
            compare(initialized.values, values)
            compare(initialized.count, 3)
            compare(initialized.selectedBars, [])

            // Signals
            compare(labelSpy.count, 1)
            compare(colorSpy.count, 1)
            compare(labelColorSpy.count, 1)
            compare(borderColorSpy.count, 1)
            compare(borderWidthSpy.count, 1)
            compare(valuesSpy.count, 2)
        }

        function test_3_initialized_change_to_null() {
            initialized.label = ""
            initialized.values = []

            compare(initialized.label, "")
            compare(initialized.values, [])
            compare(initialized.count, 0)
            compare(initialized.selectedBars, [])
        }

        function test_4_initialized_append_remove() {
            initialized.clear()
            compare(initialized.values, [])

            // 6 calls from when values list is first initialized
            // 9 calls when original values are replaced with different values (remove old values, append new ones)
            // 3 calls when values are replaced with empty list (remove old values)
            compare(countSpy.count, 18)

            //append
            initialized.append(10)
            compare(initialized.count, 1)
            initialized.append([20,30,40])
            compare(initialized.count, 4)
            compare(countSpy.count, 20)

            //remove
            initialized.remove(0, 2) // 30, 40
            compare(initialized.count, 2)
            compare(countSpy.count, 21)

            //insert
            initialized.insert(0, 20) // 20, 30, 40
            compare(initialized.count, 3)
            compare(countSpy.count, 22)

            //replace
            initialized.replace(1, 10) //20, 10, 40
            compare(initialized.count, 3)

            //at
            let value = initialized.at(2)
            compare(value, 40)

            //sum
            let sum = initialized.sum()
            compare(sum, 70)
        }

        function test_5_initialized_select_invokables() {
            let selected = initialized.isBarSelected(0)
            compare(selected, false)

            initialized.selectBar(0)
            selected = initialized.isBarSelected(0)
            compare(selected, true)

            initialized.deselectBar(0)

            initialized.setBarSelected(1, true)
            selected = initialized.isBarSelected(1)
            compare(selected, true)

            initialized.selectAllBars()

            initialized.deselectAllBars()

            initialized.selectBars([0, 1, 2])
            compare(initialized.isBarSelected(0), true)
            compare(initialized.isBarSelected(1), true)
            compare(initialized.isBarSelected(2), true)

            initialized.deselectBars([0, 1])
            compare(initialized.isBarSelected(0), false)
            compare(initialized.isBarSelected(1), false)

            initialized.toggleSelection([2])
            compare(initialized.isBarSelected(2), false)
        }
    }

    SignalSpy {
        id: labelSpy
        target: initialized
        signalName: "labelChanged"
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
        id: labelColorSpy
        target: initialized
        signalName: "labelColorChanged"
    }

    SignalSpy {
        id: valuesSpy
        target: initialized
        signalName: "valuesChanged"
    }

    SignalSpy {
        id: borderWidthSpy
        target: initialized
        signalName: "borderWidthChanged"
    }

    SignalSpy {
        id: countSpy
        target: initialized
        signalName: "countChanged"
    }

    SignalSpy {
        id: selectedBarsSpy
        target: initialized
        signalName: "selectedBarsChanged"
    }
}
