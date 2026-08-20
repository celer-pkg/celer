// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D

CustomMaterial {
    shadingMode: CustomMaterial.Unshaded
    vertexShader: "material.vert"
    fragmentShader: "material.frag"
    property var uVarInt: 60
    property bool uBoolFalse: false
    property bool uBoolTrue: true
    property int uInt: 33
    property real uReal: 3.3
    property point uPointS: "0.0,1.0"
    property point uPointF: Qt.point(1,0);
    property size uSizeS: "1x1"
    property size uSizeF: Qt.size(2,2)
    property rect uRectS: "0,1,100x101"
    property rect uRectF: Qt.rect(1,0,101,100)
    property vector2d uVec2: Qt.vector2d(1.0, 2.0)
    property vector3d uVec3: Qt.vector3d(1.0, 2.0, 3.0)
    property vector4d uVec4: Qt.vector4d(1.0, 2.0, 3.0, 4.0)
    property quaternion uQuat: Qt.quaternion(1, 2, 3, 4)
    property matrix4x4 uM44: Qt.matrix4x4(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)
    property color uColor1: "green"
    property color uColor2: "#ff0000"
    property Texture uTex: Texture { }
    property TextureInput uTexInput: TextureInput { texture: Texture { } }
}
