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
    title: qsTr("QRhi-based rendering with combined extensions within a View3D")

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
            MyExtension {
                id: renderer
                x: 300
                SequentialAnimation on x {
                    running: cbAnim.checked
                    NumberAnimation { from: 300; to: -300; duration: 4000 }
                    NumberAnimation { from: -300; to: 300; duration: 4000 }
                    loops: -1
                }
                NumberAnimation on subSceneRotation {
                    running: cbSubAnim.checked
                    from: 0
                    to: 360
                    loops: -1
                    duration: 5000
                }
            }
        ]

        PerspectiveCamera {
            id: camera
            z: 300
        }

        DirectionalLight {
        }

        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    textureProvider: renderer
                }
            }
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
            id: cbMsaa
            text: "MSAA (4x) on View3D"
            checked: false
        }
        CheckBox {
            id: cbAnim
            text: "Animate"
            checked: true
        }
        CheckBox {
            id: cbSubAnim
            text: "Animate sub-scene"
            checked: true
        }
    }
}
