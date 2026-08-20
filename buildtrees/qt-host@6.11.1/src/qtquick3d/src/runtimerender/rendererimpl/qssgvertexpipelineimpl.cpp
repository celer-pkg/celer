// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgvertexpipelineimpl_p.h"

#include <QtCore/qregularexpression.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>

QT_BEGIN_NAMESPACE

QSSGMaterialVertexPipeline::QSSGMaterialVertexPipeline(QSSGProgramGenerator &programGen,
                                                       const QSSGShaderDefaultMaterialKeyProperties &materialProperties,
                                                       QSSGShaderMaterialAdapter *materialAdapter)
    : m_programGenerator(&programGen)
    , defaultMaterialShaderKeyProperties(materialProperties)
    , materialAdapter(materialAdapter)
    , hasCustomShadedMain(false)
    , skipCustomFragmentSnippet(false)
{
}

static inline void insertProcessorArgsFragmentMain(QByteArray &snippet, const char *argKey, const char* (*argListFunc)(),
                                       QSSGShaderMaterialAdapter *materialAdapter = nullptr, bool isSharedInout = false)
{
    const int argKeyLen = int(strlen(argKey));
    const int argKeyPos = snippet.indexOf(argKey);
    if (argKeyPos >= 0) {
        QByteArray flexArgs;
        if (materialAdapter) {
            if (materialAdapter->isClearcoatEnabled()) {
                flexArgs += QByteArrayLiteral(", inout float CLEARCOAT_AMOUNT, inout float CLEARCOAT_FRESNEL_POWER, inout float CLEARCOAT_ROUGHNESS, inout vec3 CLEARCOAT_NORMAL");
                if (materialAdapter->isClearcoatFresnelScaleBiasEnabled())
                    flexArgs += QByteArrayLiteral(", inout float CLEARCOAT_FRESNEL_SCALE, inout float CLEARCOAT_FRESNEL_BIAS");
            }
            if (materialAdapter->isFresnelScaleBiasEnabled())
                flexArgs += QByteArrayLiteral(", inout float FRESNEL_SCALE, inout float FRESNEL_BIAS");
            if (materialAdapter->isTransmissionEnabled())
                flexArgs += QByteArrayLiteral(", inout float TRANSMISSION_FACTOR, inout float THICKNESS_FACTOR, inout vec3 ATTENUATION_COLOR, inout float ATTENUATION_DISTANCE");
            if (materialAdapter->usesSharedVariables()) {
                const char *inoutString = isSharedInout ? ", inout " : ", in ";
                flexArgs += inoutString + QByteArrayLiteral("QT_SHARED_VARS SHARED");
            }
        }
        snippet = snippet.left(argKeyPos) + argListFunc() + flexArgs + snippet.mid(argKeyPos + argKeyLen);
    }
}

static inline void insertProcessorArgs(QByteArray &snippet, const char *argKey, const char* (*argListFunc)(),
                                    QSSGShaderMaterialAdapter *materialAdapter = nullptr, bool isSharedInout = false)
{
    const int argKeyLen = int(strlen(argKey));
    const int argKeyPos = snippet.indexOf(argKey);
    if (argKeyPos >= 0) {
        QByteArray flexArgs;
        if (materialAdapter && materialAdapter->usesSharedVariables()) {
            const char *inoutString = isSharedInout ? ", inout " : ", in ";
            flexArgs += inoutString + QByteArrayLiteral("QT_SHARED_VARS SHARED");
        }
        snippet = snippet.left(argKeyPos) + argListFunc() + flexArgs + snippet.mid(argKeyPos + argKeyLen);
    }
}

static inline void insertDirectionalLightProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_DIRECTIONAL_LIGHT%*/", QSSGMaterialShaderGenerator::directionalLightProcessorArgumentList, materialAdapter, true);
}

static inline void insertPointLightProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_POINT_LIGHT%*/", QSSGMaterialShaderGenerator::pointLightProcessorArgumentList, materialAdapter, true);
}

static inline void insertSpotLightProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_SPOT_LIGHT%*/", QSSGMaterialShaderGenerator::spotLightProcessorArgumentList, materialAdapter, true);
}

static inline void insertAmbientLightProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_AMBIENT_LIGHT%*/", QSSGMaterialShaderGenerator::ambientLightProcessorArgumentList, materialAdapter, true);
}

