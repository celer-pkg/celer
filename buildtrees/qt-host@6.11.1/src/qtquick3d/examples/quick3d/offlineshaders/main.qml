// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    title: "Offline Materials Example"
    color: "#848895"

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Underlay

        //! [environment]
        environment: SceneEnvironment {
            clearColor: window.color
        }
        //! [environment]

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        Texture {
            id: baseColorMap
            source: "maps/metallic/basecolor.jpg"
        }

        //! [redMaterial]
        Model {
            position: Qt.vector3d(0, -30, 0)
            scale: Qt.vector3d(4, 4, 4)
            source: "#Sphere"
            materials: MaterialRed {
                id: redMaterial
            }
        //! [redMaterial]
            SequentialAnimation on eulerRotation {
                loops: Animation.Infinite
                PropertyAnimation {
                    duration: 5000
                    from: Qt.vector3d(0, 0, 0)
                    to: Qt.vector3d(360, 360, 360)
                }
            }
        }
        //! [setMap]
        MouseArea {
            anchors.fill: parent
            onClicked: {
            if (redMaterial.baseColorMap === null)
                redMaterial.baseColorMap = baseColorMap
            else
                redMaterial.baseColorMap = null
            }
        }
        //! [setMap]
    }
}
