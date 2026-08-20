// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    ValueAxis {
        id: initial
    }

    Text {
        id: initializedDummy
        font.family: "Sans Serif"
        font.bold: true
    }

    ValueAxis {
        id: initialized

        labelDecimals: 2
        labelFormat: "f"
        min: 2.0
        max: 20.0
        subTickCount: 1
        tickAnchor: 1
        tickInterval: 2.0

        gridVisible: false
        labelsAngle: 25
        labelsVisible: false
        labelDelegate: Text {
            font.pixelSize: 20
        }
        lineVisible: false
        subGridVisible: false
        titleColor: "#ff0000"
        titleFont: initializedDummy.font
        titleText: "Initialized"
        titleVisible: false
        visible: false
        alignment: Qt.AlignLeft
        color: "#00ff00"
        subColor: "#0000ff"
    }

    TestCase {
        name: "ValueAxis with GraphsView"

        GraphsView {id: graphsView1}
        GraphsView {id: graphsView2}
        ValueAxis {id: axis1}

        function test_1_add_to_multiple() {
            ignoreWarning(/.*axis already associated with.*/)
            graphsView1.axisX = axis1
            graphsView2.axisX = axis1
        }
    }

    TestCase {
        name: "valueaxis initial"

        Text { id: dummy }

        function test_1_initial() {
            compare(initial.labelDecimals, -1)
            compare(initial.labelFormat, "")
            compare(initial.min, 0.0)
            compare(initial.max, 10.0)
            compare(initial.subTickCount, 0)
            compare(initial.tickAnchor, 0)
            compare(initial.tickInterval, 0)
        }

        function test_2_initial_common() {
            // Common properties from AbstractAxis
            compare(initial.gridVisible, true)
            compare(initial.labelsAngle, 0)
            compare(initial.labelsVisible, true)
            compare(initial.labelDelegate, null)
            compare(initial.lineVisible, true)
            compare(initial.subGridVisible, true)
            compare(initial.titleColor, "#000000")
            compare(initial.color, "#000000")
            compare(initial.subColor, "#000000")
            // Initial font needs to be tested like this, as different platforms have different default font (QFont())
            compare(initial.titleFont.family, dummy.font.family)
            compare(initial.titleText, "")
            compare(initial.titleVisible, true)
            compare(initial.visible, true)
            compare(initial.alignment, Qt.AlignAbsolute)
        }

        function test_3_initial_change() {
            dummy.font.family = "Arial"

            // Properties from ValueAxis
            initial.labelDecimals = 1
            initial.labelFormat = "d"
            initial.min = -10
            initial.max = 0
            initial.subTickCount = 2
            initial.tickAnchor = 2
            initial.tickInterval = 3.0

            // Common properties from AbstractAxis
            initial.gridVisible = false
            initial.labelsAngle = 45
            initial.labelsVisible = false
            initial.lineVisible = false
            initial.subGridVisible = false
            initial.titleColor = "#ffffff"
            initial.titleFont = dummy.font
            initial.titleText = "Dummy"
            initial.titleVisible = false
            initial.visible = false
            initial.alignment = Qt.AlignTop
            initial.color = "#ffffff"
            initial.subColor = "#ffffff"

            // Properties from ValueAxis
            compare(initial.labelDecimals, 1)
            compare(initial.labelFormat, "d")
            compare(initial.min, -10.0)
            compare(initial.max, 0.0)
            compare(initial.subTickCount, 2)
            compare(initial.tickAnchor, 2)
            compare(initial.tickInterval, 3.0)

            // Common properties from AbstractAxis
            compare(initial.gridVisible, false)
            compare(initial.labelsAngle, 45)
            compare(initial.labelsVisible, false)
            compare(initial.lineVisible, false)
            compare(initial.subGridVisible, false)
            compare(initial.titleColor, "#ffffff")
            compare(initial.titleFont, dummy.font)
            compare(initial.titleText, "Dummy")
            compare(initial.titleVisible, false)
            compare(initial.visible, false)
            compare(initial.alignment, Qt.AlignTop)
            compare(initial.color, "#ffffff")
            compare(initial.subColor, "#ffffff")
        }
    }

    TestCase {
        name: "ValueAxis Initialized"

        function test_1_initialized() {
            // Properties from ValueAxis
            compare(initialized.labelDecimals, 2)
            compare(initialized.labelFormat, "f")
            compare(initialized.min, 2.0)
            compare(initialized.max, 20.0)
            compare(initialized.subTickCount, 1)
            compare(initialized.tickAnchor, 1)
            compare(initialized.tickInterval, 2.0)

            // Common properties from AbstractAxis
            compare(initialized.gridVisible, false)
            compare(initialized.labelsAngle, 25)
            compare(initialized.labelsVisible, false)
            verify(initialized.labelDelegate)
            compare(initialized.lineVisible, false)
            compare(initialized.subGridVisible, false)
            compare(initialized.titleColor, "#ff0000")
            compare(initialized.titleFont, initializedDummy.font)
            compare(initialized.titleText, "Initialized")
            compare(initialized.titleVisible, false)
            compare(initialized.visible, false)
            compare(initialized.alignment, Qt.AlignLeft)
            compare(initialized.color, "#00ff00")
            compare(initialized.subColor, "#0000ff")
        }

        function test_2_initialized_change() {
            // Properties from ValueAxis
            initialized.labelDecimals = 1
            initialized.labelFormat = "d"
            initialized.min = -10.0
            initialized.max = 0.0
            initialized.subTickCount = 2
            initialized.tickAnchor = 2
            initialized.tickInterval = 3.0

            // Common properties from AbstractAxis
            initialized.gridVisible = true
            initialized.labelsAngle = 45
            initialized.labelsVisible = true
            initialized.labelDelegate = null
            initialized.lineVisible = true
            initialized.subGridVisible = true
            initialized.titleColor = "#ffffff"
            initialized.titleFont = dummy.font
            initialized.titleText = "Dummy"
            initialized.titleVisible = true
            initialized.visible = true
            initialized.alignment = Qt.AlignRight
            initialized.color = "#ffffff"
            initialized.subColor = "#ffffff"

            // Properties from ValueAxis
            compare(initialized.labelDecimals, 1)
            compare(initialized.labelFormat, "d")
            compare(initialized.min, -10.)
            compare(initialized.max, 0.0)
            compare(initialized.subTickCount, 2)
            compare(initialized.tickAnchor, 2)
            compare(initialized.tickInterval, 3.0)

            initialized.labelFormat = "%.1f test"
            compare(initialized.labelFormat, "%.1f test")

            // Common properties from AbstractAxis
            compare(initialized.gridVisible, true)
            compare(initialized.labelsAngle, 45)
            compare(initialized.labelsVisible, true)
            verify(!initialized.labelDelegate)
            compare(initialized.lineVisible, true)
            compare(initialized.subGridVisible, true)
            compare(initialized.titleColor, "#ffffff")
            compare(initialized.titleFont, dummy.font)
            compare(initialized.titleText, "Dummy")
            compare(initialized.titleVisible, true)
            compare(initialized.visible, true)
            compare(initialized.alignment, Qt.AlignRight)
            compare(initialized.color, "#ffffff")
            compare(initialized.subColor, "#ffffff")

            // Signals
            compare(minSpy.count, 1)
            compare(maxSpy.count, 1)
            compare(labelFormatSpy.count, 2)
            compare(labelDecimalsSpy.count, 1)
            compare(tickSpy.count, 1)
            compare(subTickSpy.count, 1)
            compare(tickAnchorSpy.count, 1)

            // Common signals
            compare(visibleSpy.count, 1)
            compare(lineVisibleSpy.count, 1)
            compare(labelsVisibleSpy.count, 1)
            compare(labelsAngle.count, 1)
            compare(labelDelegateSpy.count, 1)
            compare(gridVisibleSpy.count, 1)
            compare(subGridVisibleSpy.count, 1)
            compare(titleTextSpy.count, 1)
            compare(titleColorSpy.count, 1)
            compare(titleVisibleSpy.count, 1)
            compare(titleFontSpy.count, 2)
            compare(alignmentSpy.count, 1)
            compare(colorSpy.count, 1)
            compare(subColorSpy.count, 1)
        }

        function test_3_initialized_change_to_invalid() {
            initialized.max = -10.0
            initialized.min = 10.0
            initialized.subTickCount = -1

            compare(initialized.max, 10.0)
            compare(initialized.min, 10.0)
            compare(initialized.subTickCount, 2) // This was set in previous test case
        }

        SignalSpy {
            id: minSpy
            target: initialized
            signalName: "minChanged"
        }

        SignalSpy {
            id: maxSpy
            target: initialized
            signalName: "maxChanged"
        }

        SignalSpy {
            id: labelFormatSpy
            target: initialized
            signalName: "labelFormatChanged"
        }

        SignalSpy {
            id: labelDecimalsSpy
            target: initialized
            signalName: "labelDecimalsChanged"
        }

        SignalSpy {
            id: tickSpy
            target: initialized
            signalName: "tickIntervalChanged"
        }

        SignalSpy {
            id: subTickSpy
            target: initialized
            signalName: "subTickCountChanged"
        }

        SignalSpy {
            id: tickAnchorSpy
            target: initialized
            signalName: "tickAnchorChanged"
        }

        // Common signals from AbstarctAxis
        SignalSpy {
            id: visibleSpy
            target: initialized
            signalName: "visibleChanged"
        }

        SignalSpy {
            id: lineVisibleSpy
            target: initialized
            signalName: "lineVisibleChanged"
        }

        SignalSpy {
            id: labelsVisibleSpy
            target: initialized
            signalName: "labelsVisibleChanged"
        }

        SignalSpy {
            id: labelsAngle
            target: initialized
            signalName: "labelsAngleChanged"
        }

        SignalSpy {
            id: labelDelegateSpy
            target: initialized
            signalName: "labelDelegateChanged"
        }

        SignalSpy {
            id: gridVisibleSpy
            target: initialized
            signalName: "gridVisibleChanged"
        }

        SignalSpy {
            id: subGridVisibleSpy
            target: initialized
            signalName: "subGridVisibleChanged"
        }

        SignalSpy {
            id: titleTextSpy
            target: initialized
            signalName: "titleTextChanged"
        }

        SignalSpy {
            id: titleColorSpy
            target: initialized
            signalName: "titleColorChanged"
        }

        SignalSpy {
            id: titleVisibleSpy
            target: initialized
            signalName: "titleVisibleChanged"
        }

        SignalSpy {
            id: titleFontSpy
            target: initialized
            signalName: "titleFontChanged"
        }

        SignalSpy {
            id: alignmentSpy
            target: initialized
            signalName: "alignmentChanged"
        }

        SignalSpy {
            id: colorSpy
            target: initialized
            signalName: "colorChanged"
        }

        SignalSpy {
            id: subColorSpy
            target: initialized
            signalName: "subColorChanged"
        }
    }
}