static inline void insertIblProbeProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_IBL_PROBE%*/", QSSGMaterialShaderGenerator::iblProbeProcessorArgumentList, materialAdapter, true);
}

static inline void insertSpecularLightProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_SPECULAR_LIGHT%*/", QSSGMaterialShaderGenerator::specularLightProcessorArgumentList, materialAdapter, true);
}

static inline void insertFragmentMainArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgsFragmentMain(snippet, "/*%QT_ARGS_MAIN%*/", QSSGMaterialShaderGenerator::shadedFragmentMainArgumentList, materialAdapter, true);
}

static inline void insertPostProcessorArgs(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_POST_PROCESS%*/", QSSGMaterialShaderGenerator::postProcessorArgumentList, materialAdapter, false);
}

static inline void insertVertexMainArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_MAIN%*/", QSSGMaterialShaderGenerator::vertexMainArgumentList);
}

static inline void insertVertexInstancedMainArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_MAIN%*/", QSSGMaterialShaderGenerator::vertexInstancedMainArgumentList);
}

static inline const char *customMainCallWithArguments(bool usesInstancing)
{
    if (usesInstancing)
        return "    qt_customMain(qt_vertPosition.xyz, qt_vertNormal, qt_vertUV0, qt_vertUV1, qt_vertTangent, qt_vertBinormal, qt_vertJoints, qt_vertWeights, qt_vertColor, qt_instancedModelMatrix, qt_instancedMVPMatrix);";
    else
        return "    qt_customMain(qt_vertPosition.xyz, qt_vertNormal, qt_vertUV0, qt_vertUV1, qt_vertTangent, qt_vertBinormal, qt_vertJoints, qt_vertWeights, qt_vertColor);\n";
}

