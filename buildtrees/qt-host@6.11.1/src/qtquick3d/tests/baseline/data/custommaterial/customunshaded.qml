// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        Model {
            position: Qt.vector3d(0, 0, 0)
            eulerRotation.y: 45
            scale: Qt.vector3d(2, 2, 2)
            source: "#Sphere"
            materials: [
                CustomMaterial {
                    property real time: 0.5
                    property real amplitude: 10.0
                    property real alpha: 0.3

                    shadingMode: CustomMaterial.Unshaded
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.OneMinusSrcAlpha
                    cullMode: CustomMaterial.BackFaceCulling

                    vertexShader: "customunshaded.vert"
                    fragmentShader: "customunshaded.frag"
                }
            ]
        }
    }
}
