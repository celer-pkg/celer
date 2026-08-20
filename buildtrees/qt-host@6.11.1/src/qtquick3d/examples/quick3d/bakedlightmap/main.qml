// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D
import QtQuick3D.Helpers

ApplicationWindow {
    width: 1280
    height: 720
    title: "Qt Quick 3D Baked Lightmap Example"
    visible: true

    View3D {
        id: root
        anchors.fill: parent

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        PerspectiveCamera {
            id: camera
            z: 300
            y: 100
        }

        property bool lmEnabled: lmToggle.checked
        property int lightBakeMode: lmToggle.checked ? Light.BakeModeAll : Light.BakeModeDisabled

        //! [light]
        PointLight {
            bakeMode: root.lightBakeMode
            y: 190
            brightness: brightnessSlider.value
            castsShadow: true
            shadowFactor: 75
            shadowBias: 20
        }
        //! [light]

        //! [model]
        Box {
            usedInBakedLighting: true
            bakedLightmap: BakedLightmap {
                enabled: root.lmEnabled
                key: "box"
            }
            scale: Qt.vector3d(100, 100, 100)
        }
        //! [model]

        Pane {
            width: 320
            height: 160
            ColumnLayout {
                anchors.centerIn: parent
                CheckBox {
                    id: lmToggle
                    text: "Use lightmaps (fully baked direct+indirect)\nif available"
                    checked: true
                    focusPolicy: Qt.NoFocus
                }
                Label {
                    text: "How to bake lightmaps: \nOpen DebugView -> Tools -> Bake lightmap"
                }
                Label {
                    text: "Slider controls light brightness"
                }
                Slider {
                    id: brightnessSlider
                    value: 5.0
                    from: 0
                    to: 10
                }
            }
        }
    }

    Item {
        width: debugViewToggleText.implicitWidth
        height: debugViewToggleText.implicitHeight
        anchors.right: parent.right
        Label {
            id: debugViewToggleText
            text: "Click here " + (dbg.visible ? "to hide DebugView" : "for DebugView")
            color: "white"
            anchors.right: parent.right
            anchors.top: parent.top
        }
        MouseArea {
            anchors.fill: parent
            onClicked: dbg.visible = !dbg.visible
            DebugView {
                y: debugViewToggleText.height * 2
                anchors.right: parent.right
                source: root
                id: dbg
                visible: false
            }
        }
    }
}
