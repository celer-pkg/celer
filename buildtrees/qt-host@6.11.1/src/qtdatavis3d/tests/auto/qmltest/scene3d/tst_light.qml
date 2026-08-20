// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.3
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Light3D {
        id: initial
    }

    Light3D {
        id: initialized
        autoPosition: true
    }


    Light3D {
        id: change
        autoPosition: true
    }

    TestCase {
        name: "Light3D Initial"

        function test_initial() {
            compare(initial.autoPosition, false)
        }
    }

    TestCase {
        name: "Light3D Initialized"

        function test_initialized() {
            compare(initialized.autoPosition, true)
        }
    }

    TestCase {
        name: "Light3D Change"

        function test_change() {
            compare(change.autoPosition, true)
            change.autoPosition = false
            compare(change.autoPosition, false)
        }
    }
}
