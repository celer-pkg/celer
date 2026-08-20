// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    ValueAxis3D {
        id: initial
    }

    ValueAxis3D {
        id: initialized
        formatter: ValueAxis3DFormatter { objectName: "formatter1" }
        labelFormat: "%f"
        reversed: true
        segmentCount: 10
        subSegmentCount: 5

        autoAdjustRange: false
        labelAutoRotation: 10.0
        max: 20
        min: -10
        title: "initialized"
        titleFixed: false
        titleVisible: true
    }

    ValueAxis3D {
        id: change
    }

    ValueAxis3D {
        id: invalid
    }

    TestCase {
        name: "ValueAxis3D Initial"

        function test_initial() {
            verify(initial.formatter)
            compare(initial.labelFormat, "%.2f")
            compare(initial.reversed, false)
            compare(initial.segmentCount, 5)
            compare(initial.subSegmentCount, 1)

            compare(initial.autoAdjustRange, true)
            compare(initial.labelAutoRotation, 0.0)
            compare(initial.max, 10)
            compare(initial.min, 0)
            compare(initial.orientation, AbstractAxis3D.AxisOrientationNone)
            compare(initial.title, "")
            compare(initial.titleFixed, true)
            compare(initial.titleVisible, false)
            compare(initial.type, AbstractAxis3D.AxisTypeValue)
        }
    }

    TestCase {
        name: "ValueAxis3D Initialized"

        function test_initialized() {
            compare(initialized.formatter.objectName, "formatter1")
            compare(initialized.labelFormat, "%f")
            compare(initialized.reversed, true)
            compare(initialized.segmentCount, 10)
            compare(initialized.subSegmentCount, 5)

            compare(initialized.autoAdjustRange, false)
            compare(initialized.labelAutoRotation, 10.0)
            compare(initialized.max, 20)
            compare(initialized.min, -10)
            compare(initialized.title, "initialized")
            compare(initialized.titleFixed, false)
            compare(initialized.titleVisible, true)
        }
    }

    TestCase {
        name: "ValueAxis3D Change"

        ValueAxis3DFormatter { id: formatter1 }

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
            change.labelAutoRotation = 10.0
            change.max = 20
            change.min = -10
            change.title = "initialized"
            change.titleFixed = false
            change.titleVisible = true

            compare(change.autoAdjustRange, false)
            compare(change.labelAutoRotation, 10.0)
            compare(change.max, 20)
            compare(change.min, -10)
            compare(change.title, "initialized")
            compare(change.titleFixed, false)
            compare(change.titleVisible, true)
        }
    }

    TestCase {
        name: "ValueAxis3D Invalid"

        function test_invalid() {
            invalid.segmentCount = -1
            compare(invalid.segmentCount, 1)
            invalid.subSegmentCount = -1
            compare(invalid.subSegmentCount, 1)

            invalid.labelAutoRotation = -10
            compare(invalid.labelAutoRotation, 0.0)
            invalid.labelAutoRotation = 100
            compare(invalid.labelAutoRotation, 90.0)
            invalid.max = -10
            compare(invalid.min, -11)
            invalid.min = 10
            compare(invalid.max, 11)
        }
    }
}
