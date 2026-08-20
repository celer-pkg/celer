// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/* clang-format off */

#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include "qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadowmap_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>
#include <QtQuick3DRuntimeRender/private/qssgvertexpipelineimpl_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderpass_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderhelpers_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshaderresourcemergecontext_p.h>

#include <QtCore/QByteArray>

#include <cstdio>

QT_BEGIN_NAMESPACE

namespace  {
using Type = QSSGRenderableImage::Type;
template<Type> struct ImageStrings {};
#define DefineImageStrings(V) template<> struct ImageStrings<Type::V> \
{\
    static constexpr const char* sampler() { return "qt_"#V"Map_sampler"; }\
    static constexpr const char* offsets() { return "qt_"#V"Map_offsets"; }\
    static constexpr const char* rotations() { return "qt_"#V"Map_rotations"; }\
    static constexpr const char* fragCoords1() { return "qt_"#V"Map_uv_coords1"; }\
    static constexpr const char* fragCoords2() { return "qt_"#V"Map_uv_coords2"; }\
    static constexpr const char* samplerSize() { return "qt_"#V"Map_size"; }\
}

DefineImageStrings(Unknown);
DefineImageStrings(Diffuse);
DefineImageStrings(Opacity);
DefineImageStrings(Specular);
DefineImageStrings(Emissive);
DefineImageStrings(Bump);
DefineImageStrings(SpecularAmountMap);
DefineImageStrings(Normal);
DefineImageStrings(Translucency);
DefineImageStrings(Roughness);
DefineImageStrings(BaseColor);
DefineImageStrings(Metalness);
DefineImageStrings(Occlusion);
DefineImageStrings(Height);
DefineImageStrings(Clearcoat);
DefineImageStrings(ClearcoatRoughness);
DefineImageStrings(ClearcoatNormal);
DefineImageStrings(Transmission);
DefineImageStrings(Thickness);

struct ImageStringSet
{
    const char *imageSampler;
    const char *imageFragCoords;
    const char *imageFragCoordsTemp;
    const char *imageOffsets;
    const char *imageRotations;
};

#define DefineImageStringTableEntry(V) \
    { ImageStrings<Type::V>::sampler(), ImageStrings<Type::V>::fragCoords1(), ImageStrings<Type::V>::fragCoords2(), \
      ImageStrings<Type::V>::offsets(), ImageStrings<Type::V>::rotations() }

constexpr ImageStringSet imageStringTable[] {
    DefineImageStringTableEntry(Unknown),
    DefineImageStringTableEntry(Diffuse),
    DefineImageStringTableEntry(Opacity),
    DefineImageStringTableEntry(Specular),
    DefineImageStringTableEntry(Emissive),
    DefineImageStringTableEntry(Bump),
    DefineImageStringTableEntry(SpecularAmountMap),
    DefineImageStringTableEntry(Normal),
    DefineImageStringTableEntry(Translucency),
    DefineImageStringTableEntry(Roughness),
    DefineImageStringTableEntry(BaseColor),
    DefineImageStringTableEntry(Metalness),
    DefineImageStringTableEntry(Occlusion),
    DefineImageStringTableEntry(Height),
    DefineImageStringTableEntry(Clearcoat),
    DefineImageStringTableEntry(ClearcoatRoughness),
    DefineImageStringTableEntry(ClearcoatNormal),
    DefineImageStringTableEntry(Transmission),
    DefineImageStringTableEntry(Thickness)
};

const int TEXCOORD_VAR_LEN = 16;

void textureCoordVaryingName(char (&outString)[TEXCOORD_VAR_LEN], quint8 uvSet)
{
    // For now, uvSet will be less than 2.
    // But this value will be verified in the setProperty function.
    Q_ASSERT(uvSet < 9);
    qstrncpy(outString, "qt_varTexCoordX", TEXCOORD_VAR_LEN);
    outString[14] = '0' + uvSet;
}

void textureCoordVariableName(char (&outString)[TEXCOORD_VAR_LEN], quint8 uvSet)
{
    // For now, uvSet will be less than 2.
    // But this value will be verified in the setProperty function.
    Q_ASSERT(uvSet < 9);
    qstrncpy(outString, "qt_texCoordX", TEXCOORD_VAR_LEN);
    outString[11] = '0' + uvSet;
}

}

const char *QSSGMaterialShaderGenerator::getSamplerName(QSSGRenderableImage::Type type)
{
    return imageStringTable[int(type)].imageSampler;
}

static void addLocalVariable(QSSGStageGeneratorBase &inGenerator, const QByteArray &inName, const QByteArray &inType)
{
    inGenerator << "    " << inType << " " << inName << ";\n";
}

static QByteArray uvTransform(const QByteArray& imageRotations, const QByteArray& imageOffsets)
{
    QByteArray transform;
    transform = "    qt_uTransform = vec3(" + imageRotations + ".x, " + imageRotations + ".y, " + imageOffsets + ".x);\n";
    transform += "    qt_vTransform = vec3(" + imageRotations + ".z, " + imageRotations + ".w, " + imageOffsets + ".y);\n";
    return transform;
}

static void generateImageUVCoordinates(QSSGMaterialVertexPipeline &vertexShader,
                                       QSSGStageGeneratorBase &fragmentShader,
                                       const QSSGShaderDefaultMaterialKey &key,
                                       const ImageStringSet &names,
                                       bool forceFragmentShader = false,
                                       quint32 uvSet = 0,
                                       bool reuseImageCoords = false,
                                       bool useEnvironmentMapping = false)
{
    char textureCoordName[TEXCOORD_VAR_LEN];
    fragmentShader.addUniform(names.imageSampler, "sampler2D");
    if (!forceFragmentShader) {
        vertexShader.addUniform(names.imageOffsets, "vec3");
        vertexShader.addUniform(names.imageRotations, "vec4");
    } else {
        fragmentShader.addUniform(names.imageOffsets, "vec3");
        fragmentShader.addUniform(names.imageRotations, "vec4");
    }
    QByteArray uvTrans = uvTransform(names.imageRotations, names.imageOffsets);
    if (!useEnvironmentMapping) { // default to UV mapping
        if (!forceFragmentShader) {
            vertexShader << uvTrans;
            vertexShader.addOutgoing(names.imageFragCoords, "vec2");
            vertexShader.addFunction("getTransformedUVCoords");
        } else {
            fragmentShader << uvTrans;
            fragmentShader.addFunction("getTransformedUVCoords");
        }
        vertexShader.generateUVCoords(uvSet, key);
        if (!forceFragmentShader) {
            textureCoordVaryingName(textureCoordName, uvSet);
            vertexShader << "    vec2 " << names.imageFragCoordsTemp << " = qt_getTransformedUVCoords(vec3(" << textureCoordName << ", 1.0), qt_uTransform, qt_vTransform);\n";
            vertexShader.assignOutput(names.imageFragCoords, names.imageFragCoordsTemp);
        } else {
            textureCoordVariableName(textureCoordName, uvSet);
            if (reuseImageCoords)
                fragmentShader << "    ";
            else
                fragmentShader << "    vec2 ";
            fragmentShader << names.imageFragCoords << " = qt_getTransformedUVCoords(vec3(" << textureCoordName << ", 1.0), qt_uTransform, qt_vTransform);\n";
        }
    } else {
        fragmentShader.addUniform(names.imageOffsets, "vec3");
        fragmentShader.addUniform(names.imageRotations, "vec4");
        fragmentShader << uvTrans;
        vertexShader.generateEnvMapReflection(key);
        fragmentShader.addFunction("getTransformedUVCoords");
        if (reuseImageCoords)
            fragmentShader << "    ";
        else
            fragmentShader << "    vec2 ";
        fragmentShader << names.imageFragCoords << " = qt_getTransformedUVCoords(environment_map_reflection, qt_uTransform, qt_vTransform);\n";
    }
}

static void generateImageUVSampler(QSSGMaterialVertexPipeline &vertexGenerator,
                                   QSSGStageGeneratorBase &fragmentShader,
                                   const QSSGShaderDefaultMaterialKey &key,
                                   const ImageStringSet &names,
                                   char (&outString)[TEXCOORD_VAR_LEN],
                                   quint8 uvSet = 0)
{
    fragmentShader.addUniform(names.imageSampler, "sampler2D");
    // NOTE: Actually update the uniform name here
    textureCoordVariableName(outString, uvSet);
    vertexGenerator.generateUVCoords(uvSet, key);
}

static inline QSSGShaderMaterialAdapter *getMaterialAdapter(const QSSGRenderGraphObject &inMaterial)
{
    switch (inMaterial.type) {
    case QSSGRenderGraphObject::Type::DefaultMaterial:
    case QSSGRenderGraphObject::Type::PrincipledMaterial:
    case QSSGRenderGraphObject::Type::SpecularGlossyMaterial:
        return static_cast<const QSSGRenderDefaultMaterial &>(inMaterial).adapter;
    case QSSGRenderGraphObject::Type::CustomMaterial:
        return static_cast<const QSSGRenderCustomMaterial &>(inMaterial).adapter;
    default:
        break;
    }
    return nullptr;
}

// NOTE!!!: PLEASE ADD NEW VARS HERE!
static constexpr QByteArrayView qssg_shader_arg_names[] {
    { "DIFFUSE" },
    { "BASE_COLOR" },
    { "METALNESS" },
    { "ROUGHNESS" },
    { "EMISSIVE" },
    { "SPECULAR_AMOUNT" },
    { "EMISSIVE_COLOR" },
    { "LIGHT_COLOR" },
    { "LIGHT_ATTENUATION" },
    { "SPOT_FACTOR" },
    { "SHADOW_CONTRIB" },
    { "FRESNEL_CONTRIB" },
    { "TO_LIGHT_DIR" },
    { "NORMAL" },
    { "VIEW_VECTOR" },
    { "TOTAL_AMBIENT_COLOR" },
    { "COLOR_SUM" },
    { "BINORMAL" },
    { "TANGENT" },
    { "FRESNEL_POWER" },
    { "INSTANCE_MODEL_MATRIX" },
    { "INSTANCE_MODELVIEWPROJECTION_MATRIX" },
    { "UV0" },
    { "UV1" },
    { "VERTEX" },
    { "FRESNEL_SCALE" },
    { "FRESNEL_BIAS" },
    { "CLEARCOAT_FRESNEL_POWER" },
    { "CLEARCOAT_FRESNEL_SCALE" },
    { "CLEARCOAT_FRESNEL_BIAS" },
    { "CLEARCOAT_AMOUNT" },
    { "CLEARCOAT_NORMAL" },
    { "CLEARCOAT_ROUGHNESS" },
    { "IOR" },
    { "TRANSMISSION_FACTOR" },
    { "THICKNESS_FACTOR" },
    { "ATTENUATION_COLOR" },
    { "ATTENUATION_DISTANCE" },
    { "OCCLUSION_AMOUNT" },
};

const char *QSSGMaterialShaderGenerator::directionalLightProcessorArgumentList()
{
    return "inout vec3 DIFFUSE, in vec3 LIGHT_COLOR, in float SHADOW_CONTRIB, in vec3 TO_LIGHT_DIR, in vec3 NORMAL, in vec4 BASE_COLOR, in float METALNESS, in float ROUGHNESS, in vec3 VIEW_VECTOR";
}

const char *QSSGMaterialShaderGenerator::pointLightProcessorArgumentList()
{
    return "inout vec3 DIFFUSE, in vec3 LIGHT_COLOR, in float LIGHT_ATTENUATION, in float SHADOW_CONTRIB, in vec3 TO_LIGHT_DIR, in vec3 NORMAL, in vec4 BASE_COLOR, in float METALNESS, in float ROUGHNESS, in vec3 VIEW_VECTOR";
}

const char *QSSGMaterialShaderGenerator::spotLightProcessorArgumentList()
{
    return "inout vec3 DIFFUSE, in vec3 LIGHT_COLOR, in float LIGHT_ATTENUATION, float SPOT_FACTOR, in float SHADOW_CONTRIB, in vec3 TO_LIGHT_DIR, in vec3 NORMAL, in vec4 BASE_COLOR, in float METALNESS, in float ROUGHNESS, in vec3 VIEW_VECTOR";
}

const char *QSSGMaterialShaderGenerator::ambientLightProcessorArgumentList()
{
    return "inout vec3 DIFFUSE, in vec3 TOTAL_AMBIENT_COLOR, in vec3 NORMAL, in vec3 VIEW_VECTOR";
}

const char *QSSGMaterialShaderGenerator::specularLightProcessorArgumentList()
{
    return "inout vec3 SPECULAR, in vec3 LIGHT_COLOR, in float LIGHT_ATTENUATION, in float SHADOW_CONTRIB, in vec3 FRESNEL_CONTRIB, in vec3 TO_LIGHT_DIR, in vec3 NORMAL, in vec4 BASE_COLOR, in float METALNESS, in float ROUGHNESS, in float SPECULAR_AMOUNT, in vec3 VIEW_VECTOR";
}

const char *QSSGMaterialShaderGenerator::shadedFragmentMainArgumentList()
{
    return "inout vec4 BASE_COLOR, inout vec3 EMISSIVE_COLOR, inout float METALNESS, inout float ROUGHNESS, inout float SPECULAR_AMOUNT, inout float FRESNEL_POWER, inout vec3 NORMAL, inout vec3 TANGENT, inout vec3 BINORMAL, in vec2 UV0, in vec2 UV1, in vec3 VIEW_VECTOR, inout float IOR, inout float OCCLUSION_AMOUNT";
}

const char *QSSGMaterialShaderGenerator::postProcessorArgumentList()
{
    return "inout vec4 COLOR_SUM, in vec4 DIFFUSE, in vec3 SPECULAR, in vec3 EMISSIVE, in vec2 UV0, in vec2 UV1";
}

const char *QSSGMaterialShaderGenerator::iblProbeProcessorArgumentList()
{
    return "inout vec3 DIFFUSE, inout vec3 SPECULAR, in vec4 BASE_COLOR, in float AO_FACTOR, in float SPECULAR_AMOUNT, in float ROUGHNESS, in vec3 NORMAL, in vec3 VIEW_VECTOR, in mat3 IBL_ORIENTATION";
}

const char *QSSGMaterialShaderGenerator::vertexMainArgumentList()
{
    return "inout vec3 VERTEX, inout vec3 NORMAL, inout vec2 UV0, inout vec2 UV1, inout vec3 TANGENT, inout vec3 BINORMAL, inout ivec4 JOINTS, inout vec4 WEIGHTS, inout vec4 COLOR";
}

const char *QSSGMaterialShaderGenerator::vertexInstancedMainArgumentList()
{
    return "inout vec3 VERTEX, inout vec3 NORMAL, inout vec2 UV0, inout vec2 UV1, inout vec3 TANGENT, inout vec3 BINORMAL, inout ivec4 JOINTS, inout vec4 WEIGHTS, inout vec4 COLOR, inout mat4 INSTANCE_MODEL_MATRIX, inout mat4 INSTANCE_MODELVIEWPROJECTION_MATRIX";
}

#define MAX_MORPH_TARGET 8

static void generateFragmentDefines(QSSGStageGeneratorBase &fragmentShader,
                                    const QSSGShaderDefaultMaterialKey &inKey,
                                    const QSSGShaderDefaultMaterialKeyProperties &keyProps,
                                    QSSGShaderMaterialAdapter *materialAdapter,
                                    QSSGShaderLibraryManager &shaderLibraryManager,
                                    const QSSGUserShaderAugmentation &shaderAugmentation)
{
    if (materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment)) {
        auto hasCustomFunction = [&shaderLibraryManager, materialAdapter](const QByteArray &funcName) {
            return materialAdapter->hasCustomShaderFunction(QSSGShaderCache::ShaderType::Fragment, funcName, shaderLibraryManager);
        };

        if (hasCustomFunction(QByteArrayLiteral("qt_directionalLightProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_DIRECTIONAL_LIGHT_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_pointLightProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_POINT_LIGHT_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_spotLightProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_SPOT_LIGHT_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_specularLightProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_iblProbeProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_IBL_PROBE_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_ambientLightProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_AMBIENT_LIGHT_PROCESSOR", "1");
        if (hasCustomFunction(QByteArrayLiteral("qt_postProcessor")))
            fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_POST_PROCESSOR", "1");
    }

    if (materialAdapter->usesSharedVariables())
        fragmentShader.addDefinition("QSSG_CUSTOM_MATERIAL_SHARED_VARIABLES", "1");

    if (keyProps.m_hasShadows.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_SHADOWMAPPING", "1");
    if (keyProps.m_specularEnabled.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_SPECULAR", "1");
    if (keyProps.m_clearcoatEnabled.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_CLEARCOAT", "1");
    if (keyProps.m_transmissionEnabled.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_TRANSMISSION", "1");
    if (keyProps.m_metallicRoughnessEnabled.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_METALLIC_ROUGHNESS_WORKFLOW", "1");
    if (keyProps.m_specularGlossyEnabled.getValue(inKey))
        fragmentShader.addDefinition("QSSG_ENABLE_SPECULAR_GLOSSY_WORKFLOW", "1");
    switch (keyProps.m_diffuseModel.getDiffuseModel(inKey)) {
    case QSSGRenderDefaultMaterial::MaterialDiffuseModel::Burley:
        fragmentShader.addDefinition("QSSG_ENABLE_DIFFUSE_MODEL_BURLEY", "1");
        break;
    case QSSGRenderDefaultMaterial::MaterialDiffuseModel::Lambert:
        fragmentShader.addDefinition("QSSG_ENABLE_DIFFUSE_MODEL_LAMBERT", "1");
        break;
    case QSSGRenderDefaultMaterial::MaterialDiffuseModel::LambertWrap:
        fragmentShader.addDefinition("QSSG_ENABLE_DIFFUSE_MODEL_LAMBERT_WRAP", "1");
        break;
    }
    switch (keyProps.m_specularModel.getSpecularModel(inKey)) {
    case QSSGRenderDefaultMaterial::MaterialSpecularModel::BlinnPhong:
        fragmentShader.addDefinition("QSSG_ENABLE_SPECULAR_MODEL_BLINN_PHONG", "1");
        break;
    case QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX:
        fragmentShader.addDefinition("QSSG_ENABLE_SPECULAR_MODEL_SCHLICK_GGX", "1");
        break;
    }
    // Shadow softness
    switch (keyProps.m_shadowSoftness.getShadowSoftness(inKey)) {
    case QSSGRenderLight::SoftShadowQuality::Hard:
        fragmentShader.addDefinition("QSSG_SHADOW_SOFTNESS", "0");
        break;
    case QSSGRenderLight::SoftShadowQuality::PCF4:
        fragmentShader.addDefinition("QSSG_SHADOW_SOFTNESS", "4");
        break;
    case QSSGRenderLight::SoftShadowQuality::PCF8:
        fragmentShader.addDefinition("QSSG_SHADOW_SOFTNESS", "8");
        break;
    case QSSGRenderLight::SoftShadowQuality::PCF16:
    case QSSGRenderLight::SoftShadowQuality::PCF32:
    case QSSGRenderLight::SoftShadowQuality::PCF64:
        fragmentShader.addDefinition("QSSG_SHADOW_SOFTNESS", "16");
        break;
    ;}


    for (const auto &def : std::as_const(shaderAugmentation.defines))
        fragmentShader.addDefinition(def.name, def.value);
}

struct SamplerState {
    bool uvCoordinatesGenerated[QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ImageMapCount] = { false };
    bool m_isActive = false;
    bool m_uvCoordinateVariableDeclared = false;

    // Use shared texcoord when transforms are identity
    char imageFragCoords[TEXCOORD_VAR_LEN];
    const QSSGShaderDefaultMaterialKey &m_inKey;
    const QSSGShaderDefaultMaterialKeyProperties &m_keyProps;

    std::optional<QSSGShaderDefaultMaterialKeyProperties::ImageMapNames> fromType(QSSGRenderableImage::Type type) const
    {
        switch (type) {
        case QSSGRenderableImage::Type::Diffuse:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::DiffuseMap;
        case QSSGRenderableImage::Type::Opacity:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::OpacityMap;
        case QSSGRenderableImage::Type::Specular:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::SpecularMap;
        case QSSGRenderableImage::Type::Emissive:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::EmissiveMap;
        case QSSGRenderableImage::Type::Bump:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::BumpMap;
        case QSSGRenderableImage::Type::SpecularAmountMap:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::SpecularAmountMap;
        case QSSGRenderableImage::Type::Normal:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::NormalMap;
        case QSSGRenderableImage::Type::Translucency:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::TranslucencyMap;
        case QSSGRenderableImage::Type::Roughness:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::RoughnessMap;
        case QSSGRenderableImage::Type::BaseColor:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::BaseColorMap;
        case QSSGRenderableImage::Type::Metalness:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::MetalnessMap;
        case QSSGRenderableImage::Type::Occlusion:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::OcclusionMap;
        case QSSGRenderableImage::Type::Height:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::HeightMap;
        case QSSGRenderableImage::Type::Clearcoat:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ClearcoatMap;
        case QSSGRenderableImage::Type::ClearcoatRoughness:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ClearcoatRoughnessMap;
        case QSSGRenderableImage::Type::ClearcoatNormal:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ClearcoatNormalMap;
        case QSSGRenderableImage::Type::Transmission:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::TransmissionMap;
        case QSSGRenderableImage::Type::Thickness:
            return QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ThicknessMap;
        case QSSGRenderableImage::Type::Unknown:
            break;
        }
        return {};
    }

    SamplerState(const QSSGShaderDefaultMaterialKey &inKey, const QSSGShaderDefaultMaterialKeyProperties &keyProps)
        : m_inKey(inKey)
        , m_keyProps(keyProps)
    {
        for (int i = 0; i < QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ImageMapCount; ++i) {
            const QSSGShaderDefaultMaterialKeyProperties::ImageMapNames mapName = QSSGShaderDefaultMaterialKeyProperties::ImageMapNames(i);
            const QSSGShaderKeyImageMap &mapValue = m_keyProps.m_imageMaps[mapName];
            // just check if any are enabled
            if (mapValue.isEnabled(m_inKey))
                m_isActive = true;
        }
    }

    bool isActive() const { return m_isActive; }

    bool hasImage(QSSGRenderableImage::Type type) const
    {
        if (auto imageType = fromType(type))
            return m_keyProps.m_imageMaps[*imageType].isEnabled(m_inKey);

        return false;
    }

    bool uvGenerated(QSSGShaderDefaultMaterialKeyProperties::ImageMapNames imageType) const
    {
        if (imageType < 0 || imageType >= QSSGShaderDefaultMaterialKeyProperties::ImageMapNames::ImageMapCount)
            return false;
        return uvCoordinatesGenerated[size_t(imageType)];
    }

    void generateImageUVAndSampler(QSSGRenderableImage::Type imageType,
                                   QSSGMaterialVertexPipeline &vertexShader,
                                   QSSGStageGeneratorBase &fragmentShader,
                                   const QSSGShaderDefaultMaterialKey &key,
                                   bool forceFragmentShader = false)
    {
        if (auto mapType = fromType(imageType)) {
            if (uvGenerated(*mapType))
                return; // don't do it a second time

            const QSSGShaderKeyImageMap &mapValue = m_keyProps.m_imageMaps[size_t(*mapType)];
            const quint8 indexUV = mapValue.isUsingUV1(m_inKey) ? 1 : 0;
            const auto &samplerNames = imageStringTable[int(imageType)]; // using the QSSGRenderableImage::Type
            if (mapValue.isIdentityTransform(m_inKey)) {
                generateImageUVSampler(vertexShader, fragmentShader, key, samplerNames, imageFragCoords, indexUV);
            } else {
                if (!m_uvCoordinateVariableDeclared) {
                    // The frist time we need to declare the shared variable
                    fragmentShader.append("    vec3 qt_uTransform;");
                    fragmentShader.append("    vec3 qt_vTransform;");
                    m_uvCoordinateVariableDeclared = true;
                }
                generateImageUVCoordinates(vertexShader, fragmentShader, key, samplerNames, forceFragmentShader, indexUV, false, mapValue.isEnvMap(m_inKey) || mapValue.isLightProbe(m_inKey));
            }
            uvCoordinatesGenerated[int(*mapType)] = true;
        }
    }

    const char *samplerName(QSSGRenderableImage::Type imageType) const
    {
        if (imageType <= QSSGRenderableImage::Type::Unknown)
            return "";
        return imageStringTable[int(imageType)].imageSampler;
    }

    const char *fragCoordsName(QSSGRenderableImage::Type imageType) const
    {
        if (auto mapType = fromType(imageType)) {
            if (!uvGenerated(*mapType))
                qWarning("Requesting image frag coords for image type %d that has not been generated", int(imageType));

            const QSSGShaderKeyImageMap &mapValue = m_keyProps.m_imageMaps[size_t(*mapType)];
            if (mapValue.isIdentityTransform(m_inKey))
                return imageFragCoords;

            return imageStringTable[int(imageType)].imageFragCoords;
        }

        return "";
    }

};

struct PassRequirmentsState {
    enum PassType {
        None = 0,
        Color,
        Depth,
        OrthoShadow,
        PerspectiveShadow,
        Normal,
        Debug,
        User
    };
    PassType passType = None;

    // Requirments for Pass
    bool needsBaseColor = false;     // qt_diffuseColor
    bool needsRoughness = false;      // qt_roughnessAmount
    bool needsMetalness = false;     // qt_metalnessAmount
    bool needsDiffuseLight = false;  // global_diffuse_light
    bool needsSpecularLight = false; // global_specular_light
    bool needsEmission = false;      // global_emission
    bool needsWorldNormal = false;   // qt_world_normal
    bool needsWorldTangent = false;  // qt_tangent
    bool needsWorldBinormal = false; // qt_binormal

    bool needsF0 = false;            // qt_f0
    bool needsF90 = false;           // qt_f90
    bool needsAmbientOcclusion = false; // qt_ao_factor


    // Available Information
    bool hasVertexColors = false;
    bool hasLighting = false;
    bool hasPunctualLights = false;
    bool hasSpecularLight = false;
    bool hasIblProbe = false;
    bool hasReflectionProbe = false;
    bool hasIblOrientation = false;
    bool hasShadowMap = false;
    bool hasSSAOMap = false;
    bool hasLightMap = false;
    bool hasBumpNormalMap = false;
    bool hasParallaxMapping = false;
    bool hasClearcoat = false;
    bool hasTransmission = false;
    bool hasFresnelScaleBias = false;
    bool hasClearcoatFresnelScaleBias = false;
    bool hasFog = false;


    // Material Properties
    bool isDoubleSided = false;
    bool isSpecularAAEnabled = false;
    bool isMetallicRoughnessWorkflow = false;
    bool isSpecularGlossinessWorkflow = false;
    bool isPbrMaterial = false;
    bool isOpaqueDepthPrePass = false;
    bool isUserPass = false;
    quint32 numMorphTargets = 0;
    int viewCount = 1;
    QSSGRenderLayer::OITMethod oitMethod = QSSGRenderLayer::OITMethod::None;
    bool oitMSAA = false;
    QSSGRenderLayer::MaterialDebugMode debugMode = QSSGRenderLayer::MaterialDebugMode::None;

    PassRequirmentsState(const QSSGShaderDefaultMaterialKey &inKey,
                         const QSSGShaderDefaultMaterialKeyProperties &keyProps,
                         const QSSGShaderFeatures &featureSet,
                         const SamplerState &samplerState,
                         const QSSGUserShaderAugmentation &shaderAugmentation)
    {
        const bool isDepthPass = featureSet.isSet(QSSGShaderFeatures::Feature::DepthPass);
        const bool isOrthoShadowPass = featureSet.isSet(QSSGShaderFeatures::Feature::OrthoShadowPass);
        const bool isPerspectiveShadowPass = featureSet.isSet(QSSGShaderFeatures::Feature::PerspectiveShadowPass);
        isOpaqueDepthPrePass = featureSet.isSet(QSSGShaderFeatures::Feature::OpaqueDepthPrePass);
        const bool isNormalPass = featureSet.isSet(QSSGShaderFeatures::Feature::NormalPass);

        hasVertexColors = keyProps.m_vertexColorsEnabled.getValue(inKey)
                       || keyProps.m_usesVarColor.getValue(inKey)
                       || keyProps.m_vertexColorsMaskEnabled.getValue(inKey)
                       || keyProps.m_usesInstancing.getValue(inKey)
                       || keyProps.m_blendParticles.getValue(inKey);
        hasLighting = keyProps.m_hasLighting.getValue(inKey);
        hasPunctualLights = keyProps.m_hasPunctualLights.getValue(inKey);
        isDoubleSided = keyProps.m_isDoubleSided.getValue(inKey);
        hasIblProbe = keyProps.m_hasIbl.getValue(inKey);
        hasReflectionProbe = featureSet.isSet(QSSGShaderFeatures::Feature::ReflectionProbe);
        oitMethod = static_cast<QSSGRenderLayer::OITMethod>(keyProps.m_orderIndependentTransparency.getValue(inKey));
        oitMSAA = keyProps.m_oitMSAA.getValue(inKey);
        isSpecularAAEnabled = keyProps.m_specularAAEnabled.getValue(inKey);

        // TODO: Not sure I agree with the following, but this is the current behavior
        hasSpecularLight |= keyProps.m_specularEnabled.getValue(inKey);
        hasSpecularLight |= hasIblProbe;
        hasSpecularLight |= hasReflectionProbe;
        hasSpecularLight |= samplerState.hasImage(QSSGRenderableImage::Type::SpecularAmountMap);

        hasIblOrientation = featureSet.isSet(QSSGShaderFeatures::Feature::IblOrientation);
        hasShadowMap = featureSet.isSet(QSSGShaderFeatures::Feature::Ssm);
        hasSSAOMap = featureSet.isSet(QSSGShaderFeatures::Feature::Ssao);
        hasLightMap = featureSet.isSet(QSSGShaderFeatures::Feature::Lightmap);
        hasBumpNormalMap = samplerState.hasImage(QSSGRenderableImage::Type::Normal) || samplerState.hasImage(QSSGRenderableImage::Type::Bump);
        hasParallaxMapping = samplerState.hasImage(QSSGRenderableImage::Type::Height);
        hasClearcoat = keyProps.m_clearcoatEnabled.getValue(inKey);
        hasTransmission = keyProps.m_transmissionEnabled.getValue(inKey);
        hasFresnelScaleBias = keyProps.m_fresnelScaleBiasEnabled.getValue(inKey);
        hasClearcoatFresnelScaleBias = keyProps.m_clearcoatFresnelScaleBiasEnabled.getValue(inKey);
        isMetallicRoughnessWorkflow = keyProps.m_metallicRoughnessEnabled.getValue(inKey);
        isSpecularGlossinessWorkflow = keyProps.m_specularGlossyEnabled.getValue(inKey);
        isPbrMaterial = isMetallicRoughnessWorkflow || isSpecularGlossinessWorkflow;
        isUserPass = featureSet.isSet(QSSGShaderFeatures::Feature::UserRenderPass);
        hasFog = keyProps.m_fogEnabled.getValue(inKey);
        numMorphTargets = keyProps.m_targetCount.getValue(inKey);
        viewCount = featureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView) ? 1 : keyProps.m_viewCount.getValue(inKey);

        if (isDepthPass) {
            passType = Depth;
            if (isOpaqueDepthPrePass)
                needsBaseColor = true;
        } else if (isOrthoShadowPass) {
            passType = OrthoShadow;
            if (isOpaqueDepthPrePass)
                needsBaseColor = true;
        } else if (isPerspectiveShadowPass) {
            passType = PerspectiveShadow;
            if (isOpaqueDepthPrePass)
                needsBaseColor = true;
        } else if (isNormalPass) {
            passType = Normal;
            needsWorldNormal = true;
            needsWorldTangent = true;
            needsWorldBinormal = true;
            needsRoughness = true;
        } else if (isUserPass) {
            passType = User;
            // Use shaderAugmentation to figure out what features are needed
            needsBaseColor = shaderAugmentation.needsBaseColor;
            needsRoughness = shaderAugmentation.needsRoughness;
            needsMetalness = shaderAugmentation.needsMetalness;
            needsEmission = shaderAugmentation.needsEmissiveLight;
            needsWorldNormal = shaderAugmentation.needsWorldNormal;
            // WORLD_NORMAL for a normal-mapped material requires tangent space to be set up.
            // Ensure tangent/binormal are generated whenever the world normal is requested.
            needsWorldTangent = shaderAugmentation.needsWorldTangent || shaderAugmentation.needsWorldNormal;
            needsWorldBinormal = shaderAugmentation.needsWorldBinormal || shaderAugmentation.needsWorldNormal;

            if (shaderAugmentation.needsDiffuseLight ||
                shaderAugmentation.needsSpecularLight ||
                shaderAugmentation.needsF0 ||
                shaderAugmentation.needsF90) {
                // Turn everything on for now
                needsBaseColor = true;
                needsRoughness = true;
                needsMetalness = true;
                needsDiffuseLight = true;
                needsSpecularLight = true;
                needsEmission = true;
                needsWorldNormal = true;
                needsWorldTangent = true;
                needsWorldBinormal = true;
                needsF0 = true;
                needsF90 = true;
                needsAmbientOcclusion = true;
            }

        } else {
            // Either a Color or Debug Pass
            passType = Color;
            debugMode = QSSGRenderLayer::MaterialDebugMode(keyProps.m_debugMode.getValue(inKey));
            if (debugMode == QSSGRenderLayer::MaterialDebugMode::None) {
                needsBaseColor = true;
                needsRoughness = true;
                needsMetalness = true;
                needsDiffuseLight = true;
                needsSpecularLight = true;
                needsEmission = true;
                needsWorldNormal = true;
                needsWorldTangent = true;
                needsWorldBinormal = true;
                needsF0 = true;
                needsF90 = true;
                needsAmbientOcclusion = true;
            } else {
                passType = Debug;
                switch (debugMode) {
                case QSSGRenderLayer::MaterialDebugMode::None:
                    break;
                case QSSGRenderLayer::MaterialDebugMode::BaseColor:
                    needsBaseColor = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Roughness:
                    needsRoughness = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Metalness:
                    needsMetalness = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Diffuse:
                    needsBaseColor = true;
                    needsRoughness = true;
                    needsMetalness = true;
                    needsDiffuseLight = true;
                    needsEmission = true;
                    needsWorldNormal = true;
                    needsWorldTangent = true;
                    needsWorldBinormal = true;
                    needsF0 = true;
                    needsF90 = true;
                    needsAmbientOcclusion = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Specular:
                    needsBaseColor = true;
                    needsRoughness = true;
                    needsMetalness = true;
                    needsSpecularLight = true;
                    needsEmission = true;
                    needsWorldNormal = true;
                    needsWorldTangent = true;
                    needsWorldBinormal = true;
                    needsF0 = true;
                    needsF90 = true;
                    needsAmbientOcclusion = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::ShadowOcclusion:
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Emission:
                    needsEmission = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::AmbientOcclusion:
                    needsAmbientOcclusion = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Normal:
                    needsWorldNormal = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Tangent:
                    needsWorldTangent = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::Binormal:
                    needsWorldBinormal = true;
                    break;
                case QSSGRenderLayer::MaterialDebugMode::F0:
                    needsBaseColor = true;
                    needsRoughness = true;
                    needsMetalness = true;
                    needsSpecularLight = true;
                    needsEmission = true;
                    needsWorldNormal = true;
                    needsWorldTangent = true;
                    needsWorldBinormal = true;
                    needsF0 = true;
                    needsF90 = true;
                    needsAmbientOcclusion = true;
                    break;
                }
            }
        }

        // Parallax mapping requires world tangent/binormal in all pass types,
        // since qt_tangent and qt_binormal are passed to qt_parallaxMapping().
        if (hasParallaxMapping) {
            needsWorldNormal = true;
            needsWorldTangent = true;
            needsWorldBinormal = true;
        }

        // Normal/bump map sampling uses the TBN matrix (qt_tangent, qt_binormal)
        // to transform the sampled normal to world space. Ensure tangent and binormal
        // are generated whenever a normal map is present and the world normal is needed.
        if (needsWorldNormal && hasBumpNormalMap) {
            needsWorldTangent = true;
            needsWorldBinormal = true;
        }
    }

    bool shouldIncludeCustomFragmentMain() const {
        // In debug passes, the custom fragment main must be called so that
        // material-derived values (e.g. NORMAL modified by normal-map sampling code in
        // MAIN()) are applied before the debug output is generated.
        if (passType == Debug)
            return true;
        return needsBaseColor || needsRoughness || needsMetalness || needsDiffuseLight || needsSpecularLight || needsEmission;
    }

    bool shouldDiscardNonOpaque() const {
        return (passType == OrthoShadow || passType == PerspectiveShadow || passType == Depth) && isOpaqueDepthPrePass;
    }

};


static void generateFragmentShader(QSSGStageGeneratorBase &fragmentShader,
                                   QSSGMaterialVertexPipeline &vertexShader,
                                   const QSSGShaderDefaultMaterialKey &inKey,
                                   const QSSGShaderDefaultMaterialKeyProperties &keyProps,
                                   const QSSGShaderFeatures &featureSet,
                                   const QSSGRenderGraphObject &inMaterial,
                                   const QSSGUserShaderAugmentation &shaderAugmentation,
                                   QSSGShaderLibraryManager &shaderLibraryManager)
{
    QSSGShaderMaterialAdapter *materialAdapter = getMaterialAdapter(inMaterial);
    auto hasCustomFunction = [&shaderLibraryManager, materialAdapter](const QByteArray &funcName) {
        return materialAdapter->hasCustomShaderFunction(QSSGShaderCache::ShaderType::Fragment,
                                                 funcName,
                                                 shaderLibraryManager);
    };

    auto channelStr = [](const QSSGShaderKeyTextureChannel &chProp, const QSSGShaderDefaultMaterialKey &inKey) -> QByteArray {
        QByteArray ret;
        switch (chProp.getTextureChannel(inKey)) {
        case QSSGShaderKeyTextureChannel::R:
            ret.append(".r");
            break;
        case QSSGShaderKeyTextureChannel::G:
            ret.append(".g");
            break;
        case QSSGShaderKeyTextureChannel::B:
            ret.append(".b");
            break;
        case QSSGShaderKeyTextureChannel::A:
            ret.append(".a");
            break;
        }
        return ret;
    };

    auto maskVariableByVertexColorChannel = [&fragmentShader, keyProps, inKey]( const QByteArray &maskVariable, const QSSGRenderDefaultMaterial::VertexColorMask &maskEnum ){
        if (keyProps.m_vertexColorsMaskEnabled.getValue(inKey)) {
            if ( keyProps.m_vertexColorRedMask.getValue(inKey) & maskEnum )
                fragmentShader << "    " << maskVariable << " *= qt_vertColorMask.r;\n";
            else if ( keyProps.m_vertexColorGreenMask.getValue(inKey) & maskEnum )
                fragmentShader << "    " << maskVariable << " *= qt_vertColorMask.g;\n";
            else if ( keyProps.m_vertexColorBlueMask.getValue(inKey) & maskEnum )
                fragmentShader << "    " << maskVariable << " *= qt_vertColorMask.b;\n";
            else if ( keyProps.m_vertexColorAlphaMask.getValue(inKey) & maskEnum )
                fragmentShader << "    " << maskVariable << " *= qt_vertColorMask.a;\n";
        }
    };

    generateFragmentDefines(fragmentShader, inKey, keyProps, materialAdapter, shaderLibraryManager, shaderAugmentation);

    // Determine the available texture channels
    SamplerState samplerState(inKey, keyProps);
    // Determine the requirements of this rendering pass
    const PassRequirmentsState passRequirmentState(inKey, keyProps, featureSet, samplerState, shaderAugmentation);

    const bool hasCustomVert = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Vertex);

    const int viewCount = featureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView)
        ? 1 : keyProps.m_viewCount.getValue(inKey);

    // Morphing
    if (passRequirmentState.numMorphTargets > 0 || hasCustomVert) {
        vertexShader.addDefinition(QByteArrayLiteral("QT_MORPH_MAX_COUNT"),
                    QByteArray::number(passRequirmentState.numMorphTargets));
        quint8 offset;
        if ((offset = keyProps.m_targetPositionOffset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_POSITION_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetNormalOffset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_NORMAL_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetTangentOffset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_TANGENT_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetBinormalOffset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_BINORMAL_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetTexCoord0Offset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_TEX0_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetTexCoord1Offset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_TEX1_OFFSET"),
                                       QByteArray::number(offset));
        }
        if ((offset = keyProps.m_targetColorOffset.getValue(inKey)) < UINT8_MAX) {
            vertexShader.addDefinition(QByteArrayLiteral("QT_TARGET_COLOR_OFFSET"),
                                       QByteArray::number(offset));
        }
    }

    if (passRequirmentState.passType == PassRequirmentsState::User) {
        if (shaderAugmentation.hasUserAugmentation())
            fragmentShader << shaderAugmentation.preamble;
    }

    for (const auto &u : shaderAugmentation.propertyUniforms)
        fragmentShader.addUniform(u.name, u.typeName);

    // Unshaded custom materials need no code in main (apart from calling qt_customMain)
    const bool hasCustomFrag = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment);
    const bool usesSharedVar = materialAdapter->usesSharedVariables();

    vertexShader.beginFragmentGeneration(shaderLibraryManager, passRequirmentState.oitMethod);

    if (passRequirmentState.passType == PassRequirmentsState::OrthoShadow) {
        vertexShader.generateDepth();
        fragmentShader.addUniform("qt_shadowDepthAdjust", "vec2");
    }


    if (passRequirmentState.passType == PassRequirmentsState::PerspectiveShadow)
        vertexShader.generateShadowWorldPosition(inKey);


    if (hasCustomFrag && materialAdapter->isUnshaded()) {
        // Unlike the depth texture pass, the normal texture pass needs to
        // output valid fragment values. The custom main is skipped in
        // beginVertexGeneration if isNormalPass is true, but something must be
        // written to gl_FragColor (the normals), so pretend we are shaded...
        // if (passRequirmentState.passType != PassRequirmentsState::Normal)
            return;
    }

    // hasCustomFrag == Shaded custom material from this point on, for Unshaded we returned above

    // The fragment or vertex shaders may not use the material_properties or diffuse
    // uniforms in all cases but it is simpler to just add them and let the linker strip them.
    // if (passRequirmentState.needsEmission)
    fragmentShader.addUniform("qt_material_emissive_color", "vec3");
    // if (passRequirmentState.needsBaseColor)
    fragmentShader.addUniform("qt_material_base_color", "vec4");
    fragmentShader.addUniform("qt_material_properties", "vec4");
    fragmentShader.addUniform("qt_material_properties2", "vec4");
    fragmentShader.addUniform("qt_material_properties3", "vec4");
    if (passRequirmentState.hasParallaxMapping || passRequirmentState.hasTransmission)
        fragmentShader.addUniform("qt_material_properties4", "vec4");
    if (!hasCustomFrag) {
        if (passRequirmentState.hasTransmission) {
            fragmentShader.addUniform("qt_material_attenuation", "vec4");
            fragmentShader.addUniform("qt_material_thickness", "float");
        }
        if (passRequirmentState.hasFresnelScaleBias || passRequirmentState.hasClearcoatFresnelScaleBias)
            fragmentShader.addUniform("qt_material_properties5", "vec4");
        fragmentShader.addUniform("qt_material_clearcoat_normal_strength", "float");
        fragmentShader.addUniform("qt_material_clearcoat_fresnel_power", "float");
    }

    if (passRequirmentState.hasVertexColors) {
        vertexShader.generateVertexColor(inKey);
    }else {
        fragmentShader.append("    vec4 qt_vertColorMask = vec4(1.0);");
        fragmentShader.append("    vec4 qt_vertColor = vec4(1.0);");
    }

    if (passRequirmentState.needsWorldNormal || passRequirmentState.needsWorldTangent || passRequirmentState.needsWorldBinormal || hasCustomFrag) {
        vertexShader.generateViewVector(inKey);
        if (keyProps.m_usesProjectionMatrix.getValue(inKey)) {
            if (viewCount >= 2)
                fragmentShader.addUniformArray("qt_projectionMatrix", "mat4", viewCount);
            else
                fragmentShader.addUniform("qt_projectionMatrix", "mat4");
        }
        if (keyProps.m_usesInverseProjectionMatrix.getValue(inKey)) {
            if (viewCount >= 2)
                fragmentShader.addUniformArray("qt_inverseProjectionMatrix", "mat4", viewCount);
            else
                fragmentShader.addUniform("qt_inverseProjectionMatrix", "mat4");
        }
        vertexShader.generateWorldNormal(inKey);
        vertexShader.generateWorldPosition(inKey);

        if (passRequirmentState.needsWorldTangent || passRequirmentState.needsWorldBinormal || hasCustomFrag) {
            bool genTangent = false;
            bool genBinormal = false;
            vertexShader.generateVarTangentAndBinormal(inKey, genTangent, genBinormal);

            if (!genTangent) {
                QSSGRenderableImage::Type id = QSSGRenderableImage::Type::Unknown;
                if (passRequirmentState.hasBumpNormalMap) {
                    // Generate imageCoords for bump/normal map first.
                    // Some operations needs to use the TBN transform and if the
                    // tangent vector is not provided, it is necessary.
                    id = samplerState.hasImage(QSSGRenderableImage::Type::Bump) ? QSSGRenderableImage::Type::Bump : QSSGRenderableImage::Type::Normal;
                } else if (samplerState.hasImage(QSSGRenderableImage::Type::ClearcoatNormal)) {
                    // For the corner case that there is only a clearcoat normal map
                    id = QSSGRenderableImage::Type::ClearcoatNormal;
                } else if (passRequirmentState.hasParallaxMapping) {
                    // For the corner case that there is only a height map (parallax mapping),
                    // use its UV coordinates to derive tangents analytically.
                    id = QSSGRenderableImage::Type::Height;
                }

                if (id > QSSGRenderableImage::Type::Unknown) {
                    samplerState.generateImageUVAndSampler(id, vertexShader, fragmentShader, inKey, true);
                    fragmentShader << "    vec2 dUVdx = dFdx(" << samplerState.fragCoordsName(id) << ");\n"
                                   << "    vec2 dUVdy = dFdy(" << samplerState.fragCoordsName(id) << ");\n";
                    fragmentShader << "    qt_tangent = (dUVdy.y * dFdx(qt_varWorldPos) - dUVdx.y * dFdy(qt_varWorldPos)) / (dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x);\n"
                                   << "    qt_tangent = qt_tangent - dot(qt_world_normal, qt_tangent) * qt_world_normal;\n"
                                   << "    qt_tangent = normalize(qt_tangent);\n";
                }
            }
            if (!genBinormal)
                fragmentShader << "    qt_binormal = cross(qt_world_normal, qt_tangent);\n";
        }

        if (passRequirmentState.isDoubleSided) {
            fragmentShader.append("#if QSHADER_HLSL && QSHADER_VIEW_COUNT >= 2");
            fragmentShader.append("    const float qt_facing = 1.0;");
            fragmentShader.append("#else");
            fragmentShader.append("    const float qt_facing = gl_FrontFacing ? 1.0 : -1.0;");
            fragmentShader.append("#endif");
            fragmentShader.append("    qt_world_normal *= qt_facing;\n");
            if (passRequirmentState.needsWorldTangent || passRequirmentState.needsWorldBinormal || hasCustomFrag) {
                fragmentShader.append("    qt_tangent *= qt_facing;");
                fragmentShader.append("    qt_binormal *= qt_facing;");
            }
        }
    }

    if (hasCustomFrag) {
        // A custom shaded material is effectively a principled material for
        // our purposes here. The defaults are different from a
        // PrincipledMaterial however, since this is more sensible here.
        // (because the shader has to state it to get things)
        // These should match the defaults of PrincipledMaterial.
        fragmentShader << "    float qt_customOcclusionAmount = 1.0;\n";
        fragmentShader << "    float qt_customIOR = 1.5;\n";
        fragmentShader << "    float qt_customSpecularAmount = 0.5;\n"; // overrides qt_material_properties.x
        fragmentShader << "    float qt_customSpecularRoughness = 0.0;\n"; // overrides qt_material_properties.y
        fragmentShader << "    float qt_customMetalnessAmount = 0.0;\n"; // overrides qt_material_properties.z
        fragmentShader << "    float qt_customFresnelPower = 5.0;\n"; // overrides qt_material_properties2.x
        fragmentShader << "    vec4 qt_customBaseColor = vec4(1.0);\n"; // overrides qt_material_base_color
        fragmentShader << "    vec3 qt_customEmissiveColor = vec3(0.0);\n"; // overrides qt_material_emissive_color
        if (passRequirmentState.hasClearcoat) {
            fragmentShader << "    float qt_customClearcoatAmount = 0.0;\n";
            fragmentShader << "    float qt_customClearcoatFresnelPower = 5.0;\n";
            fragmentShader << "    float qt_customClearcoatRoughness = 0.0;\n";
            fragmentShader << "    vec3 qt_customClearcoatNormal = qt_world_normal;\n";
            if (passRequirmentState.hasClearcoatFresnelScaleBias) {
                fragmentShader << "    float qt_customClearcoatFresnelScale = 1.0;\n";
                fragmentShader << "    float qt_customClearcoatFresnelBias = 0.0;\n";
            }
        }
        if (passRequirmentState.hasFresnelScaleBias) {
            fragmentShader << "    float qt_customFresnelScale = 1.0;\n";
            fragmentShader << "    float qt_customFresnelBias = 0.0;\n";
        }

        if (passRequirmentState.hasTransmission) {
            fragmentShader << "    float qt_customTransmissionFactor = 0.0;\n";
            fragmentShader << "    float qt_customThicknessFactor = 0.0;\n";
            fragmentShader << "    vec3 qt_customAttenuationColor = vec3(1.0);\n";
            fragmentShader << "    float qt_customAttenuationDistance = 0.0;\n";
        }
        if (usesSharedVar)
            fragmentShader << "    QT_SHARED_VARS qt_customShared;\n";
        // Generate the varyings for UV0 and UV1 since customer materials don't use image
        // properties directly.
        vertexShader.generateUVCoords(0, inKey);
        vertexShader.generateUVCoords(1, inKey);
        if (passRequirmentState.shouldIncludeCustomFragmentMain() && hasCustomFunction(QByteArrayLiteral("qt_customMain"))) {
            fragmentShader << "    qt_customMain(qt_customBaseColor,\n"
                           << "                  qt_customEmissiveColor,\n"
                           << "                  qt_customMetalnessAmount,\n"
                           << "                  qt_customSpecularRoughness,\n"
                           << "                  qt_customSpecularAmount,\n"
                           << "                  qt_customFresnelPower,\n"
                           << "                  qt_world_normal,\n"
                           << "                  qt_tangent,\n"
                           << "                  qt_binormal,\n"
                           << "                  qt_texCoord0,\n"
                           << "                  qt_texCoord1,\n"
                           << "                  qt_view_vector,\n"
                           << "                  qt_customIOR,\n"
                           << "                  qt_customOcclusionAmount";
            if (passRequirmentState.hasClearcoat) {
                fragmentShader << ",\n                  qt_customClearcoatAmount,\n"
                               << "                  qt_customClearcoatFresnelPower,\n"
                               << "                  qt_customClearcoatRoughness,\n"
                               << "                  qt_customClearcoatNormal";
                if (passRequirmentState.hasClearcoatFresnelScaleBias) {
                    fragmentShader << ",\n                  qt_customClearcoatFresnelScale,\n"
                                   << "                  qt_customClearcoatFresnelBias";
                }
            }
            if (passRequirmentState.hasFresnelScaleBias) {
                fragmentShader << ",\n                  qt_customFresnelScale,\n"
                               << "                  qt_customFresnelBias";
            }
            if (passRequirmentState.hasTransmission) {
                fragmentShader << ",\n                  qt_customTransmissionFactor,\n"
                               << "                  qt_customThicknessFactor,\n"
                               << "                  qt_customAttenuationColor,\n"
                               << "                  qt_customAttenuationDistance";
            }
            if (usesSharedVar)
                fragmentShader << "\n,                  qt_customShared);\n";
            else
                fragmentShader << ");\n";
        }
        fragmentShader << "    vec4 qt_diffuseColor = qt_customBaseColor * qt_vertColor;\n";
        fragmentShader << "    vec3 qt_global_emission = qt_customEmissiveColor;\n";
        fragmentShader << "    float qt_iOR = qt_customIOR;\n";
    } else {
        fragmentShader << "    vec4 qt_diffuseColor = qt_material_base_color * qt_vertColor;\n";
        fragmentShader << "    vec3 qt_global_emission = qt_material_emissive_color;\n";
        if (passRequirmentState.hasSpecularLight || samplerState.isActive())
            fragmentShader << "    float qt_iOR = qt_material_specular.w;\n";
    }

    const bool hasCustomIblProbe = hasCustomFrag && hasCustomFunction(QByteArrayLiteral("qt_iblProbeProcessor"));


    // Lightmaps
    // hasLighting && needsDiffuseLight || needsSpecularLight
    if (passRequirmentState.hasLighting && (passRequirmentState.needsDiffuseLight || passRequirmentState.needsSpecularLight)) {
        if (passRequirmentState.hasLightMap) {
            vertexShader.generateLightmapUVCoords(inKey);
            fragmentShader.addFunction("lightmap");
        }
    }

    // Heightmap / Parallax Mapping
    // Possible Optimization: Also conditionally, only when we plan on using another map/texture since it only gets used when sampling
    if (passRequirmentState.hasParallaxMapping) {
        // Adjust UV coordinates to account for parallaxMapping before
        // reading any other texture.
        fragmentShader.addInclude("parallaxMapping.glsllib");
        samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Height, vertexShader, fragmentShader, inKey, true);
        fragmentShader << "    float qt_heightAmount = qt_material_properties4.x;\n";
        maskVariableByVertexColorChannel( "qt_heightAmount", QSSGRenderDefaultMaterial::HeightAmountMask );
        fragmentShader << "    qt_texCoord0 = qt_parallaxMapping(" << samplerState.fragCoordsName(QSSGRenderableImage::Type::Height) << ",\n"
                       << "                                      " << samplerState.samplerName(QSSGRenderableImage::Type::Height) << ",\n"
                       << "                                       qt_tangent,\n"
                       << "                                       qt_binormal,\n"
                       << "                                       qt_world_normal,\n"
                       << "                                       qt_varWorldPos, \n"
                       << "#if QSHADER_VIEW_COUNT >= 2\n"
                       << "                                       qt_cameraPosition[qt_viewIndex],\n"
                       << "#else\n"
                       << "                                       qt_cameraPosition,\n"
                       << "#endif\n"
                       << "                                       qt_heightAmount,\n"
                       << "                                       qt_material_properties4.y,\n"
                       << "                                       qt_material_properties4.z);\n";
    }

    // Clearcoat (before normal/bump has a chance to overwrite qt_world_normal)
    if (passRequirmentState.hasClearcoat && (passRequirmentState.needsDiffuseLight || passRequirmentState.needsSpecularLight)) {
        addLocalVariable(fragmentShader, "qt_clearcoatNormal", "vec3");
        // Clearcoat normal should be calculated not considering the normalImage for the base material
        // If both are to be the same then just set the same normalImage for the base and clearcoat
        // This does mean that this value should be calculated before qt_world_normal is overwritten by
        // the normalMap.
        if (hasCustomFrag) {
            fragmentShader << "    qt_clearcoatNormal = qt_customClearcoatNormal;\n";
        } else {
            if (samplerState.hasImage(QSSGRenderableImage::Type::ClearcoatNormal)) {
                samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::ClearcoatNormal, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                fragmentShader.addFunction("sampleNormalTexture");
                fragmentShader << "    float qt_clearcoat_normal_strength = qt_material_clearcoat_normal_strength;\n";
                maskVariableByVertexColorChannel( "qt_clearcoat_normal_strength", QSSGRenderDefaultMaterial::ClearcoatNormalStrengthMask  );
                fragmentShader << "    qt_clearcoatNormal = qt_sampleNormalTexture(" << samplerState.samplerName(QSSGRenderableImage::Type::ClearcoatNormal)
                               << ", qt_clearcoat_normal_strength, "
                               << samplerState.fragCoordsName(QSSGRenderableImage::Type::ClearcoatNormal)
                               << ", qt_tangent, qt_binormal, qt_world_normal);\n";

            } else {
                // same as qt_world_normal then
                fragmentShader << "    qt_clearcoatNormal = qt_world_normal;\n";
            }
        }
    }

    // Normal / Bump Map
    if (passRequirmentState.needsWorldNormal) {
        if (samplerState.hasImage(QSSGRenderableImage::Type::Bump)) {
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Bump, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            fragmentShader.append("    float qt_bumpAmount = qt_material_properties2.y;\n");
            maskVariableByVertexColorChannel( "qt_bumpAmount", QSSGRenderDefaultMaterial::NormalStrengthMask );
            fragmentShader.addInclude("defaultMaterialBumpNoLod.glsllib");
            fragmentShader << "    qt_world_normal = qt_defaultMaterialBumpNoLod("
                           << samplerState.samplerName(QSSGRenderableImage::Type::Bump)
                           << ", qt_bumpAmount, " << samplerState.fragCoordsName(QSSGRenderableImage::Type::Bump)
                           << ", qt_tangent, qt_binormal, qt_world_normal);\n";
        } else if (samplerState.hasImage(QSSGRenderableImage::Type::Normal)) {
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Normal, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            fragmentShader.append("    float qt_normalStrength = qt_material_properties2.y;\n");
            maskVariableByVertexColorChannel( "qt_normalStrength", QSSGRenderDefaultMaterial::NormalStrengthMask );
            fragmentShader.addFunction("sampleNormalTexture");
            fragmentShader << "    qt_world_normal = qt_sampleNormalTexture(" << samplerState.samplerName(QSSGRenderableImage::Type::Normal)
                           << ", qt_normalStrength, " << samplerState.fragCoordsName(QSSGRenderableImage::Type::Normal)
                           << ", qt_tangent, qt_binormal, qt_world_normal);\n";
        }
    }

    // !hasLighting does not mean 'no light source'
    // it should be KHR_materials_unlit
    // https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_unlit
    if ((passRequirmentState.hasLighting || passRequirmentState.passType == PassRequirmentsState::Normal) && passRequirmentState.needsWorldNormal) {
        fragmentShader.append("    vec3 tmp_light_color;");
    }

    if (passRequirmentState.hasSpecularLight || samplerState.isActive()) {
        fragmentShader.append("    vec3 qt_specularBase;");
        fragmentShader.addUniform("qt_material_specular", "vec4");
        if (hasCustomFrag)
            fragmentShader.append("    vec3 qt_specularTint = vec3(1.0);");
        else
            fragmentShader.append("    vec3 qt_specularTint = qt_material_specular.rgb;");
    }

    // Base Color / Diffuse / Albedo
    if ((samplerState.hasImage(QSSGRenderableImage::Type::BaseColor) || samplerState.hasImage(QSSGRenderableImage::Type::Diffuse)) && passRequirmentState.needsBaseColor) {

        // first off, which one
        QSSGRenderableImage::Type baseImageType = QSSGRenderableImage::Type::Unknown;
        if (samplerState.hasImage(QSSGRenderableImage::Type::BaseColor))
            baseImageType = QSSGRenderableImage::Type::BaseColor;
        else if (samplerState.hasImage(QSSGRenderableImage::Type::Diffuse))
            baseImageType = QSSGRenderableImage::Type::Diffuse;

        // Generate the UVs and sampler snippets
        samplerState.generateImageUVAndSampler(baseImageType, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);

        if (keyProps.m_baseColorSingleChannelEnabled.getValue(inKey)) {
            const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::BaseColorChannel];
            fragmentShader << "    vec4 qt_base_texture_color = vec4(vec3(texture2D(" << samplerState.samplerName(baseImageType)
                           << ", " << samplerState.fragCoordsName(baseImageType) << ")" << channelStr(channelProps, inKey) << "), 1.0f);\n";
        } else {
            fragmentShader << "    vec4 qt_base_texture_color = texture2D(" << samplerState.samplerName(baseImageType)
                           << ", " << samplerState.fragCoordsName(baseImageType) << ");\n";
        }

        if (keyProps.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BaseColorMap].isPreMultipliedAlpha(inKey))
            fragmentShader << "    qt_base_texture_color.rgb /= qt_base_texture_color.a;\n";

        if (!keyProps.m_imageMaps[QSSGShaderDefaultMaterialKeyProperties::BaseColorMap].isLinear(inKey)) {
            // Diffuse/BaseColor maps need to converted to linear color space
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader << "    qt_base_texture_color = qt_sRGBToLinear(qt_base_texture_color);\n";
        }

        fragmentShader << "    qt_diffuseColor *= qt_base_texture_color;\n";
    }

    // Alpha cutoff
    if (keyProps.m_alphaMode.getAlphaMode(inKey) == QSSGRenderDefaultMaterial::MaterialAlphaMode::Mask) {
        // The Implementation Notes from
        // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#alpha-coverage
        // must be met. Hence the discard.
        fragmentShader << "    if (qt_diffuseColor.a < qt_material_properties3.y) {\n"
                       << "        qt_diffuseColor = vec4(0.0);\n"
                       << "        discard;\n"
                       << "    } else {\n"
                       << "        qt_diffuseColor.a = 1.0;\n"
                       << "    }\n";
    } else if (keyProps.m_alphaMode.getAlphaMode(inKey) == QSSGRenderDefaultMaterial::MaterialAlphaMode::Opaque) {
        fragmentShader << "    qt_diffuseColor.a = 1.0;\n";
    }

    // Opacity
    if (samplerState.hasImage(QSSGRenderableImage::Type::Opacity)) {
        samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Opacity, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
        const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OpacityChannel];
        fragmentShader << "    float qt_opacity_map_value = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Opacity)
                       << ", " << samplerState.fragCoordsName(QSSGRenderableImage::Type::Opacity) << ")" << channelStr(channelProps, inKey) << ";\n";
        if (keyProps.m_invertOpacityMapValue.getValue(inKey))
            fragmentShader << "    qt_opacity_map_value = 1.0 - qt_opacity_map_value;\n";
        fragmentShader << "    qt_objectOpacity *= qt_opacity_map_value;\n";
    }

    // Ambient Occlusion
    if (passRequirmentState.needsAmbientOcclusion) {
        addLocalVariable(fragmentShader, "qt_aoFactor", "float");

        if (passRequirmentState.hasSSAOMap) {
            fragmentShader.addInclude("ssao.glsllib");
            fragmentShader.append("    qt_aoFactor = qt_screenSpaceAmbientOcclusionFactor();");
        } else {
            fragmentShader.append("    qt_aoFactor = 1.0;");
        }

        if (hasCustomFrag)
            fragmentShader << "    qt_aoFactor *= qt_customOcclusionAmount;\n";
    }

    // Roughness
    if (passRequirmentState.needsRoughness) {
        if (hasCustomFrag)
            fragmentShader << "    float qt_roughnessAmount = qt_customSpecularRoughness;\n";
        else
            fragmentShader << "    float qt_roughnessAmount = qt_material_properties.y;\n";

        maskVariableByVertexColorChannel( "qt_roughnessAmount", QSSGRenderDefaultMaterial::RoughnessMask );

        if (samplerState.hasImage(QSSGRenderableImage::Type::Roughness)) {
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Roughness, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::RoughnessChannel];
            fragmentShader << "    qt_roughnessAmount *= texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Roughness) << ", "
                           << samplerState.fragCoordsName(QSSGRenderableImage::Type::Roughness) << ")" << channelStr(channelProps, inKey) << ";\n";
        }

        // Convert Glossy to Roughness
        if (passRequirmentState.isSpecularGlossinessWorkflow)
            fragmentShader << "    qt_roughnessAmount = clamp(1.0 - qt_roughnessAmount, 0.0, 1.0);\n";
    }

    // Metalness
    if (passRequirmentState.needsMetalness) {
        if (hasCustomFrag)
            fragmentShader << "    float qt_metalnessAmount = qt_customMetalnessAmount;\n";
        else if (!passRequirmentState.isSpecularGlossinessWorkflow)
            fragmentShader << "    float qt_metalnessAmount = qt_material_properties.z;\n";
        else
            fragmentShader << "    float qt_metalnessAmount = 0.0;\n";

        maskVariableByVertexColorChannel( "qt_metalnessAmount", QSSGRenderDefaultMaterial::MetalnessMask );

        if (passRequirmentState.hasSpecularLight && samplerState.hasImage(QSSGRenderableImage::Type::Metalness)) {
            const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::MetalnessChannel];
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Metalness, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            fragmentShader << "    float qt_sampledMetalness = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Metalness) << ", "
                           << samplerState.fragCoordsName(QSSGRenderableImage::Type::Metalness) << ")" << channelStr(channelProps, inKey) << ";\n";
            fragmentShader << "    qt_metalnessAmount = clamp(qt_metalnessAmount * qt_sampledMetalness, 0.0, 1.0);\n";
        }
    }

    // Special case for depth pre-pass
    if (passRequirmentState.shouldDiscardNonOpaque()) {
        fragmentShader << "    if ((qt_diffuseColor.a * qt_objectOpacity) < 1.0)\n";
        fragmentShader << "        discard;\n";
    }

    // This is a Lighting Pass
    if (passRequirmentState.hasLighting && (passRequirmentState.needsDiffuseLight || passRequirmentState.needsSpecularLight)) {
        if (passRequirmentState.hasSpecularLight) {
            vertexShader.generateViewVector(inKey);
            fragmentShader.addUniform("qt_material_properties", "vec4");

            if (passRequirmentState.isPbrMaterial)
                fragmentShader << "    qt_specularBase = vec3(1.0);\n";
            else
                fragmentShader << "    qt_specularBase = qt_diffuseColor.rgb;\n";
            if (hasCustomFrag)
                fragmentShader << "    float qt_specularFactor = qt_customSpecularAmount;\n";
            else
                fragmentShader << "    float qt_specularFactor = qt_material_properties.x;\n";

            maskVariableByVertexColorChannel( "qt_specularFactor", QSSGRenderDefaultMaterial::SpecularAmountMask  );
        }

        fragmentShader.addUniform("qt_light_ambient_total", "vec3");

        fragmentShader.append("    vec4 global_diffuse_light = vec4(0.0);");

        if (passRequirmentState.hasLightMap) {
            fragmentShader << "    global_diffuse_light.rgb = qt_lightmap_color(qt_texCoordLightmap) * (1.0 - qt_metalnessAmount) * qt_diffuseColor.rgb;\n";
        } else {
            if (hasCustomFrag && hasCustomFunction(QByteArrayLiteral("qt_ambientLightProcessor"))) {
                // DIFFUSE, TOTAL_AMBIENT_COLOR, NORMAL, VIEW_VECTOR(, SHARED)
                fragmentShader.append("    qt_ambientLightProcessor(global_diffuse_light.rgb, qt_light_ambient_total.rgb * (1.0 - qt_metalnessAmount) * qt_diffuseColor.rgb, qt_world_normal, qt_view_vector");
                if (usesSharedVar)
                    fragmentShader << ", qt_customShared);\n";
                else
                    fragmentShader << ");\n";
            } else {
                fragmentShader.append("    global_diffuse_light = vec4(qt_light_ambient_total.rgb * (1.0 - qt_metalnessAmount) * qt_diffuseColor.rgb, 0.0);");
            }
        }

        fragmentShader.append("    vec3 global_specular_light = vec3(0.0);");

        // Fragment lighting means we can perhaps attenuate the specular amount by a texture
        // lookup.
        if (samplerState.hasImage(QSSGRenderableImage::Type::SpecularAmountMap)) {
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::SpecularAmountMap, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);

            if (keyProps.m_specularSingleChannelEnabled.getValue(inKey)) {
                const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::SpecularAmountChannel];
                fragmentShader << "    vec4 qt_specular_amount_map = vec4(vec3(texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::SpecularAmountMap)
                               << ", " << samplerState.fragCoordsName(QSSGRenderableImage::Type::SpecularAmountMap) << ")" << channelStr(channelProps, inKey) << "), 1.0f);\n";
            } else {
                fragmentShader << "    vec4 qt_specular_amount_map = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::SpecularAmountMap)
                               << ", " << samplerState.fragCoordsName(QSSGRenderableImage::Type::SpecularAmountMap) << ");\n";
            }
            fragmentShader << "    qt_specularBase *= qt_sRGBToLinear(qt_specular_amount_map).rgb;\n";
        }

        if (passRequirmentState.hasSpecularLight) {
            if (passRequirmentState.isSpecularGlossinessWorkflow) {
                fragmentShader << "    qt_specularTint *= qt_specularBase;\n";
                fragmentShader << "    vec3 qt_specularAmount = vec3(1.0);\n";
            } else {
                fragmentShader << "    vec3 qt_specularAmount = qt_specularBase * vec3(qt_metalnessAmount + qt_specularFactor * (1.0 - qt_metalnessAmount));\n";
            }
        }

        if (samplerState.hasImage(QSSGRenderableImage::Type::Translucency)) {
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Translucency, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TranslucencyChannel];
            fragmentShader << "    float qt_translucent_depth_range = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Translucency)
                           << ", " << samplerState.fragCoordsName(QSSGRenderableImage::Type::Translucency) << ")" << channelStr(channelProps, inKey) << ";\n";
            fragmentShader << "    float qt_translucent_thickness = qt_translucent_depth_range * qt_translucent_depth_range;\n";
            fragmentShader << "    float qt_translucent_thickness_exp = exp(qt_translucent_thickness * qt_material_properties2.z);\n";
        }

