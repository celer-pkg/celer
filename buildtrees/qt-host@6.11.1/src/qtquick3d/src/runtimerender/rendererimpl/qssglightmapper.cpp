// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssglightmapper_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhiquadrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DUtils/private/qssgutils_p.h>

#ifdef QT_QUICK3D_HAS_LIGHTMAPPER
#include <QtCore/qfuture.h>
#include <QtCore/qfileinfo.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QRandomGenerator>
#include <qsimd.h>
#include <embree3/rtcore.h>
#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>
#include <QDir>
#include <QBuffer>
#include <QWaitCondition>
#include <QMutex>
#include <QTemporaryFile>
#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#include <QOpenGLContext>
#endif
#endif

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

// References:
//   https://ndotl.wordpress.com/2018/08/29/baking-artifact-free-lightmaps/
//   https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/
//   https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/gdc2018-precomputedgiobalilluminationinfrostbite.pdf
//   https://therealmjp.github.io/posts/new-blog-series-lightmap-baking-and-spherical-gaussians/
//   https://computergraphics.stackexchange.com/questions/2316/is-russian-roulette-really-the-answer
//   https://computergraphics.stackexchange.com/questions/4664/does-cosine-weighted-hemisphere-sampling-still-require-ndotl-when-calculating-co
//   https://www.rorydriscoll.com/2009/01/07/better-sampling/
//   https://github.com/TheRealMJP/BakingLab
//   https://github.com/candycat1992/LightmapperToy
//   https://github.com/godotengine/
//   https://github.com/jpcy/xatlas

#ifdef QT_QUICK3D_HAS_LIGHTMAPPER

static constexpr int GAUSS_HALF_KERNEL_SIZE = 3;
static constexpr int DIRECT_MAP_UPSCALE_FACTOR = 4;
static constexpr int MAX_TILE_SIZE = 1024;
static constexpr quint32 PIXEL_VOID = 0; // Pixel not part of any mask
static constexpr quint32 PIXEL_UNSET = -1; // Pixel part of mask, but not yet set
static constexpr char KEY_SCENE_METADATA[] = "_qt_scene_metadata";

static void floodFill(quint32 *maskUintPtr, const int rows, const int cols)
{
    quint32 targetColor = 1;
    QList<std::array<int, 2>> stack;
    stack.reserve(rows * cols);
    for (int y0 = 0; y0 < rows; y0++) {
        for (int x0 = 0; x0 < cols; x0++) {
            bool filled = false;
            stack.push_back({ x0, y0 });
            while (!stack.empty()) {
                const auto [x, y] = stack.takeLast();
                const int idx = cols * y + x;
                const quint32 value = maskUintPtr[idx];

                // If the target color is already the same as the replacement color, no need to proceed
                if (value != PIXEL_UNSET)
                    continue;

                // Fill the current cell with the replacement color
                maskUintPtr[idx] = targetColor;
                filled = true;

                // Push the neighboring cells onto the stack
                if (x + 1 < cols)
                    stack.push_back({ x + 1, y });
                if (x > 0)
                    stack.push_back({ x - 1, y });
                if (y + 1 < rows)
                    stack.push_back({ x, y + 1 });
                if (y > 0)
                    stack.push_back({ x, y - 1 });
            }

            if (filled) {
                do {
                    targetColor++;
                } while (targetColor == PIXEL_VOID || targetColor == PIXEL_UNSET);
            }
        }
    }
}

static QString formatDuration(quint64 milliseconds, bool showMilliseconds = true)
{
    const quint64 partMilliseconds = milliseconds % 1000;
    const quint64 partSeconds = (milliseconds / 1000) % 60;
    const quint64 partMinutes = (milliseconds / 60000) % 60;
    const quint64 partHours = (milliseconds / 3600000) % 60;

    if (partHours > 0) {
        return showMilliseconds
                ? QStringLiteral("%1h %2m %3s %4ms").arg(partHours).arg(partMinutes).arg(partSeconds).arg(partMilliseconds)
                : QStringLiteral("%1h %2m %3s").arg(partHours).arg(partMinutes).arg(partSeconds);
    }
    if (partMinutes > 0) {
        return showMilliseconds ? QStringLiteral("%1m %2s %3ms").arg(partMinutes).arg(partSeconds).arg(partMilliseconds)
                                : QStringLiteral("%1m %2s").arg(partMinutes).arg(partSeconds);
    }
    if (partSeconds > 0) {
        return showMilliseconds ? QStringLiteral("%1s %2ms").arg(partSeconds).arg(partMilliseconds)
                                : QStringLiteral("%1s").arg(partSeconds);
    }
    return showMilliseconds ? QStringLiteral("%1ms").arg(partMilliseconds) : QStringLiteral("0s");
}

enum class Stage {
    Direct = 0,
    Indirect = 1,
    Denoise = 2
};

struct ProgressTracker
{
    void initBake(quint32 numIndirectSamples, quint32 numIndirectBounces)
    {
        // Just guesstimating the relative work loads here
        const double direct = 2;
        const double indirect = numIndirectSamples * numIndirectBounces;
        const double denoise = 1;
        const double combined = direct + indirect + denoise;

        fractionDirect = qMax(direct / combined, 0.02); // Make direct and denoise at least 2% for cosmetics
        fractionDenoise = qMax(denoise / combined, 0.02);
        fractionIndirect = qMax(1.0 - fractionDirect - fractionDenoise, 0.0);
    }

    void initDenoise()
    {
        fractionDirect = 0;
        fractionDenoise = 1;
        fractionIndirect = 0;
    }

    void setTotalDirectTiles(quint32 totalDirectTilesNew)
    {
        totalDirectTiles = totalDirectTilesNew;
    }

    void setStage(Stage stageNew)
    {
        if (stage == stageNew)
            return;
        stage = stageNew;
        if (stage == Stage::Indirect)
            indirectTimer.start();
    }

    double getEstimatedTimeRemaining()
    {
        double estimatedTimeRemaining = -1.0;
        if (stage == Stage::Indirect && indirectTimer.isValid()) {
            double totalElapsed = indirectTimer.elapsed();
            double fullEstimate = static_cast<double>(totalElapsed) / progressIndirect;
            estimatedTimeRemaining = (1.0 - progressIndirect) * fullEstimate;
        }
        return estimatedTimeRemaining;
    }

    double getProgress()
    {
        return progress;
    }

    void directTileDone()
    {
        Q_ASSERT(stage == Stage::Direct);
        directTilesDone++;
        progress = (fractionDirect * directTilesDone) / qMax(1u, totalDirectTiles);
    }

    void denoisedModelDone(int i, int n)
    {
        Q_ASSERT(stage == Stage::Denoise);
        progress = fractionDirect + fractionIndirect + (fractionDenoise * double(i) / n);
    }

    void indirectTexelDone(quint64 i, quint64 n)
    {
        Q_ASSERT(stage == Stage::Indirect);
        progressIndirect = double(i) / n;
        progress = fractionDirect + (fractionIndirect * progressIndirect);
    }

private:
    double fractionDirect = 0;
    double fractionIndirect = 0;
    double fractionDenoise = 0;
    double progress = 0;
    double progressIndirect = 0;
    quint32 totalDirectTiles = 0;
    quint32 directTilesDone = 0;
    Stage stage = Stage::Direct;
    QElapsedTimer indirectTimer;
};

struct QSSGLightmapperPrivate
{
    explicit QSSGLightmapperPrivate() = default;

    QSSGLightmapperOptions options;
    QString outputPath;
    QVector<QSSGBakedLightingModel> bakedLightingModels;
    QRhi::Implementation rhiBackend = QRhi::Null;
    std::unique_ptr<QSSGRenderContextInterface> rhiCtxInterface;
    std::unique_ptr<QSSGRenderer> renderer;

    // For the main thread to wait on the lightmapper being initialized
    QWaitCondition initCondition;
    QMutex initMutex;

    QSSGLightmapper::Callback outputCallback;
    QSSGLightmapper::BakingControl bakingControl;
    QElapsedTimer totalTimer;

    struct SubMeshInfo {
        quint32 offset = 0;
        quint32 count = 0;
        unsigned int geomId = RTC_INVALID_GEOMETRY_ID;
        QVector4D baseColor;
        QSSGRenderImage *baseColorNode = nullptr;
        QRhiTexture *baseColorMap = nullptr;
        QVector3D emissiveFactor;
        QSSGRenderImage *emissiveNode = nullptr;
        QRhiTexture *emissiveMap = nullptr;
        QSSGRenderImage *normalMapNode = nullptr;
        QRhiTexture *normalMap = nullptr;
        float normalStrength = 0.0f;
        float opacity = 0.0f;
    };
    using SubMeshInfoList = QVector<SubMeshInfo>;
    QVector<SubMeshInfoList> subMeshInfos;

    struct DrawInfo {
        QSize lightmapSize;
        QByteArray vertexData;
        quint32 vertexStride;
        QByteArray indexData;
        QRhiCommandBuffer::IndexFormat indexFormat = QRhiCommandBuffer::IndexUInt32;
        quint32 positionOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format positionFormat = QRhiVertexInputAttribute::Float;
        quint32 normalOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format normalFormat = QRhiVertexInputAttribute::Float;
        quint32 uvOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format uvFormat = QRhiVertexInputAttribute::Float;
        quint32 lightmapUVOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format lightmapUVFormat = QRhiVertexInputAttribute::Float;
        quint32 tangentOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format tangentFormat = QRhiVertexInputAttribute::Float;
        quint32 binormalOffset = UINT_MAX;
        QRhiVertexInputAttribute::Format binormalFormat = QRhiVertexInputAttribute::Float;
        int meshIndex = -1; // Maps to an index in meshInfos;
    };
    QVector<DrawInfo> drawInfos; // per model
    QVector<QByteArray> meshes;

    struct Light {
        enum {
            Directional,
            Point,
            Spot
        } type;
        bool indirectOnly;
        QVector3D direction;
        QVector3D color;
        QVector3D worldPos;
        float cosConeAngle;
        float cosInnerConeAngle;
        float constantAttenuation;
        float linearAttenuation;
        float quadraticAttenuation;
    };
    QVector<Light> lights;

    RTCDevice rdev = nullptr;
    RTCScene rscene = nullptr;

    struct RasterResult {
        bool success = false;
        int width = 0;
        int height = 0;
        QByteArray worldPositions; // vec4
        QByteArray normals; // vec4
        QByteArray baseColors; // vec4, static color * texture map value (both linear)
        QByteArray emissions; // vec4, static factor * emission map value
    };

    struct ModelTexel {
        QVector3D worldPos;
        QVector3D normal;
        QVector4D baseColor; // static color * texture map value (both linear)
        QVector3D emission; // static factor * emission map value
        bool isValid() const { return !worldPos.isNull() && !normal.isNull(); }
    };

    QVector<QVector<ModelTexel>> modelTexels; // commit geom
    QVector<bool> modelHasBaseColorTransparency;
    quint32 emissiveModelCount = 0; // Models that has any ModelTexel with emission > 0.
    QVector<quint32> numValidTexels;

    QVector<int> geomLightmapMap; // [geomId] -> index in lightmaps (NB lightmap is per-model, geomId is per-submesh)
    QVector<float> subMeshOpacityMap; // [geomId] -> opacity

    bool denoiseOnly = false;

    double totalProgress = 0; // [0-1]
    qint64 estimatedTimeRemaining = -1; // ms
    quint64 indirectTexelsTotal = 0;
    quint64 indirectTexelsDone = 0;

    inline const ModelTexel &texelForLightmapUV(unsigned int geomId, float u, float v) const
    {
        // find the hit texel in the lightmap for the model to which the submesh with geomId belongs
        const int modelIdx = geomLightmapMap[geomId];
        QSize texelSize = drawInfos[modelIdx].lightmapSize;
        u = qBound(0.0f, u, 1.0f);
        // flip V, CPU-side data is top-left based
        v = 1.0f - qBound(0.0f, v, 1.0f);

        const int w = texelSize.width();
        const int h = texelSize.height();
        const int x = qBound(0, int(w * u), w - 1);
        const int y = qBound(0, int(h * v), h - 1);
        const int texelIdx = x + y * w;

        return modelTexels[modelIdx][texelIdx];
    }

    bool userCancelled();
    void sendOutputInfo(QSSGLightmapper::BakingStatus type,
                        std::optional<QString> msg,
                        bool outputToConsole = true,
                        bool outputConsoleTimeRemanining = false);
    void updateStage(const QString &newStage);
    bool commitGeometry();
    bool prepareLightmaps();
    bool verifyLights() const;
    QVector<QVector3D> computeDirectLight(int lmIdx);
    QVector<QVector3D> computeIndirectLight(int lmIdx,
                                            int wgSizePerGroup,
                                            int wgCount);
    bool storeMeshes(QSharedPointer<QSSGLightmapWriter> tempFile);

    RasterResult rasterizeLightmap(int lmIdx,
                                QSize outputSize,
                                QVector2D minUVRegion = QVector2D(0, 0),
                                QVector2D maxUVRegion = QVector2D(1, 1));

    bool storeSceneMetadata(QSharedPointer<QSSGLightmapWriter> writer);
    bool storeMetadata(int lmIdx, QSharedPointer<QSSGLightmapWriter> tempFile);
    bool storeDirectLightData(int lmIdx, const QVector<QVector3D> &directLight, QSharedPointer<QSSGLightmapWriter> tempFile);
    bool storeIndirectLightData(int lmIdx, const QVector<QVector3D> &indirectLight, QSharedPointer<QSSGLightmapWriter> tempFile);
    bool storeMaskImage(int lmIdx, QSharedPointer<QSSGLightmapWriter> tempFile);

    bool denoiseLightmaps();

    QVector3D sampleDirectLight(QVector3D worldPos, QVector3D normal, bool allLight) const;
    QByteArray dilate(const QSize &pixelSize, const QByteArray &image);

    QString stage = QStringLiteral("Initializing");

    ProgressTracker progressTracker;
    qint64 bakeStartTime = 0;
};

// Used to output progress ETA during baking.
// Have to do it this way because we are blocking on the render thread, so no event loop
// for regular timers.
class TimerThread : public QThread {
    Q_OBJECT
public:
    TimerThread(QObject *parent = nullptr)
        : QThread(parent), intervalMs(1000), stopped(false) {}

    ~TimerThread() {
        stop();
        wait();
    }

    void setInterval(int ms) {
        intervalMs = ms;
    }

    void setCallback(const std::function<void()>& func) {
        callback = func;
    }

    void stop() {
        stopped = true;
    }

protected:
    void run() override {
        int elapsed = 0;
        while (!stopped) {
            msleep(100);
            if (stopped) break;

            elapsed += 100;
            if (elapsed >= intervalMs && callback) {
                callback();
                elapsed = 0;
            }
        }
    }

private:
    int intervalMs;
    std::function<void()> callback;
    std::atomic<bool> stopped;
};

static const int LM_SEAM_BLEND_ITER_COUNT = 4;

QSSGLightmapper::QSSGLightmapper() : d(new QSSGLightmapperPrivate())
{
#ifdef __SSE2__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
}

QSSGLightmapper::~QSSGLightmapper()
{
    reset();
    delete d;

#ifdef __SSE2__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
#endif
}

void QSSGLightmapper::reset()
{
    d->bakedLightingModels.clear();
    d->subMeshInfos.clear();
    d->drawInfos.clear();
    d->lights.clear();

    d->modelHasBaseColorTransparency.clear();
    d->emissiveModelCount = 0;
    d->meshes.clear();

    d->geomLightmapMap.clear();
    d->subMeshOpacityMap.clear();

    if (d->rscene) {
        rtcReleaseScene(d->rscene);
        d->rscene = nullptr;
    }
    if (d->rdev) {
        rtcReleaseDevice(d->rdev);
        d->rdev = nullptr;
    }

    d->bakingControl.cancelled = false;
    d->totalProgress = 0.0;
    d->estimatedTimeRemaining = -1;
}

void QSSGLightmapper::setOptions(const QSSGLightmapperOptions &options)
{
    d->options = options;
}

void QSSGLightmapper::setOutputCallback(Callback callback)
{
    d->outputCallback = callback;
}

qsizetype QSSGLightmapper::add(const QSSGBakedLightingModel &model)
{
    d->bakedLightingModels.append(model);
    return d->bakedLightingModels.size() - 1;
}

void QSSGLightmapper::setRhiBackend(QRhi::Implementation backend)
{
    d->rhiBackend = backend;
}

