// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtGraphs
import QtTest 1.1

Item {
    id: top
    height: 150
    width: 150

    // Scene3D is uncreatable, so it needs to be accessed via a graph
    Bars3D {
        id: initial
    }

    Bars3D {
        id: initialized
        scene.devicePixelRatio: Screen.devicePixelRatio
        scene.graphPositionQuery: Qt.point(0, 0)
        scene.primarySubViewport: Qt.rect(0, 0, 50, 50)
        scene.secondarySubViewport: Qt.rect(50, 50, 100, 100)
        scene.secondarySubviewOnTop: false
        scene.selectionQueryPosition: Qt.point(0, 0)
        scene.slicingActive: true
    }

    Bars3D {
        id: change
    }

    Bars3D {
        id: invalid
    }

    TestCase {
        name: "Scene3D Initial"

        function test_initial() {
            compare(initial.scene.devicePixelRatio, Screen.devicePixelRatio)
            compare(initial.scene.graphPositionQuery, Qt.point(-1, -1))
            compare(initial.scene.invalidSelectionPoint, Qt.point(-1, -1))
            compare(initial.scene.primarySubViewport.x, 0)
            compare(initial.scene.primarySubViewport.y, 0)
            compare(initial.scene.primarySubViewport.width, 0)
            compare(initial.scene.primarySubViewport.height, 0)
            compare(initial.scene.secondarySubViewport.x, 0)
            compare(initial.scene.secondarySubViewport.y, 0)
            compare(initial.scene.secondarySubViewport.width, 0)
            compare(initial.scene.secondarySubViewport.height, 0)
            compare(initial.scene.secondarySubviewOnTop, true)
            compare(initial.scene.selectionQueryPosition, Qt.point(-1, -1))
            compare(initial.scene.slicingActive, false)
            compare(initial.scene.viewport.x, 0)
            compare(initial.scene.viewport.y, 0)
            compare(initial.scene.viewport.width, 0)
            compare(initial.scene.viewport.height, 0)
        }
    }

    TestCase {
        name: "Scene3D Initialized"

        function test_initialized() {
            compare(initialized.scene.devicePixelRatio, Screen.devicePixelRatio)
            compare(initialized.scene.graphPositionQuery, Qt.point(0, 0))
            compare(initialized.scene.primarySubViewport.x, 0)
            compare(initialized.scene.primarySubViewport.y, 0)
            compare(initialized.scene.primarySubViewport.width, 50)
            compare(initialized.scene.primarySubViewport.height, 50)
            compare(initialized.scene.secondarySubViewport.x, 50)
            compare(initialized.scene.secondarySubViewport.y, 50)
            compare(initialized.scene.secondarySubViewport.width, 100)
            compare(initialized.scene.secondarySubViewport.height, 100)
            compare(initialized.scene.secondarySubviewOnTop, false)
            compare(initialized.scene.selectionQueryPosition, Qt.point(0, 0))
            compare(initialized.scene.slicingActive, true)
        }
    }

    TestCase {
        name: "Scene3D Change"

        function test_change() {
            change.scene.devicePixelRatio = 2.0
            change.scene.graphPositionQuery = Qt.point(0, 0)
            change.scene.primarySubViewport = Qt.rect(0, 0, 50, 50)
            change.scene.secondarySubViewport = Qt.rect(50, 50, 100, 100)
            change.scene.secondarySubviewOnTop = false
            change.scene.selectionQueryPosition = Qt.point(0, 0) // TODO: When doing signal checks, add tests to check that queries return something (asynchronously)
            change.scene.slicingActive = true

            compare(change.scene.devicePixelRatio, Screen.devicePixelRatio)
            compare(change.scene.graphPositionQuery, Qt.point(0, 0))
            compare(change.scene.primarySubViewport.x, 0)
            compare(change.scene.primarySubViewport.y, 0)
            compare(change.scene.primarySubViewport.width, 50)
            compare(change.scene.primarySubViewport.height, 50)
            compare(change.scene.secondarySubViewport.x, 50)
            compare(change.scene.secondarySubViewport.y, 50)
            compare(change.scene.secondarySubViewport.width, 100)
            compare(change.scene.secondarySubViewport.height, 100)
            compare(change.scene.secondarySubviewOnTop, false)
            compare(change.scene.selectionQueryPosition, Qt.point(0, 0))
            compare(change.scene.slicingActive, true)

            // Signals
            compare(devicePixelRatioSpy.count, 2)
            compare(graphPosQuerySpy.count, 1)
            compare(primarySubViewportSpy.count, 1)
            compare(secondarySubViewportSpy.count, 1)
            compare(secondarySubViewOnTopSpy.count, 1)
            compare(selectionQueryPosSpy.count, 1)
            compare(slicingActiveSpy.count, 1)
        }
    }

    TestCase {
        name: "Scene3D Invalid"

        function test_invalid() {
            invalid.scene.primarySubViewport = Qt.rect(0, 0, -50, -50)
            compare(invalid.scene.primarySubViewport.x, 0)
            compare(invalid.scene.primarySubViewport.y, 0)
            compare(invalid.scene.primarySubViewport.width, 0)
            compare(invalid.scene.primarySubViewport.height, 0)
        }
    }

    SignalSpy {
        id: primarySubViewportSpy
        target: change.scene
        signalName: "primarySubViewportChanged"
    }

    SignalSpy {
        id: secondarySubViewportSpy
        target: change.scene
        signalName: "secondarySubViewportChanged"
    }

    SignalSpy {
        id: selectionQueryPosSpy
        target: change.scene
        signalName: "selectionQueryPositionChanged"
    }

    SignalSpy {
        id: secondarySubViewOnTopSpy
        target: change.scene
        signalName: "secondarySubviewOnTopChanged"
    }

    SignalSpy {
        id: slicingActiveSpy
        target: change.scene
        signalName: "slicingActiveChanged"
    }

    SignalSpy {
        id: devicePixelRatioSpy
        target: change.scene
        signalName: "devicePixelRatioChanged"
    }

    SignalSpy {
        id: graphPosQuerySpy
        target: change.scene
        signalName: "graphPositionQueryChanged"
    }
}
