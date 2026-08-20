// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D
import Example

Window {
    id: window
    visible: true
    width: 1366
    height: 768
    title: qsTr("Custom Morphing Example")

    View3D {
        id: view
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        DirectionalLight {
            eulerRotation.x: -10
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 17000
                loops: Animation.Infinite
            }
            ambientColor: Qt.rgba(0.3, 0.3, 0.3, 1.0)
        }

        PointLight {
            position: Qt.vector3d(-15, 10, 15)
        }

        PerspectiveCamera {
            id: camera
            position.z: 25.0
            position.y: 10.0
            eulerRotation.x: -30
            clipNear: 1.0
            clipFar: 40.0
        }

        PrincipledMaterial {
            id: material
            baseColor: "#af4f1f"
            roughness: 0.3
            specularAmount: 0.6
        }

        //! [target]
        MorphTarget {
            id: morphtarget
            attributes: MorphTarget.Position | MorphTarget.Normal | MorphTarget.Color
            SequentialAnimation on weight {
                PauseAnimation { duration: 1000 }
                NumberAnimation { from: 0; to: 1; duration: 4000 }
                PauseAnimation { duration: 1000 }
                NumberAnimation { from: 1; to: 0; duration: 4000 }
                loops: Animation.Infinite
            }
        }
        //! [target]

        //! [model]
        Model {
            y: -1
            geometry: MorphGeometry {}
            morphTargets: [ morphtarget ]
            materials: [ material ]
        }
        //! [model]
    }
}
