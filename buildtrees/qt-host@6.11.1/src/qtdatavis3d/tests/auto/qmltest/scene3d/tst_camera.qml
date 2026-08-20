// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    Camera3D {
        id: initial
    }

    Camera3D {
        id: initialized
        maxZoomLevel: 1000.0
        minZoomLevel: 100.0
        target: Qt.vector3d(1.0, -1.0, 1.0)
        wrapXRotation: false
        wrapYRotation: true
        xRotation: 30.0
        yRotation: 30.0
        zoomLevel: 500.0
    }

    Camera3D {
        id: change
    }

    Camera3D {
        id: invalid
    }

    TestCase {
        name: "Camera3D Initial"

        function test_initial() {
            compare(initial.cameraPreset, Camera3D.CameraPresetNone)
            compare(initial.maxZoomLevel, 500.0)
            compare(initial.minZoomLevel, 10.0)
            compare(initial.target, Qt.vector3d(0.0, 0.0, 0.0))
            compare(initial.wrapXRotation, true)
            compare(initial.wrapYRotation, false)
            compare(initial.xRotation, 0.0)
            compare(initial.yRotation, 0.0)
            compare(initial.zoomLevel, 100.0)
        }
    }

    TestCase {
        name: "Camera3D Initialized"

        function test_initialized() {
            compare(initialized.maxZoomLevel, 1000.0)
            compare(initialized.minZoomLevel, 100.0)
            compare(initialized.target, Qt.vector3d(1.0, -1.0, 1.0))
            compare(initialized.wrapXRotation, false)
            compare(initialized.wrapYRotation, true)
            compare(initialized.xRotation, 30.0)
            compare(initialized.yRotation, 30.0)
            compare(initialized.zoomLevel, 500.0)
        }
    }

    TestCase {
        name: "Camera3D Change"

        function test_1_change() {
            change.cameraPreset = Camera3D.CameraPresetBehind // Will be overridden by the the following sets
            change.maxZoomLevel = 1000.0
            change.minZoomLevel = 100.0
            change.target = Qt.vector3d(1.0, -1.0, 1.0)
            change.wrapXRotation = false
            change.wrapYRotation = true
            change.xRotation = 30.0
            change.yRotation = 30.0
            change.zoomLevel = 500.0

            compare(change.cameraPreset, Camera3D.CameraPresetNone)
            compare(change.maxZoomLevel, 1000.0)
            compare(change.minZoomLevel, 100.0)
            compare(change.target, Qt.vector3d(1.0, -1.0, 1.0))
            compare(change.wrapXRotation, false)
            compare(change.wrapYRotation, true)
            compare(change.xRotation, 30.0)
            compare(change.yRotation, 30.0)
            compare(change.zoomLevel, 500.0)
        }

        function test_2_change_preset() {
            change.cameraPreset = Camera3D.CameraPresetBehind // Sets target and rotations

            compare(change.cameraPreset, Camera3D.CameraPresetBehind)
            compare(change.maxZoomLevel, 1000.0)
            compare(change.minZoomLevel, 100.0)
            compare(change.target, Qt.vector3d(0.0, 0.0, 0.0))
            compare(change.wrapXRotation, false)
            compare(change.wrapYRotation, true)
            compare(change.xRotation, 180.0)
            compare(change.yRotation, 22.5)
            compare(change.zoomLevel, 500.0)
        }
    }

    TestCase {
        name: "Camera3D Invalid"

        function test_invalid() {
            invalid.target = Qt.vector3d(-1.5, -1.5, -1.5)
            compare(invalid.target, Qt.vector3d(-1.0, -1.0, -1.0))
            invalid.target = Qt.vector3d(1.5, 1.5, 1.5)
            compare(invalid.target, Qt.vector3d(1.0, 1.0, 1.0))
            invalid.minZoomLevel = 0.1
            compare(invalid.minZoomLevel, 1.0)
        }
    }
}
