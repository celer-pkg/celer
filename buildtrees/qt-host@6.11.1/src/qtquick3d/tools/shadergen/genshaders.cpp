// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "genshaders.h"

#include <QtCore/qdir.h>

#include <QtQml/qqmllist.h>

#include <QtQuick3D/private/qquick3dsceneenvironment_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dscenerenderer_p.h>
#include <QtQuick3D/private/qquick3dscenemanager_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>

// Lights
#include <QtQuick3D/private/qquick3dspotlight_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>

#include <QtQuick3DUtils/private/qqsbcollection_p.h>

#include <private/qssgrenderer_p.h>
#include <private/qssglayerrenderdata_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrhieffectsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgdebugdrawsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderroot_p.h>

#include <rhi/qshaderbaker.h>

static inline void qDryRunPrintQsbcAdd(const QByteArray &id)
{
    printf("Shader pipeline generated for (dry run):\n %s\n\n", qPrintable(id));
}

static void initBaker(QShaderBaker *baker, QRhi *rhi)
{
    Q_UNUSED(rhi); // that's a Null-backed rhi here anyways
    QVector<QShaderBaker::GeneratedShader> outputs;
    // TODO: For simplicity we're just going to add all off these for now.
    outputs.append({ QShader::SpirvShader, QShaderVersion(100) }); // Vulkan 1.0
    outputs.append({ QShader::HlslShader, QShaderVersion(50) }); // Shader Model 5.0
    outputs.append({ QShader::HlslShader, QShaderVersion(60) }); // Shader Model 6.0
    outputs.append({ QShader::MslShader, QShaderVersion(12) }); // Metal 1.2
    outputs.append({ QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs) }); // GLES 3.0+
    outputs.append({ QShader::GlslShader, QShaderVersion(140) }); // OpenGL 3.1+

    baker->setGeneratedShaders(outputs);
    baker->setGeneratedShaderVariants({ QShader::StandardShader });
}

GenShaders::GenShaders()
{
    sceneManager = new QQuick3DSceneManager;

    rhi = QRhi::create(QRhi::Null, nullptr);
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    std::unique_ptr<QSSGRhiContext> rhiContext = std::make_unique<QSSGRhiContext>(rhi);
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiContext.get());
    rhiCtxD->setCommandBuffer(cb);

    renderContext = std::make_shared<QSSGRenderContextInterface>(std::make_unique<QSSGBufferManager>(),
                                                                 std::make_unique<QSSGRenderer>(),
                                                                 std::make_shared<QSSGShaderLibraryManager>(),
                                                                 std::make_unique<QSSGShaderCache>(*rhiContext, &initBaker),
                                                                 std::make_unique<QSSGCustomMaterialSystem>(),
                                                                 std::make_unique<QSSGProgramGenerator>(),
                                                                 std::move(rhiContext));
    wa = new QQuick3DWindowAttachment(nullptr);
    wa->setRci(renderContext);
    sceneManager->wattached = wa;
}

GenShaders::~GenShaders() = default;

