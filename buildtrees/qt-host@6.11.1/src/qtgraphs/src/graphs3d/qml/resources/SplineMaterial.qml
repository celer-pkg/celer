// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

CustomMaterial {
    property TextureInput controlPoints: TextureInput {}
    property int points

    property real tension
    property real knotting
    property bool loop
    property color color

    shadingMode: CustomMaterial.Shaded

    vertexShader: "qrc:/shaders/splinevert"
    fragmentShader: "qrc:/shaders/splinefrag"
}
