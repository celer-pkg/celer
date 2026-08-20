// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/* clang-format off */

#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>
#include "qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>

QT_BEGIN_NAMESPACE

QSSGShaderMaterialAdapter::~QSSGShaderMaterialAdapter() = default;

QSSGShaderMaterialAdapter *QSSGShaderMaterialAdapter::create(const QSSGRenderGraphObject &materialNode)
{
    switch (materialNode.type) {
    case QSSGRenderGraphObject::Type::DefaultMaterial:
    case QSSGRenderGraphObject::Type::PrincipledMaterial:
    case QSSGRenderGraphObject::Type::SpecularGlossyMaterial:
        return new QSSGShaderDefaultMaterialAdapter(static_cast<const QSSGRenderDefaultMaterial &>(materialNode));

    case QSSGRenderGraphObject::Type::CustomMaterial:
        return new QSSGShaderCustomMaterialAdapter(static_cast<const QSSGRenderCustomMaterial &>(materialNode));

    default:
        break;
    }

    return nullptr;
}

bool QSSGShaderMaterialAdapter::isUnshaded()
{
    return false;
}

bool QSSGShaderMaterialAdapter::hasCustomShaderSnippet(QSSGShaderCache::ShaderType)
{
    return false;
}

QByteArray QSSGShaderMaterialAdapter::customShaderSnippet(QSSGShaderCache::ShaderType,
                                                          QSSGShaderLibraryManager &,
                                                          bool)
{
    return QByteArray();
}

bool QSSGShaderMaterialAdapter::hasCustomShaderFunction(QSSGShaderCache::ShaderType,
                                                        const QByteArray &,
                                                        QSSGShaderLibraryManager &)
{
    return false;
}

void QSSGShaderMaterialAdapter::setCustomPropertyUniforms(char *,
                                                          QSSGRhiShaderPipeline &,
                                                          const QSSGRenderContextInterface &)
{
}

bool QSSGShaderMaterialAdapter::usesSharedVariables()
{
    return false;
}



QSSGShaderDefaultMaterialAdapter::QSSGShaderDefaultMaterialAdapter(const QSSGRenderDefaultMaterial &material)
    : m_material(material)
{
}

bool QSSGShaderDefaultMaterialAdapter::isPrincipled()
{
    return m_material.type == QSSGRenderGraphObject::Type::PrincipledMaterial;
}

bool QSSGShaderDefaultMaterialAdapter::isSpecularGlossy()
{
    return m_material.type == QSSGRenderGraphObject::Type::SpecularGlossyMaterial;
}