void QSSGLightmapper::setDenoiseOnly(bool value)
{
    d->denoiseOnly = value;
}

static void embreeErrFunc(void *, RTCError error, const char *str)
{
    qWarning("lm: Embree error: %d: %s", error, str);
}

static const unsigned int NORMAL_SLOT = 0;
static const unsigned int LIGHTMAP_UV_SLOT = 1;

static void embreeFilterFunc(const RTCFilterFunctionNArguments *args)
{
    RTCHit *hit = reinterpret_cast<RTCHit *>(args->hit);
    QSSGLightmapperPrivate *d = static_cast<QSSGLightmapperPrivate *>(args->geometryUserPtr);
    RTCGeometry geom = rtcGetGeometry(d->rscene, hit->geomID);

    // convert from barycentric and overwrite u and v in hit with the result
    rtcInterpolate0(geom, hit->primID, hit->u, hit->v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, LIGHTMAP_UV_SLOT, &hit->u, 2);

    const float opacity = d->subMeshOpacityMap[hit->geomID];
    const int modelIdx = d->geomLightmapMap[hit->geomID];
    if (opacity < 1.0f || d->modelHasBaseColorTransparency[modelIdx]) {
        const QSSGLightmapperPrivate::ModelTexel &texel(d->texelForLightmapUV(hit->geomID, hit->u, hit->v));

        // In addition to material.opacity, take at least the base color (both
        // the static color and the value from the base color map, if there is
        // one) into account. Opacity map, alpha cutoff, etc. are ignored.
        const float alpha = opacity * texel.baseColor.w();

        // Ignore the hit if the alpha is low enough. This is not exactly perfect,
        // but better than nothing. An object with an opacity lower than the
        // threshold will act is if it was not there, as far as the intersection is
        // concerned. So then the object won't cast shadows for example.
        if (alpha < d->options.opacityThreshold)
            args->valid[0] = 0;
    }
}

static QByteArray meshToByteArray(const QSSGMesh::Mesh &mesh)
{
    QByteArray meshData;
    QBuffer buffer(&meshData);
    buffer.open(QIODevice::WriteOnly);
    mesh.save(&buffer);

    return meshData;
}

// Function to extract a scale-only matrix from a transform matrix
static QMatrix4x4 extractScaleMatrix(const QMatrix4x4 &transform)
{
    Q_ASSERT(transform.isAffine());

    // Extract scale factors by computing the length of the basis vectors (columns)
    const QVector4D col0 = transform.column(0);
    const QVector4D col1 = transform.column(1);
    const QVector4D col2 = transform.column(2);

    const float scaleX = QVector3D(col0[0], col0[1], col0[2]).length(); // X column
    const float scaleY = QVector3D(col1[0], col1[1], col1[2]).length(); // Y column
    const float scaleZ = QVector3D(col2[0], col2[1], col2[2]).length(); // Z column

    // Construct a scale-only matrix
    QMatrix4x4 scaleMatrix;
    scaleMatrix.data()[0 * 4 + 0] = scaleX;
    scaleMatrix.data()[1 * 4 + 1] = scaleY;
    scaleMatrix.data()[2 * 4 + 2] = scaleZ;
    return scaleMatrix;
}

bool QSSGLightmapperPrivate::commitGeometry()
{
    if (bakedLightingModels.isEmpty()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("No models with usedInBakedLighting, cannot bake"));
        return false;
    }

    sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Preparing geometry..."));
    QElapsedTimer geomPrepTimer;
    geomPrepTimer.start();

    const auto &bufferManager(renderer->contextInterface()->bufferManager());

    const int bakedLightingModelCount = bakedLightingModels.size();
    subMeshInfos.resize(bakedLightingModelCount);
    drawInfos.resize(bakedLightingModelCount);
    modelTexels.resize(bakedLightingModelCount);
    modelHasBaseColorTransparency.resize(bakedLightingModelCount, false);

    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
        if (lm.renderables.isEmpty()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("No submeshes, model %1 cannot be lightmapped").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }
        if (lm.model->skin || lm.model->skeleton) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Skinned models not supported: %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }

        subMeshInfos[lmIdx].reserve(lm.renderables.size());
        for (const QSSGRenderableObjectHandle &handle : std::as_const(lm.renderables)) {
            Q_ASSERT(handle.obj->type == QSSGRenderableObject::Type::DefaultMaterialMeshSubset
                     || handle.obj->type == QSSGRenderableObject::Type::CustomMaterialMeshSubset);
            QSSGSubsetRenderable *renderableObj = static_cast<QSSGSubsetRenderable *>(handle.obj);
            SubMeshInfo info;
            info.offset = renderableObj->subset.offset;
            info.count = renderableObj->subset.count;
            info.opacity = renderableObj->opacity;
            if (handle.obj->type == QSSGRenderableObject::Type::DefaultMaterialMeshSubset) {
                const QSSGRenderDefaultMaterial *defMat = static_cast<const QSSGRenderDefaultMaterial *>(&renderableObj->material);
                info.baseColor = defMat->color;
                info.emissiveFactor = defMat->emissiveColor;
                if (defMat->colorMap) {
                    info.baseColorNode = defMat->colorMap;
                    QSSGRenderImageTexture texture = bufferManager->loadRenderImage(defMat->colorMap);
                    info.baseColorMap = texture.m_texture;
                }
                if (defMat->emissiveMap) {
                    info.emissiveNode = defMat->emissiveMap;
                    QSSGRenderImageTexture texture = bufferManager->loadRenderImage(defMat->emissiveMap);
                    info.emissiveMap = texture.m_texture;
                }
                if (defMat->normalMap) {
                    info.normalMapNode = defMat->normalMap;
                    QSSGRenderImageTexture texture = bufferManager->loadRenderImage(defMat->normalMap);
                    info.normalMap = texture.m_texture;
                    info.normalStrength = defMat->bumpAmount;
                }
            } else {
                info.baseColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
                info.emissiveFactor = QVector3D(0.0f, 0.0f, 0.0f);
            }
            subMeshInfos[lmIdx].append(info);
        }

        QMatrix4x4 worldTransform;
        QMatrix3x3 normalMatrix;
        QSSGSubsetRenderable *renderableObj = static_cast<QSSGSubsetRenderable *>(lm.renderables.first().obj);
        worldTransform = renderableObj->modelContext.globalTransform;
        normalMatrix = renderableObj->modelContext.normalMatrix;
        const QMatrix4x4 scaleTransform = extractScaleMatrix(worldTransform);

        DrawInfo &drawInfo(drawInfos[lmIdx]);
        QSSGMesh::Mesh mesh;

        if (lm.model->geometry)
            mesh = bufferManager->loadMeshData(lm.model->geometry);
        else
            mesh = bufferManager->loadMeshData(lm.model->meshPath);

        if (!mesh.isValid()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning,
                           QStringLiteral("Failed to load geometry for model %1").arg(lm.model->lightmapKey));
            return false;
        }

        QElapsedTimer unwrapTimer;
        unwrapTimer.start();
        // Use scene texelsPerUnit if the model's texelsPerUnit is unset (< 0)
        const float texelsPerUnit = lm.model->texelsPerUnit <= 0.0f ? options.texelsPerUnit : lm.model->texelsPerUnit;
        if (!mesh.createLightmapUVChannel(texelsPerUnit, scaleTransform)) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to do lightmap UV unwrapping for model %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }
        sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Lightmap UV unwrap done for model %1 in %2").
                                                              arg(lm.model->lightmapKey).
                                                              arg(formatDuration(unwrapTimer.elapsed())));

        if (lm.model->hasLightmap()) {
            QByteArray meshData = meshToByteArray(mesh);

            int meshIndex = -1;
            bool doAdd = true;
            for (int i = 0; i < meshes.size(); ++i) {
                if (meshData == meshes[i]) {
                    doAdd = false;
                    meshIndex = i;
                }
            }

            if (doAdd) {
                meshes.push_back(meshData);
                meshIndex = meshes.size() - 1;
            }
            drawInfo.meshIndex = meshIndex;
        }

        drawInfo.lightmapSize = mesh.subsets().first().lightmapSizeHint;
        drawInfo.vertexData = mesh.vertexBuffer().data;
        drawInfo.vertexStride = mesh.vertexBuffer().stride;
        drawInfo.indexData = mesh.indexBuffer().data;

        if (drawInfo.vertexData.isEmpty()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("No vertex data for model %1").arg(lm.model->lightmapKey));
            return false;
        }
        if (drawInfo.indexData.isEmpty()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("No index data for model %1").arg(lm.model->lightmapKey));
            return false;
        }

        switch (mesh.indexBuffer().componentType) {
        case QSSGMesh::Mesh::ComponentType::UnsignedInt16:
            drawInfo.indexFormat = QRhiCommandBuffer::IndexUInt16;
            break;
        case QSSGMesh::Mesh::ComponentType::UnsignedInt32:
            drawInfo.indexFormat = QRhiCommandBuffer::IndexUInt32;
            break;
        default:
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Unknown index component type %1 for model %2").
                                                                 arg(int(mesh.indexBuffer().componentType)).
                                                                 arg(lm.model->lightmapKey));
            break;
        }

        for (const QSSGMesh::Mesh::VertexBufferEntry &vbe : mesh.vertexBuffer().entries) {
            if (vbe.name == QSSGMesh::MeshInternal::getPositionAttrName()) {
                drawInfo.positionOffset = vbe.offset;
                drawInfo.positionFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            } else if (vbe.name == QSSGMesh::MeshInternal::getNormalAttrName()) {
                drawInfo.normalOffset = vbe.offset;
                drawInfo.normalFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            } else if (vbe.name == QSSGMesh::MeshInternal::getUV0AttrName()) {
                drawInfo.uvOffset = vbe.offset;
                drawInfo.uvFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            } else if (vbe.name == QSSGMesh::MeshInternal::getLightmapUVAttrName()) {
                drawInfo.lightmapUVOffset = vbe.offset;
                drawInfo.lightmapUVFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            } else if (vbe.name == QSSGMesh::MeshInternal::getTexTanAttrName()) {
                drawInfo.tangentOffset = vbe.offset;
                drawInfo.tangentFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            } else if (vbe.name == QSSGMesh::MeshInternal::getTexBinormalAttrName()) {
                drawInfo.binormalOffset = vbe.offset;
                drawInfo.binormalFormat = QSSGRhiHelpers::toVertexInputFormat(QSSGRenderComponentType(vbe.componentType), vbe.componentCount);
            }
        }

        if (!(drawInfo.positionOffset != UINT_MAX && drawInfo.normalOffset != UINT_MAX)) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Could not figure out position and normal attribute offsets for model %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }

        // We will manually access and massage the data, so cannot just work with arbitrary formats.
        if (!(drawInfo.positionFormat == QRhiVertexInputAttribute::Float3
              && drawInfo.normalFormat == QRhiVertexInputAttribute::Float3))
        {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Position or normal attribute format is not as expected (float3) for model %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }

        if (drawInfo.lightmapUVOffset == UINT_MAX) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Could not figure out lightmap UV attribute offset for model %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }

        if (drawInfo.lightmapUVFormat != QRhiVertexInputAttribute::Float2) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Lightmap UV attribute format is not as expected (float2) for model %1").
                                                                 arg(lm.model->lightmapKey));
            return false;
        }

        // UV0 is optional
        if (drawInfo.uvOffset != UINT_MAX) {
            if (drawInfo.uvFormat != QRhiVertexInputAttribute::Float2) {
                sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("UV0 attribute format is not as expected (float2) for model %1").
                                                                     arg(lm.model->lightmapKey));
                return false;
            }
        }
        // tangent and binormal are optional too
        if (drawInfo.tangentOffset != UINT_MAX) {
            if (drawInfo.tangentFormat != QRhiVertexInputAttribute::Float3) {
                sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Tangent attribute format is not as expected (float3) for model %1").
                                                                     arg(lm.model->lightmapKey));
                return false;
            }
        }
        if (drawInfo.binormalOffset != UINT_MAX) {
            if (drawInfo.binormalFormat != QRhiVertexInputAttribute::Float3) {
                sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Binormal attribute format is not as expected (float3) for model %1").
                                                                     arg(lm.model->lightmapKey));
                return false;
            }
        }

        if (drawInfo.indexFormat == QRhiCommandBuffer::IndexUInt16) {
            drawInfo.indexFormat = QRhiCommandBuffer::IndexUInt32;
            QByteArray newIndexData(drawInfo.indexData.size() * 2, Qt::Uninitialized);
            const quint16 *s = reinterpret_cast<const quint16 *>(drawInfo.indexData.constData());
            size_t sz = drawInfo.indexData.size() / 2;
            quint32 *p = reinterpret_cast<quint32 *>(newIndexData.data());
            while (sz--)
                *p++ = *s++;
            drawInfo.indexData = newIndexData;
        }

        // Bake in the world transform.
        {
            char *vertexBase = drawInfo.vertexData.data();
            const qsizetype sz = drawInfo.vertexData.size();
            for (qsizetype offset = 0; offset < sz; offset += drawInfo.vertexStride) {
                char *posPtr = vertexBase + offset + drawInfo.positionOffset;
                float *fPosPtr = reinterpret_cast<float *>(posPtr);
                QVector3D pos(fPosPtr[0], fPosPtr[1], fPosPtr[2]);
                char *normalPtr = vertexBase + offset + drawInfo.normalOffset;
                float *fNormalPtr = reinterpret_cast<float *>(normalPtr);
                QVector3D normal(fNormalPtr[0], fNormalPtr[1], fNormalPtr[2]);
                pos = worldTransform.map(pos);
                normal = QSSGUtils::mat33::transform(normalMatrix, normal).normalized();
                *fPosPtr++ = pos.x();
                *fPosPtr++ = pos.y();
                *fPosPtr++ = pos.z();
                *fNormalPtr++ = normal.x();
                *fNormalPtr++ = normal.y();
                *fNormalPtr++ = normal.z();
            }
        }
    } // end loop over models used in the lightmap

    rdev = rtcNewDevice(nullptr);
    if (!rdev) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create Embree device"));
        return false;
    }

    rtcSetDeviceErrorFunction(rdev, embreeErrFunc, nullptr);

    rscene = rtcNewScene(rdev);

    unsigned int geomId = 1;

    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);

        // While Light.castsShadow and Model.receivesShadows are irrelevant for
        // baked lighting (they are effectively ignored, shadows are always
        // there with baked direct lighting), Model.castsShadows is something
        // we can and should take into account.
        if (!lm.model->castsShadows)
            continue;

        const DrawInfo &drawInfo(drawInfos[lmIdx]);
        const char *vbase = drawInfo.vertexData.constData();
        const quint32 *ibase = reinterpret_cast<const quint32 *>(drawInfo.indexData.constData());

        for (SubMeshInfo &subMeshInfo : subMeshInfos[lmIdx]) {
            RTCGeometry geom = rtcNewGeometry(rdev, RTC_GEOMETRY_TYPE_TRIANGLE);
            rtcSetGeometryVertexAttributeCount(geom, 2);
            quint32 *ip = static_cast<quint32 *>(rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), subMeshInfo.count / 3));
            for (quint32 i = 0; i < subMeshInfo.count; ++i)
                *ip++ = i;
            float *vp = static_cast<float *>(rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), subMeshInfo.count));
            for (quint32 i = 0; i < subMeshInfo.count; ++i) {
                const quint32 idx = *(ibase + subMeshInfo.offset + i);
                const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.positionOffset);
                *vp++ = *src++;
                *vp++ = *src++;
                *vp++ = *src++;
            }
            vp = static_cast<float *>(rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, NORMAL_SLOT, RTC_FORMAT_FLOAT3, 3 * sizeof(float), subMeshInfo.count));
            for (quint32 i = 0; i < subMeshInfo.count; ++i) {
                const quint32 idx = *(ibase + subMeshInfo.offset + i);
                const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.normalOffset);
                *vp++ = *src++;
                *vp++ = *src++;
                *vp++ = *src++;
            }
            vp = static_cast<float *>(rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, LIGHTMAP_UV_SLOT, RTC_FORMAT_FLOAT2, 2 * sizeof(float), subMeshInfo.count));
            for (quint32 i = 0; i < subMeshInfo.count; ++i) {
                const quint32 idx = *(ibase + subMeshInfo.offset + i);
                const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.lightmapUVOffset);
                *vp++ = *src++;
                *vp++ = *src++;
            }
            rtcCommitGeometry(geom);
            rtcSetGeometryIntersectFilterFunction(geom, embreeFilterFunc);
            rtcSetGeometryUserData(geom, this);
            rtcAttachGeometryByID(rscene, geom, geomId);
            subMeshInfo.geomId = geomId++;
            rtcReleaseGeometry(geom);
        }
    }

    rtcCommitScene(rscene);

    RTCBounds bounds;
    rtcGetSceneBounds(rscene, &bounds);
    QVector3D lowerBound(bounds.lower_x, bounds.lower_y, bounds.lower_z);
    QVector3D upperBound(bounds.upper_x, bounds.upper_y, bounds.upper_z);
    qDebug() << "[lm] Bounds in world space for raytracing scene:" << lowerBound << upperBound;

    const unsigned int geomIdBasedMapSize = geomId;
    // Need fast lookup, hence indexing by geomId here. geomId starts from 1,
    // meaning index 0 will be unused, but that's ok.
    geomLightmapMap.fill(-1, geomIdBasedMapSize);
    subMeshOpacityMap.fill(0.0f, geomIdBasedMapSize);

    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
        if (!lm.model->castsShadows) // only matters if it's in the raytracer scene
            continue;
        for (SubMeshInfo &subMeshInfo : subMeshInfos[lmIdx])
            subMeshOpacityMap[subMeshInfo.geomId] = subMeshInfo.opacity;
    }

    sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Geometry ready. Time taken: %1").arg(formatDuration(geomPrepTimer.elapsed())));
    return true;
}

