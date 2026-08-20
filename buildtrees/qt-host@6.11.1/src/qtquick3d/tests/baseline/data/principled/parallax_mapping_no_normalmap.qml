// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Test for QTBUG-142937: parallax mapping (heightMap) without a normalMap on
// geometry that has no pre-computed tangents (SphereGeometry). The fix must
// ensure tangents/binormals are generated analytically for all pass types,
// including shadow passes (castsShadow: true).

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    width: 400
    height: 400
    color: "lightgrey"

    View3D {
        anchors.fill: parent
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
        }

        DirectionalLight {
            brightness: 1.5
            eulerRotation.y: 45
            eulerRotation.x: -30
            castsShadow: true
        }

        PerspectiveCamera {
            z: 400
            y: 80
            eulerRotation.x: -10
        }

        Texture {
            id: heightTex
            source: "../shared/maps/heightmap.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }

        Texture {
            id: gridTex
            source: "../shared/maps/GridWithDetails.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }

        // Height map only + custom geometry without tangents: the QTBUG-142937 case.
        // SphereGeometry has no pre-computed tangent/binormal attributes, so they
        // must be derived analytically. Without the fix this renders incorrectly.
        Model {
            geometry: SphereGeometry { radius: 70 }
            position: Qt.vector3d(-100, 30, 0)
            materials: PrincipledMaterial {
                baseColorMap: gridTex
                heightMap: heightTex
                heightAmount: 0.3
                roughness: 0.8
                metalness: 0.0
            }
        }

        // Control: same geometry + height map but heightAmount = 0 (no parallax)
        Model {
            geometry: SphereGeometry { radius: 70 }
            position: Qt.vector3d(100, 30, 0)
            materials: PrincipledMaterial {
                baseColorMap: gridTex
                heightMap: heightTex
                heightAmount: 0.0
                roughness: 0.8
                metalness: 0.0
            }
        }

        // Control: built-in #Sphere (has pre-computed tangents) with height map only
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.7, 0.7, 0.7)
            position: Qt.vector3d(-100, -120, 0)
            materials: PrincipledMaterial {
                baseColorMap: gridTex
                heightMap: heightTex
                heightAmount: 0.3
                roughness: 0.8
                metalness: 0.0
            }
        }

        // Control: no height map at all
        Model {
            geometry: SphereGeometry { radius: 70 }
            position: Qt.vector3d(100, -120, 0)
            materials: PrincipledMaterial {
                baseColorMap: gridTex
                roughness: 0.8
                metalness: 0.0
            }
        }
    }
}
