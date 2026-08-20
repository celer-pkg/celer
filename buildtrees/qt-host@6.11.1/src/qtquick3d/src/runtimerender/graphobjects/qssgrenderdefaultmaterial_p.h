// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_DEFAULT_MATERIAL_H
#define QSSG_RENDER_DEFAULT_MATERIAL_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

struct QSSGRenderImage;
struct QSSGRenderModel;
struct QSSGShaderMaterialAdapter;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderDefaultMaterial : QSSGRenderGraphObject
{
    enum class MaterialLighting : quint8
    {
        NoLighting = 0,
        FragmentLighting
    };
    enum class MaterialBlendMode : quint8
    {
        SourceOver = 0,
        Screen,
        Multiply
    };
    enum class MaterialSpecularModel : quint8
    {
        BlinnPhong = 0,
        SchlickGGX
    };

    enum class MaterialDiffuseModel : quint8
    {
        Burley = 0,
        Lambert,
        LambertWrap
    };

    enum MaterialAlphaMode : quint8
    {
        Default = 0,
        Mask,
        Blend,
        Opaque
    };
    enum TextureChannelMapping : quint8
    {
        R = 0,
        G,
        B,
        A
    };

    enum VertexColorMask : quint16
    {
        NoMask = 0,
        RoughnessMask = 1,
        NormalStrengthMask = 2,
        SpecularAmountMask = 4,
        ClearcoatAmountMask = 8,
        ClearcoatRoughnessAmountMask = 16,
        ClearcoatNormalStrengthMask = 32,
        HeightAmountMask = 64,
        MetalnessMask = 128,
        OcclusionAmountMask = 256,
        ThicknessFactorMask = 512,
        TransmissionFactorMask = 1024
    };
    Q_DECLARE_FLAGS(VertexColorMaskFlags, VertexColorMask)

    QSSGRenderImage *colorMap = nullptr;
    // material section
    QSSGRenderImage *iblProbe = nullptr;
    QSSGRenderImage *emissiveMap = nullptr;
    QSSGRenderImage *specularReflection = nullptr;
    QSSGRenderImage *specularMap = nullptr;
    QSSGRenderImage *roughnessMap = nullptr;
    QSSGRenderImage *opacityMap = nullptr;
    QSSGRenderImage *bumpMap = nullptr;
    QSSGRenderImage *normalMap = nullptr;
    QSSGRenderImage *translucencyMap = nullptr;
    QSSGRenderImage *metalnessMap = nullptr;
    QSSGRenderImage *occlusionMap = nullptr;
    QSSGRenderImage *heightMap = nullptr;
    QSSGRenderImage *clearcoatMap = nullptr;
    QSSGRenderImage *clearcoatRoughnessMap = nullptr;
    QSSGRenderImage *clearcoatNormalMap = nullptr;
    QSSGRenderImage *transmissionMap = nullptr;
    QSSGRenderImage *thicknessMap = nullptr;

    // Note that most default values here are irrelevant as the material
    // (Default or Principled) will write its own defaults or actual values
    // during sync.
    QVector3D specularTint{ 1.0f, 1.0f, 1.0f };
    float ior = 1.45f; // relevant for Default only
    QVector3D emissiveColor = { 1.0f, 1.0f, 1.0f };
    QVector4D color{ 1.0f, 1.0f, 1.0f, 1.0f }; // colors are 0-1 normalized
    float diffuseLightWrap = 0.0f; // 0 - 1
    float fresnelScaleBiasEnabled = false;
    float fresnelScale = 1.0f;
    float fresnelBias = 0.0f;
    float fresnelPower = 0.0f;
    float clearcoatFresnelScaleBiasEnabled = false;
    float clearcoatFresnelScale = 1.0f;
    float clearcoatFresnelBias = 0.0f;
    float clearcoatFresnelPower = 5.0f;
    float specularAmount = 1.0f; // 0-1
    float specularRoughness = 0.0f; // 0-1
    float metalnessAmount = 0.0f;
    float opacity = 1.0f; // 0-1
    bool invertOpacityMapValue = false;
    bool baseColorSingleChannelEnabled = false;
    bool specularAmountSingleChannelEnabled = false;
    bool emissiveSingleChannelEnabled = false;
    float bumpAmount = 0.0f; // 0-??
    float translucentFalloff = 0.0f; // 0 - ??
    float occlusionAmount = 1.0f; // 0 - 1
    float alphaCutoff = 0.5f; // 0 - 1
    float heightAmount = 0.0f; // 0 - 1
    int minHeightSamples = 8;
    int maxHeightSamples = 32;
    float clearcoatAmount = 0.0f; // 0 - 1
    float clearcoatRoughnessAmount = 0.0f; // 0 - 1
    float clearcoatNormalStrength = 1.0f; // 0 - 1
    float transmissionFactor = 0.0f; // 0 - 1
    float thicknessFactor = 0.0f; // 0 - 1
    float attenuationDistance = std::numeric_limits<float>::infinity();
    QVector3D attenuationColor { 1.0f, 1.0f, 1.0f };