QSSGLightmapperPrivate::RasterResult QSSGLightmapperPrivate::rasterizeLightmap(int lmIdx, QSize outputSize, QVector2D minUVRegion, QVector2D maxUVRegion)
{
    QSSGLightmapperPrivate::RasterResult result;

    QSSGRhiContext *rhiCtx = rhiCtxInterface->rhiContext().get();
    QRhi *rhi = rhiCtx->rhi();
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    const DrawInfo &bakeModelDrawInfo(drawInfos[lmIdx]);
    const bool hasUV0 = bakeModelDrawInfo.uvOffset != UINT_MAX;
    const bool hasTangentAndBinormal = bakeModelDrawInfo.tangentOffset != UINT_MAX
            && bakeModelDrawInfo.binormalOffset != UINT_MAX;

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(bakeModelDrawInfo.vertexStride) });

    std::unique_ptr<QRhiBuffer> vbuf(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, bakeModelDrawInfo.vertexData.size()));
    if (!vbuf->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create vertex buffer"));
        return result;
    }
    std::unique_ptr<QRhiBuffer> ibuf(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, bakeModelDrawInfo.indexData.size()));
    if (!ibuf->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create index buffer"));
        return result;
    }
    QRhiResourceUpdateBatch *resUpd = rhi->nextResourceUpdateBatch();
    resUpd->uploadStaticBuffer(vbuf.get(), bakeModelDrawInfo.vertexData.constData());
    resUpd->uploadStaticBuffer(ibuf.get(), bakeModelDrawInfo.indexData.constData());
    QRhiTexture *dummyTexture = rhiCtx->dummyTexture({}, resUpd);
    cb->resourceUpdate(resUpd);

    std::unique_ptr<QRhiTexture> positionData(rhi->newTexture(QRhiTexture::RGBA32F, outputSize, 1,
                                                              QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!positionData->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create FP32 texture for positions"));
        return result;
    }
    std::unique_ptr<QRhiTexture> normalData(rhi->newTexture(QRhiTexture::RGBA32F, outputSize, 1,
                                                            QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!normalData->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create FP32 texture for normals"));
        return result;
    }
    std::unique_ptr<QRhiTexture> baseColorData(rhi->newTexture(QRhiTexture::RGBA32F, outputSize, 1,
                                                               QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!baseColorData->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create FP32 texture for base color"));
        return result;
    }
    std::unique_ptr<QRhiTexture> emissionData(rhi->newTexture(QRhiTexture::RGBA32F, outputSize, 1,
                                                              QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!emissionData->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create FP32 texture for emissive color"));
        return result;
    }

    std::unique_ptr<QRhiRenderBuffer> ds(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, outputSize));
    if (!ds->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create depth-stencil buffer"));
        return result;
    }

    QRhiColorAttachment posAtt(positionData.get());
    QRhiColorAttachment normalAtt(normalData.get());
    QRhiColorAttachment baseColorAtt(baseColorData.get());
    QRhiColorAttachment emissionAtt(emissionData.get());
    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ posAtt, normalAtt, baseColorAtt, emissionAtt });
    rtDesc.setDepthStencilBuffer(ds.get());

    std::unique_ptr<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    std::unique_ptr<QRhiRenderPassDescriptor> rpDesc(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rpDesc.get());
    if (!rt->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create texture render target"));
        return result;
    }

    static const int UBUF_SIZE = 64;
    const int subMeshCount = subMeshInfos[lmIdx].size();
    const int alignedUbufSize = rhi->ubufAligned(UBUF_SIZE);
    const int totalUbufSize = alignedUbufSize * subMeshCount;
    std::unique_ptr<QRhiBuffer> ubuf(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, totalUbufSize));
    if (!ubuf->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create uniform buffer of size %1").arg(totalUbufSize));
        return result;
    }

    // Must ensure that the final image is identical with all graphics APIs,
    // regardless of how the Y axis goes in the image and normalized device
    // coordinate systems.
    qint32 flipY = rhi->isYUpInFramebuffer() ? 0 : 1;
    if (rhi->isYUpInNDC())
        flipY = 1 - flipY;

    char *ubufData = ubuf->beginFullDynamicBufferUpdateForCurrentFrame();
    for (int subMeshIdx = 0; subMeshIdx != subMeshCount; ++subMeshIdx) {
        const SubMeshInfo &subMeshInfo(subMeshInfos[lmIdx][subMeshIdx]);
        qint32 hasBaseColorMap = subMeshInfo.baseColorMap ? 1 : 0;
        qint32 hasEmissiveMap = subMeshInfo.emissiveMap ? 1 : 0;
        qint32 hasNormalMap = subMeshInfo.normalMap && hasTangentAndBinormal ? 1 : 0;
        const float minRegionU = minUVRegion.x();
        const float minRegionV = minUVRegion.y();
        const float maxRegionU = maxUVRegion.x();
        const float maxRegionV = maxUVRegion.y();
        char *p = ubufData + subMeshIdx * alignedUbufSize;
        memcpy(p, &subMeshInfo.baseColor, 4 * sizeof(float));
        memcpy(p + 16, &subMeshInfo.emissiveFactor, 3 * sizeof(float));
        memcpy(p + 28, &flipY, sizeof(qint32));
        memcpy(p + 32, &hasBaseColorMap, sizeof(qint32));
        memcpy(p + 36, &hasEmissiveMap, sizeof(qint32));
        memcpy(p + 40, &hasNormalMap, sizeof(qint32));
        memcpy(p + 44, &subMeshInfo.normalStrength, sizeof(float));
        memcpy(p + 48, &minRegionU, sizeof(float));
        memcpy(p + 52, &minRegionV, sizeof(float));
        memcpy(p + 56, &maxRegionU, sizeof(float));
        memcpy(p + 60, &maxRegionV, sizeof(float));
    }
    ubuf->endFullDynamicBufferUpdateForCurrentFrame();

    auto setupPipeline = [rhi, &rpDesc](QSSGRhiShaderPipeline *shaderPipeline,
                                        QRhiShaderResourceBindings *srb,
                                        const QRhiVertexInputLayout &inputLayout)
    {
        QRhiGraphicsPipeline *ps = rhi->newGraphicsPipeline();
        ps->setTopology(QRhiGraphicsPipeline::Triangles);
        ps->setDepthTest(true);
        ps->setDepthWrite(true);
        ps->setDepthOp(QRhiGraphicsPipeline::Less);
        ps->setShaderStages(shaderPipeline->cbeginStages(), shaderPipeline->cendStages());
        ps->setTargetBlends({ {}, {}, {}, {} });
        ps->setRenderPassDescriptor(rpDesc.get());
        ps->setVertexInputLayout(inputLayout);
        ps->setShaderResourceBindings(srb);
        return ps;
    };

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QVector<QRhiGraphicsPipeline *> ps;
    // Everything is going to be rendered twice (but note depth testing), first
    // with polygon mode fill, then line.
    QVector<QRhiGraphicsPipeline *> psLine;

    for (int subMeshIdx = 0; subMeshIdx != subMeshCount; ++subMeshIdx) {
        const SubMeshInfo &subMeshInfo(subMeshInfos[lmIdx][subMeshIdx]);
        QVarLengthArray<QRhiVertexInputAttribute, 6> vertexAttrs;
        vertexAttrs << QRhiVertexInputAttribute(0, 0, bakeModelDrawInfo.positionFormat, bakeModelDrawInfo.positionOffset)
                    << QRhiVertexInputAttribute(0, 1, bakeModelDrawInfo.normalFormat, bakeModelDrawInfo.normalOffset)
                    << QRhiVertexInputAttribute(0, 2, bakeModelDrawInfo.lightmapUVFormat, bakeModelDrawInfo.lightmapUVOffset);

        // Vertex inputs (just like the sampler uniforms) must match exactly on
        // the shader and the application side, cannot just leave out or have
        // unused inputs.
        QSSGBuiltInRhiShaderCache::LightmapUVRasterizationShaderMode shaderVariant = QSSGBuiltInRhiShaderCache::LightmapUVRasterizationShaderMode::Default;
        if (hasUV0) {
            shaderVariant = QSSGBuiltInRhiShaderCache::LightmapUVRasterizationShaderMode::Uv;
            if (hasTangentAndBinormal)
                shaderVariant = QSSGBuiltInRhiShaderCache::LightmapUVRasterizationShaderMode::UvTangent;
        }

        const auto &shaderCache = renderer->contextInterface()->shaderCache();
        const auto &lmUvRastShaderPipeline = shaderCache->getBuiltInRhiShaders().getRhiLightmapUVRasterizationShader(shaderVariant);
        if (!lmUvRastShaderPipeline) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to load shaders"));
            return result;
        }

        if (hasUV0) {
            vertexAttrs << QRhiVertexInputAttribute(0, 3, bakeModelDrawInfo.uvFormat, bakeModelDrawInfo.uvOffset);
            if (hasTangentAndBinormal) {
                vertexAttrs << QRhiVertexInputAttribute(0, 4, bakeModelDrawInfo.tangentFormat, bakeModelDrawInfo.tangentOffset);
                vertexAttrs << QRhiVertexInputAttribute(0, 5, bakeModelDrawInfo.binormalFormat, bakeModelDrawInfo.binormalOffset);
            }
        }

        inputLayout.setAttributes(vertexAttrs.cbegin(), vertexAttrs.cend());

        QSSGRhiShaderResourceBindingList bindings;
        bindings.addUniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, ubuf.get(),
                                  subMeshIdx * alignedUbufSize, UBUF_SIZE);
        QRhiSampler *dummySampler = rhiCtx->sampler({ QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                      QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
        if (subMeshInfo.baseColorMap) {
            const bool mipmapped = subMeshInfo.baseColorMap->flags().testFlag(QRhiTexture::MipMapped);
            QRhiSampler *sampler = rhiCtx->sampler({ QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_minFilterType),
                    QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_magFilterType),
                    mipmapped ? QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_mipFilterType) : QRhiSampler::None,
                    QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_horizontalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_verticalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.baseColorNode->m_depthTilingMode)
            });
            bindings.addTexture(1, QRhiShaderResourceBinding::FragmentStage, subMeshInfo.baseColorMap, sampler);
        } else {
            bindings.addTexture(1, QRhiShaderResourceBinding::FragmentStage, dummyTexture, dummySampler);
        }
        if (subMeshInfo.emissiveMap) {
            const bool mipmapped = subMeshInfo.emissiveMap->flags().testFlag(QRhiTexture::MipMapped);
            QRhiSampler *sampler = rhiCtx->sampler({ QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_minFilterType),
                    QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_magFilterType),
                    mipmapped ? QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_mipFilterType) : QRhiSampler::None,
                    QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_horizontalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_verticalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.emissiveNode->m_depthTilingMode)
            });
            bindings.addTexture(2, QRhiShaderResourceBinding::FragmentStage, subMeshInfo.emissiveMap, sampler);
        } else {
            bindings.addTexture(2, QRhiShaderResourceBinding::FragmentStage, dummyTexture, dummySampler);
        }
        if (subMeshInfo.normalMap) {
            const bool mipmapped = subMeshInfo.normalMap->flags().testFlag(QRhiTexture::MipMapped);
            QRhiSampler *sampler = rhiCtx->sampler({ QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_minFilterType),
                    QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_magFilterType),
                    mipmapped ? QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_mipFilterType) : QRhiSampler::None,
                    QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_horizontalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_verticalTilingMode),
                    QSSGRhiHelpers::toRhi(subMeshInfo.normalMapNode->m_depthTilingMode)
            });
            bindings.addTexture(3, QRhiShaderResourceBinding::FragmentStage, subMeshInfo.normalMap, sampler);
        } else {
            bindings.addTexture(3, QRhiShaderResourceBinding::FragmentStage, dummyTexture, dummySampler);
        }
        QRhiShaderResourceBindings *srb = rhiCtxD->srb(bindings);

        QRhiGraphicsPipeline *pipeline = setupPipeline(lmUvRastShaderPipeline.get(), srb, inputLayout);
        if (!pipeline->create()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create graphics pipeline (mesh %1 submesh %2)").
                                                                   arg(lmIdx).
                                                                   arg(subMeshIdx));
            qDeleteAll(ps);
            qDeleteAll(psLine);
            return result;
        }
        ps.append(pipeline);
        pipeline = setupPipeline(lmUvRastShaderPipeline.get(), srb, inputLayout);
        pipeline->setPolygonMode(QRhiGraphicsPipeline::Line);
        if (!pipeline->create()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create graphics pipeline with line fill mode (mesh %1 submesh %2)").
                                                                   arg(lmIdx).
                                                                   arg(subMeshIdx));
            qDeleteAll(ps);
            qDeleteAll(psLine);
            return result;
        }
        psLine.append(pipeline);
    }

    QRhiCommandBuffer::VertexInput vertexBuffers = { vbuf.get(), 0 };
    const QRhiViewport viewport(0, 0, float(outputSize.width()), float(outputSize.height()));
    bool hadViewport = false;

    cb->beginPass(rt.get(), Qt::black, { 1.0f, 0 });
    for (int subMeshIdx = 0; subMeshIdx != subMeshCount; ++subMeshIdx) {
        const SubMeshInfo &subMeshInfo(subMeshInfos[lmIdx][subMeshIdx]);
        cb->setGraphicsPipeline(ps[subMeshIdx]);
        if (!hadViewport) {
            cb->setViewport(viewport);
            hadViewport = true;
        }
        cb->setShaderResources();
        cb->setVertexInput(0, 1, &vertexBuffers, ibuf.get(), 0, QRhiCommandBuffer::IndexUInt32);
        cb->drawIndexed(subMeshInfo.count, 1, subMeshInfo.offset);
        cb->setGraphicsPipeline(psLine[subMeshIdx]);
        cb->setShaderResources();
        cb->drawIndexed(subMeshInfo.count, 1, subMeshInfo.offset);
    }

    resUpd = rhi->nextResourceUpdateBatch();
    QRhiReadbackResult posReadResult;
    QRhiReadbackResult normalReadResult;
    QRhiReadbackResult baseColorReadResult;
    QRhiReadbackResult emissionReadResult;
    resUpd->readBackTexture({ positionData.get() }, &posReadResult);
    resUpd->readBackTexture({ normalData.get() }, &normalReadResult);
    resUpd->readBackTexture({ baseColorData.get() }, &baseColorReadResult);
    resUpd->readBackTexture({ emissionData.get() }, &emissionReadResult);
    cb->endPass(resUpd);

    // Submit and wait for completion.
    rhi->finish();

    qDeleteAll(ps);
    qDeleteAll(psLine);

    const int numPixels = outputSize.width() * outputSize.height();

    result.worldPositions.resize(numPixels);
    result.normals.resize(numPixels);
    result.baseColors.resize(numPixels);
    result.emissions.resize(numPixels);

    // The readback results are tightly packed (which is supposed to be ensured
    // by each rhi backend), so one line is 16 * width bytes.
    if (posReadResult.data.size() < numPixels * 16) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Position data is smaller than expected"));
        return result;
    }
    if (normalReadResult.data.size() < numPixels * 16) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Normal data is smaller than expected"));
        return result;
    }
    if (baseColorReadResult.data.size() < numPixels * 16) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Base color data is smaller than expected"));
        return result;
    }
    if (emissionReadResult.data.size() < numPixels * 16) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Emission data is smaller than expected"));
        return result;
    }

    result.success = true;
    result.width = outputSize.width();
    result.height = outputSize.height();
    result.worldPositions = posReadResult.data;
    result.normals = normalReadResult.data;
    result.baseColors = baseColorReadResult.data;
    result.emissions = emissionReadResult.data;

    return result;
}