static inline QByteArray extractSharedVarsTypeDefinition(QByteArray &snippet, QSSGShaderMaterialAdapter *materialAdapter) {
    if (materialAdapter->usesSharedVariables()) {
        // Extract shared variables from the custom shader snippet
        static QRegularExpression re(QString::fromLocal8Bit(R"(struct\s+QT_SHARED_VARS\s*\{[\s\S]*?\};)"), QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = re.match(QString::fromLocal8Bit(snippet));
        if (!match.hasMatch())
            return QByteArray();
        QString typeDefinition = match.captured(0);
        snippet.remove(match.capturedStart(0), match.capturedLength(0));
        return typeDefinition.toLocal8Bit();
    }
    return QByteArray();
}

void QSSGMaterialVertexPipeline::beginVertexGeneration(const QSSGShaderDefaultMaterialKey &inKey,
                                                       const QSSGShaderFeatures &inFeatureSet,
                                                       QSSGShaderLibraryManager &shaderLibraryManager)
{
    QSSGShaderGeneratorStageFlags theStages(QSSGProgramGenerator::defaultFlags());
    programGenerator()->beginProgram(theStages);

    QSSGStageGeneratorBase &vertexShader(vertex());

    const bool meshHasNormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Normal, inKey);
    const bool meshHasTexCoord0 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::TexCoord0, inKey);
    const bool meshHasTexCoord1 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::TexCoord1, inKey);
    const bool meshHasTexCoordLightmap = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::TexCoordLightmap, inKey);
    const bool meshHasTangents = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Tangent, inKey);
    const bool meshHasBinormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Binormal, inKey);
    const bool meshHasColors = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Color, inKey);
    const bool meshHasJointsAndWeights = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::JointAndWeight, inKey);
    const bool overridesPosition = defaultMaterialShaderKeyProperties.m_overridesPosition.getValue(inKey);
    const bool usesProjectionMatrix = defaultMaterialShaderKeyProperties.m_usesProjectionMatrix.getValue(inKey);
    const bool usesInvProjectionMatrix = defaultMaterialShaderKeyProperties.m_usesInverseProjectionMatrix.getValue(inKey);
    const bool usesPointsTopology = defaultMaterialShaderKeyProperties.m_usesPointsTopology.getValue(inKey);
    const bool usesFloatJointIndices = defaultMaterialShaderKeyProperties.m_usesFloatJointIndices.getValue(inKey);
    const bool blendParticles = defaultMaterialShaderKeyProperties.m_blendParticles.getValue(inKey);
    usesInstancing = defaultMaterialShaderKeyProperties.m_usesInstancing.getValue(inKey);
    m_hasSkinning = defaultMaterialShaderKeyProperties.m_boneCount.getValue(inKey) > 0;
    const auto morphSize = defaultMaterialShaderKeyProperties.m_targetCount.getValue(inKey);
    m_hasMorphing = morphSize > 0;
    m_viewCount = inFeatureSet.isSet(QSSGShaderFeatures::Feature::DisableMultiView)
        ? 1 : defaultMaterialShaderKeyProperties.m_viewCount.getValue(inKey);
    const bool usesViewIndex = defaultMaterialShaderKeyProperties.m_usesViewIndex.getValue(inKey);

    vertexShader.addIncoming("attr_pos", "vec3");
    if (usesInstancing) {
        vertexShader.addIncoming("qt_instanceTransform0", "vec4");
        vertexShader.addIncoming("qt_instanceTransform1", "vec4");
        vertexShader.addIncoming("qt_instanceTransform2", "vec4");
        vertexShader.addIncoming("qt_instanceColor", "vec4");
        vertexShader.addIncoming("qt_instanceData", "vec4");
    }
    if (blendParticles) {
        vertexShader.addInclude("particles.glsllib");
        vertexShader.addUniform("qt_particleTexture", "sampler2D");
        vertexShader.addUniform("qt_countPerSlice", "uint");
        vertexShader.addUniform("qt_oneOverParticleImageSize", "vec2");
        vertexShader.addUniform("qt_particleMatrix", "mat4");
        vertexShader.addUniform("qt_particleIndexOffset", "uint");
    }

    if (m_hasSkinning && meshHasJointsAndWeights) {
        vertexShader.addInclude("skinanim.glsllib");
        if (usesFloatJointIndices)
            vertexShader.addIncoming("attr_joints", "vec4");
        else
            vertexShader.addIncoming("attr_joints", "ivec4");
        vertexShader.addIncoming("attr_weights", "vec4");

        vertexShader.addUniform("qt_boneTexture", "sampler2D");
    }
    if (m_hasMorphing) {
        vertexShader.addInclude("morphanim.glsllib");
        vertexShader.addUniformArray("qt_morphWeights", "float", morphSize);
        vertexShader.addUniform("qt_morphTargetTexture", "sampler2DArray");
    }

    const bool hasCustomVertexShader = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Vertex);
    const bool hasCustomFragmentShader = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment);
    if (hasCustomVertexShader) {
        QByteArray snippet = materialAdapter->customShaderSnippet(QSSGShaderCache::ShaderType::Vertex,
                                                                  shaderLibraryManager,
                                                                  m_viewCount >= 2);
        if (materialAdapter->hasCustomShaderFunction(QSSGShaderCache::ShaderType::Vertex,
                                                     QByteArrayLiteral("qt_customMain"),
                                                     shaderLibraryManager))
        {
            if (usesInstancing)
                insertVertexInstancedMainArgs(snippet);
            else
                insertVertexMainArgs(snippet);

            if (materialAdapter->usesCustomSkinning()) {
                vertexShader.addInclude("skinanim.glsllib");
                vertexShader.addUniform("qt_boneTexture", "sampler2D");
            }

            if (materialAdapter->usesCustomMorphing()) {
                vertexShader.addInclude("morphanim_custom.glsllib");
                if (morphSize > 0)
                    vertexShader.addUniformArray("qt_morphWeights", "float", morphSize);
                vertexShader.addUniform("qt_morphTargetTexture", "sampler2DArray");
                m_hasMorphing = false;
            }

            if (!materialAdapter->isUnshaded()) {
                hasCustomShadedMain = true;
            }
        }
        vertexShader << snippet;
    }

    vertexShader << "void main()"
                 << "\n"
                 << "{"
                 << "\n";
    // These local variables will be used for whole the pipeline
    // instead of each attributes since it is more convenient
    // for adding new routines.
    vertexShader.append("    vec4 qt_vertPosition = vec4(attr_pos, 1.0);");
    vertexShader.append("    vec3 qt_vertNormal = vec3(0.0);");
    vertexShader.append("    vec3 qt_vertTangent = vec3(0.0);");
    vertexShader.append("    vec3 qt_vertBinormal = vec3(0.0);");
    if (meshHasTexCoord0 || hasCustomVertexShader)
        vertexShader.append("    vec2 qt_vertUV0 = vec2(0.0);");
    if (meshHasTexCoord1 || hasCustomVertexShader)
        vertexShader.append("    vec2 qt_vertUV1 = vec2(0.0);");
    if (m_hasSkinning || hasCustomVertexShader)
        vertexShader.append("    ivec4 qt_vertJoints = ivec4(0);");
    if (meshHasJointsAndWeights || m_hasSkinning || hasCustomVertexShader)
        vertexShader.append("    vec4 qt_vertWeights = vec4(0.0);");
    if (meshHasColors || usesInstancing || blendParticles || hasCustomVertexShader || hasCustomFragmentShader)
        vertexShader.append("    vec4 qt_vertColor = vec4(1.0);"); // must be 1,1,1,1 to not alter when multiplying with it

    if (!usesInstancing) {
        if (m_viewCount < 2)
            vertexShader.addUniform("qt_modelViewProjection", "mat4");
        else
            vertexShader.addUniformArray("qt_modelViewProjection", "mat4", m_viewCount);
    } else {
        // Must manualy calculate a MVP
        vertexShader.addUniform("qt_modelMatrix", "mat4");
        vertexShader.addUniform("qt_parentMatrix", "mat4");
        if (m_viewCount < 2)
            vertexShader.addUniform("qt_viewProjectionMatrix", "mat4");
        else
            vertexShader.addUniformArray("qt_viewProjectionMatrix", "mat4", m_viewCount);
    }

    // The custom fragment main should be skipped if this is a
    // depth (or normal texture) pass, but not if it is also a OpaqueDepthPrePass
    // because then we need to know the real alpha values
    skipCustomFragmentSnippet = false;
    const bool isDepthPass = inFeatureSet.isSet(QSSGShaderFeatures::Feature::DepthPass);
    const bool isOpaqueDepthPrePass = inFeatureSet.isSet(QSSGShaderFeatures::Feature::OpaqueDepthPrePass);
    skipCustomFragmentSnippet = (isDepthPass && !isOpaqueDepthPrePass);

    if (hasCustomVertexShader || hasCustomFragmentShader) {
        // This is both for unshaded and shaded. Regardless of any other
        // condition we have to ensure the keywords (VIEW_MATRIX etc.) promised
        // by the documentation are available in *both* the custom vertex and
        // fragment shader snippets, even if only one of them is present.
        if (m_viewCount < 2) {
            vertexShader.addUniform("qt_viewProjectionMatrix", "mat4");
            vertexShader.addUniform("qt_viewMatrix", "mat4");
            vertexShader.addUniform("qt_cameraPosition", "vec3");
            vertexShader.addUniform("qt_cameraDirection", "vec3");
            if (usesProjectionMatrix)
                vertexShader.addUniform("qt_projectionMatrix", "mat4");
            if (usesInvProjectionMatrix)
                vertexShader.addUniform("qt_inverseProjectionMatrix", "mat4");
        } else {
            vertexShader.addUniformArray("qt_viewProjectionMatrix", "mat4", m_viewCount);
            vertexShader.addUniformArray("qt_viewMatrix", "mat4", m_viewCount);
            vertexShader.addUniformArray("qt_cameraPosition", "vec3", m_viewCount);
            vertexShader.addUniformArray("qt_cameraDirection", "vec3", m_viewCount);
            if (usesProjectionMatrix)
                vertexShader.addUniformArray("qt_projectionMatrix", "mat4", m_viewCount);
            if (usesInvProjectionMatrix)
                vertexShader.addUniformArray("qt_inverseProjectionMatrix", "mat4", m_viewCount);
        }
        vertexShader.addUniform("qt_modelMatrix", "mat4");
        vertexShader.addUniform("qt_normalMatrix", "mat3");
        vertexShader.addUniform("qt_cameraProperties", "vec2");
    }

    // With multiview, qt_viewIndex (aka VIEW_INDEX) is always present.
    // Otherwise, we still make VIEW_INDEX functional (always 0) in custom
    // materials, if the keyword is used.
    if (m_viewCount >= 2) {
        addFlatParameter("qt_viewIndex", "uint");
        vertexShader.append("    qt_viewIndex = gl_ViewIndex;");
    } else if (usesViewIndex) {
        addFlatParameter("qt_viewIndex", "uint");
        vertexShader.append("    qt_viewIndex = 0;");
    }

    if (meshHasNormals) {
        vertexShader.append("    qt_vertNormal = attr_norm;");
        vertexShader.addIncoming("attr_norm", "vec3");
    }
    if (meshHasTexCoord0) {
        vertexShader.append("    qt_vertUV0 = attr_uv0;");
        vertexShader.addIncoming("attr_uv0", "vec2");
    }
    if (meshHasTexCoord1) {
        vertexShader.append("    qt_vertUV1 = attr_uv1;");
        vertexShader.addIncoming("attr_uv1", "vec2");
    }
    if (meshHasTexCoordLightmap) {
        vertexShader.append("    vec2 qt_vertLightmapUV = attr_lightmapuv;");
        vertexShader.addIncoming("attr_lightmapuv", "vec2");
    }
    if (meshHasTangents) {
        vertexShader.append("    qt_vertTangent = attr_textan;");
        vertexShader.addIncoming("attr_textan", "vec3");
    }
    if (meshHasBinormals) {
        vertexShader.append("    qt_vertBinormal = attr_binormal;");
        vertexShader.addIncoming("attr_binormal", "vec3");
    }
    if (meshHasColors) {
        vertexShader.append("    qt_vertColor = attr_color;");
        vertexShader.addIncoming("attr_color", "vec4");
    }

    if (meshHasJointsAndWeights && (m_hasSkinning || hasCustomVertexShader)) {
        if (usesFloatJointIndices) {
            vertexShader.addIncoming("attr_joints", "vec4");
            vertexShader.append("    qt_vertJoints = ivec4(attr_joints);");
        } else {
            vertexShader.addIncoming("attr_joints", "ivec4");
            vertexShader.append("    qt_vertJoints = attr_joints;");
        }
        vertexShader.addIncoming("attr_weights", "vec4");
        vertexShader.append("    qt_vertWeights = attr_weights;");
    }

    if (usesInstancing) {
        vertexShader.append("    qt_vertColor *= qt_instanceColor;");
        vertexShader.append("    mat4 qt_instanceMatrix = mat4(qt_instanceTransform0, qt_instanceTransform1, qt_instanceTransform2, vec4(0.0, 0.0, 0.0, 1.0));");
        if (m_hasSkinning)
            vertexShader.append("    mat4 qt_instancedModelMatrix =  qt_parentMatrix * transpose(qt_instanceMatrix);");
        else
            vertexShader.append("    mat4 qt_instancedModelMatrix =  qt_parentMatrix * transpose(qt_instanceMatrix) * qt_modelMatrix;");
        vertexShader.append("    mat3 qt_instancedNormalMatrix = mat3(transpose(inverse(qt_instancedModelMatrix)));");
        if (m_viewCount < 2)
            vertexShader.append("    mat4 qt_instancedMVPMatrix = qt_viewProjectionMatrix * qt_instancedModelMatrix;");
        else
            vertexShader.append("    mat4 qt_instancedMVPMatrix = qt_viewProjectionMatrix[qt_viewIndex] * qt_instancedModelMatrix;");
    }

    if (!materialAdapter->isUnshaded() || !hasCustomVertexShader) {
        vertexShader << "    vec3 qt_uTransform;\n";
        vertexShader << "    vec3 qt_vTransform;\n";

        if (hasCustomShadedMain)
            vertexShader.append(customMainCallWithArguments(usesInstancing));

        if (m_hasMorphing && !hasCustomVertexShader)
            vertexShader.append("    qt_vertPosition.xyz = qt_getTargetPosition(qt_vertPosition.xyz);");

        m_needsSkinning = m_hasSkinning && !materialAdapter->usesCustomSkinning();
        if (m_needsSkinning) {
            vertexShader.append("    mat4 skinMat = mat4(1);");
            vertexShader.append("    if (qt_vertWeights != vec4(0.0)) {");
            vertexShader.append("        skinMat = qt_getSkinMatrix(qt_vertJoints, qt_vertWeights);");
            vertexShader.append("        qt_vertPosition = skinMat * qt_vertPosition;");
            vertexShader.append("    }");
        }
        if (blendParticles) {
            vertexShader.append("    qt_vertPosition.xyz = qt_applyParticle(qt_vertPosition.xyz, qt_vertNormal, qt_vertColor, qt_vertNormal, qt_vertColor, qt_particleMatrix);");
        }

        if (!hasCustomShadedMain || !overridesPosition) {
            if (!usesInstancing) {
                if (m_viewCount < 2)
                    vertexShader.append("    gl_Position = qt_modelViewProjection * qt_vertPosition;");
                else
                    vertexShader.append("    gl_Position = qt_modelViewProjection[qt_viewIndex] * qt_vertPosition;");
            } else {
                vertexShader.append("    gl_Position = qt_instancedMVPMatrix * qt_vertPosition;");
            }
        }
    }

    if (usesPointsTopology && !hasCustomVertexShader) {
        vertexShader.addUniform("qt_materialPointSize", "float");
        vertexShader.append("    gl_PointSize = qt_materialPointSize;");
    } // with a custom vertex shader it is up to it to set gl_PointSize (aka POINT_SIZE)
}

