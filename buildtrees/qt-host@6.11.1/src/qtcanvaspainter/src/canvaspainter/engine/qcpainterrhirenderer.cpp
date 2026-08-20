// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2013 Mikko Mononen memon@inside.org
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "engine/qcpainterengineutils_p.h"
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
#include "qcdistancefieldglyphcache_p.h"
#endif
#include "qcpainterengine_p.h"
#include "qcpainterrhirenderer_p.h"
#include "qcanvaspainter_p.h"
#include "qcanvascustombrush.h"
#include "qcanvaspath.h"
#include "qcanvasoffscreencanvas_p.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QFile>
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

// TODO: Measure optimal values for these
#ifndef QCPAINTER_INITIAL_TOTAL_CALLS_SIZE
#define QCPAINTER_INITIAL_TOTAL_CALLS_SIZE 128
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_PATHS_SIZE
#define QCPAINTER_INITIAL_TOTAL_PATHS_SIZE 256
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_VERTICES_SIZE
#define QCPAINTER_INITIAL_TOTAL_VERTICES_SIZE 16384
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_INDICES_SIZE
#define QCPAINTER_INITIAL_TOTAL_INDICES_SIZE 16384
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_COMMON_UNIFORMS_SIZE
// Note: Not the amount of uniforms, but uniform chars
#define QCPAINTER_INITIAL_TOTAL_COMMON_UNIFORMS_SIZE 16384
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_VERT_UNIFORMS_SIZE
#define QCPAINTER_INITIAL_TOTAL_VERT_UNIFORMS_SIZE 16
#endif
#ifndef QCPAINTER_INITIAL_TOTAL_TEXTURES_SIZE
#define QCPAINTER_INITIAL_TOTAL_TEXTURES_SIZE 16
#endif

// must match vertUBuf in qcpainter.vert
constexpr int VERT_UNIFORM_BUFFER_COMMON_REGION_SIZE = 96;

constexpr int FILL_QUAD_VERTEX_COUNT = 4;

// Note: Values need to match with the 'type' in shader code.
enum QCRHIShaderType {
    ShaderColor = 0,
    ShaderLinearGradient = 1,
    ShaderTexturedLinearGradient = 2,
    ShaderRadialGradient = 3,
    ShaderTexturedRadialGradient = 4,
    ShaderConicalGradient = 5,
    ShaderTexturedConicalGradient = 6,
    ShaderBoxGradient = 7,
    ShaderTexturedBoxGradient = 8,
    ShaderBoxShadow = 9,
    ShaderImage = 10,
    ShaderStencilFill = 11,
    ShaderGridPattern = 12,
};

enum QCRHICallType {
    CallFill,
    CallConvexFill,
    CallStroke,
    CallText,
};

struct QCRHIBlend
{
    bool enable;
    QRhiGraphicsPipeline::BlendFactor srcRGB;
    QRhiGraphicsPipeline::BlendFactor dstRGB;
    QRhiGraphicsPipeline::BlendFactor srcAlpha;
    QRhiGraphicsPipeline::BlendFactor dstAlpha;
};

// Note: memset-zeroed in allocCall(), so must set non-zero default values in that function, not here.
struct QCRHICall {
    QCRHICallType type;
    int image;
    int font;
    int pathOffset;
    int pathCount;
    int indexOffset;
    int indexCount;
    int triangleOffset;
    int triangleCount;
    int commonUniformBufferOffset;
    int vertUniformBufferOffset;
    QCPainterRhiRenderer::RenderFlags renderFlags;
    QRhiScissor scissor;
    QCRHIBlend blendFunc;
    QRhiShaderResourceBindings *srb[2];
    QRhiGraphicsPipeline *ps[4];
    QShader customFragShader;
    QShader customVertShader;
    bool textTriangleOffsetBakedInToIndices;
    quint64 pathGroupEntryId;
    int pathGroup;
};

struct QCRHIPath {
    int fillOffset;
    int fillCount;
    int strokeOffset;
    int strokeCount;
};

struct QCRHIVertUniforms {
    // Total size 256 bytes.
    // This is often the minimum uniform buffer offset alignment.
    float vertMat[12]; // matrices are actually 3 vec4s
    float unused[52];
};

struct QCRHICommonUniforms {
    // Total size 112 + 112 = 224 bytes.
    // Built-in variables size is 112 bytes
    float scissorMat[12]; // matrices are actually 3 vec4s
    float scissorExt[2];
    float scissorScale[2];
    float alphaMult;
    float strokeThr;
    float fontAlphaMin;
    float fontAlphaMax;
    float colorEffects[4];
    int texType;
    int type;
    float globalAlpha;
    // Take these into use when needed
    int unusedInt;
    // Custom input size is 112 bytes.
    float paintMat[12];
    QCColor innerCol;
    QCColor outerCol;
    float extent[2];
    float radius;
    float feather;
    // Take these into use when needed
    float unused2[4];
};

struct QCRHISamplerDesc
{
    QRhiSampler::Filter minFilter;
    QRhiSampler::Filter magFilter;
    QRhiSampler::Filter mipmap;
    QRhiSampler::AddressMode hTiling;
    QRhiSampler::AddressMode vTiling;
    QRhiSampler::AddressMode zTiling;
};

inline bool operator==(const QCRHISamplerDesc &a, const QCRHISamplerDesc &b) noexcept
{
    return a.hTiling == b.hTiling && a.vTiling == b.vTiling && a.zTiling == b.zTiling
           && a.minFilter == b.minFilter && a.magFilter == b.magFilter
           && a.mipmap == b.mipmap;
}

static void transformToMat3x4(const QTransform &t, float* m3) noexcept
{
    m3[0] = t.m11();
    m3[1] = t.m12();
    m3[2] = 0.0f;
    m3[3] = 0.0f;
    m3[4] = t.m21();
    m3[5] = t.m22();
    m3[6] = 0.0f;
    m3[7] = 0.0f;
    m3[8] = t.m31();
    m3[9] = t.m32();
    m3[10] = 1.0f;
    m3[11] = 0.0f;
}

struct QCRHIPipelineState
{
    QCPainterRhiRenderer::RenderFlags renderFlags = {};
    QRhiGraphicsPipeline::Topology topology = QRhiGraphicsPipeline::Triangles;
    QRhiGraphicsPipeline::CullMode cullMode = QRhiGraphicsPipeline::Back;

    bool stencilTestEnable = false;
    bool usesStencilRef = false;
    QRhiGraphicsPipeline::StencilOpState stencilFront;
    QRhiGraphicsPipeline::StencilOpState stencilBack;
    quint32 stencilReadMask = 0xFFFFFFFF;
    quint32 stencilWriteMask = 0xFFFFFFFF;

    QRhiGraphicsPipeline::TargetBlend targetBlend;

    int sampleCount = 1;
    QShader customFragShader;
    QShader customVertShader;
};

inline bool operator==(const QCRHIPipelineState &a, const QCRHIPipelineState &b) noexcept
{
    return a.renderFlags == b.renderFlags
           && a.topology == b.topology
           && a.cullMode == b.cullMode
           && a.stencilTestEnable == b.stencilTestEnable
           && a.usesStencilRef == b.usesStencilRef
           && a.stencilFront.failOp == b.stencilFront.failOp
           && a.stencilFront.depthFailOp == b.stencilFront.depthFailOp
           && a.stencilFront.passOp == b.stencilFront.passOp
           && a.stencilFront.compareOp == b.stencilFront.compareOp
           && a.stencilBack.failOp == b.stencilBack.failOp
           && a.stencilBack.depthFailOp == b.stencilBack.depthFailOp
           && a.stencilBack.passOp == b.stencilBack.passOp
           && a.stencilBack.compareOp == b.stencilBack.compareOp
           && a.stencilReadMask == b.stencilReadMask
           && a.stencilWriteMask == b.stencilWriteMask
           && a.customFragShader == b.customFragShader
           && a.customVertShader == b.customVertShader
           // NB! not memcmp
           && a.targetBlend.colorWrite == b.targetBlend.colorWrite
           && a.targetBlend.enable == b.targetBlend.enable
           && a.targetBlend.srcColor == b.targetBlend.srcColor
           && a.targetBlend.dstColor == b.targetBlend.dstColor
           && a.targetBlend.opColor == b.targetBlend.opColor
           && a.targetBlend.srcAlpha == b.targetBlend.srcAlpha
           && a.targetBlend.dstAlpha == b.targetBlend.dstAlpha
           && a.targetBlend.opAlpha == b.targetBlend.opAlpha
           && a.sampleCount == b.sampleCount;
}

inline bool operator!=(const QCRHIPipelineState &a, const QCRHIPipelineState &b) noexcept
{
    return !(a == b);
}

inline size_t qHash(const QCRHIPipelineState &s, size_t seed) noexcept
{
    // do not bother with all fields
    return qHash(s.renderFlags, seed)
           ^ qHash(s.sampleCount)
           ^ qHash(s.targetBlend.dstColor)
           ^ qHash(s.cullMode)
           ^ qHashBits(&s.stencilFront, sizeof(QRhiGraphicsPipeline::StencilOpState))
           ^ (s.stencilTestEnable << 3)
           ^ (s.usesStencilRef << 4)
           ^ (s.targetBlend.enable << 5);
}

struct QCRHIPipelineStateKey
{
    QCRHIPipelineState state;
    QVector<quint32> renderTargetDescription;
    QVector<quint32> srbLayoutDescription;
    struct {
        size_t renderTargetDescriptionHash;
        size_t srbLayoutDescriptionHash;
    } extra;
    static QCRHIPipelineStateKey create(const QCRHIPipelineState &state,
                                         const QRhiRenderPassDescriptor *rpDesc,
                                         const QRhiShaderResourceBindings *srb)
    {
        const QVector<quint32> rtDesc = rpDesc->serializedFormat();
        const QVector<quint32> srbDesc = srb->serializedLayoutDescription();
        return { state, rtDesc, srbDesc, { qHash(rtDesc), qHash(srbDesc) } };
    }
};

inline bool operator==(const QCRHIPipelineStateKey &a, const QCRHIPipelineStateKey &b) noexcept
{
    return a.state == b.state
           && a.renderTargetDescription == b.renderTargetDescription
           && a.srbLayoutDescription == b.srbLayoutDescription;
}

inline bool operator!=(const QCRHIPipelineStateKey &a, const QCRHIPipelineStateKey &b) noexcept
{
    return !(a == b);
}

inline size_t qHash(const QCRHIPipelineStateKey &k, size_t seed) noexcept
{
    return qHash(k.state, seed)
    ^ k.extra.renderTargetDescriptionHash
        ^ k.extra.srbLayoutDescriptionHash;
}

// The uniform buffers are the same for every draw call (the dynamic
// uniform buffers use dynamic offsets), only the QRhiTexture and Sampler
// can be different. The image flags (which defines the QRhiSampler) are
// static once an image id is created, so a simple general + font
// image id pair -> srb mapping is possible.
struct QCRHISrbKey {
    int passId;
    int imageId;
    int fontId;
};

inline size_t qHash(const QCRHISrbKey &k, size_t seed) noexcept
{
    QtPrivate::QHashCombineWithSeed hash(seed);
    seed = hash(seed, k.passId);
    seed = hash(seed, k.imageId);
    seed = hash(seed, k.fontId);
    return seed;
}

inline bool operator==(const QCRHISrbKey &lhs, const QCRHISrbKey &rhs) noexcept
{
    return lhs.passId == rhs.passId
           && lhs.imageId == rhs.imageId
           && lhs.fontId == rhs.fontId;
}

inline bool operator!=(const QCRHISrbKey &lhs, const QCRHISrbKey &rhs) noexcept
{
    return !(lhs == rhs);
}

static QShader getShader(const QString &name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

struct QCRHIShaders
{
    QCRHIShaders() {
        vs = getShader(QLatin1String(":/qcshaders/qcpainter.vert.qsb"));
        vsCustomMat = getShader(QLatin1String(":/qcshaders/qcpainter_custommat.vert.qsb"));
        fs = getShader(QLatin1String(":/qcshaders/qcpainter.frag.qsb"));
        fsTA = getShader(QLatin1String(":/qcshaders/qcpainter_t.frag.qsb"));
        fsSC = getShader(QLatin1String(":/qcshaders/qcpainter_sc.frag.qsb"));
        fsSCT= getShader(QLatin1String(":/qcshaders/qcpainter_sct.frag.qsb"));
        fsAA = getShader(QLatin1String(":/qcshaders/qcpainter_aa.frag.qsb"));
        fsAAT = getShader(QLatin1String(":/qcshaders/qcpainter_aat.frag.qsb"));
        fsAASS = getShader(QLatin1String(":/qcshaders/qcpainter_aa_ss.frag.qsb"));
        fsAASST = getShader(QLatin1String(":/qcshaders/qcpainter_aa_sst.frag.qsb"));
        fsAASC = getShader(QLatin1String(":/qcshaders/qcpainter_aa_sc.frag.qsb"));
        fsAASCT = getShader(QLatin1String(":/qcshaders/qcpainter_aa_sct.frag.qsb"));
        fsAASSSC = getShader(QLatin1String(":/qcshaders/qcpainter_aa_ss_sc.frag.qsb"));
        fsAASSSCT = getShader(QLatin1String(":/qcshaders/qcpainter_aa_ss_sct.frag.qsb"));

        if (!vs.isValid() || !vsCustomMat.isValid() || !fsTA.isValid()  || !fs.isValid() || !fsAA.isValid() || !fsAAT.isValid() || !fsSC.isValid() ||
            !fsSCT.isValid() || !fsAASS.isValid() || !fsAASST.isValid() || !fsAASC.isValid() || !fsAASCT.isValid() ||
            !fsAASSSC.isValid() || !fsAASSSCT.isValid()) {
            qFatal("Failed to load shaders!");
        }

        vertexInputLayout.setBindings({
            { 4 * sizeof(float) }
        });
        vertexInputLayout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }
        });
    }

    QShader vs;
    QShader vsCustomMat;
    QShader fs;
    QShader fsTA; // For APIs where the text glyph texture is generated using single Alpha insteand of single red
    QShader fsAA; // EDGE_AA enabled
    QShader fsAAT; // EDGE_AA with R8 text enabled
    QShader fsSC; // SCISSORING enabled
    QShader fsSCT; // SCISSORING with R8 enabled
    QShader fsAASS; // EDGE_AA and STENCIL_STROKES enabled
    QShader fsAASST; // EDGE_AA, STENCIL_STROKES and R8 text enabled
    QShader fsAASC; // EDGE_AA and SCISSORING enabled
    QShader fsAASCT; // EDGE_AA, SCISSORING and R8 text enabled
    QShader fsAASSSC; // EDGE_AA, STENCIL_STROKES and SCISSORING enabled
    QShader fsAASSSCT; // EDGE_AA, STENCIL_STROKES,SCISSORING and R8 text enabled
    QRhiVertexInputLayout vertexInputLayout;
};

Q_GLOBAL_STATIC(QCRHIShaders, QCPAINTER_RHI_SHADERS)

struct QCRhiCachedPathFillData
{
    QVector<QCVertex> verts; // might also contain stroke vertices
    QVector<QCRHIPath> paths;
    quint64 entryId;
    int indexCount;
    bool isConvex;
};

struct QCRhiCachedPathStrokeData
{
    QVector<QCVertex> verts;
    QVector<QCRHIPath> paths;
    quint64 entryId;
};

struct QCRhiCachedPath
{
    QHash<QCCachedPathFillProperties, QCRhiCachedPathFillData> fill;
    QHash<QCCachedPathStrokeProperties, QCRhiCachedPathStrokeData> stroke;
};

struct QCRhiCachedPathGroup
{
    QHash<QCanvasPath *, QCRhiCachedPath> cachedPaths;
    qsizetype totalSubpathCount = 0;
    qsizetype cachedVertexDataBytes = 0;

