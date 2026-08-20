// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Surface3DSeries {
        dataProxy: HeightMapSurfaceDataProxy {
            id: initial
        }
    }

    Surface3DSeries {
        dataProxy: HeightMapSurfaceDataProxy {
            id: initialized
            heightMapFile: ":/customtexture.jpg"
            maxXValue: 10.0
            maxZValue: 10.0
            minXValue: -10.0
            minZValue: -10.0
        }
    }

    Surface3DSeries {
        dataProxy: HeightMapSurfaceDataProxy {
            id: change
        }
    }
    Surface3DSeries {
        dataProxy: HeightMapSurfaceDataProxy {
            id: invalid
        }
    }

    TestCase {
        name: "HeightMapSurfaceDataProxy Initial"

        function test_initial() {
            compare(initial.heightMapFile, "")
            compare(initial.maxXValue, 10.0)
            compare(initial.maxZValue, 10.0)
            compare(initial.minXValue, 0)
            compare(initial.minZValue, 0)

            compare(initial.columnCount, 0)
            compare(initial.rowCount, 0)
            verify(initial.series)

            compare(initial.type, AbstractDataProxy.DataType.Surface)
        }
    }

    TestCase {
        name: "HeightMapSurfaceDataProxy Initialized"

        function test_initialized() {
            compare(initialized.heightMapFile, ":/customtexture.jpg")
            compare(initialized.maxXValue, 10.0)
            compare(initialized.maxZValue, 10.0)
            compare(initialized.minXValue, -10.0)
            compare(initialized.minZValue, -10.0)

            compare(initialized.columnCount, 24)
            compare(initialized.rowCount, 24)
        }
    }

    TestCase {
        name: "HeightMapSurfaceDataProxy Change"

        function test_1_change() {
            change.heightMapFile = ":/customtexture.jpg"
            change.maxXValue = 11.0
            change.maxZValue = 11.0
            change.maxYValue = 11.0
            change.minXValue = -10.0
            change.minZValue = -10.0
            change.minYValue = -10.0
            change.autoScaleY = true
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.heightMapFile, ":/customtexture.jpg")
            compare(change.maxXValue, 11.0)
            compare(change.maxZValue, 11.0)
            compare(change.maxYValue, 11.0)
            compare(change.minXValue, -10.0)
            compare(change.minZValue, -10.0)
            compare(change.minYValue, -10.0)

            compare(change.columnCount, 24)
            compare(change.rowCount, 24)

            // Signals
            compare(heightMapSpy.count, 1)
            compare(heightMapFileSpy.count, 1)
            compare(maxXValueSpy.count, 1)
            compare(maxZValueSpy.count, 1)
            compare(maxYValueSpy.count, 1)
            compare(minXValueSpy.count, 1)
            compare(minZValueSpy.count, 1)
            compare(minYValueSpy.count, 1)
            compare(autoScaleYSpy.count, 1)
        }
    }

    TestCase {
        name: "HeightMapSurfaceDataProxy Invalid"

        function test_invalid() {
            invalid.maxXValue = -10
            compare(invalid.minXValue, -11)
            invalid.minZValue = 20
            compare(invalid.maxZValue, 21)
        }
    }

    SignalSpy {
        id: heightMapSpy
        target: change
        signalName: "heightMapChanged"
    }

    SignalSpy {
        id: heightMapFileSpy
        target: change
        signalName: "heightMapFileChanged"
    }

    SignalSpy {
        id: minXValueSpy
        target: change
        signalName: "minXValueChanged"
    }

    SignalSpy {
        id: minYValueSpy
        target: change
        signalName: "minYValueChanged"
    }

    SignalSpy {
        id: minZValueSpy
        target: change
        signalName: "minZValueChanged"
    }

    SignalSpy {
        id: maxXValueSpy
        target: change
        signalName: "maxXValueChanged"
    }

    SignalSpy {
        id: maxYValueSpy
        target: change
        signalName: "maxYValueChanged"
    }

    SignalSpy {
        id: maxZValueSpy
        target: change
        signalName: "maxZValueChanged"
    }

    SignalSpy {
        id: autoScaleYSpy
        target: change
        signalName: "autoScaleYChanged"
    }
}
