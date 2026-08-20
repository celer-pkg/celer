// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DEFAULTMATERIALSHADERPROPERTIES_H
#define DEFAULTMATERIALSHADERPROPERTIES_H

#include <QObject>
#include <QQmlEngine>

#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
class DefaultMaterialShaderProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasLighting READ hasLighting WRITE setHasLighting NOTIFY hasLightingChanged)
    Q_PROPERTY(bool hasPunctualLights READ hasPunctualLights WRITE setHasPunctualLights NOTIFY hasPunctualLightsChanged)
    Q_PROPERTY(bool hasShadows READ hasShadows WRITE setHasShadows NOTIFY hasShadowsChanged)
    Q_PROPERTY(bool hasIbl READ hasIbl WRITE setHasIbl NOTIFY hasIblChanged)
    Q_PROPERTY(bool specularEnabled READ specularEnabled WRITE setSpecularEnabled NOTIFY specularEnabledChanged FINAL)
    Q_PROPERTY(bool fresnelScaleBiasEnabled READ fresnelScaleBiasEnabled WRITE setFresnelScaleBiasEnabled NOTIFY fresnelScaleBiasEnabledChanged)
    Q_PROPERTY(bool clearcoatFresnelScaleBiasEnabled READ clearcoatFresnelScaleBiasEnabled WRITE setClearcoatFresnelScaleBiasEnabled NOTIFY clearcoatFresnelScaleBiasEnabledChanged)
    Q_PROPERTY(bool fresnelEnabled READ fresnelEnabled WRITE setFresnelEnabled NOTIFY fresnelEnabledChanged)
    Q_PROPERTY(bool baseColorSingleChannelEnabled READ baseColorSingleChannelEnabled WRITE setBaseColorSingleChannelEnabled NOTIFY baseColorSingleChannelEnabledChanged)
    Q_PROPERTY(bool specularSingleChannelEnabled READ specularSingleChannelEnabled WRITE setSpecularSingleChannelEnabled NOTIFY specularSingleChannelEnabledChanged)
    Q_PROPERTY(bool emissiveSingleChannelEnabled READ emissiveSingleChannelEnabled WRITE setEmissiveSingleChannelEnabled NOTIFY emissiveSingleChannelEnabledChanged)
    Q_PROPERTY(bool invertOpacityMapValue READ invertOpacityMapValue WRITE setInvertOpacityMapValue NOTIFY invertOpacityMapValueChanged)
    Q_PROPERTY(bool vertexColorsEnabled READ vertexColorsEnabled WRITE setVertexColorsEnabled NOTIFY vertexColorsEnabledChanged)
    Q_PROPERTY(bool vertexColorsMaskEnabled READ vertexColorsMaskEnabled WRITE setVertexColorsMaskEnabled NOTIFY vertexColorsMaskEnabledChanged)
    Q_PROPERTY(quint16 vertexColorRedMask READ vertexColorRedMask WRITE setVertexColorRedMask NOTIFY vertexColorRedMaskChanged)
    Q_PROPERTY(quint16 vertexColorGreenMask READ vertexColorGreenMask WRITE setVertexColorGreenMask NOTIFY vertexColorGreenMaskChanged)
    Q_PROPERTY(quint16 vertexColorBlueMask READ vertexColorBlueMask WRITE setVertexColorBlueMask NOTIFY vertexColorBlueMaskChanged)
    Q_PROPERTY(quint16 vertexColorAlphaMask READ vertexColorAlphaMask WRITE setVertexColorAlphaMask NOTIFY vertexColorAlphaMaskChanged)
    Q_PROPERTY(quint8 specularModel READ specularModel WRITE setSpecularModel NOTIFY specularModelChanged FINAL)
    Q_PROPERTY(quint8 diffuseModel READ diffuseModel WRITE setDiffuseModel NOTIFY diffuseModelChanged FINAL)
    Q_PROPERTY(quint8 diffuseMap READ diffuseMap WRITE setDiffuseMap NOTIFY diffuseMapChanged FINAL)
    Q_PROPERTY(quint8 emissiveMap READ emissiveMap WRITE setEmissiveMap NOTIFY emissiveMapChanged FINAL)
    Q_PROPERTY(quint8 specularMap READ specularMap WRITE setSpecularMap NOTIFY specularMapChanged FINAL)
    Q_PROPERTY(quint8 baseColorMap READ baseColorMap WRITE setBaseColorMap NOTIFY baseColorMapChanged FINAL)
    Q_PROPERTY(quint8 bumpMap READ bumpMap WRITE setBumpMap NOTIFY bumpMapChanged FINAL)
    Q_PROPERTY(quint8 specularAmountMap READ specularAmountMap WRITE setSpecularAmountMap NOTIFY specularAmountMapChanged FINAL)
    Q_PROPERTY(quint8 normalMap READ normalMap WRITE setNormalMap NOTIFY normalMapChanged FINAL)
    Q_PROPERTY(quint8 clearcoatNormalMap READ clearcoatNormalMap WRITE setClearcoatNormalMap NOTIFY clearcoatNormalMapChanged FINAL)
    Q_PROPERTY(quint8 opacityMap READ opacityMap WRITE setOpacityMap NOTIFY opacityMapChanged FINAL)
    Q_PROPERTY(quint8 roughnessMap READ roughnessMap WRITE setRoughnessMap NOTIFY roughnessMapChanged FINAL)
    Q_PROPERTY(quint8 metalnessMap READ metalnessMap WRITE setMetalnessMap NOTIFY metalnessMapChanged FINAL)
    Q_PROPERTY(quint8 occlusionMap READ occlusionMap WRITE setOcclusionMap NOTIFY occlusionMapChanged FINAL)
    Q_PROPERTY(quint8 translucencyMap READ translucencyMap WRITE setTranslucencyMap NOTIFY translucencyMapChanged FINAL)
    Q_PROPERTY(quint8 heightMap READ heightMap WRITE setHeightMap NOTIFY heightMapChanged FINAL)
    Q_PROPERTY(quint8 clearcoatMap READ clearcoatMap WRITE setClearcoatMap NOTIFY clearcoatMapChanged FINAL)
    Q_PROPERTY(quint8 clearcoatRoughnessMap READ clearcoatRoughnessMap WRITE setClearcoatRoughnessMap NOTIFY clearcoatRoughnessMapChanged FINAL)
    Q_PROPERTY(quint8 transmissionMap READ transmissionMap WRITE setTransmissionMap NOTIFY transmissionMapChanged FINAL)
    Q_PROPERTY(quint8 thicknessMap READ thicknessMap WRITE setThicknessMap NOTIFY thicknessMapChanged FINAL)

    Q_PROPERTY(quint8 opacityChannel READ opacityChannel WRITE setOpacityChannel NOTIFY opacityChannelChanged FINAL)
    Q_PROPERTY(quint8 roughnessChannel READ roughnessChannel WRITE setRoughnessChannel NOTIFY roughnessChannelChanged FINAL)
    Q_PROPERTY(quint8 metalnessChannel READ metalnessChannel WRITE setMetalnessChannel NOTIFY metalnessChannelChanged FINAL)
    Q_PROPERTY(quint8 occlusionChannel READ occlusionChannel WRITE setOcclusionChannel NOTIFY occlusionChannelChanged FINAL)
    Q_PROPERTY(quint8 translucencyChannel READ translucencyChannel WRITE setTranslucencyChannel NOTIFY translucencyChannelChanged FINAL)
    Q_PROPERTY(quint8 heightChannel READ heightChannel WRITE setHeightChannel NOTIFY heightChannelChanged FINAL)
    Q_PROPERTY(quint8 clearcoatChannel READ clearcoatChannel WRITE setClearcoatChannel NOTIFY clearcoatChannelChanged FINAL)
    Q_PROPERTY(quint8 clearcoatRoughnessChannel READ clearcoatRoughnessChannel WRITE setClearcoatRoughnessChannel NOTIFY clearcoatRoughnessChannelChanged FINAL)
    Q_PROPERTY(quint8 transmissionChannel READ transmissionChannel WRITE setTransmissionChannel NOTIFY transmissionChannelChanged FINAL)
    Q_PROPERTY(quint8 thicknessChannel READ thicknessChannel WRITE setThicknessChannel NOTIFY thicknessChannelChanged FINAL)
    Q_PROPERTY(quint8 baseColorChannel READ baseColorChannel WRITE setBaseColorChannel NOTIFY baseColorChannelChanged FINAL)
    Q_PROPERTY(quint8 specularAmountChannel READ specularAmountChannel WRITE setSpecularAmountChannel NOTIFY specularAmountChannelChanged FINAL)
    Q_PROPERTY(quint8 emissiveChannel READ emissiveChannel WRITE setEmissiveChannel NOTIFY emissiveChannelChanged FINAL)

    Q_PROPERTY(quint16 boneCount READ boneCount WRITE setBoneCount NOTIFY boneCountChanged)
    Q_PROPERTY(bool isDoubleSided READ isDoubleSided WRITE setIsDoubleSided NOTIFY isDoubleSidedChanged)
    Q_PROPERTY(bool overridesPosition READ overridesPosition WRITE setOverridesPosition NOTIFY overridesPositionChanged)
    Q_PROPERTY(bool usesProjectionMatrix READ usesProjectionMatrix WRITE setUsesProjectionMatrix NOTIFY usesProjectionMatrixChanged)
    Q_PROPERTY(bool usesInverseProjectionMatrix READ usesInverseProjectionMatrix WRITE setUsesInverseProjectionMatrix NOTIFY usesInverseProjectionMatrixChanged)
    Q_PROPERTY(bool usesPointsTopology READ usesPointsTopology WRITE setUsesPointsTopology NOTIFY usesPointsTopologyChanged)
    Q_PROPERTY(bool usesVarColor READ usesVarColor WRITE setUsesVarColor NOTIFY usesVarColorChanged)
    Q_PROPERTY(quint8 alphaMode READ alphaMode WRITE setAlphaMode NOTIFY alphaModeChanged)
    Q_PROPERTY(quint16 vertexAttributes READ vertexAttributes WRITE setVertexAttributes NOTIFY vertexAttributesChanged)
    Q_PROPERTY(bool usesFloatJointIndices READ usesFloatJointIndices WRITE setUsesFloatJointIndices NOTIFY usesFloatJointIndicesChanged)
    Q_PROPERTY(bool usesInstancing READ usesInstancing WRITE setUsesInstancing NOTIFY usesInstancingChanged)
    Q_PROPERTY(quint8 targetCount READ targetCount WRITE setTargetCount NOTIFY targetCountChanged)
    Q_PROPERTY(quint8 targetPositionOffset READ targetPositionOffset WRITE setTargetPositionOffset NOTIFY targetPositionOffsetChanged)
    Q_PROPERTY(quint8 targetNormalOffset READ targetNormalOffset WRITE setTargetNormalOffset NOTIFY targetNormalOffsetChanged)
    Q_PROPERTY(quint8 targetTangentOffset READ targetTangentOffset WRITE setTargetTangentOffset NOTIFY targetTangentOffsetChanged)
    Q_PROPERTY(quint8 targetBinormalOffset READ targetBinormalOffset WRITE setTargetBinormalOffset NOTIFY targetBinormalOffsetChanged)
    Q_PROPERTY(quint8 targetTexCoord0Offset READ targetTexCoord0Offset WRITE setTargetTexCoord0Offset NOTIFY targetTexCoord0OffsetChanged)
    Q_PROPERTY(quint8 targetTexCoord1Offset READ targetTexCoord1Offset WRITE setTargetTexCoord1Offset NOTIFY targetTexCoord1OffsetChanged)
    Q_PROPERTY(quint8 targetColorOffset READ targetColorOffset WRITE setTargetColorOffset NOTIFY targetColorOffsetChanged)
    Q_PROPERTY(bool blendParticles READ blendParticles WRITE setBlendParticles NOTIFY blendParticlesChanged)
    Q_PROPERTY(bool clearcoatEnabled READ clearcoatEnabled WRITE setClearcoatEnabled NOTIFY clearcoatEnabledChanged)
    Q_PROPERTY(bool transmissionEnabled READ transmissionEnabled WRITE setTransmissionEnabled NOTIFY transmissionEnabledChanged)
    Q_PROPERTY(bool specularAAEnabled READ specularAAEnabled WRITE setSpecularAAEnabled NOTIFY specularAAEnabledChanged)
    Q_PROPERTY(bool lightmapEnabled READ lightmapEnabled WRITE setLightmapEnabled NOTIFY lightmapEnabledChanged)
    Q_PROPERTY(bool specularGlossyEnabled READ specularGlossyEnabled WRITE setSpecularGlossyEnabled NOTIFY specularGlossyEnabledChanged FINAL)
    Q_PROPERTY(bool metallicRoughnessEnabled READ metallicRoughnessEnabled WRITE setMetallicRoughnessEnabled NOTIFY metallicRoughnessEnabledChanged FINAL)
    Q_PROPERTY(quint8 debugMode READ debugMode WRITE setDebugMode NOTIFY debugModeChanged)
    Q_PROPERTY(bool fogEnabled READ fogEnabled WRITE setFogEnabled NOTIFY fogEnabledChanged)
    Q_PROPERTY(quint8 viewCount READ viewCount WRITE setViewCount NOTIFY viewCountChanged)
    Q_PROPERTY(bool usesViewIndex READ usesViewIndex WRITE setUsesViewIndex NOTIFY usesViewIndexChanged)
    Q_PROPERTY(quint8 orderIndependentTransparency READ orderIndependentTransparency WRITE setOrderIndependentTransparency NOTIFY orderIndependentTransparencyChanged)
    QML_ELEMENT