bool GenShaders::process(const MaterialParser::SceneData &sceneData,
                         QVector<QString> &qsbcFiles,
                         const QDir &outDir,
                         bool generateMultipleLights,
                         bool dryRun)
{
    Q_UNUSED(generateMultipleLights);

    const QString resourceFolderRelative = QSSGShaderCache::resourceFolder().mid(2);
    if (!dryRun && !outDir.exists(resourceFolderRelative)) {
        if (!outDir.mkpath(resourceFolderRelative)) {
            qDebug("Unable to create folder: %s", qPrintable(outDir.path() + QDir::separator() + resourceFolderRelative));
            return false;
        }
    }

    const QString outputFolder = outDir.canonicalPath() + QDir::separator() + resourceFolderRelative;

    QSSGRenderLayer layer;
    if (!layer.rootNode) {
        auto *rootNode = new QSSGRenderRoot;
        rootNode->addChild(layer);
        layer.rootNode = rootNode;
    }
    renderContext->renderer()->setViewport(QRect(QPoint(), QSize(888,666)));
    const auto &renderer = renderContext->renderer();
    QSSGLayerRenderData layerData(layer, *renderer);

    const auto &shaderLibraryManager = renderContext->shaderLibraryManager();
    const auto &shaderCache = renderContext->shaderCache();
    const auto &shaderProgramGenerator = renderContext->shaderProgramGenerator();

    bool aaIsDirty = false;
    bool temporalIsDirty = false;

    QQuick3DViewport *view3D = sceneData.viewport;
    Q_ASSERT(view3D);

    QVector<QSSGRenderGraphObject *> nodes;

    if (!view3D->camera()) {
        auto camera = new QQuick3DPerspectiveCamera();
        auto node = QQuick3DObjectPrivate::updateSpatialNode(camera, nullptr);
        QQuick3DObjectPrivate::get(camera)->spatialNode = node;
        nodes.append(node);
        view3D->setCamera(camera);
    }

    // Realize resources
    // Textures
    const auto &textures = sceneData.textures;
    for (const auto &tex : textures) {
        auto node = QQuick3DObjectPrivate::updateSpatialNode(tex, nullptr);
        auto obj = QQuick3DObjectPrivate::get(tex);
        obj->spatialNode = node;
        nodes.append(node);
    }

    // Free Materials (see also the model section)
    const auto &materials = sceneData.materials;
    for (const auto &mat : materials) {
        auto obj = QQuick3DObjectPrivate::get(mat);
        obj->sceneManager = sceneManager;
        auto node = QQuick3DObjectPrivate::updateSpatialNode(mat, nullptr);
        obj->spatialNode = node;
        nodes.append(node);
    }

    bool shadowPerspectivePass = false;
    bool shadowOrthoPass = false;

    // Lights
    const auto &lights = sceneData.lights;
    for (const auto &light : lights) {
        if (auto node = QQuick3DObjectPrivate::updateSpatialNode(light, nullptr)) {
            nodes.append(node);
            layer.addChild(static_cast<QSSGRenderNode &>(*node));
            const auto &lightNode = static_cast<const QSSGRenderLight &>(*node);
            if (lightNode.type == QSSGRenderLight::Type::DirectionalLight)
                shadowOrthoPass |= true;
            else
                shadowPerspectivePass |= true;
        }
    }

    // NOTE: Model.castsShadows; Model.receivesShadows; variants needs to be added for runtime support
    const auto &models = sceneData.models;
    for (const auto &model : models) {
        auto materialList = model->materials();
        for (int i = 0, e = materialList.count(&materialList); i != e; ++i) {
            auto mat = materialList.at(&materialList, i);
            auto obj = QQuick3DObjectPrivate::get(mat);
            obj->sceneManager = sceneManager;
            QSSGRenderGraphObject *node = nullptr;
            if (obj->type == QQuick3DObjectPrivate::Type::CustomMaterial) {
                auto customMatNode = new QSSGRenderCustomMaterial;
                customMatNode->incompleteBuildTimeObject = true;
                node = QQuick3DObjectPrivate::updateSpatialNode(mat, customMatNode);
                customMatNode->incompleteBuildTimeObject = false;
            } else {
                node = QQuick3DObjectPrivate::updateSpatialNode(mat, nullptr);
            }
            QQuick3DObjectPrivate::get(mat)->spatialNode = node;
            nodes.append(node);
        }
        if (auto instanceList = qobject_cast<QQuick3DInstanceList *>(model->instancing())) {
            auto obj = QQuick3DObjectPrivate::get(instanceList);
            auto node = QQuick3DObjectPrivate::updateSpatialNode(instanceList, nullptr);
            obj->spatialNode = node;
            nodes.append(node);
        }

        auto node = QQuick3DObjectPrivate::updateSpatialNode(model, nullptr);
        QQuick3DObjectPrivate::get(model)->spatialNode = node;
        nodes.append(node);
    }

    // Ensure all spatial node types are in the render tree
    for (auto *node : std::as_const(nodes)) {
        if (QSSGRenderGraphObject::isNodeType(node->type)) {
            QSSGRenderNode *n = static_cast<QSSGRenderNode *>(node);
            if (!n->parent)
                layer.addChild(*n);
        }
    }

    QQuick3DRenderLayerHelpers::updateLayerNodeHelper(*view3D, renderContext, layer, aaIsDirty, temporalIsDirty);

    const QString outCollectionFile = outputFolder + QString::fromLatin1(QSSGShaderCache::shaderCollectionFile());
    QQsbIODeviceCollection qsbc(outCollectionFile);
    if (!dryRun && !qsbc.map(QQsbIODeviceCollection::Write))
        return false;

    QByteArray shaderString;
    const auto generateShaderForModel = [&](QSSGRenderModel &model) {
        layerData.resetForFrame();
        if (!model.parent)
            layer.addChild(model);
        layerData.layer.rootNode->reindex();
        layerData.prepareForRender();

        const auto &features = layerData.getShaderFeatures();

        const auto &propertyTable = layerData.getDefaultMaterialPropertyTable();

        const auto &opaqueObjects = layerData.getSortedOpaqueRenderableObjects(*layerData.renderedCameras[0]);
        const auto &transparentObjects = layerData.getSortedTransparentRenderableObjects(*layerData.renderedCameras[0]);

        QSSGRenderableObject *renderable = nullptr;
        if (!opaqueObjects.isEmpty())
            renderable = opaqueObjects[0].obj;
        else if (!transparentObjects.isEmpty())
            renderable = transparentObjects[0].obj;

        auto generateShader = [&](const QSSGShaderFeatures &features) {
            if ((renderable->type == QSSGSubsetRenderable::Type::DefaultMaterialMeshSubset)) {
                auto shaderPipeline = QSSGRendererPrivate::generateRhiShaderPipelineImpl(*static_cast<QSSGSubsetRenderable *>(renderable), *shaderLibraryManager, *shaderCache, *shaderProgramGenerator, propertyTable, features, {/*shaderAugmentation*/}, shaderString);
                if (shaderPipeline != nullptr) {
                    const auto qsbcFeatureList = QQsbCollection::toFeatureSet(features);
                    const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, qsbcFeatureList);
                    const auto vertexStage = shaderPipeline->vertexStage();
                    const auto fragmentStage = shaderPipeline->fragmentStage();
                    if (vertexStage && fragmentStage) {
                        if (dryRun)
                            qDryRunPrintQsbcAdd(shaderString);
                        else
                            qsbc.addEntry(qsbcKey, { shaderString, qsbcFeatureList, vertexStage->shader(), fragmentStage->shader() });
                    }
                }
            } else if ((renderable->type == QSSGSubsetRenderable::Type::CustomMaterialMeshSubset)) {
                Q_ASSERT(!layerData.renderedCameras.isEmpty());
                QSSGSubsetRenderable &cmr(static_cast<QSSGSubsetRenderable &>(*renderable));
                auto pipelineState = layerData.getPipelineState();
                const auto &cms = renderContext->customMaterialSystem();
                const auto &material = static_cast<const QSSGRenderCustomMaterial &>(cmr.getMaterial());
                auto shaderPipeline = cms->shadersForCustomMaterial(&pipelineState,
                                                                    material,
                                                                    cmr,
                                                                    propertyTable,
                                                                    features);

                if (shaderPipeline) {
                    shaderString = material.m_shaderPathKey[QSSGRenderCustomMaterial::RegularShaderPathKeyIndex];
                    const auto qsbcFeatureList = QQsbCollection::toFeatureSet(features);
                    const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, qsbcFeatureList);
                    const auto vertexStage = shaderPipeline->vertexStage();
                    const auto fragmentStage = shaderPipeline->fragmentStage();
                    if (vertexStage && fragmentStage) {
                        if (dryRun)
                            qDryRunPrintQsbcAdd(shaderString);
                        else
                            qsbc.addEntry(qsbcKey, { shaderString, qsbcFeatureList, vertexStage->shader(), fragmentStage->shader() });
                    }
                }
            }
        };

        if (renderable) {
            generateShader(features);

            QSSGShaderFeatures depthPassFeatures;
            depthPassFeatures.set(QSSGShaderFeatures::Feature::DepthPass, true);
            generateShader(depthPassFeatures);

            if (shadowPerspectivePass) {
                QSSGShaderFeatures shadowPassFeatures;
                shadowPassFeatures.set(QSSGShaderFeatures::Feature::PerspectiveShadowPass, true);
                generateShader(shadowPassFeatures);
            }

            if (shadowOrthoPass) {
                QSSGShaderFeatures shadowPassFeatures;
                shadowPassFeatures.set(QSSGShaderFeatures::Feature::OrthoShadowPass, true);
                generateShader(shadowPassFeatures);
            }
        }
        layer.removeChild(model);
    };

    for (const auto &model : models)
        generateShaderForModel(static_cast<QSSGRenderModel &>(*QQuick3DObjectPrivate::get(model)->spatialNode));

    // Let's generate some shaders for the "free" materials as well.
    QSSGRenderModel model; // dummy
    model.meshPath = QSSGRenderPath("#Cube");
    for (const auto &mat : materials) {
        model.materials = { QQuick3DObjectPrivate::get(mat)->spatialNode };
        generateShaderForModel(model);
    }

    // Now generate the shaders for the effects
    const auto generateEffectShader = [&](QQuick3DEffect &effect) {
        auto obj = QQuick3DObjectPrivate::get(&effect);
        obj->sceneManager = sceneManager;
        QSSGRenderEffect *renderEffect = new QSSGRenderEffect;
        renderEffect->incompleteBuildTimeObject = true;
        if (auto ret = QQuick3DObjectPrivate::updateSpatialNode(&effect, renderEffect))
            Q_ASSERT(ret == renderEffect);
        renderEffect->incompleteBuildTimeObject = false;
        obj->spatialNode = renderEffect;
        nodes.append(renderEffect);

        const auto &commands = renderEffect->commands;
        for (const QSSGCommand *command : commands) {
            if (command->m_type == CommandType::BindShader) {
                auto bindShaderCommand = static_cast<const QSSGBindShader &>(*command);
                for (const auto isYUpInFramebuffer : { true, false }) { // Generate effects for both up-directions.
                    const auto shaderPipeline = QSSGRhiEffectSystem::buildShaderForEffect(bindShaderCommand,
                                                                                          *shaderProgramGenerator,
                                                                                          *shaderLibraryManager,
                                                                                          *shaderCache,
                                                                                          isYUpInFramebuffer,
                                                                                          1); // no multiview support here yet
                    if (shaderPipeline) {
                        const auto &key = bindShaderCommand.m_shaderPathKey;
                        const QSSGShaderFeatures features = shaderLibraryManager->getShaderMetaData(key, QSSGShaderCache::ShaderType::Fragment).features;
                        const auto qsbcFeatureList = QQsbCollection::toFeatureSet(features);
                        QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(key, qsbcFeatureList);
                        const auto vertexStage = shaderPipeline->vertexStage();
                        const auto fragmentStage = shaderPipeline->fragmentStage();
                        if (vertexStage && fragmentStage) {
                            if (dryRun)
                                qDryRunPrintQsbcAdd(key);
                            else
                                qsbc.addEntry(qsbcKey, { key, qsbcFeatureList, vertexStage->shader(), fragmentStage->shader() });
                        }
                    }
                }
            }
        }
    };

    // Effects
    if (sceneData.viewport && sceneData.viewport->environment()) {
        auto &env = *sceneData.viewport->environment();
        auto effects = env.effects();
        const auto effectCount = effects.count(&effects);
        for (int i = 0; i < effectCount; ++i) {
            auto effect = effects.at(&effects, i);
            generateEffectShader(*effect);
        }
    }

    // Free Effects
    for (const auto &effect : std::as_const(sceneData.effects))
        generateEffectShader(*effect);

    if (!qsbc.availableEntries().isEmpty())
        qsbcFiles.push_back(resourceFolderRelative + QDir::separator() + QString::fromLatin1(QSSGShaderCache::shaderCollectionFile()));
    qsbc.unmap();

    auto &children = layer.children;
    for (auto it = children.begin(), end = children.end(); it != end;)
        children.remove(*it++);

    qDeleteAll(nodes);

    return true;
}
