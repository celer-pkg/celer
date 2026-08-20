// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_IMPL_RENDERABLE_OBJECTS_H
#define QSSG_RENDER_IMPL_RENDERABLE_OBJECTS_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderparticles_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderkeys_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderreflectionprobe_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderclippingfrustum_p.h>

#include <QtQuick3DUtils/private/qssginvasivelinkedlist_p.h>

QT_BEGIN_NAMESPACE

class QSSGRenderer;
class QSSGRenderableObject;
struct QSSGRenderItem2D;

enum class QSSGRenderableObjectFlag : quint32
{
    HasTransparency = 1 << 0,
    CompletelyTransparent = 1 << 1,
    Dirty = 1 << 2,
    CastsShadows = 1 << 3,
    ReceivesShadows = 1 << 4,
    HasAttributePosition = 1 << 5,
    HasAttributeNormal = 1 << 6,
    HasAttributeTexCoord0 = 1 << 7,
    HasAttributeTexCoord1 = 1 << 8,
    HasAttributeTangent = 1 << 9,
    HasAttributeBinormal = 1 << 10,
    HasAttributeColor = 1 << 11,
    HasAttributeJointAndWeight = 1 << 12,
    IsPointsTopology = 1 << 13,
    // The number of target models' attributes are too many
    // to store in a renderable flag.
    // They will be recorded in shaderKey.
    HasAttributeMorphTarget = 1 << 14,
    RequiresScreenTexture = 1 << 15,
    ReceivesReflections = 1 << 16,
    UsedInBakedLighting = 1 << 17,
    RendersWithLightmap = 1 << 18,
    HasAttributeTexCoordLightmap = 1 << 19,
    CastsReflections = 1 << 20,
    RequiresNormalTexture = 1 << 21,
    IsMotionVectorParticipant = 1 << 22
};

struct QSSGRenderableObjectFlags : public QFlags<QSSGRenderableObjectFlag>
{
    void setHasTransparency(bool inHasTransparency)
    {
        setFlag(QSSGRenderableObjectFlag::HasTransparency, inHasTransparency);
    }
    bool hasTransparency() const { return this->operator&(QSSGRenderableObjectFlag::HasTransparency); }
    void setCompletelyTransparent(bool inTransparent)
    {
        setFlag(QSSGRenderableObjectFlag::CompletelyTransparent, inTransparent);
    }
    bool isCompletelyTransparent() const
    {
        return this->operator&(QSSGRenderableObjectFlag::CompletelyTransparent);
    }
    void setDirty(bool inDirty) { setFlag(QSSGRenderableObjectFlag::Dirty, inDirty); }
    bool isDirty() const { return this->operator&(QSSGRenderableObjectFlag::Dirty); }

    void setCastsShadows(bool inCastsShadows) { setFlag(QSSGRenderableObjectFlag::CastsShadows, inCastsShadows); }
    bool castsShadows() const { return this->operator&(QSSGRenderableObjectFlag::CastsShadows); }

    void setReceivesShadows(bool inReceivesShadows) { setFlag(QSSGRenderableObjectFlag::ReceivesShadows, inReceivesShadows); }
    bool receivesShadows() const { return this->operator&(QSSGRenderableObjectFlag::ReceivesShadows); }

    void setReceivesReflections(bool inReceivesReflections) { setFlag(QSSGRenderableObjectFlag::ReceivesReflections, inReceivesReflections); }
    bool receivesReflections() const { return this->operator&(QSSGRenderableObjectFlag::ReceivesReflections); }

    void setCastsReflections(bool inCastsReflections) { setFlag(QSSGRenderableObjectFlag::CastsReflections, inCastsReflections); }
    bool castsReflections() const { return this->operator&(QSSGRenderableObjectFlag::CastsReflections); }

    void setUsedInBakedLighting(bool inUsedInBakedLighting) { setFlag(QSSGRenderableObjectFlag::UsedInBakedLighting, inUsedInBakedLighting); }
    bool usedInBakedLighting() const { return this->operator&(QSSGRenderableObjectFlag::UsedInBakedLighting); }