        // Occlusion Map
        if (samplerState.hasImage(QSSGRenderableImage::Type::Occlusion)) {
            addLocalVariable(fragmentShader, "qt_ao", "float");
            samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Occlusion, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
            const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::OcclusionChannel];
            fragmentShader << "    qt_ao = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Occlusion) << ", "
                           << samplerState.fragCoordsName(QSSGRenderableImage::Type::Occlusion) << ")" << channelStr(channelProps, inKey) << ";\n";
            fragmentShader << "    qt_aoFactor *= qt_ao * qt_material_properties3.x;\n"; // qt_material_properties3.x is the OcclusionAmount
            maskVariableByVertexColorChannel( "qt_aoFactor", QSSGRenderDefaultMaterial::OcclusionAmountMask );
        }

        if (passRequirmentState.hasClearcoat) {
            addLocalVariable(fragmentShader, "qt_clearcoatAmount", "float");
            addLocalVariable(fragmentShader, "qt_clearcoatRoughness", "float");
            addLocalVariable(fragmentShader, "qt_clearcoatF0", "vec3");
            addLocalVariable(fragmentShader, "qt_clearcoatF90", "vec3");
            addLocalVariable(fragmentShader, "qt_global_clearcoat", "vec3");

            if (hasCustomFrag)
                fragmentShader << "    qt_clearcoatAmount = qt_customClearcoatAmount;\n";
            else
                fragmentShader << "    qt_clearcoatAmount = qt_material_properties3.z;\n";
            maskVariableByVertexColorChannel( "qt_clearcoatAmount", QSSGRenderDefaultMaterial::ClearcoatAmountMask  );
            if (hasCustomFrag)
                fragmentShader << "    qt_clearcoatRoughness = qt_customClearcoatRoughness;\n";
            else
                fragmentShader << "    qt_clearcoatRoughness = qt_material_properties3.w;\n";
            maskVariableByVertexColorChannel( "qt_clearcoatRoughness", QSSGRenderDefaultMaterial::ClearcoatRoughnessAmountMask  );
            fragmentShader << "    qt_clearcoatF0 = vec3(((1.0-qt_iOR) * (1.0-qt_iOR)) / ((1.0+qt_iOR) * (1.0+qt_iOR)));\n";
            fragmentShader << "    qt_clearcoatF90 = vec3(1.0);\n";
            fragmentShader << "    qt_global_clearcoat = vec3(0.0);\n";

            if (samplerState.hasImage(QSSGRenderableImage::Type::Clearcoat)) {
                samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Clearcoat, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatChannel];
                fragmentShader << "    qt_clearcoatAmount *= texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Clearcoat) << ", "
                               << samplerState.fragCoordsName(QSSGRenderableImage::Type::Clearcoat) << ")" << channelStr(channelProps, inKey) << ";\n";
            }

            if (samplerState.hasImage(QSSGRenderableImage::Type::ClearcoatRoughness)) {
                samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::ClearcoatRoughness, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ClearcoatRoughnessChannel];
                fragmentShader << "    qt_clearcoatRoughness *= texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::ClearcoatRoughness) << ", "
                               << samplerState.fragCoordsName(QSSGRenderableImage::Type::ClearcoatRoughness) << ")" << channelStr(channelProps, inKey) << ";\n";
                fragmentShader << "    qt_clearcoatRoughness = clamp(qt_clearcoatRoughness, 0.0, 1.0);\n";
            }
        }

        if (passRequirmentState.hasTransmission) {
            fragmentShader.addInclude("transmission.glsllib");
            addLocalVariable(fragmentShader, "qt_transmissionFactor", "float");
            addLocalVariable(fragmentShader, "qt_global_transmission", "vec3");
            // Volume
            addLocalVariable(fragmentShader, "qt_thicknessFactor", "float");
            addLocalVariable(fragmentShader, "qt_attenuationColor", "vec3");
            addLocalVariable(fragmentShader, "qt_attenuationDistance", "float");
            fragmentShader << "    qt_global_transmission = vec3(0.0);\n";

            if (hasCustomFrag) {
                fragmentShader << "    qt_transmissionFactor = qt_customTransmissionFactor;\n";
                fragmentShader << "    qt_thicknessFactor = qt_customThicknessFactor;\n";
                fragmentShader << "    qt_attenuationColor = qt_customAttenuationColor;\n";
                fragmentShader << "    qt_attenuationDistance = qt_customAttenuationDistance;\n";
            } else {
                fragmentShader << "    qt_transmissionFactor = qt_material_properties4.w;\n";
                maskVariableByVertexColorChannel( "qt_transmissionFactor", QSSGRenderDefaultMaterial::TransmissionFactorMask );

                if (samplerState.hasImage(QSSGRenderableImage::Type::Transmission)) {
                    samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Transmission, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                    const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::TransmissionChannel];
                    fragmentShader << "    qt_transmissionFactor *= texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Transmission) << ", "
                                   << samplerState.fragCoordsName(QSSGRenderableImage::Type::Transmission) << ")" << channelStr(channelProps, inKey) << ";\n";
                }

                fragmentShader << "    qt_thicknessFactor = qt_material_thickness;\n";
                maskVariableByVertexColorChannel( "qt_thicknessFactor", QSSGRenderDefaultMaterial::ThicknessFactorMask );
                fragmentShader << "    qt_attenuationColor = qt_material_attenuation.xyz;\n";
                fragmentShader << "    qt_attenuationDistance = qt_material_attenuation.w;\n";

                if (samplerState.hasImage(QSSGRenderableImage::Type::Thickness)) {
                    samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Thickness, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                    const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::ThicknessChannel];
                    fragmentShader << "    qt_thicknessFactor *= texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Thickness) << ", "
                                   << samplerState.fragCoordsName(QSSGRenderableImage::Type::Thickness) << ")" << channelStr(channelProps, inKey) << ";\n";
                }
            }
        }
        if (passRequirmentState.hasPunctualLights || passRequirmentState.hasSpecularLight) {
            fragmentShader << "    vec3 qt_f0 = vec3(1.0);\n";
            fragmentShader << "    vec3 qt_f90 = vec3(1.0);\n";
        }

        if (passRequirmentState.hasSpecularLight) {
            fragmentShader.addInclude("principledMaterialFresnel.glsllib");
            if (!passRequirmentState.isSpecularGlossinessWorkflow) {
                fragmentShader << "    qt_f0 = qt_F0_ior(qt_iOR, qt_metalnessAmount, qt_diffuseColor.rgb);\n";
            } else {
                fragmentShader << "    const float qt_reflectance = max(max(qt_specularTint.r, qt_specularTint.g), qt_specularTint.b);\n";
                fragmentShader << "    qt_f0 = qt_specularTint;\n";
                fragmentShader << "    qt_specularTint = vec3(1.0);\n";
                fragmentShader << "    qt_f90 = vec3(clamp(qt_reflectance * 50.0, 0.0, 1.0));\n";
                fragmentShader << "    qt_diffuseColor.rgb *= (1 - qt_reflectance);\n";
            }

            if (passRequirmentState.isSpecularAAEnabled) {
                fragmentShader.append("    vec3 vNormalWsDdx = dFdx(qt_world_normal.xyz);\n");
                fragmentShader.append("    vec3 vNormalWsDdy = dFdy(qt_world_normal.xyz);\n");
                fragmentShader.append("    float flGeometricRoughnessFactor = pow(clamp(max(dot(vNormalWsDdx, vNormalWsDdx), dot(vNormalWsDdy, vNormalWsDdy)), 0.0, 1.0), 0.333);\n");
                fragmentShader.append("    qt_roughnessAmount = max(flGeometricRoughnessFactor, qt_roughnessAmount);\n");
            }

            if (hasCustomFrag)
                fragmentShader << "    float qt_fresnelPower = qt_customFresnelPower;\n";
            else
                fragmentShader << "    float qt_fresnelPower = qt_material_properties2.x;\n";

            if (passRequirmentState.isPbrMaterial) {
                fragmentShader << "    vec3 qt_principledMaterialFresnelValue = qt_principledMaterialFresnel(qt_world_normal, qt_view_vector, qt_f0, qt_roughnessAmount, qt_fresnelPower);\n";
                if (passRequirmentState.hasFresnelScaleBias) {
                    if (hasCustomFrag) {
                        fragmentShader << "    float qt_fresnelScale = qt_customFresnelScale;\n";
                        fragmentShader << "    float qt_fresnelBias = qt_customFresnelBias;\n";
                    } else {
                        fragmentShader << "    float qt_fresnelScale = qt_material_properties5.x;\n";
                        fragmentShader << "    float qt_fresnelBias = qt_material_properties5.y;\n";
                    }
                    fragmentShader << "    qt_principledMaterialFresnelValue = clamp(vec3(qt_fresnelBias) + "
                                   << "qt_fresnelScale * qt_principledMaterialFresnelValue, 0.0, 1.0);\n";
                }
                fragmentShader << "    qt_specularAmount *= qt_principledMaterialFresnelValue;\n";
                if (passRequirmentState.isMetallicRoughnessWorkflow) {
                    // Make sure that we scale the specularTint with repsect to metalness (no tint if qt_metalnessAmount == 1)
                    // We actually need to do this here because we won't know the final metalness value until this point.
                    fragmentShader << "    qt_specularTint = mix(vec3(1.0), qt_specularTint, 1.0 - qt_metalnessAmount);\n";
                }
            } else {
                fragmentShader << "    qt_specularAmount *= qt_principledMaterialFresnel(qt_world_normal, qt_view_vector, qt_f0, qt_roughnessAmount, qt_fresnelPower);\n";
            }
        }

        if (passRequirmentState.hasLighting && passRequirmentState.hasPunctualLights) {
            fragmentShader.addUniform("qt_lightAndShadowCounts", "vec4");
            fragmentShader.addFunction("processPunctualLighting");
            fragmentShader << "    qt_processPunctualLighting(global_diffuse_light.rgb,\n"
                           << "                               global_specular_light.rgb,\n"
                           << "                               qt_diffuseColor.rgb,\n"
                           << "                               qt_varWorldPos,\n"
                           << "                               qt_world_normal.xyz,\n"
                           << "                               qt_view_vector,\n"
                           << "#if QSSG_ENABLE_SPECULAR\n"
                           << "                               qt_specularAmount,\n"
                           << "                               qt_specularTint,\n"
                           << "#endif // QSSG_ENABLE_SPECULAR\n"
                           << "                               qt_roughnessAmount,\n"
                           << "                               qt_metalnessAmount,\n"
                           << "#if QSSG_CUSTOM_MATERIAL_DIRECTIONAL_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_POINT_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_SPOT_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR\n"
                           << "                               qt_customBaseColor,\n"
                           << "#endif // QSSG_CUSTOM_MATERIAL_*\n"
                           << "#if QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR\n"
                           << "                               qt_customSpecularAmount,\n"
                           << "#endif // QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR\n"
                           << "#if QSSG_CUSTOM_MATERIAL_SHARED_VARIABLES\n"
                           << "                               qt_customShared,\n"
                           << "#endif // QSSG_CUSTOM_MATERIAL_SHARED_VARIABLES\n"
                           << "#if QSSG_ENABLE_CLEARCOAT\n"
                           << "                               qt_global_clearcoat,\n"
                           << "                               qt_clearcoatNormal,\n"
                           << "                               qt_clearcoatRoughness,\n"
                           << "                               qt_clearcoatF0,\n"
                           << "                               qt_clearcoatF90,\n"
                           << "#endif // QSSG_ENABLE_CLEARCOAT\n"
                           << "#if QSSG_ENABLE_TRANSMISSION\n"
                           << "                               qt_global_transmission,\n"
                           << "                               qt_thicknessFactor,\n"
                           << "                               qt_iOR,\n"
                           << "                               qt_transmissionFactor,\n"
                           << "                               qt_attenuationColor,\n"
                           << "                               qt_attenuationDistance,\n"
                           << "#endif // QSSG_ENABLE_TRANSMISSION\n"
                           << "                               qt_f0,\n"
                           << "                               qt_f90);\n";
        } //QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR

        // The color in rgb is ready, including shadowing, just need to apply
        // the ambient occlusion factor. The alpha is the model opacity
        // multiplied by the alpha from the material color and/or the vertex colors.
        fragmentShader << "    global_diffuse_light = vec4(global_diffuse_light.rgb * qt_aoFactor, qt_objectOpacity * qt_diffuseColor.a);\n";

        if (passRequirmentState.hasReflectionProbe) {
            vertexShader.generateWorldNormal(inKey);
            fragmentShader.addInclude("sampleReflectionProbe.glsllib");

            // Diffuse
            if (passRequirmentState.isPbrMaterial)
                fragmentShader << "    global_diffuse_light.rgb += qt_diffuseColor.rgb * (1.0 - qt_specularAmount) * qt_sampleDiffuseReflection(qt_reflectionMap, qt_world_normal).rgb;\n";
            else
                fragmentShader << "    global_diffuse_light.rgb += qt_diffuseColor.rgb * qt_sampleDiffuseReflection(qt_reflectionMap, qt_world_normal).rgb;\n";

            // Specular
            if (passRequirmentState.hasSpecularLight) {
                if (passRequirmentState.isPbrMaterial)
                    fragmentShader << "    global_specular_light += qt_specularTint * qt_sampleGlossyReflectionPrincipled(qt_reflectionMap, qt_world_normal, qt_view_vector, qt_specularAmount, qt_roughnessAmount).rgb;\n";
                else
                    fragmentShader << "    global_specular_light += qt_specularAmount * qt_specularTint * qt_sampleGlossyReflection(qt_reflectionMap, qt_world_normal, qt_view_vector, qt_roughnessAmount).rgb;\n";
            }

            // Clearcoat (pbr Only)
            if (passRequirmentState.hasClearcoat)
                fragmentShader << "    qt_global_clearcoat += qt_sampleGlossyReflectionPrincipled(qt_reflectionMap, qt_clearcoatNormal, qt_view_vector, qt_clearcoatF0, qt_clearcoatRoughness).rgb;\n";

        } else if (passRequirmentState.hasIblProbe) {
            vertexShader.generateWorldNormal(inKey);
            fragmentShader.addInclude("sampleProbe.glsllib");
            if (hasCustomIblProbe) {
                // DIFFUSE, SPECULAR, BASE_COLOR, AO_FACTOR, SPECULAR_AMOUNT, NORMAL, VIEW_VECTOR, IBL_ORIENTATION(, SHARED)
                fragmentShader << "    vec3 qt_iblDiffuse = vec3(0.0);\n";
                fragmentShader << "    vec3 qt_iblSpecular = vec3(0.0);\n";
                fragmentShader << "    qt_iblProbeProcessor(qt_iblDiffuse, qt_iblSpecular, qt_customBaseColor, qt_aoFactor, qt_specularFactor, qt_roughnessAmount, qt_world_normal, qt_view_vector";
                if (passRequirmentState.hasIblOrientation)
                    fragmentShader << ", qt_lightProbeOrientation";
                else
                    fragmentShader << ", mat3(1.0)";
                if (usesSharedVar)
                    fragmentShader << ", qt_customShared);\n";
                else
                    fragmentShader << ");\n";
            } else {
                // Diffuse
                if (passRequirmentState.isPbrMaterial)
                    fragmentShader << "    vec3 qt_iblDiffuse = qt_diffuseColor.rgb * (1.0 - qt_specularAmount) * qt_sampleDiffuse(qt_world_normal).rgb;\n";
                else
                    fragmentShader << "    vec3 qt_iblDiffuse = qt_diffuseColor.rgb * qt_sampleDiffuse(qt_world_normal).rgb;\n";

                // Specular
                if (passRequirmentState.hasSpecularLight) {
                    if (passRequirmentState.isPbrMaterial)
                        fragmentShader << "    vec3 qt_iblSpecular = qt_specularTint * qt_sampleGlossyPrincipled(qt_world_normal, qt_view_vector, qt_specularAmount, qt_roughnessAmount).rgb;\n";
                    else
                        fragmentShader << "    vec3 qt_iblSpecular = qt_specularAmount * qt_specularTint * qt_sampleGlossy(qt_world_normal, qt_view_vector, qt_roughnessAmount).rgb;\n";
                }

                // Clearcoat (pbr Only)
                if (passRequirmentState.hasClearcoat)
                    fragmentShader << "   vec3 qt_iblClearcoat = qt_sampleGlossyPrincipled(qt_clearcoatNormal, qt_view_vector, qt_clearcoatF0, qt_clearcoatRoughness).rgb;\n";
            }

            fragmentShader << "    global_diffuse_light.rgb += qt_iblDiffuse * qt_aoFactor;\n";
            if (passRequirmentState.hasSpecularLight)
                fragmentShader << "    global_specular_light += qt_iblSpecular * qt_aoFactor;\n";
            if (passRequirmentState.hasClearcoat)
                fragmentShader << "    qt_global_clearcoat += qt_iblClearcoat * qt_aoFactor;\n";
        } else if (hasCustomIblProbe) {
            // Prevent breaking the fragment code while seeking uniforms
            fragmentShader.addUniform("qt_lightProbe", "samplerCube");
            fragmentShader.addUniform("qt_lightProbeProperties", "vec4");
        }

        // This can run even without a IBL probe
        if (passRequirmentState.hasTransmission) {
            fragmentShader << "    qt_global_transmission += qt_transmissionFactor * qt_getIBLVolumeRefraction(qt_world_normal, qt_view_vector, qt_roughnessAmount, "
                              "qt_diffuseColor.rgb, qt_specularAmount, qt_varWorldPos, qt_iOR, qt_thicknessFactor, qt_attenuationColor, qt_attenuationDistance);\n";
        }


        // Handle specular and emissive maps which just add additional color
        if (samplerState.hasImage(QSSGRenderableImage::Type::Specular) || samplerState.hasImage(QSSGRenderableImage::Type::Emissive)) {
            addLocalVariable(fragmentShader, "qt_texture_color", "vec4");

            if (samplerState.hasImage(QSSGRenderableImage::Type::Specular)) {
                samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Specular, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                fragmentShader << "    qt_texture_color = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Specular) << ", "
                               << samplerState.fragCoordsName(QSSGRenderableImage::Type::Specular) << ");\n";
                fragmentShader.addInclude("tonemapping.glsllib");
                fragmentShader << "    global_specular_light += qt_sRGBToLinear(qt_texture_color.rgb) * qt_specularTint;\n";
                fragmentShader << "    global_diffuse_light.a *= qt_texture_color.a;\n";
            }

            if (samplerState.hasImage(QSSGRenderableImage::Type::Emissive)) {
                samplerState.generateImageUVAndSampler(QSSGRenderableImage::Type::Emissive, vertexShader, fragmentShader, inKey, passRequirmentState.hasParallaxMapping);
                fragmentShader << "    qt_texture_color = texture2D(" << samplerState.samplerName(QSSGRenderableImage::Type::Emissive) << ", "
                               << samplerState.fragCoordsName(QSSGRenderableImage::Type::Emissive) << ");\n";
                fragmentShader.addInclude("tonemapping.glsllib");
                if (keyProps.m_emissiveSingleChannelEnabled.getValue(inKey)) {
                    const auto &channelProps = keyProps.m_textureChannels[QSSGShaderDefaultMaterialKeyProperties::EmissiveChannel];
                    fragmentShader << "    qt_global_emission *= qt_sRGBToLinear(vec3(qt_texture_color" <<
                                            channelStr(channelProps, inKey) << "));\n";
                } else {
                    fragmentShader << "    qt_global_emission *= qt_sRGBToLinear(qt_texture_color.rgb);\n";
                }
            }
        }

        if (passRequirmentState.hasTransmission)
            fragmentShader << "    global_diffuse_light.rgb = mix(global_diffuse_light.rgb, qt_global_transmission, qt_transmissionFactor);\n";

        if (passRequirmentState.isMetallicRoughnessWorkflow) {
            fragmentShader << "    global_diffuse_light.rgb *= 1.0 - qt_metalnessAmount;\n";
        }

        if (passRequirmentState.hasFog) {
            fragmentShader.addInclude("fog.glsllib");
            fragmentShader << "    calculateFog(qt_global_emission, global_specular_light, global_diffuse_light.rgb);\n";
        }

        fragmentShader << "    vec4 qt_color_sum = vec4(global_diffuse_light.rgb + global_specular_light + qt_global_emission, global_diffuse_light.a);\n";

        if (passRequirmentState.hasClearcoat) {
            fragmentShader.addInclude("bsdf.glsllib");
            if (hasCustomFrag)
                fragmentShader << "    float qt_clearcoatFresnelPower = qt_customClearcoatFresnelPower;\n";
            else
                fragmentShader << "    float qt_clearcoatFresnelPower = qt_material_clearcoat_fresnel_power;\n";
            fragmentShader << "    vec3 qt_clearcoatFresnel = qt_schlick3(qt_clearcoatF0, qt_clearcoatF90, clamp(dot(qt_clearcoatNormal, qt_view_vector), 0.0, 1.0), qt_clearcoatFresnelPower);\n";
            if (passRequirmentState.hasClearcoatFresnelScaleBias) {
                if (hasCustomFrag) {
                    fragmentShader << "    float qt_clearcoatFresnelScale = qt_customClearcoatFresnelScale;\n";
                    fragmentShader << "    float qt_clearcoatFresnelBias = qt_customClearcoatFresnelBias;\n";
                }else {
                    fragmentShader << "    float qt_clearcoatFresnelScale = qt_material_properties5.z;\n";
                    fragmentShader << "    float qt_clearcoatFresnelBias = qt_material_properties5.w;\n";
                }
                fragmentShader << "    qt_clearcoatFresnel = clamp(vec3(qt_clearcoatFresnelBias) + qt_clearcoatFresnelScale * qt_clearcoatFresnel, 0.0, 1.0);\n";
            }
            fragmentShader << "    qt_global_clearcoat = qt_global_clearcoat * qt_clearcoatAmount;\n";
            fragmentShader << "    qt_color_sum.rgb = qt_color_sum.rgb * (1.0 - qt_clearcoatAmount * qt_clearcoatFresnel) + qt_global_clearcoat;\n";
        }

        if (hasCustomFrag && hasCustomFunction(QByteArrayLiteral("qt_customPostProcessor"))) {
            // COLOR_SUM, DIFFUSE, SPECULAR, EMISSIVE, UV0, UV1(, SHARED)
            fragmentShader << "    qt_customPostProcessor(qt_color_sum, global_diffuse_light, global_specular_light, qt_global_emission, qt_texCoord0, qt_texCoord1";
            if (usesSharedVar)
                fragmentShader << ", qt_customShared);\n";
            else
                fragmentShader << ");\n";
        }
    } // end of lighting block

    // Outputs
    switch (passRequirmentState.passType) {

    case PassRequirmentsState::None:
        break;
    case PassRequirmentsState::Color:
        // Unlit color pass
        if (!passRequirmentState.hasLighting)
            fragmentShader.append("    vec4 qt_color_sum = vec4(qt_diffuseColor.rgb, qt_diffuseColor.a * qt_objectOpacity);");

        if (passRequirmentState.oitMethod == QSSGRenderLayer::OITMethod::WeightedBlended) {
            fragmentShader.addInclude("oitweightedblended.glsllib");
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.addUniform("qt_cameraPosition", "vec3");
            fragmentShader.addUniform("qt_cameraProperties", "vec2");
            fragmentShader.append("    float z = abs(gl_FragCoord.z);");
            fragmentShader.append("    qt_color_sum.rgb = qt_tonemap(qt_color_sum.rgb) * qt_color_sum.a;");
            fragmentShader.append("    fragOutput = qt_color_sum * qt_transparencyWeight(z, qt_color_sum.a, qt_cameraProperties.y);");
            fragmentShader.append("    revealageOutput = vec4(qt_color_sum.a);");
        } else if (passRequirmentState.oitMethod == QSSGRenderLayer::OITMethod::LinkedList) {
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.addUniform("qt_listNodeCount", "uint");
            fragmentShader.addUniform("qt_ABufImageWidth", "uint");
            fragmentShader.addUniform("qt_viewSize", "ivec2");
            if (passRequirmentState.oitMSAA)
                fragmentShader.addDefinition("QSSG_MULTISAMPLE", "1");
#ifdef QSSG_OIT_USE_BUFFERS
            QSSGShaderResourceMergeContext::setAdditionalBufferAmount(3);
            fragmentShader.addUniform("qt_samples", "uint");
            fragmentShader.addInclude("oitlinkedlist_buf.glsllib");
            if (viewCount >= 2)
                fragmentShader.append("    fragOutput = qt_oitLinkedList(qt_tonemap(qt_color_sum), qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, qt_viewIndex, qt_samples);");
            else
                fragmentShader.append("    fragOutput = qt_oitLinkedList(qt_tonemap(qt_color_sum), qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, 0, qt_samples);");
#else
            fragmentShader.addInclude("oitlinkedlist.glsllib");
            if (viewCount >= 2)
                fragmentShader.append("    fragOutput = qt_oitLinkedList(qt_tonemap(qt_color_sum), qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, qt_viewIndex);");
            else
                fragmentShader.append("    fragOutput = qt_oitLinkedList(qt_tonemap(qt_color_sum), qt_listNodeCount, qt_ABufImageWidth, qt_viewSize, 0);");
#endif

        } else {
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.append("    fragOutput = vec4(qt_tonemap(qt_color_sum));");
        }
        break;
    case PassRequirmentsState::Depth:
        fragmentShader << "    vec4 fragOutput = vec4(0.0);\n";
        break;
    case PassRequirmentsState::OrthoShadow:
        Q_ASSERT(viewCount == 1);
        fragmentShader << "    // directional shadow pass\n"
                       << "    float qt_shadowDepth = (qt_varDepth + qt_shadowDepthAdjust.x) * qt_shadowDepthAdjust.y;\n"
                       << "    fragOutput = vec4(qt_shadowDepth);\n";
        break;
    case PassRequirmentsState::PerspectiveShadow:
        Q_ASSERT(viewCount == 1);
        fragmentShader.addUniform("qt_cameraPosition", "vec3");
        fragmentShader.addUniform("qt_cameraProperties", "vec2");
        fragmentShader << "    // omnidirectional shadow pass\n"
                       << "    vec3 qt_shadowCamPos = vec3(qt_cameraPosition.x, qt_cameraPosition.y, qt_cameraPosition.z);\n"
                       << "    float qt_shadowDist = length(qt_varShadowWorldPos - qt_shadowCamPos);\n"
                       << "    qt_shadowDist = (qt_shadowDist - qt_cameraProperties.x) / (qt_cameraProperties.y - qt_cameraProperties.x);\n"
                       << "    fragOutput = vec4(qt_shadowDist, qt_shadowDist, qt_shadowDist, 1.0);\n";
        break;
    case PassRequirmentsState::Normal:
        // world space normal in rgb, roughness in alpha
        fragmentShader.append("    fragOutput = vec4(qt_world_normal, qt_roughnessAmount);\n");
        break;
    case PassRequirmentsState::Debug:
        fragmentShader.append("    vec3 debugOutput = vec3(0.0);\n");
        switch (passRequirmentState.debugMode) {
        case QSSGRenderLayer::MaterialDebugMode::BaseColor:
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.append("    debugOutput += qt_tonemap(qt_diffuseColor.rgb);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Roughness:
            fragmentShader.append("    debugOutput += vec3(qt_roughnessAmount);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Metalness:
            fragmentShader.append("    debugOutput += vec3(qt_metalnessAmount);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Diffuse:
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.append("    debugOutput += qt_tonemap(global_diffuse_light.rgb);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Specular:
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.append("    debugOutput += qt_tonemap(global_specular_light);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::ShadowOcclusion:
            // Technically speaking this was just outputing the occlusion value of the last light processed, which is likely not super useful
            // So for now this just outputs 1.0 (no occlusion)
            fragmentShader.addFunction("debugShadowOcclusion");
            vertexShader.generateWorldPosition(inKey);
            fragmentShader.append("    debugOutput += vec3(qt_debugShadowOcclusion());\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Emission:
            fragmentShader.addInclude("tonemapping.glsllib");
            fragmentShader.append("    debugOutput += qt_tonemap(qt_global_emission);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::AmbientOcclusion:
            fragmentShader.append("    debugOutput += vec3(qt_aoFactor);\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Normal:
            fragmentShader.append("    debugOutput += qt_world_normal * 0.5 + 0.5;\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Tangent:
            fragmentShader.append("    debugOutput += qt_tangent * 0.5 + 0.5;\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::Binormal:
            fragmentShader.append("    debugOutput += qt_binormal * 0.5 + 0.5;\n");
            break;
        case QSSGRenderLayer::MaterialDebugMode::F0:
            if (passRequirmentState.isPbrMaterial)
                fragmentShader.append("    debugOutput += qt_f0;");
            break;
        case QSSGRenderLayer::MaterialDebugMode::None:
            Q_UNREACHABLE();
            break;
        }
        fragmentShader.append("    fragOutput = vec4(debugOutput, 1.0);\n");
        break;
    case PassRequirmentsState::User:
        if (shaderAugmentation.hasUserAugmentation())
           fragmentShader << "    " << shaderAugmentation.body << ";\n";
        break;
    }
}

QSSGRhiShaderPipelinePtr QSSGMaterialShaderGenerator::generateMaterialRhiShader(const QByteArray &inShaderKeyPrefix,
                                                                                QSSGMaterialVertexPipeline &vertexPipeline,
                                                                                const QSSGShaderDefaultMaterialKey &key,
                                                                                const QSSGShaderDefaultMaterialKeyProperties &inProperties,
                                                                                const QSSGShaderFeatures &inFeatureSet,
                                                                                const QSSGRenderGraphObject &inMaterial,
                                                                                QSSGShaderLibraryManager &shaderLibraryManager,
                                                                                QSSGShaderCache &theCache,
                                                                                const QSSGUserShaderAugmentation &shaderAugmentation)
{
    const int viewCount = inFeatureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView)
        ? 1 : inProperties.m_viewCount.getValue(key);

    bool perTargetCompilation = false;
    // Cull mode None implies doing the gl_FrontFacing-based double sided logic
    // in the shader. This may want to ifdef the generated shader code based the
    // target shading language. This is only possible if the QShaderBaker is
    // told to compile to SPIR-V (and then transpile) separately for each target
    // (GLSL, HLSL, etc.), instead of just compiling to SPIR-V once (and
    // transpiling the same bytecode to each target language). This takes more
    // time, so we only do it for multiview since that's also how the logic is
    // going to be written in the generated shader code.
    if (viewCount >= 2) {
        const bool isDoubleSided = inProperties.m_isDoubleSided.getValue(key);
        if (isDoubleSided)
            perTargetCompilation = true;
    }

    QByteArray materialInfoString; // also serves as the key for the cache in compileGeneratedRhiShader
    // inShaderKeyPrefix can be a static string for default materials, but must
    // be unique for different sets of shaders in custom materials.
    materialInfoString = inShaderKeyPrefix;
    key.toString(materialInfoString, inProperties);

    // the call order is: beginVertex, beginFragment, endVertex, endFragment
    vertexPipeline.beginVertexGeneration(key, inFeatureSet, shaderLibraryManager);
    generateFragmentShader(vertexPipeline.fragment(), vertexPipeline, key, inProperties, inFeatureSet, inMaterial, shaderAugmentation, shaderLibraryManager);
    vertexPipeline.endVertexGeneration();
    vertexPipeline.endFragmentGeneration();

    return vertexPipeline.programGenerator()->compileGeneratedRhiShader(materialInfoString,
                                                                        inFeatureSet,
                                                                        shaderLibraryManager,
                                                                        theCache,
                                                                        {},
                                                                        shaderAugmentation,
                                                                        viewCount,
                                                                        perTargetCompilation);
}

static quint32 softShadowQualityToInt(QSSGRenderLight::SoftShadowQuality quality)
{
    quint32 samplesCount = 0;
    switch (quality) {
        case QSSGRenderLight::SoftShadowQuality::Hard:
            samplesCount = 0;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF4:
            samplesCount = 4;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF8:
            samplesCount = 8;
            break;
        case QSSGRenderLight::SoftShadowQuality::PCF16:
        case QSSGRenderLight::SoftShadowQuality::PCF32:
        case QSSGRenderLight::SoftShadowQuality::PCF64:
            samplesCount = 16;
            break;
    }

    return samplesCount;
}

void QSSGMaterialShaderGenerator::setRhiMaterialProperties(const QSSGRenderContextInterface &renderContext,
                                                           QSSGRhiShaderPipeline &shaders,
                                                           char *ubufData,
                                                           QSSGRhiGraphicsPipelineState *inPipelineState,
                                                           const QSSGRenderGraphObject &inMaterial,
                                                           const QSSGShaderDefaultMaterialKey &inKey,
                                                           const QSSGShaderDefaultMaterialKeyProperties &inProperties,
                                                           const QSSGRenderCameraList &inCameras,
                                                           const QSSGRenderMvpArray &inModelViewProjections,
                                                           const QMatrix3x3 &inNormalMatrix,
                                                           const QMatrix4x4 &inGlobalTransform,
                                                           const QMatrix4x4 &clipSpaceCorrMatrix,
                                                           const QMatrix4x4 &localInstanceTransform,
                                                           const QMatrix4x4 &globalInstanceTransform,
                                                           const QSSGDataView<float> &inMorphWeights,
                                                           QSSGRenderableImage *inFirstImage,
                                                           float inOpacity,
                                                           const QSSGLayerRenderData &inRenderProperties,
                                                           const QSSGShaderLightListView &inLights,
                                                           const QSSGShaderReflectionProbe &reflectionProbe,
                                                           bool receivesShadows,
                                                           bool receivesReflections,
                                                           const QVector2D *shadowDepthAdjust,
                                                           QRhiTexture *lightmapTexture)
{
    QSSGShaderMaterialAdapter *materialAdapter = getMaterialAdapter(inMaterial);
    QSSGRhiShaderPipeline::CommonUniformIndices &cui = shaders.commonUniformIndices;

    materialAdapter->setCustomPropertyUniforms(ubufData, shaders, renderContext);

    const QVector2D camProperties(inCameras[0]->clipPlanes);
    shaders.setUniform(ubufData, "qt_cameraProperties", &camProperties, 2 * sizeof(float), &cui.cameraPropertiesIdx);

    const int viewCount = inCameras.count();

    // Pull the camera transforms from the render data once.
    QMatrix4x4 camGlobalTransforms[2] { QMatrix4x4{Qt::Uninitialized}, QMatrix4x4{Qt::Uninitialized} };
    if (viewCount < 2) {
        camGlobalTransforms[0] = inRenderProperties.getGlobalTransform(*inCameras[0]);
    } else {
        for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex)
            camGlobalTransforms[viewIndex] = inRenderProperties.getGlobalTransform(*inCameras[viewIndex]);
    }

    if (viewCount < 2) {
        const QMatrix4x4 &camGlobalTransform = camGlobalTransforms[0];
        const QVector3D camGlobalPos = QSSGRenderNode::getGlobalPos(camGlobalTransform);
        shaders.setUniform(ubufData, "qt_cameraPosition", &camGlobalPos, 3 * sizeof(float), &cui.cameraPositionIdx);
        const QVector3D camDirection = QSSG_GUARD(inRenderProperties.renderedCameraData.has_value())
                                                  ? inRenderProperties.renderedCameraData.value()[0].direction
                                                  : QVector3D{ 0.0f, 0.0f, -1.0f };
        shaders.setUniform(ubufData, "qt_cameraDirection", &camDirection, 3 * sizeof(float), &cui.cameraDirectionIdx);
    } else {
        QVarLengthArray<QVector3D, 2> camGlobalPos(viewCount);
        QVarLengthArray<QVector3D> camDirection(viewCount);
        for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex) {
            const QMatrix4x4 &camGlobalTransform = camGlobalTransforms[viewIndex];
            camGlobalPos[viewIndex] = QSSGRenderNode::getGlobalPos(camGlobalTransform);
            camDirection[viewIndex] = QSSG_GUARD(inRenderProperties.renderedCameraData.has_value())
                                                ? inRenderProperties.renderedCameraData.value()[viewIndex].direction
                                                : QVector3D{ 0.0f, 0.0f, -1.0f };
        }
        shaders.setUniformArray(ubufData, "qt_cameraPosition", camGlobalPos.constData(), viewCount, QSSGRenderShaderValue::Vec3, &cui.cameraPositionIdx);
        shaders.setUniformArray(ubufData, "qt_cameraDirection", camDirection.constData(), viewCount, QSSGRenderShaderValue::Vec3, &cui.cameraDirectionIdx);
    }

    const auto globalRenderData = QSSGLayerRenderData::globalRenderProperties(renderContext);

    // Only calculate and update Matrix uniforms if they are needed
    bool usesProjectionMatrix = false;
    bool usesInvProjectionMatrix = false;
    bool usesViewProjectionMatrix = false;
    bool usesModelViewProjectionMatrix = false;
    bool usesNormalMatrix = false;
    bool usesParentMatrix = false;

    if (inMaterial.type == QSSGRenderGraphObject::Type::CustomMaterial) {
        const auto *customMaterial = static_cast<const QSSGRenderCustomMaterial *>(&inMaterial);
        usesProjectionMatrix = customMaterial->m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::ProjectionMatrix);
        usesInvProjectionMatrix = customMaterial->m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::InverseProjectionMatrix);
        // ### these should use flags like the above two
        usesViewProjectionMatrix = true;
    }

    const bool usesInstancing = inProperties.m_usesInstancing.getValue(inKey);
    if (usesInstancing) {
        // Instanced calls have to calculate MVP and normalMatrix in the vertex shader
        usesViewProjectionMatrix = true;
        usesParentMatrix = true;
    } else {
        usesModelViewProjectionMatrix = true;
        usesNormalMatrix = true;
    }

    if (materialAdapter->isTransmissionEnabled())
        usesViewProjectionMatrix = true;

    // Update matrix uniforms
    if (usesProjectionMatrix || usesInvProjectionMatrix) {
        if (viewCount < 2) {
            const QMatrix4x4 projection = clipSpaceCorrMatrix * inCameras[0]->projection;
            if (usesProjectionMatrix)
                shaders.setUniform(ubufData, "qt_projectionMatrix", projection.constData(), 16 * sizeof(float), &cui.projectionMatrixIdx);
            if (usesInvProjectionMatrix)
                shaders.setUniform(ubufData, "qt_inverseProjectionMatrix", projection.inverted().constData(), 16 * sizeof (float), &cui.inverseProjectionMatrixIdx);
        } else {
            QVarLengthArray<QMatrix4x4, 2> projections(viewCount);
            QVarLengthArray<QMatrix4x4, 2> invertedProjections(viewCount);
            for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex) {
                projections[viewIndex] = clipSpaceCorrMatrix * inCameras[viewIndex]->projection;
                if (usesInvProjectionMatrix)
                    invertedProjections[viewIndex] = projections[viewIndex].inverted();
            }
            if (usesProjectionMatrix)
                shaders.setUniformArray(ubufData, "qt_projectionMatrix", projections.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.projectionMatrixIdx);
            if (usesInvProjectionMatrix)
                shaders.setUniformArray(ubufData, "qt_inverseProjectionMatrix", invertedProjections.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.inverseProjectionMatrixIdx);
        }
    }

    if (viewCount < 2) {
        const QMatrix4x4 viewMatrix = camGlobalTransforms[0].inverted();
        shaders.setUniform(ubufData, "qt_viewMatrix", viewMatrix.constData(), 16 * sizeof(float), &cui.viewMatrixIdx);
    } else {
        QVarLengthArray<QMatrix4x4, 2> viewMatrices(viewCount);
        for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex)
            viewMatrices[viewIndex] = camGlobalTransforms[viewIndex].inverted();
        shaders.setUniformArray(ubufData, "qt_viewMatrix", viewMatrices.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.viewMatrixIdx);
    }

    if (usesViewProjectionMatrix) {
        if (viewCount < 2) {
            const QMatrix4x4 &camGlobalTransform = camGlobalTransforms[0];
            QMatrix4x4 viewProj(Qt::Uninitialized);
            inCameras[0]->calculateViewProjectionMatrix(camGlobalTransform, viewProj);
            viewProj = clipSpaceCorrMatrix * viewProj;
            shaders.setUniform(ubufData, "qt_viewProjectionMatrix", viewProj.constData(), 16 * sizeof(float), &cui.viewProjectionMatrixIdx);
        } else {
            QVarLengthArray<QMatrix4x4, 2> viewProjections(viewCount);
            for (size_t viewIndex = 0; viewIndex != std::size(camGlobalTransforms); ++viewIndex) {
                const auto &camGlobalTransform = camGlobalTransforms[viewIndex];
                inCameras[viewIndex]->calculateViewProjectionMatrix(camGlobalTransform, viewProjections[viewIndex]);
                viewProjections[viewIndex] = clipSpaceCorrMatrix * viewProjections[viewIndex];
            }
            shaders.setUniformArray(ubufData, "qt_viewProjectionMatrix", viewProjections.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.viewProjectionMatrixIdx);
        }
    }

    // qt_modelMatrix is always available, but differnt when using instancing
    if (usesInstancing)
        shaders.setUniform(ubufData, "qt_modelMatrix", localInstanceTransform.constData(), 16 * sizeof(float), &cui.modelMatrixIdx);
    else
        shaders.setUniform(ubufData, "qt_modelMatrix", inGlobalTransform.constData(), 16 * sizeof(float), &cui.modelMatrixIdx);

    if (usesModelViewProjectionMatrix) {
        if (viewCount < 2) {
            QMatrix4x4 mvp { clipSpaceCorrMatrix };
            mvp *= inModelViewProjections[0];
            shaders.setUniform(ubufData, "qt_modelViewProjection", mvp.constData(), 16 * sizeof(float), &cui.modelViewProjectionIdx);
        } else {
            QVarLengthArray<QMatrix4x4, 2> mvps(viewCount);
            for (int viewIndex = 0; viewIndex < viewCount; ++viewIndex)
                mvps[viewIndex] = clipSpaceCorrMatrix * inModelViewProjections[viewIndex];
            shaders.setUniformArray(ubufData, "qt_modelViewProjection", mvps.constData(), viewCount, QSSGRenderShaderValue::Matrix4x4, &cui.modelViewProjectionIdx);
        }
    }
    if (usesNormalMatrix)
        shaders.setUniform(ubufData, "qt_normalMatrix", inNormalMatrix.constData(), 12 * sizeof(float), &cui.normalMatrixIdx,
                            QSSGRhiShaderPipeline::UniformFlag::Mat3); // real size will be 12 floats, setUniform repacks as needed
    if (usesParentMatrix)
        shaders.setUniform(ubufData, "qt_parentMatrix", globalInstanceTransform.constData(), 16 * sizeof(float));

    // Morphing
    const qsizetype morphSize = inProperties.m_targetCount.getValue(inKey);
    if (morphSize > 0) {
        if (inMorphWeights.mSize >= morphSize) {
            shaders.setUniformArray(ubufData, "qt_morphWeights", inMorphWeights.mData, morphSize,
                                     QSSGRenderShaderValue::Float, &cui.morphWeightsIdx);
        } else {
            const QList<float> zeroWeights(morphSize - inMorphWeights.mSize, 0.0f);
            QList<float> newWeights(inMorphWeights.mData, inMorphWeights.mData + inMorphWeights.mSize);
            newWeights.append(zeroWeights);
            shaders.setUniformArray(ubufData, "qt_morphWeights", newWeights.constData(), morphSize,
                                     QSSGRenderShaderValue::Float, &cui.morphWeightsIdx);
        }
    }

    QVector3D theLightAmbientTotal;
    quint32 lightCount = 0;
    quint32 directionalLightCount = 0;
    quint32 shadowCount = 0;
    quint32 directionalShadowCount = 0;
    QSSGShaderLightsUniformData &lightsUniformData(shaders.lightsUniformData());
    QSSGShaderDirectionalLightsUniformData &directionalLightsUniformData(shaders.directionalLightsUniformData());

    for (quint32 lightIdx = 0, lightEnd = inLights.size();
         lightIdx < lightEnd && lightIdx < QSSG_MAX_NUM_LIGHTS; ++lightIdx)
    {
        QSSGRenderLight *theLight(inLights[lightIdx].light);

        // Gather Common Properties
        const bool lightShadows = inLights[lightIdx].shadows;
        const float brightness = theLight->m_brightness;
        quint32 lightmapState = 0;
        if (theLight->m_bakingEnabled) {
            if (theLight->m_fullyBaked) {
                // Lightmap provides Indirect + Diffuse (we still need to provide specular)
                lightmapState = 2;
            } else {
                // Lightmap provides Indirect (we still provide direct diffuse + specular)
                lightmapState = 1;
            }
        }

        const QVector3D diffuseColor(theLight->m_diffuseColor.x() * brightness,
                                     theLight->m_diffuseColor.y() * brightness,
                                     theLight->m_diffuseColor.z() * brightness);
        const QVector3D specularColor(theLight->m_specularColor.x() * brightness,
                                      theLight->m_specularColor.y() * brightness,
                                      theLight->m_specularColor.z() * brightness);
        const QVector3D direction(inLights[lightIdx].direction);


        if (theLight->type == QSSGRenderGraphObject::Type::DirectionalLight) {
            // Directional Light
            QSSGShaderDirectionalLightData &lightData(directionalLightsUniformData.directionalLightData[directionalLightCount]);
            lightData.direction[0] = direction.x();
            lightData.direction[1] = direction.y();
            lightData.direction[2] = direction.z();
            lightData.diffuseColor[0] = diffuseColor.x();
            lightData.diffuseColor[1] = diffuseColor.y();
            lightData.diffuseColor[2] = diffuseColor.z();
            lightData.specularColor[0] = specularColor.x();
            lightData.specularColor[1] = specularColor.y();
            lightData.specularColor[2] = specularColor.z();
            lightData.lightmapState = lightmapState;
            if (lightShadows && receivesShadows) {
                lightData.enableShadows = 1.0f;
                QSSGShadowMapEntry *pEntry = inRenderProperties.getShadowMapManager()->shadowMapEntry(lightIdx);
                Q_ASSERT(pEntry);

                const quint32 layerCount = pEntry->m_csmNumSplits + 1;

                for (quint32 i = 0; i < layerCount; ++i)
                    memcpy(lightData.matrices[i], pEntry->m_fixedScaleBiasMatrix[i].constData(), 16 * sizeof(float));

                lightData.shadowBias = theLight->m_shadowBias;
                // If all cascades are inactive, disable shadows for this light, as there are no casters.
                const bool noCascades = !(pEntry->m_csmActive[0] || pEntry->m_csmActive[1] || pEntry->m_csmActive[2] || pEntry->m_csmActive[3]);
                if (theLight->type == QSSGRenderLight::Type::DirectionalLight && noCascades)
                    lightData.enableShadows = 0.0f;
                lightData.shadowFactor = theLight->m_shadowFactor;
                lightData.shadowMapFar = theLight->m_shadowMapFar;
                lightData.shadowPcfSamples = softShadowQualityToInt(theLight->m_softShadowQuality);
                lightData.shadowPcfFactor = theLight->m_pcfFactor;

                for (quint32 i = 0; i < layerCount; ++i) {
                    const auto &atlasInfo = pEntry->m_atlasInfo[i];
                    lightData.atlasLocations[i][0] = atlasInfo.uOffset;
                    lightData.atlasLocations[i][1] = atlasInfo.vOffset;
                    lightData.atlasLocations[i][2] = atlasInfo.uvScale;
                    lightData.atlasLocations[i][3] = atlasInfo.layerIndex;
                }

                lightData.csmNumSplits = pEntry->m_csmNumSplits;
                memcpy(lightData.csmSplits, pEntry->m_csmSplits, 4 * sizeof(float));
                memcpy(lightData.csmActive, pEntry->m_csmActive, 4 * sizeof(float));
                lightData.csmBlendRatio = theLight->m_csmBlendRatio;
                for (quint32 i = 0; i < layerCount; ++i)
                    memcpy(lightData.dimensionsInverted[i], &pEntry->m_dimensionsInverted[i], 4 * sizeof(float));

                directionalShadowCount++;
            } else {
                lightData.enableShadows = 0.0f;
            }
            directionalLightCount++;
        } else {
            // Point or Spot Light
            QSSGShaderLightData &lightData(lightsUniformData.lightData[lightCount]);
            const auto gt = inRenderProperties.getGlobalTransform(*theLight);
            const QVector3D globalPos = QSSGRenderNode::getGlobalPos(gt);
            lightData.position[0] = globalPos.x();
            lightData.position[1] = globalPos.y();
            lightData.position[2] = globalPos.z();
            lightData.constantAttenuation = QSSGUtils::aux::translateConstantAttenuation(theLight->m_constantFade);
            lightData.linearAttenuation = QSSGUtils::aux::translateLinearAttenuation(theLight->m_linearFade);
            lightData.quadraticAttenuation = QSSGUtils::aux::translateQuadraticAttenuation(theLight->m_quadraticFade);
            lightData.coneAngle = 360.0f;
            lightData.direction[0] = direction.x();
            lightData.direction[1] = direction.y();
            lightData.direction[2] = direction.z();
            lightData.diffuseColor[0] = diffuseColor.x();
            lightData.diffuseColor[1] = diffuseColor.y();
            lightData.diffuseColor[2] = diffuseColor.z();
            lightData.specularColor[0] = specularColor.x();
            lightData.specularColor[1] = specularColor.y();
            lightData.specularColor[2] = specularColor.z();
            lightData.lightmapState = lightmapState;
            if (theLight->type == QSSGRenderLight::Type::SpotLight) {
                // NB: This is how we tell in the shader that this is a spot light
                // PointLights have a coneAngle of 360.0f which is greater than 1.0
                // Since cos(any value) will be between -1 and 1, we can use this.
                const float coneAngle = theLight->m_coneAngle;
                const float innerConeAngle = (theLight->m_innerConeAngle > coneAngle) ? coneAngle : theLight->m_innerConeAngle;
                lightData.coneAngle = qCos(qDegreesToRadians(coneAngle));
                lightData.innerConeAngle = qCos(qDegreesToRadians(innerConeAngle));
            }

            if (lightShadows && receivesShadows) {
                QSSGShadowMapEntry *pEntry = inRenderProperties.getShadowMapManager()->shadowMapEntry(lightIdx);
                Q_ASSERT(pEntry);
                lightData.enableShadows = 1.0f;
                lightData.shadowPcfFactor = theLight->m_pcfFactor;
                lightData.shadowPcfSamples = softShadowQualityToInt(theLight->m_softShadowQuality);
                lightData.shadowFactor = theLight->m_shadowFactor;
                lightData.shadowBias = theLight->m_shadowBias;
                lightData.shadowClipNear = 1.0f;
                lightData.shadowMapFar = pEntry->m_shadowMapFar;
                lightData.shadowTextureSize = pEntry->m_atlasInfo[0].uvScale;

                if (theLight->type == QSSGRenderLight::Type::SpotLight) {
                    // add fixed scale bias matrix
                    static const QMatrix4x4 bias = {
                        0.5, 0.0, 0.0, 0.5,
                        0.0, 0.5, 0.0, 0.5,
                        0.0, 0.0, 0.5, 0.5,
                        0.0, 0.0, 0.0, 1.0 };
                    const QMatrix4x4 m = bias * pEntry->m_lightViewProjection[0];
                    memcpy(lightData.shadowMatrix, m.constData(), 16 * sizeof(float));
                    lightData.shadowAtlasUV0[0] = pEntry->m_atlasInfo[0].uOffset;
                    lightData.shadowAtlasUV0[1] = pEntry->m_atlasInfo[0].vOffset;
                    lightData.shadowAtlasLayer0 = pEntry->m_atlasInfo[0].layerIndex;

                } else {
                    Q_ASSERT(theLight->type == QSSGRenderLight::Type::PointLight);
                    memcpy(lightData.shadowMatrix, pEntry->m_lightView.constData(), 16 * sizeof(float));
                    lightData.shadowAtlasUV0[0] = pEntry->m_atlasInfo[0].uOffset;
                    lightData.shadowAtlasUV0[1] = pEntry->m_atlasInfo[0].vOffset;
                    lightData.shadowAtlasLayer0 = pEntry->m_atlasInfo[0].layerIndex;
                    lightData.shadowAtlasUV1[0] = pEntry->m_atlasInfo[1].uOffset;
                    lightData.shadowAtlasUV1[1] = pEntry->m_atlasInfo[1].vOffset;
                    lightData.shadowAtlasLayer1 = pEntry->m_atlasInfo[1].layerIndex;
                }
                shadowCount++;

            } else {
                lightData.enableShadows = 0.0f;
            }

            lightCount++;
        }

        theLightAmbientTotal += theLight->m_ambientColor;
    }

    // Shadow Map Atlas Texture (if needed)
    if (shadowCount > 0 || directionalShadowCount > 0) {
        shaders.setShadowMapAtlasTexture(inRenderProperties.getShadowMapManager()->shadowMapAtlasTexture());
        shaders.setShadowMapBlueNoiseTexture(inRenderProperties.getShadowMapManager()->shadowMapBlueNoiseTexture());
    } else {
        shaders.setShadowMapAtlasTexture(nullptr);
        shaders.setShadowMapBlueNoiseTexture(nullptr);
    }

    const QSSGRhiRenderableTexture *depthTexture = inRenderProperties.getRenderResult(QSSGRenderResult::Key::DepthTexture);
    const QSSGRhiRenderableTexture *normalTexture = inRenderProperties.getRenderResult(QSSGRenderResult::Key::NormalTexture);
    const QSSGRhiRenderableTexture *ssaoTexture = inRenderProperties.getRenderResult(QSSGRenderResult::Key::AoTexture);
    const QSSGRhiRenderableTexture *screenTexture = inRenderProperties.getRenderResult(QSSGRenderResult::Key::ScreenTexture);
    const QSSGRhiRenderableTexture *motionVectorTexture = inRenderProperties.getRenderResult(QSSGRenderResult::Key::MotionVectorTexture);

    shaders.setDepthTexture(depthTexture->texture);
    shaders.setNormalTexture(normalTexture->texture);
    shaders.setSsaoTexture(ssaoTexture->texture);
    shaders.setScreenTexture(screenTexture->texture);
    shaders.setLightmapTexture(lightmapTexture);
    shaders.setMotionVectorTexture(motionVectorTexture->texture);

#ifdef QSSG_OIT_USE_BUFFERS
    shaders.setOITImages((QRhiTexture*)inRenderProperties.getOitRenderContextConst().aBuffer,
                         (QRhiTexture*)inRenderProperties.getOitRenderContextConst().auxBuffer,
                         (QRhiTexture*)inRenderProperties.getOitRenderContextConst().counterBuffer);
    if (inRenderProperties.getOitRenderContextConst().aBuffer) {
#else
    const QSSGRhiRenderableTexture *abuf = inRenderProperties.getRenderResult(QSSGRenderResult::Key::ABufferImage);
    const QSSGRhiRenderableTexture *aux = inRenderProperties.getRenderResult(QSSGRenderResult::Key::AuxiliaryImage);
    const QSSGRhiRenderableTexture *counter = inRenderProperties.getRenderResult(QSSGRenderResult::Key::CounterImage);
    shaders.setOITImages(abuf->texture, aux->texture, counter->texture);
    if (abuf->texture) {
#endif
        int abufWidth = RenderHelpers::rhiCalculateABufferSize(inRenderProperties.layer.oitNodeCount);
        int listNodeCount = abufWidth * abufWidth;
        shaders.setUniform(ubufData, "qt_ABufImageWidth", &abufWidth, sizeof(int), &cui.abufImageWidth);
        shaders.setUniform(ubufData, "qt_listNodeCount", &listNodeCount, sizeof(int), &cui.listNodeCount);
        int viewSize[2] = {inRenderProperties.layerPrepResult.textureDimensions().width(), inRenderProperties.layerPrepResult.textureDimensions().height()};
        shaders.setUniform(ubufData, "qt_viewSize", viewSize, sizeof(int) * 2, &cui.viewSize);
        int samples = inPipelineState->samples;
        shaders.setUniform(ubufData, "qt_samples", &samples, sizeof(int), &cui.samples);
    }

    const QSSGRenderLayer &layer = QSSGLayerRenderData::getCurrent(*renderContext.renderer())->layer;
    QSSGRenderImage *theLightProbe = layer.lightProbe;
    const auto &lightProbeData = layer.lightProbeSettings;

    // If the material has its own IBL Override, we should use that image instead.
    QSSGRenderImage *materialIblProbe = materialAdapter->iblProbe();
    if (materialIblProbe)
        theLightProbe = materialIblProbe;
    QSSGRenderImageTexture lightProbeTexture;
    if (theLightProbe)
        lightProbeTexture = renderContext.bufferManager()->loadRenderImage(theLightProbe, QSSGBufferManager::MipModeBsdf);
    if (theLightProbe && lightProbeTexture.m_texture) {
        QSSGRenderTextureCoordOp theHorzLightProbeTilingMode = theLightProbe->m_horizontalTilingMode;
        QSSGRenderTextureCoordOp theVertLightProbeTilingMode = theLightProbe->m_verticalTilingMode;
        const int maxMipLevel = lightProbeTexture.m_mipmapCount - 1;

        if (!materialIblProbe && !lightProbeData.probeOrientation.isIdentity()) {
            shaders.setUniform(ubufData, "qt_lightProbeOrientation",
                                lightProbeData.probeOrientation.constData(),
                                12 * sizeof(float), &cui.lightProbeOrientationIdx,
                                QSSGRhiShaderPipeline::UniformFlag::Mat3);
        }

        const float props[4] = { 0.0f, float(maxMipLevel), lightProbeData.probeHorizon, lightProbeData.probeExposure };
        shaders.setUniform(ubufData, "qt_lightProbeProperties", props, 4 * sizeof(float), &cui.lightProbePropertiesIdx);

        shaders.setLightProbeTexture(lightProbeTexture.m_texture, theHorzLightProbeTilingMode, theVertLightProbeTilingMode);
    } else {
        // no lightprobe
        const float emptyProps[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
        shaders.setUniform(ubufData, "qt_lightProbeProperties", emptyProps, 4 * sizeof(float), &cui.lightProbePropertiesIdx);

        shaders.setLightProbeTexture(nullptr);
    }

    if (receivesReflections && reflectionProbe.enabled) {
        shaders.setUniform(ubufData, "qt_reflectionProbeCubeMapCenter", &reflectionProbe.probeCubeMapCenter, 3 * sizeof(float), &cui.reflectionProbeCubeMapCenter);
        shaders.setUniform(ubufData, "qt_reflectionProbeBoxMin", &reflectionProbe.probeBoxMin, 3 * sizeof(float), &cui.reflectionProbeBoxMin);
        shaders.setUniform(ubufData, "qt_reflectionProbeBoxMax", &reflectionProbe.probeBoxMax, 3 * sizeof(float), &cui.reflectionProbeBoxMax);
        shaders.setUniform(ubufData, "qt_reflectionProbeCorrection", &reflectionProbe.parallaxCorrection, sizeof(int), &cui.reflectionProbeCorrection);
    }

    const QVector3D emissiveColor = materialAdapter->emissiveColor();
    shaders.setUniform(ubufData, "qt_material_emissive_color", &emissiveColor, 3 * sizeof(float), &cui.material_emissiveColorIdx);

    const auto qMix = [](float x, float y, float a) {
        return (x * (1.0f - a) + (y * a));
    };

    const auto qMix3 = [&qMix](const QVector3D &x, const QVector3D &y, float a) {
        return QVector3D{qMix(x.x(), y.x(), a), qMix(x.y(), y.y(), a), qMix(x.z(), y.z(), a)};
    };

    const QVector4D color = materialAdapter->color();
    const QVector3D materialSpecularTint = materialAdapter->specularTint();
    const QVector3D specularTint = materialAdapter->isPrincipled() ? qMix3(QVector3D(1.0f, 1.0f, 1.0f), color.toVector3D(), materialSpecularTint.x())
                                                                   : materialSpecularTint;
    shaders.setUniform(ubufData, "qt_material_base_color", &color, 4 * sizeof(float), &cui.material_baseColorIdx);

    const float ior = materialAdapter->ior();
    QVector4D specularColor(specularTint, ior);
    shaders.setUniform(ubufData, "qt_material_specular", &specularColor, 4 * sizeof(float), &cui.material_specularIdx);

    const bool hasLighting = materialAdapter->hasLighting();
    shaders.setLightsEnabled(hasLighting);
    if (hasLighting) {
        const float lightAndShadowCounts[4] = {
            float(lightCount),
            float(directionalLightCount),
            float(shadowCount),
            float(directionalShadowCount)
        };
        shaders.setUniform(ubufData, "qt_lightAndShadowCounts", &lightAndShadowCounts, 4 * sizeof(float), &cui.lightAndShadowCountsIdx);

        const size_t lightDataSize = lightCount * sizeof(QSSGShaderLightData);
        const size_t directionalLightDataSize = directionalLightCount * sizeof(QSSGShaderDirectionalLightData);

        memcpy(ubufData + shaders.ub0LightDataOffset(), &lightsUniformData, lightDataSize);
        memcpy(ubufData + shaders.ub0DirectionalLightDataOffset(), &directionalLightsUniformData, directionalLightDataSize);
    }

    shaders.setUniform(ubufData, "qt_light_ambient_total", &theLightAmbientTotal, 3 * sizeof(float), &cui.light_ambient_totalIdx);

    const float materialProperties[4] = {
        materialAdapter->specularAmount(),
        materialAdapter->specularRoughness(),
        materialAdapter->metalnessAmount(),
        inOpacity
    };
    shaders.setUniform(ubufData, "qt_material_properties", materialProperties, 4 * sizeof(float), &cui.material_propertiesIdx);

    const float materialProperties2[4] = {
        materialAdapter->fresnelPower(),
        materialAdapter->bumpAmount(),
        materialAdapter->translucentFallOff(),
        materialAdapter->diffuseLightWrap()
    };
    shaders.setUniform(ubufData, "qt_material_properties2", materialProperties2, 4 * sizeof(float), &cui.material_properties2Idx);

    const float materialProperties3[4] = {
        materialAdapter->occlusionAmount(),
        materialAdapter->alphaCutOff(),
        materialAdapter->clearcoatAmount(),
        materialAdapter->clearcoatRoughnessAmount()
    };
    shaders.setUniform(ubufData, "qt_material_properties3", materialProperties3, 4 * sizeof(float), &cui.material_properties3Idx);

    const float materialProperties4[4] = {
        materialAdapter->heightAmount(),
        materialAdapter->minHeightSamples(),
        materialAdapter->maxHeightSamples(),
        materialAdapter->transmissionFactor()
    };
    shaders.setUniform(ubufData, "qt_material_properties4", materialProperties4, 4 * sizeof(float), &cui.material_properties4Idx);

    const bool hasCustomFrag = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment);
    if (!hasCustomFrag) {
        if (inProperties.m_fresnelScaleBiasEnabled.getValue(inKey) || inProperties.m_clearcoatFresnelScaleBiasEnabled.getValue(inKey)) {
            const float materialProperties5[4] = {
                materialAdapter->fresnelScale(),
                materialAdapter->fresnelBias(),
                materialAdapter->clearcoatFresnelScale(),
                materialAdapter->clearcoatFresnelBias()
            };
            shaders.setUniform(ubufData, "qt_material_properties5", materialProperties5, 4 * sizeof(float), &cui.material_properties5Idx);
        }

        const float material_clearcoat_normal_strength = materialAdapter->clearcoatNormalStrength();
        shaders.setUniform(ubufData, "qt_material_clearcoat_normal_strength", &material_clearcoat_normal_strength, sizeof(float), &cui.clearcoatNormalStrengthIdx);

        const float material_clearcoat_fresnel_power = materialAdapter->clearcoatFresnelPower();
        shaders.setUniform(ubufData, "qt_material_clearcoat_fresnel_power", &material_clearcoat_fresnel_power, sizeof(float), &cui.clearcoatFresnelPowerIdx);
        // We only ever use attenuation and thickness uniforms when using transmission
        if (materialAdapter->isTransmissionEnabled()) {
            const QVector4D attenuationProperties(materialAdapter->attenuationColor(), materialAdapter->attenuationDistance());
            shaders.setUniform(ubufData, "qt_material_attenuation", &attenuationProperties, 4 * sizeof(float), &cui.material_attenuationIdx);

            const float thickness = materialAdapter->thicknessFactor();
            shaders.setUniform(ubufData, "qt_material_thickness", &thickness, sizeof(float), &cui.thicknessFactorIdx);
        }
    }

    const float rhiProperties[4] = {
        globalRenderData.isYUpInFramebuffer ? 1.0f : -1.0f,
        globalRenderData.isYUpInNDC ? 1.0f : -1.0f,
        globalRenderData.isClipDepthZeroToOne ? 0.0f : -1.0f,
        0.0f // unused
    };
    shaders.setUniform(ubufData, "qt_rhi_properties", rhiProperties, 4 * sizeof(float), &cui.rhiPropertiesIdx);

    qsizetype imageIdx = 0;
    for (QSSGRenderableImage *theImage = inFirstImage; theImage; theImage = theImage->m_nextImage, ++imageIdx) {
        // we need to map image to uniform name: "image0_rotations", "image0_offsets", etc...
        const auto &names = imageStringTable[int(theImage->m_mapType)];
        if (imageIdx == cui.imageIndices.size())
            cui.imageIndices.append(QSSGRhiShaderPipeline::CommonUniformIndices::ImageIndices());
        auto &indices = cui.imageIndices[imageIdx];

        const QMatrix4x4 &textureTransform = theImage->m_imageNode.m_textureTransform;
        // We separate rotational information from offset information so that just maybe the shader
        // will attempt to push less information to the card.
        const float *dataPtr(textureTransform.constData());
        // The third member of the offsets contains a flag indicating if the texture was
        // premultiplied or not.
        // We use this to mix the texture alpha.
        const float offsets[3] = { dataPtr[12], dataPtr[13], 0.0f /* non-premultiplied */ };
        shaders.setUniform(ubufData, names.imageOffsets, offsets, sizeof(offsets), &indices.imageOffsetsUniformIndex);
        // Grab just the upper 2x2 rotation matrix from the larger matrix.
        const float rotations[4] = { dataPtr[0], dataPtr[4], dataPtr[1], dataPtr[5] };
        shaders.setUniform(ubufData, names.imageRotations, rotations, sizeof(rotations), &indices.imageRotationsUniformIndex);
    }

    if (shadowDepthAdjust)
        shaders.setUniform(ubufData, "qt_shadowDepthAdjust", shadowDepthAdjust, 2 * sizeof(float), &cui.shadowDepthAdjustIdx);

    const bool usesPointsTopology = inProperties.m_usesPointsTopology.getValue(inKey);
    if (usesPointsTopology) {
        const float pointSize = materialAdapter->pointSize();
        shaders.setUniform(ubufData, "qt_materialPointSize", &pointSize, sizeof(float), &cui.pointSizeIdx);
    }

    // qt_fogColor = (fogColor.x, fogColor.y, fogColor.z, fogDensity)
    // qt_fogDepthProperties = (fogDepthBegin, fogDepthEnd, fogDepthCurve, fogDepthEnabled ? 1.0 : 0.0)
    // qt_fogHeightProperties = (fogHeightMin, fogHeightMax, fogHeightCurve, fogHeightEnabled ? 1.0 : 0.0)
    // qt_fogTransmitProperties = (fogTransmitCurve, 0.0, 0.0, fogTransmitEnabled ? 1.0 : 0.0)
    if (inRenderProperties.layer.fog.enabled) {
        const float fogColor[4] = {
            inRenderProperties.layer.fog.color.x(),
            inRenderProperties.layer.fog.color.y(),
            inRenderProperties.layer.fog.color.z(),
            inRenderProperties.layer.fog.density
        };
        shaders.setUniform(ubufData, "qt_fogColor", fogColor, 4 * sizeof(float), &cui.fogColorIdx);
        const float fogDepthProperties[4] = {
            inRenderProperties.layer.fog.depthBegin,
            inRenderProperties.layer.fog.depthEnd,
            inRenderProperties.layer.fog.depthCurve,
            inRenderProperties.layer.fog.depthEnabled ? 1.0f : 0.0f
        };
        shaders.setUniform(ubufData, "qt_fogDepthProperties", fogDepthProperties, 4 * sizeof(float), &cui.fogDepthPropertiesIdx);
        const float fogHeightProperties[4] = {
            inRenderProperties.layer.fog.heightMin,
            inRenderProperties.layer.fog.heightMax,
            inRenderProperties.layer.fog.heightCurve,
            inRenderProperties.layer.fog.heightEnabled ? 1.0f : 0.0f
        };
        shaders.setUniform(ubufData, "qt_fogHeightProperties", fogHeightProperties, 4 * sizeof(float), &cui.fogHeightPropertiesIdx);
        const float fogTransmitProperties[4] = {
            inRenderProperties.layer.fog.transmitCurve,
            0.0f,
            0.0f,
            inRenderProperties.layer.fog.transmitEnabled ? 1.0f : 0.0f
        };
        shaders.setUniform(ubufData, "qt_fogTransmitProperties", fogTransmitProperties, 4 * sizeof(float), &cui.fogTransmitPropertiesIdx);
    }

    inPipelineState->lineWidth = materialAdapter->lineWidth();
}

QT_END_NAMESPACE

QList<QByteArrayView> QtQuick3DEditorHelpers::CustomMaterial::reservedArgumentNames()
{
    return {std::begin(qssg_shader_arg_names), std::end(qssg_shader_arg_names) };;
}
