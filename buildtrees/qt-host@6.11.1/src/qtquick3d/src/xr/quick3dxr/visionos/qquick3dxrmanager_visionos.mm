// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qquick3dxrmanager_visionos_p.h"
#include "qquick3dxrorigin_p.h"
#include "qquick3dxrmanager_p.h"
#include "qquick3dxrinputmanager_visionos_p.h"
#include "qquick3dxranchormanager_visionos_p.h"

#include "../qquick3dxrinputmanager_p.h"
#include "../qquick3dxranimationdriver_p.h"

#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dnode_p_p.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>

#include <QQuickGraphicsDevice>
#include <rhi/qrhi.h>

#include <CompositorServices/CompositorServices.h>
#include <QtGui/qguiapplication_platform.h>

#include <QtCore/qoperatingsystemversion.h>
#include <QtCore/qloggingcategory.h>

#include <TargetConditionals.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

static const char s_renderThreadName[] = "QQuick3DXrRenderThread";

// NOTE: This only affects visionOS
static bool qssgDisableFoveation()
{
    static bool foveationDisabled = qEnvironmentVariableIntValue("QT_QUICK3D_XR_DISABLE_FOVEATION") != 0;
    return foveationDisabled;
}

class CompositorLayer : public QObject, public QNativeInterface::QVisionOSApplication::ImmersiveSpaceCompositorLayer
{
    Q_OBJECT
public:
    CompositorLayer() = default;
    ~CompositorLayer() final
    {
        m_xrManager = nullptr;
        [m_layerRenderer release];
        m_layerRenderer = nullptr;
        m_worldTrackingProvider = nullptr;
        m_arSession = nullptr;
        m_initialized = false;
    }

    using EventT = std::underlying_type_t<QEvent::Type>;
    enum Event : EventT
    {
        Render = QEvent::User + 1,
        Pause,
        Stop,
        Pulse
    };

    static constexpr QEvent::Type asQEvent(CompositorLayer::Event event) { return static_cast<QEvent::Type>(event); }

    static bool supportsLayoutType(cp_layer_renderer_capabilities_t capabilities, cp_layer_renderer_layout layout)
    {
        const size_t layoutCount = cp_layer_renderer_capabilities_supported_layouts_count(capabilities, cp_supported_layouts_options_none);
        bool found = false;
        for (size_t i = 0; i < layoutCount && !found; ++i)
            found = (layout == cp_layer_renderer_capabilities_supported_layout(capabilities, cp_supported_layouts_options_none, i));

        return found;
    }

    void configure(cp_layer_renderer_capabilities_t capabilities, cp_layer_renderer_configuration_t configuration) const override
    {
        QMutexLocker locker(&m_compositorLayerMtx);

        m_multiviewEnabled = !QQuick3DXrManager::isMultiviewRenderingDisabled();

        // If multiview isn't disabled we need to check if the target supports it.
        // NOTE: We're only doing layered or dedicated rendering, so shared is not supported (untested).
        m_multiviewSupported = supportsLayoutType(capabilities, cp_layer_renderer_layout_layered);
        m_multiviewEnabled = m_multiviewEnabled && m_multiviewSupported;

        // NOTE: If foviation is supported we enable it.
        m_foveationSupported = cp_layer_renderer_capabilities_supports_foveation(capabilities);
        // ... but we can disable it with an environment variable.
        m_foveationEnabled = m_foveationSupported && !qssgDisableFoveation();

        // NOTE: We're only doing layered or dedicated rendering, so shared is not supported, even though we technically
        // could support it. Since the only target we currently have (visionOS on the Vision Pro) support all these
        // modes anyways we only need to care about these two (dedicated is required for the emulator).
        cp_layer_renderer_layout textureLayout = m_multiviewEnabled ? cp_layer_renderer_layout_layered
                                                                    : cp_layer_renderer_layout_dedicated;

        cp_layer_renderer_configuration_set_layout(configuration, textureLayout);
        cp_layer_renderer_configuration_set_foveation_enabled(configuration, m_foveationEnabled);
        cp_layer_renderer_configuration_set_color_format(configuration, MTLPixelFormatRGBA16Float);
        simd_float2 depthRange = cp_layer_renderer_configuration_get_default_depth_range(configuration);
        // NOTE: The depth range is inverted for VisionOS (x = far, y = near)
        m_depthRange[0] = depthRange.y;
        m_depthRange[1] = depthRange.x;

        qCDebug(lcQuick3DXr) << "Configuring with the following settings:"
                             << "\n\tMultiview supported: " << m_multiviewSupported
                             << "\n\tMultiview enabled: " << m_multiviewEnabled
                             << "\n\tFoveation supported: " << m_foveationSupported
                             << "\n\tFoveation enabled: " << m_foveationEnabled
                             << "\n\tDepth Range: " << m_depthRange[0] << " to: " << m_depthRange[1];
    }

    void render(cp_layer_renderer_t renderer) override
    {
        if (m_layerRenderer != renderer) {
            m_layerRenderer = renderer;
            emit layerRendererChanged();
        }

        if (m_layerRenderer) {
            QMutexLocker locker(&m_mutex);
            // NOTE: The layer renderer is our handle to the compositor layer, so we need to ensure that
            //       we need to ensure that it stays alive long enough for us to know if we should tear down
            //       the compositor layer or not.
            [m_layerRenderer retain];
            checkRenderState();
            emit layerRendererReady();
        }
    }

    // Listen for spatial events (these are native gestures like pinch click/drag coming from SwiftUI)
    void handleSpatialEvents(const QJsonObject &events) override
    {
        if (m_xrManager)
            m_xrManager->processSpatialEvents(events);
    }

    bool isInitialized() const { return m_initialized; }

    bool isMultiviewSupported() const { return m_multiviewSupported; }
    bool isMultiviewEnabled() const { return m_multiviewEnabled; }
    bool isFoveationSupported() const { return m_foveationSupported; }
    bool isFoveationEnabled() const { return m_foveationEnabled; }

    cp_layer_renderer_t layerRenderer() const
    {
        return m_layerRenderer;
    }

