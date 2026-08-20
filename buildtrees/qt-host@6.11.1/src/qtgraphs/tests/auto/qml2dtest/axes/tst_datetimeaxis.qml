// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    DateTimeAxis {
        id: initial
    }

    DateTimeAxis {
        id: initialized

        labelFormat: "yyyy"
        min: new Date(1960,1,1)
        max: new Date(2000,1,1)
        subTickCount: 2
        tickInterval:  2

        gridVisible: false
        labelsAngle: 90
        labelsVisible: false
        lineVisible: false
        subGridVisible: false
        visible: false
    }

    TestCase {
        name: "DateTimeAxis Initial"

        function test_1_initial() {
            compare(initial.labelFormat, "dd-MMMM-yy")
            compare(initial.min.getTime(), Date.UTC(1970))
            compare(initial.max.getTime(), Date.UTC(1980))
            compare(initial.subTickCount, 0)
            compare(initial.tickInterval, 0)
        }

        function test_2_initial_common() {
            compare(initial.gridVisible, true)
            compare(initial.labelsAngle, 0)
            compare(initial.labelsVisible, true)
            compare(initial.lineVisible, true)
            compare(initial.subGridVisible, true)
            compare(initial.visible, true)
        }

        function test_3_initial_change() {
            initial.labelFormat = "yyyy"
            initial.min = new Date(1960, 1, 1)
            initial.max = new Date(2000, 1, 1)
            initial.subTickCount = 2
            initial.tickInterval = 2

            initial.gridVisible = false
            initial.labelsAngle = 90
            initial.labelsVisible = false
            initial.lineVisible = false
            initial.subGridVisible = false
            initial.visible = false

            compare(initial.labelFormat, "yyyy")
            compare(initial.min, new Date(1960, 1, 1))
            compare(initial.max, new Date(2000, 1, 1))
            compare(initial.subTickCount, 2)
            compare(initial.tickInterval, 2)

            compare(initial.gridVisible, false)
            compare(initial.labelsAngle, 90)
            compare(initial.labelsVisible, false)
            compare(initial.lineVisible, false)
            compare(initial.subGridVisible, false)
            compare(initial.visible, false)
        }
    }

    TestCase {
        name: "DateTimeAxis Initialized"

        function test_1_initialized() {
            compare(initialized.labelFormat, "yyyy")
            compare(initialized.min, new Date(1960, 1, 1))
            compare(initialized.max, new Date(2000, 1, 1))
            compare(initialized.subTickCount, 2)
            compare(initialized.tickInterval, 2)

            compare(initialized.gridVisible, false)
            compare(initialized.labelsAngle, 90)
            compare(initialized.labelsVisible, false)
            compare(initialized.lineVisible, false)
            compare(initialized.subGridVisible, false)
            compare(initialized.visible, false)
        }

        function test_2_initialized_changed() {
            initialized.labelFormat = "dddd"
            initialized.min = new Date(2000, 1, 1)
            initialized.max = new Date(2025, 1, 1)
            initialized.subTickCount = 8
            initialized.tickInterval = 8

            initialized.gridVisible = true
            initialized.labelsAngle = 50
            initialized.labelsVisible = true
            initialized.lineVisible = true
            initialized.subGridVisible = true
            initialized.visible = true

            compare(initialized.labelFormat, "dddd")
            compare(initialized.min, new Date(2000, 1, 1))
            compare(initialized.max, new Date(2025, 1, 1))
            compare(initialized.subTickCount, 8)
            compare(initialized.tickInterval, 8)

            compare(initialized.gridVisible, true)
            compare(initialized.labelsAngle, 50)
            compare(initialized.labelsVisible, true)
            compare(initialized.lineVisible, true)
            compare(initialized.subGridVisible, true)
            compare(initialized.visible, true)

            // Signals
            compare(minSpy.count, 2)
            compare(maxSpy.count, 2)
            compare(labelFormatSpy.count, 1)
            compare(subTickCountSpy.count, 1)
            compare(tickIntervalSpy.count, 1)

            // Common signals
            compare(visibleSpy.count, 1)
            compare(gridVisibleSpy.count, 1)
            compare(labelsVisibleSpy.count, 1)
            compare(lineVisibleSpy.count, 1)
            compare(subGridVisibleSpy.count, 1)
            compare(labelsAngleSpy.count, 1)
        }

        function test_3_invalid() {
            initialized.subTickCount = -1;
            initialized.tickInterval = -1;

            compare(initialized.tickInterval, 0)
            compare(initialized.subTickCount, 0)
        }
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
        id: subTickCountSpy
        target: initialized
        signalName: "subTickCountChanged"
    }

    SignalSpy {
        id: tickIntervalSpy
        target: initialized
        signalName: "tickIntervalChanged"
    }

    // Common signals
    SignalSpy {
        id: visibleSpy
        target: initialized
        signalName: "visibleChanged"
    }

    SignalSpy {
        id: labelsVisibleSpy
        target: initialized
        signalName: "labelsVisibleChanged"
    }

    SignalSpy {
        id: lineVisibleSpy
        target: initialized
        signalName: "lineVisibleChanged"
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
        id: labelsAngleSpy
        target: initialized
        signalName: "labelsAngleChanged"
    }
}
