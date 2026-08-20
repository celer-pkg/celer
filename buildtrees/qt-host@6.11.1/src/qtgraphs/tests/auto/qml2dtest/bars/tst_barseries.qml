// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    BarSeries {
        id: initial
    }

    BarSeries {
        id: initialized

        barWidth: 0.2
        labelsVisible: true
        labelsFormat: "i"
        labelsPosition: BarSeries.LabelsPosition.OutsideEnd
        labelsMargin: 15.0
        labelsAngle: 25.0
        labelsPrecision: 10
        barsType: BarSeries.BarsType.Stacked

        name: "BarSeries"
        visible: false
        selectable: true
        hoverable: true
        opacity: 0.75
        valuesMultiplier: 0.75

        BarSet { label: "Set1"; values: [1, 2, 3, 4, 5, 6] }
    }

    Component {
        id: customBarComponent
        Item {
            id: comp
            property color barColor
            property color barBorderColor
            property real barBorderWidth
            Rectangle {
                anchors.fill: parent
                color: comp.barColor
                border.color: comp.barBorderColor
                border.width: comp.barBorderWidth
            }
        }
    }

    TestCase {
        name: "BarSeries Initial"

        function test_1_initial_common() {
            // Common properties from QBarSeries
            compare(initial.barWidth, 0.5)
            compare(initial.count, 0)
            compare(initial.labelsVisible, false)
            compare(initial.labelsFormat, "")
            compare(initial.labelsPosition, BarSeries.LabelsPosition.Center)
            compare(initial.labelsMargin, 0)
            compare(initial.labelsAngle, 0)
            compare(initial.labelsPrecision, 6)
            compare(initial.seriesColors, [])
            compare(initial.borderColors, [])
            compare(initial.barsType, BarSeries.BarsType.Groups)

            // Properties from QAbstractSeries
            compare(initial.name, "")
            compare(initial.visible, true)
            compare(initial.selectable, false)
            compare(initial.hoverable, false)
            compare(initial.opacity, 1.0)
            compare(initial.valuesMultiplier, 1.0)
            compare(initial.barDelegate, null)
        }

        function test_2_initial_change() {
            initial.barDelegate = customBarComponent

            initial.barWidth = 0.1
            // TODO: How to add a set dynamically?
            //initial.count = 1
            initial.labelsVisible = true
            initial.labelsFormat = "i"
            initial.labelsPosition = BarSeries.LabelsPosition.InsideBase
            initial.labelsMargin = 20.0
            initial.labelsAngle = 45.0
            initial.labelsPrecision = 3
            initial.barsType = BarSeries.BarsType.StackedPercent

            initial.name = "Bars"
            initial.visible = false
            initial.selectable = true
            initial.hoverable = true
            initial.opacity = 0.5
            initial.valuesMultiplier = 0.0

            compare(initial.barDelegate, customBarComponent)

            compare(initial.barWidth, 0.1)
            // TODO: How to add a set dynamically?
            //compare(initial.count, 1)
            compare(initial.labelsVisible, true)
            compare(initial.labelsFormat, "i")
            compare(initial.labelsPosition, BarSeries.LabelsPosition.InsideBase)
            compare(initial.labelsMargin, 20.0)
            compare(initial.labelsAngle, 45.0)
            compare(initial.labelsPrecision, 3)
            compare(initial.barsType, BarSeries.BarsType.StackedPercent)

            compare(initial.name, "Bars")
            compare(initial.visible, false)
            compare(initial.selectable, true)
            compare(initial.hoverable, true)
            compare(initial.opacity, 0.5)
            compare(initial.valuesMultiplier, 0.0)
        }
    }

    TestCase {
        name: "BarSeries Initialized"

        function test_1_initialized() {
            compare(initialized.barWidth, 0.2)
            compare(initialized.count, 1)
            compare(initialized.labelsVisible, true)
            compare(initialized.labelsFormat, "i")
            compare(initialized.labelsPosition, BarSeries.LabelsPosition.OutsideEnd)
            compare(initialized.labelsMargin, 15.0)
            compare(initialized.labelsAngle, 25.0)
            compare(initialized.labelsPrecision, 10)
            compare(initialized.barsType, BarSeries.BarsType.Stacked)

            compare(initialized.name, "BarSeries")
            compare(initialized.visible, false)
            compare(initialized.selectable, true)
            compare(initialized.hoverable, true)
            compare(initialized.opacity, 0.75)
            compare(initialized.valuesMultiplier, 0.75)
        }

        function test_2_initialized_change() {
            initialized.barWidth = 0.1
            // TODO: How to add a set dynamically?
            //initialized.count = 2
            initialized.labelsVisible = false
            initialized.labelsFormat = "d"
            initialized.labelsPosition = BarSeries.LabelsPosition.InsideBase
            initialized.labelsMargin = 35.0
            initialized.labelsAngle = 45.0
            initialized.labelsPrecision = 3
            initialized.barsType = BarSeries.BarsType.Groups
            initialized.seriesColors = ["Yellow", "Green", "Blue"]
            initialized.borderColors = ["Blue", "Yellow", "Green"]

            initialized.name = "Bars"
            initialized.visible = true
            initialized.selectable = false
            initialized.hoverable = false
            initialized.opacity = 0.5
            initialized.valuesMultiplier = 0.5

            compare(initialized.barWidth, 0.1)
            // TODO: How to add a set dynamically?
            //compare(initialized.count, 2)
            compare(initialized.labelsVisible, false)
            compare(initialized.labelsFormat, "d")
            compare(initialized.labelsPosition, BarSeries.LabelsPosition.InsideBase)
            compare(initialized.labelsMargin, 35.0)
            compare(initialized.labelsAngle, 45.0)
            compare(initialized.labelsPrecision, 3)
            compare(initialized.barsType, BarSeries.BarsType.Groups)

            compare(initialized.name, "Bars")
            compare(initialized.visible, true)
            compare(initialized.selectable, false)
            compare(initialized.hoverable, false)
            compare(initialized.opacity, 0.5)
            compare(initialized.valuesMultiplier, 0.5)

            // Signals
            compare(seriesColorsSpy.count, 1)
            compare(borderColorsSpy.count, 1)
            compare(barWidthSpy.count, 1)
            compare(labelsVisibleSpy.count, 1)
            compare(labelsFormatSpy.count, 1)
            compare(labelsPositionSpy.count, 2)
            compare(labelsMarginSpy.count, 1)
            compare(labelsAngleSpy.count, 1)
            compare(labelsPrecisionSpy.count, 1)
            compare(barsTypeSpy.count, 2)

            // Common
            compare(nameSpy.count, 1)
            compare(visibleSpy.count, 1)
            compare(selectableSpy.count, 1)
            compare(hoverableSpy.count, 1)
            compare(opacitySpy.count, 1)
            compare(valuesMultiplierSpy.count, 1)
        }

        function test_3_initialized_change_to_invalid() {
            initialized.barWidth = 2.0 // range 0...1
            initialized.valuesMultiplier = 2.0 // range 0...1

            compare(initialized.barWidth, 1.0)
            compare(initialized.valuesMultiplier, 1.0)

            initialized.barWidth = -1.0 // range 0...1
            initialized.valuesMultiplier = -1.0 // range 0...1
            compare(initialized.barWidth, 0.0)
            compare(initialized.valuesMultiplier, 0.0)
        }

        BarSet {id:b1}
        BarSet {id:b2}
        BarSet {id:b3}
        BarSet {id:b4}
        BarSet {id:b5}
        BarSet {id:b6}
        BarSet {id:b7}
        BarSet {id: newb6}
        BarSet {id: newb4}
        BarSet {id: replaceList4}
        BarSet {id: replaceList6}

        function test_5_modify_series() {
            // append
            let list = [b4, b5, b6]
            let newlist = [replaceList4, replaceList6]

            initialized.append(b1)
            initialized.append(b2)
            initialized.append(b3)
            initialized.append(list)
            compare(initialized.count, 7)
            compare(countSpy.count, 5)
            compare(barSetsSpy.count, 5)

            // insert
            initialized.insert(3, b7)
            compare(initialized.count, 8)
            compare(countSpy.count, 6)
            compare(barSetsSpy.count, 6)

            // at
            let atBar = initialized.at(5)
            compare(atBar, b4)

            // find
            let findIndex = initialized.find(b6)
            compare(findIndex, 7)

            // remove
            initialized.remove(b1)
            initialized.remove(0)
            compare(initialized.count, 6)
            compare(countSpy.count, 8)
            compare(barSetsSpy.count, 8)

            // remove multiple
            initialized.removeMultiple(0,3)
            compare(initialized.count, 3)
            compare(countSpy.count, 11)
            compare(barSetsSpy.count, 11)

            // take
            verify(initialized.take(b5))
            compare(initialized.count, 2)
            compare(countSpy.count, 12)
            compare(barSetsSpy.count, 12)

            //replace
            initialized.replace(b6, newb6)
            initialized.replace(0, newb4)
            compare(initialized.at(0), newb4)
            compare(initialized.at(1), newb6)
            initialized.replace(newlist)
            compare(initialized.barSets, newlist)
            compare(barSetsSpy.count, 16)
        }
    }

    SignalSpy {
        id: seriesColorsSpy
        target: initialized
        signalName: "seriesColorsChanged"
    }

    SignalSpy {
        id: borderColorsSpy
        target: initialized
        signalName: "borderColorsChanged"
    }

    SignalSpy {
        id: barsTypeSpy
        target: initialized
        signalName: "barsTypeChanged"
    }

    SignalSpy {
        id: barWidthSpy
        target: initialized
        signalName: "barWidthChanged"
    }

    SignalSpy {
        id: countSpy
        target: initialized
        signalName: "countChanged"
    }

    SignalSpy {
        id: labelsVisibleSpy
        target: initialized
        signalName: "labelsVisibleChanged"
    }

    SignalSpy {
        id: labelsFormatSpy
        target: initialized
        signalName: "labelsFormatChanged"
    }

    SignalSpy {
        id: labelsPositionSpy
        target: initialized
        signalName: "labelsPositionChanged"
    }

    SignalSpy {
        id: labelsMarginSpy
        target: initialized
        signalName: "labelsMarginChanged"
    }

    SignalSpy {
        id: labelsAngleSpy
        target: initialized
        signalName: "labelsAngleChanged"
    }

    SignalSpy {
        id: labelsPrecisionSpy
        target: initialized
        signalName: "labelsPrecisionChanged"
    }

    SignalSpy {
        id: barDelegateSpy
        target: initialized
        signalName: "barDelegateChanged"
    }

    SignalSpy {
        id: barSetsSpy
        target: initialized
        signalName: "barSetsChanged"
    }

    // Common
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