    void getDefaultDepthRange(float &near, float &far) const
    {
        near = m_depthRange[0];
        far = m_depthRange[1];
    }

    // Must be called from the GUI thread
    void init(QQuick3DXrManagerPrivate *xrManager)
    {
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        QSSG_ASSERT(!m_initialized, return);

        m_xrManager = xrManager;
        runWorldTrackingARSession();
        m_initialized = true;
    }

    // Called from the gui thread while holding the render lock.
    // For now we only let the thread know that it should stop rendering,
    // as we're going to tear down the compositor layer on the render thread.
    void teardown() { m_teardown = true; }

    void stopArSession()
    {
        Q_ASSERT(qApp->thread() == QThread::currentThread());

        QMutexLocker locker(&m_arSessionMtx);

        if (m_arSession) {
            qCDebug(lcQuick3DXr, "Stopping AR session");
            // NOTE: We're stopping the AR session, so we need to ensure that the AR session
            //       doesn't get posted at some later point, so we change the dispatch queue to
            //       use the main queue (which is the GUI thread) before calling stop.
            ar_session_set_data_provider_state_change_handler_f(m_arSession, dispatch_get_main_queue(), this, &onArStateChanged);
            ar_session_stop(m_arSession);
            m_arSession = nullptr;
            m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Uninitialized;
        }
    }

    QMutex &renderLock() { return m_mutex; }
    bool waitForSyncToComplete()
    {
        return m_waitCondition.wait(&m_mutex);
    }

    Q_INVOKABLE static void destroy(QQuick3DXrManagerPrivate *mngr, QQuickWindow *window, CompositorLayer *compositorLayer)
    {
        QSSG_ASSERT(mngr != nullptr, return);

        qCDebug(lcQuick3DXr) << "Destroying compositor layer";

        if (auto *visionOSApplicaton = qGuiApp->nativeInterface<QNativeInterface::QVisionOSApplication>())
            visionOSApplicaton->setImmersiveSpaceCompositorLayer(nullptr);

        // NOTE: This is a bit of a hack, but we need to cleanup the nodes
        // on the render thread while the GUI thread is blocked (as documented.)
        // Since we cannot destruct the window on the render thread, and we cannot
        // cleanup all nodes on the GUI thread due to dependencies on the render
        // thread and resources created on it. Instead we just invalidate the render
        // control and cleanup the nodes on the render thread. This is not ideal,
        // but it's what we have for now...
        if (window) { // If we never got past the setup we don't have a window.
            auto *d = QQuickWindowPrivate::get(window);
            d->cleanupNodesOnShutdown();
            mngr->releaseResources();
            if (auto *rc = d->renderControl)
                rc->invalidate();
        }

        // We're holding onto the lock, so defer the delete until we're done here.
        // All reasources should be released at this point anyways.
        compositorLayer->deleteLater();
    }

Q_SIGNALS:
    void layerRendererReady();
    void layerRendererChanged();
    void renderStateChanged(QQuick3DXrManagerPrivate::RenderState);
    void arStateChanged(QQuick3DXrManagerPrivate::ArTrackingState);

protected:
    bool event(QEvent *event) override
    {
        {
            // NOTE: Intentionally scoped to avoid locking the mutex for events we're not handling
            // and that has side-effects (e.g. cleanup due to a deferred delete).
            QMutexLocker locker(&m_mutex);

            if (m_teardown) {
                qCDebug(lcQuick3DXr) << "Teardown in progress, skipping event handling";
                // If we're tearing down we're about to be destroyed, so we don't want to
                // ensure that the mutex is unlocked, as we're going to be destroyed anyways.
                if (event->type() == QEvent::DeferredDelete)
                    locker.unlock();
                return QObject::event(event);
            }

            switch (static_cast<Event>(event->type())) {
            case Event::Render:
            {
                const bool success = renderFrame(locker);
                // Check if we successfully rendered the frame, if not the GUI thread
                // is likely waiting for the render thread to complete rendering, so we
                // need to wake it up.
                if (!success) {
                    qCDebug(lcQuick3DXr) << "Waking up the GUI thread, rendering returned early...";
                    m_xrManager->m_syncDone = false;
                    m_waitCondition.wakeOne();
                }
            }
                return true;
            case Event::Stop:
                cleanup();
                return true;
            case Event::Pause:
                pause();
                return true;
            case Event::Pulse:
                checkRenderState();
                return true;
            }
        }

        return QObject::event(event);
    }

private:
    friend bool QQuick3DXrManagerPrivate::renderFrameImpl(QMutexLocker<QMutex> &locker, QWaitCondition &waitCondition);
    friend void QQuick3DXrManagerPrivate::update();

    static void onArStateChanged(void *context,
                                 ar_data_providers_t data_providers,
                                 ar_data_provider_state_t new_state,
                                 ar_error_t error,
                                 ar_data_provider_t failed_data_provider)
    {
        Q_UNUSED(context);
        Q_UNUSED(data_providers);
        Q_UNUSED(error);
        Q_UNUSED(failed_data_provider);

        auto *that = reinterpret_cast<CompositorLayer *>(context);

        QMutexLocker lock(&that->m_arSessionMtx);

        // If the ar session is null we're called after we tore down the session.
        QSSG_ASSERT_X(that->m_arSession != nullptr, "AR session is not running, skipping state change!", return);

        const auto oldState = that->m_arTrackingState;
        switch (new_state) {
        case ar_data_provider_state_initialized:
            that->m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Initialized;
            break;
        case ar_data_provider_state_running:
            that->m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Running;
            break;
        case ar_data_provider_state_paused:
            that->m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Paused;
            break;
        case ar_data_provider_state_stopped:
            that->m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Stopped;
            break;
        }


        if (oldState != that->m_arTrackingState) {
            QMutexLocker renderLocker(&that->m_mutex);
            emit that->arStateChanged(that->m_arTrackingState);
        }
    }

    enum class RenderStateAction
    {
        CheckOnly,
        WaitUntilRunning,
    };

