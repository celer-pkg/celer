// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Value3DAxis {
        id: changedX
    }
    Value3DAxis {
        id: changedY
    }
    Value3DAxis {
        id: changedZ
    }

    Category3DAxis {
        id: changedRow
    }
    Category3DAxis {
        id: changedColumn
    }
    Value3DAxis {
        id: changedValue
    }

    Surface3D {
        id: surface
        anchors.fill: parent
        Surface3DSeries {
            id: surfInit

            axisX: Value3DAxis {
                id: initialX
            }
            axisY: Value3DAxis {
                id: initialY
            }
            axisZ: Value3DAxis {
                id: initialZ
            }
        }

        Surface3DSeries {
            id: surfChange
        }
    }

    Bars3D {
        id: barsInit
        anchors.fill: parent

        Bar3DSeries {
            id: barInit

            rowAxis: Category3DAxis {
                id: initialRow
            }
            columnAxis: Category3DAxis {
                id: initialColumn
            }
            valueAxis: Value3DAxis {
                id: initialValue
            }
        }

        Bar3DSeries {
            id: barChange
        }
    }

    TestCase {
        name: "Series-Axis 3D Initial"

        function test_inital() {
            compare(surfInit.axisX, initialX)
            compare(surfInit.axisY, initialY)
            compare(surfInit.axisZ, initialZ)

            compare(barInit.rowAxis, initialRow)
            compare(barInit.columnAxis, initialColumn)
            compare(barInit.valueAxis, initialValue)
        }
    }

    TestCase {
        name: "Series-Axis 3D Change"

        function test_changed() {
            surfChange.axisX = changedX
            surfChange.axisY = changedY
            surfChange.axisZ = changedZ

            barChange.rowAxis = changedRow
            barChange.columnAxis = changedColumn
            barChange.valueAxis = changedValue

            compare(surfChange.axisX, changedX)
            compare(surfChange.axisY, changedY)
            compare(surfChange.axisZ, changedZ)

            compare(barChange.rowAxis, changedRow)
            compare(barChange.columnAxis, changedColumn)
            compare(barChange.valueAxis, changedValue)

            compare(surfXSpy.count, 1)
            compare(surfYSpy.count, 1)
            compare(surfZSpy.count, 1)

            compare(barRowSpy.count, 1)
            compare(barColumnSpy.count, 1)
            compare(barValueSpy.count, 1)

            surfChange.axisX = undefined
            surfChange.axisY = undefined
            surfChange.axisZ = undefined

            barChange.rowAxis = undefined
            barChange.columnAxis = undefined
            barChange.valueAxis = undefined

            compare(surfChange.axisX, null)
            compare(surfChange.axisY, null)
            compare(surfChange.axisZ, null)

            compare(barChange.rowAxis, null)
            compare(barChange.columnAxis, null)
            compare(barChange.valueAxis, null)

            compare(surfXSpy.count, 2)
            compare(surfYSpy.count, 2)
            compare(surfZSpy.count, 2)

            compare(barRowSpy.count, 2)
            compare(barColumnSpy.count, 2)
            compare(barValueSpy.count, 2)

        }
    }

    SignalSpy {
        id: surfXSpy
        target: surfChange
        signalName: "axisXChanged"
    }

    SignalSpy {
        id: surfYSpy
        target: surfChange
        signalName: "axisYChanged"
    }

    SignalSpy {
        id: surfZSpy
        target: surfChange
        signalName: "axisZChanged"
    }

    SignalSpy {
        id: barRowSpy
        target: barChange
        signalName: "rowAxisChanged"
    }

    SignalSpy {
        id: barColumnSpy
        target: barChange
        signalName: "columnAxisChanged"
    }

    SignalSpy {
        id: barValueSpy
        target: barChange
        signalName: "valueAxisChanged"
    }
}