public:
    DefaultMaterialShaderProperties();

    QSSGShaderDefaultMaterialKeyProperties properties() const;
    QSSGShaderDefaultMaterialKey key() const;

    bool hasLighting() const;
    void setHasLighting(bool newHasLighting);


    bool hasPunctualLights() const;
    void setHasPunctualLights(bool newHasPunctualLights);

    bool hasIbl() const;
    void setHasIbl(bool newHasIbl);

    bool fresnelScaleBiasEnabled() const;
    void setFresnelScaleBiasEnabled(bool newFresnelScaleBiasEnabled);

    bool clearcoatFresnelScaleBiasEnabled() const;
    void setClearcoatFresnelScaleBiasEnabled(bool newClearcoatFresnelScaleBiasEnabled);

    bool fresnelEnabled() const;
    void setFresnelEnabled(bool newFresnelEnabled);

    bool baseColorSingleChannelEnabled() const;
    void setBaseColorSingleChannelEnabled(bool newBaseColorSingleChannelEnabled);

    bool specularSingleChannelEnabled() const;
    void setSpecularSingleChannelEnabled(bool newSpecularSingleChannelEnabled);

    bool emissiveSingleChannelEnabled() const;
    void setEmissiveSingleChannelEnabled(bool newEmissiveSingleChannelEnabled);

    bool invertOpacityMapValue() const;
    void setInvertOpacityMapValue(bool newInvertOpacityMapValue);

    bool vertexColorsEnabled() const;
    void setVertexColorsEnabled(bool newVertexColorsEnabled);

    bool vertexColorsMaskEnabled() const;
    void setVertexColorsMaskEnabled(bool newVertexColorsMaskEnabled);

    quint16 vertexColorRedMask() const;
    void setVertexColorRedMask(quint16 newVertexColorRedMask);

    quint16 vertexColorGreenMask() const;
    void setVertexColorGreenMask(quint16 newVertexColorGreenMask);

    quint16 vertexColorBlueMask() const;
    void setVertexColorBlueMask(quint16 newVertexColorBlueMask);

    quint16 vertexColorAlphaMask() const;
    void setVertexColorAlphaMask(quint16 newVertexColorAlphaMask);

    quint16 boneCount() const;
    void setBoneCount(quint16 newBoneCount);

    bool isDoubleSided() const;
    void setIsDoubleSided(bool newIsDoubleSided);

    bool overridesPosition() const;
    void setOverridesPosition(bool newOverridesPosition);

    bool usesProjectionMatrix() const;
    void setUsesProjectionMatrix(bool newUsesProjectionMatrix);

    bool usesInverseProjectionMatrix() const;
    void setUsesInverseProjectionMatrix(bool newUsesInverseProjectionMatrix);

    bool usesPointsTopology() const;
    void setUsesPointsTopology(bool newUsesPointsTopology);

    bool usesVarColor() const;
    void setUsesVarColor(bool newUsesVarColor);

    quint8 alphaMode() const;
    void setAlphaMode(quint8 newAlphaMode);

    quint16 vertexAttributes() const;
    void setVertexAttributes(quint16 newVertexAttributes);

    bool usesFloatJointIndices() const;
    void setUsesFloatJointIndices(bool newUsesFloatJointIndices);

    bool usesInstancing() const;
    void setUsesInstancing(bool newUsesInstancing);

    quint8 targetCount() const;
    void setTargetCount(quint8 newTargetCount);

    quint8 targetPositionOffset() const;
    void setTargetPositionOffset(quint8 newTargetPositionOffset);

    quint8 targetTangentOffset() const;
    void setTargetTangentOffset(quint8 newTargetTangentOffset);

    quint8 targetNormalOffset() const;
    void setTargetNormalOffset(quint8 newTargetNormalOffset);

    quint8 targetBinormalOffset() const;
    void setTargetBinormalOffset(quint8 newTargetBinormalOffset);

    quint8 targetTexCoord0Offset() const;
    void setTargetTexCoord0Offset(quint8 newTargetTexCoord0Offset);

    quint8 targetTexCoord1Offset() const;
    void setTargetTexCoord1Offset(quint8 newTargetTexCoord1Offset);

    quint8 targetColorOffset() const;
    void setTargetColorOffset(quint8 newTargetColorOffset);

    bool blendParticles() const;
    void setBlendParticles(bool newBlendParticles);

    bool clearcoatEnabled() const;
    void setClearcoatEnabled(bool newClearcoatEnabled);

    bool transmissionEnabled() const;
    void setTransmissionEnabled(bool newTransmissionEnabled);

    bool specularAAEnabled() const;
    void setSpecularAAEnabled(bool newSpecularAAEnabled);

    bool lightmapEnabled() const;
    void setLightmapEnabled(bool newLightmapEnabled);

    quint8 debugMode() const;
    void setDebugMode(quint8 newDebugMode);

    bool fogEnabled() const;
    void setFogEnabled(bool newFogEnabled);

    quint8 viewCount() const;
    void setViewCount(quint8 newViewCount);

    bool usesViewIndex() const;
    void setUsesViewIndex(bool newUsesViewIndex);

    quint8 orderIndependentTransparency() const;
    void setOrderIndependentTransparency(quint8 newOrderIndependentTransparency);

    quint8 diffuseMap() const;
    void setDiffuseMap(quint8 newDiffuseMap);

    quint8 emissiveMap() const;
    void setEmissiveMap(quint8 newEmissiveMap);

    quint8 specularMap() const;
    void setSpecularMap(quint8 newSpecularMap);

    quint8 baseColorMap() const;
    void setBaseColorMap(quint8 newBaseColorMap);

    quint8 bumpMap() const;
    void setBumpMap(quint8 newBumpMap);

    quint8 specularAmountMap() const;
    void setSpecularAmountMap(quint8 newSpecularAmountMap);

    quint8 normalMap() const;
    void setNormalMap(quint8 newNormalMap);

    quint8 clearcoatNormalMap() const;
    void setClearcoatNormalMap(quint8 newClearcoatNormalMap);

    quint8 opacityMap() const;
    void setOpacityMap(quint8 newOpacityMap);

    quint8 roughnessMap() const;
    void setRoughnessMap(quint8 newRoughnessMap);

    quint8 metalnessMap() const;
    void setMetalnessMap(quint8 newMetalnessMap);

    quint8 occlusionMap() const;
    void setOcclusionMap(quint8 newOcclusionMap);

    quint8 translucencyMap() const;
    void setTranslucencyMap(quint8 newTranslucencyMap);

    quint8 heightMap() const;
    void setHeightMap(quint8 newHeightMap);

    quint8 clearcoatMap() const;
    void setClearcoatMap(quint8 newClearcoatMap);

    quint8 clearcoatRoughnessMap() const;
    void setClearcoatRoughnessMap(quint8 newClearcoatRoughnessMap);

    quint8 transmissionMap() const;
    void setTransmissionMap(quint8 newTransmissionMap);

    quint8 thicknessMap() const;
    void setThicknessMap(quint8 newThicknessMap);

    quint8 roughnessChannel() const;
    void setRoughnessChannel(quint8 newRoughnessChannel);

    quint8 metalnessChannel() const;
    void setMetalnessChannel(quint8 newMetalnessChannel);

    quint8 occlusionChannel() const;
    void setOcclusionChannel(quint8 newOcclusionChannel);

    quint8 translucencyChannel() const;
    void setTranslucencyChannel(quint8 newTranslucencyChannel);

    quint8 heightChannel() const;
    void setHeightChannel(quint8 newHeightChannel);

    quint8 clearcoatChannel() const;
    void setClearcoatChannel(quint8 newClearcoatChannel);

    quint8 clearcoatRoughnessChannel() const;
    void setClearcoatRoughnessChannel(quint8 newClearcoatRoughnessChannel);

    quint8 transmissionChannel() const;
    void setTransmissionChannel(quint8 newTransmissionChannel);

    quint8 thicknessChannel() const;
    void setThicknessChannel(quint8 newThicknessChannel);

    quint8 baseColorChannel() const;
    void setBaseColorChannel(quint8 newBaseColorChannel);

    quint8 specularAmountChannel() const;
    void setSpecularAmountChannel(quint8 newSpecularAmountChannel);

    quint8 emissiveChannel() const;
    void setEmissiveChannel(quint8 newEmissiveChannel);

    quint8 opacityChannel() const;
    void setOpacityChannel(quint8 newOpacityChannel);


    bool hasShadows() const;
    void setHasShadows(bool newHasShadows);

    bool specularEnabled() const;
    void setSpecularEnabled(bool newSpecularEnabled);

    quint8 specularModel() const;
    void setSpecularModel(quint8 newSpecularModel);

    quint8 diffuseModel() const;
    void setDiffuseModel(quint8 newDiffuseModel);

    bool specularGlossyEnabled() const;
    void setSpecularGlossyEnabled(bool newSpecularGlossyEnabled);

    bool metallicRoughnessEnabled() const;
    void setMetallicRoughnessEnabled(bool newMetallicRoughnessEnabled);