    QRhiBuffer *fillVertexBuffer = nullptr;
    QRhiBuffer *fillIndexBuffer = nullptr;
    QRhiBuffer *strokeVertexBuffer = nullptr;

    bool fillBuffersNeedUpdate = false;
    bool strokeBufferNeedsUpdate = false;
    bool fillBuffersReusableInCurrentFrame = true;
    bool strokeBufferReusableInCurrentFrame = true;
    struct FillBufferOffsets {
        int vertexOffset;
        int indexOffset;
    };
    QHash<quint64, FillBufferOffsets> fillBufferOffsetAdjust;
    QHash<quint64, int> strokeVertexBufferOffsetAdjust;
    quint64 idGen = 0;
};

struct QCRHIContext
{
    QRhi *rhi = nullptr;
    QRhiCommandBuffer *cb = nullptr;
    QRhiRenderTarget *rt = nullptr;
    float dpr = 1.0f;

    float viewRect[4] = {};
    int oneCommonUniformBufferSize = 0;
    int oneVertUniformBufferSize = 0;
    int vertUniformBufferCommonRegionAlignedSize = 0;
    QCPainterRhiRenderer::RenderFlags flags = QCPainterRhiRenderer::Antialiasing;

    // Per frame buffers
    QVector<QCRHICall> calls;
    QVector<QCRHIPath> paths;
    QVector<QCVertex> verts;
    QVector<uint32_t> indices;

    // Note: This is uchar as it can contain both
    // QCRHICommonUniforms and QCanvasCustomBrushPrivate::CommonUniforms.
    QVector<uchar> commonUniforms;
    QVector<QCRHIVertUniforms> vertUniforms;
    QVector<QCRHITexture> textures;
    int callsCount = 0;
    int pathsCount = 0;
    int vertsCount = 0;
    int indicesCount = 0;
    int commonUniformsCount = 0;
    int vertUniformsCount = 0;
    int texturesCount = 0;

    int textureId = 0;
    int dummyTex = 0;

    QVector<std::pair<QCRHISamplerDesc, QRhiSampler*>> samplers;
    QHash<QCRHIPipelineStateKey, QRhiGraphicsPipeline *> pipelines;

    int passId = 0;
    struct PerPassData {
        QRhiBuffer *vertexBuffer = nullptr;
        QRhiBuffer *indexBuffer = nullptr;
        QRhiBuffer *uniformBuffer = nullptr;
    };
    QHash<int, PerPassData> perPassData;
    PerPassData *currentPerPassData() { return &perPassData[passId]; }

    QHash<int, QCRhiCachedPathGroup> cachedPathGroups;

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    QCDistanceFieldGlyphCache *fontCache;
#endif
    QRhiResourceUpdateBatch *resourceUpdates = nullptr;
    QHash<QCRHISrbKey, QRhiShaderResourceBindings *> srbs;

    // Used for custom brushes iTime.
    QElapsedTimer animationElapsedTimer;
    qint64 renderTimeElapsedMs = 0;

    // Text
    uint32_t textVertexOffset = 0;
    uint32_t textIndexOffset = 0;
};

QRhiGraphicsPipeline *QCPainterRhiRenderer::pipeline(const QCRHIPipelineStateKey &key,
                                                     QRhiRenderPassDescriptor *rpDesc,
                                                     QRhiShaderResourceBindings *srb)
{
    // Return pipeline if it already exists.
    auto it = rhiCtx->pipelines.constFind(key);
    if (it != rhiCtx->pipelines.constEnd())
        return it.value();

    QRhiGraphicsPipeline *ps = rhiCtx->rhi->newGraphicsPipeline();

    QCRHIShaders *shaders = QCPAINTER_RHI_SHADERS();

    const bool isTextRed = rhiCtx->rhi->isFeatureSupported(QRhi::RedOrAlpha8IsRed);
    QShader const *fs = nullptr;
    QShader const *vs = nullptr;

    if (key.state.customVertShader.isValid()) {
        vs = &key.state.customVertShader;
    } else {
        if (key.state.renderFlags & RenderFlag::CustomMatrix)
            vs = &shaders->vsCustomMat;
        else
            vs = &shaders->vs;
    }

    if (key.state.customFragShader.isValid()) {
        // Use custom brush shader
        fs = &key.state.customFragShader;
    } else {
        // Use standard shaders, choose the correct one
        if (key.state.renderFlags & RenderFlag::Antialiasing) {
            if (key.state.renderFlags & RenderFlag::StencilStrokes) {
                if (key.state.renderFlags & RenderFlag::TransformedClipping)
                    fs = isTextRed ? &shaders->fsAASSSC : &shaders->fsAASSSCT;
                else
                    fs = isTextRed ? &shaders->fsAASS : &shaders->fsAASST;
            } else {
                if (key.state.renderFlags & RenderFlag::TransformedClipping)
                    fs = isTextRed ? &shaders->fsAASC : &shaders->fsAASCT;
                else
                    fs = isTextRed ? &shaders->fsAA : &shaders->fsAAT;
            }
        } else {
            if (key.state.renderFlags & RenderFlag::TransformedClipping) {
                fs = isTextRed ? &shaders->fsSC : &shaders->fsSCT;
            } else {
                fs = isTextRed ? &shaders->fs : &shaders->fsTA;
            }
        }
    }
    ps->setShaderStages({
        { QRhiShaderStage::Vertex, *vs },
        { QRhiShaderStage::Fragment, *fs }
    });
    ps->setVertexInputLayout(shaders->vertexInputLayout);
    ps->setShaderResourceBindings(srb);
    ps->setRenderPassDescriptor(rpDesc);

    QRhiGraphicsPipeline::Flags flags;
    if (key.state.usesStencilRef)
        flags |= QRhiGraphicsPipeline::UsesStencilRef;
    if (key.state.renderFlags & RenderFlag::SimpleClipping)
        flags |= QRhiGraphicsPipeline::UsesScissor;
    ps->setFlags(flags);

    ps->setTopology(key.state.topology);
    ps->setCullMode(key.state.cullMode);

    ps->setTargetBlends({ key.state.targetBlend });

    ps->setSampleCount(key.state.sampleCount);

    // Depth test and write are generally OFF, but test can be requested with a flag.
    //
    // We do require a depth-stencil buffer though, due to relying on stencil,
    // but the depth part of the buffer is not written by QCanvasPainter. If this
    // would be needed for some feature in the future, note that enabling depth
    // buffer usage is not trivial, since it can cause various conflicts in
    // applications integrating QCanvasPainter rendering in 3D scenes, if they also
    // use the depth buffer while rendering the 2D drawing inline, targeting the
    // same main color and depth-stencil buffers the 3D rendering also targets.
    ps->setDepthTest(key.state.renderFlags & RenderFlag::DepthTest);
    ps->setDepthWrite(false);

    ps->setStencilTest(key.state.stencilTestEnable);
    ps->setStencilFront(key.state.stencilFront);
    ps->setStencilBack(key.state.stencilBack);
    ps->setStencilReadMask(key.state.stencilReadMask);
    ps->setStencilWriteMask(key.state.stencilWriteMask);

    if (!ps->create()) {
        qWarning("Failed to build graphics pipeline state");
        delete ps;
        return nullptr;
    }

    rhiCtx->pipelines.insert(key, ps);
    return ps;
}

QRhiSampler *QCPainterRhiRenderer::sampler(const QCRHISamplerDesc &samplerDescription)
{
    auto compareSampler = [samplerDescription](const std::pair<QCRHISamplerDesc, QRhiSampler*> &info) {
        return info.first == samplerDescription;
    };
    const auto found = std::find_if(rhiCtx->samplers.cbegin(), rhiCtx->samplers.cend(), compareSampler);
    if (found != rhiCtx->samplers.cend())
        return found->second;

    QRhiSampler *newSampler = rhiCtx->rhi->newSampler(samplerDescription.magFilter,
                                                      samplerDescription.minFilter,
                                                      samplerDescription.mipmap,
                                                      samplerDescription.hTiling,
                                                      samplerDescription.vTiling,
                                                      samplerDescription.zTiling);
    if (!newSampler->create()) {
        qWarning("Failed to build image sampler");
        delete newSampler;
        return nullptr;
    }
    rhiCtx->samplers << std::make_pair(samplerDescription, newSampler);
    return newSampler;
}

static bool ensureBufferCapacity(QRhiBuffer **buf, quint32 size, float overAllocateMultiplier = 1.0f)
{
    if ((*buf)->size() < size) {
        quint32 newSize = size * overAllocateMultiplier;
        (*buf)->setSize(newSize);
        if (!(*buf)->create()) {
            qWarning("Failed to recreate buffer with size %u", newSize);
            return false;
        }
    }
    return true;
}

QRhiResourceUpdateBatch *QCPainterRhiRenderer::resourceUpdateBatch()
{
    if (!rhiCtx->resourceUpdates)
        rhiCtx->resourceUpdates = rhiCtx->rhi->nextResourceUpdateBatch();
    return rhiCtx->resourceUpdates;
}

void QCPainterRhiRenderer::commitResourceUpdates()
{
    if (rhiCtx->resourceUpdates) {
        rhiCtx->cb->resourceUpdate(rhiCtx->resourceUpdates);
        rhiCtx->resourceUpdates = nullptr;
    }
}

QCRHITexture *QCPainterRhiRenderer::findTexture(QRhiTexture *texture) const
{
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        auto *tex = &rhiCtx->textures[i];
        if (tex->tex == texture)
            return tex;
    }
    return nullptr;
}

QCRHITexture *QCPainterRhiRenderer::findTexture(int id) const
{
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        auto *tex = &rhiCtx->textures[i];
        if (tex->id == id)
            return tex;

    }
    return nullptr;
}

QRhiShaderResourceBindings *QCPainterRhiRenderer::createSrb(int brushImage, int fontImage)
{
    QCRHITexture* tex = nullptr;
    QCRHITexture* fontTex = nullptr;
    if (brushImage != 0)
        tex = findTexture(brushImage);
    if (!tex)
        tex = findTexture(rhiCtx->dummyTex);

    if (fontImage != 0)
        fontTex = findTexture(fontImage);
    if (!fontTex)
        fontTex = findTexture(rhiCtx->dummyTex);

    // As we fallback to dummy textures, at this point
    // textures and QRhiTexture inside them always exist.
    Q_ASSERT(tex && tex->tex);
    Q_ASSERT(fontTex && fontTex->tex);

    QCRHISamplerDesc samplerDesc;
    samplerDesc.minFilter = (tex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear;
    samplerDesc.magFilter = (tex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear;
    samplerDesc.mipmap = (tex->flags & QCanvasPainter::ImageFlag::GenerateMipmaps) ?
                             ((tex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear) :
                             QRhiSampler::None;
    samplerDesc.hTiling = (tex->flags & QCanvasPainter::ImageFlag::RepeatX) ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
    samplerDesc.vTiling = (tex->flags & QCanvasPainter::ImageFlag::RepeatY) ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
    samplerDesc.zTiling = QRhiSampler::Repeat;

    QCRHISamplerDesc fontSamplerDesc;
    fontSamplerDesc.minFilter = (fontTex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear;
    fontSamplerDesc.magFilter = (fontTex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear;
    fontSamplerDesc.mipmap = (fontTex->flags & QCanvasPainter::ImageFlag::GenerateMipmaps) ?
                                 ((fontTex->flags & QCanvasPainter::ImageFlag::Nearest) ? QRhiSampler::Nearest : QRhiSampler::Linear) :
                                 QRhiSampler::None;
    fontSamplerDesc.hTiling = (fontTex->flags & QCanvasPainter::ImageFlag::RepeatX) ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
    fontSamplerDesc.vTiling = (fontTex->flags & QCanvasPainter::ImageFlag::RepeatY) ? QRhiSampler::Repeat : QRhiSampler::ClampToEdge;
    fontSamplerDesc.zTiling = QRhiSampler::Repeat;

    QRhiShaderResourceBindings *srb = rhiCtx->rhi->newShaderResourceBindings();
    QCRHIContext::PerPassData *ppd = rhiCtx->currentPerPassData();
    srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, ppd->uniformBuffer,
            0, VERT_UNIFORM_BUFFER_COMMON_REGION_SIZE),
        QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(
            1, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            ppd->uniformBuffer, sizeof(QCRHICommonUniforms)),
        QRhiShaderResourceBinding::uniformBufferWithDynamicOffset(4, QRhiShaderResourceBinding::VertexStage,
            ppd->uniformBuffer, sizeof(QCRHIVertUniforms)),
        QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, tex->tex, sampler(samplerDesc)),
        QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, fontTex->tex, sampler(fontSamplerDesc)),
    });
    if (!srb->create()) {
        qWarning("Failed to create resource bindings");
        delete srb;
        return nullptr;
    }
    return srb;
}

bool QCPainterRhiRenderer::renderCreate()
{
    // Initial allocations into data arrays
    rhiCtx->calls.resize(QCPAINTER_INITIAL_TOTAL_CALLS_SIZE);
    rhiCtx->paths.resize(QCPAINTER_INITIAL_TOTAL_PATHS_SIZE);
    rhiCtx->verts.resize(QCPAINTER_INITIAL_TOTAL_VERTICES_SIZE);
    rhiCtx->indices.resize(QCPAINTER_INITIAL_TOTAL_INDICES_SIZE);
    rhiCtx->commonUniforms.resize(QCPAINTER_INITIAL_TOTAL_COMMON_UNIFORMS_SIZE);
    rhiCtx->vertUniforms.resize(QCPAINTER_INITIAL_TOTAL_VERT_UNIFORMS_SIZE);
    rhiCtx->textures.resize(QCPAINTER_INITIAL_TOTAL_TEXTURES_SIZE);

    rhiCtx->textureId = 0;

    // Add default non-transforming uniform as first.
    QTransform t;
    QCRHIVertUniforms u;
    transformToMat3x4(t, u.vertMat);
    rhiCtx->vertUniforms[0] = u;
    rhiCtx->vertUniformsCount = 1;

    // Some platforms do not allow to have unset textures.
    // Create empty one which is bound when there's no texture specified.
    rhiCtx->dummyTex = renderCreateTexture(TextureFormatAlpha, 16, 16, {}, nullptr);

    // The Dynamic, host-visible buffers in PerPassData stay not created until endPrepare.

    // Make sure that standard and custom brush structs match..
    Q_ASSERT(sizeof(QCRHICommonUniforms) == sizeof(QCanvasCustomBrushPrivate::CommonUniforms));
    Q_ASSERT(rhiCtx->rhi->ubufAligned(sizeof(QCRHICommonUniforms))
             == rhiCtx->rhi->ubufAligned(sizeof(QCanvasCustomBrushPrivate::CommonUniforms)));

    // ..so we can use either of them as the oneCommonUniformBufferSize
    // and insert them into commonUniforms.
    rhiCtx->oneCommonUniformBufferSize = rhiCtx->rhi->ubufAligned(sizeof(QCRHICommonUniforms));
    rhiCtx->oneVertUniformBufferSize = rhiCtx->rhi->ubufAligned(sizeof(QCRHIVertUniforms));
    rhiCtx->vertUniformBufferCommonRegionAlignedSize = rhiCtx->rhi->ubufAligned(VERT_UNIFORM_BUFFER_COMMON_REGION_SIZE);

    return true;
}