    template<RenderStateAction Action = RenderStateAction::WaitUntilRunning>
    void checkRenderState()
    {
        QSSG_ASSERT(m_layerRenderer != nullptr, return);

        const auto oldState = m_renderState;
        switch (cp_layer_renderer_get_state(m_layerRenderer)) {
        case cp_layer_renderer_state_paused:
            m_renderState = QQuick3DXrManagerPrivate::RenderState::Paused;
            break;
        case cp_layer_renderer_state_running:
            m_renderState = QQuick3DXrManagerPrivate::RenderState::Running;
            break;
        case cp_layer_renderer_state_invalidated:
            m_renderState = QQuick3DXrManagerPrivate::RenderState::Invalidated;
            break;
        }

        if (oldState != m_renderState)
            emit renderStateChanged(m_renderState);

        if (m_renderState == QQuick3DXrManagerPrivate::RenderState::Paused) {
            if constexpr (Action == RenderStateAction::WaitUntilRunning) {
                qCDebug(lcQuick3DXr) << "Waiting for rendering to resume...";
                cp_layer_renderer_wait_until_running(m_layerRenderer);
                QCoreApplication::postEvent(this, new QEvent(asQEvent(CompositorLayer::Event::Pulse)));
            }
        }
    }

    ar_device_anchor_t createPoseForTiming(cp_frame_timing_t timing)
    {
        QSSG_ASSERT(m_worldTrackingProvider != nullptr, return nullptr);

        ar_device_anchor_t outAnchor = ar_device_anchor_create();
        cp_time_t presentationTime = cp_frame_timing_get_presentation_time(timing);
        CFTimeInterval queryTime = cp_time_to_cf_time_interval(presentationTime);
        ar_device_anchor_query_status_t status = ar_world_tracking_provider_query_device_anchor_at_timestamp(m_worldTrackingProvider, queryTime, outAnchor);
        if (status != ar_device_anchor_query_status_success) {
            NSLog(@"Failed to get estimated pose from world tracking provider for presentation timestamp %0.3f", queryTime);
        }
        return outAnchor;
    }

    void runWorldTrackingARSession()
    {
        QMutexLocker locker(&m_arSessionMtx);

        ar_world_tracking_configuration_t worldTrackingConfiguration = ar_world_tracking_configuration_create();
        m_worldTrackingProvider = ar_world_tracking_provider_create(worldTrackingConfiguration);

        ar_data_providers_t dataProviders = ar_data_providers_create();
        ar_data_providers_add_data_provider(dataProviders, m_worldTrackingProvider);

        QQuick3DXrInputManager *inputManager = QQuick3DXrInputManager::instance();
        QQuick3DXrAnchorManager *anchorManager = QQuick3DXrAnchorManager::instance();

        // 1. prepare
        QQuick3DXrInputManagerPrivate *pim = nullptr;
        if (QSSG_GUARD_X(inputManager != nullptr, "No InputManager available!")) {
            pim = QQuick3DXrInputManagerPrivate::get(inputManager);
            if (QSSG_GUARD(pim != nullptr))
                pim->prepareHandtracking(dataProviders);
        }

        if (QSSG_GUARD_X(anchorManager != nullptr, "No AnchorManager available!"))
            QQuick3DXrManagerPrivate::prepareAnchorManager(anchorManager, dataProviders);

        m_arSession = ar_session_create();
        ar_session_set_data_provider_state_change_handler_f(m_arSession, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), this, &onArStateChanged);
        ar_session_run(m_arSession, dataProviders);

        // 2. initialize
        QQuick3DXrManagerPrivate::initInputManager(inputManager);
        QQuick3DXrManagerPrivate::initAnchorManager(anchorManager);
    }

    void pause()
    {
        qCDebug(lcQuick3DXr, "Pausing rendering");
    }

    void cleanup()
    {
        qCDebug(lcQuick3DXr, "Cleaning up");
        if (m_xrManager)
            m_xrManager->releaseResources();
    }

    [[nodiscard]] bool renderFrame(QMutexLocker<QMutex> &locker)
    {
        Q_ASSERT(locker.isLocked());
        checkRenderState();

        if (m_renderState != QQuick3DXrManagerPrivate::RenderState::Running) {
            qCDebug(lcQuick3DXr, "Rendering is not running, skipping frame rendering!");
            return false;
        }

        // Lock the AR session mutex and render the frame. We hold onto the lock until we're done rendering
        // to ensure that the AR session isn't stopped while we're rendering (this can happen!)
        QMutexLocker arLocker(&m_arSessionMtx);
        if (m_arTrackingState != QQuick3DXrManagerPrivate::ArTrackingState::Running) {
            qCDebug(lcQuick3DXr, "AR tracking is not running, skipping frame rendering!");
            return false;
        }

        // Lock the compositor layer mutex and render the frame. We hold onto the lock until we're done rendering
        // as the configure call comes from the GUI thread and we need to ensure that the configuration is not
        // changed while we're rendering.
        QMutexLocker compositorLayer(&m_compositorLayerMtx);
        return m_xrManager->renderFrameImpl(locker, m_waitCondition);
    }

    ar_world_tracking_provider_t worldTrackingProvider() const
    {
        return m_worldTrackingProvider;
    }

    // Rendering
    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;

    mutable QMutex m_compositorLayerMtx;

    // AR Session
    mutable QMutex m_arSessionMtx;

    QQuick3DXrManagerPrivate *m_xrManager = nullptr;

    cp_layer_renderer_t m_layerRenderer = nullptr;
    ar_world_tracking_provider_t m_worldTrackingProvider = nullptr;
    ar_session_t m_arSession = nullptr;
    mutable float m_depthRange[2] {1.0f, 10000.0f}; // NOTE: Near, Far
    QQuick3DXrManagerPrivate::RenderState m_renderState = QQuick3DXrManagerPrivate::RenderState::Uninitialized;
    QQuick3DXrManagerPrivate::ArTrackingState m_arTrackingState = QQuick3DXrManagerPrivate::ArTrackingState::Uninitialized;
    bool m_initialized = false;
    bool m_teardown = false;
    mutable bool m_multiviewSupported = true;
    mutable bool m_multiviewEnabled = true;
    mutable bool m_foveationSupported = true;
    mutable bool m_foveationEnabled = true;
};

