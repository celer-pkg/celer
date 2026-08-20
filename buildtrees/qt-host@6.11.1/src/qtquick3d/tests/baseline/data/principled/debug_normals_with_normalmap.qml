// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Test for QTBUG-145057: PrincipledMaterial with a normalMap, alphaMode=Mask,
// OpaquePrePassDepthDraw, shadows, and IBL — switching to DebugSettings.Normals
// caused a shader compile crash ('qt_tangent' undeclared identifier).
// The Normal debug mode only set needsWorldNormal, but normal map sampling also
// requires tangent/binormal for the TBN matrix.

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData {}
            }
            debugSettings: DebugSettings {
                materialOverride: DebugSettings.Normals
            }
        }

        DirectionalLight {
            brightness: 1.5
            eulerRotation.x: -40
            eulerRotation.y: 45
            castsShadow: true
        }

        PerspectiveCamera {
            id: camera
            z: 350
        }

        Texture {
            id: normalTex
            source: "../shared/maps/RibsNormal.png"
        }

        Texture {
            id: baseTex
            source: "../shared/maps/oulu_2.jpeg"
        }

        // PrincipledMaterial with normalMap + alphaMode=Mask + OpaquePrePassDepthDraw:
        // the exact combination that triggered QTBUG-145057 when debug Normals was active.
        Model {
            source: "#Sphere"
            position: Qt.vector3d(-100, 0, 0)
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            materials: PrincipledMaterial {
                baseColorMap: baseTex
                normalMap: normalTex
                roughness: 0.8
                alphaMode: PrincipledMaterial.Mask
                depthDrawMode: PrincipledMaterial.OpaquePrePassDepthDraw
            }
        }

        // Control: same setup without a normal map
        Model {
            source: "#Sphere"
            position: Qt.vector3d(100, 0, 0)
            scale: Qt.vector3d(0.8, 0.8, 0.8)
            materials: PrincipledMaterial {
                baseColorMap: baseTex
                roughness: 0.8
                alphaMode: PrincipledMaterial.Mask
                depthDrawMode: PrincipledMaterial.OpaquePrePassDepthDraw
            }
        }
    }
}