QCRHITexture *QCPainterRhiRenderer::renderCreateNativeTexture(QRhiTexture *texture, QCanvasPainter::ImageFlags flags)
{
    Q_ASSERT(texture);

    QCRHITexture *tex = nullptr;

    // Check if the texture already exists.
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        QCRHITexture *cached = &rhiCtx->textures[i];
        // A bit complicated. Check the rhi global resource id to eliminate the
        // problem of a new object possibly getting the same address as a
        // previously destroyed (and registered) one. Then also the size, given
        // one could do setPixelSize(); create(); on the texture and then our
        // stored size is stale.
        if (cached->tex && cached->tex == texture && cached->tex->globalResourceId() == texture->globalResourceId()) {
            if (cached->width == texture->pixelSize().width() && cached->height == texture->pixelSize().height())
                return cached;
            // update the existing entry
            tex = cached;
        }
    }

    if (!tex) {
        tex = allocTexture();
        tex->id = ++rhiCtx->textureId;
    }

    tex->width = texture->pixelSize().width();
    tex->height = texture->pixelSize().height();
    tex->flags = flags | QCanvasPainter::ImageFlag::NativeTexture; // so 'texture' is not owned by tex
    tex->tex = texture;

    if (flags & QCanvasPainter::ImageFlag::GenerateMipmaps) {
        QRhiResourceUpdateBatch *u = resourceUpdateBatch();
        u->generateMips(tex->tex);
    }

    return tex;
}

// Considers the texture to have changed outside of QCanvasPainter, hence needs updating
QCRHITexture *QCPainterRhiRenderer::renderUpdateNativeTexture(
    QRhiTexture *oldTexture, QRhiTexture *texture)
{
    QCRHITexture *tex = nullptr;

    // Replace old texture with new one
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        if (rhiCtx->textures.at(i).tex == oldTexture) {
            tex = &rhiCtx->textures[i];
            tex->tex = texture;
            break;
        }
    }

    if (!tex) {
        //qDebug() << "Texture not found while updating, returning... (oldTexture: " << oldTexture
        //         << ", newTexture: " << texture << ")";
        return nullptr;
    }

    //qDebug() << "Updating QCanvasPainter font, w: h: " << texture->pixelSize().width() << ", "
    //         << texture->pixelSize().height() << " object: " << oldTexture
    //         << ", new object: " << texture << ", id: " << tex->id << ", flags: " << tex->flags;
    tex->width = texture->pixelSize().width();
    tex->height = texture->pixelSize().height();

    // Remove old binding that use this font texture from srbs
    auto &srbHash = rhiCtx->srbs;
    QHash<QCRHISrbKey, QRhiShaderResourceBindings *>::iterator i = srbHash.begin();
    while (i != srbHash.end()) {
        if (i.key().fontId == tex->id) {
            if (*i)
                (*i)->deleteLater();
            (*i) = nullptr;
        }
        i++;
    }

    return tex;
}

int QCPainterRhiRenderer::renderCreateTexture(QCTextureFormat format, int w, int h, QCanvasPainter::ImageFlags imageFlags, const uchar* data)
{
    QRhiTexture::Format texFormat = QRhiTexture::RGBA8;
    if (format == TextureFormatAlpha)
        texFormat = QRhiTexture::R8; // this excludes supporting OpenGL ES 2.0 but perhaps that's fine

    const QSize size(w, h);
    quint32 byteSize = 0;
    textureFormatInfo(texFormat, size, nullptr, &byteSize, nullptr);

    QRhiTexture::Flags flags;
    if (imageFlags & QCanvasPainter::ImageFlag::GenerateMipmaps)
        flags |= QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips;

    QRhiTexture *t = rhiCtx->rhi->newTexture(texFormat, size, 1, flags);
    if (!t->create()) {
        qWarning("Failed to create texture of size %dx%d", w, h);
        delete t;
        return 0;
    }

    QCRHITexture *tex = allocTexture();
    tex->id = ++rhiCtx->textureId;
    tex->width = w;
    tex->height = h;
    tex->flags = imageFlags;
    tex->tex = t;

    QRhiResourceUpdateBatch *u = resourceUpdateBatch();
    if (data) {
        QRhiTextureSubresourceUploadDescription image(data, byteSize);
        QRhiTextureUploadDescription desc({ 0, 0, image });
        u->uploadTexture(tex->tex, desc);
    }

    if (imageFlags & QCanvasPainter::ImageFlag::GenerateMipmaps)
        u->generateMips(tex->tex);

    return tex->id;
}

bool QCPainterRhiRenderer::renderDeleteTexture(int image)
{
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        QCRHITexture *tex = &rhiCtx->textures[i];
        if (tex->id == image) {
            // Delete QRhiTexture (unless not owned), but leave QCRHITexture
            // to be reused.
            if (!tex->flags.testFlag(QCanvasPainter::ImageFlag::NativeTexture))
                delete tex->tex;
            tex->tex = nullptr;
            tex->id = 0;
            return true;
        }
    }
    return false;
}

// TODO: Unused for now. Remove if not needed.
bool QCPainterRhiRenderer::renderUpdateTexture(int image, int x, int y, int w, int h, const uchar* data)
{
    QCRHITexture* tex = findTexture(image);
    if (!tex)
        return false;

    QRhiResourceUpdateBatch *u = resourceUpdateBatch();
    if (data) {
        quint32 stride, byteSize, bytesPerPixel;
        textureFormatInfo(tex->tex->format(), tex->tex->pixelSize(), &stride, &byteSize, &bytesPerPixel);

        const quint32 startOffset = x * bytesPerPixel + y * stride;
        QRhiTextureSubresourceUploadDescription image(data + startOffset, byteSize - startOffset);
        image.setDataStride(stride);
        image.setDestinationTopLeft(QPoint(x, y));
        image.setSourceSize(QSize(w, h));

        QRhiTextureUploadDescription desc({ 0, 0, image });
        u->uploadTexture(tex->tex, desc);
    }

    return true;
}

bool QCPainterRhiRenderer::isOffscreenCanvasYUp() const
{
    return rhiCtx->rhi->isYUpInFramebuffer();
}

void QCPainterRhiRenderer::setViewport(float x, float y, float width, float height)
{
    rhiCtx->viewRect[0] = x;
    rhiCtx->viewRect[1] = y;
    rhiCtx->viewRect[2] = width;
    rhiCtx->viewRect[3] = height;
}

static QCRHIBlend blendCompositeOperation(QCanvasPainter::CompositeOperation op, bool blendEnable)
{
    QRhiGraphicsPipeline::BlendFactor sourceFactor;
    QRhiGraphicsPipeline::BlendFactor destinationFactor;

    switch (op) {
    case QCanvasPainter::CompositeOperation::SourceAtop:
        sourceFactor = QRhiGraphicsPipeline::DstAlpha;
        destinationFactor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        break;
    case QCanvasPainter::CompositeOperation::DestinationOut:
        sourceFactor = QRhiGraphicsPipeline::Zero;
        destinationFactor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        break;

    case QCanvasPainter::CompositeOperation::SourceOver:
    default:
        sourceFactor = QRhiGraphicsPipeline::One;
        destinationFactor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
        break;
    }
    QCRHIBlend blend { blendEnable,
                       sourceFactor, destinationFactor,
                       sourceFactor, destinationFactor };
    return blend;
}

// Returns the needed vertex count for \a path.
// If \a indexCount is provided, calculates there the needed indices count.
static int maxVertCount(const QCPaths &paths, int pathsCount, int *indexCount = nullptr)
{
    int count = 0;
    if (indexCount)
        *indexCount = 0;
    for (int i = 0; i < pathsCount; i++) {
        const QCPath &path = paths.at(i);
        const int fillCount = path.fillCount;
        if (fillCount > 2) {
            count += fillCount;
            if (indexCount)
                *indexCount += (fillCount - 2) * 3;
        }
        count += path.strokeCount;
    }
    return count;
}

void QCPainterRhiRenderer::updateVertUniforms(QCRHICall *call, const QTransform &transform)
{
    if (rhiCtx->vertUniforms.size() < rhiCtx->vertUniformsCount + 1) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->vertUniformsCount + 1) + rhiCtx->vertUniforms.size() * 0.5;
        rhiCtx->vertUniforms.resize(newSize);
    }
    // Set the transform and the offset.
    call->vertUniformBufferOffset = rhiCtx->vertUniformsCount;
    QCRHIVertUniforms &u = rhiCtx->vertUniforms[rhiCtx->vertUniformsCount++];
    transformToMat3x4(transform, u.vertMat);
}

QCRHICall* QCPainterRhiRenderer::allocCall()
{
    if (rhiCtx->callsCount + 1 > rhiCtx->calls.size()) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->callsCount + 1) + rhiCtx->calls.size() * 0.5;
        rhiCtx->calls.resize(newSize);
    }
    QCRHICall *ret = &rhiCtx->calls[rhiCtx->callsCount++];
    memset((void*)ret, 0, sizeof(QCRHICall));
    ret->pathGroup = -1;
    return ret;
}

// Return unused texture slot or if not found,
// return new one and increase the texturesCount.
QCRHITexture *QCPainterRhiRenderer::allocTexture()
{
    QCRHITexture *tex = nullptr;
    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        // Locate unused texture
        if (rhiCtx->textures.at(i).id == 0) {
            tex = &rhiCtx->textures[i];
            break;
        }
    }
    if (!tex) {
        // Or take next texture, allocate more if needed
        if (rhiCtx->texturesCount + 1 > rhiCtx->textures.size()) {
            // Overallocate as suitable
            const int newCount = (rhiCtx->texturesCount + 1) + rhiCtx->textures.size() * 0.5;
            rhiCtx->textures.resize(newCount);
        }
        tex = &rhiCtx->textures[rhiCtx->texturesCount++];
    }
    return tex;
}

// Makes sure there are enough paths available.
// Returns offset of the path (so previous amount of paths).
int QCPainterRhiRenderer::allocPaths(int count)
{
    if (rhiCtx->pathsCount + count > rhiCtx->paths.size()) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->pathsCount + count) + rhiCtx->paths.size() * 0.5;
        rhiCtx->paths.resize(newSize);
    }
    int ret = rhiCtx->pathsCount;
    rhiCtx->pathsCount += count;
    return ret;
}

int QCPainterRhiRenderer::allocVerts(int count)
{
    if (rhiCtx->vertsCount + count > rhiCtx->verts.size()) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->vertsCount + count) + rhiCtx->verts.size() * 0.5;
        rhiCtx->verts.resize(newSize);
    }
    int ret = rhiCtx->vertsCount;
    rhiCtx->vertsCount += count;
    return ret;
}

int QCPainterRhiRenderer::allocIndices(int count)
{
    if (rhiCtx->indicesCount + count > rhiCtx->indices.size()) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->indicesCount + count) + rhiCtx->indices.size() * 0.5;
        rhiCtx->indices.resize(newSize);
    }
    int ret = rhiCtx->indicesCount;
    rhiCtx->indicesCount += count;
    return ret;
}

int QCPainterRhiRenderer::allocCommonUniforms(int count)
{
    const int structSize = rhiCtx->oneCommonUniformBufferSize;
    if ((rhiCtx->commonUniformsCount + (count * structSize)) > rhiCtx->commonUniforms.size()) {
        // Overallocate as suitable
        const int newSize = (rhiCtx->commonUniformsCount + count * structSize) + rhiCtx->commonUniforms.size() * 0.5;
        rhiCtx->commonUniforms.resize(newSize);
    }
    int ret = rhiCtx->commonUniformsCount;
    rhiCtx->commonUniformsCount += count * structSize;
    return ret;
}

// Returns pointer to uniforms at i as QCRHICommonUniforms
QCRHICommonUniforms *QCPainterRhiRenderer::uniformPtr(int i) const
{
    return (QCRHICommonUniforms*)&rhiCtx->commonUniforms[i];
}

// Returns pointer to uniforms at i as QCanvasCustomBrushPrivate::CommonUniforms
QCanvasCustomBrushPrivate::CommonUniforms *QCPainterRhiRenderer::customUniformPtr(int i) const
{
    return (QCanvasCustomBrushPrivate::CommonUniforms*)&rhiCtx->commonUniforms[i];
}

static constexpr void setVert(QCVertex *vtx, float x, float y, float u, float v) noexcept
{
    vtx->x = x;
    vtx->y = y;
    vtx->u = u;
    vtx->v = v;
}

// Set inverted transform of \a t into \a m3. If t is
// identity matrix, no inversion is needed so set as-is.
static void setPaintTransform(const QTransform &t, float *m3) noexcept
{
    if (t.isIdentity())
        transformToMat3x4(t, m3);
    else
        transformToMat3x4(t.inverted(), m3);
}

static constexpr QCColor premulColor(QCColor c) noexcept
{
    c.r *= c.a;
    c.g *= c.a;
    c.b *= c.a;
    return c;
}

static constexpr QCRHIShaderType shaderTypeFromBrush(QCanvasBrushType brushType, bool textured) noexcept
{
    if (brushType == BrushColor)
        return ShaderColor;
    else if (brushType == BrushLinearGradient)
        return textured ? ShaderTexturedLinearGradient : ShaderLinearGradient;
    else if (brushType == BrushRadialGradient)
        return textured ? ShaderTexturedRadialGradient : ShaderRadialGradient;
    else if (brushType == BrushConicalGradient)
        return textured ? ShaderTexturedConicalGradient : ShaderConicalGradient;
    else if (brushType == BrushBoxGradient)
        return textured ? ShaderTexturedBoxGradient : ShaderBoxGradient;
    else if (brushType == BrushBoxShadow)
        return ShaderBoxShadow;
    else if (brushType == BrushGrid)
        return ShaderGridPattern;

    // BrushImage
    return ShaderImage;
}

// Prepare common uniforms according to paint & clip.
void QCPainterRhiRenderer::preparePaint(QCRHICommonUniforms *frag, const QCPaint &paint,
                                        const QCState &state, float width, float aa, float strokeThr,
                                        float fontAlphaMin, float fontAlphaMax)
{
    memset((void*)frag, 0, sizeof(*frag));
    // If antialiasing is disabled, both aa and width must be at least 0.01
    // so alphaMult becomes >= 1.
    aa = std::max(aa, 0.01f);
    width = std::max(width, 0.01f);
    frag->innerCol = premulColor(paint.innerColor);
    if (paint.brushType == BrushBoxShadow)
        frag->outerCol = paint.outerColor; // Used for corner radius
    else
        frag->outerCol = premulColor(paint.outerColor);

    frag->globalAlpha = paint.alpha;
    frag->fontAlphaMin = fontAlphaMin;
    frag->fontAlphaMax = fontAlphaMax;

    if (rhiCtx->flags & QCPainterRhiRenderer::TransformedClipping) {
        const auto &clip = state.clip;
        if (clip.extent[0] < -0.5f || clip.extent[1] < -0.5f) {
            memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
            frag->scissorExt[0] = 1.0f;
            frag->scissorExt[1] = 1.0f;
            frag->scissorScale[0] = 1.0f;
            frag->scissorScale[1] = 1.0f;
        } else {
            setPaintTransform(clip.transform, frag->scissorMat);
            frag->scissorExt[0] = clip.extent[0];
            frag->scissorExt[1] = clip.extent[1];
            const float m1 = clip.transform.m11();
            const float m2 = clip.transform.m12();
            const float m3 = clip.transform.m21();
            const float m4 = clip.transform.m22();
            frag->scissorScale[0] = sqrt(m1 * m1 + m3 * m3) / aa;
            frag->scissorScale[1] = sqrt(m2 * m2 + m4 * m4) / aa;
        }
    }

    frag->extent[0] = paint.extent[0];
    frag->extent[1] = paint.extent[1];
    frag->alphaMult = (width * 0.5f + aa * 0.5f) / aa;
    frag->strokeThr = strokeThr;

    // Apply color effects
    if (qFuzzyCompare(state.brightness, 1.0f) &&
        qFuzzyCompare(state.contrast, 1.0f) &&
        qFuzzyCompare(state.saturate, 1.0f)) {
        // No color effects enabled
        frag->colorEffects[0] = -1.0f;
    } else {
        frag->colorEffects[0] = 1.0f; // Enabled
        frag->colorEffects[1] = state.brightness - 1.0f; // Brightness, default 0.0
        frag->colorEffects[2] = state.contrast; // Contrast, default 1.0
        frag->colorEffects[3] = state.saturate; // Saturation, default 1.0
    }

    if (paint.imageId != 0) {
        QCRHITexture* tex = findTexture(paint.imageId);
        if (tex && tex->flags & QCanvasPainter::ImageFlag::FlipY) {
            // Flip image vertically
            QTransform transform = paint.transform;
            transform.scale(1, -1);
            transform = transform.translate(0.0f, -frag->extent[1]);
            setPaintTransform(transform, frag->paintMat);
        } else {
            setPaintTransform(paint.transform, frag->paintMat);
        }

        frag->type = shaderTypeFromBrush(paint.brushType, true);
        frag->radius = paint.radius;
        frag->feather = paint.feather;

        if (tex && tex->tex->format() != QRhiTexture::R8)
            frag->texType = (tex->flags & QCanvasPainter::ImageFlag::Premultiplied) ? 0 : 1;
        else
            frag->texType = 2;
    } else {
        if (paint.brushType == BrushColor) {
            // Color
            frag->type = ShaderColor;
        } else {
            frag->type = shaderTypeFromBrush(paint.brushType, false);
            frag->radius = paint.radius;
            frag->feather = paint.feather;
            setPaintTransform(paint.transform, frag->paintMat);
        }
    }
}