struct QSSGCompositionLayerInstance
{
    QPointer<CompositorLayer> instance;
};

Q_GLOBAL_STATIC(QSSGCompositionLayerInstance, s_compositorLayer)

// FIXME: Maybe unify with the openxr implementation?!
void QQuick3DXrManagerPrivate::updateCameraImp(simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin, int i)
{
    cp_view_t view = cp_drawable_get_view(drawable, i);
    simd_float2 depth_range = cp_drawable_get_depth_range(drawable);
    const float clipNear = depth_range[1];
    const float clipFar = depth_range[0];

    xrOrigin->eyeCamera(i)->setClipNear(clipNear);
    xrOrigin->eyeCamera(i)->setClipFar(clipFar);

    simd_float4x4 projection = cp_drawable_compute_projection(drawable, cp_axis_direction_convention_right_up_forward, i);
    QMatrix4x4 proj{projection.columns[0].x, projection.columns[1].x, projection.columns[2].x, projection.columns[3].x,
                     projection.columns[0].y, projection.columns[1].y, projection.columns[2].y, projection.columns[3].y,
                     projection.columns[0].z, projection.columns[1].z, projection.columns[2].z, projection.columns[3].z,
                     projection.columns[0].w, projection.columns[1].w, projection.columns[2].w, projection.columns[3].w};
    xrOrigin->eyeCamera(i)->setProjection(proj);

    simd_float4x4 localEyeTransform = cp_view_get_transform(view);
    simd_float4x4 eyeCameraTransform = simd_mul(headTransform, localEyeTransform);
    // NOTE: We need to convert from meters to centimeters here
    QMatrix4x4 transform{eyeCameraTransform.columns[0].x, eyeCameraTransform.columns[1].x, eyeCameraTransform.columns[2].x, eyeCameraTransform.columns[3].x * 100,
                           eyeCameraTransform.columns[0].y, eyeCameraTransform.columns[1].y, eyeCameraTransform.columns[2].y, eyeCameraTransform.columns[3].y * 100,
                           eyeCameraTransform.columns[0].z, eyeCameraTransform.columns[1].z, eyeCameraTransform.columns[2].z, eyeCameraTransform.columns[3].z * 100,
                           0.0f, 0.0f, 0.0f, 1.0f};
    QQuick3DNodePrivate::get(xrOrigin->eyeCamera(i))->setLocalTransform(transform);
}

void QQuick3DXrManagerPrivate::updateCamera(QQuick3DViewport *xrViewport, simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin, int i)
{
    updateCameraImp(headTransform, drawable, xrOrigin, i);
    xrViewport->setCamera(xrOrigin->eyeCamera(i));
}

void QQuick3DXrManagerPrivate::updateCameraMultiview(QQuick3DViewport *xrViewport, simd_float4x4 headTransform, cp_drawable_t drawable, QQuick3DXrOrigin *xrOrigin)
{
    QQuick3DCamera *cameras[2] {xrOrigin->eyeCamera(0), xrOrigin->eyeCamera(1)};

    for (int i = 0; i < 2; ++i)
        updateCameraImp(headTransform, drawable, xrOrigin, i);

    xrViewport->setMultiViewCameras(cameras);
}


QQuick3DXrManagerPrivate::QQuick3DXrManagerPrivate(QQuick3DXrManager &manager)
    : q_ptr(&manager)
{
}

QQuick3DXrManagerPrivate::~QQuick3DXrManagerPrivate()
{
}

QQuick3DXrManagerPrivate *QQuick3DXrManagerPrivate::get(QQuick3DXrManager *manager)
{
    QSSG_ASSERT(manager != nullptr, return nullptr);
    return manager->d_func();
}

bool QQuick3DXrManagerPrivate::initialize()
{
    Q_Q(QQuick3DXrManager);

    // NOTE: Check if there's a global instance of the compositor layer
    // already created, if not create one. Should probably move this over
    // to the native interface.
    if (!m_compositorLayer) {
        m_compositorLayer = s_compositorLayer->instance;
        if (!m_compositorLayer) {
            m_compositorLayer = new CompositorLayer;
            s_compositorLayer->instance = m_compositorLayer;
        }
    }

    if (!m_inputManager)
        m_inputManager = QQuick3DXrInputManager::instance();
    if (!m_anchorManager)
        m_anchorManager = QQuick3DXrAnchorManager::instance();

    // NOTE: Check if the compository layer proxy is already active.
    if (!m_compositorLayer->isInitialized()) {
        if (auto *visionOSApplicaton = qGuiApp->nativeInterface<QNativeInterface::QVisionOSApplication>()) {
            visionOSApplicaton->setImmersiveSpaceCompositorLayer(m_compositorLayer);
            // FIXME: We don't actually handle the case where the rendere changes or we get multiple calls should do something.
            QObject::connect(m_compositorLayer, &CompositorLayer::layerRendererReady, q, &QQuick3DXrManager::initialized, Qt::ConnectionType(Qt::SingleShotConnection | Qt::QueuedConnection));
            QObject::connect(m_compositorLayer, &CompositorLayer::renderStateChanged, q, [q](QQuick3DXrManagerPrivate::RenderState state) {
                switch (state) {
                    case QQuick3DXrManagerPrivate::RenderState::Uninitialized:
                        qCDebug(lcQuick3DXr, "Render state: Uninitialized");
                        QQuick3DXrManagerPrivate::get(q)->m_running = false;
                        break;
                    case QQuick3DXrManagerPrivate::RenderState::Running:
                        qCDebug(lcQuick3DXr, "Render state: Running");
                        QQuick3DXrManagerPrivate::get(q)->m_running = true;
                        QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
                        break;
                    case QQuick3DXrManagerPrivate::RenderState::Invalidated:
                        qCDebug(lcQuick3DXr, "Render state: Invalidated");
                        QQuick3DXrManagerPrivate::get(q)->m_running = false;
                        emit q->sessionEnded();
                        break;
                    case QQuick3DXrManagerPrivate::RenderState::Paused:
                        QQuick3DXrManagerPrivate::get(q)->m_running = false;
                        qCDebug(lcQuick3DXr, "Render state: Paused");
                        break;
                }
            }, Qt::DirectConnection);

            QObject::connect(m_compositorLayer, &CompositorLayer::arStateChanged, q, [q](QQuick3DXrManagerPrivate::ArTrackingState state) {
                switch (state) {
                    case QQuick3DXrManagerPrivate::ArTrackingState::Uninitialized:
                        qCDebug(lcQuick3DXr, "AR state: Uninitialized");
                        QQuick3DXrManagerPrivate::get(q)->m_arRunning = false;
                        break;
                    case QQuick3DXrManagerPrivate::ArTrackingState::Initialized:
                        qCDebug(lcQuick3DXr, "AR state: Initialized");
                        QQuick3DXrManagerPrivate::get(q)->m_arRunning = false;
                        break;
                    case QQuick3DXrManagerPrivate::ArTrackingState::Running:
                        qCDebug(lcQuick3DXr, "AR state: Running");
                        QQuick3DXrManagerPrivate::get(q)->m_arRunning = true;
                        QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
                        break;
                    case QQuick3DXrManagerPrivate::ArTrackingState::Paused:
                        qCDebug(lcQuick3DXr, "AR state: Paused");
                        QQuick3DXrManagerPrivate::get(q)->m_arRunning = false;
                        break;
                    case QQuick3DXrManagerPrivate::ArTrackingState::Stopped:
                        qCDebug(lcQuick3DXr, "AR state: Stopped");
                        QQuick3DXrManagerPrivate::get(q)->m_arRunning = false;
                        break;
                }
            }, Qt::DirectConnection);

            m_compositorLayer->init(this);
        }
        return false;
    }

    return true;
}

