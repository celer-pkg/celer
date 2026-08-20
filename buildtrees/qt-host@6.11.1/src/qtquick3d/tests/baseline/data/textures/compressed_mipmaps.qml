// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "lightgray"

    View3D {
        anchors.fill: parent
        camera: camera
        renderMode: View3D.Offscreen

        OrthographicCamera {
            id: camera
            z: 500
        }

        DirectionalLight {
            id: dirLight1
            eulerRotation.y: 60
            ambientColor: Qt.rgba(0.5, 0.5, 0.5, 1.0);
        }

        Texture {
            id: tex_miptester
            source: "../shared/maps/miptester_etc2.ktx"
        }

        DefaultMaterial {
            id: mat_miptester
            diffuseMap: tex_miptester
        }

        // Row 1
        Model {
            source: "#Cube"
            scale: Qt.vector3d(1.3, 1.3, 1.3)
            position: Qt.vector3d(-125, 125, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(1, 1, 1)
            position: Qt.vector3d(0, 125, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(.6, .6, .6)
            position: Qt.vector3d(125, 125, 0)
            materials: [ mat_miptester ]
        }

        // Row 2
        Model {
            source: "#Cube"
            scale: Qt.vector3d(.45, .45, .45)
            position: Qt.vector3d(-125, 0, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(.3, .3, .3)
            position: Qt.vector3d(0, 0, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(.15, .15, .15)
            position: Qt.vector3d(125, 0, 0)
            materials: [ mat_miptester ]
        }

        // Row 3
        Model {
            source: "#Cube"
            scale: Qt.vector3d(.08, .08, .08)
            position: Qt.vector3d(-125, -125, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(.04, .04, .04)
            position: Qt.vector3d(0, -125, 0)
            materials: [ mat_miptester ]
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(.02, .02, .02)
            position: Qt.vector3d(125, -125, 0)
            materials: [ mat_miptester ]
        }
    }
}
