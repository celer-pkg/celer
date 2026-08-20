// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

CustomMaterial {
    id: material
    property color baseColor: "white"
    property bool baseColorSingleChannelEnabled: false
    property int baseColorChannel: PrincipledMaterial.R
    property Texture baseColorMap: null
    property var baseColorTexture: null
    onBaseColorMapChanged: {
        if (baseColorMap && !baseColorTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.baseColorTexture = component.createObject(material, { "texture": baseColorMap});
        }
    }

    readonly property real baseColorMapFlipU: baseColorMap ? (baseColorMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real baseColorMapFlipV: baseColorMap ? (baseColorMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real baseColorMapPositionU: baseColorMap ? baseColorMap.positionU : 0.0
    readonly property real baseColorMapPositionV: baseColorMap ? baseColorMap.positionV : 0.0
    readonly property real baseColorMapScaleU: baseColorMap ? baseColorMap.scaleU : 1.0
    readonly property real baseColorMapScaleV: baseColorMap ? baseColorMap.scaleV : 1.0
    readonly property real baseColorMapPivotU: baseColorMap ? baseColorMap.pivotU : 0.0
    readonly property real baseColorMapPivotV: baseColorMap ? baseColorMap.pivotV : 0.0
    readonly property real baseColorMapRotationUV: baseColorMap ? baseColorMap.rotationUV : 0.0

    property real metalness: 0
    property int metalnessChannel: PrincipledMaterial.R

    property Texture metalnessMap: null
    property var metalnessTexture: null
    onMetalnessMapChanged: {
        if (metalnessMap && !metalnessTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.metalnessTexture = component.createObject(material, { "texture": metalnessMap});
        }
    }

    readonly property real metalnessMapFlipU: metalnessMap ? (metalnessMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real metalnessMapFlipV: metalnessMap ? (metalnessMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real metalnessMapPositionU: metalnessMap ? metalnessMap.positionU : 0.0
    readonly property real metalnessMapPositionV: metalnessMap ? metalnessMap.positionV : 0.0
    readonly property real metalnessMapScaleU: metalnessMap ? metalnessMap.scaleU : 1.0
    readonly property real metalnessMapScaleV: metalnessMap ? metalnessMap.scaleV : 1.0
    readonly property real metalnessMapPivotU: metalnessMap ? metalnessMap.pivotU : 0.0
    readonly property real metalnessMapPivotV: metalnessMap ? metalnessMap.pivotV : 0.0
    readonly property real metalnessMapRotationUV: metalnessMap ? metalnessMap.rotationUV : 0.0

    property real metalnessVertexColorMaskInfluence: 0
    property int metalnessVertexColorMaskChannel: PrincipledMaterial.R
    onMetalnessVertexColorMaskInfluenceChanged: {
        influences.metalnessVertexColorMaskInfluence[metalnessVertexColorMaskChannel] =
                metalnessVertexColorMaskInfluence
    }
    onMetalnessVertexColorMaskChannelChanged: {
        metalnessVertexColorMaskInfluence =
                influences.metalnessVertexColorMaskInfluence[metalnessVertexColorMaskChannel]
    }

    property color fresnelColor: "red"
    property bool fresnelColorSingleChannelEnabled: false
    property int fresnelColorChannel: PrincipledMaterial.R

    property Texture fresnelColorMap: null
    property var fresnelColorTexture: null
    onFresnelColorMapChanged: {
        if (fresnelColorMap && !fresnelColorTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.fresnelColorTexture = component.createObject(material, { "texture": fresnelColorMap});
        }
    }

    readonly property real fresnelColorMapFlipU: fresnelColorMap ? (fresnelColorMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real fresnelColorMapFlipV: fresnelColorMap ? (fresnelColorMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real fresnelColorMapPositionU: fresnelColorMap ? fresnelColorMap.positionU : 0.0
    readonly property real fresnelColorMapPositionV: fresnelColorMap ? fresnelColorMap.positionV : 0.0
    readonly property real fresnelColorMapScaleU: fresnelColorMap ? fresnelColorMap.scaleU : 1.0
    readonly property real fresnelColorMapScaleV: fresnelColorMap ? fresnelColorMap.scaleV : 1.0
    readonly property real fresnelColorMapPivotU: fresnelColorMap ? fresnelColorMap.pivotU : 0.0
    readonly property real fresnelColorMapPivotV: fresnelColorMap ? fresnelColorMap.pivotV : 0.0
    readonly property real fresnelColorMapRotationUV: fresnelColorMap ? fresnelColorMap.rotationUV : 0.0

    property bool useFresnelAsColorValue: false
    property bool invertFresnelPower: false
    property real fresnelPower: 5.0
    property bool fresnelScaleBiasEnabled: false
    property real fresnelScale: 1.0
    property real fresnelBias: 0.0

    property real roughness: 0
    property int roughnessChannel: PrincipledMaterial.R

    property Texture roughnessMap: null
    property var roughnessTexture: null
    onRoughnessMapChanged: {
        if (roughnessMap && !roughnessTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.roughnessTexture = component.createObject(material, { "texture": roughnessMap});
        }
    }

    readonly property real roughnessMapFlipU: roughnessMap ? (roughnessMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real roughnessMapFlipV: roughnessMap ? (roughnessMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real roughnessMapPositionU: roughnessMap ? roughnessMap.positionU : 0.0
    readonly property real roughnessMapPositionV: roughnessMap ? roughnessMap.positionV : 0.0
    readonly property real roughnessMapScaleU: roughnessMap ? roughnessMap.scaleU : 1.0
    readonly property real roughnessMapScaleV: roughnessMap ? roughnessMap.scaleV : 1.0
    readonly property real roughnessMapPivotU: roughnessMap ? roughnessMap.pivotU : 0.0
    readonly property real roughnessMapPivotV: roughnessMap ? roughnessMap.pivotV : 0.0
    readonly property real roughnessMapRotationUV: roughnessMap ? roughnessMap.rotationUV : 0.0

    property real roughnessVertexColorMaskInfluence: 0.0
    property int roughnessVertexColorMaskChannel: PrincipledMaterial.B
    onRoughnessVertexColorMaskInfluenceChanged: {
        influences.roughnessVertexColorMaskInfluence[roughnessVertexColorMaskChannel] =
                roughnessVertexColorMaskInfluence
    }
    onRoughnessVertexColorMaskChannelChanged: {
        roughnessVertexColorMaskInfluence =
                influences.roughnessVertexColorMaskInfluence[roughnessVertexColorMaskChannel]
    }

    property real heightAmount: 0
    property int heightChannel: PrincipledMaterial.R
    property Texture heightMap: null
    property var heightTexture: null
    onHeightMapChanged: {
        if (heightMap && !heightTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.heightTexture = component.createObject(material, { "texture": heightMap});
        }
    }
    property real minHeightMapSamples: 8.0
    property real maxHeightMapSamples: 32.0

    readonly property real heightMapFlipU: heightMap ? (heightMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real heightMapFlipV: heightMap ? (heightMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real heightMapPositionU: heightMap ? heightMap.positionU : 0.0
    readonly property real heightMapPositionV: heightMap ? heightMap.positionV : 0.0
    readonly property real heightMapScaleU: heightMap ? heightMap.scaleU : 1.0
    readonly property real heightMapScaleV: heightMap ? heightMap.scaleV : 1.0
    readonly property real heightMapPivotU: heightMap ? heightMap.pivotU : 0.0
    readonly property real heightMapPivotV: heightMap ? heightMap.pivotV : 0.0
    readonly property real heightMapRotationUV: heightMap ? heightMap.rotationUV : 0.0

    property real heightAmountVertexColorMaskInfluence: 0.0
    property int heightAmountVertexColorMaskChannel: PrincipledMaterial.R
    onHeightAmountVertexColorMaskInfluenceChanged: {
        influences.heightAmountVertexColorMaskInfluence[heightAmountVertexColorMaskChannel] =
                heightAmountVertexColorMaskInfluence
    }
    onHeightAmountVertexColorMaskChannelChanged: {
        heightAmountVertexColorMaskInfluence =
                influences.heightAmountVertexColorMaskInfluence[heightAmountVertexColorMaskChannel]
    }

    property real occlusionAmount: 1.0
    property int occlusionChannel: PrincipledMaterial.R
    property Texture occlusionMap: null
    property var occlusionTexture: null
    onOcclusionMapChanged: {
        if (occlusionMap && !occlusionTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.occlusionTexture = component.createObject(material, { "texture": occlusionMap});
        }
    }

    readonly property real occlusionMapFlipU: occlusionMap ? (occlusionMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real occlusionMapFlipV: occlusionMap ? (occlusionMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real occlusionMapPositionU: occlusionMap ? occlusionMap.positionU : 0.0
    readonly property real occlusionMapPositionV: occlusionMap ? occlusionMap.positionV : 0.0
    readonly property real occlusionMapScaleU: occlusionMap ? occlusionMap.scaleU : 1.0
    readonly property real occlusionMapScaleV: occlusionMap ? occlusionMap.scaleV : 1.0
    readonly property real occlusionMapPivotU: occlusionMap ? occlusionMap.pivotU : 0.0
    readonly property real occlusionMapPivotV: occlusionMap ? occlusionMap.pivotV : 0.0
    readonly property real occlusionMapRotationUV: occlusionMap ? occlusionMap.rotationUV : 0.0

    property real occlusionAmountVertexColorMaskInfluence: 0.0
    property int occlusionAmountVertexColorMaskChannel: PrincipledMaterial.B
    onOcclusionAmountVertexColorMaskInfluenceChanged: {
        influences.occlusionAmountVertexColorMaskInfluence[occlusionAmountVertexColorMaskChannel] =
                occlusionAmountVertexColorMaskInfluence
    }
    onOcclusionAmountVertexColorMaskChannelChanged: {
        occlusionAmountVertexColorMaskInfluence =
                influences.occlusionAmountVertexColorMaskInfluence[occlusionAmountVertexColorMaskChannel]
    }

    property real opacity: 1.0
    property int opacityChannel: PrincipledMaterial.R
    property alias opacityMap: _opacityTexture.texture
    property TextureInput opacityTexture: TextureInput {id: _opacityTexture }

    property bool invertOpacityMapValue: false

    readonly property real opacityMapFlipU: opacityMap ? (opacityMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real opacityMapFlipV: opacityMap ? (opacityMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real opacityMapPositionU: opacityMap ? opacityMap.positionU : 0.0
    readonly property real opacityMapPositionV: opacityMap ? opacityMap.positionV : 0.0
    readonly property real opacityMapScaleU: opacityMap ? opacityMap.scaleU : 1.0
    readonly property real opacityMapScaleV: opacityMap ? opacityMap.scaleV : 1.0
    readonly property real opacityMapPivotU: opacityMap ? opacityMap.pivotU : 0.0
    readonly property real opacityMapPivotV: opacityMap ? opacityMap.pivotV : 0.0
    readonly property real opacityMapRotationUV: opacityMap ? opacityMap.rotationUV : 0.0

    property real alphaCutoff: 0.0
    property int alphaMode: PrincipledMaterial.Default
    property int lastAlphaMode: PrincipledMaterial.Default

    property real normalStrength: 1.0

    property Texture normalMap: null
    property var normalTexture: null
    onNormalMapChanged: {
        if (normalMap && !normalTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.normalTexture = component.createObject(material, { "texture": normalMap});
        }
    }

    readonly property real normalMapFlipU: normalMap ? (normalMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real normalMapFlipV: normalMap ? (normalMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real normalMapPositionU: normalMap ? normalMap.positionU : 0.0
    readonly property real normalMapPositionV: normalMap ? normalMap.positionV : 0.0
    readonly property real normalMapScaleU: normalMap ? normalMap.scaleU : 1.0
    readonly property real normalMapScaleV: normalMap ? normalMap.scaleV : 1.0
    readonly property real normalMapPivotU: normalMap ? normalMap.pivotU : 0.0
    readonly property real normalMapPivotV: normalMap ? normalMap.pivotV : 0.0
    readonly property real normalMapRotationUV: normalMap ? normalMap.rotationUV : 0.0

    property real normalStrengthVertexColorMaskInfluence: 0.0
    property int normalStrengthVertexColorMaskChannel: PrincipledMaterial.R
    onNormalStrengthVertexColorMaskInfluenceChanged: {
        influences.normalStrengthVertexColorMaskInfluence[normalStrengthVertexColorMaskChannel] =
                normalStrengthVertexColorMaskInfluence
    }
    onNormalStrengthVertexColorMaskChannelChanged: {
        normalStrengthVertexColorMaskInfluence =
                influences.normalStrengthVertexColorMaskInfluence[normalStrengthVertexColorMaskChannel]
    }

    property vector3d emissiveFactor: Qt.vector3d(0, 0, 0)
    property bool emissiveSingleChannelEnabled: false
    property int emissiveChannel: PrincipledMaterial.R

    property Texture emissiveMap: null
    property var emissiveTexture: null
    onEmissiveMapChanged: {
        if (emissiveMap && !emissiveTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.emissiveTexture = component.createObject(material, { "texture": emissiveMap});
        }
    }

    readonly property real emissiveMapFlipU: emissiveMap ? (emissiveMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real emissiveMapFlipV: emissiveMap ? (emissiveMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real emissiveMapPositionU: emissiveMap ? emissiveMap.positionU : 0.0
    readonly property real emissiveMapPositionV: emissiveMap ? emissiveMap.positionV : 0.0
    readonly property real emissiveMapScaleU: emissiveMap ? emissiveMap.scaleU : 1.0
    readonly property real emissiveMapScaleV: emissiveMap ? emissiveMap.scaleV : 1.0
    readonly property real emissiveMapPivotU: emissiveMap ? emissiveMap.pivotU : 0.0
    readonly property real emissiveMapPivotV: emissiveMap ? emissiveMap.pivotV : 0.0
    readonly property real emissiveMapRotationUV: emissiveMap ? emissiveMap.rotationUV : 0.0

    property real clearcoatAmount: 0
    property int clearcoatChannel: PrincipledMaterial.R
    property alias clearcoatMap: _clearcoatTexture.texture
    property TextureInput clearcoatTexture: TextureInput {id: _clearcoatTexture }
    property bool invertClearcoatFresnelPower: true
    property real clearcoatFresnelPower: 5.0
    property bool clearcoatFresnelScaleBiasEnabled: false
    property real clearcoatFresnelScale: 1.0
    property real clearcoatFresnelBias: 0.0

    readonly property real clearcoatMapFlipU: clearcoatMap ? (clearcoatMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatMapFlipV: clearcoatMap ? (clearcoatMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatMapPositionU: clearcoatMap ? clearcoatMap.positionU : 0.0
    readonly property real clearcoatMapPositionV: clearcoatMap ? clearcoatMap.positionV : 0.0
    readonly property real clearcoatMapScaleU: clearcoatMap ? clearcoatMap.scaleU : 1.0
    readonly property real clearcoatMapScaleV: clearcoatMap ? clearcoatMap.scaleV : 1.0
    readonly property real clearcoatMapPivotU: clearcoatMap ? clearcoatMap.pivotU : 0.0
    readonly property real clearcoatMapPivotV: clearcoatMap ? clearcoatMap.pivotV : 0.0
    readonly property real clearcoatMapRotationUV: clearcoatMap ? clearcoatMap.rotationUV : 0.0

    property real clearcoatAmountVertexColorMaskInfluence: 0.0
    property int clearcoatAmountVertexColorMaskChannel: PrincipledMaterial.R
    onClearcoatAmountVertexColorMaskInfluenceChanged: {
        influences.clearcoatAmountVertexColorMaskInfluence[clearcoatAmountVertexColorMaskChannel] =
                clearcoatAmountVertexColorMaskInfluence
    }
    onClearcoatAmountVertexColorMaskChannelChanged: {
        clearcoatAmountVertexColorMaskInfluence =
                influences.clearcoatAmountVertexColorMaskInfluence[clearcoatAmountVertexColorMaskChannel]
    }

    property real clearcoatRoughnessAmount: 0
    property int clearcoatRoughnessChannel: PrincipledMaterial.R

    property Texture clearcoatRoughnessMap: null
    property var clearcoatRoughnessTexture: null
    onClearcoatRoughnessMapChanged: {
        if (clearcoatRoughnessMap && !clearcoatRoughnessTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.clearcoatRoughnessTexture = component.createObject(material, { "texture": clearcoatRoughnessMap});
        }
    }

    readonly property real clearcoatRoughnessMapFlipU: clearcoatRoughnessMap ? (clearcoatRoughnessMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatRoughnessMapFlipV: clearcoatRoughnessMap ? (clearcoatRoughnessMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatRoughnessMapPositionU: clearcoatRoughnessMap ? clearcoatRoughnessMap.positionU : 0.0
    readonly property real clearcoatRoughnessMapPositionV: clearcoatRoughnessMap ? clearcoatRoughnessMap.positionV : 0.0
    readonly property real clearcoatRoughnessMapScaleU: clearcoatRoughnessMap ? clearcoatRoughnessMap.scaleU : 1.0
    readonly property real clearcoatRoughnessMapScaleV: clearcoatRoughnessMap ? clearcoatRoughnessMap.scaleV : 1.0
    readonly property real clearcoatRoughnessMapPivotU: clearcoatRoughnessMap ? clearcoatRoughnessMap.pivotU : 0.0
    readonly property real clearcoatRoughnessMapPivotV: clearcoatRoughnessMap ? clearcoatRoughnessMap.pivotV : 0.0
    readonly property real clearcoatRoughnessMapRotationUV: clearcoatRoughnessMap ? clearcoatRoughnessMap.rotationUV : 0.0

    property real clearcoatRoughnessAmountVertexColorMaskInfluence: 0.0
    property int clearcoatRoughnessAmountVertexColorMaskChannel: PrincipledMaterial.R
    onClearcoatRoughnessAmountVertexColorMaskInfluenceChanged: {
        influences.clearcoatRoughnessAmountVertexColorMaskInfluence[clearcoatRoughnessAmountVertexColorMaskChannel] =
                clearcoatRoughnessAmountVertexColorMaskInfluence
    }
    onClearcoatRoughnessAmountVertexColorMaskChannelChanged: {
        clearcoatRoughnessAmountVertexColorMaskInfluence =
                influences.clearcoatRoughnessAmountVertexColorMaskInfluence[clearcoatRoughnessAmountVertexColorMaskChannel]
    }

    property real clearcoatNormalStrength: 1.0

    property Texture clearcoatNormalMap: null
    property var clearcoatNormalTexture: null
    onClearcoatNormalMapChanged: {
        if (clearcoatNormalMap && !clearcoatNormalTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.clearcoatNormalTexture = component.createObject(material, { "texture": clearcoatNormalMap});
        }
    }

    readonly property real clearcoatNormalMapFlipU: clearcoatNormalMap ? (clearcoatNormalMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatNormalMapFlipV: clearcoatNormalMap ? (clearcoatNormalMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real clearcoatNormalMapPositionU: clearcoatNormalMap ? clearcoatNormalMap.positionU : 0.0
    readonly property real clearcoatNormalMapPositionV: clearcoatNormalMap ? clearcoatNormalMap.positionV : 0.0
    readonly property real clearcoatNormalMapScaleU: clearcoatNormalMap ? clearcoatNormalMap.scaleU : 1.0
    readonly property real clearcoatNormalMapScaleV: clearcoatNormalMap ? clearcoatNormalMap.scaleV : 1.0
    readonly property real clearcoatNormalMapPivotU: clearcoatNormalMap ? clearcoatNormalMap.pivotU : 0.0
    readonly property real clearcoatNormalMapPivotV: clearcoatNormalMap ? clearcoatNormalMap.pivotV : 0.0
    readonly property real clearcoatNormalMapRotationUV: clearcoatNormalMap ? clearcoatNormalMap.rotationUV : 0.0

    property real clearcoatNormalStrengthVertexColorMaskInfluence: 0.0
    property int clearcoatNormalStrengthVertexColorMaskChannel: PrincipledMaterial.R
    onClearcoatNormalStrengthVertexColorMaskInfluenceChanged: {
        influences.clearcoatNormalStrengthVertexColorMaskInfluence[clearcoatNormalStrengthVertexColorMaskChannel] =
                clearcoatNormalStrengthVertexColorMaskInfluence
    }
    onClearcoatNormalStrengthVertexColorMaskChannelChanged: {
        clearcoatNormalStrengthVertexColorMaskInfluence =
                influences.clearcoatNormalStrengthVertexColorMaskInfluence[clearcoatNormalStrengthVertexColorMaskChannel]
    }

    property real transmissionFactor: 0
    property int transmissionChannel: PrincipledMaterial.R

    property Texture transmissionMap: null
    property var transmissionTexture: null
    onTransmissionMapChanged: {
        if (transmissionMap && !transmissionTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.transmissionTexture = component.createObject(material, { "texture": transmissionMap});
        }
    }

    readonly property real transmissionMapFlipU: transmissionMap ? (transmissionMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real transmissionMapFlipV: transmissionMap ? (transmissionMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real transmissionMapPositionU: transmissionMap ? transmissionMap.positionU : 0.0
    readonly property real transmissionMapPositionV: transmissionMap ? transmissionMap.positionV : 0.0
    readonly property real transmissionMapScaleU: transmissionMap ? transmissionMap.scaleU : 1.0
    readonly property real transmissionMapScaleV: transmissionMap ? transmissionMap.scaleV : 1.0
    readonly property real transmissionMapPivotU: transmissionMap ? transmissionMap.pivotU : 0.0
    readonly property real transmissionMapPivotV: transmissionMap ? transmissionMap.pivotV : 0.0
    readonly property real transmissionMapRotationUV: transmissionMap ? transmissionMap.rotationUV : 0.0

    property real transmissionFactorVertexColorMaskInfluence: 0.0
    property int transmissionFactorVertexColorMaskChannel: PrincipledMaterial.R
    onTransmissionFactorVertexColorMaskInfluenceChanged: {
        influences.transmissionFactorVertexColorMaskInfluence[transmissionFactorVertexColorMaskChannel] =
                transmissionFactorVertexColorMaskInfluence
    }
    onTransmissionFactorVertexColorMaskChannelChanged: {
        transmissionFactorVertexColorMaskInfluence =
                influences.transmissionFactorVertexColorMaskInfluence[transmissionFactorVertexColorMaskChannel]
    }

    property real indexOfRefraction: 1.5
    property real thicknessFactor: 0
    property int thicknessChannel: PrincipledMaterial.R

    property Texture thicknessMap: null
    property var thicknessTexture: null
    onThicknessMapChanged: {
        if (thicknessMap && !thicknessTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.thicknessTexture = component.createObject(material, { "texture": thicknessMap});
        }
    }

    readonly property real thicknessMapFlipU: thicknessMap ? (thicknessMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real thicknessMapFlipV: thicknessMap ? (thicknessMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real thicknessMapPositionU: thicknessMap ? thicknessMap.positionU : 0.0
    readonly property real thicknessMapPositionV: thicknessMap ? thicknessMap.positionV : 0.0
    readonly property real thicknessMapScaleU: thicknessMap ? thicknessMap.scaleU : 1.0
    readonly property real thicknessMapScaleV: thicknessMap ? thicknessMap.scaleV : 1.0
    readonly property real thicknessMapPivotU: thicknessMap ? thicknessMap.pivotU : 0.0
    readonly property real thicknessMapPivotV: thicknessMap ? thicknessMap.pivotV : 0.0
    readonly property real thicknessMapRotationUV: thicknessMap ? thicknessMap.rotationUV : 0.0

    property real attenuationDistance: 100.0
    property color attenuationColor: "white"

    property real thicknessFactorVertexColorMaskInfluence: 0.0
    property int thicknessFactorVertexColorMaskChannel: PrincipledMaterial.R
    onThicknessFactorVertexColorMaskInfluenceChanged: {
        influences.thicknessFactorVertexColorMaskInfluence[thicknessFactorVertexColorMaskChannel] =
                thicknessFactorVertexColorMaskInfluence
    }
    onThicknessFactorVertexColorMaskChannelChanged: {
        thicknessFactorVertexColorMaskInfluence =
                influences.thicknessFactorVertexColorMaskInfluence[thicknessFactorVertexColorMaskChannel]
    }

    property bool vertexColorsEnabled: true

    property real pointSize: 1.0

    property real specularAmount: 0.5
    property int specularAmountChannel: PrincipledMaterial.R

    property Texture specularAmountMap: null
    property var specularAmountTexture: null
    onSpecularAmountMapChanged: {
        if (specularAmountMap && !specularAmountTexture) {
            var component = Qt.createComponent("QtQuick3D", "TextureInput");
            material.specularAmountTexture = component.createObject(material, { "texture": specularAmountMap});
        }
    }

    readonly property real specularAmountMapFlipU: specularAmountMap ? (specularAmountMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real specularAmountMapFlipV: specularAmountMap ? (specularAmountMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real specularAmountMapPositionU: specularAmountMap ? specularAmountMap.positionU : 0.0
    readonly property real specularAmountMapPositionV: specularAmountMap ? specularAmountMap.positionV : 0.0
    readonly property real specularAmountMapScaleU: specularAmountMap ? specularAmountMap.scaleU : 1.0
    readonly property real specularAmountMapScaleV: specularAmountMap ? specularAmountMap.scaleV : 1.0
    readonly property real specularAmountMapPivotU: specularAmountMap ? specularAmountMap.pivotU : 0.0
    readonly property real specularAmountMapPivotV: specularAmountMap ? specularAmountMap.pivotV : 0.0
    readonly property real specularAmountMapRotationUV: specularAmountMap ? specularAmountMap.rotationUV : 0.0

    property real specularAmountVertexColorMaskInfluence: 0.0
    property int specularAmountVertexColorMaskChannel: PrincipledMaterial.R
    onSpecularAmountVertexColorMaskInfluenceChanged: {
        influences.specularAmountVertexColorMaskInfluence[specularAmountVertexColorMaskChannel] =
                specularAmountVertexColorMaskInfluence
    }
    onSpecularAmountVertexColorMaskChannelChanged: {
        specularAmountVertexColorMaskInfluence =
                influences.specularAmountVertexColorMaskInfluence[specularAmountVertexColorMaskChannel]
    }

    property vector4d snowColor: Qt.vector4d(0.4, 0.4, 0.4, 1.)
    property vector4d snowGlintColor: Qt.vector4d(3.3, 3.3, 3.3, 1.)
    property real snowGlintPower: 3.3
    property real snowGlintOffsetHeight: 1.03
    property real snowGlintOffsetRatio: 0.18
    property real snowNormalStrength: 0.5
    property real snowStrength: 0.9
    property real snowMaxDisplacement: 4.0
    property real snowMeltFadeStart: 0.0
    property real snowMeltFadeEnd: 1.0
    property real snowDirectionFadeStart: 0.4
    property real snowDirectionFadeEnd: 0.8
    property vector3d snowDirection: Qt.vector3d(0, -1., 0.)
    property matrix4x4 snowViewProjection: Qt.matrix4x4()

    property alias snowDisplacementMap: _snowDisplacementTexture.texture
    property TextureInput snowDisplacementTexture: TextureInput {id: _snowDisplacementTexture }

    readonly property real snowDisplacementMapFlipU: snowDisplacementMap ? (snowDisplacementMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real snowDisplacementMapFlipV: snowDisplacementMap ? (snowDisplacementMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real snowDisplacementMapPositionU: snowDisplacementMap ? snowDisplacementMap.positionU : 0.0
    readonly property real snowDisplacementMapPositionV: snowDisplacementMap ? snowDisplacementMap.positionV : 0.0
    readonly property real snowDisplacementMapScaleU: snowDisplacementMap ? snowDisplacementMap.scaleU : 1.0
    readonly property real snowDisplacementMapScaleV: snowDisplacementMap ? snowDisplacementMap.scaleV : 1.0
    readonly property real snowDisplacementMapPivotU: snowDisplacementMap ? snowDisplacementMap.pivotU : 0.0
    readonly property real snowDisplacementMapPivotV: snowDisplacementMap ? snowDisplacementMap.pivotV : 0.0
    readonly property real snowDisplacementMapRotationUV: snowDisplacementMap ? snowDisplacementMap.rotationUV : 0.0


    property alias snowNormalMap: _snowNormalTexture.texture
    property TextureInput snowNormalTexture: TextureInput {id: _snowNormalTexture }

    snowNormalMap: Texture {
        source: "snow_normal_displacement.png"
    }

    readonly property real snowNormalMapFlipU: snowNormalMap ? (snowNormalMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real snowNormalMapFlipV: snowNormalMap ? (snowNormalMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real snowNormalMapPositionU: snowNormalMap ? snowNormalMap.positionU : 0.0
    readonly property real snowNormalMapPositionV: snowNormalMap ? snowNormalMap.positionV : 0.0
    readonly property real snowNormalMapScaleU: snowNormalMap ? snowNormalMap.scaleU : 1.0
    readonly property real snowNormalMapScaleV: snowNormalMap ? snowNormalMap.scaleV : 1.0
    readonly property real snowNormalMapPivotU: snowNormalMap ? snowNormalMap.pivotU : 0.0
    readonly property real snowNormalMapPivotV: snowNormalMap ? snowNormalMap.pivotV : 0.0
    readonly property real snowNormalMapRotationUV: snowNormalMap ? snowNormalMap.rotationUV : 0.0

    property real snowNormalStrengthVertexColorMaskInfluence: 0.0
    property int snowNormalStrengthVertexColorMaskChannel: PrincipledMaterial.R
    onSnowNormalStrengthVertexColorMaskInfluenceChanged: {
        influences.snowNormalStrengthVertexColorMaskInfluence[snowNormalStrengthVertexColorMaskChannel] =
                snowNormalStrengthVertexColorMaskInfluence
    }
    onSnowNormalStrengthVertexColorMaskChannelChanged: {
        snowNormalStrengthVertexColorMaskInfluence =
                influences.snowNormalStrengthVertexColorMaskInfluence[snowNormalStrengthVertexColorMaskChannel]
    }

    property alias snowSparkleMap: _snowSparkleTexture.texture
    property TextureInput snowSparkleTexture: TextureInput {id: _snowSparkleTexture }

    snowSparkleMap: Texture {
        source: "sparkles.png"
        scaleU: 12
        scaleV: 12
    }

    readonly property real snowSparkleMapFlipU: snowSparkleMap ? (snowSparkleMap.flipU ? 1.0 : 0.0) : 0.0
    readonly property real snowSparkleMapFlipV: snowSparkleMap ? (snowSparkleMap.flipV ? 1.0 : 0.0) : 0.0
    readonly property real snowSparkleMapPositionU: snowSparkleMap ? snowSparkleMap.positionU : 0.0
    readonly property real snowSparkleMapPositionV: snowSparkleMap ? snowSparkleMap.positionV : 0.0
    readonly property real snowSparkleMapScaleU: snowSparkleMap ? snowSparkleMap.scaleU : 1.0
    readonly property real snowSparkleMapScaleV: snowSparkleMap ? snowSparkleMap.scaleV : 1.0
    readonly property real snowSparkleMapPivotU: snowSparkleMap ? snowSparkleMap.pivotU : 0.0
    readonly property real snowSparkleMapPivotV: snowSparkleMap ? snowSparkleMap.pivotV : 0.0
    readonly property real snowSparkleMapRotationUV: snowSparkleMap ? snowSparkleMap.rotationUV : 0.0


    QtObject {
        id: influences
        property var specularAmountVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var thicknessFactorVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var transmissionFactorVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var clearcoatNormalStrengthVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var clearcoatRoughnessAmountVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var clearcoatAmountVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var normalStrengthVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var occlusionAmountVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var heightAmountVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var roughnessVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var metalnessVertexColorMaskInfluence: [0., 0., 0., 0.]
        property var snowNormalStrengthVertexColorMaskInfluence: [0., 0., 0., 0.]
    }

    onAlphaModeChanged: {

        switch (alphaMode)
        {
        case PrincipledMaterial.Default: {
            sourceAlphaBlend = CustomMaterial.NoBlend
            sourceBlend = CustomMaterial.NoBlend
            destinationBlend = CustomMaterial.NoBlend
            break;
        }
        case PrincipledMaterial.Blend: {
            sourceAlphaBlend = CustomMaterial.OneMinusSrcAlpha
            sourceBlend = CustomMaterial.SrcAlpha
            destinationBlend = CustomMaterial.OneMinusSrcAlpha
            break;
        }
        case PrincipledMaterial.Opaque: {
            sourceAlphaBlend = CustomMaterial.NoBlend
            sourceBlend = CustomMaterial.NoBlend
            destinationBlend = CustomMaterial.NoBlend
            break;
        }
        case PrincipledMaterial.Mask: {
            sourceAlphaBlend = CustomMaterial.NoBlend
            sourceBlend = CustomMaterial.NoBlend
            destinationBlend = CustomMaterial.NoBlend
            break;
        }
        }
    }

    onOpacityChanged: {
        updateBlendState()
    }
    onOpacityMapChanged: {
        updateBlendState()
    }

    function updateBlendState() {
        if (opacity < 1.0 || opacityMap) {
            if (alphaMode != PrincipledMaterial.Blend)
                lastAlphaMode = alphaMode
            alphaMode = PrincipledMaterial.Blend
        }else {
            alphaMode = lastAlphaMode
        }
    }

    property var channelString: (new Map([
                                             [PrincipledMaterial.R, ".r"],
                                             [PrincipledMaterial.G, ".g"],
                                             [PrincipledMaterial.B, ".b"],
                                             [PrincipledMaterial.A, ".a"]
                                         ]))

    function maskByVertexColor(maskedVariable, channel, influenceString, influenceValue) {
        if (influenceValue > 0) {
            return maskedVariable + " *= mix(1.0, vertexColor" +
                    channelString.get(channel) + ", " + influenceString  + ");\n";
        }
        return "";
    }

    function getColorValueByChannel(initialString, colorString, textureString, singleChannel, channel, threeChannel, textureMapString) {
        let outString = colorString;
        if (outString !== "" && threeChannel)
            outString += ".xyz";

        if (textureString !== "") {
            let sampleString
            initialString = "    tempUV = transformUV(texCoord,
        " +textureMapString+ "FlipU,
        " +textureMapString+ "FlipV,
        " +textureMapString+ "PivotU,
        " +textureMapString+ "PivotV,
        " +textureMapString+ "PositionU,
        " +textureMapString+ "PositionV,
        " +textureMapString+ "RotationUV,
        " +textureMapString+ "ScaleU,
        " +textureMapString+ "ScaleV);\n" + initialString;
            if (singleChannel)
                sampleString = " vec4(vec3(texture2D(" + textureString + ", tempUV)" +
                        channelString.get(channel) + "), 1.0)";
            else
                sampleString = " texture2D(" + textureString + ", tempUV)";

            if (threeChannel)
                sampleString += ".xyz";

            if (outString !== "")
                outString += " * ";
            outString += "qt_sRGBToLinear(" + sampleString + ")";
        }

        if (outString !== "") {
            outString = initialString + outString;
            outString += ";\n";
        }
        return outString
    }

    function getScalerValueByChannel(initialString, scalerString, textureString, channel, invertMap, textureMapString) {
        let outString = scalerString;

        if (textureString !== "") {
            let sampleString
            initialString = "    tempUV = transformUV(texCoord,
        " +textureMapString+ "FlipU,
        " +textureMapString+ "FlipV,
        " +textureMapString+ "PivotU,
        " +textureMapString+ "PivotV,
        " +textureMapString+ "PositionU,
        " +textureMapString+ "PositionV,
        " +textureMapString+ "RotationUV,
        " +textureMapString+ "ScaleU,
        " +textureMapString+ "ScaleV);\n" + initialString;

            sampleString = " texture2D(" + textureString + ", tempUV)" +
                    channelString.get(channel) + "";
            if (invertMap)
                sampleString = "1.0 -" + sampleString;
            if (outString !== "")
                outString += " * "
            outString += sampleString;
        }

        if (outString !== "") {
            outString = initialString + outString;
            outString += ";\n";
        }
        return outString
    }

    function baseColorFragment() {
        let outString = getColorValueByChannel("    BASE_COLOR =" + (vertexColorsEnabled ? " vertexColor * " : ""), "baseColor",
                                               (baseColorMap ? "baseColorTexture" : ""),
                                               baseColorSingleChannelEnabled,
                                               baseColorChannel, false, "baseColorMap")

        if (alphaMode == PrincipledMaterial.Mask) {
            outString += "    if (BASE_COLOR.w < alphaCutoff)\n" +
                    "        discard;\n";
        }
        if (alphaMode == PrincipledMaterial.Opaque)
            outString += "    BASE_COLOR.w = 1.0;\n";
        else
            outString += getScalerValueByChannel("    BASE_COLOR.w *= ", "opacity",
                                                 (opacityMap ? "opacityTexture" : ""),
                                                 opacityChannel,
                                                 invertOpacityMapValue, "opacityMap");

        outString += (invertFresnelPower ? "    FRESNEL_POWER = 10 - fresnelPower;\n" :
                                           "    FRESNEL_POWER = fresnelPower;\n");
        if (fresnelScaleBiasEnabled) {
            outString += "    FRESNEL_SCALE = fresnelScale;\n" +
                    "    FRESNEL_BIAS = fresnelBias;\n";
        }
        return outString
    }

    function normalMapFragment() {
        let outString = ""
        if (normalMap) {
            outString += "    float normalStrengthValue = normalStrength;\n" +

                    maskByVertexColor( "    normalStrengthValue",
                                      normalStrengthVertexColorMaskChannel,
                                      "normalStrengthVertexColorMaskInfluence",
                                      normalStrengthVertexColorMaskInfluence ) +

                    "    tempUV = transformUV(texCoord,
                            normalMapFlipU,
                            normalMapFlipV,
                            normalMapPivotU,
                            normalMapPivotV,
                            normalMapPositionU,
                            normalMapPositionV,
                            normalMapRotationUV,
                            normalMapScaleU,
                            normalMapScaleV);\n" +

"    vec3 tsNormal = texture(normalTexture, tempUV).xyz * 2.0 - vec3(1.0);\n" +
"    tsNormal *= vec3(normalStrengthValue, normalStrengthValue, 1.0);\n" +
"    NORMAL = tanFrame * normalize(tsNormal);\n";
        }
        return outString
    }

    function clearcoatFragment() {
        let outString = ""
        if (clearcoatAmount > 0) {
            outString += getScalerValueByChannel("    CLEARCOAT_AMOUNT = ",
                                                 "clearcoatAmount",
                                                 (clearcoatMap ? "clearcoatTexture" : ""),
                                                 clearcoatChannel, false, "clearcoatMap") +

                    maskByVertexColor("    CLEARCOAT_AMOUNT",
                                      clearcoatAmountVertexColorMaskChannel,
                                      "clearcoatAmountVertexColorMaskInfluence",
                                      clearcoatAmountVertexColorMaskInfluence) +

                    "    CLEARCOAT_FRESNEL_POWER = 10 - clearcoatFresnelPower;\n";
            if (clearcoatFresnelScaleBiasEnabled) {
                outString += "    CLEARCOAT_FRESNEL_SCALE = clearcoatFresnelScale;\n" +
                        "    CLEARCOAT_FRESNEL_BIAS = clearcoatFresnelBias;\n";
            }

            outString += getScalerValueByChannel("    CLEARCOAT_ROUGHNESS = ",
                                                 "clearcoatRoughnessAmount",
                                                 (clearcoatRoughnessMap ? "clearcoatRoughnessTexture" : ""),
                                                 clearcoatRoughnessChannel, false, "clearcoatRoughnessMap") +

                    maskByVertexColor("    CLEARCOAT_ROUGHNESS",
                                      clearcoatRoughnessAmountVertexColorMaskChannel,
                                      "clearcoatRoughnessAmountVertexColorMaskInfluence",
                                      clearcoatRoughnessAmountVertexColorMaskInfluence)

            if (clearcoatNormalMap) {
                outString += "    float clearcoatNormalStrengthValue = clearcoatNormalStrength;\n" +

                        maskByVertexColor("    clearcoatNormalStrengthValue",
                                          clearcoatNormalStrengthVertexColorMaskChannel,
                                          "clearcoatNormalStrengthVertexColorMaskInfluence",
                                          clearcoatNormalStrengthVertexColorMaskInfluence) +

                        "    tempUV = transformUV(texCoord,
                                clearcoatNormalMapFlipU,
                                clearcoatNormalMapFlipV,
                                clearcoatNormalMapPivotU,
                                clearcoatNormalMapPivotV,
                                clearcoatNormalMapPositionU,
                                clearcoatNormalMapPositionV,
                                clearcoatNormalMapRotationUV,
                                clearcoatNormalMapScaleU,
                                clearcoatNormalMapScaleV);\n" +

"    vec3 tsNormalCoat = texture(clearcoatNormalTexture, tempUV).xyz * 2.0 - vec3(1.0);\n" +
"    tsNormalCoat *= vec3(clearcoatNormalStrengthValue, clearcoatNormalStrengthValue, 1.0);\n" +
"    CLEARCOAT_NORMAL = tanFrame * normalize(tsNormalCoat);\n";
            }
        }
        return outString
    }

    function transmissionFragment() {
        let outString = ""
        if (transmissionFactor > 0) {
            outString += getScalerValueByChannel("    TRANSMISSION_FACTOR = ",
                                                 "transmissionFactor",
                                                 (transmissionMap ? "transmissionTexture" : ""),
                                                 transmissionChannel, false, "transmissionMap") +

                    maskByVertexColor("    TRANSMISSION_FACTOR",
                                      transmissionFactorVertexColorMaskChannel,
                                      "transmissionFactorVertexColorMaskInfluence",
                                      transmissionFactorVertexColorMaskInfluence)

            outString += "    ATTENUATION_COLOR = attenuationColor.xyz;\n" +
                    "    ATTENUATION_DISTANCE = attenuationDistance;\n";

            outString += getScalerValueByChannel("    THICKNESS_FACTOR = ", "thicknessFactor",
                                                 (thicknessMap ? "thicknessTexture" : ""),
                                                 thicknessChannel, false, "thicknessMap") +
                    maskByVertexColor("    THICKNESS_FACTOR",
                                      thicknessFactorVertexColorMaskChannel,
                                      "thicknessFactorVertexColorMaskInfluence",
                                      thicknessFactorVertexColorMaskInfluence)

        }
        return outString
    }

    function parallaxFragment() {
        let outString = ""
        if (heightAmount > 0 && heightMap) {
            outString += "    float heightAmountValue = heightAmount;\n"
            outString += maskByVertexColor("    heightAmountValue",
                                           heightAmountVertexColorMaskChannel,
                                           "heightAmountVertexColorMaskInfluence",
                                           heightAmountVertexColorMaskInfluence );

            outString += "    tempUV = transformUV(texCoord,
                                heightMapFlipU,
                                heightMapFlipV,
                                heightMapPivotU,
                                heightMapPivotV,
                                heightMapPositionU,
                                heightMapPositionV,
                                heightMapRotationUV,
                                heightMapScaleU,
                                heightMapScaleV);\n";
            outString += "    texCoord = qt_parallaxMapping(tempUV, heightTexture, TANGENT, BINORMAL, worldNormal, VAR_WORLD_POSITION, CAMERA_POSITION, heightAmountValue, minHeightMapSamples, maxHeightMapSamples);\n";
        }
        return outString;
    }

    function fresnelMaskFragment() {
        let outString = "";

        if (useFresnelAsColorValue) {
            outString += "    FRESNEL_BIAS = FRESNEL_SCALE = 0.0;\n" +
                    "    vec3 fresnelColorOverlay = vec3(0);\n";


            getColorValueByChannel("    fresnelColorOverlay = ", "fresnelColor",
                                   (fresnelColorMap ? "fresnelColorTexture" : ""),
                                   fresnelColorSingleChannelEnabled,
                                   fresnelColorChannel, true, "fresnelColorMap")

            outString += (invertFresnelPower ? "    float maskedFresnelPower = 10 - fresnelPower;\n" :
                                               "    float maskedFresnelPower = fresnelPower;\n");
            outString += "    vec3 maskedF0 = qt_F0_ior(IOR, METALNESS, fresnelColor.xyz);\n";
            outString += "    vec3 fresnelValue = qt_principledMaterialFresnel(NORMAL, VIEW_VECTOR, maskedF0, ROUGHNESS, maskedFresnelPower);\n";

            if (fresnelScaleBiasEnabled)
                outString += "    fresnelValue = clamp(vec3(fresnelBias) + fresnelScale * fresnelValue, 0.0, 1.0);\n";

            outString += "    BASE_COLOR.xyz = mix( BASE_COLOR.xyz, fresnelColor.xyz, length(fresnelValue));\n";
        }

        return outString
    }

    function snowFragment() {
        let outString = "";

        if (snowNormalMap) {



            outString += "    float snowNormalStrengthValue = snowNormalStrength;\n" +

                    maskByVertexColor("    snowNormalStrengthValue",
                                      snowNormalStrengthVertexColorMaskChannel,
                                      "snowNormalStrengthVertexColorMaskInfluence",
                                      snowNormalStrengthVertexColorMaskInfluence)


                    + "

            float alignment = dot(-normalize(NORMAL), snowDirection);
            alignment = smoothstep(snowDirectionFadeStart, snowDirectionFadeEnd, alignment);

           tempUV = transformUV(pUV,
                snowDisplacementMapFlipU,
                snowDisplacementMapFlipV,
                snowDisplacementMapPivotU,
                snowDisplacementMapPivotV,
                snowDisplacementMapPositionU,
                snowDisplacementMapPositionV,
                snowDisplacementMapRotationUV,
                snowDisplacementMapScaleU,
                snowDisplacementMapScaleV);

            float displacementSnow = texture(snowDisplacementTexture, tempUV).r;

            alignment *= smoothstep(snowMeltFadeStart, snowMeltFadeEnd, displacementSnow * snowStrength);

            tempUV = transformUV(pUV,
                snowSparkleMapFlipU,
                snowSparkleMapFlipV,
                snowSparkleMapPivotU,
                snowSparkleMapPivotV,
                snowSparkleMapPositionU,
                snowSparkleMapPositionV,
                snowSparkleMapRotationUV,
                snowSparkleMapScaleU,
                snowSparkleMapScaleV);

            vec2 uvBO = bumpOffset(tempUV, VIEW_VECTOR, snowGlintOffsetHeight, snowGlintOffsetRatio, 0.5);

            vec3 spR = texture(snowSparkleTexture, tempUV).xyz;
            vec3 spBO = texture(snowSparkleTexture, uvBO).xyz;

            spR *= spBO;

            float sp = clamp(pow(spR.x + spR.y + spR.z, snowGlintPower), 0.0, 1.0);

           tempUV = transformUV(pUV,
                snowNormalMapFlipU,
                snowNormalMapFlipV,
                snowNormalMapPivotU,
                snowNormalMapPivotV,
                snowNormalMapPositionU,
                snowNormalMapPositionV,
                snowNormalMapRotationUV,
                snowNormalMapScaleU,
                snowNormalMapScaleV);

            vec3 normalSnow = texture(snowNormalTexture, tempUV).rgb;
            normalSnow.xyz = normalSnow * 2.0 - vec3(1.0);
            normalSnow *= vec3(snowNormalStrengthValue, snowNormalStrengthValue, 1.0);
            NORMAL = mix(NORMAL, tanFrame * normalSnow, alignment);

            IOR = mix(IOR, 1.54, alignment);
            METALNESS = mix(METALNESS, 0., alignment);
            ROUGHNESS = mix(ROUGHNESS, 0.6, alignment);
            SPECULAR_AMOUNT = mix(SPECULAR_AMOUNT, 0.0, alignment);
            FRESNEL_POWER = mix(FRESNEL_POWER, 5.0, alignment);
            FRESNEL_SCALE = mix(FRESNEL_SCALE, 1.0, alignment);
            FRESNEL_BIAS = mix(FRESNEL_BIAS, 0.0, alignment);
            CLEARCOAT_AMOUNT = mix(CLEARCOAT_AMOUNT, 0., alignment);
            OCCLUSION_AMOUNT = mix(OCCLUSION_AMOUNT, 1., alignment);

            // BASE_COLOR.w = mix(BASE_COLOR.w, snowColor.w, alignment);
            // BASE_COLOR.xyz = mix(BASE_COLOR.xyz, mix(snowColor.xyz, snowGlintColor.xyz, sp), alignment);
            BASE_COLOR = mix(BASE_COLOR, snowColor, alignment);
            EMISSIVE_COLOR.xyz = mix(EMISSIVE_COLOR.xyz, snowGlintColor.xyz * sp, alignment);


        \n"
        }

        return outString
    }

    __fragmentShaderCode:
        ((heightAmount > 0 && heightMap) ? "#include \"parallaxMapping.glsllib\"\n" : "") +
        ( useFresnelAsColorValue ? "#include \"principledMaterialFresnel.glsllib\"\n" : "" ) +
        "
vec2 hash2D(vec2 p) {
    vec2 p2 = vec2(p);
    float x = fract(sin(dot(p2, vec2(127.1, 311.7))) * 43758.5453);
    float y = fract(sin(dot(p2, vec2(269.5, 183.3))) * 43758.5453);
    return vec2(x, y);
}

float hash1D(vec2 p, vec2 seed) {
    return fract(sin(dot(p, seed)) * 43758.5453123);
}

float glint(vec2 uv, float tile, vec3 camDir, vec3 viewPos, vec3 normal, float power) {
    vec2 uvr = uv * tile;
    vec2 iuv = floor(uvr);
    uvr = fract(uvr);

    float size = 0.5;

    size *= hash1D(iuv, vec2(127.1, 311.7));

    uvr += (hash2D(iuv) * 2. - 1.) * (0.5 - size);

    return smoothstep(size, size - 0.1,
         length(uvr - vec2(0.5))) *
                 pow(hash1D(iuv, vec2(2866.674, 253.822)), power);
}

vec2 transformUV(vec2 uv, float flipU, float flipV, float pivotU, float pivotV,
                 float positionU, float positionV, float rotationUV,
                 float scaleU, float scaleV) {
    uv.x = flipU - uv.x;
    uv.y = flipV - uv.y;
    uv -= vec2(pivotU, pivotV);
    uv *= vec2(scaleU, scaleV);
    float cosTheta = cos(rotationUV);
    float sinTheta = sin(rotationUV);
    uv = vec2(uv.x * cosTheta - uv.y * sinTheta, uv.x * sinTheta + uv.y * cosTheta);
    uv += vec2(pivotU, pivotV);
    uv += vec2(positionU, positionV);
    return uv;
}
VARYING vec4 vertexColor;
VARYING vec2 pUV;
vec2 bumpOffset(vec2 uv, vec3 viewDir, float height, float heightRatio, float referencePlane) {
    float offset = (height - referencePlane) * heightRatio;
    return uv - viewDir.xy * offset;
}

void MAIN() {
    vec2 texCoord = UV0;
    vec2 tempUV = texCoord;
    vec3 worldNormal = NORMAL;
    mat3 tanFrame = mat3(TANGENT, BINORMAL, worldNormal);
    IOR = indexOfRefraction;\n" +

    parallaxFragment() +

    baseColorFragment() +

    getScalerValueByChannel("    METALNESS = ",
    "metalness",
    (metalnessMap ? "metalnessTexture" : ""),
    metalnessChannel, false, "metalnessMap") +

    maskByVertexColor( "    METALNESS",
    metalnessVertexColorMaskChannel,
    "metalnessVertexColorMaskInfluence",
    metalnessVertexColorMaskInfluence ) +

     getColorValueByChannel("    EMISSIVE_COLOR = ",
    "emissiveFactor",
    (emissiveMap ? "emissiveTexture" : ""),
    emissiveSingleChannelEnabled,
    emissiveChannel, true, "emissiveMap") +

    getScalerValueByChannel("    ROUGHNESS = ",
    "roughness",
    (roughnessMap ? "roughnessTexture" : ""),
    roughnessChannel, false, "roughnessMap") +

    maskByVertexColor("    ROUGHNESS",
    roughnessVertexColorMaskChannel,
    "roughnessVertexColorMaskInfluence",
    roughnessVertexColorMaskInfluence ) +

    getScalerValueByChannel("    SPECULAR_AMOUNT = ",
    "specularAmount",
    (specularAmountMap ? "specularAmountTexture" : ""),
    specularAmountChannel, false, "specularAmountMap") +

    maskByVertexColor( "    SPECULAR_AMOUNT",
    specularAmountVertexColorMaskChannel,
    "specularAmountVertexColorMaskInfluence",
    specularAmountVertexColorMaskInfluence ) +

    getScalerValueByChannel("    OCCLUSION_AMOUNT = ",
    "occlusionAmount",
    (occlusionMap ? "occlusionTexture" : ""),
    occlusionChannel, false, "occlusionMap") +

    maskByVertexColor( "    OCCLUSION_AMOUNT",
    occlusionAmountVertexColorMaskChannel,
    "occlusionAmountVertexColorMaskInfluence",
    occlusionAmountVertexColorMaskInfluence ) +

    normalMapFragment() +

    clearcoatFragment() +

    transmissionFragment() +

    fresnelMaskFragment() +

    snowFragment() +

    "}\n
"
    function snowVertex() {
        let outString = ""
        outString += "
        pUV = projectUV((MODEL_MATRIX * vec4(VERTEX, 1.)).xyz, snowViewProjection);
           vec2 tempUV = transformUV(pUV,
                snowDisplacementMapFlipU,
                snowDisplacementMapFlipV,
                snowDisplacementMapPivotU,
                snowDisplacementMapPivotV,
                snowDisplacementMapPositionU,
                snowDisplacementMapPositionV,
                snowDisplacementMapRotationUV,
                snowDisplacementMapScaleU,
                snowDisplacementMapScaleV);

                float displacementSnow = texture(snowDisplacementTexture, tempUV).r;

        vec3 n = NORMAL_MATRIX * NORMAL;
        float alignment = dot(-normalize(n), snowDirection);
        alignment = smoothstep(snowDirectionFadeStart, snowDirectionFadeEnd, alignment);
        alignment *= smoothstep(snowMeltFadeStart, snowMeltFadeEnd, displacementSnow * snowStrength);
        VERTEX += n * alignment * snowMaxDisplacement;"
        return outString
    }

        __vertexShaderCode:
            "VARYING vec4 vertexColor;
VARYING vec2 pUV;

vec2 transformUV(vec2 uv, float flipU, float flipV, float pivotU, float pivotV,
                 float positionU, float positionV, float rotationUV,
                 float scaleU, float scaleV) {
    uv.x = flipU - uv.x;
    uv.y = flipV - uv.y;
    uv -= vec2(pivotU, pivotV);
    uv *= vec2(scaleU, scaleV);
    float cosTheta = cos(rotationUV);
    float sinTheta = sin(rotationUV);
    uv = vec2(uv.x * cosTheta - uv.y * sinTheta, uv.x * sinTheta + uv.y * cosTheta);
    uv += vec2(pivotU, pivotV);
    uv += vec2(positionU, positionV);
    return uv;
}

vec2 projectUV(vec3 position, mat4 viewProjection) {
    vec4 clipPos = viewProjection * vec4(position, 1.0);
    vec3 ndc = clipPos.xyz / clipPos.w;
    return ndc.xy * 0.5 + 0.5;
}

void MAIN() {
    POINT_SIZE = pointSize;
    vertexColor = COLOR;

    vec3 up = normalize(abs(NORMAL.x) > 0.5 ? vec3(0, 1, 0) : vec3(1, 0, 0));
    TANGENT = normalize(cross(up, NORMAL));
    BINORMAL = normalize(cross(NORMAL, TANGENT));

    "+
        snowVertex() +
        "}\n"
    }
