// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Value3DAxis {
        id: initial
    }

    Value3DAxis {
        id: initialized
        formatter: Value3DAxisFormatter { objectName: "formatter1" }
        labelFormat: "%f"
        reversed: true
        segmentCount: 10
        subSegmentCount: 5

        autoAdjustRange: false
        labelAutoAngle: 10.0
        max: 20
        min: -10
        title: "initialized"
        titleFixed: false
        titleVisible: true
        labelsVisible: false
        titleOffset: 1
    }

    Value3DAxis {
        id: change
    }

    Value3DAxis {
        id: invalid
    }

    TestCase {
        name: "Value3DAxis Initial"

        function test_initial() {
            verify(initial.formatter)
            compare(initial.labelFormat, "%.2f")
            compare(initial.reversed, false)
            compare(initial.segmentCount, 5)
            compare(initial.subSegmentCount, 1)

            compare(initial.autoAdjustRange, true)
            compare(initial.labelAutoAngle, 0.0)
            compare(initial.max, 10)
            compare(initial.min, 0)
            compare(initial.orientation, Abstract3DAxis.AxisOrientation.None)
            compare(initial.title, "")
            compare(initial.titleFixed, true)
            compare(initial.titleVisible, false)
            compare(initial.labelsVisible, true)
            compare(initial.titleOffset, 0)
            compare(initial.type, Abstract3DAxis.AxisType.Value)
        }
    }

    TestCase {
        name: "Value3DAxis Initialized"

        function test_initialized() {
            compare(initialized.formatter.objectName, "formatter1")
            compare(initialized.labelFormat, "%f")
            compare(initialized.reversed, true)
            compare(initialized.segmentCount, 10)
            compare(initialized.subSegmentCount, 5)

            compare(initialized.autoAdjustRange, false)
            compare(initialized.labelAutoAngle, 10.0)
            compare(initialized.max, 20)
            compare(initialized.min, -10)
            compare(initialized.title, "initialized")
            compare(initialized.titleFixed, false)
            compare(initialized.titleVisible, true)
            compare(initialized.labelsVisible, false)
            compare(initialized.titleOffset, 1)
        }
    }

    TestCase {
        name: "Value3DAxis Change"

        Value3DAxisFormatter { id: formatter1 }

        function test_change() {
            change.formatter = formatter1
            change.labelFormat = "%f"
            change.reversed = true
            change.segmentCount = 10
            change.subSegmentCount = 5

            compare(change.formatter, formatter1)
            compare(change.labelFormat, "%f")
            compare(change.reversed, true)
            compare(change.segmentCount, 10)
            compare(change.subSegmentCount, 5)

            change.autoAdjustRange = false
            change.labelAutoAngle = 10.0
            change.max = 20
            change.min = -10
            change.title = "initialized"
            change.titleFixed = false
            change.titleVisible = true
            change.labelsVisible = false
            change.titleOffset = -1

            compare(change.autoAdjustRange, false)
            compare(change.labelAutoAngle, 10.0)
            compare(change.max, 20)
            compare(change.min, -10)
            compare(change.title, "initialized")
            compare(change.titleFixed, false)
            compare(change.titleVisible, true)
            compare(change.labelsVisible, false)
            compare(change.titleOffset, -1)
        }
    }

    TestCase {
        name: "Value3DAxis Invalid"

        function test_invalid() {
            invalid.segmentCount = -1
            compare(invalid.segmentCount, 1)
            invalid.subSegmentCount = -1
            compare(invalid.subSegmentCount, 1)

            invalid.labelAutoAngle = -10
            compare(invalid.labelAutoAngle, 0.0)
            invalid.labelAutoAngle = 100
            compare(invalid.labelAutoAngle, 90.0)
            invalid.max = -10
            compare(invalid.min, -11)
            invalid.min = 10
            compare(invalid.max, 11)
            invalid.titleOffset = 2
            compare(invalid.titleOffset, 0)
        }
    }
}
