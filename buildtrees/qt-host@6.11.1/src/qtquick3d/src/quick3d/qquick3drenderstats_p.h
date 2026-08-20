// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDERSTATS_H
#define QQUICK3DRENDERSTATS_H

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

#include <QtQuick3D/qtquick3dglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qelapsedtimer.h>
#include <ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderLayer;
class QQuickItem;

class Q_QUICK3D_EXPORT QQuick3DRenderStats : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(float frameTime READ frameTime NOTIFY frameTimeChanged)
    Q_PROPERTY(float renderTime READ renderTime NOTIFY renderTimeChanged)
    Q_PROPERTY(float renderPrepareTime READ renderPrepareTime NOTIFY renderTimeChanged)
    Q_PROPERTY(float syncTime READ syncTime NOTIFY syncTimeChanged)
    Q_PROPERTY(float maxFrameTime READ maxFrameTime NOTIFY maxFrameTimeChanged)

    Q_PROPERTY(bool extendedDataCollectionEnabled READ extendedDataCollectionEnabled WRITE setExtendedDataCollectionEnabled NOTIFY extendedDataCollectionEnabledChanged)
    Q_PROPERTY(quint64 drawCallCount READ drawCallCount NOTIFY drawCallCountChanged)
    Q_PROPERTY(quint64 drawVertexCount READ drawVertexCount NOTIFY drawVertexCountChanged)
    Q_PROPERTY(quint64 imageDataSize READ imageDataSize NOTIFY imageDataSizeChanged)
    Q_PROPERTY(quint64 meshDataSize READ meshDataSize NOTIFY meshDataSizeChanged)
    Q_PROPERTY(int renderPassCount READ renderPassCount NOTIFY renderPassCountChanged)
    Q_PROPERTY(QString renderPassDetails READ renderPassDetails NOTIFY renderPassDetailsChanged)
    Q_PROPERTY(QString textureDetails READ textureDetails NOTIFY textureDetailsChanged)
    Q_PROPERTY(QString meshDetails READ meshDetails NOTIFY meshDetailsChanged)
    Q_PROPERTY(int pipelineCount READ pipelineCount NOTIFY pipelineCountChanged)
    Q_PROPERTY(qint64 materialGenerationTime READ materialGenerationTime NOTIFY materialGenerationTimeChanged)
    Q_PROPERTY(qint64 effectGenerationTime READ effectGenerationTime NOTIFY effectGenerationTimeChanged)
    Q_PROPERTY(qint64 pipelineCreationTime READ pipelineCreationTime NOTIFY pipelineCreationTimeChanged)
    Q_PROPERTY(quint32 vmemAllocCount READ vmemAllocCount NOTIFY vmemAllocCountChanged)
    Q_PROPERTY(quint64 vmemUsedBytes READ vmemUsedBytes NOTIFY vmemUsedBytesChanged)
    Q_PROPERTY(QString graphicsApiName READ graphicsApiName NOTIFY graphicsApiNameChanged)
    Q_PROPERTY(float lastCompletedGpuTime READ lastCompletedGpuTime NOTIFY lastCompletedGpuTimeChanged)

    QML_NAMED_ELEMENT(RenderStats)
    QML_UNCREATABLE("The type is internal and only intended to be accessed by user through a View3D.")
    QML_ADDED_IN_VERSION(6, 10)

public:
    QQuick3DRenderStats(QObject *parent = nullptr);

    int fps() const;
    float frameTime() const;
    float renderTime() const;
    float renderPrepareTime() const;
    float syncTime() const;
    float maxFrameTime() const;

    void startSync();
    void endSync(bool dump = false);

    void startRender();
    void startRenderPrepare();
    void endRenderPrepare();
    void endRender(bool dump = false);

    void setRhiContext(QSSGRhiContext *ctx, QSSGRenderLayer *layer);

    bool extendedDataCollectionEnabled() const;
    void setExtendedDataCollectionEnabled(bool enable);

    quint64 drawCallCount() const;
    quint64 drawVertexCount() const;
    quint64 imageDataSize() const;
    quint64 meshDataSize() const;
    int renderPassCount() const;
    QString renderPassDetails() const;
    QString textureDetails() const;
    QString meshDetails() const;
    int pipelineCount() const;
    qint64 materialGenerationTime() const;
    qint64 effectGenerationTime() const;
    qint64 pipelineCreationTime() const;
    quint32 vmemAllocCount() const;
    quint64 vmemUsedBytes() const;
    QString graphicsApiName() const;
    float lastCompletedGpuTime() const;

    Q_INVOKABLE void releaseCachedResources();

    void setWindow(QQuickWindow *window);

Q_SIGNALS:
    void fpsChanged();
    void frameTimeChanged();
    void renderTimeChanged();
    void syncTimeChanged();
    void maxFrameTimeChanged();
    void extendedDataCollectionEnabledChanged();
    void drawCallCountChanged();
    void drawVertexCountChanged();
    void imageDataSizeChanged();
    void meshDataSizeChanged();
    void renderPassCountChanged();
    void renderPassDetailsChanged();
    void textureDetailsChanged();
    void meshDetailsChanged();
    void pipelineCountChanged();
    void materialGenerationTimeChanged();
    void effectGenerationTimeChanged();
    void pipelineCreationTimeChanged();
    void vmemAllocCountChanged();
    void vmemUsedBytesChanged();
    void graphicsApiNameChanged();
    void lastCompletedGpuTimeChanged();

private Q_SLOTS:
    void onFrameSwapped();

private:
    float timestamp() const;
    void processRhiContextStats();
    void notifyRhiContextStats();

    QElapsedTimer m_frameTimer;
    int m_frameCount = 0;
    float m_secTimer = 0;
    float m_notifyTimer = 0;
    float m_renderStartTime = 0;
    float m_renderPrepareStartTime = 0;
    float m_syncStartTime = 0;

    float m_internalMaxFrameTime = 0;
    float m_maxFrameTime = 0;

    int m_fps = 0;

    struct Results {
        float frameTime = 0;
        float renderTime = 0;
        float renderPrepareTime = 0;
        float syncTime = 0;
        float lastCompletedGpuTime = 0;
        quint64 drawCallCount = 0;
        quint64 drawVertexCount = 0;
        quint64 imageDataSize = 0;
        quint64 meshDataSize = 0;
        int renderPassCount = 0;
        QString renderPassDetails;
        QString textureDetails;
        QString meshDetails;
        QSet<QRhiTexture *> activeTextures;
        QSet<QSSGRenderMesh *> activeMeshes;
        int pipelineCount = 0;
        qint64 materialGenerationTime = 0;
        qint64 effectGenerationTime = 0;
        QRhiStats rhiStats;
    };

    Results m_results;
    Results m_notifiedResults;
    QSSGRhiContextStats *m_contextStats = nullptr;
    bool m_extendedDataCollectionEnabled = false;
    QSSGRenderLayer *m_layer = nullptr;
    QMetaObject::Connection m_frameSwappedConnection;
    QQuickWindow *m_window = nullptr;
    bool m_renderingThisFrame = false;
    QString m_graphicsApiName;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuick3DRenderStats *)

#endif // QQUICK3DRENDERSTATS_H
