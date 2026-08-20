// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

GridView {
    width: 600
    height: 400
    cellWidth: 200
    cellHeight: 200

    model: 6

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

    Effect {
        id: e1
        property real uRed: 0.5
        Shader {
            id: e1_vs
            stage: Shader.Vertex
            shader: "custom_effect_simple_1.vert"
        }
        Shader {
            id: e1_fs
            stage: Shader.Fragment
            shader: "custom_effect_simple_1.frag"
        }
        passes: Pass {
            shaders: [ e1_vs, e1_fs ]
        }
    }

    Effect {
        id: e2
        passes: Pass {
            shaders: Shader {
                stage: Shader.Vertex
                shader: "custom_effect_simple_2.vert"
            }
        }
    }

    Effect {
        id: e3
        Shader {
            id: e3_fs_1
            stage: Shader.Fragment
            shader: "custom_effect_simple_3_1.frag"
        }
        Shader {
            id: e3_fs_2
            stage: Shader.Fragment
            shader: "custom_effect_simple_3_2.frag"
        }
        Buffer {
            id: tempBuffer
            name: "tempBuffer"
            sizeMultiplier: 0.5
        }
        passes: [
            Pass {
                shaders: [ e3_fs_1 ]
                output: tempBuffer
            },
            Pass {
                shaders: [ e3_fs_2 ]
                commands: BufferInput {
                    buffer: tempBuffer
                    sampler: "pass1Result"
                }
            }
        ]
        property TextureInput pass1Result: TextureInput { texture: Texture {} }
    }

    Effect {
        id: e4
        Buffer {
            id: tempBuffer2
            name: "tempBuffer"
        }
        passes: [
            Pass {
                shaders: [ e3_fs_1 ]
                output: tempBuffer2
            },
            Pass {
                shaders: [ e3_fs_2 ]
                commands: BufferInput {
                    buffer: tempBuffer2
                    sampler: "pass1Result"
                }
            }
        ]
        property TextureInput pass1Result: TextureInput { texture: Texture {} }
    }

    Effect {
        id: e5
        // referencing e0's TextureInput does not work for whatever reason, so have our own
        property TextureInput tex: TextureInput {
            texture: Texture { source: "../shared/maps/rgba.png" }
        }
        property TextureInput originalInput: TextureInput { texture: Texture {} }
        property real uRed: 1.0
        Buffer {
            id: tempBuffer3
            name: "tempBuffer"
        }
        Shader {
            id: e5_fs_1
            stage: Shader.Fragment
            shader: "custom_effect_simple_5_1.frag"
        }
        Shader {
            id: e5_fs_2
            stage: Shader.Fragment
            shader: "custom_effect_simple_5_2.frag"
        }
        passes: [
            Pass {
                shaders: [ e5_fs_1 ]
                output: tempBuffer3
            },
            Pass {
                shaders: [ e5_fs_2 ]
                commands: [
                    BufferInput {
                        buffer: tempBuffer3
                    },
                    BufferInput {
                        sampler: "originalInput"
                    },
                    SetUniformValue {
                        target: "uRed"
                        // the background with the original scene behind the
                        // RGBA text will have it's red channel removed,
                        // becoming more cyan than white, where the rgba image
                        // does not add back red
                        value: 0.0
                    }
                ]
            }
        ]
    }

    delegate: PlainView {
        effect: index == 0 ? e0 : index == 1 ? e1 : index == 2 ? e2 : index == 3 ? e3 : index == 4 ? e4 : e5
    }
}