bool QSSGLightmapperPrivate::prepareLightmaps()
{
    QRhi *rhi = rhiCtxInterface->rhiContext()->rhi();
    Q_ASSERT(rhi);
    if (!rhi->isTextureFormatSupported(QRhiTexture::RGBA32F)) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("FP32 textures not supported, cannot bake"));
        return false;
    }
    if (rhi->resourceLimit(QRhi::MaxColorAttachments) < 4) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Multiple render targets not supported, cannot bake"));
        return false;
    }
    if (!rhi->isFeatureSupported(QRhi::NonFillPolygonMode)) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Line polygon mode not supported, cannot bake"));
        return false;
    }

    sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Preparing lightmaps..."));
    QElapsedTimer lightmapPrepTimer;
    lightmapPrepTimer.start();
    const int bakedLightingModelCount = bakedLightingModels.size();
    Q_ASSERT(drawInfos.size() == bakedLightingModelCount);
    Q_ASSERT(subMeshInfos.size() == bakedLightingModelCount);

    numValidTexels.resize(bakedLightingModelCount);

    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        QElapsedTimer rasterizeTimer;
        rasterizeTimer.start();

        const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
        const QSize lightmapSize = drawInfos[lmIdx].lightmapSize;

        const int w = lightmapSize.width();
        const int h = lightmapSize.height();
        const int numPixels = w * h;

        int unusedEntries = 0;
        QVector<ModelTexel> &texels = modelTexels[lmIdx];
        texels.resize(numPixels);

        // Dynamically compute number of tiles so that each tile is <= MAX_TILE_SIZE
        constexpr int maxTileSize = MAX_TILE_SIZE;
        const int numTilesX = (w + maxTileSize - 1) / maxTileSize;
        const int numTilesY = (h + maxTileSize - 1) / maxTileSize;

        bool isEmissive = false;

        // Render tiled to make sure enough GPU memory is available
        for (int tileY = 0; tileY < numTilesY; ++tileY) {
            for (int tileX = 0; tileX < numTilesX; ++tileX) {
                // Compute actual tile size (may be less than maxTileSize on edges)
                const int startX = tileX * maxTileSize;
                const int startY = tileY * maxTileSize;

                const int tileWidth = qMin(maxTileSize, w - startX);
                const int tileHeight = qMin(maxTileSize, h - startY);

                const int endX = startX + tileWidth;
                const int endY = startY + tileHeight;

                const float minU = startX / double(w);
                const float maxV = 1.0 - startY / double(h);
                const float maxU = endX / double(w);
                const float minV = 1.0 - endY / double(h);

                QSSGLightmapperPrivate::RasterResult raster = rasterizeLightmap(lmIdx,
                                                                                QSize(tileWidth, tileHeight),
                                                                                QVector2D(minU, minV),
                                                                                QVector2D(maxU, maxV));
                if (!raster.success)
                    return false;

                QVector4D *worldPositions = reinterpret_cast<QVector4D *>(raster.worldPositions.data());
                QVector4D *normals = reinterpret_cast<QVector4D *>(raster.normals.data());
                QVector4D *baseColors = reinterpret_cast<QVector4D *>(raster.baseColors.data());
                QVector4D *emissions = reinterpret_cast<QVector4D *>(raster.emissions.data());

                for (int y = startY; y < endY; ++y) {
                    const int ySrc = y - startY;
                    Q_ASSERT(ySrc < tileHeight);
                    for (int x = startX; x < endX; ++x) {
                        const int xSrc = x - startX;
                        Q_ASSERT(xSrc < tileWidth);

                        const int dstPixelI = y * w + x;
                        const int srcPixelI = ySrc * tileWidth + xSrc;

                        ModelTexel &lmPix(texels[dstPixelI]);

                        lmPix.worldPos = worldPositions[srcPixelI].toVector3D();
                        lmPix.normal = normals[srcPixelI].toVector3D();
                        if (lmPix.isValid())
                            ++numValidTexels[lmIdx];

                        lmPix.baseColor = baseColors[srcPixelI];
                        if (lmPix.baseColor[3] < 1.0f)
                            modelHasBaseColorTransparency[lmIdx] = true;

                        lmPix.emission = emissions[srcPixelI].toVector3D();
                        if (!isEmissive && !qFuzzyIsNull(lmPix.emission.length()))
                            isEmissive = true;
                    }
                }
            }
        }

        if (isEmissive)
            ++emissiveModelCount;

        sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                       QStringLiteral(
                               "Successfully rasterized %1/%2 lightmap texels for model %3, lightmap size %4 in %5")
                               .arg(texels.size() - unusedEntries)
                               .arg(texels.size())
                               .arg(lm.model->lightmapKey)
                               .arg(QStringLiteral("(%1, %2)").arg(w).arg(h))
                               .arg(formatDuration(rasterizeTimer.elapsed())));
        for (const SubMeshInfo &subMeshInfo : std::as_const(subMeshInfos[lmIdx])) {
            if (!lm.model->castsShadows) // only matters if it's in the raytracer scene
                continue;
            geomLightmapMap[subMeshInfo.geomId] = lmIdx;
        }
    }

    sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                   QStringLiteral("Lightmaps ready. Time taken: %1")
                       .arg(formatDuration(lightmapPrepTimer.elapsed())));
    return true;
}

bool QSSGLightmapperPrivate::verifyLights() const {

    return !lights.empty() || emissiveModelCount > 0;
}

bool QSSGLightmapper::setupLights(const QSSGRenderer &renderer)
{
    QSSGLayerRenderData *renderData = QSSGRendererPrivate::getCurrentRenderData(renderer);
    if (!renderData) {
        qWarning() << "lm: No render data, cannot bake lightmaps";
        return false;
    }

    if (d->bakedLightingModels.isEmpty()) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                          QStringLiteral("No models provided, cannot bake lightmaps"));
        return false;
    }

    // All subsets for a model reference the same QSSGShaderLight list,
    // take the first one, but filter it based on the bake flag.
    // also tracks seenLights, as multiple models might reference the same lights.
    auto lights = static_cast<QSSGSubsetRenderable *>(d->bakedLightingModels.first().renderables.first().obj)->lights;
    for (const QSSGShaderLight &sl : lights) {
        if (!sl.light->m_bakingEnabled)
            continue;

        QSSGLightmapperPrivate::Light light;
        light.indirectOnly = !sl.light->m_fullyBaked;
        light.direction = sl.direction;

        const float brightness = sl.light->m_brightness;
        light.color = QVector3D(sl.light->m_diffuseColor.x() * brightness,
                                sl.light->m_diffuseColor.y() * brightness,
                                sl.light->m_diffuseColor.z() * brightness);

        if (sl.light->type == QSSGRenderLight::Type::PointLight
            || sl.light->type == QSSGRenderLight::Type::SpotLight) {
            const QMatrix4x4 lightGlobalTransform = renderData->getGlobalTransform(*sl.light);
            light.worldPos = QSSGRenderNode::getGlobalPos(lightGlobalTransform);
            if (sl.light->type == QSSGRenderLight::Type::SpotLight) {
                light.type = QSSGLightmapperPrivate::Light::Spot;
                light.cosConeAngle = qCos(qDegreesToRadians(sl.light->m_coneAngle));
                light.cosInnerConeAngle = qCos(
                        qDegreesToRadians(qMin(sl.light->m_innerConeAngle, sl.light->m_coneAngle)));
            } else {
                light.type = QSSGLightmapperPrivate::Light::Point;
            }
            light.constantAttenuation = QSSGUtils::aux::translateConstantAttenuation(
                    sl.light->m_constantFade);
            light.linearAttenuation = QSSGUtils::aux::translateLinearAttenuation(
                    sl.light->m_linearFade);
            light.quadraticAttenuation = QSSGUtils::aux::translateQuadraticAttenuation(
                    sl.light->m_quadraticFade);
        } else {
            light.type = QSSGLightmapperPrivate::Light::Directional;
        }

        d->lights.append(light);
    }

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                      QStringLiteral("Total lights registered: %1").arg(d->lights.size()));

    return true;
}


struct RayHit
{
    RayHit(const QVector3D &org, const QVector3D &dir, float tnear = 0.0f, float tfar = std::numeric_limits<float>::infinity()) {
        rayhit.ray.org_x = org.x();
        rayhit.ray.org_y = org.y();
        rayhit.ray.org_z = org.z();
        rayhit.ray.dir_x = dir.x();
        rayhit.ray.dir_y = dir.y();
        rayhit.ray.dir_z = dir.z();
        rayhit.ray.tnear = tnear;
        rayhit.ray.tfar = tfar;
        rayhit.hit.u = 0.0f;
        rayhit.hit.v = 0.0f;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    }

    RTCRayHit rayhit;

    bool intersect(RTCScene scene)
    {
        RTCIntersectContext ctx;
        rtcInitIntersectContext(&ctx);
        rtcIntersect1(scene, &ctx, &rayhit);
        return rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
    }
};

static inline QVector3D vectorSign(const QVector3D &v)
{
    return QVector3D(v.x() < 1.0f ? -1.0f : 1.0f,
                     v.y() < 1.0f ? -1.0f : 1.0f,
                     v.z() < 1.0f ? -1.0f : 1.0f);
}

static inline QVector3D vectorAbs(const QVector3D &v)
{
    return QVector3D(std::abs(v.x()),
                     std::abs(v.y()),
                     std::abs(v.z()));
}

// Function to apply a Gaussian blur to an image
QList<QVector3D> applyGaussianBlur(const QList<QVector3D>& image, const QList<quint32>& mask, int width, int height, float sigma) {
    // Create a Gaussian kernel
    constexpr int halfKernelSize = GAUSS_HALF_KERNEL_SIZE;
    constexpr int kernelSize = halfKernelSize * 2 + 1;

    double sum = 0.0;
    double kernel[kernelSize][kernelSize];
    double mean = halfKernelSize;
    for (int y = 0; y < kernelSize; ++y) {
        for (int x = 0; x < kernelSize; ++x) {
            kernel[y][x] = exp(-0.5 * (pow((x - mean) / sigma, 2.0) + pow((y - mean) / sigma, 2.0))) / (2 * M_PI * sigma * sigma);

            // Accumulate the kernel values
            sum += kernel[y][x];
        }
    }

    // Normalize the kernel
    for (int x = 0; x < kernelSize; ++x)
        for (int y = 0; y < kernelSize; ++y)
            kernel[y][x] /= sum;

    // Create a copy of the image for the output
    QList<QVector3D> output(image.size(), QVector3D(0, 0, 0));

    // Apply the kernel to each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int centerIdx = y * width + x;
            const quint32 maskID = mask[centerIdx];
            if (maskID == PIXEL_VOID)
                continue;

            QVector3D blurredPixel(0, 0, 0);
            float weightSum = 0.0f;

            // Convolve the kernel with the image
            for (int ky = -halfKernelSize; ky <= halfKernelSize; ++ky) {
                for (int kx = -halfKernelSize; kx <= halfKernelSize; ++kx) {
                    int px = x + kx;
                    int py = y + ky;
                    if (px < 0 || px >= width || py < 0 || py >= height)
                        continue;

                    int idx = py * width + px;
                    if (mask[idx] != maskID)
                        continue;

                    double weight = kernel[ky + halfKernelSize][kx + halfKernelSize];
                    blurredPixel += image[idx] * weight;
                    weightSum += weight;
                }
            }

            // Normalize if needed to avoid darkening near edges
            if (weightSum > 0.0f)
                blurredPixel /= weightSum;

            output[centerIdx] = blurredPixel;
        }
    }

    return output;
}

struct Edge
{
    std::array<QVector3D, 2> pos;
    std::array<QVector3D, 2> normal;
};

inline bool operator==(const Edge &a, const Edge &b)
{
    return qFuzzyCompare(a.pos[0], b.pos[0]) && qFuzzyCompare(a.pos[1], b.pos[1])
            && qFuzzyCompare(a.normal[0], b.normal[0]) && qFuzzyCompare(a.normal[1], b.normal[1]);
}

inline size_t qHash(const Edge &e, size_t seed) Q_DECL_NOTHROW
{
    return qHash(e.pos[0].x(), seed) ^ qHash(e.pos[0].y()) ^ qHash(e.pos[0].z()) ^ qHash(e.pos[1].x())
            ^ qHash(e.pos[1].y()) ^ qHash(e.pos[1].z());
}

struct EdgeUV
{
    std::array<QVector2D, 2> uv;
    bool seam = false;
};

struct SeamUV
{
    std::array<std::array<QVector2D, 2>, 2> uv;
};

static inline bool vectorLessThan(const QVector3D &a, const QVector3D &b)
{
    if (a.x() == b.x()) {
        if (a.y() == b.y())
            return a.z() < b.z();
        else
            return a.y() < b.y();
    }
    return a.x() < b.x();
}

static inline float floatSign(float f)
{
    return f > 0.0f ? 1.0f : (f < 0.0f ? -1.0f : 0.0f);
}

static inline QVector2D flooredVec(const QVector2D &v)
{
    return QVector2D(std::floor(v.x()), std::floor(v.y()));
}

static inline QVector2D projectPointToLine(const QVector2D &point, const std::array<QVector2D, 2> &line)
{
    const QVector2D p = point - line[0];
    const QVector2D n = line[1] - line[0];
    const float lengthSquared = n.lengthSquared();
    if (!qFuzzyIsNull(lengthSquared)) {
        const float d = (n.x() * p.x() + n.y() * p.y()) / lengthSquared;
        return d <= 0.0f ? line[0] : (d >= 1.0f ? line[1] : line[0] + n * d);
    }
    return line[0];
}

static void blendLine(const QVector2D &from,
                      const QVector2D &to,
                      const QVector2D &uvFrom,
                      const QVector2D &uvTo,
                      const float *readBuf,
                      float *writeBuf,
                      const QSize &lightmapPixelSize,
                      const int stride = 4)
{
    const QVector2D size(lightmapPixelSize.width(), lightmapPixelSize.height());
    const std::array<QVector2D, 2> line = { QVector2D(from.x(), 1.0f - from.y()) * size, QVector2D(to.x(), 1.0f - to.y()) * size };
    const float lineLength = line[0].distanceToPoint(line[1]);
    if (qFuzzyIsNull(lineLength))
        return;

    const QVector2D startPixel = flooredVec(line[0]);
    const QVector2D endPixel = flooredVec(line[1]);

    const QVector2D dir = (line[1] - line[0]).normalized();
    const QVector2D tStep(1.0f / std::abs(dir.x()), 1.0f / std::abs(dir.y()));
    const QVector2D pixelStep(floatSign(dir.x()), floatSign(dir.y()));

    QVector2D nextT(std::fmod(line[0].x(), 1.0f), std::fmod(line[0].y(), 1.0f));
    if (pixelStep.x() == 1.0f)
        nextT.setX(1.0f - nextT.x());
    if (pixelStep.y() == 1.0f)
        nextT.setY(1.0f - nextT.y());

    if (!qFuzzyIsNull(dir.x()))
        nextT.setX(nextT.x() / std::abs(dir.x()));
    else
        nextT.setX(std::numeric_limits<float>::max());

    if (!qFuzzyIsNull(dir.y()))
        nextT.setY(nextT.y() / std::abs(dir.y()));
    else
        nextT.setY(std::numeric_limits<float>::max());

    QVector2D pixel = startPixel;

    const auto clampedXY = [s = lightmapPixelSize](QVector2D xy) -> std::array<int, 2> {
        return { qBound(0, int(xy.x()), s.width() - 1), qBound(0, int(xy.y()), s.height() - 1) };
    };

    while (startPixel.distanceToPoint(pixel) < lineLength + 1.0f) {
        const QVector2D point = projectPointToLine(pixel + QVector2D(0.5f, 0.5f), line);
        const float t = line[0].distanceToPoint(point) / lineLength;
        const QVector2D uvInterp = uvFrom * (1.0 - t) + uvTo * t;
        const auto sampledPixelXY = clampedXY(flooredVec(QVector2D(uvInterp.x(), 1.0f - uvInterp.y()) * size));
        const int sampOfs = (sampledPixelXY[0] + sampledPixelXY[1] * lightmapPixelSize.width()) * stride;
        const QVector3D sampledColor(readBuf[sampOfs], readBuf[sampOfs + 1], readBuf[sampOfs + 2]);
        const auto pixelXY = clampedXY(pixel);
        const int pixOfs = (pixelXY[0] + pixelXY[1] * lightmapPixelSize.width()) * stride;
        QVector3D currentColor(writeBuf[pixOfs], writeBuf[pixOfs + 1], writeBuf[pixOfs + 2]);
        currentColor = currentColor * 0.6f + sampledColor * 0.4f;
        writeBuf[pixOfs] = currentColor.x();
        writeBuf[pixOfs + 1] = currentColor.y();
        writeBuf[pixOfs + 2] = currentColor.z();

        if (pixel != endPixel) {
            if (nextT.x() < nextT.y()) {
                pixel.setX(pixel.x() + pixelStep.x());
                nextT.setX(nextT.x() + tStep.x());
            } else {
                pixel.setY(pixel.y() + pixelStep.y());
                nextT.setY(nextT.y() + tStep.y());
            }
        } else {
            break;
        }
    }
}

