// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

// Test for QTBUG-145056: Writing to NORMAL via TANGENT/BINORMAL in a CustomMaterial
// MAIN() must take effect when DebugSettings.Normals is active.  The regression
// (introduced by the DefaultMaterial shader-generator cleanup) caused qt_customMain()
// to be skipped entirely in debug passes so the debug visualization always showed
// unperturbed vertex normals regardless of what MAIN() wrote to NORMAL.

import QtQuick
import QtQuick3D

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        renderMode: View3D.Offscreen

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "black"
            debugSettings: DebugSettings {
                materialOverride: DebugSettings.Normals
            }
        }

        DirectionalLight {
            brightness: 1.5
            eulerRotation.x: -40
            eulerRotation.y: 45
        }

        PerspectiveCamera {
            z: 350
        }

        // Left sphere: CustomMaterial with TBN-space normal map applied in MAIN().
        // In debug Normals mode the output must show the perturbed normals, not
        // the smooth vertex normals — confirming MAIN() was actually called.
        Model {
            source: "#Sphere"
            position: Qt.vector3d(-100, 0, 0)
            materials: CustomMaterial {
                property TextureInput normalMap: TextureInput {
                    enabled: true
                    texture: Texture {
                        source: "../shared/maps/RibsNormal.png"
                    }
                }
                fragmentShader: "custommaterial_debug_normals.frag"
            }
        }

        // Right sphere: same CustomMaterial but normalMap texture disabled —
        // the NORMAL stays unperturbed, producing a smooth normal-sphere gradient.
        Model {
            source: "#Sphere"
            position: Qt.vector3d(100, 0, 0)
            materials: CustomMaterial {
                property TextureInput normalMap: TextureInput {
                    enabled: false
                    texture: Texture {
                        source: "../shared/maps/RibsNormal.png"
                    }
                }
                fragmentShader: "custommaterial_debug_normals.frag"
            }
        }
    }
}
