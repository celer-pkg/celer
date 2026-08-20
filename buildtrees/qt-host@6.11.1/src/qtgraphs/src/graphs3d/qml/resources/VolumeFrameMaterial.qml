// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property color color
    property vector2d sliceFrameWidth

    shadingMode: CustomMaterial.Unshaded
    sourceBlend: CustomMaterial.SrcAlpha
    destinationBlend: CustomMaterial.OneMinusSrcAlpha
    vertexShader: "qrc:/shaders/positionvert"
    fragmentShader: "qrc:/shaders/3dsliceframesfrag"
}