void QSSGMaterialVertexPipeline::beginFragmentGeneration(QSSGShaderLibraryManager &shaderLibraryManager, QSSGRenderLayer::OITMethod oitMethod)
{
    fragment().addUniform("qt_material_properties", "vec4");
    fragment().addUniform("qt_rhi_properties", "vec4");

    if (m_viewCount < 2) {
        fragment().addUniform("qt_viewMatrix", "mat4");
    } else {
        fragment().addUniformArray("qt_viewMatrix", "mat4", m_viewCount);
    }

    if (!skipCustomFragmentSnippet && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment)) {
        QByteArray snippet = materialAdapter->customShaderSnippet(QSSGShaderCache::ShaderType::Fragment,
                                                                  shaderLibraryManager,
                                                                  m_viewCount >= 2);
        if (!materialAdapter->isUnshaded()) {
            insertAmbientLightProcessorArgs(snippet, materialAdapter);
            insertIblProbeProcessorArgs(snippet, materialAdapter);
            insertSpecularLightProcessorArgs(snippet, materialAdapter);
            insertSpotLightProcessorArgs(snippet, materialAdapter);
            insertPointLightProcessorArgs(snippet, materialAdapter);
            insertDirectionalLightProcessorArgs(snippet, materialAdapter);
            insertFragmentMainArgs(snippet, materialAdapter);
            insertPostProcessorArgs(snippet, materialAdapter);
            auto sharedVars = extractSharedVarsTypeDefinition(snippet, materialAdapter);
            fragment().addTypeDeclaration("QT_SHARED_VARS", sharedVars);
        }
        fragment() << snippet;
    }

    if (oitMethod != QSSGRenderLayer::OITMethod::None) {
        if (oitMethod == QSSGRenderLayer::OITMethod::LinkedList) {
            // Early tests are required since we can't undo modifications to
            // the image data structures after running the fragment shader.
            // Modifying depth in fragment shader is not possible then.
            fragment() << "layout(early_fragment_tests) in;" << "\n";
        }
        if (oitMethod == QSSGRenderLayer::OITMethod::WeightedBlended) {
            fragment().addDefinition("QSSG_OIT_METHOD", "QSSG_OIT_WEIGHTED_BLENDED");
            fragment() << "layout(location = 1) out vec4 revealageOutput;" << "\n";
        } else if (oitMethod == QSSGRenderLayer::OITMethod::LinkedList) {
            fragment() << "#extension GL_ARB_shading_language_packing : enable" << "\n";
            fragment().addDefinition("QSSG_OIT_METHOD", "QSSG_OIT_LINKED_LIST");
        }
    }
    fragment() << "void main()"
               << "\n"
               << "{"
               << "\n";

    if (!materialAdapter->isUnshaded() || !materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment))
        fragment() << "    float qt_objectOpacity = qt_material_properties.a;\n";
}