QVector3D QSSGLightmapperPrivate::sampleDirectLight(QVector3D worldPos, QVector3D normal, bool allLight) const
{
    QVector3D directLight = QVector3D(0.f, 0.f, 0.f);

    if (options.useAdaptiveBias)
        worldPos += vectorSign(normal) * vectorAbs(worldPos * 0.0000002f);

    // 'lights' should have all lights that are either BakeModeIndirect or BakeModeAll
    for (const Light &light : lights) {
        if (light.indirectOnly && !allLight)
            continue;

        QVector3D lightWorldPos;
        float dist = std::numeric_limits<float>::infinity();
        float attenuation = 1.0f;
        if (light.type == Light::Directional) {
            lightWorldPos = worldPos - light.direction;
        } else {
            lightWorldPos = light.worldPos;
            dist = (worldPos - lightWorldPos).length();
            attenuation = 1.0f
                    / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist);
            if (light.type == Light::Spot) {
                const float spotAngle = QVector3D::dotProduct((worldPos - lightWorldPos).normalized(), light.direction.normalized());
                if (spotAngle > light.cosConeAngle) {
                    // spotFactor = smoothstep(light.cosConeAngle, light.cosInnerConeAngle, spotAngle);
                    const float edge0 = light.cosConeAngle;
                    const float edge1 = light.cosInnerConeAngle;
                    const float x = spotAngle;
                    const float t = qBound(0.0f, (x - edge0) / (edge1 - edge0), 1.0f);
                    const float spotFactor = t * t * (3.0f - 2.0f * t);
                    attenuation *= spotFactor;
                } else {
                    attenuation = 0.0f;
                }
            }
        }

        const QVector3D L = (lightWorldPos - worldPos).normalized();
        const float energy = qMax(0.0f, QVector3D::dotProduct(normal, L)) * attenuation;
        if (qFuzzyIsNull(energy))
            continue;

        // trace a ray from this point towards the light, and see if something is hit on the way
        RayHit ray(worldPos, L, options.bias, dist);
        const bool lightReachable = !ray.intersect(rscene);
        if (lightReachable) {
            directLight += light.color * energy;
        }
    }

    return directLight;
}

QByteArray QSSGLightmapperPrivate::dilate(const QSize &pixelSize, const QByteArray &image)
{
    QSSGRhiContext *rhiCtx = rhiCtxInterface->rhiContext().get();
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QRhi *rhi = rhiCtx->rhi();
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();

    const QRhiViewport viewport(0, 0, float(pixelSize.width()), float(pixelSize.height()));

    std::unique_ptr<QRhiTexture> lightmapTex(rhi->newTexture(QRhiTexture::RGBA32F, pixelSize));
    if (!lightmapTex->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create FP32 texture for postprocessing"));
        return {};
    }
    std::unique_ptr<QRhiTexture> dilatedLightmapTex(
        rhi->newTexture(QRhiTexture::RGBA32F, pixelSize, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!dilatedLightmapTex->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning,
                       QStringLiteral("Failed to create FP32 dest. texture for postprocessing"));
        return {};
    }
    QRhiTextureRenderTargetDescription rtDescDilate(dilatedLightmapTex.get());
    std::unique_ptr<QRhiTextureRenderTarget> rtDilate(rhi->newTextureRenderTarget(rtDescDilate));
    std::unique_ptr<QRhiRenderPassDescriptor> rpDescDilate(rtDilate->newCompatibleRenderPassDescriptor());
    rtDilate->setRenderPassDescriptor(rpDescDilate.get());
    if (!rtDilate->create()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning,
                       QStringLiteral("Failed to create postprocessing texture render target"));
        return {};
    }
    QRhiResourceUpdateBatch *resUpd = rhi->nextResourceUpdateBatch();
    QRhiTextureSubresourceUploadDescription lightmapTexUpload(image.constData(), image.size());
    resUpd->uploadTexture(lightmapTex.get(), QRhiTextureUploadDescription({ 0, 0, lightmapTexUpload }));
    QSSGRhiShaderResourceBindingList bindings;
    QRhiSampler *nearestSampler = rhiCtx->sampler(
        { QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::Repeat });
    bindings.addTexture(0, QRhiShaderResourceBinding::FragmentStage, lightmapTex.get(), nearestSampler);
    renderer->rhiQuadRenderer()->prepareQuad(rhiCtx, resUpd);
    const auto &shaderCache = renderer->contextInterface()->shaderCache();
    const auto &lmDilatePipeline = shaderCache->getBuiltInRhiShaders().getRhiLightmapDilateShader();
    if (!lmDilatePipeline) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to load shaders"));
        return {};
    }
    QSSGRhiGraphicsPipelineState dilatePs;
    dilatePs.viewport = viewport;
    QSSGRhiGraphicsPipelineStatePrivate::setShaderPipeline(dilatePs, lmDilatePipeline.get());
    renderer->rhiQuadRenderer()->recordRenderQuadPass(rhiCtx, &dilatePs, rhiCtxD->srb(bindings), rtDilate.get(), QSSGRhiQuadRenderer::UvCoords);
    resUpd = rhi->nextResourceUpdateBatch();
    QRhiReadbackResult dilateReadResult;
    resUpd->readBackTexture({ dilatedLightmapTex.get() }, &dilateReadResult);
    cb->resourceUpdate(resUpd);

    // Submit and wait for completion.
    rhi->finish();

    return dilateReadResult.data;
}

QVector<QVector3D> QSSGLightmapperPrivate::computeDirectLight(int lmIdx)
{
    const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);

    // While Light.castsShadow and Model.receivesShadows are irrelevant for
    // baked lighting (they are effectively ignored, shadows are always
    // there with baked direct lighting), Model.castsShadows is something
    // we can and should take into account.
    if (!lm.model->castsShadows)
        return {};

    const DrawInfo &drawInfo(drawInfos[lmIdx]);
    const char *vbase = drawInfo.vertexData.constData();
    const quint32 *ibase = reinterpret_cast<const quint32 *>(drawInfo.indexData.constData());

    const QSize sz = drawInfo.lightmapSize;
    const int w = sz.width();
    const int h = sz.height();
    constexpr int padding = GAUSS_HALF_KERNEL_SIZE;
    const int numPixelsFinal = w * h;

    QVector<QVector3D> grid(numPixelsFinal);
    QVector<quint32> mask(numPixelsFinal, PIXEL_VOID);

    // Setup grid and mask
    const QVector<ModelTexel>& texels = modelTexels[lmIdx];
    for (int pixelI = 0; pixelI < numPixelsFinal; ++pixelI) {
        const auto &entry = texels[pixelI];
        if (!entry.isValid())
            continue;
        mask[pixelI] = PIXEL_UNSET;
        grid[pixelI] =  sampleDirectLight(entry.worldPos, entry.normal, false);
    }

    if (std::all_of(grid.begin(), grid.end(), [](const QVector3D &v) { return v.isNull(); })) {
        return grid; // All black, meaning no lights hit or all are indirectOnly.
    }

    floodFill(reinterpret_cast<quint32 *>(mask.data()), h, w);

    // Dynamically compute number of tiles so that each tile is <= MAX_TILE_SIZE
    constexpr int maxTileSize = MAX_TILE_SIZE / DIRECT_MAP_UPSCALE_FACTOR;
    const int numTilesX = (w + maxTileSize - 1) / maxTileSize;
    const int numTilesY = (h + maxTileSize - 1) / maxTileSize;

    // Render upscaled tiles then blur and downscale to remove jaggies in output
    for (int tileY = 0; tileY < numTilesY; ++tileY) {
        for (int tileX = 0; tileX < numTilesX; ++tileX) {
            // Compute actual tile size (may be less than maxTileSize on edges)
            const int startX = tileX * maxTileSize;
            const int startY = tileY * maxTileSize;

            const int tileWidth = qMin(maxTileSize, w - startX);
            const int tileHeight = qMin(maxTileSize, h - startY);

            const int currentTileWidth = tileWidth + 2 * padding;
            const int currentTileHeight = tileHeight + 2 * padding;

            const int wExp = currentTileWidth * DIRECT_MAP_UPSCALE_FACTOR;
            const int hExp = currentTileHeight * DIRECT_MAP_UPSCALE_FACTOR;
            const int numPixelsExpanded = wExp * hExp;

            QVector<quint32> maskTile(numPixelsExpanded, PIXEL_VOID);
            QVector<QVector3D> gridTile(numPixelsExpanded);

            // Compute full-padded pixel bounds (including kernel padding)
            const int pixelStartX = startX - padding;
            const int pixelStartY = startY - padding;
            const int pixelEndX = startX + tileWidth + padding;
            const int pixelEndY = startY + tileHeight + padding;

            const float minU = pixelStartX / double(w);
            const float maxV = 1.0 - pixelStartY / double(h);
            const float maxU = pixelEndX / double(w);
            const float minV = 1.0 - pixelEndY / double(h);

            // Temporary storage for rasterized, avoids copy
            QByteArray worldPositionsBuffer;
            QByteArray normalsBuffer;
            {
                QSSGLightmapperPrivate::RasterResult raster = rasterizeLightmap(lmIdx,
                                                                                QSize(wExp, hExp),
                                                                                QVector2D(minU, minV),
                                                                                QVector2D(maxU, maxV));
                if (!raster.success)
                    return {};
                Q_ASSERT(raster.width * raster.height == numPixelsExpanded);
                worldPositionsBuffer = raster.worldPositions;
                normalsBuffer = raster.normals;
            }

            QVector4D *worldPositions = reinterpret_cast<QVector4D *>(worldPositionsBuffer.data());
            QVector4D *normals = reinterpret_cast<QVector4D *>(normalsBuffer.data());

            for (int pixelI = 0; pixelI < numPixelsExpanded; ++pixelI) {
                QVector3D position = worldPositions[pixelI].toVector3D();
                QVector3D normal = normals[pixelI].toVector3D();
                if (normal.isNull()) {
                    maskTile[pixelI] = PIXEL_VOID;
                    continue;
                }

                maskTile[pixelI] = PIXEL_UNSET;
                gridTile[pixelI] += sampleDirectLight(position, normal, false);
            }

            floodFill(reinterpret_cast<quint32 *>(maskTile.data()), hExp, wExp); // Flood fill mask in place
            gridTile = applyGaussianBlur(gridTile, maskTile, wExp, hExp, 3.f);

            const int endX = qMin(w, startX + tileWidth);
            const int endY = qMin(h, startY + tileHeight);

            // Downscale and put in the finished grid
            // Loop through each pixel in the output image
            for (int y = startY; y < endY; ++y) {
                const int ySrc = (padding + y - startY) * DIRECT_MAP_UPSCALE_FACTOR;
                Q_ASSERT(ySrc < hExp);
                for (int x = startX; x < endX; ++x) {
                    const int xSrc = (padding + x - startX) * DIRECT_MAP_UPSCALE_FACTOR;
                    Q_ASSERT(xSrc < wExp);

                    if (mask[y * w + x] == PIXEL_VOID)
                        continue;

                    const int dstPixelI = y * w + x;
                    QVector3D average;
                    int hits = 0;
                    for (int sY = 0; sY < DIRECT_MAP_UPSCALE_FACTOR; ++sY) {
                        for (int sX = 0; sX < DIRECT_MAP_UPSCALE_FACTOR; ++sX) {
                            int srcPixelI = (ySrc + sY) * wExp + (xSrc + sX);
                            Q_ASSERT(srcPixelI < numPixelsExpanded);
                            if (maskTile[srcPixelI] == PIXEL_VOID)
                                continue;
                            average += gridTile[srcPixelI];
                            ++hits;
                        }
                    }

                    // Write value only if we have any hits. Due to sampling and precision differences it is
                    // technically possible to miss hits. In this case we fallback to the original sampled value.
                    if (hits > 0)
                        grid[dstPixelI] = average / hits;
                }
            }

            // Update progress tracker
            progressTracker.directTileDone();
        }
    }

    QHash<Edge, EdgeUV> edgeUVMap;
    QVector<SeamUV> seams;

    for (SubMeshInfo &subMeshInfo : subMeshInfos[lmIdx]) {
        QVector<std::array<quint32, 3>> triangles;
        QVector<QVector3D> positions;
        QVector<QVector3D> normals;
        QVector<QVector2D> uvs;

        triangles.reserve(subMeshInfo.count / 3);
        positions.reserve(subMeshInfo.count);
        normals.reserve(subMeshInfo.count);
        uvs.reserve(subMeshInfo.count);

        for (quint32 i = 0; i < subMeshInfo.count / 3; ++i)
            triangles.push_back({ i * 3, i * 3 + 1, i * 3 + 2 });

        for (quint32 i = 0; i < subMeshInfo.count; ++i) {
            const quint32 idx = *(ibase + subMeshInfo.offset + i);
            const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.positionOffset);
            float x = *src++;
            float y = *src++;
            float z = *src++;
            positions.push_back(QVector3D(x, y, z));
        }

        for (quint32 i = 0; i < subMeshInfo.count; ++i) {
            const quint32 idx = *(ibase + subMeshInfo.offset + i);
            const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.normalOffset);
            float x = *src++;
            float y = *src++;
            float z = *src++;
            normals.push_back(QVector3D(x, y, z));
        }

        for (quint32 i = 0; i < subMeshInfo.count; ++i) {
            const quint32 idx = *(ibase + subMeshInfo.offset + i);
            const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.lightmapUVOffset);
            float x = *src++;
            float y = *src++;
            uvs.push_back(QVector2D(x, 1.0f - y)); // NOTE: Flip y
        }

        for (auto [i0, i1, i2] : triangles) {
            const QVector3D triVert[3] = { positions[i0], positions[i1], positions[i2] };
            const QVector3D triNorm[3] = { normals[i0], normals[i1], normals[i2] };
            const QVector2D triUV[3] = { uvs[i0], uvs[i1], uvs[i2] };

            for (int i = 0; i < 3; ++i) {
                int i0 = i;
                int i1 = (i + 1) % 3;
                if (vectorLessThan(triVert[i1], triVert[i0]))
                    std::swap(i0, i1);

                const Edge e = { { triVert[i0], triVert[i1] }, { triNorm[i0], triNorm[i1] } };
                const EdgeUV edgeUV = { { triUV[i0], triUV[i1] } };
                auto it = edgeUVMap.find(e);
                if (it == edgeUVMap.end()) {
                    edgeUVMap.insert(e, edgeUV);
                } else if (!qFuzzyCompare(it->uv[0], edgeUV.uv[0]) || !qFuzzyCompare(it->uv[1], edgeUV.uv[1])) {
                    if (!it->seam) {
                        std::array<QVector2D, 2> eUV = {QVector2D(edgeUV.uv[0][0], 1.0f - edgeUV.uv[0][1]), QVector2D(edgeUV.uv[1][0], 1.0f - edgeUV.uv[1][1])};
                        std::array<QVector2D, 2> itUV = {QVector2D(it->uv[0][0], 1.0f - it->uv[0][1]), QVector2D(it->uv[1][0], 1.0f - it->uv[1][1])};

                        seams.append(SeamUV({ { eUV, itUV } }));
                        it->seam = true;
                    }
                }
            }
        }
    }

    // Blend edges
    // NOTE: We only need to blend grid since that is the resulting lightmap for direct light
    {
        QByteArray workBuf(grid.size() * sizeof(QVector3D), Qt::Uninitialized);
        for (int blendIter = 0; blendIter < LM_SEAM_BLEND_ITER_COUNT; ++blendIter) {
            memcpy(workBuf.data(), grid.constData(), grid.size() * sizeof(QVector3D));
            for (int seamIdx = 0, end = seams.size(); seamIdx != end; ++seamIdx) {
                const SeamUV &seam(seams[seamIdx]);
                blendLine(seam.uv[0][0],
                          seam.uv[0][1],
                          seam.uv[1][0],
                          seam.uv[1][1],
                          reinterpret_cast<const float *>(workBuf.data()),
                          reinterpret_cast<float *>(grid.data()),
                          QSize(w, h),
                          3);
                blendLine(seam.uv[1][0],
                          seam.uv[1][1],
                          seam.uv[0][0],
                          seam.uv[0][1],
                          reinterpret_cast<const float *>(workBuf.data()),
                          reinterpret_cast<float *>(grid.data()),
                          QSize(w, h),
                          3);
            }
        }
    }

    return grid;
}

