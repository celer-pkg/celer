// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderstats_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickitem.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype RenderStats
    \inqmlmodule QtQuick3D
    \brief Provides information of the scene rendering.

    The RenderStats type provides information about scene rendering statistics. This
    cannot be created directly, but can be retrieved from a \l View3D.

    Use the \l DebugView item to display the data on-screen.
*/

QQuick3DRenderStats::QQuick3DRenderStats(QObject *parent)
    : QObject(parent)
{
    m_frameTimer.start();
}

/*!
    \qmlproperty int QtQuick3D::RenderStats::fps
    \readonly

    This property holds the number of frames rendered during the last second.
*/
int QQuick3DRenderStats::fps() const
{
    return m_fps;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::frameTime
    \readonly

    This property holds the amount of time elapsed since the last frame, in
    milliseconds.
*/
float QQuick3DRenderStats::frameTime() const
{
    return m_results.frameTime;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::renderTime
    \readonly

    This property holds the amount of time spent on generating a new frame,
    including both the preparation phase and the recording of draw calls. The
    value is in milliseconds.
*/
float QQuick3DRenderStats::renderTime() const
{
    return m_results.renderTime;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::renderPrepareTime
    \readonly

    This property holds the amount of time spent in the preparation phase of
    rendering, in milliseconds. This is a subset of the total render time
    reported in renderTime.
*/
float QQuick3DRenderStats::renderPrepareTime() const
{
    return m_results.renderPrepareTime;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::syncTime
    \readonly

    This property holds the amount of time spent inside the sync function, in
    milliseconds. The property values of the objects are updated during the
    sync.
*/
float QQuick3DRenderStats::syncTime() const
{
    return m_results.syncTime;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::maxFrameTime
    \readonly

    This property holds the maximum time spent rendering a single frame during
    the last second.
*/
float QQuick3DRenderStats::maxFrameTime() const
{
    return m_maxFrameTime;
}

float QQuick3DRenderStats::timestamp() const
{
    return m_frameTimer.nsecsElapsed() / 1000000.0f;
}

void QQuick3DRenderStats::startSync()
{
    m_syncStartTime = timestamp();
}

void QQuick3DRenderStats::endSync(bool dump)
{
    m_results.syncTime = timestamp() - m_syncStartTime;

    if (dump)
        qDebug("Sync took: %f ms", m_results.syncTime);
}

void QQuick3DRenderStats::startRender()
{
    m_renderStartTime = timestamp();
}

void QQuick3DRenderStats::startRenderPrepare()
{
    m_renderPrepareStartTime = timestamp();
}

void QQuick3DRenderStats::endRenderPrepare()
{
    m_results.renderPrepareTime = timestamp() - m_renderPrepareStartTime;
}

void QQuick3DRenderStats::endRender(bool dump)
{
    // Threading-wise this and onFrameSwapped are not perfect. These are called
    // on the render thread (if there is one) outside of the sync step, so
    // writing the data in m_results, which then may be read by the properties
    // on the main thread concurrently, is not ideal. But at least the data the
    // results are generated from (the m_* timings and all the stuff from
    // QSSGRhiContextStats) belong to the render thread, so that's good.

    m_renderingThisFrame = true;
    const float endTime = timestamp();
    m_results.renderTime = endTime - m_renderStartTime;

    if (dump)
        qDebug("Render took: %f ms (of which prep: %f ms)", m_results.renderTime, m_results.renderPrepareTime);
}

void QQuick3DRenderStats::onFrameSwapped()
{
    // NOTE: This is called on the render thread
    // This is the real start and end of a frame

    if (m_renderingThisFrame) {
        ++m_frameCount;
        m_results.frameTime = timestamp();
        m_internalMaxFrameTime = qMax(m_results.frameTime, m_internalMaxFrameTime);

        m_secTimer += m_results.frameTime;
        m_notifyTimer += m_results.frameTime;

        m_results.renderTime = m_results.frameTime - m_renderStartTime;

        processRhiContextStats();

        if (m_window) {
            QRhiSwapChain *sc = m_window->swapChain();
            if (sc) {
                QRhiCommandBuffer *cb = sc->currentFrameCommandBuffer();
                if (cb) {
                    const float msecs = float(cb->lastCompletedGpuTime() * 1000.0);
                    if (!qFuzzyIsNull(msecs))
                        m_results.lastCompletedGpuTime = msecs;
                }
            }
        }

        const float notifyInterval = 200.0f;
        if (m_notifyTimer >= notifyInterval) {
            m_notifyTimer -= notifyInterval;

            if (m_results.frameTime != m_notifiedResults.frameTime) {
                m_notifiedResults.frameTime = m_results.frameTime;
                emit frameTimeChanged();
            }

            if (m_results.syncTime != m_notifiedResults.syncTime) {
                m_notifiedResults.syncTime = m_results.syncTime;
                emit syncTimeChanged();
            }

            if (m_results.renderTime != m_notifiedResults.renderTime) {
                m_notifiedResults.renderTime = m_results.renderTime;
                m_notifiedResults.renderPrepareTime = m_results.renderPrepareTime;
                emit renderTimeChanged();
            }

            if (m_results.lastCompletedGpuTime != m_notifiedResults.lastCompletedGpuTime) {
                m_notifiedResults.lastCompletedGpuTime = m_results.lastCompletedGpuTime;
                emit lastCompletedGpuTimeChanged();
            }

            notifyRhiContextStats();
        }

        const float fpsInterval = 1000.0f;
        if (m_secTimer >= fpsInterval) {
            m_secTimer -= fpsInterval;

            m_fps = m_frameCount;
            m_frameCount = 0;
            emit fpsChanged();

            m_maxFrameTime = m_internalMaxFrameTime;
            m_internalMaxFrameTime = 0;
            emit maxFrameTimeChanged();
        }

        m_renderingThisFrame = false; // reset for next frame
    }

    // Always reset the frame timer
    m_frameTimer.start();
}

void QQuick3DRenderStats::setRhiContext(QSSGRhiContext *ctx, QSSGRenderLayer *layer)
{
    // called from synchronize(), so on the render thread with gui blocked

    m_layer = layer;
    m_contextStats = &QSSGRhiContextStats::get(*ctx);

    // setExtendedDataCollectionEnabled will likely get called at some point
    // before this (so too early), sync the flag here as well now that we know
    // all we need to know.
    if (m_extendedDataCollectionEnabled)
        m_contextStats->dynamicDataSources.insert(layer);

    if (m_contextStats && m_contextStats->rhiCtx->rhi()) {
        const QString backendName = QString::fromUtf8(m_contextStats->rhiCtx->rhi()->backendName());
        if (m_graphicsApiName != backendName) {
            m_graphicsApiName = backendName;
            emit graphicsApiNameChanged();
        }
    }
}

void QQuick3DRenderStats::setWindow(QQuickWindow *window)
{
    if (m_window == window)
        return;

    if (m_window)
        disconnect(m_frameSwappedConnection);

    m_window = window;

    if (m_window) {
        m_frameSwappedConnection = connect(m_window, &QQuickWindow::afterFrameEnd,
                                           this, &QQuick3DRenderStats::onFrameSwapped,
                                           Qt::DirectConnection);
    }
}

/*!
    \qmlproperty bool QtQuick3D::RenderStats::extendedDataCollectionEnabled

    This property controls if render pass and draw call statistics are
    processed and made available. This may incur a small performance cost and
    is therefore optional.

    Properties such as drawCallCount, drawVertexCount, or renderPassCount are
    updated only when this property is set to true.

    The default value is false.

    \note Changing the visibility of a \l DebugView associated with the \l
    View3D automatically toggles the value based on the DebugView's
    \l{QQuickItem::visible}{visible} property.

    \since 6.5
 */
bool QQuick3DRenderStats::extendedDataCollectionEnabled() const
{
    return m_extendedDataCollectionEnabled;
}

void QQuick3DRenderStats::setExtendedDataCollectionEnabled(bool enable)
{
    if (enable != m_extendedDataCollectionEnabled) {
        m_extendedDataCollectionEnabled = enable;
        emit extendedDataCollectionEnabledChanged();
    }
    if (m_contextStats) {
        // This is what allows recognizing that there is at least one DebugView
        // that is visible and wants all the data, and also helps in not
        // performing all the processing if the set is empty (because then we
        // know that no DebugView wants to display the data)
        if (m_extendedDataCollectionEnabled)
            m_contextStats->dynamicDataSources.insert(m_layer);
        else
            m_contextStats->dynamicDataSources.remove(m_layer);
    }
}

static const char *textureFormatStr(QRhiTexture::Format format)
{
    switch (format) {
    case QRhiTexture::RGBA8:
        return "RGBA8";
    case QRhiTexture::BGRA8:
        return "BGRA8";
    case QRhiTexture::R8:
        return "R8";
    case QRhiTexture::RG8:
        return "RG8";
    case QRhiTexture::R16:
        return "R16";
    case QRhiTexture::RG16:
        return "RG16";
    case QRhiTexture::RED_OR_ALPHA8:
        return "R8/A8";
    case QRhiTexture::RGBA16F:
        return "RGBA16F";
    case QRhiTexture::RGBA32F:
        return "RGBA32F";
    case QRhiTexture::R16F:
        return "R16F";
    case QRhiTexture::R32F:
        return "R32F";
    case QRhiTexture::RGB10A2:
        return "RGB10A2";
    case QRhiTexture::D16:
        return "D16";
    case QRhiTexture::D24:
        return "D24";
    case QRhiTexture::D24S8:
        return "D24S8";
    case QRhiTexture::D32F:
        return "D32F";
    case QRhiTexture::BC1:
        return "BC1";
    case QRhiTexture::BC2:
        return "BC2";
    case QRhiTexture::BC3:
        return "BC3";
    case QRhiTexture::BC4:
        return "BC4";
    case QRhiTexture::BC5:
        return "BC5";
    case QRhiTexture::BC6H:
        return "BC6H";
    case QRhiTexture::BC7:
        return "BC7";
    case QRhiTexture::ETC2_RGB8:
        return "ETC2_RGB8";
    case QRhiTexture::ETC2_RGB8A1:
        return "ETC2_RGB8A1";
    case QRhiTexture::ETC2_RGBA8:
        return "ETC2_RGBA8";
    case QRhiTexture::ASTC_4x4:
        return "ASTC_4x4";
    case QRhiTexture::ASTC_5x4:
        return "ASTC_5x4";
    case QRhiTexture::ASTC_5x5:
        return "ASTC_5x5";
    case QRhiTexture::ASTC_6x5:
        return "ASTC_6x5";
    case QRhiTexture::ASTC_6x6:
        return "ASTC_6x6";
    case QRhiTexture::ASTC_8x5:
        return "ASTC_8x5";
    case QRhiTexture::ASTC_8x6:
        return "ASTC_8x6";
    case QRhiTexture::ASTC_8x8:
        return "ASTC_8x8";
    case QRhiTexture::ASTC_10x5:
        return "ASTC_10x5";
    case QRhiTexture::ASTC_10x6:
        return "ASTC_10x6";
    case QRhiTexture::ASTC_10x8:
        return "ASTC_10x8";
    case QRhiTexture::ASTC_10x10:
        return "ASTC_10x10";
    case QRhiTexture::ASTC_12x10:
        return "ASTC_12x10";
    case QRhiTexture::ASTC_12x12:
        return "ASTC_12x12";
    default:
        break;
    }
    return "<unknown>";
}

static inline void printRenderPassDetails(QString *dst, const QSSGRhiContextStats::RenderPassInfo &rp)
{
    *dst += QString::asprintf("| %s | %dx%d | %llu | %llu |\n",
                              rp.rtName.constData(),
                              rp.pixelSize.width(),
                              rp.pixelSize.height(),
                              QSSGRhiContextStats::totalVertexCountForPass(rp),
                              QSSGRhiContextStats::totalDrawCallCountForPass(rp));
}

static inline QByteArray nameForRenderMesh(const QSSGRenderMesh *mesh)
{
    if (!mesh->subsets.isEmpty()) {
        auto buf = mesh->subsets[0].rhi.vertexBuffer;
        if (buf)
            return buf->buffer()->name();
    }
    return {};
}

void QQuick3DRenderStats::processRhiContextStats()
{
    if (!m_contextStats || !m_extendedDataCollectionEnabled)
        return;

    // the render pass list is per renderer, i.e. per View3D
    const QSSGRhiContextStats::PerLayerInfo data = m_contextStats->perLayerInfo[m_layer];

    const QSSGRhiContext *rhiCtx = m_contextStats->rhiCtx;
    const QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);

    // textures and meshes include all assets registered to the per-QQuickWindow QSSGRhiContext
    const QSSGRhiContextStats::GlobalInfo globalData = m_contextStats->globalInfo;
    const auto textures = rhiCtxD->m_textures;
    const auto meshes = rhiCtxD->m_meshes;
    const auto pipelines = rhiCtxD->m_pipelines;

    m_results.drawCallCount = 0;
    m_results.drawVertexCount = 0;
    for (const auto &pass : data.renderPasses) {
        m_results.drawCallCount += QSSGRhiContextStats::totalDrawCallCountForPass(pass);
        m_results.drawVertexCount += QSSGRhiContextStats::totalVertexCountForPass(pass);
    }
    m_results.drawCallCount += QSSGRhiContextStats::totalDrawCallCountForPass(data.externalRenderPass);
    m_results.drawVertexCount += QSSGRhiContextStats::totalVertexCountForPass(data.externalRenderPass);

    m_results.imageDataSize = globalData.imageDataSize;
    m_results.meshDataSize = globalData.meshDataSize;

    m_results.renderPassCount = data.renderPasses.size()
            + (data.externalRenderPass.pixelSize.isEmpty() ? 0 : 1);

    QString renderPassDetails = QLatin1String(R"(
| Name | Size | Vertices | Draw calls |
| ---- | ---- | -------- | ---------- |
)");

    if (!data.externalRenderPass.pixelSize.isEmpty())
        printRenderPassDetails(&renderPassDetails, data.externalRenderPass);
    for (const auto &pass : data.renderPasses) {
        if (!pass.pixelSize.isEmpty())
            printRenderPassDetails(&renderPassDetails, pass);
    }
    renderPassDetails += QString::asprintf("\nGenerated from QSSGRenderLayer %p", m_layer);
    m_results.renderPassDetails = renderPassDetails;

    if (m_results.activeTextures != textures) {
        m_results.activeTextures = textures;
        QString texDetails = QLatin1String(R"(
| Name | Size | Format | Mip | Flags |
| ---- | ---- | ------ | --- | ----- |
)");
        QList<QRhiTexture *> textureList = textures.values();
        std::sort(textureList.begin(), textureList.end(), [](QRhiTexture *a, QRhiTexture *b) {
            return a->name() < b->name();
        });
        for (QRhiTexture *tex : textureList) {
            int mipCount = 1;
            const QRhiTexture::Flags flags = tex->flags();
            if (flags.testFlag(QRhiTexture::MipMapped))
                mipCount = m_contextStats->rhiCtx->rhi()->mipLevelsForSize(tex->pixelSize());
            QByteArray flagMsg;
            if (flags.testFlag(QRhiTexture::CubeMap))
                flagMsg += QByteArrayLiteral("[cube]");
            texDetails += QString::asprintf("| %s | %dx%d | %s | %d | %s |\n",
                                            tex->name().constData(),
                                            tex->pixelSize().width(),
                                            tex->pixelSize().height(),
                                            textureFormatStr(tex->format()),
                                            mipCount,
                                            flagMsg.constData());
        }
        texDetails += QString::asprintf("\nAsset textures registered with QSSGRhiContext %p", m_contextStats->rhiCtx);
        m_results.textureDetails = texDetails;
    }

    if (m_results.activeMeshes != meshes) {
        m_results.activeMeshes = meshes;
        QString meshDetails = QLatin1String(R"(
| Name | Submeshes | Vertices | V.buf size | I.buf size |
| ---- | --------- | -------- | ---------- | ---------- |
)");
        QList<QSSGRenderMesh *> meshList = meshes.values();
        std::sort(meshList.begin(), meshList.end(), [](QSSGRenderMesh *a, QSSGRenderMesh *b) {
            return nameForRenderMesh(a) < nameForRenderMesh(b);
        });
        for (QSSGRenderMesh *mesh : meshList) {
            const QByteArray name = nameForRenderMesh(mesh);
            const int subsetCount = int(mesh->subsets.size());
            quint64 vertexCount = 0;
            quint32 vbufSize = 0;
            quint32 ibufSize = 0;
            if (subsetCount > 0) {
                for (const QSSGRenderSubset &subset : std::as_const(mesh->subsets))
                    vertexCount += subset.count;
                // submeshes ref into the same vertex and index buffer
                const QSSGRhiBuffer *vbuf = mesh->subsets[0].rhi.vertexBuffer.get();
                if (vbuf)
                    vbufSize = vbuf->buffer()->size();
                const QSSGRhiBuffer *ibuf = mesh->subsets[0].rhi.indexBuffer.get();
                if (ibuf)
                    ibufSize = ibuf->buffer()->size();
            }
            meshDetails += QString::asprintf("| %s | %d | %llu | %u | %u |\n",
                                             name.constData(),
                                             subsetCount,
                                             vertexCount,
                                             vbufSize,
                                             ibufSize);

        }
        meshDetails += QString::asprintf("\nAsset meshes registered with QSSGRhiContext %p", m_contextStats->rhiCtx);
        m_results.meshDetails = meshDetails;
    }

    m_results.pipelineCount = pipelines.count();

    m_results.materialGenerationTime = m_contextStats->globalInfo.materialGenerationTime;
    m_results.effectGenerationTime = m_contextStats->globalInfo.effectGenerationTime;

    m_results.rhiStats = m_contextStats->rhiCtx->rhi()->statistics();
}

void QQuick3DRenderStats::notifyRhiContextStats()
{
    if (!m_contextStats || !m_extendedDataCollectionEnabled)
        return;

    if (m_results.drawCallCount != m_notifiedResults.drawCallCount) {
        m_notifiedResults.drawCallCount = m_results.drawCallCount;
        emit drawCallCountChanged();
    }

    if (m_results.drawVertexCount != m_notifiedResults.drawVertexCount) {
        m_notifiedResults.drawVertexCount = m_results.drawVertexCount;
        emit drawVertexCountChanged();
    }

    if (m_results.imageDataSize != m_notifiedResults.imageDataSize) {
        m_notifiedResults.imageDataSize = m_results.imageDataSize;
        emit imageDataSizeChanged();
    }

    if (m_results.meshDataSize != m_notifiedResults.meshDataSize) {
        m_notifiedResults.meshDataSize = m_results.meshDataSize;
        emit meshDataSizeChanged();
    }

    if (m_results.renderPassCount != m_notifiedResults.renderPassCount) {
        m_notifiedResults.renderPassCount = m_results.renderPassCount;
        emit renderPassCountChanged();
    }

    if (m_results.renderPassDetails != m_notifiedResults.renderPassDetails) {
        m_notifiedResults.renderPassDetails = m_results.renderPassDetails;
        emit renderPassDetailsChanged();
    }

    if (m_results.textureDetails != m_notifiedResults.textureDetails) {
        m_notifiedResults.textureDetails = m_results.textureDetails;
        emit textureDetailsChanged();
    }

    if (m_results.meshDetails != m_notifiedResults.meshDetails) {
        m_notifiedResults.meshDetails = m_results.meshDetails;
        emit meshDetailsChanged();
    }

    if (m_results.pipelineCount != m_notifiedResults.pipelineCount) {
        m_notifiedResults.pipelineCount = m_results.pipelineCount;
        emit pipelineCountChanged();
    }

    if (m_results.materialGenerationTime != m_notifiedResults.materialGenerationTime) {
        m_notifiedResults.materialGenerationTime = m_results.materialGenerationTime;
        emit materialGenerationTimeChanged();
    }

    if (m_results.effectGenerationTime != m_notifiedResults.effectGenerationTime) {
        m_notifiedResults.effectGenerationTime = m_results.effectGenerationTime;
        emit effectGenerationTimeChanged();
    }

    if (m_results.rhiStats.totalPipelineCreationTime != m_notifiedResults.rhiStats.totalPipelineCreationTime) {
        m_notifiedResults.rhiStats.totalPipelineCreationTime = m_results.rhiStats.totalPipelineCreationTime;
        emit pipelineCreationTimeChanged();
    }

    if (m_results.rhiStats.allocCount != m_notifiedResults.rhiStats.allocCount) {
        m_notifiedResults.rhiStats.allocCount = m_results.rhiStats.allocCount;
        emit vmemAllocCountChanged();
    }

    if (m_results.rhiStats.usedBytes != m_notifiedResults.rhiStats.usedBytes) {
        m_notifiedResults.rhiStats.usedBytes = m_results.rhiStats.usedBytes;
        emit vmemUsedBytesChanged();
    }
}

/*!
    \qmlproperty quint64 QtQuick3D::RenderStats::drawCallCount
    \readonly

    This property holds the total number of draw calls (including non-indexed,
    indexed, instanced, and instanced indexed draw calls) that were registered
    during the last render of the \l View3D.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \since 6.5
*/
quint64 QQuick3DRenderStats::drawCallCount() const
{
    return m_results.drawCallCount;
}

/*!
    \qmlproperty quint64 QtQuick3D::RenderStats::drawVertexCount
    \readonly

    This property holds the total number of vertices in draw calls that were
    registered during the last render of the \l View3D.

    The value includes the number of vertex and index count from draw calls
    that were registered during the last render of the \l View3D. While the
    number is not guaranteed to be totally accurate, it is expected to give a
    good indication of the complexity of the scene rendering.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \since 6.5
*/
quint64 QQuick3DRenderStats::drawVertexCount() const
{
    return m_results.drawVertexCount;
}

/*!
    \qmlproperty quint64 QtQuick3D::RenderStats::imageDataSize
    \readonly

    This property holds the approximate size in bytes of the image data for
    texture maps currently registered with the View3D's window. The value is
    per-window, meaning if there are multiple View3D objects within the same
    QQuickWindow, those will likely report the same value.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
quint64 QQuick3DRenderStats::imageDataSize() const
{
    return m_results.imageDataSize;
}

/*!
    \qmlproperty quint64 QtQuick3D::RenderStats::meshDataSize
    \readonly

    This property holds the approximate size in bytes of the mesh data
    currently registered with the View3D's window. The value is per-window,
    meaning if there are multiple View3D objects within the same QQuickWindow,
    those will likely report the same value.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
quint64 QQuick3DRenderStats::meshDataSize() const
{
    return m_results.meshDataSize;
}

/*!
    \qmlproperty int QtQuick3D::RenderStats::renderPassCount
    \readonly

    This property holds the total number of render passes that were registered
    during the last render of the \l View3D.

    Many features, such as realtime shadow mapping, postprocessing effects, the
    depth and screen textures, and certain antialiasing methods involve
    multiple additional render passes. While the number is not guaranteed to
    include absolutely all render passes, it is expected to give a good
    indication of the complexity of the scene rendering.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \since 6.5
*/
int QQuick3DRenderStats::renderPassCount() const
{
    return m_results.renderPassCount;
}

/*!
    \qmlproperty string QtQuick3D::RenderStats::renderPassDetails
    \readonly
    \internal
    \since 6.5
*/
QString QQuick3DRenderStats::renderPassDetails() const
{
    return m_results.renderPassDetails;
}

/*!
    \qmlproperty string QtQuick3D::RenderStats::textureDetails
    \readonly
    \internal
    \since 6.5
*/
QString QQuick3DRenderStats::textureDetails() const
{
    return m_results.textureDetails;
}

/*!
    \qmlproperty string QtQuick3D::RenderStats::meshDetails
    \readonly
    \internal
    \since 6.5
*/
QString QQuick3DRenderStats::meshDetails() const
{
    return m_results.meshDetails;
}

/*!
    \qmlproperty int QtQuick3D::RenderStats::pipelineCount
    \readonly

    This property holds the total number of cached graphics pipelines for the
    window the \l View3D belongs to.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
int QQuick3DRenderStats::pipelineCount() const
{
    return m_results.pipelineCount;
}

/*!
    \qmlproperty qint64 QtQuick3D::RenderStats::materialGenerationTime
    \readonly

    This property holds the total number of milliseconds spent on generating
    and processing shader code for \l DefaultMaterial, \l PrincipledMaterial,
    and \l CustomMaterial in the window the \l View3D belongs to.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
qint64 QQuick3DRenderStats::materialGenerationTime() const
{
    return m_results.materialGenerationTime;
}

/*!
    \qmlproperty qint64 QtQuick3D::RenderStats::effectGenerationTime
    \readonly

    This property holds the total number of milliseconds spent on generating
    and processing shader code for post-processing effects in the window the \l
    View3D belongs to.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
qint64 QQuick3DRenderStats::effectGenerationTime() const
{
    return m_results.effectGenerationTime;
}

/*!
    \qmlproperty qint64 QtQuick3D::RenderStats::pipelineCreationTime
    \readonly

    This property holds the total number of milliseconds spent on creating
    graphics pipelines on the rendering hardware interface level. This can
    include, among other things: compilation times for compiling HLSL to an
    intermediate format, compiling MSL, compiling GLSL code with
    glCompileShader or linking using program binaries, and generating Vulkan
    pipelines with all that entails (e.g. SPIR-V -> ISA compilation). The value
    reflects all Qt Quick and Qt Quick 3D rendering in the window the \l View3D
    belongs to.

    \note The value includes operations that are under Qt's control. Depending
    on the underlying graphics API, some pipeline (shader, graphics state)
    related operations may happen asynchronously, and may be affected by
    caching on various levels in the graphics stack. Releasing cached resource
    by calling QQuickWindow::releaseResources() or clicking the corresponding
    DebugView button may also have varying results, depending on the underlying
    details (rhi backend, graphics API); it may or may not affect this counter
    due to a varying number of factors.

    This timing is provided as a general, high level indication. Combined with
    \l materialGenerationTime, application developers can use these values to
    confirm that the time spent on material and graphics pipeline processing is
    reasonably low during the normal use of the application, once all caches
    (both persistent and in-memory) are warm. Avoid drawing conclusions from
    the first run of the application. (since that may not benefit from
    persistent, disk-based caches yet)

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
qint64 QQuick3DRenderStats::pipelineCreationTime() const
{
    return m_results.rhiStats.totalPipelineCreationTime;
}

/*!
    \qmlproperty quint32 QtQuick3D::RenderStats::vmemAllocCount
    \readonly

    When applicable, the number of allocations made by the graphics memory
    allocator library. This includes allocations from all Qt Quick and Qt Quick
    3D rendering in the QQuickWindow to which the \l View3D belongs. The value
    is zero with graphics APIs such as OpenGL, Direct3D, and Metal because
    memory allocation is not under Qt's control then.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
quint32 QQuick3DRenderStats::vmemAllocCount() const
{
    return m_results.rhiStats.allocCount;
}

/*!
    \qmlproperty quint64 QtQuick3D::RenderStats::vmemUsedBytes
    \readonly

    When applicable, the number of bytes used by allocations made by the
    graphics memory allocator library. This includes allocations from all Qt
    Quick and Qt Quick 3D rendering in the QQuickWindow to which the \l View3D
    belongs. The value is zero with graphics APIs such as OpenGL, Direct3D, and
    Metal because memory allocation is not under Qt's control then.

    The value is updated only when extendedDataCollectionEnabled is enabled.

    \note The value is reported on a per-QQuickWindow basis. If there are
    multiple View3D instances within the same window, the DebugView shows the
    same value for all those View3Ds.

    \since 6.5
*/
quint64 QQuick3DRenderStats::vmemUsedBytes() const
{
    return m_results.rhiStats.usedBytes;
}

/*!
    \qmlproperty string QtQuick3D::RenderStats::graphicsAPIName
    \readonly

    This property holds the name of the current graphics API (RHI) backend
    currently in use.

    \since 6.5
*/
QString QQuick3DRenderStats::graphicsApiName() const
{
    return m_graphicsApiName;
}

/*!
    \qmlproperty real QtQuick3D::RenderStats::lastCompletedGpuTime
    \readonly

    When GPU timing collection is
    \l{QQuickGraphicsConfiguration::setTimestamps()}{enabled in Qt Quick}, and
    the relevant features are supported by the underlying graphics API, this
    property contains the last retrieved elapsed GPU time in milliseconds.

    \note The value is retrieved asynchronously, and usually refers to a frame
    older than the previous one, meaning that the value is not necessarily in
    sync with the other, CPU-side timings.

    \note The result is based on the rendering of the entire contents of the
    QQuickWindow the View3D belongs to. It includes all the contents of Qt
    Quick scene, including all 2D elements and all View3D items within that
    window.

    \since 6.6

    \sa QQuickGraphicsConfiguration::setTimestamps()
*/
float QQuick3DRenderStats::lastCompletedGpuTime() const
{
    return m_results.lastCompletedGpuTime;
}

/*!
    \internal
 */
void QQuick3DRenderStats::releaseCachedResources()
{
    if (m_window)
        m_window->releaseResources();
    else
        qWarning("QQuick3DRenderStats: No window, cannot request releasing cached resources");
}

QT_END_NAMESPACE
