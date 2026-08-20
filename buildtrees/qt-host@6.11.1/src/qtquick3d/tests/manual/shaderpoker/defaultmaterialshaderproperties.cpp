// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "defaultmaterialshaderproperties.h"

DefaultMaterialShaderProperties::DefaultMaterialShaderProperties() {}

QSSGShaderDefaultMaterialKeyProperties DefaultMaterialShaderProperties::properties() const
{
    return m_properties;
}

QSSGShaderDefaultMaterialKey DefaultMaterialShaderProperties::key() const
{
    return m_key;
}

bool DefaultMaterialShaderProperties::hasLighting() const
{
    return m_properties.m_hasLighting.getValue(m_key);
}

void DefaultMaterialShaderProperties::setHasLighting(bool newHasLighting)
{
    if (this->hasLighting() == newHasLighting)
        return;

    m_properties.m_hasLighting.setValue(m_key, newHasLighting);
    emit hasLightingChanged();
    updated();
}

bool DefaultMaterialShaderProperties::hasIbl() const
{
    return m_properties.m_hasIbl.getValue(m_key);
}

void DefaultMaterialShaderProperties::setHasIbl(bool newHasIbl)
{
    if (this->hasIbl() == newHasIbl)
        return;

    m_properties.m_hasIbl.setValue(m_key, newHasIbl);
    emit hasIblChanged();
    updated();
}