void QSSGMaterialVertexPipeline::assignOutput(const QByteArray &inVarName, const QByteArray &inVarValue)
{
    vertex() << "    " << inVarName << " = " << inVarValue << ";\n";
}

void QSSGMaterialVertexPipeline::doGenerateWorldNormal(const QSSGShaderDefaultMaterialKey &inKey)
{
    QSSGStageGeneratorBase &vertexGenerator(vertex());
    const bool usesInstancing = defaultMaterialShaderKeyProperties.m_usesInstancing.getValue(inKey);
    if (!usesInstancing)
        vertexGenerator.addUniform("qt_normalMatrix", "mat3");
    if (m_hasMorphing)
        vertexGenerator.append("    qt_vertNormal = qt_getTargetNormal(qt_vertNormal);");
    if (m_hasSkinning) {
        vertexGenerator.append("    if (qt_vertWeights != vec4(0.0))");
        vertexGenerator.append("        qt_vertNormal = qt_getSkinNormalMatrix(qt_vertJoints, qt_vertWeights) * qt_vertNormal;");
    }
    // If new model->skin is used,
    // both qt_normalMatrix and qt_modelMatrix are identity.
    if (!usesInstancing) {
        if (m_hasSkinning)
            vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_vertNormal);");
        else
            vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_normalMatrix * qt_vertNormal);");
    } else {
        vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_instancedNormalMatrix * qt_vertNormal);");
    }
    vertexGenerator.append("    qt_varNormal = qt_world_normal;");
}

