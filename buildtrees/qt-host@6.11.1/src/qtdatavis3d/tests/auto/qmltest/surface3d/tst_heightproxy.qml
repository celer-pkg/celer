// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    HeightMapSurfaceDataProxy {
        id: initial
    }

    HeightMapSurfaceDataProxy {
        id: initialized
        heightMapFile: ":/customtexture.jpg"
        maxXValue: 10.0
        maxZValue: 10.0
        minXValue: -10.0
        minZValue: -10.0
    }

    HeightMapSurfaceDataProxy {
        id: change
    }

    HeightMapSurfaceDataProxy {
        id: invalid
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
            verify(!initial.series)

            compare(initial.type, AbstractDataProxy.DataTypeSurface)
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
            change.maxXValue = 10.0
            change.maxZValue = 10.0
            change.minXValue = -10.0
            change.minZValue = -10.0
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.heightMapFile, ":/customtexture.jpg")
            compare(change.maxXValue, 10.0)
            compare(change.maxZValue, 10.0)
            compare(change.minXValue, -10.0)
            compare(change.minZValue, -10.0)

            compare(change.columnCount, 24)
            compare(change.rowCount, 24)
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
}