bool DefaultMaterialShaderProperties::fresnelScaleBiasEnabled() const
{
    return m_properties.m_fresnelScaleBiasEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setFresnelScaleBiasEnabled(bool newFresnelScaleBiasEnabled)
{
    if (fresnelScaleBiasEnabled() == newFresnelScaleBiasEnabled)
        return;
    m_properties.m_fresnelScaleBiasEnabled.setValue(m_key, newFresnelScaleBiasEnabled);
    emit fresnelScaleBiasEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::clearcoatFresnelScaleBiasEnabled() const
{
    return m_properties.m_clearcoatFresnelScaleBiasEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatFresnelScaleBiasEnabled(bool newClearcoatFresnelScaleBiasEnabled)
{
    if (clearcoatFresnelScaleBiasEnabled() == newClearcoatFresnelScaleBiasEnabled)
        return;
    m_properties.m_clearcoatFresnelScaleBiasEnabled.setValue(m_key, newClearcoatFresnelScaleBiasEnabled);
    emit clearcoatFresnelScaleBiasEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::fresnelEnabled() const
{
    return m_properties.m_fresnelEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setFresnelEnabled(bool newFresnelEnabled)
{
    if (fresnelEnabled() == newFresnelEnabled)
        return;
    m_properties.m_fresnelEnabled.setValue(m_key, newFresnelEnabled);
    emit fresnelEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::baseColorSingleChannelEnabled() const
{
    return m_properties.m_baseColorSingleChannelEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setBaseColorSingleChannelEnabled(bool newBaseColorSingleChannelEnabled)
{
    if (baseColorSingleChannelEnabled() == newBaseColorSingleChannelEnabled)
        return;
    m_properties.m_baseColorSingleChannelEnabled.setValue(m_key, newBaseColorSingleChannelEnabled);
    emit baseColorSingleChannelEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::specularSingleChannelEnabled() const
{
    return m_properties.m_specularSingleChannelEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularSingleChannelEnabled(bool newSpecularSingleChannelEnabled)
{
    if (specularSingleChannelEnabled() == newSpecularSingleChannelEnabled)
        return;
    m_properties.m_specularSingleChannelEnabled.setValue(m_key, newSpecularSingleChannelEnabled);
    emit specularSingleChannelEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::emissiveSingleChannelEnabled() const
{
    return m_properties.m_emissiveSingleChannelEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setEmissiveSingleChannelEnabled(bool newEmissiveSingleChannelEnabled)
{
    if (emissiveSingleChannelEnabled() == newEmissiveSingleChannelEnabled)
        return;
    m_properties.m_emissiveSingleChannelEnabled.setValue(m_key, newEmissiveSingleChannelEnabled);
    emit emissiveSingleChannelEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::invertOpacityMapValue() const
{
    return m_properties.m_invertOpacityMapValue.getValue(m_key);
}

void DefaultMaterialShaderProperties::setInvertOpacityMapValue(bool newInvertOpacityMapValue)
{
    if (invertOpacityMapValue() == newInvertOpacityMapValue)
        return;
    m_properties.m_invertOpacityMapValue.setValue(m_key, newInvertOpacityMapValue);
    emit invertOpacityMapValueChanged();
    updated();
}

bool DefaultMaterialShaderProperties::vertexColorsEnabled() const
{
    return m_properties.m_vertexColorsEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorsEnabled(bool newVertexColorsEnabled)
{
    if (vertexColorsEnabled() == newVertexColorsEnabled)
        return;
    m_properties.m_vertexColorsEnabled.setValue(m_key, newVertexColorsEnabled);
    emit vertexColorsEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::vertexColorsMaskEnabled() const
{
    return m_properties.m_vertexColorsMaskEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorsMaskEnabled(bool newVertexColorsMaskEnabled)
{
    if (vertexColorsMaskEnabled() == newVertexColorsMaskEnabled)
        return;
    m_properties.m_vertexColorsMaskEnabled.setValue(m_key, newVertexColorsMaskEnabled);
    emit vertexColorsMaskEnabledChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::vertexColorRedMask() const
{
    return m_properties.m_vertexColorRedMask.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorRedMask(quint16 newVertexColorRedMask)
{
    if (vertexColorRedMask() == newVertexColorRedMask)
        return;
    m_properties.m_vertexColorRedMask.setValue(m_key, newVertexColorRedMask);
    emit vertexColorRedMaskChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::vertexColorGreenMask() const
{
    return m_properties.m_vertexColorGreenMask.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorGreenMask(quint16 newVertexColorGreenMask)
{
    if (vertexColorGreenMask() == newVertexColorGreenMask)
        return;
    m_properties.m_vertexColorGreenMask.setValue(m_key, newVertexColorGreenMask);
    emit vertexColorGreenMaskChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::vertexColorBlueMask() const
{
    return m_properties.m_vertexColorBlueMask.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorBlueMask(quint16 newVertexColorBlueMask)
{
    if (vertexColorBlueMask() == newVertexColorBlueMask)
        return;
    m_properties.m_vertexColorBlueMask.setValue(m_key, newVertexColorBlueMask);
    emit vertexColorBlueMaskChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::vertexColorAlphaMask() const
{
    return m_properties.m_vertexColorAlphaMask.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexColorAlphaMask(quint16 newVertexColorAlphaMask)
{
    if (vertexColorAlphaMask() == newVertexColorAlphaMask)
        return;
    m_properties.m_vertexColorAlphaMask.setValue(m_key, newVertexColorAlphaMask);
    emit vertexColorAlphaMaskChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::boneCount() const
{
    return m_properties.m_boneCount.getValue(m_key);
}

void DefaultMaterialShaderProperties::setBoneCount(quint16 newBoneCount)
{
    if (boneCount() == newBoneCount)
        return;
    m_properties.m_boneCount.setValue(m_key, newBoneCount);
    emit boneCountChanged();
    updated();
}

bool DefaultMaterialShaderProperties::isDoubleSided() const
{
    return m_properties.m_isDoubleSided.getValue(m_key);
}

void DefaultMaterialShaderProperties::setIsDoubleSided(bool newIsDoubleSided)
{
    if (isDoubleSided() == newIsDoubleSided)
        return;
    m_properties.m_isDoubleSided.setValue(m_key, newIsDoubleSided);
    emit isDoubleSidedChanged();
    updated();
}

bool DefaultMaterialShaderProperties::overridesPosition() const
{
    return m_properties.m_overridesPosition.getValue(m_key);
}

void DefaultMaterialShaderProperties::setOverridesPosition(bool newOverridesPosition)
{
    if (overridesPosition() == newOverridesPosition)
        return;
    m_properties.m_overridesPosition.setValue(m_key, newOverridesPosition);
    emit overridesPositionChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesProjectionMatrix() const
{
    return m_properties.m_usesProjectionMatrix.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesProjectionMatrix(bool newUsesProjectionMatrix)
{
    if (usesProjectionMatrix() == newUsesProjectionMatrix)
        return;
    m_properties.m_usesProjectionMatrix.setValue(m_key, newUsesProjectionMatrix);
    emit usesProjectionMatrixChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesInverseProjectionMatrix() const
{
    return m_properties.m_usesInverseProjectionMatrix.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesInverseProjectionMatrix(bool newUsesInverseProjectionMatrix)
{
    if (usesInverseProjectionMatrix() == newUsesInverseProjectionMatrix)
        return;
    m_properties.m_usesInverseProjectionMatrix.setValue(m_key, newUsesInverseProjectionMatrix);
    emit usesInverseProjectionMatrixChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesPointsTopology() const
{
    return m_properties.m_usesPointsTopology.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesPointsTopology(bool newUsesPointsTopology)
{
    if (usesPointsTopology() == newUsesPointsTopology)
        return;
    m_properties.m_usesPointsTopology.setValue(m_key, newUsesPointsTopology);
    emit usesPointsTopologyChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesVarColor() const
{
    return m_properties.m_usesVarColor.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesVarColor(bool newUsesVarColor)
{
    if (usesVarColor() == newUsesVarColor)
        return;
    m_properties.m_usesVarColor.setValue(m_key, newUsesVarColor);
    emit usesVarColorChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::alphaMode() const
{
    return m_properties.m_alphaMode.getValue(m_key);
}

void DefaultMaterialShaderProperties::setAlphaMode(quint8 newAlphaMode)
{
    if (alphaMode() == newAlphaMode)
        return;
    m_properties.m_alphaMode.setValue(m_key, newAlphaMode);
    emit alphaModeChanged();
    updated();
}

quint16 DefaultMaterialShaderProperties::vertexAttributes() const
{
    return m_properties.m_vertexAttributes.getValue(m_key);
}

void DefaultMaterialShaderProperties::setVertexAttributes(quint16 newVertexAttributes)
{
    if (vertexAttributes() == newVertexAttributes)
        return;
    m_properties.m_vertexAttributes.setValue(m_key, newVertexAttributes);
    emit vertexAttributesChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesFloatJointIndices() const
{
    return m_properties.m_usesFloatJointIndices.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesFloatJointIndices(bool newUsesFloatJointIndices)
{
    if (usesFloatJointIndices() == newUsesFloatJointIndices)
        return;
    m_properties.m_usesFloatJointIndices.setValue(m_key, newUsesFloatJointIndices);
    emit usesFloatJointIndicesChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesInstancing() const
{
    return m_properties.m_usesInstancing.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesInstancing(bool newUsesInstancing)
{
    if (usesInstancing() == newUsesInstancing)
        return;
    m_properties.m_usesInstancing.setValue(m_key, newUsesInstancing);
    emit usesInstancingChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetCount() const
{
    return m_properties.m_targetCount.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetCount(quint8 newTargetCount)
{
    if (targetCount() == newTargetCount)
        return;
    m_properties.m_targetCount.setValue(m_key, newTargetCount);
    emit targetCountChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetPositionOffset() const
{
    return m_properties.m_targetPositionOffset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetPositionOffset(quint8 newTargetPositionOffset)
{
    if (targetPositionOffset() == newTargetPositionOffset)
        return;
    m_properties.m_targetPositionOffset.setValue(m_key, newTargetPositionOffset);
    emit targetPositionOffsetChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetTangentOffset() const
{
    return m_properties.m_targetTangentOffset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetTangentOffset(quint8 newTargetTangentOffset)
{
    if (targetTangentOffset() == newTargetTangentOffset)
        return;
    m_properties.m_targetTangentOffset.setValue(m_key, newTargetTangentOffset);
    emit targetTangentOffsetChanged();
}

quint8 DefaultMaterialShaderProperties::targetNormalOffset() const
{
    return m_properties.m_targetNormalOffset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetNormalOffset(quint8 newTargetNormalOffset)
{
    if (targetNormalOffset() == newTargetNormalOffset)
        return;
    m_properties.m_targetNormalOffset.setValue(m_key, newTargetNormalOffset);
    emit targetNormalOffsetChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetBinormalOffset() const
{
    return m_properties.m_targetBinormalOffset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetBinormalOffset(quint8 newTargetBinormalOffset)
{
    if (targetBinormalOffset() == newTargetBinormalOffset)
        return;
    m_properties.m_targetBinormalOffset.setValue(m_key, newTargetBinormalOffset);
    emit targetBinormalOffsetChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetTexCoord0Offset() const
{
    return m_properties.m_targetTexCoord0Offset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetTexCoord0Offset(quint8 newTargetTexCoord0Offset)
{
    if (targetTexCoord0Offset() == newTargetTexCoord0Offset)
        return;
    m_properties.m_targetTexCoord0Offset.setValue(m_key, newTargetTexCoord0Offset);
    emit targetTexCoord0OffsetChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetTexCoord1Offset() const
{
    return m_properties.m_targetTexCoord1Offset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetTexCoord1Offset(quint8 newTargetTexCoord1Offset)
{
    if (targetTexCoord1Offset() == newTargetTexCoord1Offset)
        return;
    m_properties.m_targetTexCoord1Offset.setValue(m_key, newTargetTexCoord1Offset);
    emit targetTexCoord1OffsetChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::targetColorOffset() const
{
    return m_properties.m_targetColorOffset.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTargetColorOffset(quint8 newTargetColorOffset)
{
    if (targetColorOffset() == newTargetColorOffset)
        return;
    m_properties.m_targetColorOffset.setValue(m_key, newTargetColorOffset);
    emit targetColorOffsetChanged();
    updated();
}

bool DefaultMaterialShaderProperties::blendParticles() const
{
    return m_properties.m_blendParticles.getValue(m_key);
}

void DefaultMaterialShaderProperties::setBlendParticles(bool newBlendParticles)
{
    if (blendParticles() == newBlendParticles)
        return;
    m_properties.m_blendParticles.setValue(m_key, newBlendParticles);
    emit blendParticlesChanged();
    updated();
}

bool DefaultMaterialShaderProperties::clearcoatEnabled() const
{
    return m_properties.m_clearcoatEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatEnabled(bool newClearcoatEnabled)
{
    if (clearcoatEnabled() == newClearcoatEnabled)
        return;
    m_properties.m_clearcoatEnabled.setValue(m_key, newClearcoatEnabled);
    emit clearcoatEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::transmissionEnabled() const
{
    return m_properties.m_transmissionEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setTransmissionEnabled(bool newTransmissionEnabled)
{
    if (transmissionEnabled() == newTransmissionEnabled)
        return;
    m_properties.m_transmissionEnabled.setValue(m_key, newTransmissionEnabled);
    emit transmissionEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::specularAAEnabled() const
{
    return m_properties.m_specularAAEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularAAEnabled(bool newSpecularAAEnabled)
{
    if (specularAAEnabled() == newSpecularAAEnabled)
        return;
    m_properties.m_specularAAEnabled.setValue(m_key, newSpecularAAEnabled);
    emit specularAAEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::lightmapEnabled() const
{
    return m_properties.m_lightmapEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setLightmapEnabled(bool newLightmapEnabled)
{
    if (lightmapEnabled() == newLightmapEnabled)
        return;
    m_properties.m_lightmapEnabled.setValue(m_key, newLightmapEnabled);
    emit lightmapEnabledChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::debugMode() const
{
    return m_properties.m_debugMode.getValue(m_key);
}

void DefaultMaterialShaderProperties::setDebugMode(quint8 newDebugMode)
{
    if (debugMode() == newDebugMode)
        return;
    m_properties.m_debugMode.setValue(m_key, newDebugMode);
    emit debugModeChanged();
    updated();
}

bool DefaultMaterialShaderProperties::fogEnabled() const
{
    return m_properties.m_fogEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setFogEnabled(bool newFogEnabled)
{
    if (fogEnabled() == newFogEnabled)
        return;
    m_properties.m_fogEnabled.setValue(m_key, newFogEnabled);
    emit fogEnabledChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::viewCount() const
{
    return m_properties.m_viewCount.getValue(m_key);
}

void DefaultMaterialShaderProperties::setViewCount(quint8 newViewCount)
{
    if (viewCount() == newViewCount)
        return;
    m_properties.m_viewCount.setValue(m_key, newViewCount);
    emit viewCountChanged();
    updated();
}

bool DefaultMaterialShaderProperties::usesViewIndex() const
{
    return m_properties.m_usesViewIndex.getValue(m_key);
}

void DefaultMaterialShaderProperties::setUsesViewIndex(bool newUsesViewIndex)
{
    if (usesViewIndex() == newUsesViewIndex)
        return;
    m_properties.m_usesViewIndex.setValue(m_key, newUsesViewIndex);
    emit usesViewIndexChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::orderIndependentTransparency() const
{
    return m_properties.m_orderIndependentTransparency.getValue(m_key);
}

void DefaultMaterialShaderProperties::setOrderIndependentTransparency(quint8 newOrderIndependentTransparency)
{
    if (orderIndependentTransparency() == newOrderIndependentTransparency)
        return;
    m_properties.m_orderIndependentTransparency.setValue(m_key, newOrderIndependentTransparency);
    emit orderIndependentTransparencyChanged();
    updated();
}

void DefaultMaterialShaderProperties::updated()
{
    emit shaderKeyUpdated();
}

quint8 DefaultMaterialShaderProperties::diffuseMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::DiffuseMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setDiffuseMap(quint8 newDiffuseMap)
{
    if (diffuseMap() == newDiffuseMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::DiffuseMap].setValue(m_key, newDiffuseMap);
    emit diffuseMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::emissiveMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::EmissiveMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setEmissiveMap(quint8 newEmissiveMap)
{
    if (emissiveMap() == newEmissiveMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::EmissiveMap].setValue(m_key, newEmissiveMap);
    emit emissiveMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::specularMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::SpecularMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularMap(quint8 newSpecularMap)
{
    if (specularMap() == newSpecularMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::SpecularMap].setValue(m_key, newSpecularMap);
    emit specularMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::baseColorMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BaseColorMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setBaseColorMap(quint8 newBaseColorMap)
{
    if (baseColorMap() == newBaseColorMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BaseColorMap].setValue(m_key, newBaseColorMap);
    emit baseColorMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::bumpMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BumpMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setBumpMap(quint8 newBumpMap)
{
    if (bumpMap() == newBumpMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BumpMap].setValue(m_key, newBumpMap);
    emit bumpMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::specularAmountMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::SpecularAmountMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularAmountMap(quint8 newSpecularAmountMap)
{
    if (specularAmountMap() == newSpecularAmountMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::SpecularAmountMap].setValue(m_key, newSpecularAmountMap);
    emit specularAmountMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::normalMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::NormalMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setNormalMap(quint8 newNormalMap)
{
    if (normalMap() == newNormalMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::NormalMap].setValue(m_key, newNormalMap);
    emit normalMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::clearcoatNormalMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatNormalMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatNormalMap(quint8 newClearcoatNormalMap)
{
    if (clearcoatNormalMap() == newClearcoatNormalMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatNormalMap].setValue(m_key, newClearcoatNormalMap);
    emit clearcoatNormalMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::opacityMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::OpacityMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setOpacityMap(quint8 newOpacityMap)
{
    if (opacityMap() == newOpacityMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::OpacityMap].setValue(m_key, newOpacityMap);
    emit opacityMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::roughnessMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::RoughnessMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setRoughnessMap(quint8 newRoughnessMap)
{
    if (roughnessMap() == newRoughnessMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::RoughnessMap].setValue(m_key, newRoughnessMap);
    emit roughnessMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::metalnessMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::MetalnessMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setMetalnessMap(quint8 newMetalnessMap)
{
    if (metalnessMap() == newMetalnessMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::MetalnessMap].setValue(m_key, newMetalnessMap);
    emit metalnessMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::occlusionMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::OcclusionMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setOcclusionMap(quint8 newOcclusionMap)
{
    if (occlusionMap() == newOcclusionMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::OcclusionMap].setValue(m_key, newOcclusionMap);
    emit occlusionMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::translucencyMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::TranslucencyMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setTranslucencyMap(quint8 newTranslucencyMap)
{
    if (translucencyMap() == newTranslucencyMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::TranslucencyMap].setValue(m_key, newTranslucencyMap);
    emit translucencyMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::heightMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::HeightMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setHeightMap(quint8 newHeightMap)
{
    if (heightMap() == newHeightMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::HeightMap].setValue(m_key, newHeightMap);
    emit heightMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::clearcoatMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatMap(quint8 newClearcoatMap)
{
    if (clearcoatMap() == newClearcoatMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatMap].setValue(m_key, newClearcoatMap);
    emit clearcoatMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::clearcoatRoughnessMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatRoughnessMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatRoughnessMap(quint8 newClearcoatRoughnessMap)
{
    if (clearcoatRoughnessMap() == newClearcoatRoughnessMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ClearcoatRoughnessMap].setValue(m_key, newClearcoatRoughnessMap);
    emit clearcoatRoughnessMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::transmissionMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::TransmissionMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setTransmissionMap(quint8 newTransmissionMap)
{
    if (transmissionMap() == newTransmissionMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::TransmissionMap].setValue(m_key, newTransmissionMap);
    emit transmissionMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::thicknessMap() const
{
    return m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ThicknessMap].getValue(m_key);
}

void DefaultMaterialShaderProperties::setThicknessMap(quint8 newThicknessMap)
{
    if (thicknessMap() == newThicknessMap)
        return;
    m_properties.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::ThicknessMap].setValue(m_key, newThicknessMap);
    emit thicknessMapChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::roughnessChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::RoughnessChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setRoughnessChannel(quint8 newRoughnessChannel)
{
    if (roughnessChannel() == newRoughnessChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::RoughnessChannel].setValue(m_key, newRoughnessChannel);
    emit roughnessChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::metalnessChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::MetalnessChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setMetalnessChannel(quint8 newMetalnessChannel)
{
    if (metalnessChannel() == newMetalnessChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::MetalnessChannel].setValue(m_key, newMetalnessChannel);
    emit metalnessChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::occlusionChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OcclusionChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setOcclusionChannel(quint8 newOcclusionChannel)
{
    if (occlusionChannel() == newOcclusionChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OcclusionChannel].setValue(m_key, newOcclusionChannel);
    emit occlusionChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::translucencyChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TranslucencyChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setTranslucencyChannel(quint8 newTranslucencyChannel)
{
    if (translucencyChannel() == newTranslucencyChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TranslucencyChannel].setValue(m_key, newTranslucencyChannel);
    emit translucencyChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::heightChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::HeightChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setHeightChannel(quint8 newHeightChannel)
{
    if (heightChannel() == newHeightChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::HeightChannel].setValue(m_key, newHeightChannel);
    emit heightChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::clearcoatChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatChannel(quint8 newClearcoatChannel)
{
    if (clearcoatChannel() == newClearcoatChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatChannel].setValue(m_key, newClearcoatChannel);
    emit clearcoatChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::clearcoatRoughnessChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatRoughnessChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setClearcoatRoughnessChannel(quint8 newClearcoatRoughnessChannel)
{
    if (clearcoatRoughnessChannel() == newClearcoatRoughnessChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatRoughnessChannel].setValue(m_key, newClearcoatRoughnessChannel);
    emit clearcoatRoughnessChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::transmissionChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TransmissionChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setTransmissionChannel(quint8 newTransmissionChannel)
{
    if (transmissionChannel() == newTransmissionChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TransmissionChannel].setValue(m_key, newTransmissionChannel);
    emit transmissionChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::thicknessChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ThicknessChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setThicknessChannel(quint8 newThicknessChannel)
{
    if (thicknessChannel() == newThicknessChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ThicknessChannel].setValue(m_key, newThicknessChannel);
    emit thicknessChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::baseColorChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::BaseColorChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setBaseColorChannel(quint8 newBaseColorChannel)
{
    if (baseColorChannel() == newBaseColorChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::BaseColorChannel].setValue(m_key, newBaseColorChannel);
    emit baseColorChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::specularAmountChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::SpecularAmountChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularAmountChannel(quint8 newSpecularAmountChannel)
{
    if (specularAmountChannel() == newSpecularAmountChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::SpecularAmountChannel].setValue(m_key, newSpecularAmountChannel);
    emit specularAmountChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::emissiveChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::EmissiveChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setEmissiveChannel(quint8 newEmissiveChannel)
{
    if (emissiveChannel() == newEmissiveChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::EmissiveChannel].setValue(m_key, newEmissiveChannel);
    emit emissiveChannelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::opacityChannel() const
{
    return m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OpacityChannel].getValue(m_key);
}

void DefaultMaterialShaderProperties::setOpacityChannel(quint8 newOpacityChannel)
{
    if (opacityChannel() == newOpacityChannel)
        return;
    m_properties.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OpacityChannel].setValue(m_key, newOpacityChannel);
    emit opacityChannelChanged();
    updated();
}

bool DefaultMaterialShaderProperties::hasPunctualLights() const
{
    return m_properties.m_hasPunctualLights.getValue(m_key);
}

void DefaultMaterialShaderProperties::setHasPunctualLights(bool newHasPunctualLights)
{
    if (hasPunctualLights() == newHasPunctualLights)
        return;

    m_properties.m_hasPunctualLights.setValue(m_key, newHasPunctualLights);
    emit hasPunctualLightsChanged();
    updated();
}

bool DefaultMaterialShaderProperties::hasShadows() const
{
    return m_properties.m_hasShadows.getValue(m_key);
}

void DefaultMaterialShaderProperties::setHasShadows(bool newHasShadows)
{
    if (hasShadows() == newHasShadows)
        return;

    m_properties.m_hasShadows.setValue(m_key, newHasShadows);
    emit hasShadowsChanged();
    updated();
}

bool DefaultMaterialShaderProperties::specularEnabled() const
{
    return m_properties.m_specularEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularEnabled(bool newSpecularEnabled)
{
    if (specularEnabled() == newSpecularEnabled)
        return;

    m_properties.m_specularEnabled.setValue(m_key, newSpecularEnabled);
    emit specularEnabledChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::specularModel() const
{
    return m_properties.m_specularModel.getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularModel(quint8 newSpecularModel)
{
    if (specularModel() == newSpecularModel)
        return;
    m_properties.m_specularModel.setValue(m_key, newSpecularModel);
    emit specularModelChanged();
    updated();
}

quint8 DefaultMaterialShaderProperties::diffuseModel() const
{
    return m_properties.m_diffuseModel.getValue(m_key);
}

void DefaultMaterialShaderProperties::setDiffuseModel(quint8 newDiffuseModel)
{
    if (diffuseModel() == newDiffuseModel)
        return;
    m_properties.m_diffuseModel.setValue(m_key, newDiffuseModel);
    emit diffuseModelChanged();
    updated();
}

bool DefaultMaterialShaderProperties::specularGlossyEnabled() const
{
    return m_properties.m_specularGlossyEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setSpecularGlossyEnabled(bool newSpecularGlossyEnabled)
{
    if (specularGlossyEnabled() == newSpecularGlossyEnabled)
        return;
    m_properties.m_specularGlossyEnabled.setValue(m_key, newSpecularGlossyEnabled);
    emit specularGlossyEnabledChanged();
    updated();
}

bool DefaultMaterialShaderProperties::metallicRoughnessEnabled() const
{
    return m_properties.m_metallicRoughnessEnabled.getValue(m_key);
}

void DefaultMaterialShaderProperties::setMetallicRoughnessEnabled(bool newMetallicRoughnessEnabled)
{
    if (metallicRoughnessEnabled() == newMetallicRoughnessEnabled)
        return;
    m_properties.m_metallicRoughnessEnabled.setValue(m_key, newMetallicRoughnessEnabled);
    emit metallicRoughnessEnabledChanged();
    updated();
}
