// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import Example 1.0

Node {
    property alias text: textItem.text
    property alias position: geometry.position
    property alias normal: geometry.normal
    property alias texcoord0: geometry.texcoord0
    property alias texcoord1: geometry.texcoord1
    property alias tangent: geometry.tangent
    property alias binormal: geometry.binormal
    property alias color: geometry.color

    PrincipledMaterial {
        id: material
        baseColor: "white"
        emissiveFactor: Qt.vector3d(1.0, 0.0, 0.0)
        baseColorMap: Texture {
            source: "maps/basemetal_small.png"
        }
        normalMap: Texture {
            source: "maps/normalrough_small.png"
        }
        metalnessMap: Texture {
            source: "maps/basemetal_small.png"
        }
        metalnessChannel: Material.A
        roughnessMap: Texture {
            source: "maps/normalrough_small.png"
        }
        roughnessChannel: Material.A
        specularAmount: 0.5
        roughness: 0.1
        metalness: 0.1
    }
    Node {
        y: 100
        x: -100
        z: -300

        Model {
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 90
            geometry: TestGeometry {
                id: geometry
            }
            materials: [
                material
            ]
        }
        Text {
            id: textItem
            x: 50
            y: -50
            text: "Position Only"
        }
    }
}