signals:
    void hasLightingChanged();
    void hasPunctualLightsChanged();
    void hasIblChanged();
    void fresnelScaleBiasEnabledChanged();
    void clearcoatFresnelScaleBiasEnabledChanged();
    void fresnelEnabledChanged();
    void baseColorSingleChannelEnabledChanged();
    void specularSingleChannelEnabledChanged();
    void emissiveSingleChannelEnabledChanged();
    void invertOpacityMapValueChanged();
    void vertexColorsEnabledChanged();
    void vertexColorsMaskEnabledChanged();
    void vertexColorRedMaskChanged();
    void vertexColorGreenMaskChanged();
    void vertexColorBlueMaskChanged();
    void vertexColorAlphaMaskChanged();
    void boneCountChanged();
    void isDoubleSidedChanged();
    void overridesPositionChanged();
    void usesProjectionMatrixChanged();
    void usesInverseProjectionMatrixChanged();
    void usesPointsTopologyChanged();
    void usesVarColorChanged();
    void alphaModeChanged();
    void vertexAttributesChanged();
    void usesFloatJointIndicesChanged();
    void usesInstancingChanged();
    void targetCountChanged();
    void targetPositionOffsetChanged();
    void targetTangentOffsetChanged();
    void targetNormalOffsetChanged();
    void targetBinormalOffsetChanged();
    void targetTexCoord0OffsetChanged();
    void targetTexCoord1OffsetChanged();
    void targetColorOffsetChanged();
    void blendParticlesChanged();
    void clearcoatEnabledChanged();
    void transmissionEnabledChanged();
    void specularAAEnabledChanged();
    void lightmapEnabledChanged();
    void debugModeChanged();
    void fogEnabledChanged();
    void viewCountChanged();
    void usesViewIndexChanged();
    void orderIndependentTransparencyChanged();

    void shaderKeyUpdated();
    void diffuseMapChanged();
    void emissiveMapChanged();
    void specularMapChanged();
    void baseColorMapChanged();
    void bumpMapChanged();
    void specularAmountMapChanged();
    void normalMapChanged();
    void clearcoatNormalMapChanged();
    void opacityMapChanged();
    void roughnessMapChanged();
    void metalnessMapChanged();
    void occlusionMapChanged();
    void translucencyMapChanged();
    void heightMapChanged();
    void clearcoatMapChanged();
    void clearcoatRoughnessMapChanged();
    void transmissionMapChanged();
    void thicknessMapChanged();

    void roughnessChannelChanged();
    void metalnessChannelChanged();
    void occlusionChannelChanged();
    void translucencyChannelChanged();
    void heightChannelChanged();
    void clearcoatChannelChanged();
    void clearcoatRoughnessChannelChanged();
    void transmissionChannelChanged();
    void thicknessChannelChanged();
    void baseColorChannelChanged();
    void specularAmountChannelChanged();
    void emissiveChannelChanged();
    void opacityChannelChanged();
    void hasShadowsChanged();
    void specularEnabledChanged();
    void specularModelChanged();
    void diffuseModelChanged();
    void specularGlossyEnabledChanged();
    void metallicRoughnessEnabledChanged();

private:
    void updated();
    QSSGShaderDefaultMaterialKeyProperties m_properties;
    QSSGShaderDefaultMaterialKey m_key;

};

#endif // DEFAULTMATERIALSHADERPROPERTIES_H
