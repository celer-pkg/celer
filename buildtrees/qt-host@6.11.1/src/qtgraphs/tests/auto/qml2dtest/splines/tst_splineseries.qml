// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    height: 150
    width: 150

    SplineSeries {
        id: initial
    }

    SplineSeries {
        id: initialized

        name: "spline"
        width: 15
        capStyle: Qt.RoundCap
        pointDelegate: Rectangle { width: 2; height: 2 }
        color: "#aabbcc"
        selectedColor:  "#aabbcc"
        draggable: false
        visible: false
        selectable: false
        hoverable: true
        opacity: 0.8
        valuesMultiplier: 0.8
    }

    Component { id: marker; Rectangle { width: 10; height: 10 } }

    TestCase {
        name: "SplineSeries Initial"

        function test_initial() {
            compare(initial.width, 1.0)
            compare(initial.pointDelegate, null)
            compare(initial.capStyle, Qt.SquareCap)

            compare(initial.color, "#00000000")
            compare(initial.selectedColor, "#00000000")
            compare(initial.draggable, false)

            compare(initial.name, "")
            compare(initial.visible, true)
            compare(initial.selectable, false)
            compare(initial.hoverable, false)
            compare(initial.opacity, 1.0)
            compare(initial.valuesMultiplier, 1.0)
        }

        function initial_changed() {
            initial.width = 10.0
            initial.capStyle = Qt.RoundCap
            initial.pointDelegate = marker

            initial.color = "#ff00ff"
            initial.selectedColor = "#00ff00"
            initial.draggable = true

            initial.name = "spline"
            initial.visible = false
            initial.selectable = true
            initial.hoverable = true
            initial.opacity = 0.5
            initial.valuesMultiplier = 0.5

            compare(initial.width, 10.0)
            compare(initial.capStyle, Qt.RoundCap)
            compare(initial.pointDelegate, marker)

            compare(initial.color, "#ff00ff")
            compare(initial.selectedColor, "#00ff00")
            compare(initial.draggable, true)

            compare(initial.name, "spline")
            compare(initial.visible, false)
            compare(initial.selectable, true)
            compare(initial.hoverable, true)
            compare(initial.opacity, 0.5)
            compare(initial.valuesMultiplier, 0.5)
        }
    }

    TestCase {
        name: "SplineSeries Initialized"

        function test_initialized() {
            compare(initialized.width, 15)
            compare(initialized.capStyle, Qt.RoundCap)
            verify(initialized.pointDelegate)

            compare(initialized.color, "#aabbcc")
            compare(initialized.selectedColor, "#aabbcc")
            compare(initialized.draggable, false)

            compare(initialized.name, "spline")
            compare(initialized.visible, false)
            compare(initialized.selectable, false)
            compare(initialized.hoverable, true)
            compare(initialized.opacity, 0.8)
            compare(initialized.valuesMultiplier, 0.8)

            initialized.width = 20
            initialized.capStyle = Qt.SquareCap

            initialized.color = "#0000ff"
            initialized.selectedColor = "#ff00ff"
            initialized.draggable = true

            initialized.name = "changedSpline"
            initialized.visible = true
            initialized.selectable = true
            initialized.hoverable = false
            initialized.opacity = 0.5
            initialized.valuesMultiplier = 0.5

            compare(initialized.width, 20)
            compare(initialized.capStyle, Qt.SquareCap)

            compare(initialized.color, "#0000ff")
            compare(initialized.selectedColor, "#ff00ff")
            compare(initialized.draggable, true)

            compare(initialized.name, "changedSpline")
            compare(initialized.visible, true)
            compare(initialized.selectable, true)
            compare(initialized.hoverable, false)
            compare(initialized.opacity, 0.5)
            compare(initialized.valuesMultiplier,  0.5)

            compare(widthSpy.count, 1)
            compare(capStyleSpy.count, 1)

            compare(colorSpy.count, 1)
            compare(selectedColorSpy.count, 1)
            compare(draggableSpy.count, 1)

            compare(nameSpy.count, 1)
            compare(visibleSpy.count, 1)
            compare(selectableSpy.count, 1)
            compare(hoverableSpy.count, 1)
            compare(opacitySpy.count, 1)
            compare(valuesMultiplierSpy.count, 1)
        }

        // ScatterSeries signals
        SignalSpy {
            id: widthSpy
            target: initialized
            signalName: "widthChanged"
        }

        SignalSpy {
            id: capStyleSpy
            target: initialized
            signalName: "capStyleChanged"
        }

        //QXYSeries signals
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
            id: draggableSpy
            target: initialized
            signalName: "draggableChanged"
        }

        SignalSpy {
            id: pointDelegateSpy
            target: initialized
            signalName: "pointDelegateChanged"
        }

        // AbstractSeries signals

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
            id: hoverableSpy
            target: initialized
            signalName: "hoverableChanged"
        }

        SignalSpy {
            id: opacitySpy
            target: initialized
            signalName: "opacityChanged"
        }

        SignalSpy {
            id: valuesMultiplierSpy
            target: initialized
            signalName: "valuesMultiplierChanged"
        }
    }

}
