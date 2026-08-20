// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    Custom3DItem {
        id: initial
    }

    Custom3DItem {
        id: initialized
        meshFile: ":\customitem.mesh"
        position: Qt.vector3d(1.0, 0.5, 1.0)
        positionAbsolute: true
        rotation: Qt.quaternion(1, 0.5, 0, 0)
        scaling: Qt.vector3d(0.2, 0.2, 0.2)
        scalingAbsolute: false
        shadowCasting: false
        textureFile: ":\customtexture.jpg"
        visible: false
    }

    Custom3DItem {
        id: change
    }

    TestCase {
        name: "Custom3DItem Initial"

        function test_initial() {
            compare(initial.meshFile, "")
            compare(initial.position, Qt.vector3d(0.0, 0.0, 0.0))
            compare(initial.positionAbsolute, false)
            compare(initial.rotation, Qt.quaternion(0, 0, 0, 0))
            compare(initial.scaling, Qt.vector3d(0.1, 0.1, 0.1))
            compare(initial.scalingAbsolute, true)
            compare(initial.shadowCasting, true)
            compare(initial.textureFile, "")
            compare(initial.visible, true)
        }
    }

    TestCase {
        name: "Custom3DItem Initialized"

        function test_initialized() {
            compare(initialized.meshFile, ":\customitem.mesh")
            compare(initialized.position, Qt.vector3d(1.0, 0.5, 1.0))
            compare(initialized.positionAbsolute, true)
            compare(initialized.rotation, Qt.quaternion(1, 0.5, 0, 0))
            compare(initialized.scaling, Qt.vector3d(0.2, 0.2, 0.2))
            compare(initialized.scalingAbsolute, false)
            compare(initialized.shadowCasting, false)
            compare(initialized.textureFile, ":\customtexture.jpg")
            compare(initialized.visible, false)
        }
    }

    TestCase {
        name: "Custom3DItem Change"

        function test_change() {
            change.meshFile = ":\customitem.mesh"
            change.position = Qt.vector3d(1.0, 0.5, 1.0)
            change.positionAbsolute = true
            change.rotation = Qt.quaternion(1, 0.5, 0, 0)
            change.scaling = Qt.vector3d(0.2, 0.2, 0.2)
            change.scalingAbsolute = false
            change.shadowCasting = false
            change.textureFile = ":\customtexture.jpg"
            change.visible = false

            compare(change.meshFile, ":\customitem.mesh")
            compare(change.position, Qt.vector3d(1.0, 0.5, 1.0))
            compare(change.positionAbsolute, true)
            compare(change.rotation, Qt.quaternion(1, 0.5, 0, 0))
            compare(change.scaling, Qt.vector3d(0.2, 0.2, 0.2))
            compare(change.scalingAbsolute, false)
            compare(change.shadowCasting, false)
            compare(change.textureFile, ":\customtexture.jpg")
            compare(change.visible, false)

            // Signals
            compare(meshFileSpy.count, 1)
            compare(positionSpy.count, 1)
            compare(absoluteSpy.count, 1)
            compare(rotationSpy.count, 1)
            compare(scalingSpy.count, 1)
            compare(absoluteSpy2.count, 1)
            compare(shadowCastingSpy.count, 1)
            compare(textureFileSpy.count, 1)
            compare(visibleSpy.count, 1)
        }
    }

    SignalSpy {
        id: meshFileSpy
        target: change
        signalName: "meshFileChanged"
    }

    SignalSpy {
        id: positionSpy
        target: change
        signalName: "positionChanged"
    }

    SignalSpy {
        id: absoluteSpy
        target: change
        signalName: "positionAbsoluteChanged"
    }

    SignalSpy {
        id: scalingSpy
        target: change
        signalName: "scalingChanged"
    }

    SignalSpy {
        id: rotationSpy
        target: change
        signalName: "rotationChanged"
    }

    SignalSpy {
        id: visibleSpy
        target: change
        signalName: "visibleChanged"
    }

    SignalSpy {
        id: absoluteSpy2
        target: change
        signalName: "scalingAbsoluteChanged"
    }

    SignalSpy {
        id: shadowCastingSpy
        target: change
        signalName: "shadowCastingChanged"
    }

    SignalSpy {
        id: textureFileSpy
        target: change
        signalName: "textureFileChanged"
    }
}
