// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtQuick3D
import QtQuick3D.Helpers
import QtQuick3DTest.RhiRendering

Window {
    width: 1280
    height: 720
    visible: true
    title: qsTr("QRhi-based rendering within a View3D")

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "skyblue"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: cbMsaa.checked ? SceneEnvironment.MSAA : SceneEnvironment.NoAA
            antialiasingQuality: SceneEnvironment.High // 4x
        }

        extensions: [
            RhiRenderingExtension {
                id: renderer
                SequentialAnimation on x {
                    NumberAnimation { from: 500; to: -500; duration: 4000 }
                    NumberAnimation { from: -500; to: 500; duration: 4000 }
                    loops: -1
                }
            }
        ]

        PerspectiveCamera {
            id: camera
            z: 500
        }

        DirectionalLight {
        }

        PrincipledMaterial {
            id: material
            baseColor: "red"
            // Having < 1 opacity or setting alphaMode to Blend all make
            // the object belong to the back-to-front pass with no depth
            // write, which has then consequences for the QRhi-rendered
            // object's depth test. (and then the mode being Underlay or
            // Overlay suddenly matters!)
            opacity: cbAlpha.checked ? 0.25 : 1.0
        }

        Model {
            source: "#Sphere"
            materials: material
        }

        Model {
            source: "#Cube"
            materials: material
            x: 200
        }

        Model {
            source: "#Cube"
            materials: material
            x: -200
        }
    }

    WasdController {
        controlledObject: camera
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: view3d
                id: dbg
                visible: false
            }
        }
    }

    ColumnLayout {
        CheckBox {
            id: cbAlpha
            text: "Semi-transparent material"
            checked: false
        }
        RowLayout {
            Label {
                text: "Extension render mode"
            }
            ComboBox {
                id: modeComboBox
                textRole: "text"
                valueRole: "value"
                implicitContentWidthPolicy: ComboBox.WidestText
                onActivated: {
                    renderer.mode = currentValue;
                    view3d.rebuildExtensionList();
                }
                Component.onCompleted: modeComboBox.currentIndex = modeComboBox.indexOfValue(renderer.mode)
                model: [
                    { value: RhiRenderingExtension.Underlay, text: "Underlay"},
                    { value: RhiRenderingExtension.Overlay, text: "Overlay"}
                ]
            }
        }
        CheckBox {
            id: cbMsaa
            text: "MSAA (4x)"
            checked: false
        }
    }
}