    void setRendersWithLightmap(bool inRendersWithLightmap) { setFlag(QSSGRenderableObjectFlag::RendersWithLightmap, inRendersWithLightmap); }
    bool rendersWithLightmap() const { return this->operator&(QSSGRenderableObjectFlag::RendersWithLightmap); }

    void setHasAttributePosition(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributePosition, b); }
    bool hasAttributePosition() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributePosition); }

    void setHasAttributeNormal(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeNormal, b); }
    bool hasAttributeNormal() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeNormal); }

    void setHasAttributeTexCoord0(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeTexCoord0, b); }
    bool hasAttributeTexCoord0() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeTexCoord0); }

    void setHasAttributeTexCoord1(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeTexCoord1, b); }
    bool hasAttributeTexCoord1() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeTexCoord1); }

    void setHasAttributeTexCoordLightmap(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeTexCoordLightmap, b); }
    bool hasAttributeTexCoordLightmap() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeTexCoordLightmap); }

    void setHasAttributeTangent(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeTangent, b); }
    bool hasAttributeTangent() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeTangent); }

    void setHasAttributeBinormal(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeBinormal, b); }
    bool hasAttributeBinormal() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeBinormal); }

    void setHasAttributeColor(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeColor, b); }
    bool hasAttributeColor() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeColor); }

    void setHasAttributeJointAndWeight(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeJointAndWeight, b);
    }
    bool hasAttributeJointAndWeight() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeJointAndWeight); }

    void setHasAttributeMorphTarget(bool b) { setFlag(QSSGRenderableObjectFlag::HasAttributeMorphTarget, b);
    }
    bool hasAttributeMorphTarget() const { return this->operator&(QSSGRenderableObjectFlag::HasAttributeMorphTarget); }

    void setPointsTopology(bool v)
    {
        setFlag(QSSGRenderableObjectFlag::IsPointsTopology, v);
    }
    bool isPointsTopology() const
    {
        return this->operator&(QSSGRenderableObjectFlag::IsPointsTopology);
    }
    void setRequiresScreenTexture(bool v)
    {
        setFlag(QSSGRenderableObjectFlag::RequiresScreenTexture, v);
    }
    bool requiresScreenTexture() const {
        return this->operator&(QSSGRenderableObjectFlag::RequiresScreenTexture);
    }

    void setRequiresNormalTexture(bool v)
    {
        setFlag(QSSGRenderableObjectFlag::RequiresNormalTexture, v);
    }
    bool requiresNormalTexture() const {
        return this->operator&(QSSGRenderableObjectFlag::RequiresNormalTexture);
    }
    void setMotionVectorParticipant(bool v) { setFlag(QSSGRenderableObjectFlag::IsMotionVectorParticipant, v); }
    bool isMotionVectorParticipant() const { return this->operator&(QSSGRenderableObjectFlag::IsMotionVectorParticipant); }
};

struct QSSGShaderLight
{
    QSSGRenderLight *light = nullptr;
    bool shadows = false;
    QVector3D direction;

    inline bool operator < (const QSSGShaderLight &o) const
    {
        // sort by light type
        if (light->type < o.light->type)
            return true;
        // then shadow lights first
        if (shadows > o.shadows)
            return true;
        return false;
    }
};

struct QSSGShaderReflectionProbe
{
    QVector3D probeCubeMapCenter;
    QVector3D probeBoxMax;
    QVector3D probeBoxMin;
    bool enabled = false;
    int parallaxCorrection = 0;
};

// Having this as a QVLA is beneficial mainly because QVector would need to
// detach somewhere in QSSGLayerRenderPreparationData::prepareForRender so the
// implicit sharing's benefits do not outweigh the cost of allocations in this case.
typedef QVarLengthArray<QSSGShaderLight, 16> QSSGShaderLightList;
using QSSGShaderLightListView = QSSGDataView<QSSGShaderLight>;

struct QSSGRenderableNodeEntry
{
    enum Overridden : quint16
    {
        Original = 0,
        Disabled = 0x1,
        GlobalTransform = 0x2,
        Materials = 0x4,
        GlobalOpacity = 0x8,
    };

    QSSGRenderNode *node = nullptr;
    mutable QSSGShaderLightListView lights;
    mutable quint16 overridden { Original };

