// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    TouchInputHandler3D {
        id: initial
    }

    TouchInputHandler3D {
        id: initialized
        rotationEnabled: false
        selectionEnabled: false
        zoomAtTargetEnabled: false
        zoomEnabled: false
    }

    TouchInputHandler3D {
        id: change
    }

    TestCase {
        name: "TouchInputHandler3D Initial"

        function test_initial() {
            compare(initial.rotationEnabled, true)
            compare(initial.selectionEnabled, true)
            compare(initial.zoomAtTargetEnabled, true)
            compare(initial.zoomEnabled, true)
        }
    }

    TestCase {
        name: "TouchInputHandler3D Initialized"

        function test_initialized() {
            compare(initialized.rotationEnabled, false)
            compare(initialized.selectionEnabled, false)
            compare(initialized.zoomAtTargetEnabled, false)
            compare(initialized.zoomEnabled, false)
        }
    }

    TestCase {
        name: "TouchInputHandler3D Change"

        function test_change() {
            change.rotationEnabled = false
            change.selectionEnabled = false
            change.zoomAtTargetEnabled = false
            change.zoomEnabled = false

            compare(change.rotationEnabled, false)
            compare(change.selectionEnabled, false)
            compare(change.zoomAtTargetEnabled, false)
            compare(change.zoomEnabled, false)

            // TODO: QTRD-3380 (mouse events)
        }
    }
}
