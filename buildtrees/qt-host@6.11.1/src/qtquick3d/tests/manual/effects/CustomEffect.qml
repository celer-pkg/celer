// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

Effect {
    property real amount: 0.01
    property bool flipHorizontally: false
    property bool flipVertically: true

    Shader {
        id: blur
        stage: Shader.Fragment
        shader: "qrc:/qtquick3deffects/shaders/blur.frag"
    }

    Shader {
        id: flip
        stage: Shader.Fragment
        shader: "qrc:/qtquick3deffects/shaders/flip.frag"
    }

    Buffer {
        id: tempBuffer
        name: "tempBuffer"
        format: Buffer.RGBA8
        textureFilterOperation: Buffer.Linear
        textureCoordOperation: Buffer.ClampToEdge
        bufferFlags: Buffer.None // aka frame lifetime
    }

    passes: [
        Pass {
            shaders: blur
            output: tempBuffer
        },
        Pass {
            shaders: flip
            commands: [
                BufferInput {
                    buffer: tempBuffer
                }
            ]
        }
    ]
}
