// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

GridView {
    width: 400
    height: 400
    cellWidth: 200
    cellHeight: 200

    model: 3

    Effect {
        id: e0
        property TextureInput tex: TextureInput {
            texture: Texture { source: "../shared/maps/rgba.png" }
        }
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "custom_effect_simple_0.frag"
            }
        }
    }

    // now with the TextureInput set to enabled=false
    Effect {
        id: e1
        property TextureInput tex: TextureInput {
            enabled: false
            texture: Texture { source: "../shared/maps/rgba.png" }
        }
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "custom_effect_simple_0.frag"
            }
        }
    }

    // enabled TextureInput but null texture in it
    Effect {
        id: e2
        property TextureInput tex: TextureInput { }
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "custom_effect_simple_0.frag"
            }
        }
    }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : e2
    }
}
