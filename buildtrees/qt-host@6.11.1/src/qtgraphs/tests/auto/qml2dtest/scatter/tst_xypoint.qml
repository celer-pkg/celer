// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest

Item {
    id: top
    height: 150
    width: 150

    XYPoint {
        id: initial
    }

    XYPoint {
        id: initialized
        x: 10
        y: 20
    }

    TestCase {
        name: "XYPoint Initial"

        function test_1_initial() {
            compare(initial.x, 0)
            compare(initial.y, 0)
        }

        function test_2_initial_change() {
            initial.x = 1
            initial.y = 2

            compare(initial.x, 1)
            compare(initial.y, 2)
        }
    }

    TestCase {
        name: "XYPoint Initialized"

        function test_1_initialized() {
            compare(initialized.x, 10)
            compare(initialized.y, 20)
        }

        function test_2_initialized_change() {
            initialized.x = 3
            initialized.y = 9

            compare(initialized.x, 3)
            compare(initialized.y, 9)
        }
    }
}
