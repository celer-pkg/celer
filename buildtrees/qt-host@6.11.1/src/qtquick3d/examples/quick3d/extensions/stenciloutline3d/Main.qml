// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QtQuick3D
import QtQuick3D.Examples.OutlineRenderExtension

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Stencil outline example")

    ColumnLayout {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        width: 250
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Outline Color"
                Layout.fillWidth: true
            }
            ColorPicker {
                id: colorPicker
                color: outlineMaterial.baseColor
                onColorModified: (color) => {outlineMaterial.baseColor = color}
            }

        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Outline Width (" + outlineRenderer.outlineScale.toFixed(2) + ")"
                Layout.fillWidth: true
            }
            Slider {
                value: 1.05
                from: 1.00
                to: 3.0
                stepSize: 0.01
                onValueChanged: outlineRenderer.outlineScale = value
            }
        }
        Label {
            text: "Click on a model to select it."
        }
    }

    //! [1]
    View3D {
        id: view3d
        anchors.topMargin: 100
        anchors.fill: parent
        extensions: [ OutlineRenderExtension {
                id: outlineRenderer
                outlineMaterial: outlineMaterial
            }
        ]
    //! [1]

        PerspectiveCamera {
            id: camera
            z: 500
        }

        DirectionalLight {

        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(0, 180, 0)
            position.z: -600
        }
        PrincipledMaterial {
            id: outlineMaterial
            baseColor: "blue"
            lighting: PrincipledMaterial.NoLighting
        }

        Model {
            source: "models/suzanne.mesh"
            pickable: true
            materials: PrincipledMaterial {
                baseColor: "red"
            }
        }

        Model {
            position.x: 300
            source: "models/suzanne.mesh"
            pickable: true
            materials: PrincipledMaterial {
                baseColor: "green"
            }
        }


        Model {
            position.x: -300
            source: "models/suzanne.mesh"
            pickable: true
            materials: PrincipledMaterial {
                baseColor: "pink"
            }
        }
    }

//! [2]
    MouseArea {
        anchors.fill: view3d
        onClicked: (mouse)=> {
              let hit = view3d.pick(mouse.x, mouse.y)
              outlineRenderer.target = hit.objectHit
        }
    }
//! [2]
}