void QQuick3DXrManagerPrivate::setupWindow(QQuickWindow *window)
{
    if (!window) {
        qWarning("QQuick3DXrManagerPrivate: Window is null!");
        return;
    }

    QSSG_ASSERT_X(m_compositorLayer != nullptr, "No composition layer!", return);

    cp_layer_renderer_t renderer = m_compositorLayer->layerRenderer();
    if (!renderer) {
        qWarning("QQuick3DXrManagerPrivate: Layer renderer is not available.");
        return;
    }

    auto device = cp_layer_renderer_get_device(renderer);
    auto commandQueue = [device newCommandQueue];

    auto qqGraphicsDevice = QQuickGraphicsDevice::fromDeviceAndCommandQueue(static_cast<MTLDevice*>(device), static_cast<MTLCommandQueue *>(commandQueue));

    window->setGraphicsDevice(qqGraphicsDevice);
}

bool QQuick3DXrManagerPrivate::finalizeGraphics(QRhi *rhi)
{
    Q_UNUSED(rhi);

    m_isGraphicsInitialized = true;
    return m_isGraphicsInitialized;
}

bool QQuick3DXrManagerPrivate::isReady() const
{
    return m_compositorLayer && (m_compositorLayer->layerRenderer() != nullptr);
}

bool QQuick3DXrManagerPrivate::isGraphicsInitialized() const
{
    return m_isGraphicsInitialized;
}

bool QQuick3DXrManagerPrivate::setupGraphics(QQuickWindow *window)
{
    QSSG_ASSERT(window != nullptr && m_compositorLayer != nullptr, return false);

    if (m_compositorLayer->isMultiviewEnabled()) {
        if (!m_renderThread) {
            m_renderThread = new QThread;
            m_renderThread->setObjectName(QLatin1StringView(s_renderThreadName));
            m_renderThread->start();
        }

        if (m_compositorLayer->thread() != m_renderThread)
            m_compositorLayer->moveToThread(m_renderThread);

        Q_ASSERT(m_renderThread != nullptr);
        QQuickWindowPrivate::get(window)->renderControl->prepareThread(m_renderThread);
    }

    // NOTE: See QQuick3DXrManager::setupGraphics() for emission of the value changed signal
    m_multiviewRenderingEnabled = m_compositorLayer->isMultiviewEnabled();

    return true;
}

void QQuick3DXrManagerPrivate::getDefaultClipDistances(float &nearClip, float &farClip) const
{
    m_compositorLayer->getDefaultDepthRange(nearClip, farClip);
}

void QQuick3DXrManagerPrivate::teardown()
{
    Q_Q(QQuick3DXrManager);

    qCDebug(lcQuick3DXr) << "Tearing down XR session";

    QMutexLocker<QMutex> locker { nullptr };
    if (m_compositorLayer)
        locker = QMutexLocker<QMutex>{ &m_compositorLayer->renderLock() };

    m_running = false;

    if (m_inputManager)
        QQuick3DXrInputManagerPrivate::get(m_inputManager)->teardown();

    if (m_anchorManager)
        m_anchorManager->teardown();

    if (m_compositorLayer) {
        m_compositorLayer->stopArSession();
        m_compositorLayer->teardown();

        // NOTE: Unlock the render mutex. The compositor layer will be destroyed on the render thread now!
        // Also, be aware of the race condition here once the lock is released (see: teardownStarted())...
        locker.unlock();

        Qt::ConnectionType connection = (m_compositorLayer->thread() == QThread::currentThread())
                                        ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
        QMetaObject::invokeMethod(m_compositorLayer, &CompositorLayer::destroy, connection, this, q->m_quickWindow, m_compositorLayer);
        m_compositorLayer = nullptr;
    }

    if (m_renderThread) {
        m_renderThread->quit();
        m_renderThread->wait();
        delete m_renderThread;
        m_renderThread = nullptr;
    }
}

void QQuick3DXrManagerPrivate::setMultiViewRenderingEnabled(bool enable)
{
    Q_UNUSED(enable);
    qCWarning(lcQuick3DXr) << "Changing multiview rendering is not supported at runtime on visionOS!";
}

