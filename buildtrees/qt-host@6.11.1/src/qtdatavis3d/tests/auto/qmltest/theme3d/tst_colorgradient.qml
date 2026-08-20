// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    ColorGradient {
        id: initial
    }

    ColorGradient {
        id: initialized
        stops: [
            ColorGradientStop { color: "blue"; position: 0 },
            ColorGradientStop { color: "white"; position: 0.5 },
            ColorGradientStop { color: "red"; position: 1 }
        ]
    }

    ColorGradient {
        id: change
    }

    TestCase {
        name: "ColorGradient Initial"

        function test_initial() {
            compare(initial.stops.length, 0)
        }
    }

    TestCase {
        name: "ColorGradient Initialized"

        function test_initialized() {
            compare(initialized.stops.length, 3)
            compare(initialized.stops[0].color, "#0000ff")
            compare(initialized.stops[1].color, "#ffffff")
            compare(initialized.stops[2].color, "#ff0000")
        }
    }

    TestCase {
        name: "ColorGradient Change"

        ColorGradientStop { id: stop1; color: "blue"; position: 0 }
        ColorGradientStop { id: stop2; color: "red"; position: 1.0 }
        ColorGradientStop { id: stop3; color: "white"; position: 0.5 }

        function test_change() {
            change.stops = [stop1]
            compare(change.stops.length, 1)
            change.stops = [stop1, stop2]
            compare(change.stops.length, 2)
            compare(change.stops[0].color, "#0000ff")
            change.stops[0].color = "red"
            compare(change.stops[0].color, "#ff0000")
            compare(change.stops[1].color, "#ff0000")
            change.stops = [stop1, stop2, stop3]
            compare(change.stops[2].color, "#ffffff")
            compare(change.stops.length, 3)
            stop2.position = 0.25
            stop3.position = 1.0
            compare(change.stops[1].position, 0.25)
            compare(change.stops[2].position, 1.0)
        }
    }
}
