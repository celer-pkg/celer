// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

PlainView {
    width: 400
    height: 400

    Effect {
        id: e0
        passes: Pass {
            shaders: Shader {
                stage: Shader.Fragment
                shader: "normaltexture.frag"
            }
        }
    }

    effect: e0
}