// Prepare custom fragment shader uniforms according to brush & clip.
void QCPainterRhiRenderer::prepareCustomPaint(QCanvasCustomBrushPrivate::CommonUniforms *frag, const QCPaint &paint,
                                              QCanvasCustomBrush *brush, const QCState &state,
                                              float width, float aa, float strokeThr,
                                              float fontAlphaMin, float fontAlphaMax)
{
    Q_UNUSED(paint);
    Q_ASSERT(brush);
    memset((void*)frag, 0, sizeof(*frag));

    // Apply custom data
    auto *privBrush = QCanvasCustomBrushPrivate::get(brush);
    frag->data[0] = privBrush->data[0];
    frag->data[1] = privBrush->data[1];
    frag->data[2] = privBrush->data[2];
    frag->data[3] = privBrush->data[3];

    frag->globalAlpha = paint.alpha;
    frag->fontAlphaMin = fontAlphaMin;
    frag->fontAlphaMax = fontAlphaMax;

    // Handle iTime animation
    if (brush->timeRunning()) {
        auto ms = rhiCtx->renderTimeElapsedMs;
        float s = ms * 0.001;
        privBrush->time += s;
        frag->iTime = privBrush->time;
    }

    const bool isTextRed = rhiCtx->rhi->isFeatureSupported(QRhi::RedOrAlpha8IsRed);
    frag->alphaIsRed = isTextRed;

    // If antialiasing is disabled, both aa and width must be at least 0.01
    // so alphaMult becomes >= 1.
    aa = std::max(aa, 0.01f);
    width = std::max(width, 0.01f);

    // Custom brush always contains clipping uniforms?
    const auto &clip = state.clip;
    if (clip.extent[0] < -0.5f || clip.extent[1] < -0.5f) {
        memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
        frag->scissorExt[0] = 1.0f;
        frag->scissorExt[1] = 1.0f;
        frag->scissorScale[0] = 1.0f;
        frag->scissorScale[1] = 1.0f;
    } else {
        setPaintTransform(clip.transform, frag->scissorMat);
        frag->scissorExt[0] = clip.extent[0];
        frag->scissorExt[1] = clip.extent[1];
        const float m1 = clip.transform.m11();
        const float m2 = clip.transform.m12();
        const float m3 = clip.transform.m21();
        const float m4 = clip.transform.m22();
        frag->scissorScale[0] = sqrt(m1 * m1 + m3 * m3) / aa;
        frag->scissorScale[1] = sqrt(m2 * m2 + m4 * m4) / aa;
    }

    frag->alphaMult = (width * 0.5f + aa * 0.5f) / aa;
    frag->strokeThr = strokeThr;

    // Apply color effects
    if (qFuzzyCompare(state.brightness, 1.0f) &&
        qFuzzyCompare(state.contrast, 1.0f) &&
        qFuzzyCompare(state.saturate, 1.0f)) {
        // No color effects enabled
        frag->colorEffects[0] = -1.0f;
    } else {
        frag->colorEffects[0] = 1.0f; // Enabled
        frag->colorEffects[1] = state.brightness - 1.0f; // Brightness, default 0.0
        frag->colorEffects[2] = state.contrast; // Contrast, default 1.0
        frag->colorEffects[3] = state.saturate; // Saturation, default 1.0
    }
}

// Adds the provided fill path geometry into rhiCtx->verts, also generating rhiCtx->indices,
// and sets up the draw call.
int QCPainterRhiRenderer::transferFillGeom(QCRHICall *call,
                                           int vertexCount, const QCVertex *vertices, int indexCount,
                                           int pathCount, const QCPath *pathInfos)
{
    int vertOffset = allocVerts(vertexCount);
    call->indexCount = indexCount;
    call->indexOffset = allocIndices(call->indexCount);
    call->pathCount = pathCount;
    call->pathOffset = allocPaths(call->pathCount);

    uint32_t *indexPtr = &rhiCtx->indices[call->indexOffset];
    for (int i = 0; i < call->pathCount; i++) {
        QCRHIPath *renderPath = &rhiCtx->paths[call->pathOffset + i];
        memset(renderPath, 0, sizeof(QCRHIPath));
        const QCPath *pathFillInfo = &pathInfos[i];
        const int fillCount = pathFillInfo->fillCount;
        if (fillCount > 2) {
            renderPath->fillOffset = vertOffset;
            renderPath->fillCount = fillCount;
            memcpy(&rhiCtx->verts[vertOffset], &vertices[pathFillInfo->fillOffset], sizeof(QCVertex) * fillCount);
            int baseVertexIndex = vertOffset;
            for (int j = 2; j < fillCount; j++) {
                *indexPtr++ = baseVertexIndex;
                *indexPtr++ = baseVertexIndex + j - 1;
                *indexPtr++ = baseVertexIndex + j;
            }
            vertOffset += fillCount;
        }
        const int strokeCount = pathFillInfo->strokeCount;
        if (strokeCount > 0) {
            renderPath->strokeOffset = vertOffset;
            renderPath->strokeCount = strokeCount;
            memcpy(&rhiCtx->verts[vertOffset], &vertices[pathFillInfo->strokeOffset], sizeof(QCVertex) * strokeCount);
            vertOffset += strokeCount;
        }
    }

    return vertOffset;
}

void QCPainterRhiRenderer::transferPathsFromCachedPathGroup(QCRHICall *call, int pathCount, const QCRHIPath *paths)
{
    call->pathCount = pathCount;
    call->pathOffset = allocPaths(call->pathCount);
    for (int i = 0; i < call->pathCount; ++i) {
        // Can copy as-is. The offsets will need to be adjusted (in
        // render()) by cpg.*vertexBufferOffsetAdjust[entryId] but that
        // is calculated only when baking the dedicated path group
        // buffer (in endPrepare()).
        rhiCtx->paths[call->pathOffset + i] = paths[i];
    }
}

void QCPainterRhiRenderer::renderFill(const QCPaint &paint, const QCState &state,
                                      const QRectF &bounds,
                                      std::optional<QCRhiUncachedPathDrawArgs> uncachedPathInfo,
                                      std::optional<QCRhiCachedPathDrawArgs> cachedPathInfo)
{
    QCRHICall *call = allocCall();
    auto &ctx = m_e->ctx;
    const float aa = state.antialias;

    call->type = CallFill;
    call->renderFlags = rhiCtx->flags;
    if (state.customFill) {
        auto *customBrushPriv = QCanvasCustomBrushPrivate::get(state.customFill);
        call->customFragShader = customBrushPriv->fragmentShader;
        call->customVertShader = customBrushPriv->vertexShader;
    }
    call->triangleCount = FILL_QUAD_VERTEX_COUNT;
    call->image = paint.imageId;
    call->blendFunc = blendCompositeOperation(state.compositeOperation, state.blendEnable);
    const QRectF clipRect = state.clip.rect;
    call->scissor.setScissor(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());

    int vertOffset = 0;

    Q_ASSERT((uncachedPathInfo || cachedPathInfo) && !(uncachedPathInfo && cachedPathInfo));

    if (uncachedPathInfo) {
        // Painting directly, not using painterpath
        const QCRhiUncachedPathDrawArgs &pti = uncachedPathInfo.value();

        const bool isConvex = (pti.pathsCount == 1 && pti.paths.first().isConvex);
        if (isConvex) {
            call->type = CallConvexFill;
            call->triangleCount = 0;
        }

        int indexCount;
        int vertsCount = maxVertCount(pti.paths, pti.pathsCount, &indexCount) + call->triangleCount;
        // Note that it includes the fill quad (4 vertices unless convex) in vertsCount, and the return
        // value is exactly where those vertices start.
        vertOffset = transferFillGeom(call, vertsCount, ctx.vertices.constData(), indexCount,
                                      pti.pathsCount, pti.paths.constData());

    } else if (cachedPathInfo) {
        // Using a pathGroup. The geometry for this is only there in
        // ctx.vertices if the engine decided there was a cache miss
        // (pti.updateData is valid in that case, it's nullopt on a hit)

        const QCRhiCachedPathDrawArgs &pti = cachedPathInfo.value();

        // transform is applied in the vertex shader
        updateVertUniforms(call, pti.pathTransform);

        QCRhiCachedPathGroup *cpg = &rhiCtx->cachedPathGroups[pti.pathGroup];
        QCRhiCachedPath *cp = &cpg->cachedPaths[pti.canvasPath];
        QCCachedPathFillProperties fillProps { state.antialias, int(ctx.renderHints) };
        QCRhiCachedPathFillData *cpf = &cp->fill[fillProps];

        if (pti.updateData.has_value()) {
            // The cached vertex data in cp is stale. Update it from ctx.vertices.

            cpf->entryId = cpg->idGen++;
            cpf->isConvex = (pti.updateData->pathsCount == 1 && pti.updateData->paths.first().isConvex);
            const int triangleCount = cpf->isConvex ? 0 : call->triangleCount;

            int vertsCount = maxVertCount(pti.updateData->paths, pti.updateData->pathsCount, &cpf->indexCount) + triangleCount;

            // As on the uncached path, vertsCount, and so cpf->verts,
            // includes the space for the fill quad (4 vertices unless convex),
            // even though that is isn't in ctx.vertices.
            cpg->cachedVertexDataBytes -= cpf->verts.size() * sizeof(QCVertex);
            cpf->verts.resize(vertsCount);
            cpg->cachedVertexDataBytes += cpf->verts.size() * sizeof(QCVertex);

            cpg->totalSubpathCount -= cpf->paths.count();
            cpf->paths.resize(pti.updateData->pathsCount);
            cpg->totalSubpathCount += cpf->paths.count();

            // The section for this path fill in the buffer will be laid out as:
            //   <fill_vertices_subpath_0> [<stroke_vertices_subpath_0>] [<fill_vertices_subpath_1> [<stroke_vertices_subpath_1>] ...] <fill quad>
            vertOffset = 0;
            for (int i = 0; i < pti.updateData->pathsCount; ++i) {
                QCRHIPath *cachedFillInfo = &cpf->paths[i];
                memset(cachedFillInfo, 0, sizeof(QCRHIPath));
                const QCPath *preparedPath = &pti.updateData->paths.at(i);
                const int fillCount = preparedPath->fillCount;
                if (fillCount > 2) {
                    cachedFillInfo->fillOffset = vertOffset;
                    cachedFillInfo->fillCount = fillCount;
                    const auto *vertices = &ctx.vertices;
                    memcpy(&cpf->verts[vertOffset], &vertices->at(preparedPath->fillOffset), sizeof(QCVertex) * fillCount);
                    vertOffset += fillCount;
                }
                const int strokeCount = preparedPath->strokeCount;
                if (strokeCount > 0) {
                    cachedFillInfo->strokeOffset = vertOffset;
                    cachedFillInfo->strokeCount = strokeCount;
                    const auto *vertices = &ctx.vertices;
                    memcpy(&cpf->verts[vertOffset], &vertices->at(preparedPath->strokeOffset), sizeof(QCVertex) * strokeCount);
                    vertOffset += strokeCount;
                }
            }

            if (!cpf->isConvex) {
                QCVertex *quad = &cpf->verts[vertOffset];
                setVert(&quad[0], bounds.width(), bounds.height(), 0.5f, 1.0f);
                setVert(&quad[1], bounds.width(), bounds.y(), 0.5f, 1.0f);
                setVert(&quad[2], bounds.x(), bounds.height(), 0.5f, 1.0f);
                setVert(&quad[3], bounds.x(), bounds.y(), 0.5f, 1.0f);
            }

            cpg->fillBuffersNeedUpdate = true;
        }

        if (cpf->isConvex) {
            call->type = CallConvexFill;
            call->triangleCount = 0;
        } else {
            call->triangleOffset = cpf->verts.count() - FILL_QUAD_VERTEX_COUNT; // will need to be adjusted by cpg.fillVertexBufferOffsetAdjust[id]
        }

        call->pathGroup = pti.pathGroup;
        call->pathGroupEntryId = cpf->entryId;
        call->indexCount = cpf->indexCount;
        call->indexOffset = 0; // will need to be adjusted by cpg.fillIndexBufferOffsetAdjust[id]
        transferPathsFromCachedPathGroup(call, cpf->paths.count(), cpf->paths.constData());
    }

    // Setup uniforms for draw calls
    if (call->type == CallFill) {
        if (uncachedPathInfo) {
            // Update fill quad
            call->triangleOffset = vertOffset;
            QCVertex *quad = &rhiCtx->verts[call->triangleOffset];
            setVert(&quad[0], bounds.width(), bounds.height(), 0.5f, 1.0f);
            setVert(&quad[1], bounds.width(), bounds.y(), 0.5f, 1.0f);
            setVert(&quad[2], bounds.x(), bounds.height(), 0.5f, 1.0f);
            setVert(&quad[3], bounds.x(), bounds.y(), 0.5f, 1.0f);
        }

        call->commonUniformBufferOffset = allocCommonUniforms(2);
        // Simple shader for stencil
        QCRHICommonUniforms* frag = uniformPtr(call->commonUniformBufferOffset);
        memset((void*)frag, 0, sizeof(*frag));
        frag->strokeThr = -1.0f;
        frag->type = ShaderStencilFill;
        // Fill shader
        if (state.customFill) {
            prepareCustomPaint(customUniformPtr(call->commonUniformBufferOffset + rhiCtx->oneCommonUniformBufferSize),
                               paint, state.customFill, state, aa, aa, -1.0f,
                               -1.0f, -1.0f);
        } else {
            preparePaint(uniformPtr(call->commonUniformBufferOffset + rhiCtx->oneCommonUniformBufferSize),
                         paint, state, aa, aa, -1.0f, -1.0f, -1.0f);
        }
    } else {
        // CallConvexFill
        // Fill shader
        call->commonUniformBufferOffset = allocCommonUniforms(1);
        if (state.customFill) {
            prepareCustomPaint(customUniformPtr(call->commonUniformBufferOffset),
                               paint, state.customFill, state, aa, aa, -1.0f,
                               -1.0f, -1.0f);
        } else {
            preparePaint(uniformPtr(call->commonUniformBufferOffset),
                         paint, state, aa, aa, -1.0f, -1.0f, -1.0f);
        }
    }
}

