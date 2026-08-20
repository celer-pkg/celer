// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

CustomMaterial {
    id: material
    shadingMode: CustomMaterial.Shaded
    property alias baseTexture : baseTex.source
    property alias normalTexture : normalTex.source
    property color uBaseColor
    property alias baseColor : material.uBaseColor
    property real uOpacity : 1.0
    property alias opacity : material.uOpacity
    property real uMetalness : 0.0
    property alias metalness : material.uMetalness
    property real uRoughness : 0.0
    property alias roughness : material.uRoughness
    property real uIOR : 1.5
    property alias indexOfRefraction : material.uIOR
    property real uFresnelPower : 5.0
    property alias fresnelPower : material.uFresnelPower
    property real uFresnelScale : 1.0
    property alias fresnelScale : material.uFresnelScale
    property real uFresnelBias : 0.0
    property alias fresnelBias : material.uFresnelBias
    property real uSpecularAmount : 0.5
    property alias specularAmount : material.uSpecularAmount
    property bool blending : false
    property real uClearcoatAmount : 0.0
    property alias clearcoatAmount : material.uClearcoatAmount
    property real uClearcoatRoughnessAmount : 0.0
    property alias clearcoatRoughnessAmount : material.uClearcoatRoughnessAmount
    property real uClearcoatFresnelPower : 0.0
    property alias clearcoatFresnelPower : material.uClearcoatFresnelPower
    property real uClearcoatFresnelScale : 0.0
    property alias clearcoatFresnelScale : material.uClearcoatFresnelScale
    property real uClearcoatFresnelBias : 0.0
    property alias clearcoatFresnelBias : material.uClearcoatFresnelBias

    fragmentShader: "shaders/rain_material.frag"
    vertexShader: "shaders/rain_material.vert"

    property bool uBaseEnabled : uBase.enabled

    property real uTime: 0.0
    property TextureInput uBase: TextureInput {
        enabled: !baseTex.source.toString().length > 0
        texture: Texture {
            generateMipmaps: true
            mipFilter: Texture.Linear
            id: baseTex
        }
    }
    property TextureInput uNormal: TextureInput {
        enabled: !normalTex.source.toString().length > 0
        texture: Texture {
            generateMipmaps: true
            mipFilter: Texture.Linear
            id: normalTex
        }
    }
    property real rainScale
    property real dripSize
    property real normalFactor
    property real rainStrength
    property real rainSize
    property real rainFrequency
    property real rainPower
    property real dripSpeed
    property real dripLength
    property real dripSharpness
    property vector2d dripWaveSize: Qt.vector2d( 20.0 * rainSize, 4.0 * rainSize);
    property vector2d dripWaveFreq: Qt.vector2d( 47.0 * rainFrequency, 15.0 * rainFrequency);
    property vector2d dripWavePower: Qt.vector2d( 0.045 * rainPower, 1.0 * rainPower);

    NumberAnimation on uTime {
        from: 0.0
        to: 1000.0
        duration: 10000
        loops: NumberAnimation.Infinite
        running: true
    }

    destinationBlend: (uOpacity < 1.0 || blending) ? CustomMaterial.OneMinusSrcAlpha : CustomMaterial.NoBlend
    sourceBlend: (uOpacity < 1.0 || blending) ? CustomMaterial.SrcAlpha : CustomMaterial.NoBlend
    cullMode: (uOpacity < 1.0 || blending) ? CustomMaterial.NoCulling: CustomMaterial.BackFaceCulling
}
