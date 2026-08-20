// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    ThemeColor {
        id: initial
    }

    ThemeColor {
        id: initialized
        color: "red"
    }

    ThemeColor {
        id: change
    }

    TestCase {
        name: "ThemeColor Initial"

        function test_initial() {
            compare(initial.color, "#000000")
        }
    }

    TestCase {
        name: "ThemeColor Initialized"

        function test_initialized() {
            compare(initialized.color, "#ff0000")
        }
    }

    TestCase {
        name: "ThemeColor Change"

        function test_change() {
            change.color = "blue"

            compare(change.color, "#0000ff")
        }
    }
}