void QSSGMaterialVertexPipeline::doGenerateVarTangent(const QSSGShaderDefaultMaterialKey &inKey)
{
    if (m_hasMorphing)
        vertex() << "    qt_vertTangent = qt_getTargetTangent(qt_vertTangent);\n";
    if (m_needsSkinning) {
        vertex() << "    if (qt_vertWeights != vec4(0.0))\n"
                 << "       qt_vertTangent = (skinMat * vec4(qt_vertTangent, 0.0)).xyz;\n";

    }
    const bool usesInstancing = defaultMaterialShaderKeyProperties.m_usesInstancing.getValue(inKey);
    if (!usesInstancing) {
        if (!m_hasSkinning)
            vertex() << "    qt_varTangent = (qt_modelMatrix * vec4(qt_vertTangent, 0.0)).xyz;\n";
        else
            vertex() << "    qt_varTangent = qt_vertTangent;\n";
    } else {
        vertex() << "    qt_varTangent = (qt_instancedModelMatrix * vec4(qt_vertTangent, 0.0)).xyz;\n";
    }
}

void QSSGMaterialVertexPipeline::doGenerateVarBinormal(const QSSGShaderDefaultMaterialKey &inKey)
{
    if (m_hasMorphing)
        vertex() << "    qt_vertBinormal = qt_getTargetBinormal(qt_vertBinormal);\n";
    if (m_needsSkinning) {
        vertex() << "    if (qt_vertWeights != vec4(0.0))\n"
                 << "       qt_vertBinormal = (skinMat * vec4(qt_vertBinormal, 0.0)).xyz;\n";
    }
    const bool usesInstancing = defaultMaterialShaderKeyProperties.m_usesInstancing.getValue(inKey);
    if (!usesInstancing) {
        if (!m_hasSkinning)
            vertex() << "    qt_varBinormal = (qt_modelMatrix * vec4(qt_vertBinormal, 0.0)).xyz;\n";
        else
            vertex() << "    qt_varBinormal = qt_vertBinormal;\n";
    } else {
        vertex() << "    qt_varBinormal = (qt_instancedModelMatrix * vec4(qt_vertBinormal, 0.0)).xyz;\n";
    }
}