// Adds the provided stroke path geometry into rhiCtx->verts and sets up the
// draw call.
void QCPainterRhiRenderer::transferStrokeGeom(QCRHICall *call,
                                              int vertexCount, const QCVertex *vertices,
                                              int pathCount, const QCPath *pathInfos)
{
    int vertOffset = allocVerts(vertexCount);
    call->pathCount = pathCount;
    call->pathOffset = allocPaths(call->pathCount);

    for (int i = 0; i < call->pathCount; i++) {
        QCRHIPath *renderPath = &rhiCtx->paths[call->pathOffset + i];
        memset(renderPath, 0, sizeof(QCRHIPath));
        const QCPath *pathStrokeInfo = &pathInfos[i];
        const int strokeCount = pathStrokeInfo->strokeCount;
        if (strokeCount > 0) {
            renderPath->strokeOffset = vertOffset;
            renderPath->strokeCount = strokeCount;
            memcpy(&rhiCtx->verts[vertOffset], &vertices[pathStrokeInfo->strokeOffset], sizeof(QCVertex) * strokeCount);
            vertOffset += strokeCount;
        }
    }
}

void QCPainterRhiRenderer::renderStroke(const QCPaint &paint, const QCState &state,
                                        float strokeWidth,
                                        std::optional<QCRhiUncachedPathDrawArgs> uncachedPathInfo,
                                        std::optional<QCRhiCachedPathDrawArgs> cachedPathInfo)
{
    QCRHICall *call = allocCall();
    auto &ctx = m_e->ctx;
    const float aa = state.antialias;
    call->type = CallStroke;
    call->renderFlags = rhiCtx->flags;
    if (state.customStroke) {
        auto *customBrushPriv = QCanvasCustomBrushPrivate::get(state.customStroke);
        call->customFragShader = customBrushPriv->fragmentShader;
        call->customVertShader = customBrushPriv->vertexShader;
    }
    call->image = paint.imageId;
    call->blendFunc = blendCompositeOperation(state.compositeOperation, state.blendEnable);
    const QRectF clipRect = state.clip.rect;
    call->scissor.setScissor(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());

    Q_ASSERT((uncachedPathInfo || cachedPathInfo) && !(uncachedPathInfo && cachedPathInfo));

    if (uncachedPathInfo) {
        // Painting directly, not using painterpath
        const QCRhiUncachedPathDrawArgs &pti = uncachedPathInfo.value();

        const int vertsCount = maxVertCount(pti.paths, pti.pathsCount);

        transferStrokeGeom(call, vertsCount, ctx.vertices.constData(),
                           pti.pathsCount, pti.paths.constData());

    } else if (cachedPathInfo) {
        // Using a pathGroup. The geometry for this is only there in
        // ctx.vertices if the engine decided there was a cache miss
        // (pti.updateData is valid in that case, it's nullopt on a hit)

        const QCRhiCachedPathDrawArgs &pti = cachedPathInfo.value();

        // transform is applied in the vertex shader
        updateVertUniforms(call, pti.pathTransform);

        QCRhiCachedPathGroup *cpg = &rhiCtx->cachedPathGroups[pti.pathGroup];
        QCRhiCachedPath *cp = &cpg->cachedPaths[pti.canvasPath];
        QCCachedPathStrokeProperties strokeProps { state.antialias, state.strokeWidth, state.lineCap, state.lineJoin, int(ctx.renderHints) };
        QCRhiCachedPathStrokeData *cps = &cp->stroke[strokeProps];

        if (pti.updateData.has_value()) {
            // The cached vertex data in cp is stale. Update it from ctx.vertices.

            cps->entryId = cpg->idGen++;

            const int vertsCount = maxVertCount(pti.updateData->paths, pti.updateData->pathsCount);

            cpg->cachedVertexDataBytes -= cps->verts.size() * sizeof(QCVertex);
            cps->verts.resize(vertsCount);
            cpg->cachedVertexDataBytes += cps->verts.size() * sizeof(QCVertex);

            cpg->totalSubpathCount -= cps->paths.count();
            cps->paths.resize(pti.updateData->pathsCount);
            cpg->totalSubpathCount += cps->paths.count();

            int vertOffset = 0;
            for (int i = 0; i < pti.updateData->pathsCount; i++) {
                QCRHIPath *cachedStrokeInfo = &cps->paths[i];
                memset(cachedStrokeInfo, 0, sizeof(QCRHIPath));
                const QCPath *preparedPath = &pti.updateData->paths.at(i);
                const int strokeCount = preparedPath->strokeCount;
                if (strokeCount > 0) {
                    cachedStrokeInfo->strokeOffset = vertOffset;
                    cachedStrokeInfo->strokeCount = strokeCount;
                    const auto *vertices = &ctx.vertices;
                    memcpy(&cps->verts[vertOffset], &vertices->at(preparedPath->strokeOffset), sizeof(QCVertex) * strokeCount);
                    vertOffset += strokeCount;
                }
            }

            cpg->strokeBufferNeedsUpdate = true;
        }

        call->pathGroup = pti.pathGroup;
        call->pathGroupEntryId = cps->entryId;
        transferPathsFromCachedPathGroup(call, cps->paths.count(), cps->paths.constData());
    }

    if (rhiCtx->flags & QCPainterRhiRenderer::StencilStrokes) {
        call->commonUniformBufferOffset = allocCommonUniforms(2);
        if (state.customStroke) {
            prepareCustomPaint(customUniformPtr(call->commonUniformBufferOffset),
                               paint, state.customStroke, state, strokeWidth, aa, -1.0f,
                               -1.0f, -1.0f);
            prepareCustomPaint(customUniformPtr(call->commonUniformBufferOffset + rhiCtx->oneCommonUniformBufferSize),
                               paint, state.customStroke, state, strokeWidth, aa, 1.0f - 0.5f/255.0f,
                               -1.0f, -1.0f);
        } else {
            preparePaint(uniformPtr(call->commonUniformBufferOffset),
                         paint, state, strokeWidth, aa, -1.0f,
                         -1.0f, -1.0f);
            preparePaint(uniformPtr(call->commonUniformBufferOffset + rhiCtx->oneCommonUniformBufferSize),
                         paint, state, strokeWidth, aa, 1.0f - 0.5f/255.0f,
                         -1.0f, -1.0f);
        }
    } else {
        call->commonUniformBufferOffset = allocCommonUniforms(1);
        if (state.customStroke) {
            prepareCustomPaint(customUniformPtr(call->commonUniformBufferOffset),
                               paint, state.customStroke, state, strokeWidth, aa, -1.0f,
                               -1.0f, -1.0f);
        } else {
            preparePaint(uniformPtr(call->commonUniformBufferOffset),
                         paint, state, strokeWidth, aa, -1.0f,
                         -1.0f, -1.0f);
        }
    }
}

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT

// Fill direct text with normal brush
void QCPainterRhiRenderer::renderTextFill(
    const QCPaint &paint,
    const QCState &state,
    const QCRhiDistanceFieldGlyphCache::VertexList &verts,
    const QCRhiDistanceFieldGlyphCache::IndexList &indices)
{
    QCRHICall *call = allocCall();
    auto &ctx = m_e->ctx;

    call->type = CallText;
    // Text uses own AA, so disable stroke antialiasing.
    call->renderFlags = rhiCtx->flags;
    call->renderFlags &= ~RenderFlag::Antialiasing;
    call->image = paint.imageId;
    call->font = ctx.fontId;
    call->blendFunc = blendCompositeOperation(state.compositeOperation, state.blendEnable);
    const QRectF clipRect = state.clip.rect;
    call->scissor.setScissor(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());

    // Allocate vertices for all the paths.
    const int vertsCount = int(verts.size());
    call->triangleOffset = allocVerts(vertsCount);
    call->triangleCount = vertsCount;

    memcpy(&rhiCtx->verts[call->triangleOffset], verts.data(), sizeof(QCVertex) * vertsCount);

    // Allocate indices
    const int indicesCount = int(indices.size());
    call->indexOffset = allocIndices(indicesCount);
    call->indexCount = indicesCount;

    if (rhiCtx->rhi->isFeatureSupported(QRhi::BaseVertex)) {
        memcpy(&rhiCtx->indices[call->indexOffset],
               indices.data(),
               sizeof(uint32_t) * indicesCount);
    } else {
        const uint32_t *idxSrc = indices.data();
        uint32_t *idxDst = &rhiCtx->indices[call->indexOffset];
        for (int idx = 0; idx < indicesCount; ++idx) {
            *idxDst++ = *idxSrc + call->triangleOffset;
            ++idxSrc;
        }
        call->textTriangleOffsetBakedInToIndices = true;
    }

    // Fill shader
    call->commonUniformBufferOffset = allocCommonUniforms(1);
    auto frag = uniformPtr(call->commonUniformBufferOffset);
    const float aa = 1.0f;
    preparePaint(frag, paint, state, 1.0f, aa, -1.0f, ctx.fontAlphaMin, ctx.fontAlphaMax);
}

// Fill direct text with custom brush
void QCPainterRhiRenderer::renderTextFillCustom(
    const QCPaint &paint,
    const QCState &state,
    QCanvasCustomBrush *brush,
    const QCRhiDistanceFieldGlyphCache::VertexList &verts,
    const QCRhiDistanceFieldGlyphCache::IndexList &indices)
{
    QCRHICall *call = allocCall();
    auto &ctx = m_e->ctx;

    call->type = CallText;
    // Text uses own AA, so disable stroke antialiasing.
    call->renderFlags = rhiCtx->flags;
    call->renderFlags &= ~RenderFlag::Antialiasing;
    call->image = paint.imageId;
    call->font = ctx.fontId;
    if (brush) {
        auto *customBrushPriv = QCanvasCustomBrushPrivate::get(brush);
        call->customFragShader = customBrushPriv->fragmentShader;
        call->customVertShader = customBrushPriv->vertexShader;
    }
    call->blendFunc = blendCompositeOperation(state.compositeOperation, state.blendEnable);
    const QRectF clipRect = state.clip.rect;
    call->scissor.setScissor(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height());

    // Allocate vertices for all the paths.
    const int vertsCount = int(verts.size());
    call->triangleOffset = allocVerts(vertsCount);
    call->triangleCount = vertsCount;

    memcpy(&rhiCtx->verts[call->triangleOffset], verts.data(), sizeof(QCVertex) * vertsCount);

    // Allocate indices
    const int indicesCount = int(indices.size());
    call->indexOffset = allocIndices(indicesCount);
    call->indexCount = indicesCount;

    if (rhiCtx->rhi->isFeatureSupported(QRhi::BaseVertex)) {
        memcpy(&rhiCtx->indices[call->indexOffset],
               indices.data(),
               sizeof(uint32_t) * indicesCount);
    } else {
        const uint32_t *idxSrc = indices.data();
        uint32_t *idxDst = &rhiCtx->indices[call->indexOffset];
        for (int idx = 0; idx < indicesCount; ++idx) {
            *idxDst++ = *idxSrc + call->triangleOffset;
            ++idxSrc;
        }
        call->textTriangleOffsetBakedInToIndices = true;
    }

    // Fill shader
    call->commonUniformBufferOffset = allocCommonUniforms(1);
    auto frag = customUniformPtr(call->commonUniformBufferOffset);
    const float aa = 1.0f;
    prepareCustomPaint(frag, paint, brush, state, 0.1f, aa, -1.0f,
                       ctx.fontAlphaMin, ctx.fontAlphaMax);
}

#endif

void QCPainterRhiRenderer::beginPrepare(QRhiCommandBuffer *cb,
                                        QRhiRenderTarget *rt,
                                        float devicePixelRatio)
{
    rhiCtx->cb = cb;
    rhiCtx->rt = rt;
    rhiCtx->dpr = qFuzzyIsNull(devicePixelRatio) ? rt->devicePixelRatio() : devicePixelRatio;

    resetForPass();
}

