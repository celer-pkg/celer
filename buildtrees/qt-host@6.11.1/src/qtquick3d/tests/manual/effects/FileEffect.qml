// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

Effect {
    id: root
    property url vertexShaderFile
    property url fragmentShaderFile

    passes: [
        Pass {
            shaders: [
                Shader {
                    stage: Shader.Vertex
                    shader: root.vertexShaderFile
                },
                Shader {
                    stage: Shader.Fragment
                    shader: root.fragmentShaderFile
                }
            ]
        }
    ]
}
