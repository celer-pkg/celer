// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root
    property real stickSize : 1
    property real stickRotationX : 0
    property real stickRotationY : 0
    property real stickRotationZ : 0
    property real glowFactor: 1.0
    Model {
        source: "meshes/stick3.mesh"

        scale: Qt.vector3d(root.stickSize * 5, root.stickSize * 5, root.stickSize * 5)
        eulerRotation: Qt.vector3d(root.stickRotationX, root.stickRotationY, root.stickRotationZ)
        materials: [
            PrincipledMaterial {
                baseColorMap: Texture {
                    source: "images/stick_charred.png"
                    scaleU: 0.2
                }
            }
        ]
    }
    Model {
        source: "meshes/stick3.mesh"

        scale: Qt.vector3d(root.stickSize * 5.1, root.stickSize * 5.1, root.stickSize * 5.1)
        eulerRotation: Qt.vector3d(root.stickRotationX, root.stickRotationY, root.stickRotationZ)
        opacity: 0.95
        materials: [
            PrincipledMaterial {
                id: material
                property real ef

                baseColor: Qt.vector3d(0.1, 0.1, 0.1)

                emissiveMap: Texture {
                    source: "images/stick_heat3.png"
                    scaleU: 0.2
                }

                function emissiveFunction(time)
                {
                    var rad = Math.PI * 2.0 * time
                    var s = Math.sin(rad)
                    var e = 0.7 + 0.3 * s
                    return Qt.vector3d(e, e, e)
                }

                emissiveFactor: material.emissiveFunction(ef)
                NumberAnimation on ef {
                    from: 0.0
                    to: 1.0
                    duration: 2000 * root.glowFactor
                    loops: NumberAnimation.Infinite
                }
            }
        ]
    }
}