void QCPainterRhiRenderer::endPrepare()
{
    rhiCtx->cb->debugMarkBegin("QCanvasPainter prep"_ba);
    if (rhiCtx->callsCount > 0) {
        QRhiResourceUpdateBatch *u = resourceUpdateBatch();
        QCRHIContext::PerPassData *ppd = rhiCtx->currentPerPassData();

        if (!ppd->vertexBuffer) {
            ppd->vertexBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, 16384);
            ppd->vertexBuffer->setName("qc vertex buffer");
            if (!ppd->vertexBuffer->create()) {
                qWarning("Failed to create vertex buffer");
                return;
            }
        }
        if (!ppd->indexBuffer) {
            ppd->indexBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::IndexBuffer, 16384);
            ppd->indexBuffer->setName("qc index buffer");
            if (!ppd->indexBuffer->create()) {
                qWarning("Failed to create index buffer");
                return;
            }
        }
        if (!ppd->uniformBuffer) {
            ppd->uniformBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 16384);
            ppd->uniformBuffer->setName("qc uniform buffer");
            if (!ppd->uniformBuffer->create()) {
                qWarning("Failed to create uniform buffer for binding 0, 1, and 4");
                return;
            }
        }

        // Dynamic uniform buffer for vertex shader, exposed as two bindings (0 and 4) in qcpainter.vert
        {
            const quint32 sizeOfVUBuf = rhiCtx->vertUniformsCount * rhiCtx->oneVertUniformBufferSize;
            ensureBufferCapacity(&ppd->uniformBuffer,
                rhiCtx->vertUniformBufferCommonRegionAlignedSize // common part for vertex shader
                + sizeOfVUBuf // per-call part for vertex shader
                + rhiCtx->commonUniformsCount // per-call part for vertex and fragment shaders
            );
            char *p = ppd->uniformBuffer->beginFullDynamicBufferUpdateForCurrentFrame();
            // vertUBuf contents
            constexpr int sizeOfViewRect = 4 * sizeof(float);
            constexpr int sizeOfYDown = sizeof(qint32);
            memcpy(p, rhiCtx->viewRect, sizeOfViewRect);
            const qint32 ndcIsYDown = !rhiCtx->rhi->isYUpInNDC();
            memcpy(p + 16, &ndcIsYDown, sizeOfYDown);
            if (m_e->ctx.customMatrixValid) {
                // mat4 is 16 aligned, hence the offset is 32, not 20
                memcpy(p + 32, m_e->ctx.customMatrix.constData(), 64);
            }

            // vertUBuf2 contents (different data for each call)
            int offset = rhiCtx->vertUniformBufferCommonRegionAlignedSize;
            memcpy(p + offset, rhiCtx->vertUniforms.constData(), sizeOfVUBuf);

            // fragUBuf contents (different data for each call)
            offset += sizeOfVUBuf; // aligned
            memcpy(p + offset, rhiCtx->commonUniforms.constData(), rhiCtx->commonUniformsCount);
            ppd->uniformBuffer->endFullDynamicBufferUpdateForCurrentFrame();
        }

        // Vertex buffer
        const int vertsCount = rhiCtx->vertsCount;
        const float overAllocate = 1.05f; // Allocate 5% extra buffer for vertices and indices
        const quint32 sizeOfVBuf = vertsCount * sizeof(QCVertex);
        ensureBufferCapacity(&ppd->vertexBuffer, sizeOfVBuf, overAllocate);
        u->uploadStaticBuffer(ppd->vertexBuffer, 0, sizeOfVBuf, rhiCtx->verts.constData());
        if (rhiCtx->indicesCount) {
            // Index buffer
            const quint32 sizeOfIBuf = rhiCtx->indicesCount * sizeof(uint32_t);
            ensureBufferCapacity(&ppd->indexBuffer, sizeOfIBuf, overAllocate);
            u->uploadStaticBuffer(ppd->indexBuffer, 0, sizeOfIBuf, rhiCtx->indices.constData());
        }

        for (auto it = rhiCtx->cachedPathGroups.begin(), end = rhiCtx->cachedPathGroups.end(); it != end; ++it) {
            QCRhiCachedPathGroup &cpg(*it);

            if (cpg.fillBuffersNeedUpdate) {
                cpg.fillBuffersNeedUpdate = false;
                quint32 vbufSize = 0;
                quint32 ibufSize = 0;
                for (const QCRhiCachedPath &cp : std::as_const(cpg.cachedPaths)) {
                    for (const QCRhiCachedPathFillData &cpf : cp.fill) {
                        vbufSize += quint32(cpf.verts.count() * sizeof(QCVertex));
                        ibufSize += quint32(cpf.indexCount * sizeof(uint32_t));
                    }
                }

                if (!cpg.fillBuffersReusableInCurrentFrame) {
                    // The buffer may still be bound in an earlier pass of the current
                    // frame (another widget/item drawing via the same shared painter).
                    // Defer destruction to end-of-frame with deleteLater(); new buffers
                    // are created below. Path group buffers are not duplicated per pass
                    // (unlike the main vertex/index buffers in PerPassData) because they
                    // are designed to persist across frames. The set of widgets/items
                    // sharing a painter can change between frames, so a stable per-pass
                    // slot assignment is not viable.
                    if (cpg.fillVertexBuffer) {
                        cpg.fillVertexBuffer->deleteLater();
                        cpg.fillVertexBuffer = nullptr;
                    }
                    if (cpg.fillIndexBuffer) {
                        cpg.fillIndexBuffer->deleteLater();
                        cpg.fillIndexBuffer = nullptr;
                    }
                } else {
                    cpg.fillBuffersReusableInCurrentFrame = false;
                }

                if (!cpg.fillVertexBuffer) {
                    cpg.fillVertexBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, std::max(16384u, vbufSize));
                    const QString bufResName = QString::asprintf("qc path group %d fill vbuf", it.key());
                    cpg.fillVertexBuffer->setName(bufResName.toLatin1());
                    if (!cpg.fillVertexBuffer->create()) {
                        qWarning("Failed to create path group vertex buffer");
                        return;
                    }
                }

                if (!cpg.fillIndexBuffer) {
                    cpg.fillIndexBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::IndexBuffer, std::max(16384u, ibufSize));
                    const QString bufResName = QString::asprintf("qc path group %d fill ibuf", it.key());
                    cpg.fillIndexBuffer->setName(bufResName.toLatin1());
                    if (!cpg.fillIndexBuffer->create()) {
                        qWarning("Failed to create path group index buffer");
                        return;
                    }
                }

                ensureBufferCapacity(&cpg.fillVertexBuffer, vbufSize, overAllocate);
                ensureBufferCapacity(&cpg.fillIndexBuffer, ibufSize, overAllocate);

                QByteArray indexBufData(ibufSize, Qt::Uninitialized);
                uint32_t *indexBase = reinterpret_cast<uint32_t *>(indexBufData.data());
                uint32_t *indexPtr = indexBase;

                cpg.fillBufferOffsetAdjust.clear();
                int vertOffset = 0; // in vertices, not bytes
                for (const QCRhiCachedPath &cp : std::as_const(cpg.cachedPaths)) {
                    for (const QCRhiCachedPathFillData &cpf : cp.fill) {
                        const int vertCount = cpf.verts.count();
                        const quint32 byteSize = quint32(vertCount * sizeof(QCVertex));
                        u->uploadStaticBuffer(cpg.fillVertexBuffer, vertOffset * sizeof(QCVertex), byteSize, cpf.verts.constData());
                        cpg.fillBufferOffsetAdjust[cpf.entryId] = { vertOffset, int(indexPtr - indexBase) };
                        int baseVertexIndex = vertOffset;
                        for (const QCRHIPath &path : cpf.paths) {
                            if (path.fillCount > 2) {
                                for (int j = 2; j < path.fillCount; j++) {
                                    *indexPtr++ = baseVertexIndex + path.fillOffset;
                                    *indexPtr++ = baseVertexIndex + path.fillOffset + j - 1;
                                    *indexPtr++ = baseVertexIndex + path.fillOffset + j;
                                }
                            }
                        }
                        vertOffset += vertCount;
                    }
                }

                u->uploadStaticBuffer(cpg.fillIndexBuffer, 0, indexBufData);
            }

            if (cpg.strokeBufferNeedsUpdate) {
                cpg.strokeBufferNeedsUpdate = false;
                quint32 vbufSize = 0;
                for (const QCRhiCachedPath &cp : std::as_const(cpg.cachedPaths)) {
                    for (const QCRhiCachedPathStrokeData &cps : cp.stroke)
                        vbufSize += quint32(cps.verts.count() * sizeof(QCVertex));
                }

                if (!cpg.strokeBufferReusableInCurrentFrame) {
                    if (cpg.strokeVertexBuffer) {
                        // Delete only at the end of the frame, might be in use
                        // already in the current frame by an earlier "pass"
                        // (i.e., another item/widget with shared painter)
                        cpg.strokeVertexBuffer->deleteLater();
                        cpg.strokeVertexBuffer = nullptr;
                    }
                } else {
                    cpg.strokeBufferReusableInCurrentFrame = false;
                }

                if (!cpg.strokeVertexBuffer) {
                    cpg.strokeVertexBuffer = rhiCtx->rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, std::max(16384u, vbufSize));
                    const QString bufResName = QString::asprintf("qc path group %d stroke vbuf", it.key());
                    cpg.strokeVertexBuffer->setName(bufResName.toLatin1());
                    if (!cpg.strokeVertexBuffer->create()) {
                        qWarning("Failed to create path group vertex buffer");
                        return;
                    }
                }

                ensureBufferCapacity(&cpg.strokeVertexBuffer, vbufSize, overAllocate);
                cpg.strokeVertexBufferOffsetAdjust.clear();
                int vertOffset = 0; // in vertices, not bytes
                for (const QCRhiCachedPath &cp : std::as_const(cpg.cachedPaths)) {
                    for (const QCRhiCachedPathStrokeData &cps : cp.stroke) {
                        const int vertCount = cps.verts.count();
                        const quint32 byteSize = quint32(vertCount * sizeof(QCVertex));
                        u->uploadStaticBuffer(cpg.strokeVertexBuffer, vertOffset * sizeof(QCVertex), byteSize, cps.verts.constData());
                        cpg.strokeVertexBufferOffsetAdjust[cps.entryId] = vertOffset;
                        vertOffset += vertCount;
                    }
                }
            }
        }

        commitResourceUpdates();

        const int dummyTexId = findTexture(rhiCtx->dummyTex)->id;
        const QCRHISrbKey key = { rhiCtx->passId, dummyTexId, dummyTexId };
        QRhiShaderResourceBindings *srbWithDummyTexture = rhiCtx->srbs[key];
        if (!srbWithDummyTexture) {
            srbWithDummyTexture = createSrb(dummyTexId, dummyTexId);
            rhiCtx->srbs[key] = srbWithDummyTexture;
        }

        QRhiShaderResourceBindings *srbForLayout = srbWithDummyTexture; // all of them are layout-compatible, could use any

        QCRHIPipelineState basePs;
        // Note: Currently these are renderer scope flags, so same ones used
        // for all calls. In case we want to enable antialiasing/stencil
        // per-call, test them in calls loop.
        basePs.renderFlags = rhiCtx->flags;
        basePs.sampleCount = rhiCtx->rt->sampleCount();
        for (int i = 0; i < rhiCtx->callsCount; i++) {

            QCRHICall *call = &rhiCtx->calls[i];
            QRhiRenderPassDescriptor *rpDesc = rhiCtx->rt->renderPassDescriptor();

            const QCRHISrbKey key = { rhiCtx->passId, call->image, call->font };
            QRhiShaderResourceBindings *srbWithCallTexture = rhiCtx->srbs.value(key);
            if (!srbWithCallTexture) {
                srbWithCallTexture = createSrb(call->image, call->font);
                rhiCtx->srbs[key] = srbWithCallTexture;
            }

            // Set the blending mode
            basePs.targetBlend.enable = call->blendFunc.enable;
            basePs.targetBlend.srcColor = call->blendFunc.srcRGB;
            basePs.targetBlend.dstColor = call->blendFunc.dstRGB;
            basePs.targetBlend.srcAlpha = call->blendFunc.srcAlpha;
            basePs.targetBlend.dstAlpha = call->blendFunc.dstAlpha;

            // Set a possible clipping mode
            basePs.renderFlags.setFlag(QCPainterRhiRenderer::TransformedClipping,
                                       call->renderFlags & QCPainterRhiRenderer::TransformedClipping);
            basePs.renderFlags.setFlag(QCPainterRhiRenderer::SimpleClipping,
                                       call->renderFlags & QCPainterRhiRenderer::SimpleClipping);

            // Set antialiasing mode.
            basePs.renderFlags.setFlag(QCPainterRhiRenderer::Antialiasing,
                                       call->renderFlags & QCPainterRhiRenderer::Antialiasing);

            basePs.renderFlags.setFlag(QCPainterRhiRenderer::CustomMatrix, m_e->ctx.customMatrixValid);

            // Custom shader
            basePs.customFragShader = call->customFragShader;
            basePs.customVertShader = call->customVertShader;

            if (call->type == CallFill) {
                call->srb[0] = srbWithDummyTexture;
                call->srb[1] = srbWithCallTexture;

                // 1. Draw shapes
                QCRHIPipelineState ps = basePs;
                ps.stencilTestEnable = true;
                ps.stencilWriteMask = 0xFF;
                ps.stencilReadMask = 0xFF;
                ps.stencilFront = {
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::IncrementAndWrap,
                    QRhiGraphicsPipeline::Always
                };
                ps.stencilBack = {
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::DecrementAndWrap,
                    QRhiGraphicsPipeline::Always
                };
                ps.cullMode = QRhiGraphicsPipeline::None;
                ps.targetBlend.colorWrite = {};

                call->ps[0] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 2. Draw anti-aliased pixels
                ps.cullMode = QRhiGraphicsPipeline::Back;
                ps.targetBlend.colorWrite = QRhiGraphicsPipeline::ColorMask(0xF);
                ps.stencilFront = {
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Equal
                };
                ps.stencilBack = ps.stencilFront;
                ps.topology = QRhiGraphicsPipeline::TriangleStrip;

                call->ps[1] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 3. Draw fill
                ps.stencilFront = {
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::NotEqual
                };
                ps.stencilBack = ps.stencilFront;

                call->ps[2] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);
            } else if (call->type == CallConvexFill) {
                call->srb[0] = srbWithCallTexture;

                // 1. Draw fill
                call->ps[0] = pipeline(QCRHIPipelineStateKey::create(basePs, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 2. Draw antialiased edges
                QCRHIPipelineState ps = basePs;
                ps.topology = QRhiGraphicsPipeline::TriangleStrip;

                call->ps[1] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);
            } else if (call->type == CallStroke) {
                call->srb[0] = srbWithCallTexture;

                // 1. Draw Strokes (no stencil)
                QCRHIPipelineState ps = basePs;
                ps.topology = QRhiGraphicsPipeline::TriangleStrip;

                call->ps[0] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 2. Fill the stroke base without overlap
                ps.stencilTestEnable = true;
                ps.stencilWriteMask = 0xFF;
                ps.stencilReadMask = 0xFF;
                ps.stencilFront = {
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::IncrementAndClamp,
                    QRhiGraphicsPipeline::Equal
                };
                ps.stencilBack = ps.stencilFront;

                call->ps[1] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 3. Draw anti-aliased pixels.
                ps.stencilFront = {
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Keep,
                    QRhiGraphicsPipeline::Equal
                };
                ps.stencilBack = ps.stencilFront;

                call->ps[2] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);

                // 4. Clear stencil buffer
                ps.targetBlend.colorWrite = {};
                ps.stencilFront = {
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::StencilZero,
                    QRhiGraphicsPipeline::Always
                };
                ps.stencilBack = ps.stencilFront;

                call->ps[3] = pipeline(QCRHIPipelineStateKey::create(ps, rpDesc, srbForLayout), rpDesc, srbForLayout);
            } else if (call->type == CallText) {
                call->srb[0] = srbWithCallTexture;

                // 1.
                call->ps[0] = pipeline(
                    QCRHIPipelineStateKey::create(basePs, rpDesc, srbForLayout),
                    rpDesc,
                    srbForLayout);
            }
        }
    }
#ifdef QCPAINTER_PERF_DEBUG
    m_e->perf.tick();
#endif
    rhiCtx->cb->debugMarkEnd();
}

void QCPainterRhiRenderer::beginPrepareAndPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt)
{
    const float dpr = rt->devicePixelRatio();
    const float logicalWidth = rt->pixelSize().width() / dpr;
    const float logicalHeight = rt->pixelSize().height() / dpr;
    beginPrepareAndPaint(cb, rt, logicalWidth, logicalHeight, dpr);
}

void QCPainterRhiRenderer::beginPrepareAndPaint(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, float logicalWidth, float logicalHeight, float dpr)
{
    beginPrepare(cb, rt, dpr);
    m_e->beginPaint(logicalWidth, logicalHeight, dpr);
}

void QCPainterRhiRenderer::setCustomMatrix(const QMatrix4x4 &matrix)
{
    m_e->setCustomMatrix(matrix);
}

void QCPainterRhiRenderer::endPrepareAndPaint()
{
    m_e->endPaint();
    endPrepare();
}

void QCPainterRhiRenderer::bindPipeline(QCRHICall *call,
                                        int pipelineIndex, int srbIndex,
                                        const QRhiCommandBuffer::DynamicOffset &vertDynamicOffset,
                                        const QRhiCommandBuffer::DynamicOffset &dynamicOffset,
                                        bool indexedDraw,
                                        bool *needsViewport)
{
    rhiCtx->cb->setGraphicsPipeline(call->ps[pipelineIndex]);
    // Combine offsets
    QRhiCommandBuffer::DynamicOffset offsets[2] { dynamicOffset, vertDynamicOffset };
    rhiCtx->cb->setShaderResources(call->srb[srbIndex], 2, offsets);

    if (*needsViewport) {
        *needsViewport = false;
        const QSize size = rhiCtx->rt->pixelSize();
        rhiCtx->cb->setViewport({ 0.0f, 0.0f, float(size.width()), float(size.height()) });
    }

    if (call->renderFlags & QCPainterRhiRenderer::SimpleClipping)
        rhiCtx->cb->setScissor(call->scissor);

    QCRHIContext::PerPassData *ppd = rhiCtx->currentPerPassData();
    QRhiCommandBuffer::VertexInput vbufBinding(ppd->vertexBuffer, 0);
    QRhiBuffer *indexBuffer = ppd->indexBuffer;

    if (call->pathGroup != -1) {
        const QCRhiCachedPathGroup &cpg(rhiCtx->cachedPathGroups[call->pathGroup]);
        if (call->type == CallStroke) {
            vbufBinding.first = cpg.strokeVertexBuffer;
        } else {
            vbufBinding.first = cpg.fillVertexBuffer;
            indexBuffer = cpg.fillIndexBuffer;
        }
    }

    if (indexedDraw) {
        rhiCtx->cb->setVertexInput(
            0, 1, &vbufBinding, indexBuffer,
            0, QRhiCommandBuffer::IndexUInt32);
    } else {
        rhiCtx->cb->setVertexInput(0, 1, &vbufBinding);
    }
}

