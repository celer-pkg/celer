
// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtGraphs
import QtTest

Item {
    width: 400
    height: 300

    PieSeries {
        id: pieSeries
        name: "pie"

        SignalSpy {
            id: addedSpy
            target: pieSeries
            signalName: "added"
        }
        SignalSpy {
            id: removedSpy
            target: pieSeries
            signalName: "removed"
        }
        SignalSpy {
            id: sumChangedSpy
            target: pieSeries
            signalName: "sumChanged"
        }
        SignalSpy {
            id: countChangedSpy
            target: pieSeries
            signalName: "countChanged"
        }
        SignalSpy {
            id: spanChangedSpy
            target: pieSeries
            signalName: "angleSpanVisibleLimitChanged"
        }
        SignalSpy {
            id: modeChangedSpy
            target: pieSeries
            signalName: "angleSpanLabelVisibilityChanged"
        }
    }

    TestCase {
        name: "tst_qml-qtquicktest PieSeries"

        function test_01_properties() {
            compare(pieSeries.endAngle, 360)
            compare(pieSeries.holeSize, 0)
            compare(pieSeries.horizontalPosition, 0.5)
            compare(pieSeries.pieSize, 0.7)
            compare(pieSeries.startAngle, 0)
            compare(pieSeries.sum, 0)
            compare(pieSeries.verticalPosition, 0.5)
            compare(pieSeries.angleSpanVisibleLimit, 0)
            compare(pieSeries.angleSpanLabelVisibility, PieSeries.LabelVisibility.First)

            pieSeries.clear()
        }

        function test_02_sliceproperties() {
            var slice = pieSeries.append("slice", 10)
            compare(slice.angleSpan, 360.0)
            verify(slice.borderColor !== undefined)
            compare(slice.borderWidth, 0)
            verify(slice.color !== undefined)
            compare(slice.explodeDistanceFactor, 0.15)
            compare(slice.exploded, false)
            compare(slice.label, "slice")
            compare(slice.labelArmLengthFactor, 0.15)
            verify(slice.labelColor !== undefined)
            compare(slice.labelFont.bold, false)
            compare(slice.labelPosition, PieSlice.LabelPosition.Outside)
            compare(slice.labelVisible, false)
            compare(slice.percentage, 1.0)
            compare(slice.startAngle, 0.0)
            compare(slice.value, 10.0)

            pieSeries.clear()
        }

        function test_03_take() {
            var count = 20
            for (var i = 0; i < count; i++)
                pieSeries.append("slice" + i, Math.random() + 0.01)

            verify(pieSeries.take(pieSeries.find("slice" + 5)))
            verify(pieSeries.take(pieSeries.find("slice" + 6)))
            compare(pieSeries.count, 18)

            pieSeries.clear()
        }

        function test_04_append() {
            addedSpy.clear()
            countChangedSpy.clear()
            sumChangedSpy.clear()
            var count = 50
            for (var i = 0; i < count; i++)
                pieSeries.append("slice" + i,
                                 Math.random() + 0.01) // Add 0.01 to avoid zero
            compare(addedSpy.count, count)
            compare(countChangedSpy.count, count)
            compare(sumChangedSpy.count, count)
            pieSeries.clear()
        }

        function test_05_remove() {
            removedSpy.clear()
            countChangedSpy.clear()
            sumChangedSpy.clear()

            var count = 50
            for (var i = 0; i < count; i++)
                pieSeries.append("slice" + i,
                                 Math.random() + 0.01) // Add 0.01 to avoid zero
            for (var j = 0; j < 10; j++)
                pieSeries.remove(pieSeries.at(0))

            compare(removedSpy.count, 10)
            compare(countChangedSpy.count, count + 10)
            compare(sumChangedSpy.count, count + 10)
            compare(pieSeries.count, 40)

            for (var j = 0; j < 10; j++)
                pieSeries.remove(0)

            compare(removedSpy.count, 20)
            compare(countChangedSpy.count, count + 20)
            compare(sumChangedSpy.count, count + 20)
            compare(pieSeries.count, 30)

            pieSeries.removeMultiple(0, 10)
            compare(removedSpy.count, 21)
            compare(countChangedSpy.count, count + 21)
            compare(sumChangedSpy.count, count + 30)
            compare(pieSeries.count, 20)

            pieSeries.clear()
        }

        function test_06_find_and_at() {
            var count = 50
            for (var i = 0; i < count; i++)
                pieSeries.append("slice" + i,
                                 Math.random() + 0.01) // Add 0.01 to avoid zero
            for (var j = 0; j < count; j++) {
                compare(pieSeries.find("slice" + j).label, "slice" + j)
            }
            pieSeries.clear()
        }

        function test_07_adjust_limit_and_mode() {
            modeChangedSpy.clear()
            spanChangedSpy.clear()

            var count = 10
            for (let i = 0; i < count; i++)
                pieSeries.append("slice" + i, i + 0.1) // Produces angle span range from ~0.8 to ~71.2

            pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.None
            compare(modeChangedSpy.count, 1)

            pieSeries.angleSpanVisibleLimit = 20
            compare(spanChangedSpy.count, 1)

            var visiblecount = 0
            for (let i = 0; i < count; i++)
                visiblecount += pieSeries.at(i).labelVisible
            compare(visiblecount, 7)

            pieSeries.angleSpanVisibleLimit = 35
            compare(spanChangedSpy.count, 2)

            visiblecount = 0
            for (let i = 0; i < count; i++)
                visiblecount += pieSeries.at(i).labelVisible
            compare(visiblecount, 5)

            pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.First
            compare(modeChangedSpy.count, 2)

            pieSeries.angleSpanVisibleLimit = 75 // This covers all the slices
            compare(spanChangedSpy.count, 3)

            visiblecount = 0
            for (let i = 0; i < count; i++)
                visiblecount += pieSeries.at(i).labelVisible
            compare(visiblecount, 1)

            pieSeries.angleSpanLabelVisibility = PieSeries.LabelVisibility.Even
            compare(modeChangedSpy.count, 3)

            visiblecount = 0
            for (let i = 0; i < count; i++)
                visiblecount += pieSeries.at(i).labelVisible
            compare(visiblecount, 5)

            pieSeries.clear()
        }
    }
}