// xorshift rng. this is called a lot -> rand/QRandomGenerator is out of question (way too slow)
static inline float uniformRand(quint32 &state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return float(state) / float(UINT32_MAX);
}

static inline QVector3D cosWeightedHemisphereSample(quint32 &state)
{
    const float r1 = uniformRand(state);
    const float r2 = uniformRand(state) * 2.0f * float(M_PI);
    const float sqr1 = std::sqrt(r1);
    const float sqr1m = std::sqrt(1.0f - r1);
    return QVector3D(sqr1 * std::cos(r2), sqr1 * std::sin(r2), sqr1m);
}

QVector<QVector3D> QSSGLightmapperPrivate::computeIndirectLight(int lmIdx, int wgCount, int wgSizePerGroup)
{
    const QVector<ModelTexel>& texels = modelTexels[lmIdx];
    QVector<QVector3D> result;
    result.resize(texels.size());

    QVector<QFuture<QVector3D>> wg(wgCount);

    for (int i = 0; i < texels.size(); ++i) {
        const ModelTexel& lmPix = texels[i];
        if (!lmPix.isValid())
            continue;

        ++indirectTexelsDone;
        for (int wgIdx = 0; wgIdx < wgCount; ++wgIdx) {
            const int beginIdx = wgIdx * wgSizePerGroup;
            const int endIdx = qMin(beginIdx + wgSizePerGroup, options.indirectLightSamples);

            wg[wgIdx] = QtConcurrent::run([this, wgIdx, beginIdx, endIdx, &lmPix] {
                QVector3D wgResult;
                quint32 state = QRandomGenerator(wgIdx).generate();
                for (int sampleIdx = beginIdx; sampleIdx < endIdx; ++sampleIdx) {
                    QVector3D position = lmPix.worldPos;
                    QVector3D normal = lmPix.normal;
                    QVector3D throughput(1.0f, 1.0f, 1.0f);
                    QVector3D sampleResult;

                    for (int bounce = 0; bounce < options.indirectLightBounces; ++bounce) {
                        if (options.useAdaptiveBias)
                            position += vectorSign(normal) * vectorAbs(position * 0.0000002f);

                        // get a sample using a cosine-weighted hemisphere sampler
                        const QVector3D sample = cosWeightedHemisphereSample(state);

                        // transform to the point's local coordinate system
                        const QVector3D v0 = qFuzzyCompare(qAbs(normal.z()), 1.0f)
                                ? QVector3D(0.0f, 1.0f, 0.0f)
                                : QVector3D(0.0f, 0.0f, 1.0f);
                        const QVector3D tangent = QVector3D::crossProduct(v0, normal).normalized();
                        const QVector3D bitangent = QVector3D::crossProduct(tangent, normal).normalized();
                        QVector3D direction(
                                    tangent.x() * sample.x() + bitangent.x() * sample.y() + normal.x() * sample.z(),
                                    tangent.y() * sample.x() + bitangent.y() * sample.y() + normal.y() * sample.z(),
                                    tangent.z() * sample.x() + bitangent.z() * sample.y() + normal.z() * sample.z());
                        direction.normalize();

                        // probability distribution function
                        const float NdotL = qMax(0.0f, QVector3D::dotProduct(normal, direction));
                        const float pdf = NdotL / float(M_PI);
                        if (qFuzzyIsNull(pdf))
                            break;

                        // shoot ray, stop if no hit
                        RayHit ray(position, direction, options.bias);
                        if (!ray.intersect(rscene))
                            break;

                        // see what (sub)mesh and which texel it intersected with
                        const ModelTexel &hitEntry = texelForLightmapUV(ray.rayhit.hit.geomID,
                                                                           ray.rayhit.hit.u,
                                                                           ray.rayhit.hit.v);

                        // won't bounce further from a back face
                        const bool hitBackFace = QVector3D::dotProduct(hitEntry.normal, direction) > 0.0f;
                        if (hitBackFace)
                            break;

                        // the BRDF of a diffuse surface is albedo / PI
                        const QVector3D brdf = hitEntry.baseColor.toVector3D() / float(M_PI);

                        // calculate result for this bounce
                        sampleResult += throughput * hitEntry.emission;
                        throughput *= brdf * NdotL / pdf;
                        QVector3D directLight = sampleDirectLight(hitEntry.worldPos, hitEntry.normal, true);
                        sampleResult += throughput * directLight;

                        // stop if we guess there's no point in bouncing further
                        // (low throughput path wouldn't contribute much)
                        const float p = qMax(qMax(throughput.x(), throughput.y()), throughput.z());
                        if (p < uniformRand(state))
                            break;

                        // was not terminated: boost the energy by the probability to be terminated
                        throughput /= p;

                        // next bounce starts from the hit's position
                        position = hitEntry.worldPos;
                        normal = hitEntry.normal;
                    }

                    wgResult += sampleResult;
                }
                return wgResult;
            });
        }

        QVector3D totalIndirect;
        for (const auto &future : wg)
            totalIndirect += future.result();

        result[i] += totalIndirect * options.indirectLightFactor / options.indirectLightSamples;

        if (bakingControl.cancelled)
            return {};

        progressTracker.indirectTexelDone(indirectTexelsDone, indirectTexelsTotal);
    }

    return result;
}

static QString stripQrcPrefix(const QString &path)
{
    QString result = path;
    if (result.startsWith(QStringLiteral(":/")))
        result.remove(0, 2);
    return result;
}

// Creates all parent directories needed for the given file path.
// Returns true on success, false if creation fails.
static bool createDirectory(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.path();
    QDir dir;

    if (dir.exists(dirPath))
        return true;

    if (!dir.mkpath(dirPath))
        return false;

    return true;
}

static bool isValidSavePath(const QString &path) {
    const QFileInfo info = QFileInfo(path);
    if (!info.exists()) {
        return QFileInfo(info.dir().path()).isWritable();
    }
    return info.isWritable() && !info.isDir();
}

static inline QString indexToMeshKey(int index)
{
    return QStringLiteral("_mesh_%1").arg(index);
}

bool QSSGLightmapperPrivate::storeMeshes(QSharedPointer<QSSGLightmapWriter> writer)
{
    if (!isValidSavePath(outputPath)) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                       QStringLiteral("Source path %1 is not a writable location").arg(outputPath));
        return false;
    }

    for (int i = 0; i < meshes.size(); ++i) {
        if (!writer->writeData(indexToMeshKey(i), QSSGLightmapIODataTag::Mesh, meshes[i]))
            return false;
    }

    return true;
}

bool QSSGLightmapperPrivate::storeSceneMetadata(QSharedPointer<QSSGLightmapWriter> writer)
{
    QVariantMap metadata;

    metadata[QStringLiteral("qt_version")] = QString::fromUtf8(QT_VERSION_STR);
    metadata[QStringLiteral("bake_start_time")] = bakeStartTime;
    metadata[QStringLiteral("bake_end_time")] = QDateTime::currentMSecsSinceEpoch();

    QVariantMap metadata2;
    metadata2[QStringLiteral("opacityThreshold")] = options.opacityThreshold;
    metadata2[QStringLiteral("bias")] = options.bias;
    metadata2[QStringLiteral("useAdaptiveBias")] = options.useAdaptiveBias;
    metadata2[QStringLiteral("indirectLightEnabled")] = options.indirectLightEnabled;
    metadata2[QStringLiteral("indirectLightSamples")] = options.indirectLightSamples;
    metadata2[QStringLiteral("indirectLightWorkgroupSize")] = options.indirectLightWorkgroupSize;
    metadata2[QStringLiteral("indirectLightBounces")] = options.indirectLightBounces;
    metadata2[QStringLiteral("indirectLightFactor")] = options.indirectLightFactor;
    metadata2[QStringLiteral("denoiseSigma")] = options.sigma;
    metadata2[QStringLiteral("texelsPerUnit")] = options.texelsPerUnit;

    metadata[QStringLiteral("options")] = metadata2;
    return writer->writeMap(QString::fromUtf8(KEY_SCENE_METADATA), QSSGLightmapIODataTag::SceneMetadata, metadata);
}

bool QSSGLightmapperPrivate::storeMetadata(int lmIdx, QSharedPointer<QSSGLightmapWriter> writer)
{
    const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
    const DrawInfo &drawInfo(drawInfos[lmIdx]);

    QVariantMap metadata;
    metadata[QStringLiteral("width")] = drawInfos[lmIdx].lightmapSize.width();
    metadata[QStringLiteral("height")] = drawInfos[lmIdx].lightmapSize.height();
    metadata[QStringLiteral("mesh_key")] = indexToMeshKey(drawInfo.meshIndex);

    return writer->writeMap(lm.model->lightmapKey, QSSGLightmapIODataTag::Metadata, metadata);
}

bool QSSGLightmapperPrivate::storeDirectLightData(int lmIdx, const QVector<QVector3D> &directLight, QSharedPointer<QSSGLightmapWriter> writer)
{
    const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
    const int numTexels = modelTexels[lmIdx].size();

    QByteArray directFP32(numTexels * 4 * sizeof(float), Qt::Uninitialized);
    float *directFloatPtr = reinterpret_cast<float *>(directFP32.data());

    for (int i = 0; i < numTexels; ++i) {
        const auto &lmPix = modelTexels[lmIdx][i];
        if (lmPix.isValid()) {
            *directFloatPtr++ = directLight[i].x();
            *directFloatPtr++ = directLight[i].y();
            *directFloatPtr++ = directLight[i].z();
            *directFloatPtr++ = 1.0f;
        } else {
            *directFloatPtr++ = 0.0f;
            *directFloatPtr++ = 0.0f;
            *directFloatPtr++ = 0.0f;
            *directFloatPtr++ = 0.0f;
        }
    }

    const QByteArray dilated = dilate(drawInfos[lmIdx].lightmapSize, directFP32);

    if (dilated.isEmpty())
        return false;

    writer->writeF32Image(lm.model->lightmapKey, QSSGLightmapIODataTag::Texture_Direct, dilated);

    return true;
}

bool QSSGLightmapperPrivate::storeIndirectLightData(int lmIdx, const QVector<QVector3D> &indirectLight, QSharedPointer<QSSGLightmapWriter> writer)
{
    const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
    const int numTexels = modelTexels[lmIdx].size();

    QByteArray lightmapFP32(numTexels * 4 * sizeof(float), Qt::Uninitialized);
    float *lightmapFloatPtr = reinterpret_cast<float *>(lightmapFP32.data());

    for (int i = 0; i < numTexels; ++i) {
        const auto &lmPix = modelTexels[lmIdx][i];
        if (lmPix.isValid()) {
            *lightmapFloatPtr++ = indirectLight[i].x();
            *lightmapFloatPtr++ = indirectLight[i].y();
            *lightmapFloatPtr++ = indirectLight[i].z();
            *lightmapFloatPtr++ = 1.0f;
        } else {
            *lightmapFloatPtr++ = 0.0f;
            *lightmapFloatPtr++ = 0.0f;
            *lightmapFloatPtr++ = 0.0f;
            *lightmapFloatPtr++ = 0.0f;
        }
    }

    QByteArray dilated = dilate(drawInfos[lmIdx].lightmapSize, lightmapFP32);

    if (dilated.isEmpty())
        return false;

    // Reduce UV seams by collecting all edges (going through all
    // triangles), looking for (fuzzy)matching ones, then drawing lines
    // with blending on top.
    const DrawInfo &drawInfo(drawInfos[lmIdx]);
    const char *vbase = drawInfo.vertexData.constData();
    const quint32 *ibase = reinterpret_cast<const quint32 *>(drawInfo.indexData.constData());

    // topology is Triangles, would be indexed draw - get rid of the index
    // buffer, need nothing but triangles afterwards
    qsizetype assembledVertexCount = 0;
    for (SubMeshInfo &subMeshInfo : subMeshInfos[lmIdx])
        assembledVertexCount += subMeshInfo.count;
    QVector<QVector3D> smPos(assembledVertexCount);
    QVector<QVector3D> smNormal(assembledVertexCount);
    QVector<QVector2D> smCoord(assembledVertexCount);
    qsizetype vertexIdx = 0;
    for (SubMeshInfo &subMeshInfo : subMeshInfos[lmIdx]) {
        for (quint32 i = 0; i < subMeshInfo.count; ++i) {
            const quint32 idx = *(ibase + subMeshInfo.offset + i);
            const float *src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.positionOffset);
            float x = *src++;
            float y = *src++;
            float z = *src++;
            smPos[vertexIdx] = QVector3D(x, y, z);
            src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.normalOffset);
            x = *src++;
            y = *src++;
            z = *src++;
            smNormal[vertexIdx] = QVector3D(x, y, z);
            src = reinterpret_cast<const float *>(vbase + idx * drawInfo.vertexStride + drawInfo.lightmapUVOffset);
            x = *src++;
            y = *src++;
            smCoord[vertexIdx] = QVector2D(x, y);
            ++vertexIdx;
        }
    }

    QHash<Edge, EdgeUV> edgeUVMap;
    QVector<SeamUV> seams;
    for (vertexIdx = 0; vertexIdx < assembledVertexCount; vertexIdx += 3) {
        QVector3D triVert[3] = { smPos[vertexIdx], smPos[vertexIdx + 1], smPos[vertexIdx + 2] };
        QVector3D triNorm[3] = { smNormal[vertexIdx], smNormal[vertexIdx + 1], smNormal[vertexIdx + 2] };
        QVector2D triUV[3] = { smCoord[vertexIdx], smCoord[vertexIdx + 1], smCoord[vertexIdx + 2] };

        for (int i = 0; i < 3; ++i) {
            int i0 = i;
            int i1 = (i + 1) % 3;
            if (vectorLessThan(triVert[i1], triVert[i0]))
                std::swap(i0, i1);

            const Edge e = {
                { triVert[i0], triVert[i1] },
                { triNorm[i0], triNorm[i1] }
            };
            const EdgeUV edgeUV = { { triUV[i0], triUV[i1] } };
            auto it = edgeUVMap.find(e);
            if (it == edgeUVMap.end()) {
                edgeUVMap.insert(e, edgeUV);
            } else if (!qFuzzyCompare(it->uv[0], edgeUV.uv[0]) || !qFuzzyCompare(it->uv[1], edgeUV.uv[1])) {
                if (!it->seam) {
                    seams.append(SeamUV({ { edgeUV.uv, it->uv } }));
                    it->seam = true;
                }
            }
        }
    }
    //qDebug() << "lm:" << seams.size() << "UV seams in" << lm.model;

    QByteArray workBuf(dilated.size(), Qt::Uninitialized);
    for (int blendIter = 0; blendIter < LM_SEAM_BLEND_ITER_COUNT; ++blendIter) {
        memcpy(workBuf.data(), dilated.constData(), dilated.size());
        for (int seamIdx = 0, end = seams.size(); seamIdx != end; ++seamIdx) {
            const SeamUV &seam(seams[seamIdx]);
            blendLine(seam.uv[0][0], seam.uv[0][1],
                      seam.uv[1][0], seam.uv[1][1],
                      reinterpret_cast<const float *>(workBuf.data()),
                      reinterpret_cast<float *>(dilated.data()),
                      drawInfos[lmIdx].lightmapSize);
            blendLine(seam.uv[1][0], seam.uv[1][1],
                      seam.uv[0][0], seam.uv[0][1],
                      reinterpret_cast<const float *>(workBuf.data()),
                      reinterpret_cast<float *>(dilated.data()),
                      drawInfos[lmIdx].lightmapSize);
        }
    }

    writer->writeF32Image(lm.model->lightmapKey, QSSGLightmapIODataTag::Texture_Indirect, dilated);

    return true;
}