    // FIXME: This is just for the extension API and
    // should be removed in the future.
    struct ExtensionOverrides
    {
        QMatrix4x4 globalTransform;
        QVector<QSSGRenderGraphObject *> materials;
        float globalOpacity { 1.0f };
    } mutable extOverrides;

    bool isNull() const { return (node == nullptr); }
    QSSGRenderableNodeEntry() = default;
    QSSGRenderableNodeEntry(QSSGRenderNode &inNode) : node(&inNode) {}
    QSSGRenderableNodeEntry(QSSGRenderNode *inNode) : node(inNode) {}
};

// Used for sorting
struct QSSGRenderableObjectHandle
{
    QSSGRenderableObjectHandle() = default;
    QSSGRenderableObjectHandle(QSSGRenderableObject *o, float camDistSq, QSSGRenderNodeTag tag)
        : obj(o)
        , cameraDistanceSq(camDistSq)
        , tag(tag)
    {}
    QSSGRenderableObject *obj = nullptr;
    float cameraDistanceSq = 0.0f;
    QSSGRenderNodeTag tag;
};
Q_DECLARE_TYPEINFO(QSSGRenderableObjectHandle, Q_PRIMITIVE_TYPE);

using QSSGRenderableObjectList = QVector<QSSGRenderableObjectHandle>;

class QSSGRenderableObject
{
    Q_DISABLE_COPY_MOVE(QSSGRenderableObject)
public:
    enum class Type : quint8
    {
        DefaultMaterialMeshSubset,
        CustomMaterialMeshSubset,
        Particles
    };

    // Variables used for picking
    const QSSGBounds3 &bounds;
    QSSGBounds3 globalBounds;

    // Used for shadow map bounds when model has instancing
    QSSGBounds3 globalBoundsInstancing;

    QSSGRenderableObjectFlags renderableFlags;
    // For rough sorting for transparency and for depth
    QVector3D worldCenterPoint;
    float depthBiasSq; // Squared as our sorting is based on the square distance!
    float camdistSq = 0.0f;
    QSSGDepthDrawMode depthWriteMode = QSSGDepthDrawMode::OpaqueOnly;
    const Type type;
    float instancingLodMin = -1;
    float instancingLodMax = -1;
    bool isInstanced = false;

    QSSGRenderableObject(Type ty,
                         QSSGRenderableObjectFlags inFlags,
                         const QVector3D &inWorldCenterPt,
                         const QSSGBounds3 &inBounds,
                         float inDepthBias,
                         float inMinThreshold = -1,
                         float inMaxThreshold = -1)

        : bounds(inBounds)
        , globalBounds(inBounds)
        , renderableFlags(inFlags)
        , worldCenterPoint(inWorldCenterPt)
        , depthBiasSq(inDepthBias)
        , type(ty)
        , instancingLodMin(inMinThreshold)
        , instancingLodMax(inMaxThreshold)
    {
    }
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGRenderableObject>::value);

struct QSSGRenderCameraData
{
    QMatrix4x4 viewProjection;
    std::optional<QSSGClippingFrustum> clippingFrustum;
    QVector3D direction { 0.0f, 0.0f, -1.0f };
    QVector3D position;
};

using QSSGRenderCameraList = QVarLengthArray<QSSGRenderCamera *, 2>;
using QSSGRenderCameraDataList = QVarLengthArray<QSSGRenderCameraData, 2>;
using QSSGRenderMvpArray = std::array<QMatrix4x4, 2>; // cannot be dynamic due to QSSGModelContext, must stick with 2 for now

class QSSGSubsetRenderable;

// Different subsets from the same model will get the same
// model context so we can generate the MVP and normal matrix once
// and only once per subset.
class Q_AUTOTEST_EXPORT QSSGModelContext
{
    Q_DISABLE_COPY_MOVE(QSSGModelContext)
public:
    QSSGModelContext(const QSSGRenderModel &inModel,
                     const QMatrix4x4 &globalTransform,
                     const QMatrix3x3 &inNormalMatrix,
                     const QSSGRenderMvpArray &inModelViewProjections);

