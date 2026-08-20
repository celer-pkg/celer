// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrenderableobjects_p.h"
#include "qssglayerrenderdata_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>

QT_BEGIN_NAMESPACE
struct QSSGRenderableImage;
class QSSGSubsetRenderable;

QSSGModelContext::QSSGModelContext(const QSSGRenderModel &inModel,
                                   const QMatrix4x4 &inGlobalTransform,
                                   const QMatrix3x3 &inNormalMatrix,
                                   const QSSGRenderMvpArray &inModelViewProjections)
    : model(inModel)
    , globalTransform(inGlobalTransform)
    , normalMatrix(inNormalMatrix)
    , modelViewProjections(inModelViewProjections)
{

}

QSSGSubsetRenderable::QSSGSubsetRenderable(Type type,
                                           QSSGRenderableObjectFlags inFlags,
                                           const QVector3D &inWorldCenterPt,
                                           QSSGRenderer *rendr,
                                           const QSSGRenderSubset &inSubset,
                                           const QSSGModelContext &inModelContext,
                                           float inOpacity,
                                           quint32 inSubsetLevelOfDetail,
                                           const QSSGRenderGraphObject &mat,
                                           QSSGRenderableImage *inFirstImage,
                                           QSSGShaderDefaultMaterialKey inShaderKey,
                                           const QSSGShaderLightListView &inLights,
                                           bool anyLightHasShadows)
    : QSSGRenderableObject(type,
                           inFlags,
                           inWorldCenterPt,
                           inSubset.bounds,
                           inModelContext.model.m_depthBiasSq,
                           inModelContext.model.instancingLodMin,
                           inModelContext.model.instancingLodMax)
    , subsetLevelOfDetail(inSubsetLevelOfDetail)
    , renderer(rendr)
    , modelContext(inModelContext)
    , subset(inSubset)
    , opacity(inOpacity)
    , material(mat)
    , firstImage(inFirstImage)
    , shaderDescription(inShaderKey)
    , lights(inLights)
{
    if (mat.type == QSSGRenderGraphObject::Type::CustomMaterial)
        depthWriteMode = static_cast<const QSSGRenderCustomMaterial *>(&mat)->m_depthDrawMode;
    else
        depthWriteMode = static_cast<const QSSGRenderDefaultMaterial *>(&mat)->depthDrawMode;

    const bool modelBlendParticle = (inModelContext.model.particleBuffer != nullptr
                                     && inModelContext.model.particleBuffer->particleCount());
    if (modelBlendParticle)
        globalBounds = inModelContext.model.particleBuffer->bounds();
    else
        globalBounds.transform(inModelContext.globalTransform);

    // Do we need instanced bounds
    const QSSGRenderInstanceTable *instanceTable = inModelContext.model.instanceTable;
    isInstanced = instanceTable != nullptr;
    if (!instanceTable || !anyLightHasShadows || !(inFlags.castsShadows() || inFlags.receivesShadows()))
        return;

    // Check for specified instancing shadow bounds
    QSSGBounds3 bounds = QSSGBounds3(instanceTable->getShadowBoundsMinimum(), instanceTable->getShadowBoundsMaximum());
    if (!bounds.isEmpty() && bounds.isFinite()) {
        globalBoundsInstancing = bounds;
        return;
    }

    // Calculate instancing shadow bounds
    const auto points = globalBounds.toQSSGBoxPointsNoEmptyCheck();
    for (int i = 0; i < instanceTable->count(); ++i) {
        const QMatrix4x4 transform = instanceTable->getTransform(i);
        for (const QVector3D &point : points) {
            globalBoundsInstancing.include(transform.map(point));
        }
    }
}

QSSGParticlesRenderable::QSSGParticlesRenderable(QSSGRenderableObjectFlags inFlags,
                                                 const QVector3D &inWorldCenterPt,
                                                 QSSGRenderer *rendr,
                                                 const QMatrix4x4 &inGlobalTransform,
                                                 const QSSGRenderParticles &inParticles,
                                                 QSSGRenderableImage *inFirstImage,
                                                 QSSGRenderableImage *inColorTable,
                                                 const QSSGShaderLightListView &inLights,
                                                 float inOpacity,
                                                 QSSGShaderParticleMaterialKey inShaderKey)
    : QSSGRenderableObject(Type::Particles,
                           inFlags,
                           inWorldCenterPt,
                           inParticles.m_particleBuffer.bounds(),
                           inParticles.m_depthBiasSq)
    , renderer(rendr)
    , particles(inParticles)
    , firstImage(inFirstImage)
    , colorTable(inColorTable)
    , lights(inLights)
    , globalTransform(inGlobalTransform)
    , opacity(inOpacity)
    , shaderDescription(inShaderKey)
{
    // Bounds are in global space for _model blend_ particles
    globalBounds = inParticles.m_particleBuffer.bounds();
    if (inParticles.type != QSSGRenderParticles::Type::ModelBlendParticle)
        globalBounds.transform(inGlobalTransform);
}

QT_END_NAMESPACE
