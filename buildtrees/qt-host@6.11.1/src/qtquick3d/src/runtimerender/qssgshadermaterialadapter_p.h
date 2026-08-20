// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_SHADER_MATERIAL_ADAPTER_H
#define QSSG_SHADER_MATERIAL_ADAPTER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadowmap_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>

QT_BEGIN_NAMESPACE

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGShaderMaterialAdapter
{
    static QSSGShaderMaterialAdapter *create(const QSSGRenderGraphObject &materialNode);
    virtual ~QSSGShaderMaterialAdapter();

    virtual bool isPrincipled() = 0;
    virtual bool isSpecularGlossy() = 0;
    virtual bool isMetalnessEnabled() = 0;
    virtual bool isSpecularEnabled() = 0;
    virtual bool isVertexColorsEnabled() = 0;
    virtual bool isVertexColorsMaskEnabled() = 0;
    virtual bool isInvertOpacityMapValue() = 0;
    virtual bool isBaseColorSingleChannelEnabled() = 0;
    virtual bool isSpecularAmountSingleChannelEnabled() = 0;
    virtual bool isEmissiveSingleChannelEnabled() = 0;
    virtual bool isFresnelScaleBiasEnabled() = 0;
    virtual bool isClearcoatFresnelScaleBiasEnabled() = 0;
    virtual bool isClearcoatEnabled() = 0;
    virtual bool isTransmissionEnabled() = 0;
    virtual bool hasLighting() = 0;
    virtual bool usesCustomSkinning() = 0;
    virtual bool usesCustomMorphing() = 0;
    virtual QSSGRenderDefaultMaterial::MaterialSpecularModel specularModel() = 0;
    virtual QSSGRenderDefaultMaterial::MaterialAlphaMode alphaMode() = 0;
    virtual QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorRedMask() = 0;
    virtual QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorGreenMask() = 0;
    virtual QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorBlueMask() = 0;
    virtual QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorAlphaMask() = 0;

    virtual QSSGRenderImage *iblProbe() = 0;
    virtual QVector3D emissiveColor() = 0;
    virtual QVector4D color() = 0;
    virtual QVector3D specularTint() = 0;
    virtual float ior() = 0;
    virtual float fresnelScale() = 0;
    virtual float fresnelBias() = 0;
    virtual float fresnelPower() = 0;
    virtual float clearcoatFresnelScale() = 0;
    virtual float clearcoatFresnelBias() = 0;
    virtual float clearcoatFresnelPower() = 0;
    virtual float metalnessAmount() = 0;
    virtual float specularAmount() = 0;
    virtual float specularRoughness() = 0;
    virtual float bumpAmount() = 0;
    virtual float translucentFallOff() = 0;
    virtual float diffuseLightWrap() = 0;
    virtual float occlusionAmount() = 0;
    virtual float alphaCutOff() = 0;
    virtual float pointSize() = 0;
    virtual float lineWidth() = 0;
    virtual float heightAmount() = 0;
    virtual float minHeightSamples() = 0;
    virtual float maxHeightSamples() = 0;
    virtual float clearcoatAmount() = 0;
    virtual float clearcoatRoughnessAmount() = 0;
    virtual float clearcoatNormalStrength() = 0;
    virtual float transmissionFactor() = 0;
    virtual float thicknessFactor() = 0;
    virtual float attenuationDistance() = 0;
    virtual QVector3D attenuationColor() = 0;

    virtual bool isUnshaded();
    virtual bool hasCustomShaderSnippet(QSSGShaderCache::ShaderType type);
    virtual QByteArray customShaderSnippet(QSSGShaderCache::ShaderType type,
                                           QSSGShaderLibraryManager &shaderLibraryManager,
                                           bool multiViewCompatible);
    virtual bool hasCustomShaderFunction(QSSGShaderCache::ShaderType shaderType,
                                         const QByteArray &funcName,
                                         QSSGShaderLibraryManager &shaderLibraryManager);
    virtual void setCustomPropertyUniforms(char *ubufData,
                                           QSSGRhiShaderPipeline &shaderPipeline,
                                           const QSSGRenderContextInterface &context);
    virtual bool usesSharedVariables();
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGShaderDefaultMaterialAdapter final : public QSSGShaderMaterialAdapter
{
    QSSGShaderDefaultMaterialAdapter(const QSSGRenderDefaultMaterial &material);

    bool isPrincipled() override;
    bool isSpecularGlossy() override;
    bool isMetalnessEnabled() override;
    bool isSpecularEnabled() override;
    bool isVertexColorsEnabled() override;
    bool isVertexColorsMaskEnabled() override;
    bool isInvertOpacityMapValue() override;
    bool isBaseColorSingleChannelEnabled() override;
    bool isSpecularAmountSingleChannelEnabled() override;
    bool isEmissiveSingleChannelEnabled() override;
    bool isClearcoatEnabled() override;
    bool isTransmissionEnabled() override;
    bool isFresnelScaleBiasEnabled() override;
    bool isClearcoatFresnelScaleBiasEnabled() override;
    bool hasLighting() override;
    bool usesCustomSkinning() override;
    bool usesCustomMorphing() override;
    QSSGRenderDefaultMaterial::MaterialSpecularModel specularModel() override;
    QSSGRenderDefaultMaterial::MaterialAlphaMode alphaMode() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorRedMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorGreenMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorBlueMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorAlphaMask() override;

    QSSGRenderImage *iblProbe() override;
    QVector3D emissiveColor() override;
    QVector4D color() override;
    QVector3D specularTint() override;
    float ior() override;
    float fresnelScale() override;
    float fresnelBias() override;
    float fresnelPower() override;
    float clearcoatFresnelScale() override;
    float clearcoatFresnelBias() override;
    float clearcoatFresnelPower() override;
    float metalnessAmount() override;
    float specularAmount() override;
    float specularRoughness() override;
    float bumpAmount() override;
    float translucentFallOff() override;
    float diffuseLightWrap() override;
    float occlusionAmount() override;
    float alphaCutOff() override;
    float pointSize() override;
    float lineWidth() override;
    float heightAmount() override;
    float minHeightSamples() override;
    float maxHeightSamples() override;
    float clearcoatAmount() override;
    float clearcoatRoughnessAmount() override;
    float clearcoatNormalStrength() override;
    float transmissionFactor() override;
    float thicknessFactor() override;
    float attenuationDistance() override;
    QVector3D attenuationColor() override;

private:
    const QSSGRenderDefaultMaterial &m_material;
};

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGShaderCustomMaterialAdapter final : public QSSGShaderMaterialAdapter
{
    QSSGShaderCustomMaterialAdapter(const QSSGRenderCustomMaterial &material);

    bool isPrincipled() override;
    bool isSpecularGlossy() override;
    bool isMetalnessEnabled() override;
    bool isSpecularEnabled() override;
    bool isVertexColorsEnabled() override;
    bool isVertexColorsMaskEnabled() override;
    bool isInvertOpacityMapValue() override;
    bool isBaseColorSingleChannelEnabled() override;
    bool isSpecularAmountSingleChannelEnabled() override;
    bool isEmissiveSingleChannelEnabled() override;
    bool isClearcoatEnabled() override;
    bool isTransmissionEnabled() override;
    bool isFresnelScaleBiasEnabled() override;
    bool isClearcoatFresnelScaleBiasEnabled() override;
    bool hasLighting() override;
    bool usesCustomSkinning() override;
    bool usesCustomMorphing() override;
    QSSGRenderDefaultMaterial::MaterialSpecularModel specularModel() override;
    QSSGRenderDefaultMaterial::MaterialAlphaMode alphaMode() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorRedMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorGreenMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorBlueMask() override;
    QSSGRenderDefaultMaterial::VertexColorMaskFlags vertexColorAlphaMask() override;

    QSSGRenderImage *iblProbe() override;
    QVector3D emissiveColor() override;
    QVector4D color() override;
    QVector3D specularTint() override;
    float ior() override;
    float fresnelScale() override;
    float fresnelBias() override;
    float fresnelPower() override;
    float clearcoatFresnelScale() override;
    float clearcoatFresnelBias() override;
    float clearcoatFresnelPower() override;
    float metalnessAmount() override;
    float specularAmount() override;
    float specularRoughness() override;
    float bumpAmount() override;
    float translucentFallOff() override;
    float diffuseLightWrap() override;
    float occlusionAmount() override;
    float alphaCutOff() override;
    float pointSize() override;
    float lineWidth() override;
    float heightAmount() override;
    float minHeightSamples() override;
    float maxHeightSamples() override;
    float clearcoatAmount() override;
    float clearcoatRoughnessAmount() override;
    float clearcoatNormalStrength() override;
    float transmissionFactor() override;
    float thicknessFactor() override;
    float attenuationDistance() override;
    QVector3D attenuationColor() override;

    bool isUnshaded() override;
    bool hasCustomShaderSnippet(QSSGShaderCache::ShaderType type) override;
    QByteArray customShaderSnippet(QSSGShaderCache::ShaderType type,
                                   QSSGShaderLibraryManager &shaderLibraryManager,
                                   bool multiViewCompatible) override;
    bool hasCustomShaderFunction(QSSGShaderCache::ShaderType shaderType,
                                 const QByteArray &funcName,
                                 QSSGShaderLibraryManager &shaderLibraryManager) override;
    void setCustomPropertyUniforms(char *ubufData,
                                   QSSGRhiShaderPipeline &shaderPipeline,
                                   const QSSGRenderContextInterface &context) override;
    bool usesSharedVariables() override;

    using StringPair = QPair<QByteArray, QByteArray>;
    using StringPairList = QVarLengthArray<StringPair, 16>;
    using ShaderCodeAndMetaData = QPair<QByteArray, QSSGCustomShaderMetaData>;

    struct CustomShaderPrepWorkData {
        QByteArrayList inputs;
        QByteArrayList outputs;
    };

    static void beginPrepareCustomShader(CustomShaderPrepWorkData *workData,
                                         ShaderCodeAndMetaData *codeAndMetaData,
                                         const QByteArray &shaderCode,
                                         QSSGShaderCache::ShaderType type,
                                         bool multiViewCompatible);

    static void finishPrepareCustomShader(QByteArray *dst,
                                          const CustomShaderPrepWorkData &workData,
                                          const ShaderCodeAndMetaData &codeAndMetaData,
                                          QSSGShaderCache::ShaderType type,
                                          bool multiViewCompatible,
                                          const StringPairList &baseUniforms,
                                          const StringPairList &baseInputs,
                                          const StringPairList &baseOutputs,
                                          const StringPairList &multiViewDependentSamplers,
                                          const StringPairList &multiViewDependentUniforms);

private:
    const QSSGRenderCustomMaterial &m_material;
};

struct QSSGCustomMaterialVariableSubstitution
{
    QByteArrayView builtin;
    QByteArrayView actualName;
    bool multiViewDependent;
};

namespace QtQuick3DEditorHelpers {
// NOTE: Returns a copy of the actual list, cache as needed!
namespace CustomMaterial {
[[nodiscard]] Q_QUICK3DRUNTIMERENDER_EXPORT QList<QByteArrayView> preprocessorVars();
}
}

QT_END_NAMESPACE

#endif