    const QSSGRenderModel &model;
    const QMatrix4x4 globalTransform;
    const QMatrix3x3 normalMatrix;
    QSSGRenderMvpArray modelViewProjections;
    QSSGDataRef<QSSGSubsetRenderable> subsets;
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGModelContext>::value);

class QSSGRenderer;
class QSSGLayerRenderData;
struct QSSGShadowMapEntry;

class Q_AUTOTEST_EXPORT QSSGSubsetRenderable : public QSSGRenderableObject
{
    Q_DISABLE_COPY_MOVE(QSSGSubsetRenderable)
public:
    int reflectionProbeIndex = -1;
    float distanceFromReflectionProbe;
    quint32 subsetLevelOfDetail = 0;
    QSSGShaderReflectionProbe reflectionProbe;
    QSSGRenderer *renderer = nullptr;
    const QSSGModelContext &modelContext;
    const QSSGRenderSubset &subset;
    QRhiBuffer *instanceBuffer = nullptr;
    float opacity;
    const QSSGRenderGraphObject &material;
    QSSGRenderableImage *firstImage;
    QSSGShaderDefaultMaterialKey shaderDescription;
    const QSSGShaderLightListView &lights;

    struct RhiPassData {
        QRhiGraphicsPipeline *pipeline = nullptr;
        QRhiShaderResourceBindings *srb = nullptr;
    };

    struct {
        // Transient (due to the subsetRenderable being allocated using a
        // per-frame allocator on every frame), not owned refs from the
        // rhi-prepare step, used by the rhi-render step.
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } mainPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } depthPrePass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb[6] = {};
        } shadowPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb[6] = {};
        } reflectionPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } normalPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } motionVectorPass;
        RhiPassData userPassData[QSSGUserRenderPassManager::maxUserPassSlots()] {};
    } rhiRenderData;

    QSSGSubsetRenderable(Type type,
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
                         bool anyLightHasShadows);

    [[nodiscard]] const QSSGRenderGraphObject &getMaterial() const { return material; }
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGSubsetRenderable>::value);

/**
 * A renderable that corresponds to a particles.
 */
class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGParticlesRenderable : public QSSGRenderableObject
{
    Q_DISABLE_COPY_MOVE(QSSGParticlesRenderable)
public:
    QSSGRenderer *renderer = nullptr;
    const QSSGRenderParticles &particles;
    QSSGRenderableImage *firstImage;
    QSSGRenderableImage *colorTable;
    const QSSGShaderLightListView &lights;
    QMatrix4x4 globalTransform;
    float opacity;
    QSSGShaderParticleMaterialKey shaderDescription;

    struct {
        // Transient (due to the subsetRenderable being allocated using a
        // per-frame allocator on every frame), not owned refs from the
        // rhi-prepare step, used by the rhi-render step.
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } mainPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb = nullptr;
        } depthPrePass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb[6] = {};
        } shadowPass;
        struct {
            QRhiGraphicsPipeline *pipeline = nullptr;
            QRhiShaderResourceBindings *srb[6] = {};
        } reflectionPass;
    } rhiRenderData;

    QSSGParticlesRenderable(QSSGRenderableObjectFlags inFlags,
                            const QVector3D &inWorldCenterPt,
                            QSSGRenderer *rendr,
                            const QMatrix4x4 &inGlobalTransform,
                            const QSSGRenderParticles &inParticles,
                            QSSGRenderableImage *inFirstImage,
                            QSSGRenderableImage *inColorTable,
                            const QSSGShaderLightListView &inLights,
                            float inOpacity,
                            QSSGShaderParticleMaterialKey inShaderKey);
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QSSGParticlesRenderable>::value);


using QSSGModelsView = QSSGDataView<QSSGRenderModel *>;
using QSSGParticlesView = QSSGDataView<QSSGRenderParticles *>;
using QSSGItem2DsView = QSSGDataView<QSSGRenderItem2D *>;
using QSSGCamerasView = QSSGDataView<QSSGRenderCamera *>;
using QSSGLightsView = QSSGDataView<QSSGRenderLight *>;
using QSSGReflectionProbesView = QSSGDataView<QSSGRenderReflectionProbe *>;
using QSSGNonCategorizedView = QSSGDataView<QSSGRenderNode *>;

QT_END_NAMESPACE

#endif