bool QQuick3DXrManagerPrivate::isMultiViewRenderingEnabled() const
{
    return m_multiviewRenderingEnabled;
}

bool QQuick3DXrManagerPrivate::setPassthroughEnabled(bool enable)
{
    Q_UNUSED(enable);
    qCWarning(lcQuick3DXr) << "Changing passthrough is not supported at runtime on visionOS!";
    return supportsPassthrough();
}

QtQuick3DXr::ReferenceSpace QQuick3DXrManagerPrivate::getReferenceSpace() const
{
    // FIXME: Not sure exactly what reference space is default or what is supported etc.
    return QtQuick3DXr::ReferenceSpace::ReferenceSpaceLocalFloor;
}

void QQuick3DXrManagerPrivate::setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace)
{
    Q_UNUSED(newReferenceSpace);
    qCWarning(lcQuick3DXr) << "Changing reference space is not supported at runtime on visionOS!";
}

void QQuick3DXrManagerPrivate::setDepthSubmissionEnabled(bool enable)
{
    if (!enable)
        qCWarning(lcQuick3DXr, "Depth submission is required on visionOS");
}

void QQuick3DXrManagerPrivate::update()
{
    Q_Q(QQuick3DXrManager);

    // Lock the render mutex and request a new frame to be rendered.
    QMutexLocker<QMutex> locker { &m_compositorLayer->renderLock() };

    if (!m_running || !m_arRunning || !m_isGraphicsInitialized) {
        qCDebug(lcQuick3DXr, "Not running, skipping update");
        return;
    }

    if (m_compositorLayer->isMultiviewEnabled()) {
        // polish (GUI thread)
        q->m_renderControl->polishItems();
        m_syncDone = false;
        QCoreApplication::postEvent(m_compositorLayer, new QEvent(CompositorLayer::asQEvent(CompositorLayer::Event::Render)));
        // Wait for the sync to complete.
        bool waitCompleted = m_compositorLayer->waitForSyncToComplete();
        // The gui thread can now continue.

        QQuick3DXrAnimationDriver *animationDriver = q->m_animationDriver;

        if (Q_LIKELY(waitCompleted && m_syncDone && animationDriver)) {
            animationDriver->setStep(m_nextStepSize);
            animationDriver->advance();
        } else {
            qCDebug(lcQuick3DXr) << "Failed to wait for sync to complete" << "\nWaitCompleted:" << waitCompleted << "\nsyncDone" << m_syncDone;
        }
    } else {
        [[maybe_unused]] bool ret = m_compositorLayer->renderFrame(locker);
    }

    QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
}

void QQuick3DXrManagerPrivate::processXrEvents()
{
    // NOTE: This is not used on visionOS
}

void QQuick3DXrManagerPrivate::processSpatialEvents(const QJsonObject &events)
{
    Q_Q(QQuick3DXrManager);
    QSSG_ASSERT(q->m_vrViewport != nullptr, return);
    QQuick3DXrInputManagerPrivate::processSpatialEvents(*q->m_vrViewport, events);
}

void QQuick3DXrManagerPrivate::prepareAnchorManager(QQuick3DXrAnchorManager *anchorManager, ar_data_providers_t dataProviders)
{
    anchorManager->prepareAnchorManager(dataProviders);
}

void QQuick3DXrManagerPrivate::initAnchorManager(QQuick3DXrAnchorManager *anchorManager)
{
    anchorManager->initAnchorManager();
}

void QQuick3DXrManagerPrivate::initInputManager(QQuick3DXrInputManager *im)
{
    QQuick3DXrInputManagerPrivate::get(im)->initHandtracking();
}

void QQuick3DXrManagerPrivate::setSamples(int samples)
{
    Q_UNUSED(samples);
    qCWarning(lcQuick3DXr) << "Changing sample count is not supported on visionOS!";
}

QString QQuick3DXrManagerPrivate::runtimeName() const
{
    return QStringLiteral("visionOS");
}

QVersionNumber QQuick3DXrManagerPrivate::runtimeVersion() const
{
    static const auto versionNumber = QOperatingSystemVersion::current().version();
    return versionNumber;
}

QString QQuick3DXrManagerPrivate::errorString() const
{
    return QString();
}

void QQuick3DXrManagerPrivate::setupShadingRateMap(QQuickWindow *window, QRhiShadingRateMap *srm)
{
    if (QRhiSwapChain *swapchain = window->swapChain()) {
        swapchain->setShadingRateMap(srm);
        if (!m_srmRenderPassDesc) {
            qCDebug(lcQuick3DXr) << "Creating render pass descriptor suitable for shading rate map use";
            m_srmRenderPassDesc = swapchain->newCompatibleRenderPassDescriptor();
            swapchain->setRenderPassDescriptor(m_srmRenderPassDesc);
        }
    } else {
        QSGRendererInterface *rif = window->rendererInterface();
        QRhiTextureRenderTarget *rt = static_cast<QRhiTextureRenderTarget *>(rif->getResource(window, QSGRendererInterface::RhiRedirectRenderTarget));
        if (QSSG_GUARD_X(rt, "No render target!")) {
            QRhiTextureRenderTargetDescription desc = rt->description();
            desc.setShadingRateMap(srm);
            rt->setDescription(desc);
            if (!m_srmRenderPassDesc) {
                qCDebug(lcQuick3DXr) << "Creating render pass descriptor suitable for shading rate map use";
                QRhiRenderPassDescriptor *rpd = rt->renderPassDescriptor();
                m_srmRenderPassDesc = rpd->newCompatibleRenderPassDescriptor();
                rt->setRenderPassDescriptor(m_srmRenderPassDesc);
                rt->create();
            }
        }
    }
}

void QQuick3DXrManagerPrivate::releaseResources()
{
    qCDebug(lcQuick3DXr) << "Releasing resources";
    Q_ASSERT((m_renderThread == nullptr) || QThread::currentThread() == m_renderThread);
    delete m_srmRenderPassDesc;
    m_srmRenderPassDesc = nullptr;
    delete m_rhiDepthTexture;
    m_rhiDepthTexture = nullptr;
    for (size_t i = 0, end = std::size(m_srm); i < end; ++i) {
        delete m_srm[i];
        m_srm[i] = nullptr;
    }
}

