// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercommands_p.h>
#include "../qssgrendercontextcore.h"
#include "../rendererimpl/qssglayerrenderdata_p.h"

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

QSSGRenderEffect::QSSGRenderEffect() : QSSGRenderGraphObject(Type::Effect) {}

QSSGRenderEffect::~QSSGRenderEffect()
{
    resetCommands();
}

void QSSGRenderEffect::setFlag(QSSGRenderEffect::Flags flag, bool enabled)
{
    if (enabled)
        flags |= FlagT(flag);
    else
        flags &= ~FlagT(flag);
}

// Suffix snippets added to the end of the shader strings. These are appended
// after processing so it must be valid GLSL as-is, no more magic keywords.

static const char *effect_vertex_main_pre =
        "void main()\n"
        "{\n"
        "    qt_inputUV = attr_uv;\n"
        "    qt_textureUV = qt_effectTextureMapUV(attr_uv);\n"
        "    vec4 qt_vertPosition = vec4(attr_pos, 1.0);\n"
        "#if QSHADER_VIEW_COUNT >= 2\n"
        "    qt_viewIndex = gl_ViewIndex;\n"
        "#else\n"
        "    qt_viewIndex = 0;\n"
        "#endif\n"
        "    qt_customMain(qt_vertPosition.xyz);\n";

static const char *effect_vertex_main_position =
        "    gl_Position = qt_modelViewProjection * qt_vertPosition;\n";

static const char *effect_vertex_main_post =
        "}\n";

static const char *effect_fragment_main =
        "void main()\n"
        "{\n"
        "    qt_customMain();\n"
        "}\n";

static const char *effect_fragment_main_with_tonemapping =
        "#include \"tonemapping.glsllib\"\n"
        "void main()\n"
        "{\n"
        "    qt_customMain();\n"
        "    fragOutput = qt_tonemap(fragOutput);\n"
        "}\n";

void QSSGRenderEffect::finalizeShaders(const QSSGRenderLayer &layer, QSSGRenderContextInterface *renderContext)
{
    Q_UNUSED(layer);

    // this is called on every frame, so do nothing if there are no changes
    if (!shaderPrepData.valid)
        return;

    QRhi *rhi = renderContext->rhiContext()->rhi();

    for (int i = 0, ie = shaderPrepData.passes.size(); i != ie; ++i) {
        const ShaderPrepPassData &pass(shaderPrepData.passes[i]);

        // The fragment shader of the last pass of the last effect may need to
        // perform the built-in tonemapping.
        const bool isLastEffect = m_nextEffect == nullptr;
        const bool isLastPass = i == ie - 1;
        const bool shouldTonemapIfEnabled = isLastEffect && isLastPass;

        QSSGShaderFeatures features;
        QByteArray completeVertexShader;
        QByteArray completeFragmentShader;
        QByteArray sourceCodeForHash;

        const bool multiview = layer.viewCount >= 2;
        const int srcIdx = multiview ? QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex : QSSGRenderCustomMaterial::RegularShaderPathKeyIndex;

        if (!pass.vertexShaderCode[srcIdx].isEmpty()) {
            QByteArray code = pass.vertexShaderCode[srcIdx];
            // add the real main(), with or without assigning gl_Position at the end
            code.append(effect_vertex_main_pre);
            if (!pass.vertexMetaData[srcIdx].flags.testFlag(QSSGCustomShaderMetaData::OverridesPosition))
                code.append(effect_vertex_main_position);
            code.append(effect_vertex_main_post);
            completeVertexShader = code;
            sourceCodeForHash += code;
        }

        if (!pass.fragmentShaderCode[srcIdx].isEmpty()) {
            QByteArray code = pass.fragmentShaderCode[srcIdx];
            if (shouldTonemapIfEnabled)
                code.append(effect_fragment_main_with_tonemapping);
            else
                code.append(effect_fragment_main);
            completeFragmentShader = code;
            sourceCodeForHash += code;
        }

        QByteArray shaderPathKey = pass.shaderPathKeyPrefix;
        shaderPathKey.append(':' + QCryptographicHash::hash(sourceCodeForHash, QCryptographicHash::Algorithm::Sha1).toHex());

        // QSSGRhiEffectSystem will vary the vertex shader code based on this
        // flag from the QRhi. It is therefore important to capture this in the
        // cache key as well.
        shaderPathKey.append(rhi->isYUpInFramebuffer() ? QByteArrayLiteral(":1") : QByteArrayLiteral(":0"));

        if (shouldTonemapIfEnabled) {
            // This does not always mean there will be tonemapping: if the mode
            // is TonemapModeNone, then no extra feature defines are set, and
            // so qt_tonemap() in the shader will not alter the color.
            const QSSGRenderLayer::TonemapMode tonemapMode = layer.tonemapMode;
            shaderPathKey.append(':' + QByteArray::number(int(tonemapMode)));
            QSSGLayerRenderData::setTonemapFeatures(features, tonemapMode);
        }

        shaderPathKey.append(multiview ? QByteArrayLiteral(":1") : QByteArrayLiteral(":0"));

        // Now that the final shaderPathKey is known, store the source and
        // related data; it will be retrieved later by the QSSGRhiEffectSystem.
        if (!completeVertexShader.isEmpty()) {
            renderContext->shaderLibraryManager()->setShaderSource(shaderPathKey,
                                                                   QSSGShaderCache::ShaderType::Vertex,
                                                                   completeVertexShader,
                                                                   pass.vertexMetaData[srcIdx]);
        }
        if (!completeFragmentShader.isEmpty()) {
            QSSGCustomShaderMetaData metaData = pass.fragmentMetaData[srcIdx];
            metaData.features = features;
            renderContext->shaderLibraryManager()->setShaderSource(shaderPathKey,
                                                                   QSSGShaderCache::ShaderType::Fragment,
                                                                   completeFragmentShader,
                                                                   metaData);
        }

        // and update the command
        delete commands[pass.bindShaderCmdIndex];
        commands[pass.bindShaderCmdIndex] = new QSSGBindShader(shaderPathKey);
    }

    shaderPrepData.valid = false;
}

void QSSGRenderEffect::resetCommands()
{
    qDeleteAll(commands);
    commands.clear();
    shaderPrepData.passes.clear();
}

QT_END_NAMESPACE
