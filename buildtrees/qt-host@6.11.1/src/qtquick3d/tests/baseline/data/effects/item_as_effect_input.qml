// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

Item {
    width: 640
    height: 480

    Item {
        anchors.fill: parent

        // view1 is only here so that it can be referenced as the Texture's
        // sourceItem If the Effect is not correctly picking the texture up, the
        // sphere on the right side will not be visible.
        View3D {
            id: view1
            anchors.fill: parent
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Transparent
            }

            PerspectiveCamera {
                position.z : 300
            }

            DirectionalLight { }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial { }
                scale: Qt.vector3d(1.5, 1.5, 1.5)
                position.x: 200
            }
        }

        View3D {
            id: view2
            anchors.fill: parent
            environment: SceneEnvironment {
                backgroundMode: SceneEnvironment.Color
                clearColor: "red"
                effects: effect
            }

            PerspectiveCamera {
                id: camera2
                position.z : 300
            }

            DirectionalLight { }

            Model {
                source: "#Sphere"
                materials: PrincipledMaterial { }
                scale: Qt.vector3d(1.5,1.5,1.5)
                position.x: -200
            }
        }

        Effect {
            id: effect

            property TextureInput view1Input : TextureInput {
                texture: Texture {
                    sourceItem : view1
                }
            }

            passes: Pass {
                shaders: Shader {
                    stage: Shader.Fragment
                    shader: "item_as_effect_input.frag"
                }
            }
        }
    }
}
