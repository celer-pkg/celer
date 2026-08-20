// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    visible: true
    width:  800
    height: 800
    title: qsTr("Morphing Example")

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "gray"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position.z: 3.0
            position.y: 0.75
            eulerRotation.x: -12
            clipNear: 1.0
            clipFar: 60.0
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

//! [morphtargets]
        MorphTarget {
            id: morphtarget0
            weight: mouthSlider.value
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        MorphTarget {
            id: morphtarget1
            weight: earSlider.value
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
        MorphTarget {
            id: morphtarget2
            weight: cubeSlider.value
            attributes: MorphTarget.Position | MorphTarget.Normal
        }
//! [morphtargets]

//! [model]
        Model {
            source: "suzanne.mesh"
            morphTargets: [
                morphtarget0,
                morphtarget1,
                morphtarget2
            ]
            materials: PrincipledMaterial {
                baseColor: "#41cd52"
                roughness: 0.1
            }
            SequentialAnimation on eulerRotation.y {
                NumberAnimation { from: -45; to: 45; duration: 10000 }
                NumberAnimation { from: 45; to: -45; duration: 10000 }
                loops: Animation.Infinite
            }
        }
//! [model]
    }

    Pane {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        RowLayout {
            width: parent.width
            spacing: 10
//! [sliders]
            Label { text: "Mouth:"; }
            RealSlider {
                id: mouthSlider
                from: 0.0
                to: 1.0
            }
            Label { text: "Ears and eyebrows:" }
            RealSlider {
                id: earSlider
                from: 0.0
                to: 1.0
            }
            Label { text: "Cubify:" }
            RealSlider {
                id: cubeSlider
                from: 0.0
                to: 1.0
            }
//! [sliders]
        }

    }
}
