// Copyright (C) 2025 The Qt Company Ltd.
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

#include <QtQuick3D/private/qquick3dspotlight_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>

#include <QtQuick3DUtils/private/qqsbcollection_p.h>

#include <private/qssgrenderer_p.h>
#include <private/qssglayerrenderdata_p.h>
#include <private/qssgrhiparticles_p.h>
#include <private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderroot_p.h>

#include <rhi/qshaderbaker.h>

static inline void qDryRunPrintQsbcAdd(const QByteArray &id)
{
    printf("Shader pipeline generated for (dry run):\n %s\n\n", qPrintable(id));
}

GenShaders::GenShaders()
{
    sceneManager.reset(new QQuick3DSceneManager);

    rhi.reset(QRhi::create(QRhi::Null, nullptr));
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    std::unique_ptr<QSSGRhiContext> rhiContext = std::make_unique<QSSGRhiContext>(rhi.get());
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiContext.get());
    rhiCtxD->setCommandBuffer(cb);

    renderContext = std::make_shared<QSSGRenderContextInterface>(std::make_unique<QSSGBufferManager>(),
                                                                 std::make_unique<QSSGRenderer>(),
                                                                 std::make_shared<QSSGShaderLibraryManager>(),
                                                                 std::make_unique<QSSGShaderCache>(*rhiContext, &QSSGShaderCache::initBakerForPersistentUse),
                                                                 std::make_unique<QSSGCustomMaterialSystem>(),
                                                                 std::make_unique<QSSGProgramGenerator>(),
                                                                 std::move(rhiContext));
}

GenShaders::~GenShaders() = default;

bool GenShaders::process(QVector<QString> &qsbcFiles,
                         const QDir &outDir,
                         bool verbose,
                         bool dryRun)
{
    const QString resourceFolderRelative = QSSGShaderCache::resourceFolder().mid(2);
    if (!dryRun && !outDir.exists(resourceFolderRelative)) {
        if (!outDir.mkpath(resourceFolderRelative)) {
            qDebug("Unable to create folder: %s", qPrintable(outDir.path() + QDir::separator() + resourceFolderRelative));
            return false;
        }
    }

    const QString outputFolder = outDir.canonicalPath() + QDir::separator() + resourceFolderRelative;

    renderContext->renderer()->setViewport(QRect(QPoint(), QSize(888,666)));
    const auto &renderer = renderContext->renderer();

    const QString outCollectionFile = outputFolder + QString::fromLatin1(QSSGShaderCache::particleShaderCollectionFile());
    QQsbIODeviceCollection qsbc(outCollectionFile);
    if (!dryRun && !qsbc.map(QQsbIODeviceCollection::Write))
        return false;

    QSSGRenderLayer layer;
    QSSGRenderRoot rootNode;
    if (!layer.rootNode) {
        rootNode.addChild(layer);
        layer.rootNode = &rootNode;
    }
    QSSGLayerRenderData layerRenderData(layer, *renderer);

    layerRenderData.prepareForRender();

    // Disable shader caching so that we always generate the shaders
    QSSGParticleRenderer::setShaderCacheEnabled(false);

    const auto &features = QSSGParticleRenderer::particleShaderFeatures(layerRenderData.getShaderFeatures());
    const auto &propertyTable = layerRenderData.getParticleMaterialPropertyTable();

    /*
    QSSGShaderKeyBoolean m_isSpriteLinear;
    QSSGShaderKeyBoolean m_isColorTableLinear;
    QSSGShaderKeyBoolean m_hasLighting;
    QSSGShaderKeyBoolean m_isLineParticle;
    QSSGShaderKeyBoolean m_isMapped;
    QSSGShaderKeyBoolean m_isAnimated;
    QSSGShaderKeyUnsigned<3> m_viewCount;
    QSSGShaderKeyUnsigned<3> m_orderIndependentTransparency;
     */
    /* Only generate srgb */
    const bool isSpriteLinear = false;
    const bool isColorTableLinear = false;
    /* Number of shaders to generate.
       We'll add the bits here, but skip generating for undefined values in the loop.
     */
    uint32_t permutations = 2 * 2 * 2 * 2 * 2 * 4;

    for (uint32_t i = 0; i < permutations; i++) {
        QByteArray shaderString;

        QSSGRenderableObjectFlags flags;
        flags.setHasTransparency(true);
        QSSGRenderParticles particles;
        QSSGShaderParticleMaterialKey &key(particles.materialKey);

        propertyTable.m_isSpriteLinear.setValue(key, isSpriteLinear);
        propertyTable.m_isColorTableLinear.setValue(key, isColorTableLinear);

        uint32_t perm = i;
        propertyTable.m_hasLighting.setValue(key, (perm&1));
        perm >>= 1;
        propertyTable.m_isLineParticle.setValue(key, (perm&1));
        perm >>= 1;
        propertyTable.m_isMapped.setValue(key, (perm&1));
        perm >>= 1;
        propertyTable.m_isAnimated.setValue(key, (perm&1));
        perm >>= 1;
        propertyTable.m_viewCount.setValue(key, (perm&1) ? 2 : 1);
        perm >>= 1;
        int oit = (perm&3);
        if (oit > int(QSSGRenderLayer::OITMethod::LinkedList))
            continue;
        propertyTable.m_orderIndependentTransparency.setValue(key, oit);

        QSSGParticlesRenderable particlesRenderable(flags, {}, renderer.get(), {}, particles, nullptr, nullptr, {}, 1.0, key);
        auto shaderPipeline = QSSGParticleRenderer::generateRhiShaderPipeline(*renderer.get(), particlesRenderable, features, shaderString, propertyTable);

        if (shaderPipeline != nullptr) {
            const auto qsbcFeatureList = QQsbCollection::toFeatureSet(features);
            const QByteArray qsbcKey = QQsbCollection::EntryDesc::generateSha(shaderString, qsbcFeatureList);
            const auto vertexStage = shaderPipeline->vertexStage();
            const auto fragmentStage = shaderPipeline->fragmentStage();
            if (vertexStage && fragmentStage) {
                if (verbose)
                    qDebug () << shaderString;
                if (dryRun)
                    qDryRunPrintQsbcAdd(shaderString);
                else
                    qsbc.addEntry(qsbcKey, { shaderString, qsbcFeatureList, vertexStage->shader(), fragmentStage->shader() });
            }
        }
    }

    if (!qsbc.availableEntries().isEmpty())
        qsbcFiles.push_back(resourceFolderRelative + QDir::separator() + QString::fromLatin1(QSSGShaderCache::particleShaderCollectionFile()));
    qsbc.unmap();

    return true;
}
