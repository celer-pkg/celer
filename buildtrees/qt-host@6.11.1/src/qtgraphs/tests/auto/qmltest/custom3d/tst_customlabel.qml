// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    width: 150
    height: 150

    Custom3DLabel {
        id: initial
    }

    Custom3DLabel {
        id: initialized
        backgroundColor: "red"
        backgroundVisible: false
        borderVisible: false
        facingCamera: true
        font.family: "Times New Roman"
        text: "test label"
        textColor: "blue"

        position: Qt.vector3d(1.0, 0.5, 1.0)
        positionAbsolute: true
        rotation: Qt.quaternion(1, 0.5, 0, 0)
        scaling: Qt.vector3d(0.2, 0.2, 0.2)
        shadowCasting: true
        visible: false
    }

    Custom3DLabel {
        id: change
    }

    TestCase {
        name: "Custom3DLabel Initial"

        function test_initial() {
            compare(initial.backgroundColor, "#a0a0a4")
            compare(initial.backgroundVisible, true)
            compare(initial.borderVisible, true)
            compare(initial.facingCamera, false)
            compare(initial.font.family, "Arial")
            compare(initial.text, "")
            compare(initial.textColor, "#ffffff")

            compare(initial.meshFile, ":/defaultMeshes/plane")
            compare(initial.position, Qt.vector3d(0.0, 0.0, 0.0))
            compare(initial.positionAbsolute, false)
            compare(initial.rotation, Qt.quaternion(0, 0, 0, 0))
            compare(initial.scaling, Qt.vector3d(0.1, 0.1, 0.1))
            compare(initial.scalingAbsolute, true)
            compare(initial.shadowCasting, false)
            compare(initial.textureFile, "")
            compare(initial.visible, true)
        }
    }

    TestCase {
        name: "Custom3DLabel Initialized"

        function test_initialized() {
            compare(initialized.backgroundColor, "#ff0000")
            compare(initialized.backgroundVisible, false)
            compare(initialized.borderVisible, false)
            compare(initialized.facingCamera, true)
            compare(initialized.font.family, "Times New Roman")
            compare(initialized.text, "test label")
            compare(initialized.textColor, "#0000ff")

            compare(initialized.position, Qt.vector3d(1.0, 0.5, 1.0))
            compare(initialized.positionAbsolute, true)
            compare(initialized.rotation, Qt.quaternion(1, 0.5, 0, 0))
            compare(initialized.scaling, Qt.vector3d(0.2, 0.2, 0.2))
            compare(initialized.shadowCasting, true)
            compare(initialized.visible, false)
        }
    }

    TestCase {
        name: "Custom3DLabel Change"

        function test_change() {
            change.backgroundColor = "red"
            change.backgroundVisible = false
            change.borderVisible = false
            change.facingCamera = true
            change.font.family = "Times New Roman"
            change.text = "test label"
            change.textColor = "blue"

            change.position = Qt.vector3d(1.0, 0.5, 1.0)
            change.positionAbsolute = true
            change.rotation = Qt.quaternion(1, 0.5, 0, 0)
            change.scaling = Qt.vector3d(0.2, 0.2, 0.2)
            change.shadowCasting = true
            change.visible = false

            compare(change.backgroundColor, "#ff0000")
            compare(change.backgroundVisible, false)
            compare(change.borderVisible, false)
            compare(change.facingCamera, true)
            compare(change.font.family, "Times New Roman")
            compare(change.text, "test label")
            compare(change.textColor, "#0000ff")

            compare(change.position, Qt.vector3d(1.0, 0.5, 1.0))
            compare(change.positionAbsolute, true)
            compare(change.rotation, Qt.quaternion(1, 0.5, 0, 0))
            compare(change.scaling, Qt.vector3d(0.2, 0.2, 0.2))
            compare(change.shadowCasting, true)
            compare(change.visible, false)

            // signals
            compare(backgroundColorSpy.count, 1)
            compare(backgroundVisibleSpy.count, 1)
            compare(borderVisibleSpy.count, 1)
            compare(facingCameraSpy.count, 1)
            compare(fontSpy.count, 1)
            compare(textSpy.count, 1)
            compare(textColorSpy.count, 1)

            // common signals
            compare(positionSpy.count, 1)
            compare(absoluteSpy.count, 1)
            compare(rotationSpy.count, 1)
            compare(scalingSpy.count, 1)
            compare(shadowCastingSpy.count, 1)
            compare(visibleSpy.count, 1)
        }
    }

    SignalSpy {
        id: backgroundColorSpy
        target: change
        signalName: "backgroundColorChanged"
    }

    SignalSpy {
        id: backgroundVisibleSpy
        target: change
        signalName: "backgroundVisibleChanged"
    }

    SignalSpy {
        id: borderVisibleSpy
        target: change
        signalName: "borderVisibleChanged"
    }

    SignalSpy {
        id: facingCameraSpy
        target: change
        signalName: "facingCameraChanged"
    }

    SignalSpy {
        id: fontSpy
        target: change
        signalName: "fontChanged"
    }

    SignalSpy {
        id: textSpy
        target: change
        signalName: "textChanged"
    }

    SignalSpy {
        id: textColorSpy
        target: change
        signalName: "textColorChanged"
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
        id: rotationSpy
        target: change
        signalName: "rotationChanged"
    }

    SignalSpy {
        id: scalingSpy
        target: change
        signalName: "scalingChanged"
    }

    SignalSpy {
        id: shadowCastingSpy
        target: change
        signalName: "shadowCastingChanged"
    }

    SignalSpy {
        id: visibleSpy
        target: change
        signalName: "visibleChanged"
    }
}