void QCPainterRhiRenderer::renderDelete()
{
    if (!rhiCtx)
        return;

    for (int i = 0; i < rhiCtx->texturesCount; i++) {
        if (rhiCtx->textures[i].tex
            && !(rhiCtx->textures[i].flags.testFlag(QCanvasPainter::ImageFlag::NativeTexture)))
            delete rhiCtx->textures[i].tex;
    }
    rhiCtx->textures.clear();
    rhiCtx->texturesCount = 0;

    for (const auto &samplerInfo : std::as_const(rhiCtx->samplers))
        delete samplerInfo.second;

    qDeleteAll(rhiCtx->pipelines);

    qDeleteAll(rhiCtx->srbs);

    for (auto it = rhiCtx->cachedPathGroups.begin(), end = rhiCtx->cachedPathGroups.end(); it != end; ++it) {
        delete it->fillVertexBuffer;
        delete it->fillIndexBuffer;
        delete it->strokeVertexBuffer;
    }

    for (const QCRHIContext::PerPassData &ppd : rhiCtx->perPassData) {
        delete ppd.vertexBuffer;
        delete ppd.indexBuffer;
        delete ppd.uniformBuffer;
    }
    rhiCtx->perPassData.clear();

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    delete rhiCtx->fontCache;
#endif

    rhiCtx->calls.clear();
    rhiCtx->paths.clear();
    rhiCtx->verts.clear();
    rhiCtx->indices.clear();
    rhiCtx->commonUniforms.clear();

    QVector<QCRhiCanvas> canvasesToDelete;
    std::swap(m_canvases, canvasesToDelete);
    for (QCRhiCanvas &canvas : canvasesToDelete)
        canvas.destroy();

    delete rhiCtx;
}

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
int QCPainterRhiRenderer::populateFont(
    const QFont &font,
    const QRectF &rect,
    const QString &text,
    QCRhiDistanceFieldGlyphCache::VertexList &vertices,
    QCRhiDistanceFieldGlyphCache::IndexList &indices,
    int *textureWidth,
    int *textureHeight)
{
    QCRHIContext *rc = rhiCtx;
    QCRHITexture *tex = nullptr;

    auto effectiveAlign = m_e->effectiveTextAlign(text);
    rc->fontCache->generate(text, rect, font, &(m_e->state), effectiveAlign, &vertices, &indices);

    QRhiResourceUpdateBatch *u = resourceUpdateBatch();

    rc->fontCache->commitResourceUpdates(u);
    const auto cacheKey = QCDistanceFieldGlyphCache::FontKey(QRawFont::fromFont(font));
    auto mainTexture = rc->fontCache->getCurrentTextures(cacheKey);
    auto currentTexture = rc->fontCache->getOldTextures(cacheKey);

    if (!mainTexture)
        return 0;

    // Font texture not created or changed
    if (!currentTexture) {
        tex = renderCreateNativeTexture(mainTexture);
        rc->fontCache->setOldTexture(cacheKey, tex->tex);
    }
    // Texture has already been created
    if (!tex && currentTexture != mainTexture) {
        tex = renderUpdateNativeTexture(currentTexture, mainTexture);
        rc->fontCache->setOldTexture(cacheKey, tex->tex);
    } else if (!tex) {
        // Find texture
        tex = findTexture(currentTexture);
    }

    // Todo: Gets only one texture, add support for multiple
    if (!tex)
        return 0;

    *textureWidth = tex->tex->pixelSize().width();
    *textureHeight = tex->tex->pixelSize().height();

    //Convert UVs to 0..1
    // for (int i = 0; i < vertices.size(); ++i) {
    //     vertices[i].tx /= *textureWidth;
    //     vertices[i].ty /= *textureHeight;
    // }


    tex->width = *textureWidth;
    tex->height = *textureHeight;

    return tex->id;
}
#endif

QCContext *QCPainterRhiRenderer::createRhiContext(QRhi *rhi)
{
    rhiCtx = new QCRHIContext;
    rhiCtx->rhi = rhi;
    QCContext *ctx = m_e->initialize(this);
    if (!ctx) {
        qWarning("Failed to create QCPainterEngine");
        return nullptr;
    }
    if (!renderCreate()) {
        qWarning("Failed to create QCPainterRhiRenderer");
        delete ctx;
        return nullptr;
    }
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    rhiCtx->fontCache = new QCDistanceFieldGlyphCache(rhi);
#endif
    return ctx;
}

QCPainterRhiRenderer::QCPainterRhiRenderer()
{
    m_canvasGrabs.reserve(4);
}

QCPainterRhiRenderer::~QCPainterRhiRenderer()
{
    destroy();
}

void QCPainterRhiRenderer::create(QRhi *rhi, QCanvasPainter *painter)
{
    if (ctx)
        destroy();

    // One painter -> one engine -> one renderer at a time.
    m_painter = painter;
    auto *painterPriv = QCanvasPainterPrivate::get(m_painter);
    m_e = painterPriv->m_e;
    painterPriv->m_renderer = this;

    ctx = createRhiContext(rhi);
}

void QCPainterRhiRenderer::destroy()
{
    if (!ctx)
        return;

    if (m_e) {
        // The engine is owned by the painter. The same engine object (and painter)
        // can be reused with another renderer after this.
        m_e->cleanup();
        m_e = nullptr;
    }

    renderDelete();

    // Detach from the painter.
    auto *painterPriv = QCanvasPainterPrivate::get(m_painter);
    painterPriv->m_renderer = nullptr;

    m_painter = nullptr;
    ctx = nullptr;
}

void QCPainterRhiRenderer::render()
{
    // Get elapsed time since previous render, for custom brush iTime
    if (!rhiCtx->animationElapsedTimer.isValid())
        rhiCtx->animationElapsedTimer.start();
    rhiCtx->renderTimeElapsedMs = rhiCtx->animationElapsedTimer.restart();

    rhiCtx->cb->debugMarkBegin("QCanvasPainter render"_ba);

    bool needsViewport = true;
    for (int i = 0; i < rhiCtx->callsCount; i++) {
        QCRHICall *call = &rhiCtx->calls[i];
        int pathsCount = call->pathCount;
        if (pathsCount < 1 && call->type != CallText)
            continue;

        // The default offset at 0, with empty transform. NB have to add an offset to skip
        // the data exposed as the other uniform buffer binding.
        QRhiCommandBuffer::DynamicOffset vertDynamicOffsetForCall(4, rhiCtx->vertUniformBufferCommonRegionAlignedSize);
        int vbufOffsetAdjust = 0;
        int ibufOffsetAdjust = 0;
        if (call->pathGroup != -1) {
            const auto cpgIt = rhiCtx->cachedPathGroups.constFind(call->pathGroup);
            if (cpgIt == rhiCtx->cachedPathGroups.cend())
                continue;

            const QCRhiCachedPathGroup &cpg(*cpgIt);
            if (call->type == CallStroke) {
                vbufOffsetAdjust = cpg.strokeVertexBufferOffsetAdjust.value(call->pathGroupEntryId);
            } else if (call->type == CallFill || call->type == CallConvexFill) {
                const auto &offsets(cpg.fillBufferOffsetAdjust[call->pathGroupEntryId]);
                vbufOffsetAdjust = offsets.vertexOffset;
                ibufOffsetAdjust = offsets.indexOffset;
            }
        }

        const QCRHIPath *paths = &rhiCtx->paths[call->pathOffset];
        // vertUniformBufferOffset is 0 unless updateVertUniforms() was called (due to path groups and caching).
        vertDynamicOffsetForCall.second += call->vertUniformBufferOffset * rhiCtx->oneVertUniformBufferSize;

        // where part 3 (common for vs & fs) starts in the buffer
        const quint32 startOffset = rhiCtx->vertUniformBufferCommonRegionAlignedSize + rhiCtx->vertUniformsCount * rhiCtx->oneVertUniformBufferSize;
        QRhiCommandBuffer::DynamicOffset dynamicOffsetForCall(1, startOffset + call->commonUniformBufferOffset);
        QRhiCommandBuffer::DynamicOffset dynamicOffsetForCallPlusOne(1, startOffset + call->commonUniformBufferOffset + rhiCtx->oneCommonUniformBufferSize);

        if (call->type == CallFill) {
            // 1. Draw shapes
            if (call->indexCount) {
                bindPipeline(call, 0, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, true, &needsViewport);
                rhiCtx->cb->drawIndexed(call->indexCount, 1, call->indexOffset + ibufOffsetAdjust);
                logFillDrawCallCount++;
                logFillTriCount += call->indexCount / 3;
            }

            // 2. Draw anti-aliased pixels
            if (call->renderFlags & QCPainterRhiRenderer::Antialiasing) {
                bindPipeline(call, 1, 1, vertDynamicOffsetForCall, dynamicOffsetForCallPlusOne, false, &needsViewport);
                // Draw antialiased edges
                for (int i = 0; i < pathsCount; i++) {
                    rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                    logFillDrawCallCount++;
                    logFillTriCount += paths[i].strokeCount - 2;
                }
            }

            // 3. Draw fill
            bindPipeline(call, 2, 1, vertDynamicOffsetForCall, dynamicOffsetForCallPlusOne, false, &needsViewport);
            rhiCtx->cb->draw(call->triangleCount, 1, call->triangleOffset + vbufOffsetAdjust);
            logFillDrawCallCount++;
            logFillTriCount += call->triangleCount - 2;
        } else if (call->type == CallConvexFill) {
            // 1. Draw fill
            if (call->indexCount) {
                bindPipeline(call, 0, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, true, &needsViewport);
                rhiCtx->cb->drawIndexed(call->indexCount, 1, call->indexOffset + ibufOffsetAdjust);
                logFillDrawCallCount++;
                logFillTriCount += call->indexCount / 3;
            }

            // 2. Draw antialiased edges
            if (call->renderFlags & QCPainterRhiRenderer::Antialiasing) {
                bindPipeline(call, 1, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, false, &needsViewport);
                for (int i = 0; i < pathsCount; i++) {
                    if (paths[i].strokeCount > 0) {
                        rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                        logFillDrawCallCount++;
                        logFillTriCount += paths[i].strokeCount - 2;
                    }
                }
            }
        } else if (call->type == CallStroke) {
            if (!(rhiCtx->flags & QCPainterRhiRenderer::StencilStrokes)) {
                // 1. Draw Strokes
                bindPipeline(call, 0, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, false, &needsViewport);
                for (int i = 0; i < pathsCount; i++) {
                    rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                    logStrokeDrawCallCount++;
                    logStrokeTriCount += paths[i].strokeCount - 2;
                }
            } else {
                // 2. Fill the stroke base without overlap
                bindPipeline(call, 1, 0, vertDynamicOffsetForCall, dynamicOffsetForCallPlusOne, false, &needsViewport);
                for (int i = 0; i < pathsCount; i++) {
                    rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                    logStrokeDrawCallCount++;
                    logStrokeTriCount += paths[i].strokeCount - 2;
                }

                // 3. Draw anti-aliased pixels.
                if (call->renderFlags & QCPainterRhiRenderer::Antialiasing) {
                    bindPipeline(call, 2, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, false, &needsViewport);
                    for (int i = 0; i < pathsCount; i++) {
                        if (paths[i].strokeCount > 0) {
                            rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                            logStrokeDrawCallCount++;
                            logStrokeTriCount += paths[i].strokeCount - 2;
                        }
                    }
                }

                //  4. Clear stencil buffer.
                bindPipeline(call, 3, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, false, &needsViewport);
                for (int i = 0; i < pathsCount; i++) {
                    if (paths[i].strokeCount > 0) {
                        rhiCtx->cb->draw(paths[i].strokeCount, 1, paths[i].strokeOffset + vbufOffsetAdjust);
                        logStrokeDrawCallCount++;
                        logStrokeTriCount += paths[i].strokeCount - 2;
                    }
                }
            }
        } else if (call->type == CallText) {
#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
            bindPipeline(call, 0, 0, vertDynamicOffsetForCall, dynamicOffsetForCall, true, &needsViewport);

            const int iCount = (call->triangleCount / 2) * 3;
            if (call->textTriangleOffsetBakedInToIndices)
                rhiCtx->cb->drawIndexed(iCount, 1, call->indexOffset);
            else
                rhiCtx->cb->drawIndexed(iCount, 1, call->indexOffset, call->triangleOffset);

            logTextDrawCallCount++;
            logTextTriCount += iCount / 3;
#endif
        }
    }

    rhiCtx->cb->debugMarkEnd();

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    rhiCtx->fontCache->optimizeCache();
#endif

    // Bump passId. This changes what currentPerPassData returns, and the value
    // is used as the SrbKey too. This allows subsequent render passes with this
    // same renderer (and painter+engine) within the same frame. This is
    // relevant in particular when multiple widgets or Quick items use
    // QCanvasPainterFactory::sharedInstance(), i.e. reusing the same
    // painter+engine+renderer. These subsequent drawing passes have their own
    // dedicated buffers, and by extension shader resource binding objects, to
    // not conflict with the data needed by the earlier draw calls.
    rhiCtx->passId += 1;
}

void QCPainterRhiRenderer::resetForPass()
{
    rhiCtx->pathsCount = 0;
    rhiCtx->callsCount = 0;

    rhiCtx->vertsCount = 0;
    rhiCtx->indicesCount = 0;

    rhiCtx->commonUniformsCount = 0;
    // Note: There is always 1 dynamic vert uniform, with the default transformation.
    rhiCtx->vertUniformsCount = 1;

    rhiCtx->flags &= ~(QCPainterRhiRenderer::SimpleClipping | QCPainterRhiRenderer::TransformedClipping);
}

void QCPainterRhiRenderer::resetForNewFrame()
{
    // Assumptions:
    //
    // Might be called multiple times. (e.g. when multiple widgets/items share the
    // same painter, and they do not know about each other, all calling this
    // function)
    //
    // Might be called at unusual times, e.g. when the previous frame is submitted.
    // Do not make assumptions that this directly corresponds to QRhi::beginFrame().
    //
    // Or it might not be called at all before the very first frame when the
    // renderer has just been created.

    rhiCtx->passId = 0;

    resetForPass();

    resetDebugCounters();

    for (auto it = rhiCtx->cachedPathGroups.begin(), end = rhiCtx->cachedPathGroups.end(); it != end; ++it) {
        it->fillBuffersReusableInCurrentFrame = true;
        it->strokeBufferReusableInCurrentFrame = true;
    }
}

void QCPainterRhiRenderer::resetDebugCounters()
{
    logFillDrawCallCount = 0;
    logStrokeDrawCallCount = 0;
    logTextDrawCallCount = 0;
    logFillTriCount = 0;
    logStrokeTriCount = 0;
    logTextTriCount = 0;
}

void QCPainterRhiRenderer::syncDebugCounters()
{
    QCanvasPainterPrivate *pd = QCanvasPainterPrivate::get(m_painter);
    auto &dc(ctx->debugCounters);

    dc.fillDrawCallCount = logFillDrawCallCount;
    dc.strokeDrawCallCount = logStrokeDrawCallCount;
    dc.textDrawCallCount = logTextDrawCallCount;
    dc.fillTriangleCount = logFillTriCount;
    dc.strokeTriangleCount = logStrokeTriCount;
    dc.textTriangleCount = logTextTriCount;
    dc.drawCallCount = logFillDrawCallCount + logStrokeDrawCallCount + logTextDrawCallCount;
    dc.triangleCount = logFillTriCount + logStrokeTriCount + logTextTriCount;

    dc.imageMemoryUsage = pd->m_imageTracker.dataAmount() / 1000;
    dc.imageCount = pd->m_imageTracker.size();

    dc.pathGroupCount = rhiCtx->cachedPathGroups.count();
    dc.cachedSubpathCount = 0;
    dc.cachedPathVertexDataSize = 0;
    for (auto it = rhiCtx->cachedPathGroups.cbegin(), end = rhiCtx->cachedPathGroups.cend(); it != end; ++it) {
        dc.cachedSubpathCount += it->totalSubpathCount;
        dc.cachedPathVertexDataSize += it->cachedVertexDataBytes;
    }
}

// When hasDrawCalls() is false, it effectively means that
// endPrepare() and render() would not do anything meaningful.
bool QCPainterRhiRenderer::hasDrawCalls() const
{
    return rhiCtx && rhiCtx->callsCount > 0;
}

bool QCPainterRhiRenderer::testFlag(RenderFlag flag) const
{
    return rhiCtx->flags.testFlag(flag);
}

void QCPainterRhiRenderer::setFlag(RenderFlags flag, bool enable)
{
    if (rhiCtx) {
        if (enable)
            rhiCtx->flags |= flag;
        else
            rhiCtx->flags &= ~flag;

        // Antialiasing requires also adjusting the antialiasingEnabled
        if (flag == QCPainterRhiRenderer::Antialiasing)
            ctx->antialiasingEnabled = enable;

        // Note: renderCreate doesn't seem to be required
        //renderCreate();
    }
}