bool QSSGMaterialVertexPipeline::hasAttributeInKey(QSSGShaderKeyVertexAttribute::VertexAttributeBits inAttr,
                                                   const QSSGShaderDefaultMaterialKey &inKey)
{
    return defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(inAttr, inKey);
}

void QSSGMaterialVertexPipeline::endVertexGeneration()
{
    if (materialAdapter->isUnshaded() && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Vertex))
         vertex() << customMainCallWithArguments(usesInstancing);
    vertex().append("}");
}

void QSSGMaterialVertexPipeline::endFragmentGeneration()
{
    if (!skipCustomFragmentSnippet && materialAdapter->isUnshaded() && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment))
        fragment() << "    qt_customMain();\n";

    fragment().append("}");
}

void QSSGMaterialVertexPipeline::addInterpolationParameter(const QByteArray &inName, const QByteArray &inType)
{
    vertex().addOutgoing(inName, inType);
    fragment().addIncoming(inName, inType);
}

void QSSGMaterialVertexPipeline::addFlatParameter(const QByteArray &inName, const QByteArray &inType)
{
    vertex().addFlatOutgoing(inName, inType);
    fragment().addFlatIncoming(inName, inType);
}

QSSGStageGeneratorBase &QSSGMaterialVertexPipeline::activeStage()
{
    return vertex();
}

QT_END_NAMESPACE