bool QSSGLightmapperPrivate::storeMaskImage(int lmIdx, QSharedPointer<QSSGLightmapWriter> writer)
{
    constexpr quint32 PIXEL_VOID = 0;
    constexpr quint32 PIXEL_UNSET = -1;

    const QSSGBakedLightingModel &lm(bakedLightingModels[lmIdx]);
    const int numTexels = modelTexels[lmIdx].size();

    QByteArray mask(numTexels * sizeof(quint32), Qt::Uninitialized);
    quint32 *maskUIntPtr = reinterpret_cast<quint32 *>(mask.data());

    for (int i = 0; i < numTexels; ++i) {
        *maskUIntPtr++ = modelTexels[lmIdx][i].isValid() ? PIXEL_UNSET : PIXEL_VOID;
    }

    const int rows = drawInfos[lmIdx].lightmapSize.height();
    const int cols = drawInfos[lmIdx].lightmapSize.width();

    // Use flood fill so each chart has its own "color" which
    // can then be used in the denoise shader to only take into account
    // pixels in the same chart.
    floodFill(reinterpret_cast<quint32 *>(mask.data()), rows, cols);

    writer->writeF32Image(lm.model->lightmapKey, QSSGLightmapIODataTag::Mask, mask);

    return true;
}

bool QSSGLightmapperPrivate::denoiseLightmaps()
{
    QElapsedTimer denoiseTimer;
    denoiseTimer.start();

    // Tmp file
    const QString inPath = QFileInfo(outputPath + QStringLiteral(".raw")).absoluteFilePath();
    QSharedPointer<QSSGLightmapLoader> tmpFile = QSSGLightmapLoader::open(inPath);
    if (!tmpFile) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Error, QStringLiteral("Could not read file '%1'").arg(inPath));
        return false;
    }

    // Final file
    const QString outPath = QFileInfo(outputPath).absoluteFilePath();
    QSharedPointer<QSSGLightmapWriter> finalFile = QSSGLightmapWriter::open(outPath);
    if (!finalFile) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Error, QStringLiteral("Could not read file '%1'").arg(outPath));
        return false;
    }

    QSet<QString> lightmapKeys;
    for (const auto &[key, tag] : tmpFile->getKeys()) {
        if (tag == QSSGLightmapIODataTag::SceneMetadata) continue; // Will write at end

        if (tag != QSSGLightmapIODataTag::Texture_Direct && tag != QSSGLightmapIODataTag::Texture_Indirect
            && tag != QSSGLightmapIODataTag::Mask) {
            // Clone meshes and metadata for final file
            finalFile->writeData(key, tag, tmpFile->readData(key, tag));
        } else if (tag == QSSGLightmapIODataTag::Texture_Direct) {
            lightmapKeys.insert(key);
        }
    }

    QRhi *rhi = rhiCtxInterface->rhiContext()->rhi();
    Q_ASSERT(rhi);
    if (!rhi->isFeatureSupported(QRhi::Compute)) {
        qFatal("Compute is not supported, denoising disabled");
        return false;
    }

    const int bakedLightingModelCount = lightmapKeys.size();
    if (bakedLightingModelCount == 0)
        return true;

    QShader shader;
    if (QFile f(QStringLiteral(":/res/rhishaders/nlm_denoise.comp.qsb")); f.open(QIODevice::ReadOnly)) {
        shader = QShader::fromSerialized(f.readAll());
    } else {
        qFatal() << "Could not find denoise shader";
        return false;
    }
    Q_ASSERT(shader.isValid());

    QVariantMap sceneMetadata = tmpFile->readMap(QString::fromUtf8(KEY_SCENE_METADATA), QSSGLightmapIODataTag::SceneMetadata);
    sceneMetadata[QStringLiteral("denoise_start_time")] = QDateTime::currentMSecsSinceEpoch();

    int lmIdx = -1;
    for (const QString &key : lightmapKeys) {
        ++lmIdx;
        auto incrementTracker = QScopeGuard([this, lmIdx, bakedLightingModelCount]() {
            progressTracker.denoisedModelDone(lmIdx + 1, bakedLightingModelCount);
        });


        sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                       QStringLiteral("[%2/%3] denoising '%1'").arg(key).arg(lmIdx + 1).arg(bakedLightingModelCount));

        QVariantMap metadata = tmpFile->readMap(key, QSSGLightmapIODataTag::Metadata);
        QByteArray indirect = tmpFile->readF32Image(key, QSSGLightmapIODataTag::Texture_Indirect);
        QByteArray direct = tmpFile->readF32Image(key, QSSGLightmapIODataTag::Texture_Direct);
        QByteArray mask = tmpFile->readU32Image(key, QSSGLightmapIODataTag::Mask);

        if (!metadata.contains(QStringLiteral("width")) || !metadata.contains(QStringLiteral("height"))
            || indirect.isEmpty() || direct.isEmpty() || mask.isEmpty()) {
            sendOutputInfo(QSSGLightmapper::BakingStatus::Error,
                           QStringLiteral("[%2/%3] Failed to denoise '%1'").arg(key).arg(lmIdx + 1).arg(bakedLightingModelCount));
            continue;
        }

        QRhiCommandBuffer *cb = nullptr;
        cb = rhiCtxInterface->rhiContext()->commandBuffer();
        Q_ASSERT(cb);

        QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
        Q_ASSERT(u);

        const int w = metadata[QStringLiteral("width")].toInt();
        const int h = metadata[QStringLiteral("height")].toInt();
        const QSize size(w, h);
        const int numPixels = w * h;

        Q_ASSERT(qsizetype(numPixels * sizeof(float) * 4) == indirect.size());
        Q_ASSERT(qsizetype(numPixels * sizeof(float) * 4) == direct.size());
        Q_ASSERT(qsizetype(numPixels * sizeof(quint32)) == mask.size());

        QScopedPointer<QRhiBuffer> buffIn(rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, 3 * numPixels * sizeof(float)));
        QScopedPointer<QRhiBuffer> buffCount(rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, numPixels * sizeof(quint32)));
        QScopedPointer<QRhiBuffer> buffOut(rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, 3 * numPixels * sizeof(quint32)));
        QScopedPointer<QRhiTexture> texMask(rhi->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::UsedWithLoadStore));

        buffIn->create();
        buffCount->create();
        buffOut->create();
        texMask->create();

        u->uploadTexture(texMask.data(), QImage(reinterpret_cast<const uchar *>(mask.constData()), w, h, QImage::Format_RGBA8888));

        // fill and upload input and count buffers
        {
            QByteArray inArray(3 * numPixels * sizeof(float), 0);
            QByteArray count(numPixels * sizeof(quint32), 0);
            QByteArray outArray(3 * numPixels * sizeof(float), 0);

            QVector3D* inDst = reinterpret_cast<QVector3D*>(inArray.data());
            const QVector4D* indirectSrc = reinterpret_cast<const QVector4D*>(indirect.data());
            for (int i = 0; i < numPixels; ++i) {
                inDst[i][0] = indirectSrc[i][0] * 256.f;
                inDst[i][1] = indirectSrc[i][1] * 256.f;
                inDst[i][2] = indirectSrc[i][2] * 256.f;
            }
            u->uploadStaticBuffer(buffIn.data(), inArray);
            u->uploadStaticBuffer(buffCount.data(), count);
            u->uploadStaticBuffer(buffOut.data(), outArray);
        }

        struct Settings
        {
            float sigma;
            float width; // int
            float height; // int
        } settings;

        settings.sigma = options.sigma;
        settings.width = w;
        settings.height = h;

        QScopedPointer<QRhiBuffer> settingsBuffer(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(settings)));
        settingsBuffer->create();

        u->updateDynamicBuffer(settingsBuffer.data(), 0, sizeof(settings), &settings);

        QScopedPointer<QRhiShaderResourceBindings> srb(rhi->newShaderResourceBindings());
        srb->setBindings(
                {
                  QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::ComputeStage, settingsBuffer.data()),
                  QRhiShaderResourceBinding::bufferLoad(1, QRhiShaderResourceBinding::ComputeStage, buffIn.data()),
                  QRhiShaderResourceBinding::imageLoad(2, QRhiShaderResourceBinding::ComputeStage, texMask.data(), 0),
                  QRhiShaderResourceBinding::bufferLoadStore(3, QRhiShaderResourceBinding::ComputeStage, buffOut.data()),
                  QRhiShaderResourceBinding::bufferLoadStore(4, QRhiShaderResourceBinding::ComputeStage, buffCount.data())
                });
        srb->create();

        QScopedPointer<QRhiComputePipeline> pipeline(rhi->newComputePipeline());
        pipeline->setShaderStage({ QRhiShaderStage::Compute, shader });
        pipeline->setShaderResourceBindings(srb.data());
        pipeline->create();

        cb->beginComputePass(u);
        cb->setComputePipeline(pipeline.data());
        cb->setShaderResources();
        constexpr int local_size_x = 8;
        constexpr int local_size_y = 8;
        constexpr int local_size_z = 1;
        cb->dispatch((w + local_size_x - 1) / local_size_x, (h + local_size_y - 1) / local_size_y, local_size_z);

        u = rhi->nextResourceUpdateBatch();
        Q_ASSERT(u);

        QByteArray final;
        QByteArray outOut;
        QByteArray outCount;

        QRhiReadbackResult readResultOut;
            readResultOut.completed = [&] {
            outOut = readResultOut.data;
            Q_ASSERT(outOut.size() == qsizetype(numPixels * sizeof(quint32) * 3));
        };
        QRhiReadbackResult readResultCount;
        readResultCount.completed = [&] {
            outCount = readResultCount.data;
            Q_ASSERT(outCount.size() == qsizetype(numPixels * sizeof(quint32)));
        };

        u->readBackBuffer(buffOut.get(), 0, 3 * numPixels *sizeof(quint32), &readResultOut);
        u->readBackBuffer(buffCount.get(), 0, numPixels * sizeof(quint32), &readResultCount);

        cb->endComputePass(u);
        rhi->finish();

        // Write back to image.data variable
        final.resize(indirect.size());
        memcpy(final.data(), indirect.data(), indirect.size());

        QVector4D* res = reinterpret_cast<QVector4D*>(final.data());
        quint32* ptrRGB = reinterpret_cast<quint32*>(outOut.data());
        quint32* ptrCount = reinterpret_cast<quint32*>(outCount.data());
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const int idxDst = y * w + x;
                const int idxDst1 = 3 * idxDst;
                Q_ASSERT(idxDst1 < numPixels * 3);
                quint32 cnt = ptrCount[idxDst];
                //Q_ASSERT(cnt);
                float r = (ptrRGB[idxDst1] / 256.f) / 1000.f;
                float g = (ptrRGB[idxDst1 + 1] / 256.f) / 1000.f;
                float b = (ptrRGB[idxDst1 + 2] / 256.f) / 1000.f;
                if (cnt > 0) {
                    res[idxDst][0] = r / cnt;
                    res[idxDst][1] = g / cnt;
                    res[idxDst][2] = b / cnt;
                }
            }
        }

        std::array<float, 4> *imagePtr = reinterpret_cast<std::array<float, 4>*>(const_cast<char*>(final.data()));
        std::array<float, 4> *directPtr = reinterpret_cast<std::array<float, 4>*>(const_cast<char*>(direct.data()));
        for (int i = 0; i < numPixels; ++i) {
            imagePtr[i][0] += directPtr[i][0];
            imagePtr[i][1] += directPtr[i][1];
            imagePtr[i][2] += directPtr[i][2];
            // skip alpha, always 0 or 1
            Q_ASSERT(imagePtr[i][3] == directPtr[i][3]);
            Q_ASSERT(imagePtr[i][3] == 1.f || imagePtr[i][3] == 0.f);
        }

        finalFile->writeF32Image(key, QSSGLightmapIODataTag::Texture_Final, final);
    }

    sceneMetadata[QStringLiteral("denoise_end_time")] = QDateTime::currentMSecsSinceEpoch();
    auto optionsMap = sceneMetadata[QStringLiteral("options")].toMap();
    optionsMap[QStringLiteral("denoiseSigma")] = options.sigma;
    sceneMetadata[QStringLiteral("options")] = optionsMap;

    finalFile->writeMap(QString::fromUtf8(KEY_SCENE_METADATA), QSSGLightmapIODataTag::SceneMetadata, sceneMetadata);

    if (!finalFile->close()) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Error, QStringLiteral("Could not save file '%1'").arg(outPath));
        return false;
    }

    return true;

}

bool QSSGLightmapperPrivate::userCancelled()
{
    if (bakingControl.cancelled) {
        sendOutputInfo(QSSGLightmapper::BakingStatus::Cancelled,
                       QStringLiteral("Cancelled by user"));
    }
    return bakingControl.cancelled;
}

void QSSGLightmapperPrivate::sendOutputInfo(QSSGLightmapper::BakingStatus type, std::optional<QString> msg, bool outputToConsole, bool outputConsoleTimeRemanining)
{
    if (outputToConsole) {
        QString consoleMessage;

        switch (type)
        {
        case QSSGLightmapper::BakingStatus::None:
            return;
        case QSSGLightmapper::BakingStatus::Info:
            consoleMessage = QStringLiteral("[lm] Info");
            break;
        case QSSGLightmapper::BakingStatus::Error:
            consoleMessage = QStringLiteral("[lm] Error");
            break;
        case QSSGLightmapper::BakingStatus::Warning:
            consoleMessage = QStringLiteral("[lm] Warning");
            break;
        case QSSGLightmapper::BakingStatus::Cancelled:
            consoleMessage = QStringLiteral("[lm] Cancelled");
            break;
        case QSSGLightmapper::BakingStatus::Failed:
            consoleMessage = QStringLiteral("[lm] Failed");
            break;
        case QSSGLightmapper::BakingStatus::Complete:
            consoleMessage = QStringLiteral("[lm] Complete");
            break;
        }

        if (msg.has_value())
            consoleMessage.append(QStringLiteral(": ") + msg.value());
        else if (outputConsoleTimeRemanining) {
            const QString timeRemaining = estimatedTimeRemaining >= 0 ? formatDuration(estimatedTimeRemaining, false)
                                                            : QStringLiteral("Estimating...");
            consoleMessage.append(QStringLiteral(": Time remaining: ") + timeRemaining);
        }

        if (type == QSSGLightmapper::BakingStatus::Error || type == QSSGLightmapper::BakingStatus::Warning)
            qWarning() << consoleMessage;
        else
            qInfo() << consoleMessage;
    }

    if (outputCallback) {
        QVariantMap payload;
        payload[QStringLiteral("status")] = (int)type;
        payload[QStringLiteral("stage")] = stage;
        payload[QStringLiteral("message")] = msg.value_or(QString());
        payload[QStringLiteral("totalTimeRemaining")] = estimatedTimeRemaining;
        payload[QStringLiteral("totalProgress")] = totalProgress;
        outputCallback(payload, &bakingControl);
    }
}

void QSSGLightmapperPrivate::updateStage(const QString &newStage)
{
    if (newStage == stage)
        return;

    stage = newStage;
    if (outputCallback) {
        QVariantMap payload;
        payload[QStringLiteral("stage")] = stage;
        outputCallback(payload, &bakingControl);
    }
}

