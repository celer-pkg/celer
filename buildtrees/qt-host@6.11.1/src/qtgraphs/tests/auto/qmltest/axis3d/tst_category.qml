// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Category3DAxis {
        id: initial
    }

    Category3DAxis {
        id: initialized
        labels: ["first", "second"]

        autoAdjustRange: false
        labelAutoAngle: 10.0
        max: 20
        min: 10
        title: "initialized"
        titleFixed: false
        titleVisible: true
        labelsVisible: false
        titleOffset: 1
        scaleLabelsByCount: true
        labelSize: 2
    }

    Category3DAxis {
        id: change
    }

    Category3DAxis {
        id: invalid
    }

    TestCase {
        name: "Category3DAxis Initial"

        function test_initial() {
            compare(initial.labels.length, 0)

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
            compare(initial.scaleLabelsByCount, false)
            compare(initial.labelSize, 1)
            compare(initial.type, Abstract3DAxis.AxisType.Category)
        }
    }

    TestCase {
        name: "Category3DAxis Initialized"

        function test_initialized() {
            compare(initialized.labels.length, 2)
            compare(initialized.labels[0], "first")
            compare(initialized.labels[1], "second")

            compare(initialized.autoAdjustRange, false)
            compare(initialized.labelAutoAngle, 10.0)
            compare(initialized.max, 20)
            compare(initialized.min, 10)
            compare(initialized.title, "initialized")
            compare(initialized.titleFixed, false)
            compare(initialized.titleVisible, true)
            compare(change.labelsVisible, false)
            compare(initialized.titleOffset, 1)
            compare(initialized.scaleLabelsByCount, true)
            compare(initialized.labelSize, 2)
        }
    }

    TestCase {
        name: "Category3DAxis Change"

        function test_change() {
            change.labels = ["first"]
            compare(change.labels.length, 1)
            compare(change.labels[0], "first")
            change.labels = ["first", "second"]
            compare(change.labels.length, 2)
            compare(change.labels[0], "first")
            compare(change.labels[1], "second")
            change.labels[1] = "another"
            compare(change.labels[1], "another")

            change.autoAdjustRange = false
            change.labelAutoAngle = 10.0
            change.max = 20
            change.min = 10
            change.title = "initialized"
            change.titleFixed = false
            change.titleVisible = true
            change.labelsVisible = false
            change.titleOffset = -1
            change.scaleLabelsByCount = true
            change.labelSize = 2

            compare(change.autoAdjustRange, false)
            compare(change.labelAutoAngle, 10.0)
            compare(change.max, 20)
            compare(change.min, 10)
            compare(change.title, "initialized")
            compare(change.titleFixed, false)
            compare(change.titleVisible, true)
            compare(change.labelsVisible, false)
            compare(change.titleOffset, -1)
            compare(change.scaleLabelsByCount, true)
            compare(change.labelSize, 2)
        }
    }

    TestCase {
        name: "Category3DAxis Invalid"

        function test_invalid() {
            invalid.labelAutoAngle = -10
            compare(invalid.labelAutoAngle, 0.0)
            invalid.labelAutoAngle = 100
            compare(invalid.labelAutoAngle, 90.0)
            invalid.max = -10
            compare(invalid.min, 0)
            invalid.min = 10
            compare(invalid.max, 11)
            invalid.titleOffset = 2
            compare(invalid.titleOffset, 0)

        }
    }
}
