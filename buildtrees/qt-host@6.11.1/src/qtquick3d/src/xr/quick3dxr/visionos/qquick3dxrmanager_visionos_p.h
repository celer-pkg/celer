// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRVISIONOSRENDERMANAGER_P_H
#define QQUICK3DXRVISIONOSRENDERMANAGER_P_H

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>
#include <QObject>
#import <CompositorServices/CompositorServices.h>
#import <Spatial/Spatial.h>
#import <ARKit/ARKit.h>

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

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QQuick3DXrOrigin;
class QQuick3DXrManager;
class QQuick3DViewport;
class QQuick3DXrInputManager;
class QQuick3DXrAnchorManager;

class QQuickRenderControl;
class QQuick3DXrAnimationDriver;
class QRhiTexture;
class QRhiShadingRateMap;
class QRhiRenderPassDescriptor;

class CompositorLayer;

class QMutex;
template <typename T>
class QMutexLocker;

class QQuick3DXrManagerPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DXrManager)
public:
    enum class RenderState {
        Uninitialized,
        Paused,
        Running,
        Invalidated
    };

    enum class ArTrackingState {
        Uninitialized,
        Initialized,
        Running,
        Paused,
        Stopped
    };

    explicit QQuick3DXrManagerPrivate(QQuick3DXrManager &manager);
    ~QQuick3DXrManagerPrivate();

    static QQuick3DXrManagerPrivate *get(QQuick3DXrManager *manager);

    bool initialize();
    bool finalizeGraphics(QRhi *rhi);
    bool isReady() const;

    bool isGraphicsInitialized() const;
    bool setupGraphics(QQuickWindow *window);

    void setupWindow(QQuickWindow *window);

    void teardown();

    void setMultiViewRenderingEnabled(bool enable);
    bool isMultiViewRenderingEnabled() const;

    bool setPassthroughEnabled(bool enable);
    bool isPassthroughEnabled() const { return false; }
    bool supportsPassthrough() const { return false; }

    QtQuick3DXr::ReferenceSpace getReferenceSpace() const;
    void setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace);

    void setDepthSubmissionEnabled(bool enable);
    bool isDepthSubmissionEnabled() const { return true; }

    [[nodiscard]] bool isValid() const { return true; }

    void getDefaultClipDistances(float &nearClip, float &farClip) const;

    void update();
    void processXrEvents();

    void processSpatialEvents(const QJsonObject &events);

    void setSamples(int samples);

    QStringList enabledExtensions() const { return {}; }
    QString runtimeName() const;
    QVersionNumber runtimeVersion() const;

    QString errorString() const;

    void doRenderFrame() { /* Unused */ }

Q_SIGNALS:
    void initialized();

private:
    friend class CompositorLayer;

    static void prepareAnchorManager(QQuick3DXrAnchorManager *anchorManager, ar_data_providers_t dataProviders);
    static void initAnchorManager(QQuick3DXrAnchorManager *anchorManager);
    static void initInputManager(QQuick3DXrInputManager *im);
    [[nodiscard]] bool renderFrameImpl(QMutexLocker<QMutex> &locker, QWaitCondition &waitCondition);

    static void updateCameraImp(simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin, int i);
    static void updateCamera(QQuick3DViewport *xrViewport, simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin, int i);
    static void updateCameraMultiview(QQuick3DViewport *xrViewport, simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin);

    void setupShadingRateMap(QQuickWindow *window, QRhiShadingRateMap *srm);
    // Called from the render thread
    void releaseResources();

    QQuick3DXrManager *q_ptr = nullptr;
    CompositorLayer *m_compositorLayer = nullptr;
    QRhiTexture *m_rhiDepthTexture = nullptr;
    std::array<QRhiShadingRateMap *, 2> m_srm { nullptr, nullptr };
    QRhiRenderPassDescriptor *m_srmRenderPassDesc = nullptr;
    QThread *m_renderThread = nullptr;
    QPointer<QQuick3DXrInputManager> m_inputManager;
    QPointer<QQuick3DXrAnchorManager> m_anchorManager;
    qint64 m_previousTime = 0;
    qint64 m_nextStepSize = 0;
    bool m_isGraphicsInitialized = false;
    bool m_multiviewRenderingEnabled = true;
    bool m_running = false;
    bool m_arRunning = false;
    bool m_syncDone = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRVISIONOSRENDERMANAGER_P_H