bool QSSGShaderDefaultMaterialAdapter::isMetalnessEnabled()
{
    return m_material.isMetalnessEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isSpecularEnabled()
{
    return m_material.isSpecularEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isVertexColorsEnabled()
{
    return m_material.isVertexColorsEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isVertexColorsMaskEnabled()
{
    return m_material.isVertexColorsMaskEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isInvertOpacityMapValue()
{
    return m_material.isInvertOpacityMapValue();
}

bool QSSGShaderDefaultMaterialAdapter::isBaseColorSingleChannelEnabled()
{
    return m_material.isBaseColorSingleChannelEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isSpecularAmountSingleChannelEnabled()
{
    return m_material.isSpecularAmountSingleChannelEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isEmissiveSingleChannelEnabled()
{
    return m_material.isEmissiveSingleChannelEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isClearcoatEnabled()
{
    return m_material.isClearcoatEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::isTransmissionEnabled()
{
    return m_material.isTransmissionEnabled();
}

bool QSSGShaderDefaultMaterialAdapter::hasLighting()
{
    return m_material.hasLighting();
}

bool QSSGShaderDefaultMaterialAdapter::usesCustomSkinning()
{
    return false;
}

bool QSSGShaderDefaultMaterialAdapter::usesCustomMorphing()
{
    return false;
}

QSSGRenderDefaultMaterial::MaterialSpecularModel QSSGShaderDefaultMaterialAdapter::specularModel()
{
    return m_material.specularModel;
}

QSSGRenderDefaultMaterial::MaterialAlphaMode QSSGShaderDefaultMaterialAdapter::alphaMode()
{
    return m_material.alphaMode;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderDefaultMaterialAdapter::vertexColorRedMask()
{
    return m_material.vertexColorRedMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderDefaultMaterialAdapter::vertexColorGreenMask()
{
    return m_material.vertexColorGreenMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderDefaultMaterialAdapter::vertexColorBlueMask()
{
    return m_material.vertexColorBlueMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderDefaultMaterialAdapter::vertexColorAlphaMask()
{
    return m_material.vertexColorAlphaMask;
}

QSSGRenderImage *QSSGShaderDefaultMaterialAdapter::iblProbe()
{
    return m_material.iblProbe;
}

QVector3D QSSGShaderDefaultMaterialAdapter::emissiveColor()
{
    return m_material.emissiveColor;
}

QVector4D QSSGShaderDefaultMaterialAdapter::color()
{
    return m_material.color;
}

QVector3D QSSGShaderDefaultMaterialAdapter::specularTint()
{
    return m_material.specularTint;
}

float QSSGShaderDefaultMaterialAdapter::ior()
{
    return m_material.ior;
}

bool QSSGShaderDefaultMaterialAdapter::isFresnelScaleBiasEnabled()
{
    return m_material.fresnelScaleBiasEnabled;
}

float QSSGShaderDefaultMaterialAdapter::fresnelScale()
{
    return m_material.fresnelScale;
}

float QSSGShaderDefaultMaterialAdapter::fresnelBias()
{
    return m_material.fresnelBias;
}

float QSSGShaderDefaultMaterialAdapter::fresnelPower()
{
    return m_material.fresnelPower;
}

bool QSSGShaderDefaultMaterialAdapter::isClearcoatFresnelScaleBiasEnabled()
{
    return m_material.clearcoatFresnelScaleBiasEnabled;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatFresnelScale()
{
    return m_material.clearcoatFresnelScale;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatFresnelBias()
{
    return m_material.clearcoatFresnelBias;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatFresnelPower()
{
    return m_material.clearcoatFresnelPower;
}

float QSSGShaderDefaultMaterialAdapter::metalnessAmount()
{
    return m_material.metalnessAmount;
}

float QSSGShaderDefaultMaterialAdapter::specularAmount()
{
    return m_material.specularAmount;
}

float QSSGShaderDefaultMaterialAdapter::specularRoughness()
{
    return m_material.specularRoughness;
}

float QSSGShaderDefaultMaterialAdapter::bumpAmount()
{
    return m_material.bumpAmount;
}

float QSSGShaderDefaultMaterialAdapter::translucentFallOff()
{
    return m_material.translucentFalloff;
}

float QSSGShaderDefaultMaterialAdapter::diffuseLightWrap()
{
    return m_material.diffuseLightWrap;
}

float QSSGShaderDefaultMaterialAdapter::occlusionAmount()
{
    return m_material.occlusionAmount;
}

float QSSGShaderDefaultMaterialAdapter::alphaCutOff()
{
    return m_material.alphaCutoff;
}

float QSSGShaderDefaultMaterialAdapter::pointSize()
{
    return m_material.pointSize;
}

float QSSGShaderDefaultMaterialAdapter::lineWidth()
{
    return m_material.lineWidth;
}

float QSSGShaderDefaultMaterialAdapter::heightAmount()
{
    return m_material.heightAmount;
}

float QSSGShaderDefaultMaterialAdapter::minHeightSamples()
{
    return m_material.minHeightSamples;
}

float QSSGShaderDefaultMaterialAdapter::maxHeightSamples()
{
    return m_material.maxHeightSamples;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatAmount()
{
    return m_material.clearcoatAmount;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatRoughnessAmount()
{
    return m_material.clearcoatRoughnessAmount;
}

float QSSGShaderDefaultMaterialAdapter::clearcoatNormalStrength()
{
    return m_material.clearcoatNormalStrength;
}

float QSSGShaderDefaultMaterialAdapter::transmissionFactor()
{
    return m_material.transmissionFactor;
}

float QSSGShaderDefaultMaterialAdapter::thicknessFactor()
{
    return m_material.thicknessFactor;
}

float QSSGShaderDefaultMaterialAdapter::attenuationDistance()
{
    return m_material.attenuationDistance;
}

QVector3D QSSGShaderDefaultMaterialAdapter::attenuationColor()
{
    return m_material.attenuationColor;
}

QSSGShaderCustomMaterialAdapter::QSSGShaderCustomMaterialAdapter(const QSSGRenderCustomMaterial &material)
    : m_material(material)
{
}

// Act like Principled. Lighting is always on, specular, metalness, etc. support should all be enabled.
// Unlike Principled, the *enabled values do not depend on the metalness or specularAmount values
// (we cannot tell what those are if they are written in the shader).

bool QSSGShaderCustomMaterialAdapter::isPrincipled()
{
    return true;
}

bool QSSGShaderCustomMaterialAdapter::isSpecularGlossy()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isMetalnessEnabled()
{
    return true;
}

bool QSSGShaderCustomMaterialAdapter::isSpecularEnabled()
{
    return true;
}

bool QSSGShaderCustomMaterialAdapter::isVertexColorsEnabled()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::VarColor);
}

bool QSSGShaderCustomMaterialAdapter::isVertexColorsMaskEnabled()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isInvertOpacityMapValue()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isBaseColorSingleChannelEnabled()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isSpecularAmountSingleChannelEnabled()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isEmissiveSingleChannelEnabled()
{
    return false;
}

bool QSSGShaderCustomMaterialAdapter::isClearcoatEnabled()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::Clearcoat);
}

bool QSSGShaderCustomMaterialAdapter::isTransmissionEnabled()
{
   return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::Transmission);
}

bool QSSGShaderCustomMaterialAdapter::hasLighting()
{
    return true;
}

bool QSSGShaderCustomMaterialAdapter::usesCustomSkinning()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::Skinning);
}

bool QSSGShaderCustomMaterialAdapter::usesCustomMorphing()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::Morphing);
}

QSSGRenderDefaultMaterial::MaterialSpecularModel QSSGShaderCustomMaterialAdapter::specularModel()
{
    return QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX;
}

QSSGRenderDefaultMaterial::MaterialAlphaMode QSSGShaderCustomMaterialAdapter::alphaMode()
{
    return QSSGRenderDefaultMaterial::MaterialAlphaMode::Default;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderCustomMaterialAdapter::vertexColorRedMask()
{
    return QSSGRenderDefaultMaterial::NoMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderCustomMaterialAdapter::vertexColorGreenMask()
{
    return QSSGRenderDefaultMaterial::NoMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderCustomMaterialAdapter::vertexColorBlueMask()
{
    return QSSGRenderDefaultMaterial::NoMask;
}

QSSGRenderDefaultMaterial::VertexColorMaskFlags QSSGShaderCustomMaterialAdapter::vertexColorAlphaMask()
{
    return QSSGRenderDefaultMaterial::NoMask;
}

QSSGRenderImage *QSSGShaderCustomMaterialAdapter::iblProbe()
{
    return m_material.m_iblProbe;
}

// The following are the values that get set into uniforms such as
// qt_material_properties etc. When a custom shader is present, these values
// are not used at all. However, a CustomMaterial is also valid without a
// vertex/fragment shader, or with no custom shaders at all. Therefore the
// values here must match the defaults of PrincipledMaterial, in order to make
// PrincipledMaterial { } and CustomMaterial { } identical.

QVector3D QSSGShaderCustomMaterialAdapter::emissiveColor()
{
    return QVector3D(0, 0, 0);
}

QVector4D QSSGShaderCustomMaterialAdapter::color()
{
    return QVector4D(1, 1, 1, 1);
}

QVector3D QSSGShaderCustomMaterialAdapter::specularTint()
{
    return QVector3D(1, 1, 1);
}

float QSSGShaderCustomMaterialAdapter::ior()
{
    return 1.45f;
}

bool QSSGShaderCustomMaterialAdapter::isFresnelScaleBiasEnabled()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::FresnelScaleBias);
}

float QSSGShaderCustomMaterialAdapter::fresnelScale()
{
    return 1.0f;
}

float QSSGShaderCustomMaterialAdapter::fresnelBias()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::fresnelPower()
{
    return 0.0f;
}

bool QSSGShaderCustomMaterialAdapter::isClearcoatFresnelScaleBiasEnabled()
{
    return m_material.m_renderFlags.testFlag(QSSGRenderCustomMaterial::RenderFlag::ClearcoatFresnelScaleBias);
}

float QSSGShaderCustomMaterialAdapter::clearcoatFresnelScale()
{
    return 1.0f;
}

float QSSGShaderCustomMaterialAdapter::clearcoatFresnelBias()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::clearcoatFresnelPower()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::metalnessAmount()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::specularAmount()
{
    return 0.5f;
}

float QSSGShaderCustomMaterialAdapter::specularRoughness()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::bumpAmount()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::translucentFallOff()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::diffuseLightWrap()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::occlusionAmount()
{
    return 1.0f;
}

float QSSGShaderCustomMaterialAdapter::alphaCutOff()
{
    return 0.5f;
}

float QSSGShaderCustomMaterialAdapter::pointSize()
{
    return 1.0f;
}

float QSSGShaderCustomMaterialAdapter::lineWidth()
{
    return m_material.m_lineWidth;
}

float QSSGShaderCustomMaterialAdapter::heightAmount()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::minHeightSamples()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::maxHeightSamples()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::clearcoatAmount()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::clearcoatRoughnessAmount()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::clearcoatNormalStrength()
{
    return 1.0f;
}

float QSSGShaderCustomMaterialAdapter::transmissionFactor()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::thicknessFactor()
{
    return 0.0f;
}

float QSSGShaderCustomMaterialAdapter::attenuationDistance()
{
    return std::numeric_limits<float>::infinity();
}

QVector3D QSSGShaderCustomMaterialAdapter::attenuationColor()
{
    return { 1.0f, 1.0f, 1.0f };
}

bool QSSGShaderCustomMaterialAdapter::isUnshaded()
{
    return m_material.m_shadingMode == QSSGRenderCustomMaterial::ShadingMode::Unshaded;
}

bool QSSGShaderCustomMaterialAdapter::hasCustomShaderSnippet(QSSGShaderCache::ShaderType type)
{
    if (type == QSSGShaderCache::ShaderType::Vertex)
        return m_material.m_customShaderPresence.testFlag(QSSGRenderCustomMaterial::CustomShaderPresenceFlag::Vertex);

    return m_material.m_customShaderPresence.testFlag(QSSGRenderCustomMaterial::CustomShaderPresenceFlag::Fragment);
}

QByteArray QSSGShaderCustomMaterialAdapter::customShaderSnippet(QSSGShaderCache::ShaderType type,
                                                                QSSGShaderLibraryManager &shaderLibraryManager,
                                                                bool multiViewCompatible)
{
    if (hasCustomShaderSnippet(type)) {
        const QByteArray shaderPathKey = m_material.m_shaderPathKey[multiViewCompatible ? QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex
                                                                                        : QSSGRenderCustomMaterial::RegularShaderPathKeyIndex];
        return shaderLibraryManager.getShaderSource(shaderPathKey, type);
    }

    return QByteArray();
}

bool QSSGShaderCustomMaterialAdapter::hasCustomShaderFunction(QSSGShaderCache::ShaderType shaderType,
                                                              const QByteArray &funcName,
                                                              QSSGShaderLibraryManager &shaderLibraryManager)
{
    if (hasCustomShaderSnippet(shaderType))
        return shaderLibraryManager.getShaderMetaData(m_material.m_shaderPathKey[QSSGRenderCustomMaterial::RegularShaderPathKeyIndex],
                                                      shaderType).customFunctions.contains(funcName);

    return false;
}

void QSSGShaderCustomMaterialAdapter::setCustomPropertyUniforms(char *ubufData,
                                                                QSSGRhiShaderPipeline &shaderPipeline,
                                                                const QSSGRenderContextInterface &context)
{
    context.customMaterialSystem()->applyRhiShaderPropertyValues(ubufData, m_material, shaderPipeline);
}

bool QSSGShaderCustomMaterialAdapter::usesSharedVariables()
{
    return m_material.m_usesSharedVariables;
}

namespace {

// Custom material shader substitution table.
// Must be in sync with the shader generator.
static const QSSGCustomMaterialVariableSubstitution qssg_var_subst_tab[] = {
    // uniform (block members)
    { "MODELVIEWPROJECTION_MATRIX", "qt_modelViewProjection", true },
    { "VIEWPROJECTION_MATRIX", "qt_viewProjectionMatrix", true },
    { "MODEL_MATRIX", "qt_modelMatrix", false },
    { "VIEW_MATRIX", "qt_viewMatrix", true },
    { "NORMAL_MATRIX", "qt_normalMatrix", false },
    { "BONE_TRANSFORMS", "qt_getTexMatrix", false },
    { "BONE_NORMAL_TRANSFORMS", "qt_getTexMatrix", false },
    { "PROJECTION_MATRIX", "qt_projectionMatrix", true },
    { "INVERSE_PROJECTION_MATRIX", "qt_inverseProjectionMatrix", true },
    { "CAMERA_POSITION", "qt_cameraPosition", true },
    { "CAMERA_DIRECTION", "qt_cameraDirection", true },
    { "CAMERA_PROPERTIES", "qt_cameraProperties", false },
    { "FRAMEBUFFER_Y_UP", "qt_rhi_properties.x", false },
    { "NDC_Y_UP", "qt_rhi_properties.y", false },
    { "NEAR_CLIP_VALUE", "qt_rhi_properties.z", false },
    { "IBL_MAXMIPMAP", "qt_lightProbeProperties.y", false },
    { "IBL_HORIZON", "qt_lightProbeProperties.z", false },
    { "IBL_EXPOSE", "qt_lightProbeProperties.w", false },

    // outputs
    { "POSITION", "gl_Position", false },
    { "FRAGCOLOR", "fragOutput", false },
    { "POINT_SIZE", "gl_PointSize", false },

    // fragment inputs
    { "FRAGCOORD", "gl_FragCoord", false },

    // functions
    { "DIRECTIONAL_LIGHT", "qt_directionalLightProcessor", false },
    { "POINT_LIGHT", "qt_pointLightProcessor", false },
    { "SPOT_LIGHT", "qt_spotLightProcessor", false },
    { "AMBIENT_LIGHT", "qt_ambientLightProcessor", false },
    { "SPECULAR_LIGHT", "qt_specularLightProcessor", false },
    { "MAIN", "qt_customMain", false },
    { "POST_PROCESS", "qt_customPostProcessor", false },
    { "IBL_PROBE", "qt_iblProbeProcessor", false },

    // textures
    { "SCREEN_TEXTURE", "qt_screenTexture", true },
    { "SCREEN_MIP_TEXTURE", "qt_screenTexture", true }, // same resource as SCREEN_TEXTURE under the hood
    { "DEPTH_TEXTURE", "qt_depthTexture", true },
    { "AO_TEXTURE", "qt_aoTexture", true },
    { "IBL_TEXTURE", "qt_lightProbe", false },
    { "LIGHTMAP", "qt_lightmap", false },
    { "NORMAL_ROUGHNESS_TEXTURE", "qt_normalTexture", false },
    { "MOTION_VECTOR_TEXTURE", "qt_motionVectorTexture", false },

    // For shaded only: vertex outputs, for convenience and perf. (only those
    // that are always present when lighting is enabled) The custom vertex main
    // can also calculate on its own and pass them on with VARYING but that's a
    // bit wasteful since we calculate these anyways.
    { "VAR_WORLD_NORMAL", "qt_varNormal", false },
    { "VAR_WORLD_TANGENT", "qt_varTangent", false },
    { "VAR_WORLD_BINORMAL", "qt_varBinormal", false },
    { "VAR_WORLD_POSITION", "qt_varWorldPos", false },
    // vertex color is always enabled for custom materials (shaded)
    { "VAR_COLOR", "qt_varColor", false },

    // effects
    { "INPUT", "qt_inputTexture", true },
    { "INPUT_UV", "qt_inputUV", false },
    { "TEXTURE_UV", "qt_textureUV", false },
    { "INPUT_SIZE", "qt_inputSize", false },
    { "OUTPUT_SIZE", "qt_outputSize", false },
    { "FRAME", "qt_frame_num", false },

    // instancing
    { "INSTANCE_COLOR", "qt_instanceColor", false },
    { "INSTANCE_DATA", "qt_instanceData", false },
    { "INSTANCE_INDEX", "gl_InstanceIndex", false },

    // morphing
    { "MORPH_POSITION", "qt_getTargetPositionFromTargetId", false },
    { "MORPH_NORMAL", "qt_getTargetNormalFromTargetId", false },
    { "MORPH_TANGENT", "qt_getTargetTangentFromTargetId", false },
    { "MORPH_BINORMAL", "qt_getTargetBinormalFromTargetId", false },
    { "MORPH_WEIGHTS", "qt_morphWeights", false },

    // custom variables
    { "SHARED_VARS", "struct QT_SHARED_VARS", false },

    // multiview
    { "VIEW_INDEX", "qt_viewIndex", false }
};

// Functions that, if present, get an argument list injected.
static const QByteArrayView qssg_func_injectarg_tab[] = {
    "DIRECTIONAL_LIGHT",
    "POINT_LIGHT",
    "SPOT_LIGHT",
    "AMBIENT_LIGHT",
    "SPECULAR_LIGHT",
    "MAIN",
    "POST_PROCESS",
    "IBL_PROBE"
};

// This is based on the Qt Quick shader rewriter (with fixes)
struct Tokenizer {
    enum Token {
        Token_Comment,
        Token_OpenBrace,
        Token_CloseBrace,
        Token_OpenParen,
        Token_CloseParen,
        Token_SemiColon,
        Token_Identifier,
        Token_OpenBraket,
        Token_CloseBraket,
        Token_Unspecified,

        Token_EOF
    };

    void initialize(const QByteArray &input);
    Token next();

    const char *stream;
    const char *pos;
    const char *identifier;
};

void Tokenizer::initialize(const QByteArray &input)
{
    stream = input.constData();
    pos = input;
    identifier = input;
}

Tokenizer::Token Tokenizer::next()
{
    while (*pos) {
        char c = *pos++;
        switch (c) {
        case '/':
            if (*pos == '/') {
                // '//' comment
                ++pos;
                while (*pos && *pos != '\n') ++pos;
                return Token_Comment;
            } else if (*pos == '*') {
                // /* */ comment
                ++pos;
                while (*pos && (*pos != '*' || pos[1] != '/')) ++pos;
                if (*pos) pos += 2;
                return Token_Comment;
            }
            return Token_Unspecified;

        case ';': return Token_SemiColon;
        case '\0': return Token_EOF;
        case '{': return Token_OpenBrace;
        case '}': return Token_CloseBrace;
        case '(': return Token_OpenParen;
        case ')': return Token_CloseParen;
        case '[': return Token_OpenBraket;
        case ']': return Token_CloseBraket;

        case ' ':
        case '\n':
        case '\r': break;
        default:
            // Identifier...
            if ((c >= 'a' && c <= 'z' ) || (c >= 'A' && c <= 'Z' ) || c == '_') {
                identifier = pos - 1;
                while (*pos && ((*pos >= 'a' && *pos <= 'z')
                                     || (*pos >= 'A' && *pos <= 'Z')
                                     || *pos == '_'
                                     || (*pos >= '0' && *pos <= '9'))) {
                    ++pos;
                }
                return Token_Identifier;
            } else {
                return Token_Unspecified;
            }
        }
    }

    return Token_EOF;
}
} // namespace

void QSSGShaderCustomMaterialAdapter::beginPrepareCustomShader(
    CustomShaderPrepWorkData *workData,
    ShaderCodeAndMetaData *codeAndMetaData,
    const QByteArray &shaderCode,
    QSSGShaderCache::ShaderType type,
    bool multiViewCompatible)
{
    QByteArrayList inputs;
    QByteArrayList outputs;

    Tokenizer tok;
    tok.initialize(shaderCode);

    QSSGCustomShaderMetaData md = {};
    QByteArray result;
    result.reserve(1024);
    // If shader debugging is not enabled we reset the line count to make error message
    // when a shader fails more useful. When shader debugging is enabled the whole shader
    // will be printed and not just the user written part, so in that case we do not want
    // to adjust the line numbers.
    //
    // NOTE: This is not perfect, we do expend the custom material and effect shaders, so
    // there cane still be cases where the reported line numbers are slightly off.
    if (!QSSGRhiContextPrivate::shaderDebuggingEnabled())
        result.prepend("#line 1\n");
    const char *lastPos = shaderCode.constData();

    int funcFinderState = 0;
    int useJointTexState = -1;
    int useJointNormalTexState = -1;
    QByteArray currentShadedFunc;
    Tokenizer::Token t = tok.next();
    while (t != Tokenizer::Token_EOF) {
        switch (t) {
        case Tokenizer::Token_Comment:
            break;
        case Tokenizer::Token_Identifier:
        {
            QByteArray id = QByteArray::fromRawData(lastPos, tok.pos - lastPos);
            if (id.trimmed() == QByteArrayLiteral("VARYING")) {
                QByteArray vtype;
                QByteArray vname;
                bool vflat = false;
                lastPos = tok.pos;
                t = tok.next();
                while (t != Tokenizer::Token_EOF) {
                    QByteArray data = QByteArray::fromRawData(lastPos, tok.pos - lastPos);
                    if (t == Tokenizer::Token_Identifier) {
                        if (vtype.isEmpty()) {
                            vtype = data.trimmed();
                            if (vtype == QByteArrayLiteral("flat")) {
                                vflat = true;
                                vtype.clear();
                            }
                        } else if (vname.isEmpty()) {
                            vname = data.trimmed();
                        }
                    }
                    if (t == Tokenizer::Token_SemiColon)
                        break;
                    lastPos = tok.pos;
                    t = tok.next();
                }
                if (type == QSSGShaderCache::ShaderType::Vertex)
                    outputs.append((vflat ? "flat " : "") + vtype + " " + vname);
                else
                    inputs.append((vflat ? "flat " : "") + vtype + " " + vname);
            } else {
                const QByteArray trimmedId = id.trimmed();
                if (funcFinderState == 0 && trimmedId == QByteArrayLiteral("void")) {
                    funcFinderState += 1;
                } else if (funcFinderState == 1) {
                    auto begin = qssg_func_injectarg_tab;
                    const auto end = qssg_func_injectarg_tab + (sizeof(qssg_func_injectarg_tab) / sizeof(qssg_func_injectarg_tab[0]));
                    const auto foundIt = std::find_if(begin, end, [trimmedId](const QByteArrayView &entry) { return entry == trimmedId; });
                    if (foundIt != end) {
                        currentShadedFunc = trimmedId;
                        funcFinderState += 1;
                    }
                } else {
                    funcFinderState = 0;
                }

                if (trimmedId == QByteArrayLiteral("SCREEN_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesScreenTexture;
                else if (trimmedId == QByteArrayLiteral("SCREEN_MIP_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesScreenMipTexture;
                else if (trimmedId == QByteArrayLiteral("DEPTH_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesDepthTexture;
                else if (trimmedId == QByteArrayLiteral("NORMAL_ROUGHNESS_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesNormalTexture;
                else if (trimmedId == QByteArrayLiteral("AO_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesAoTexture;
                else if (trimmedId == QByteArrayLiteral("POSITION"))
                    md.flags |= QSSGCustomShaderMetaData::OverridesPosition;
                else if (trimmedId == QByteArrayLiteral("PROJECTION_MATRIX"))
                    md.flags |= QSSGCustomShaderMetaData::UsesProjectionMatrix;
                else if (trimmedId == QByteArrayLiteral("INVERSE_PROJECTION_MATRIX"))
                    md.flags |= QSSGCustomShaderMetaData::UsesInverseProjectionMatrix;
                else if (trimmedId == QByteArrayLiteral("VIEW_MATRIX"))
                    md.flags |= QSSGCustomShaderMetaData::UsesViewMatrix;
                else if (trimmedId == QByteArrayLiteral("VAR_COLOR"))
                    md.flags |= QSSGCustomShaderMetaData::UsesVarColor;
                else if (trimmedId == QByteArrayLiteral("SHARED_VARS"))
                    md.flags |= QSSGCustomShaderMetaData::UsesSharedVars;
                else if (trimmedId == QByteArrayLiteral("IBL_ORIENTATION"))
                    md.flags |= QSSGCustomShaderMetaData::UsesIblOrientation;
                else if (trimmedId == QByteArrayLiteral("LIGHTMAP"))
                    md.flags |= QSSGCustomShaderMetaData::UsesLightmap;
                else if (trimmedId == QByteArrayLiteral("VIEW_INDEX"))
                    md.flags |= QSSGCustomShaderMetaData::UsesViewIndex;
                else if (trimmedId == QByteArrayLiteral("INPUT"))
                    md.flags |= QSSGCustomShaderMetaData::UsesInputTexture;
                else if (trimmedId == QByteArrayLiteral("CLEARCOAT_AMOUNT"))
                    md.flags |= QSSGCustomShaderMetaData::UsesClearcoat;
                else if (trimmedId == QByteArrayLiteral("CLEARCOAT_FRESNEL_SCALE") ||
                            trimmedId == QByteArrayLiteral("CLEARCOAT_FRESNEL_BIAS"))
                    md.flags |= QSSGCustomShaderMetaData::UsesClearcoatFresnelScaleBias;
                else if (trimmedId == QByteArrayLiteral("FRESNEL_SCALE") ||
                            trimmedId == QByteArrayLiteral("FRESNEL_BIAS"))
                    md.flags |= QSSGCustomShaderMetaData::UsesFresnelScaleBias;
                else if (trimmedId == QByteArrayLiteral("MOTION_VECTOR_TEXTURE"))
                    md.flags |= QSSGCustomShaderMetaData::UsesMotionVectorTexture;
                else if (trimmedId == QByteArrayLiteral("TRANSMISSION_FACTOR")) {
                    md.flags |= QSSGCustomShaderMetaData::UsesTransmission;
                    md.flags |= QSSGCustomShaderMetaData::UsesScreenTexture;
                    md.flags |= QSSGCustomShaderMetaData::UsesScreenMipTexture;
                }

                for (const QSSGCustomMaterialVariableSubstitution &subst : qssg_var_subst_tab) {
                    if (trimmedId == subst.builtin) {
                        QByteArray newExpr;
                        newExpr.assign(subst.actualName);
                        if (subst.multiViewDependent && multiViewCompatible) {
                            if (subst.builtin.endsWith(QByteArrayLiteral("_TEXTURE"))
                                || subst.builtin == QByteArrayLiteral("INPUT"))
                            {
                                newExpr += QByteArrayLiteral("Array"); // e.g. qt_depthTexture -> qt_depthTextureArray
                            } else {
                                newExpr += QByteArrayLiteral("[qt_viewIndex]"); // e.g. qt_viewProjectionMatrix -> qt_viewProjectionMatrix[qt_viewIndex]
                            }
                        }
                        id.replace(subst.builtin, newExpr); // replace, not assignment, to keep whitespace etc.
                        if (trimmedId == QByteArrayLiteral("BONE_TRANSFORMS")) {
                            useJointTexState = 0;
                            md.flags |= QSSGCustomShaderMetaData::UsesSkinning;
                        } else if (trimmedId == QByteArrayLiteral("BONE_NORMAL_TRANSFORMS")) {
                            useJointNormalTexState = 0;
                            md.flags |= QSSGCustomShaderMetaData::UsesSkinning;
                        }
                        if (trimmedId == QByteArrayLiteral("MORPH_POSITION") ||
                                trimmedId == QByteArrayLiteral("MORPH_NORMAL") ||
                                trimmedId == QByteArrayLiteral("MORPH_TANGENT") ||
                                trimmedId == QByteArrayLiteral("MORPH_BINORMAL"))
                            md.flags |= QSSGCustomShaderMetaData::UsesMorphing;
                        break;
                    }
                }
                result += id;
            }
        }
            break;
        case Tokenizer::Token_OpenParen:
            result += QByteArray::fromRawData(lastPos, tok.pos - lastPos);
            if (funcFinderState == 2) {
                result += QByteArrayLiteral("/*%QT_ARGS_");
                result += currentShadedFunc;
                result += QByteArrayLiteral("%*/");
                for (const QSSGCustomMaterialVariableSubstitution &subst : qssg_var_subst_tab) {
                    if (currentShadedFunc == subst.builtin) {
                        currentShadedFunc = subst.actualName.toByteArray();
                        break;
                    }
                }
                md.customFunctions.insert(currentShadedFunc);
                currentShadedFunc.clear();
            }
            funcFinderState = 0;
            break;
        case Tokenizer::Token_OpenBraket:
            // copy everything as-is up to the [
            result += QByteArray::fromRawData(lastPos, tok.pos - lastPos - 1);
            if (useJointTexState == 0) {
                result += QByteArrayLiteral("(2 * (");
                ++useJointTexState;
                break;
            } else if (useJointNormalTexState == 0) {
                result += QByteArrayLiteral("(1 + 2 * (");
                ++useJointNormalTexState;
                break;
            }

            if (useJointTexState >= 0)
                ++useJointTexState;
            else if (useJointNormalTexState >= 0)
                ++useJointNormalTexState;
            result += QByteArrayLiteral("[");
            break;
        case Tokenizer::Token_CloseBraket:
            // copy everything as-is up to the ]
            result += QByteArray::fromRawData(lastPos, tok.pos - lastPos - 1);
            // This implementation will not allow mixed usages of BONE_TRANSFORMS and
            // BONE_NORMAL_TRANSFORMS.
            // For example, BONE_TRANSFORM[int(BONE_NORMAL_TRANFORMS[i][0].x)]
            // cannot be compiled successfully.
            if (useJointTexState <= 0 && useJointNormalTexState <= 0) {
                result += QByteArrayLiteral("]");
                break;
            }
            if (useJointTexState > 1) {
                result += QByteArrayLiteral("]");
                --useJointTexState;
                break;
            } else if (useJointNormalTexState > 1) {
                result += QByteArrayLiteral("]");
                --useJointNormalTexState;
                break;
            }
            result += QByteArrayLiteral("))");
            useJointTexState = -1;
            useJointNormalTexState = -1;
            break;
        default:
            result += QByteArray::fromRawData(lastPos, tok.pos - lastPos);
            break;
        }
        lastPos = tok.pos;
        t = tok.next();
    }

    result += '\n';

    workData->inputs = inputs;
    workData->outputs = outputs;

    *codeAndMetaData = { result, md };
}

void QSSGShaderCustomMaterialAdapter::finishPrepareCustomShader(
    QByteArray *dst,
    const CustomShaderPrepWorkData &workData,
    const ShaderCodeAndMetaData &codeAndMetaData,
    QSSGShaderCache::ShaderType type,
    bool multiViewCompatible,
    const StringPairList &baseUniforms,
    const StringPairList &baseInputs,
    const StringPairList &baseOutputs,
    const StringPairList &multiViewDependentSamplers,
    const StringPairList &multiViewDependentUniforms)
{
    StringPairList allUniforms = baseUniforms;

    for (const StringPair &samplerTypeAndName : multiViewDependentSamplers) {
        if (multiViewCompatible)
            allUniforms.append({ "sampler2DArray", samplerTypeAndName.second });
        else
            allUniforms.append(samplerTypeAndName);
    }

    const QSSGCustomShaderMetaData &md(codeAndMetaData.second);

    // We either have qt_depthTexture or qt_depthTextureArray (or none of them),
    // but never both. We do not generally support binding a 2D texture to a
    // sampler2DArray binding point and vice versa. Therefore it is up to the
    // shader snippet to ifdef with QSHADER_VIEW_COUNT if it wants to support
    // both multiview and non-multiview rendering.
    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesDepthTexture)) {
        if (multiViewCompatible)
            allUniforms.append({ "sampler2DArray", "qt_depthTextureArray" });
        else
            allUniforms.append({ "sampler2D", "qt_depthTexture" });
    }

    // And the same pattern for qt_screenTexture(Array).
    if ((md.flags.testFlag(QSSGCustomShaderMetaData::UsesScreenTexture) || md.flags.testFlag(QSSGCustomShaderMetaData::UsesScreenMipTexture))) {
        if (multiViewCompatible)
            allUniforms.append({ "sampler2DArray", "qt_screenTextureArray" });
        else
            allUniforms.append({ "sampler2D", "qt_screenTexture" });
    }

    // And for SSAO.
    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesAoTexture)) {
        if (multiViewCompatible)
            allUniforms.append({ "sampler2DArray", "qt_aoTextureArray" });
        else
            allUniforms.append({ "sampler2D", "qt_aoTexture" });
    }

    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesNormalTexture))
        allUniforms.append({ "sampler2D", "qt_normalTexture" });

    // Input texture for post-processing effects.
    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesInputTexture)) {
        if (multiViewCompatible)
            allUniforms.append({ "sampler2DArray", "qt_inputTextureArray" });
        else
            allUniforms.append({ "sampler2D", "qt_inputTexture" });
    }

    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesLightmap))
        allUniforms.append({ "sampler2D", "qt_lightmap" });

    if (md.flags.testFlag(QSSGCustomShaderMetaData::UsesMotionVectorTexture))
        allUniforms.append({ "sampler2D", "qt_motionVectorTexture" });

    static const char *metaStart = "#ifdef QQ3D_SHADER_META\n/*{\n  \"uniforms\": [\n";
    static const char *metaEnd = "  ]\n}*/\n#endif\n";
    dst->append(metaStart);

    for (int i = 0, count = allUniforms.size(); i < count; ++i) {
        const auto &typeAndName(allUniforms[i]);
        dst->append("    { \"type\": \"" + typeAndName.first + "\", \"name\": \"" + typeAndName.second + "\" }");
        if (i < count - 1 || !multiViewDependentUniforms.isEmpty())
            dst->append(",");
        dst->append("\n");
    }

    // multiViewDependentUniforms are not in allUniforms. For these we leave it to the
    // metadata processor to add an array suffix (e.g., qt_projectionMatrix[2]) when needed.
    for (int i = 0, count = multiViewDependentUniforms.size(); i < count; ++i) {
        const auto &typeAndName(multiViewDependentUniforms[i]);
        dst->append("    { \"type\": \"" + typeAndName.first + "\", \"name\": \"" + typeAndName.second + "\", \"multiview_dependent\": true }");
        if (i < count - 1)
            dst->append(",");
        dst->append("\n");
    }

    dst->append(metaEnd);

    const char *stageStr = type == QSSGShaderCache::ShaderType::Vertex ? "vertex" : "fragment";
    StringPairList allInputs = baseInputs;
    QVarLengthArray<bool, 16> inputIsFlat(allInputs.count(), false);
    for (const QByteArray &inputTypeAndName : workData.inputs) {
        const QByteArrayList typeAndName = inputTypeAndName.split(' ');
        if (typeAndName.size() == 2) {
            allInputs.append({ typeAndName[0].trimmed(), typeAndName[1].trimmed() });
            inputIsFlat.append(false);
        } else if (typeAndName.size() == 3 && typeAndName[0].startsWith("flat")) {
            allInputs.append({ typeAndName[1].trimmed(), typeAndName[2].trimmed() });
            inputIsFlat.append(true);
        }
    }
    if (!allInputs.isEmpty()) {
        static const char *metaStart = "#ifdef QQ3D_SHADER_META\n/*{\n  \"inputs\": [\n";
        static const char *metaEnd = "  ]\n}*/\n#endif\n";
        dst->append(metaStart);
        for (int i = 0, count = allInputs.size(); i < count; ++i) {
            dst->append("    { \"type\": \"" + allInputs[i].first
                    + "\", \"name\": \"" + allInputs[i].second
                    + "\", \"stage\": \"" + stageStr
                    + (inputIsFlat[i] ? "\", \"flat\": true" : "\"")
                    + " }");
            if (i < count - 1)
                dst->append(",");
            dst->append("\n");
        }
        dst->append(metaEnd);
    }

    StringPairList allOutputs = baseOutputs;
    QVarLengthArray<bool, 16> outputIsFlat(allOutputs.count(), false);
    for (const QByteArray &outputTypeAndName : workData.outputs) {
        const QByteArrayList typeAndName = outputTypeAndName.split(' ');
        if (typeAndName.size() == 2) {
            allOutputs.append({ typeAndName[0].trimmed(), typeAndName[1].trimmed() });
            outputIsFlat.append(false);
        } else if (typeAndName.size() == 3 && typeAndName[0].startsWith("flat")) {
            allOutputs.append({ typeAndName[1].trimmed(), typeAndName[2].trimmed() });
            outputIsFlat.append(true);
        }
    }
    if (!allOutputs.isEmpty()) {
        static const char *metaStart = "#ifdef QQ3D_SHADER_META\n/*{\n  \"outputs\": [\n";
        static const char *metaEnd = "  ]\n}*/\n#endif\n";
        dst->append(metaStart);
        for (int i = 0, count = allOutputs.size(); i < count; ++i) {
            dst->append("    { \"type\": \"" + allOutputs[i].first
                    + "\", \"name\": \"" + allOutputs[i].second
                    + "\", \"stage\": \"" + stageStr
                    + (outputIsFlat[i] ? "\", \"flat\": true" : "\"")
                    + " }");
            if (i < count - 1)
                dst->append(",");
            dst->append("\n");
        }
        dst->append(metaEnd);
    }
}

QList<QByteArrayView> QtQuick3DEditorHelpers::CustomMaterial::preprocessorVars()
{
    QList<QByteArrayView> k;
    k.reserve(std::size(qssg_var_subst_tab));
    for (const auto &v : qssg_var_subst_tab)
        k.push_back(v.builtin);
    return k;
}

QT_END_NAMESPACE

