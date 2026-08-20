// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    ScatterSeries {
        id: initial
    }

    ScatterSeries {
        id: initialized

        pointDelegate: Rectangle {
            width: 12
            height: 12
        }

        color: "#ff00ff"
        selectedColor: "#00ff00"
        draggable: true

        name: "ScatterSeries"
        visible: false
        selectable: true
        hoverable: true
        opacity: 0.75
        valuesMultiplier: 0.75
    }

    // Values used for changing the properties
    Component { id: marker; Rectangle { width: 10; height: 10 } }

    TestCase {
        name: "ScatterSeries Initial"

        function test_1_initial() {
            compare(initial.pointDelegate, null)
        }

        function test_2_initial_common() {
            // Properties from QXYSeries
            compare(initial.color, "#00000000")
            compare(initial.selectedColor, "#00000000")
            compare(initial.draggable, false)

            // Properties from QAbstractSeries
            compare(initial.name, "")
            compare(initial.visible, true)
            compare(initial.selectable, false)
            compare(initial.hoverable, false)
            compare(initial.opacity, 1.0)
            compare(initial.valuesMultiplier, 1.0)
        }

        function test_3_initial_change() {
            initial.pointDelegate = marker

            initial.color = "#ff00ff"
            initial.selectedColor = "#00ff00"
            initial.draggable = true

            initial.name = "Scatter"
            initial.visible = false
            initial.selectable = true
            initial.hoverable = true
            initial.opacity = 0.5
            initial.valuesMultiplier = 0.5

            compare(initial.pointDelegate, marker)

            compare(initial.color, "#ff00ff")
            compare(initial.selectedColor, "#00ff00")

            compare(initial.name, "Scatter")
            compare(initial.visible, false)
            compare(initial.selectable, true)
            compare(initial.hoverable, true)
            compare(initial.opacity, 0.5)
            compare(initial.valuesMultiplier, 0.5)
        }
    }

    TestCase {
        name: "ScatterSeries Initialized"

        function test_1_initialized() {
            verify(initialized.pointDelegate)

            compare(initialized.color, "#ff00ff")
            compare(initialized.selectedColor, "#00ff00")
            compare(initialized.draggable, true)

            compare(initialized.name, "ScatterSeries")
            compare(initialized.visible, false)
            compare(initialized.selectable, true)
            compare(initialized.hoverable, true)
            compare(initialized.opacity, 0.75)
            compare(initialized.valuesMultiplier, 0.75)
        }

        function test_2_initialized_change() {
            initialized.pointDelegate = null

            initialized.color = "#0000ff"
            initialized.selectedColor = "#ff0000"
            initialized.draggable = false

            initialized.name = "Scatter"
            initialized.visible = true
            initialized.selectable = false
            initialized.hoverable = false
            initialized.opacity = 0.5
            initialized.valuesMultiplier = 0.25

            verify(!initialized.pointDelegate)

            compare(initialized.color, "#0000ff")
            compare(initialized.selectedColor, "#ff0000")
            compare(initialized.draggable, false)

            compare(initialized.name, "Scatter")
            compare(initialized.visible, true)
            compare(initialized.selectable, false)
            compare(initialized.hoverable, false)
            compare(initialized.opacity, 0.5)
            compare(initialized.valuesMultiplier, 0.25)
        }

        function test_3_initialized_change_to_invalid() {
            initialized.valuesMultiplier = 2.0 // range 0...1

            compare(initialized.valuesMultiplier, 1.0)

            initialized.valuesMultiplier = -1.0 // range 0...1
            compare(initialized.valuesMultiplier, 0.0)
        }
    }
}