bool QQuick3DXrManagerPrivate::renderFrameImpl(QMutexLocker<QMutex> &locker, QWaitCondition &waitCondition)
{
    Q_Q(QQuick3DXrManager);

    // NOTE: The GUI thread is locked at this point
    QQuickWindow *window = q->m_quickWindow;
    QQuickRenderControl *renderControl = q->m_renderControl;
    QQuick3DXrOrigin *xrOrigin = q->m_xrOrigin;
    QQuick3DViewport *xrViewport = q->m_vrViewport;
    QQuick3DXrAnimationDriver *animationDriver = q->m_animationDriver;

    QSSG_ASSERT_X(window && renderControl && xrViewport && xrOrigin && animationDriver, "Invalid state, rendering aborted", return false);

    const bool multiviewRenderingEnabled = isMultiViewRenderingEnabled();

    Q_ASSERT(!multiviewRenderingEnabled || QThread::currentThread() != window->thread());

    auto layerRenderer = m_compositorLayer->layerRenderer();
    cp_frame_t frame = cp_layer_renderer_query_next_frame(layerRenderer);
    if (Q_UNLIKELY(frame == nullptr)) {
        qWarning("Failed to get next frame");
        return false;
    }

    cp_frame_timing_t timing = cp_frame_predict_timing(frame);
    if (Q_UNLIKELY(timing == nullptr)) {
        qWarning("Failed to get timing for frame");
        return false;
    }

    cp_frame_start_update(frame);

    cp_frame_end_update(frame);

    cp_time_t optimalInputTime = cp_frame_timing_get_optimal_input_time(timing);
    cp_time_wait_until(optimalInputTime);

    cp_frame_start_submission(frame);
    cp_drawable_t drawable = cp_frame_query_drawable(frame);
    if (Q_UNLIKELY(drawable == nullptr)) {
        qWarning("Failed to get drawable for frame");
        return false;
    }

    cp_frame_timing_t actualTiming = cp_drawable_get_frame_timing(drawable);
    ar_device_anchor_t anchor = m_compositorLayer->createPoseForTiming(actualTiming);
    cp_drawable_set_device_anchor(drawable, anchor);

    // Get the pose transform from the anchor
    simd_float4x4 headTransform = ar_anchor_get_origin_from_anchor_transform(anchor);

    // NOTE: We need to convert from meters to centimeters here
    QMatrix4x4 qtHeadTransform{headTransform.columns[0].x, headTransform.columns[1].x, headTransform.columns[2].x, headTransform.columns[3].x * 100,
                                 headTransform.columns[0].y, headTransform.columns[1].y, headTransform.columns[2].y, headTransform.columns[3].y * 100,
                                 headTransform.columns[0].z, headTransform.columns[1].z, headTransform.columns[2].z, headTransform.columns[3].z * 100,
                                 0.0f, 0.0f, 0.0f, 1.0f};
    xrOrigin->updateTrackedCamera(qtHeadTransform);

    // Update the hands
    if (QSSG_GUARD(m_inputManager != nullptr))
        QQuick3DXrInputManagerPrivate::get(m_inputManager)->updateHandtracking();

    // Animation driver
    // Convert the cp_frame_timing_t ticks to milliseconds
    enum : size_t { DisplayPeriod  = 0, DisplayDelta };
    qint64 stepSizes[2] {0, 0};
    auto &[displayPeriodMS, displayDeltaMS] = stepSizes;
    displayPeriodMS = qint64(cp_time_to_cf_time_interval(optimalInputTime) * 1000.0);
    displayDeltaMS = ((qint64(cp_time_to_cf_time_interval(cp_frame_timing_get_optimal_input_time(actualTiming)) * 1000.0)) - m_previousTime);
    const size_t selector = ((m_previousTime == 0) || (displayDeltaMS > displayPeriodMS)) ? DisplayPeriod : DisplayDelta;
    m_nextStepSize = stepSizes[selector];
    m_previousTime = displayPeriodMS;

    if (!multiviewRenderingEnabled) {
        animationDriver->setStep(m_nextStepSize);
        animationDriver->advance();
    }

    QRhi *rhi = renderControl->rhi();

    const auto viewCount = cp_drawable_get_view_count(drawable);
    const auto textureCount = cp_drawable_get_texture_count(drawable);
    const auto renderCalls = textureCount; // To make it less confusing...

    bool foveationEnabled = m_compositorLayer->isFoveationEnabled();
    Q_ASSERT(!foveationEnabled || rhi->isFeatureSupported(QRhi::VariableRateShadingMap));
    // We still should make sure we have a rate map!
    if (foveationEnabled) {
        const size_t rrmapcount = cp_drawable_get_rasterization_rate_map_count(drawable);
        const bool validMapCount = (rrmapcount == 1 || rrmapcount == 2);
        QSSG_GUARD_X(validMapCount, "Invalid rate map count!");
        foveationEnabled = validMapCount;

        if (foveationEnabled) {
            for (size_t i = 0; i < rrmapcount; ++i) {
                if (!m_srm[i])
                    m_srm[i] = rhi->newShadingRateMap();
                id<MTLRasterizationRateMap> rrm = cp_drawable_get_rasterization_rate_map(drawable, i);
                const bool wasCreated = m_srm[i]->createFrom({ quint64(rrm) });
                QSSG_GUARD_X(wasCreated, "Failed to create shading rate map!");
                foveationEnabled = (foveationEnabled && wasCreated);
            }
        }
    }

    // NOTE: Expectation is that when multiview rendering is enabled we get a multiple drawables with a single texture array,
    //       each view/eye is then rendered to a slice in the texture array. If multiview rendering is _not_ enabled we get
    //       two drawables with two textures, one for each eye. In this case we render each eye to its own texture.
    //
    //       And just to keep things interesting the emulator will use one view and one texture, so even if we hit the
    //       "dedicated"/non-multiview case we need to handle that as well. NOTE: We don't do threaded rendering when
    //       multiview rendering is disabled!

    for (size_t i = 0, end = renderCalls; i != end ; ++i) {
        // Setup the RenderTarget based on the current drawable
        id<MTLTexture> colorMetalTexture = cp_drawable_get_color_texture(drawable, i);
        auto textureSize = QSize([colorMetalTexture width], [colorMetalTexture height]);

        QQuickRenderTarget renderTarget;

        if (multiviewRenderingEnabled)
            renderTarget = QQuickRenderTarget::fromMetalTexture(static_cast<MTLTexture*>(colorMetalTexture), [colorMetalTexture pixelFormat], [colorMetalTexture pixelFormat]/*viewFormat*/, textureSize, 1 /*sampleCount*/, viewCount, {});
        else
            renderTarget = QQuickRenderTarget::fromMetalTexture(static_cast<MTLTexture*>(colorMetalTexture), [colorMetalTexture pixelFormat], textureSize);

        auto depthMetalTexture = cp_drawable_get_depth_texture(drawable, i);
        auto depthTextureSize = QSize([depthMetalTexture width], [depthMetalTexture height]);
        MTLPixelFormat depthTextureFormat = [depthMetalTexture pixelFormat];
        static const auto convertFormat = [](MTLPixelFormat format) -> QRhiTexture::Format {
            switch (format) {
            case MTLPixelFormatDepth16Unorm:
                return QRhiTexture::D16;
            case MTLPixelFormatDepth32Float:
                return QRhiTexture::D32F;
            default:
                qWarning("Unsupported depth texture format");
                return QRhiTexture::UnknownFormat;
            }
        };
        auto depthFormat = convertFormat(depthTextureFormat);
        if (depthFormat != QRhiTexture::UnknownFormat) {
            if (m_rhiDepthTexture && (m_rhiDepthTexture->format() != depthFormat || m_rhiDepthTexture->pixelSize() != depthTextureSize)) {
                delete m_rhiDepthTexture;
                m_rhiDepthTexture = nullptr;
            }

            if (!m_rhiDepthTexture) {
                if (multiviewRenderingEnabled)
                    m_rhiDepthTexture = rhi->newTextureArray(depthFormat, viewCount, depthTextureSize, 1, QRhiTexture::RenderTarget);
                else
                    m_rhiDepthTexture = rhi->newTexture(depthFormat, depthTextureSize, 1, QRhiTexture::RenderTarget);
            }


            m_rhiDepthTexture->createFrom({ quint64(static_cast<MTLTexture*>(depthMetalTexture)), 0});
            renderTarget.setDepthTexture(m_rhiDepthTexture);
        }

        window->setRenderTarget(renderTarget);

        // Initial window size is the size of the texture, but we also need to check the viewport size
        // as this can be different. If the viewport size is different we need to adjust the window size.
        // This is the case when we render to a smaller texture and then scale it up to the window (i.e. foveated rendering).
        QSize renderSize = textureSize;
        if (foveationEnabled) {
            cp_view_t view = cp_drawable_get_view(drawable, i);
            cp_view_texture_map_t texture_map = cp_view_get_view_texture_map(view);
            auto vp = cp_view_texture_map_get_viewport(texture_map);
            renderSize = QSize(vp.width, vp.height);
        }

        window->setGeometry(0,
                            0,
                            renderSize.width(),
                            renderSize.height());
        window->contentItem()->setSize(renderSize);
        // NOTE: We set the size here as we need to make sure the window size is correct before
        // the sync, or the xrView's size will be incorrect. This happens because we're on the
        // render thread (GUI thread is blocked) and any updates to the contentItem size will
        // not be updated (see: QQuick3DXrView::init)...
        xrViewport->setSize(renderSize);

        // Update the camera pose
        if (QSSG_GUARD(xrOrigin)) {
            if (multiviewRenderingEnabled)
                updateCameraMultiview(xrViewport, headTransform, drawable, xrOrigin);
            else
                updateCamera(xrViewport, headTransform, drawable, xrOrigin, i);
        }

        // We only do this if we are rendering on a separate thread and with multiview rendering enabled,
        // or else the beginFrame and sync will be done on the GUI thread after both eyes have been
        // rendered...
        if (multiviewRenderingEnabled) {
            // Marks the start of the frame.
            renderControl->beginFrame();
            // Synchronization happens here on the render thread (with the GUI thread locked)
            m_syncDone = renderControl->sync();

            if (Q_UNLIKELY(!m_syncDone))
                return false;

            if (foveationEnabled)
                setupShadingRateMap(window, m_srm[i]);

            // Signal the GUI thread that the sync is done, so it can continue.
            waitCondition.wakeOne();
            locker.unlock();

            // Render the frame
            renderControl->render();
            // Marks the end of the frame.
            renderControl->endFrame();
        } else {
            renderControl->polishItems();
            renderControl->beginFrame();
            renderControl->sync();
            if (foveationEnabled)
                setupShadingRateMap(window, m_srm[i]);
            renderControl->render();
            renderControl->endFrame();
        }

#if TARGET_OS_SIMULATOR == 1
        // With multiview this indicates that the frame with both eyes is ready from
        // the 3D APIs perspective. Without multiview this is done - and so the
        // signal is emitted - multiple times (twice) per "frame" (eye).
        QRhiRenderTarget *rt = QQuickWindowPrivate::get(window)->activeCustomRhiRenderTarget();
        if (rt->resourceType() == QRhiResource::TextureRenderTarget && static_cast<QRhiTextureRenderTarget *>(rt)->description().colorAttachmentAt(0)->texture())
            emit q->frameReady();
#endif // TARGET_OS_SIMULATOR
    }

    id<MTLCommandBuffer> commandBuffer = [static_cast<const QRhiMetalNativeHandles*>(renderControl->rhi()->nativeHandles())->cmdQueue commandBuffer];

    cp_drawable_encode_present(drawable, commandBuffer);
    [commandBuffer commit];

    cp_frame_end_submission(frame);

    return true;
}

QT_END_NAMESPACE

#include "qquick3dxrmanager_visionos.moc"