bool QSSGLightmapper::bake()
{
    d->totalTimer.start();
    d->bakeStartTime = QDateTime::currentMSecsSinceEpoch();

    d->updateStage(QStringLiteral("Preparing"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Preparing for bake..."));

    if (!isValidSavePath(d->outputPath)) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                          QStringLiteral("Source path %1 is not a writable location").arg(d->outputPath));
        return false;
    }

    if (d->bakedLightingModels.isEmpty()) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("No Models to bake"));
        return false;
    }

    // ------------- Commit geometry -------------

    if (!d->commitGeometry()) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Baking failed"));
        return false;
    }

    // Main thread can continue
    d->initMutex.lock();
    d->initCondition.wakeAll();
    d->initMutex.unlock();

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // ------------- Init Progress Tracker ---------
    const int bakedLightingModelCount = d->bakedLightingModels.size();

    // Precompute the number of direct light tiles for progress tracking
    quint32 numDirectTiles = 0;
    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        QSSGBakedLightingModel &lm = d->bakedLightingModels[lmIdx];
        if (d->denoiseOnly)
            break;
        if (!lm.model->hasLightmap())
            continue;
        if (!lm.model->castsShadows)
            continue;

        const auto &drawInfo = d->drawInfos[lmIdx];
        const QSize sz = drawInfo.lightmapSize;
        const int w = sz.width();
        const int h = sz.height();
        constexpr int maxTileSize = MAX_TILE_SIZE / DIRECT_MAP_UPSCALE_FACTOR;
        const int numTilesX = (w + maxTileSize - 1) / maxTileSize;
        const int numTilesY = (h + maxTileSize - 1) / maxTileSize;

        numDirectTiles += numTilesX * numTilesY;
    }

    d->progressTracker.initBake(d->options.indirectLightSamples, d->options.indirectLightBounces);
    d->progressTracker.setTotalDirectTiles(numDirectTiles);

    // ------------- Prepare lightmaps -------------

    if (!d->prepareLightmaps()) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Baking failed"));
        return false;
    }

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    if (!d->verifyLights()) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                          QStringLiteral("Did not find any lights with baking enabled or any "
                                         "emissive models in the scene."));
        return false;
    }

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                      QStringLiteral("Total emissive models registered: %1")
                          .arg(d->emissiveModelCount));

    // indirect lighting is slow, so parallelize per groups of samples,
    // e.g. if sample count is 256 and workgroup size is 32, then do up to
    // 8 sets in parallel, each calculating 32 samples (how many of the 8
    // are really done concurrently that's up to the thread pool to manage)
    const int wgSizePerGroup = qMax(1, d->options.indirectLightWorkgroupSize);
    const int wgCount = (d->options.indirectLightSamples / wgSizePerGroup) + (d->options.indirectLightSamples % wgSizePerGroup ? 1: 0);

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Sample count: %1, Workgroup size: %2, Max bounces: %3, Multiplier: %4").
                                                           arg(d->options.indirectLightSamples).
                                                           arg(wgSizePerGroup).
                                                           arg(d->options.indirectLightBounces).
                                                           arg(d->options.indirectLightFactor));

    // We use a work-file where we store the baked lightmaps accumulatively and when
    // the baking process is finished successfully, replace the .raw file with it.
    QSharedPointer<QTemporaryFile> workFile = QSharedPointer<QTemporaryFile>::create(QDir::tempPath() + "/qt_lightmapper_work_file_XXXXXX"_L1);

    QElapsedTimer timer;
    timer.start();

    // ------------- Store metadata -------------

    d->updateStage(QStringLiteral("Storing Metadata"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Storing metadata..."));
    auto writer = QSSGLightmapWriter::open(workFile);

    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        if (d->userCancelled()) {
            d->updateStage(QStringLiteral("Cancelled"));
            return false;
        }
        QSSGBakedLightingModel &lm = d->bakedLightingModels[lmIdx];
        if (!lm.model->hasLightmap())
            continue;

        if (!d->storeMetadata(lmIdx, writer)) {
            d->updateStage(QStringLiteral("Failed"));
            d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                              QStringLiteral("[%1/%2] Failed to store metadata for '%3'")
                                  .arg(lmIdx + 1)
                                  .arg(bakedLightingModelCount)
                                  .arg(lm.model->lightmapKey));
            return false;
        }
    }

    // ------------- Store mask -------------

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Storing mask images..."));
    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        if (d->userCancelled()) {
            d->updateStage(QStringLiteral("Cancelled"));
            return false;
        }
        QSSGBakedLightingModel &lm = d->bakedLightingModels[lmIdx];
        if (!lm.model->hasLightmap())
            continue;

        if (!d->storeMaskImage(lmIdx, writer)) {
            d->updateStage(QStringLiteral("Failed"));
            d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                              QStringLiteral("[%1/%2] Failed to store mask for '%3'")
                                  .arg(lmIdx + 1)
                                  .arg(bakedLightingModelCount)
                                  .arg(lm.model->lightmapKey));
            return false;
        }
    }
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                      QStringLiteral("Took %1").arg(formatDuration(timer.restart())));

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // ------------- Direct compute / store -------------

    d->updateStage(QStringLiteral("Computing Direct Light"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Computing direct light..."));
    for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
        if (d->userCancelled()) {
            d->updateStage(QStringLiteral("Cancelled"));
            return false;
        }
        QSSGBakedLightingModel &lm = d->bakedLightingModels[lmIdx];
        if (!lm.model->hasLightmap())
            continue;

        timer.restart();
        const QVector<QVector3D> directLight = d->computeDirectLight(lmIdx);
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                          QStringLiteral("[%1/%2] '%3' took %4")
                              .arg(lmIdx + 1)
                              .arg(bakedLightingModelCount)
                              .arg(lm.model->lightmapKey)
                              .arg(formatDuration(timer.elapsed())));

        if (directLight.empty()) {
            d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                              QStringLiteral("[%1/%2] Failed to compute for '%3'")
                                  .arg(lmIdx + 1)
                                  .arg(bakedLightingModelCount)
                                  .arg(lm.model->lightmapKey));
            return false;
        }

        if (!d->storeDirectLightData(lmIdx, directLight, writer)) {
            d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                              QStringLiteral("[%1/%2] Failed to store data for '%3'")
                                  .arg(lmIdx + 1)
                                  .arg(bakedLightingModelCount)
                                  .arg(lm.model->lightmapKey));
            return false;
        }
    }

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // ------------- Indirect compute / store -------------

    if (d->options.indirectLightEnabled) {
        d->indirectTexelsTotal = std::accumulate(d->numValidTexels.begin(), d->numValidTexels.end(), quint64(0));
        d->updateStage(QStringLiteral("Computing Indirect Light"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                          QStringLiteral("Computing indirect light..."));
        d->progressTracker.setStage(Stage::Indirect);
        for (int lmIdx = 0; lmIdx < bakedLightingModelCount; ++lmIdx) {
            if (d->userCancelled()) {
                d->updateStage(QStringLiteral("Cancelled"));
                return false;
            }
            QSSGBakedLightingModel &lm = d->bakedLightingModels[lmIdx];
            if (!lm.model->hasLightmap())
                continue;

            timer.restart();
            const QVector<QVector3D> indirectLight = d->computeIndirectLight(lmIdx, wgCount, wgSizePerGroup);
            if (indirectLight.empty()) {
                d->updateStage(QStringLiteral("Failed"));
                d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                                  QStringLiteral("[%1/%2] Failed to compute '%3'")
                                      .arg(lmIdx + 1)
                                      .arg(bakedLightingModelCount)
                                      .arg(lm.model->lightmapKey));
                return false;
            }

            d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                              QStringLiteral("[%1/%2] '%3' took %4")
                                  .arg(lmIdx + 1)
                                  .arg(bakedLightingModelCount)
                                  .arg(lm.model->lightmapKey)
                                  .arg(formatDuration(timer.elapsed())));

            if (d->userCancelled()) {
                d->updateStage(QStringLiteral("Cancelled"));
                return false;
            }

            if (!d->storeIndirectLightData(lmIdx, indirectLight, writer)) {
                d->updateStage(QStringLiteral("Failed"));
                d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                                  QStringLiteral("[%1/%2] Failed to store data for '%3'")
                                      .arg(lmIdx + 1)
                                      .arg(bakedLightingModelCount)
                                      .arg(lm.model->lightmapKey));
                return false;
            }
        }
    }

    // ------------- Store meshes -------------

    if (!d->storeMeshes(writer)) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Failed to store meshes"));
        return false;
    }

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // ------------- Scene Metadata ---------

    d->updateStage(QStringLiteral("Storing Scene Metadata"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Storing scene metadata..."));
    if (!d->storeSceneMetadata(writer)) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                          QStringLiteral("Failed to store scene metadata"));
    }

    // ------------- Copy file from tmp -------------

    if (!writer->close()) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Error,
                          QStringLiteral("Failed to save temp file to %1").arg(workFile->fileName()));
        return false;
    }

    const QString tmpPath = QFileInfo(d->outputPath).absoluteFilePath() + ".raw"_L1;
    QFile::remove(tmpPath);
    if (!workFile->copy(tmpPath)) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Error,
                          QStringLiteral("Failed to copy temp file to %1").arg(tmpPath));
        return false;
    }

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // ------------- Denoising -------------

    d->progressTracker.setStage(Stage::Denoise);
    d->updateStage(QStringLiteral("Denoising"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Denoising..."));
    timer.restart();
    if (!d->denoiseLightmaps()) {
        d->updateStage(QStringLiteral("Failed"));
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Denoising failed"));
        return false;
    }
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Took %1").arg(formatDuration(timer.elapsed())));

    if (d->userCancelled()) {
        d->updateStage(QStringLiteral("Cancelled"));
        return false;
    }

    // -------------------------------------

    d->totalProgress = 1.0;
    d->estimatedTimeRemaining = -1;
    d->updateStage(QStringLiteral("Done"));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                      QStringLiteral("Baking took %1").arg(formatDuration(d->totalTimer.elapsed())));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Complete, std::nullopt);
    return true;
}

bool QSSGLightmapper::denoise() {

    // Main thread can continue
    d->initMutex.lock();
    d->initCondition.wakeAll();
    d->initMutex.unlock();

    QElapsedTimer totalTimer;
    totalTimer.start();

    d->progressTracker.initDenoise();
    d->progressTracker.setStage(Stage::Denoise);
    d->updateStage("Denoising"_L1);
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Denoise starting..."));

    if (!d->denoiseLightmaps()) {
        d->updateStage("Failed"_L1);
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Denoising failed"));
        return false;
    }

    d->totalProgress = 1;
    d->updateStage("Done"_L1);
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Denoising took %1 ms").arg(totalTimer.elapsed()));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Complete, std::nullopt);
    return true;
}

void QSSGLightmapper::run(QOffscreenSurface *fallbackSurface)
{
    auto releaseMainThread = qScopeGuard([&] {
        d->initMutex.lock();
        d->initCondition.wakeAll();
        d->initMutex.unlock();
    });

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info,
                      QStringLiteral("Total models registered: %1").arg(d->bakedLightingModels.size()));

    if (d->bakedLightingModels.isEmpty()) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("No Models to bake"));
        return;
    }

    d->outputPath = stripQrcPrefix(d->options.source);

    if (!createDirectory(d->outputPath)) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Failed to create output directory"));
        return;
    }

    if (!isValidSavePath(d->outputPath)) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed,
                          QStringLiteral("Source path %1 is not a writable location").arg(d->outputPath));
        return;
    }

    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Source path: %1").arg(d->outputPath));
    d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, QStringLiteral("Output path: %1").arg(d->outputPath));

    const QRhi::Flags flags = QRhi::EnableTimestamps | QRhi::EnableDebugMarkers;
#if QT_CONFIG(vulkan)
    std::unique_ptr<QVulkanInstance> vulkanInstance; // Needs to live until rhi goes out of scope
#endif
    std::unique_ptr<QRhi> rhi;

    switch (d->rhiBackend) {
    case QRhi::Vulkan: {
#if QT_CONFIG(vulkan)
        vulkanInstance = std::make_unique<QVulkanInstance>();
        vulkanInstance->create();
        QRhiVulkanInitParams params;
        params.inst = vulkanInstance.get();
        rhi = std::unique_ptr<QRhi>(QRhi::create(d->rhiBackend, &params, flags));
#endif
        break;
    }
    case QRhi::OpenGLES2: {
#if QT_CONFIG(opengl)
        QRhiGles2InitParams params;
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
            // OpenGL 4.3 or higher
            params.format.setProfile(QSurfaceFormat::CoreProfile);
            params.format.setVersion(4, 3);
        } else {
            // OpenGL ES 3.1 or higher
            params.format.setVersion(3, 1);
        }
        params.fallbackSurface = fallbackSurface;
        rhi = std::unique_ptr<QRhi>(QRhi::create(d->rhiBackend, &params, flags));
#endif
        break;
    }
    case QRhi::D3D11: {
#if defined(Q_OS_WIN)
        QRhiD3D11InitParams params;
        rhi = std::unique_ptr<QRhi>(QRhi::create(d->rhiBackend, &params, flags));
#endif
        break;
    }
    case QRhi::D3D12: {
#if defined(Q_OS_WIN)
        QRhiD3D12InitParams params;
        rhi = std::unique_ptr<QRhi>(QRhi::create(d->rhiBackend, &params, flags));
#endif
        break;
    }
    case QRhi::Metal: {
#if QT_CONFIG(metal)
        QRhiMetalInitParams params;
        rhi = std::unique_ptr<QRhi>(QRhi::create(d->rhiBackend, &params, flags));
#endif
        break;
    }
    case QRhi::Null:
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("QRhi backend is null"));
        return;
    default:
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Failed, QStringLiteral("Failed to initialize QRhi"));
        return;
    }

    if (!rhi) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Failed to create QRhi, cannot bake"));
        return;
    }

    if (!rhi->isTextureFormatSupported(QRhiTexture::RGBA32F)) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("FP32 textures not supported, cannot bake"));
        return;
    }
    if (rhi->resourceLimit(QRhi::MaxColorAttachments) < 4) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Multiple render targets not supported, cannot bake"));
        return;
    }
    if (!rhi->isFeatureSupported(QRhi::NonFillPolygonMode)) {
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Warning, QStringLiteral("Line polygon mode not supported, cannot bake"));
        return;
    }

    if (!rhi->isFeatureSupported(QRhi::Compute)) {
        qFatal("Compute is not supported, cannot bake");
        return;
    }

    d->rhiCtxInterface = std::
            unique_ptr<QSSGRenderContextInterface>(new QSSGRenderContextInterface(rhi.get()));
    d->renderer = std::unique_ptr<QSSGRenderer>(new QSSGRenderer());

    QSSGRendererPrivate::setRenderContextInterface(*d->renderer, d->rhiCtxInterface.get());

    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    QSSGRhiContext *rhiCtx = d->rhiCtxInterface->rhiContext().get();
    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    rhiCtxD->setCommandBuffer(cb);

    d->rhiCtxInterface->bufferManager()->setRenderContextInterface(d->rhiCtxInterface.get());

    constexpr int timerIntervalMs = 100;
    TimerThread timerThread;
    timerThread.setInterval(timerIntervalMs);
    // Log ETA every 5 seconds to console
    constexpr int consoleOutputInterval = 5000 / timerIntervalMs;
    int timeoutsSinceOutput = consoleOutputInterval - 1;
    timerThread.setCallback([&]() {
        d->totalProgress = d->progressTracker.getProgress();
        d->estimatedTimeRemaining = d->progressTracker.getEstimatedTimeRemaining();
        bool outputToConsole = timeoutsSinceOutput == consoleOutputInterval - 1;
        d->sendOutputInfo(QSSGLightmapper::BakingStatus::Info, std::nullopt, outputToConsole, outputToConsole);
        timeoutsSinceOutput = (timeoutsSinceOutput + 1) % consoleOutputInterval;
    });
    timerThread.start();

    if (d->denoiseOnly) {
        denoise();
    } else {
        bake();
    }

    rhi->endOffscreenFrame();
    rhi->finish();

    d->renderer.reset();
    d->rhiCtxInterface.reset();
}

void QSSGLightmapper::waitForInit()
{
    d->initMutex.lock();
    d->initCondition.wait(&d->initMutex);
    d->initMutex.unlock();
}

#else

QSSGLightmapper::QSSGLightmapper()
{
}

QSSGLightmapper::~QSSGLightmapper()
{
}

void QSSGLightmapper::reset()
{
}

void QSSGLightmapper::setOptions(const QSSGLightmapperOptions &)
{
}

void QSSGLightmapper::setOutputCallback(Callback )
{
}

qsizetype QSSGLightmapper::add(const QSSGBakedLightingModel &)
{
    return 0;
}

void QSSGLightmapper::setRhiBackend(QRhi::Implementation)
{
}

bool QSSGLightmapper::setupLights(const QSSGRenderer &)
{
    return false;
}

void QSSGLightmapper::setDenoiseOnly(bool)
{
}

void QSSGLightmapper::run(QOffscreenSurface *)
{
    qWarning("Qt Quick 3D was built without the lightmapper; cannot bake lightmaps");
}

void QSSGLightmapper::waitForInit()
{
}

bool QSSGLightmapper::bake()
{
    return false;
}

bool QSSGLightmapper::denoise()
{
    return false;
}

#endif // QT_QUICK3D_HAS_LIGHTMAPPER

QT_END_NAMESPACE

#include "qssglightmapper.moc" // Included because of TimerThread (QThread sublcass)
