// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Node {
    property SceneEnvironment qmlxr_environment: SceneEnvironment {
        effects: [ e0 ]
    }

    DirectionalLight {
    }

    Model {
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColor: "green"
        }
        position: Qt.vector3d(25, 0, 0)
        scale: Qt.vector3d(0.5, 0.5, 0.5)
    }

    Model {
        source: "#Sphere"
        materials: PrincipledMaterial {
            baseColor: "blue"
        }
        position: Qt.vector3d(-25, 0, 0)
        scale: Qt.vector3d(0.5, 0.5, 0.5)
    }

    Effect {
        id: e0
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "passthrough.frag"
            }
        }
    }
}