    MaterialLighting lighting = MaterialLighting::FragmentLighting;
    QSSGRenderDefaultMaterial::MaterialBlendMode blendMode = QSSGRenderDefaultMaterial::MaterialBlendMode::SourceOver;
    QSSGRenderDefaultMaterial::MaterialSpecularModel specularModel = QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX;
    QSSGRenderDefaultMaterial::MaterialDiffuseModel diffuseModel = QSSGRenderDefaultMaterial::MaterialDiffuseModel::Burley;
    QSSGRenderDefaultMaterial::MaterialAlphaMode alphaMode = QSSGRenderDefaultMaterial::Default;
    QSSGCullFaceMode cullMode = QSSGCullFaceMode::Back;
    QSSGDepthDrawMode depthDrawMode = QSSGDepthDrawMode::OpaqueOnly;
    bool vertexColorsEnabled = false;
    bool vertexColorsMaskEnabled = false;
    bool dirty = true;
    TextureChannelMapping roughnessChannel = TextureChannelMapping::R;
    TextureChannelMapping opacityChannel = TextureChannelMapping::A;
    TextureChannelMapping translucencyChannel = TextureChannelMapping::A;
    TextureChannelMapping metalnessChannel = TextureChannelMapping::R;
    TextureChannelMapping occlusionChannel = TextureChannelMapping::R;
    TextureChannelMapping heightChannel = TextureChannelMapping::R;
    TextureChannelMapping clearcoatChannel = TextureChannelMapping::R;
    TextureChannelMapping clearcoatRoughnessChannel = TextureChannelMapping::G;
    TextureChannelMapping transmissionChannel = TextureChannelMapping::R;
    TextureChannelMapping thicknessChannel = TextureChannelMapping::G;
    TextureChannelMapping baseColorChannel = TextureChannelMapping::R;
    TextureChannelMapping specularAmountChannel = TextureChannelMapping::R;
    TextureChannelMapping emissiveChannel = TextureChannelMapping::R;
    float pointSize = 1.0f;
    float lineWidth = 1.0f;
    VertexColorMaskFlags vertexColorRedMask = VertexColorMask::NoMask;
    VertexColorMaskFlags vertexColorGreenMask = VertexColorMask::NoMask;
    VertexColorMaskFlags vertexColorBlueMask = VertexColorMask::NoMask;
    VertexColorMaskFlags vertexColorAlphaMask = VertexColorMask::NoMask;

    QSSGRenderDefaultMaterial(Type type = Type::DefaultMaterial);
    ~QSSGRenderDefaultMaterial();

    bool isSpecularEnabled() const { return specularAmount > .01f; }
    bool isMetalnessEnabled() const { return metalnessAmount > 0.01f; }
    bool isFresnelScaleBiasEnabled() const { return fresnelScaleBiasEnabled; }
    bool isClearcoatFresnelScaleBiasEnabled() const { return clearcoatFresnelScaleBiasEnabled; }
    bool isFresnelEnabled() const { return fresnelPower > 0.0f; }
    bool isVertexColorsEnabled() const { return vertexColorsEnabled; }
    bool isVertexColorsMaskEnabled() const { return vertexColorsMaskEnabled; }
    bool isInvertOpacityMapValue() const { return invertOpacityMapValue; }
    bool isBaseColorSingleChannelEnabled() const { return baseColorSingleChannelEnabled; }
    bool isSpecularAmountSingleChannelEnabled() const { return specularAmountSingleChannelEnabled; }
    bool isEmissiveSingleChannelEnabled() const { return emissiveSingleChannelEnabled; }
    bool hasLighting() const { return lighting != MaterialLighting::NoLighting; }
    bool isClearcoatEnabled() const { return clearcoatAmount > 0.01f; }
    bool isTransmissionEnabled() const { return transmissionFactor > 0.01f; }

    [[nodiscard]] inline bool isDirty() const { return dirty; }
    void clearDirty();

    QSSGShaderMaterialAdapter *adapter = nullptr;

    QString debugObjectName;
};

QT_END_NAMESPACE

#endif