QCPainterRhiRenderer::RenderFlags QCPainterRhiRenderer::flags() const
{
    return rhiCtx ? rhiCtx->flags : QCPainterRhiRenderer::RenderFlags();
}

bool QCPainterRhiRenderer::isPathCachedForFill(QCanvasPath *path, int pathGroup, const QCCachedPathFillProperties &fillProperties)
{
    auto it = rhiCtx->cachedPathGroups.constFind(pathGroup);
    if (it != rhiCtx->cachedPathGroups.cend()) {
        auto pit = it->cachedPaths.constFind(path);
        if (pit != it->cachedPaths.cend())
            return pit->fill.contains(fillProperties);
    }
    return false;
}

bool QCPainterRhiRenderer::isPathCachedForStroke(QCanvasPath *path, int pathGroup, const QCCachedPathStrokeProperties &strokeProperties)
{
    auto it = rhiCtx->cachedPathGroups.constFind(pathGroup);
    if (it != rhiCtx->cachedPathGroups.cend()) {
        auto pit = it->cachedPaths.constFind(path);
        if (pit != it->cachedPaths.cend())
            return pit->stroke.contains(strokeProperties);
    }
    return false;
}

void QCPainterRhiRenderer::removePathGroup(int pathGroup)
{
    auto it = rhiCtx->cachedPathGroups.constFind(pathGroup);
    if (it != rhiCtx->cachedPathGroups.cend()) {
        if (it->fillVertexBuffer)
            it->fillVertexBuffer->deleteLater();
        if (it->fillIndexBuffer)
            it->fillIndexBuffer->deleteLater();
        if (it->strokeVertexBuffer)
            it->strokeVertexBuffer->deleteLater();
        rhiCtx->cachedPathGroups.erase(it);
    }
}

static inline bool isCompressedFormat(QRhiTexture::Format format)
{
    return (format >= QRhiTexture::BC1 && format <= QRhiTexture::BC7)
            || (format >= QRhiTexture::ETC2_RGB8 && format <= QRhiTexture::ETC2_RGBA8)
            || (format >= QRhiTexture::ASTC_4x4 && format <= QRhiTexture::ASTC_12x12);
}

static void compressedFormatInfo(QRhiTexture::Format format, QSize size, quint32 *bpl, quint32 *byteSize, QSize *blockDim)
{
    int xdim = 4;
    int ydim = 4;
    quint32 blockSize = 0;

    switch (format) {
    case QRhiTexture::BC1:
        blockSize = 8;
        break;
    case QRhiTexture::BC2:
        blockSize = 16;
        break;
    case QRhiTexture::BC3:
        blockSize = 16;
        break;
    case QRhiTexture::BC4:
        blockSize = 8;
        break;
    case QRhiTexture::BC5:
        blockSize = 16;
        break;
    case QRhiTexture::BC6H:
        blockSize = 16;
        break;
    case QRhiTexture::BC7:
        blockSize = 16;
        break;

    case QRhiTexture::ETC2_RGB8:
        blockSize = 8;
        break;
    case QRhiTexture::ETC2_RGB8A1:
        blockSize = 8;
        break;
    case QRhiTexture::ETC2_RGBA8:
        blockSize = 16;
        break;

    case QRhiTexture::ASTC_4x4:
        blockSize = 16;
        break;
    case QRhiTexture::ASTC_5x4:
        blockSize = 16;
        xdim = 5;
        break;
    case QRhiTexture::ASTC_5x5:
        blockSize = 16;
        xdim = ydim = 5;
        break;
    case QRhiTexture::ASTC_6x5:
        blockSize = 16;
        xdim = 6;
        ydim = 5;
        break;
    case QRhiTexture::ASTC_6x6:
        blockSize = 16;
        xdim = ydim = 6;
        break;
    case QRhiTexture::ASTC_8x5:
        blockSize = 16;
        xdim = 8;
        ydim = 5;
        break;
    case QRhiTexture::ASTC_8x6:
        blockSize = 16;
        xdim = 8;
        ydim = 6;
        break;
    case QRhiTexture::ASTC_8x8:
        blockSize = 16;
        xdim = ydim = 8;
        break;
    case QRhiTexture::ASTC_10x5:
        blockSize = 16;
        xdim = 10;
        ydim = 5;
        break;
    case QRhiTexture::ASTC_10x6:
        blockSize = 16;
        xdim = 10;
        ydim = 6;
        break;
    case QRhiTexture::ASTC_10x8:
        blockSize = 16;
        xdim = 10;
        ydim = 8;
        break;
    case QRhiTexture::ASTC_10x10:
        blockSize = 16;
        xdim = ydim = 10;
        break;
    case QRhiTexture::ASTC_12x10:
        blockSize = 16;
        xdim = 12;
        ydim = 10;
        break;
    case QRhiTexture::ASTC_12x12:
        blockSize = 16;
        xdim = ydim = 12;
        break;

    default:
        qWarning("Unhandled compressed texture format %d in QCanvasPainter compressedFormatInfo", int(format));
        break;
    }

    const quint32 wblocks = uint((size.width() + xdim - 1) / xdim);
    const quint32 hblocks = uint((size.height() + ydim - 1) / ydim);

    if (bpl)
        *bpl = wblocks * blockSize;
    if (byteSize)
        *byteSize = wblocks * hblocks * blockSize;
    if (blockDim)
        *blockDim = QSize(xdim, ydim);
}

void QCPainterRhiRenderer::textureFormatInfo(QRhiTexture::Format format, QSize size,
                                             quint32 *bpl, quint32 *byteSize, quint32 *bytesPerPixel)
{
    if (isCompressedFormat(format)) {
        compressedFormatInfo(format, size, bpl, byteSize, nullptr);
        return;
    }

    quint32 bpc = 0;
    switch (format) {
    case QRhiTexture::RGBA8:
        bpc = 4;
        break;
    case QRhiTexture::BGRA8:
        bpc = 4;
        break;
    case QRhiTexture::R8:
        bpc = 1;
        break;
    case QRhiTexture::RG8:
        bpc = 2;
        break;
    case QRhiTexture::R16:
        bpc = 2;
        break;
    case QRhiTexture::RG16:
        bpc = 4;
        break;
    case QRhiTexture::RED_OR_ALPHA8:
        bpc = 1;
        break;

    case QRhiTexture::RGBA16F:
        bpc = 8;
        break;
    case QRhiTexture::RGBA32F:
        bpc = 16;
        break;
    case QRhiTexture::R16F:
        bpc = 2;
        break;
    case QRhiTexture::R32F:
        bpc = 4;
        break;

    case QRhiTexture::RGB10A2:
        bpc = 4;
        break;

    case QRhiTexture::D16:
        bpc = 2;
        break;
    case QRhiTexture::D24:
    case QRhiTexture::D24S8:
    case QRhiTexture::D32F:
        bpc = 4;
        break;

    case QRhiTexture::D32FS8:
        bpc = 8;
        break;

    case QRhiTexture::R8SI:
    case QRhiTexture::R8UI:
        bpc = 1;
        break;
    case QRhiTexture::R32SI:
    case QRhiTexture::R32UI:
        bpc = 4;
        break;
    case QRhiTexture::RG32SI:
    case QRhiTexture::RG32UI:
        bpc = 8;
        break;
    case QRhiTexture::RGBA32SI:
    case QRhiTexture::RGBA32UI:
        bpc = 16;
        break;

    default:
        qWarning("Unhandled texture format %d in QCanvasPainter textureFormatInfo", int(format));
        break;
    }

    if (bpl)
        *bpl = uint(size.width()) * bpc;
    if (byteSize)
        *byteSize = uint(size.width() * size.height()) * bpc;
    if (bytesPerPixel)
        *bytesPerPixel = bpc;
}

bool operator==(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept
{
    if (a.tex != b.tex
        || a.msaaColorBuffer != b.msaaColorBuffer
        || a.ds != b.ds
        || a.rt != b.rt
        || a.rp != b.rp
        || a.flags != b.flags
        || (a.tex && b.tex && a.tex->globalResourceId() != b.tex->globalResourceId()))
    {
        return false;
    }
    return true;
}

bool operator!=(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept
{
    return !(a == b);
}

QCanvasOffscreenCanvas QCPainterRhiRenderer::createCanvas(QSize pixelSize, int sampleCount, QCanvasOffscreenCanvas::Flags flags)
{
    QCanvasOffscreenCanvas canvas;
    if (!rhiCtx || !rhiCtx->rhi) {
        qWarning("Cannot create a canvas without a QRhi");
        return canvas;
    }

    if (sampleCount > 1) {
        if (flags.testFlag(QCanvasOffscreenCanvas::Flag::PreserveContents)) {
            qWarning("PreserveContents is not supported for multisample canvas");
            flags.setFlag(QCanvasOffscreenCanvas::Flag::PreserveContents, false);
        }
        if (flags.testFlag(QCanvasOffscreenCanvas::Flag::MipMaps)) {
            qWarning("MipMaps are not supported for multisample canvas");
            flags.setFlag(QCanvasOffscreenCanvas::Flag::MipMaps, false);
        }
    }

    QRhiTexture::Flags textureFlags = QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource;
    if (flags.testFlag(QCanvasOffscreenCanvas::Flag::MipMaps))
        textureFlags |= QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips;

    std::unique_ptr<QRhiTexture> tex(rhiCtx->rhi->newTexture(QRhiTexture::RGBA8, pixelSize, 1, textureFlags));
    if (!tex->create())
        return canvas;

    std::unique_ptr<QRhiRenderBuffer> msaaColorBuffer;
    if (sampleCount > 1) {
        msaaColorBuffer.reset(rhiCtx->rhi->newRenderBuffer(QRhiRenderBuffer::Color, pixelSize, sampleCount));
        if (!msaaColorBuffer->create())
            return canvas;
    }

    std::unique_ptr<QRhiRenderBuffer> ds(rhiCtx->rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, pixelSize, sampleCount));
    if (!ds->create())
        return canvas;

    QRhiTextureRenderTargetDescription rtDesc;
    if (sampleCount <= 1) {
        rtDesc.setColorAttachments({ tex.get() });
    } else {
        QRhiColorAttachment att(msaaColorBuffer.get());
        att.setResolveTexture(tex.get());
        rtDesc.setColorAttachments({ att });
    }
    rtDesc.setDepthStencilBuffer(ds.get());

    QRhiTextureRenderTarget::Flags rtFlags;
    if (flags.testFlag(QCanvasOffscreenCanvas::Flag::PreserveContents)) {
        // See PreserveColorContents docs for the downsides. With tiled GPUs
        // this likely has a performance hit. And with MSAA it may not work at
        // all (like if the GLES extension is used so that msaaColorBuffer will
        // not have any actual contents whatsoever; nothing to preserve either...).
        rtFlags |= QRhiTextureRenderTarget::PreserveColorContents;
    }

    std::unique_ptr<QRhiTextureRenderTarget> rt(rhiCtx->rhi->newTextureRenderTarget(rtDesc, rtFlags));
    std::unique_ptr<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.get());
    if (!rt->create())
        return canvas;

    QCanvasOffscreenCanvasPrivate *cd = QCanvasOffscreenCanvasPrivate::get(&canvas);
    cd->rhiCanvas.tex = tex.release();
    cd->rhiCanvas.msaaColorBuffer = sampleCount > 1 ? msaaColorBuffer.release() : nullptr;
    cd->rhiCanvas.ds = ds.release();
    cd->rhiCanvas.rt = rt.release();
    cd->rhiCanvas.rp = rp.release();
    cd->rhiCanvas.flags = flags;

    m_canvases.append(cd->rhiCanvas);
    return canvas;
}

void QCRhiCanvas::destroy()
{
    delete rp;
    delete rt;
    delete ds;
    delete msaaColorBuffer;
    delete tex;
}

void QCPainterRhiRenderer::destroyCanvas(QCanvasOffscreenCanvas &canvas)
{
    // no detach!
    QCanvasOffscreenCanvasPrivate *cd = QCanvasOffscreenCanvasPrivate::get(&canvas);

    m_canvases.removeOne(cd->rhiCanvas);

    cd->rhiCanvas.destroy();
    cd->rhiCanvas = {}; // canvas, incl. shared ones, becomes a null canvas
}

QRhiRenderTarget *QCPainterRhiRenderer::canvasRenderTarget(const QCanvasOffscreenCanvas &canvas)
{
    const QCanvasOffscreenCanvasPrivate *cd = QCanvasOffscreenCanvasPrivate::get(&canvas);
    return cd->rhiCanvas.rt;
}

void QCPainterRhiRenderer::recordCanvasRenderPass(QRhiCommandBuffer *cb, const QCanvasOffscreenCanvas &canvas)
{
    cb->debugMarkBegin("QC Canvas render pass"_ba);
    cb->beginPass(canvasRenderTarget(canvas), canvas.fillColor(), { 1.0f, 0 });
    render();
    cb->endPass();
    cb->debugMarkEnd();
}

void QCPainterRhiRenderer::grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback, QRhiCommandBuffer *maybeCb)
{
    if (canvas.isNull()) {
        qWarning("Cannot grab null canvas");
        return;
    }

    m_canvasGrabs.append({ {}, callback });
    const int grabIndex = m_canvasGrabs.count() - 1;
    auto &grabInfo = m_canvasGrabs.last();

    // ### there's probably an issue when the vector changes (due to remove or growing),
    // the QRhiReadbackResult refs the QRhi may hold could become invalid...
    // The ctor does reserve(4) atm.

    auto callbackInvoker = [this, grabIndex] {
        QRhiReadbackResult &readbackResult(m_canvasGrabs[grabIndex].first);
        // assume QRhiTexture::RGBA8
        QImage image(reinterpret_cast<const uchar *>(readbackResult.data.constData()),
                        readbackResult.pixelSize.width(),
                        readbackResult.pixelSize.height(),
                        QImage::Format_RGBA8888);
        if (rhiCtx->rhi->isYUpInFramebuffer())
            image.flip();
        m_canvasGrabs[grabIndex].second(image); // invoke the callback
        m_canvasGrabs.removeAt(grabIndex);
    };

    QRhi *rhi = rhiCtx->rhi;
    if (maybeCb) {
        // A frame is being recorded already.
        grabInfo.first.completed = callbackInvoker;
        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->readBackTexture({ canvas.texture() }, &grabInfo.first);
        maybeCb->resourceUpdate(u);
        // callback might be invoked later. In a future frame, possibly.
    } else {
        // Outside a frame.
        grabInfo.first.completed = nullptr;
        QRhiCommandBuffer *cb;
        rhi->beginOffscreenFrame(&cb);
        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        u->readBackTexture({ canvas.texture() }, &grabInfo.first);
        cb->resourceUpdate(u);
        rhi->endOffscreenFrame();

        // The readback result callback would have been invoked from
        // endOffscreenFrame. We disabled that by leaving 'completed' unset.
        // This is because depending on what the callback does, it might be too
        // early to be invoked then. Consider opening a QFileDialog, which in
        // turn triggers paints on some other window, which in turn start a new
        // frame repainting a widget or Quick window using QRhi, while
        // endOffscreenFrame() is not fully finished. Probably should be
        // improved in qtbase. Until then, invoke the callback manually now that
        // endOffscreenFrame has returned.

        callbackInvoker();
    }
}

void QCPainterRhiRenderer::recordRenderPass(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QColor &clearColor)
{
    cb->debugMarkBegin("QC render pass"_ba);
    cb->beginPass(rt, clearColor, { 1.0f, 0 });
    render();
    cb->endPass();
    cb->debugMarkEnd();
}

bool QCPainterRhiRenderer::isYUpInNDC() const
{
    return rhiCtx->rhi->isYUpInNDC();
}

QT_END_NAMESPACE
