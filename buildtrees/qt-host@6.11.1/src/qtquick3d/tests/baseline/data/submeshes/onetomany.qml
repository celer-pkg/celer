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

        PerspectiveCamera {
            id: camera
            z: 10
        }

        // The top face of the cube gets its own material, the rest share the same one.
        Model {
            source: "../shared/models/distortedcube.mesh"
            scale: Qt.vector3d(3, 3, 3)
            materials: [ PrincipledMaterial {
                    metalness: 0
                    baseColor: "#FFFF0000"
                    cullMode: Material.NoCulling
                    lighting: PrincipledMaterial.NoLighting
                }, PrincipledMaterial {
                    metalness: 0
                    baseColor: "#FF00FF00"
                    cullMode: Material.NoCulling
                    lighting: PrincipledMaterial.NoLighting
                }  ]
       }
   }
}
