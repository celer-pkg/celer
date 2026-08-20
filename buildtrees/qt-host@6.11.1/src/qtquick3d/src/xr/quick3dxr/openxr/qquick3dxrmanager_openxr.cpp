// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrmanager_openxr_p.h"
#include "qquick3dxrcamera_p.h"
#include "qquick3dxrorigin_p.h"
#include "qquick3dxranimationdriver_p.h"
#include "qquick3dxrmanager_p.h"
#include "qquick3dxrinputmanager_p.h"

#include "qopenxrhelpers_p.h"
#include "qopenxrinputmanager_p.h"
#include "qquick3dxranchormanager_openxr_p.h"

#include "qtquick3dxrglobal_p.h"

#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>

#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickrendercontrol.h>

#include <QtGui/qquaternion.h>

#include <QtCore/qobject.h>

#include <openxr/openxr_reflection.h>

#ifdef XR_USE_GRAPHICS_API_VULKAN
# include "qopenxrgraphics_vulkan_p.h"
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
# include "qopenxrgraphics_d3d11_p.h"
#endif

#ifdef XR_USE_GRAPHICS_API_D3D12
# include "qopenxrgraphics_d3d12_p.h"
#endif

#ifdef XR_USE_GRAPHICS_API_OPENGL
# include "qopenxrgraphics_opengl_p.h"
#endif

#ifdef XR_USE_PLATFORM_ANDROID
# include <QtCore/qnativeinterface.h>
# include <QtCore/QJniEnvironment>
# include <QtCore/QJniObject>
# ifdef XR_USE_GRAPHICS_API_OPENGL_ES
#  include "qopenxrgraphics_opengles_p.h"
# endif // XR_USE_GRAPHICS_API_OPENGL_ES
#endif // XR_USE_PLATFORM_ANDROID

#ifdef XR_USE_GRAPHICS_API_METAL
# include "qopenxrgraphics_metal_p.h"
#endif // XR_USE_GRAPHICS_API_METAL

static XrReferenceSpaceType getXrReferenceSpaceType(QtQuick3DXr::ReferenceSpace referenceSpace)
{
    switch (referenceSpace) {
    case QtQuick3DXr::ReferenceSpace::ReferenceSpaceLocal:
        return XR_REFERENCE_SPACE_TYPE_LOCAL;
    case QtQuick3DXr::ReferenceSpace::ReferenceSpaceStage:
        return XR_REFERENCE_SPACE_TYPE_STAGE;
    case QtQuick3DXr::ReferenceSpace::ReferenceSpaceLocalFloor:
        return XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT;
    default:
        return XR_REFERENCE_SPACE_TYPE_LOCAL;
    }
}

static QtQuick3DXr::ReferenceSpace getReferenceSpaceType(XrReferenceSpaceType referenceSpace)
{
    switch (referenceSpace) {
    case XR_REFERENCE_SPACE_TYPE_LOCAL:
        return QtQuick3DXr::ReferenceSpace::ReferenceSpaceLocal;
    case XR_REFERENCE_SPACE_TYPE_STAGE:
        return QtQuick3DXr::ReferenceSpace::ReferenceSpaceStage;
    case XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT:
        return QtQuick3DXr::ReferenceSpace::ReferenceSpaceLocalFloor;
    default:
        return QtQuick3DXr::ReferenceSpace::ReferenceSpaceUnknown;
    }
}

// Macro to generate stringify functions for OpenXR enumerations based data provided in openxr_reflection.h
#define ENUM_CASE_STR(name, val) case name: return #name;
#define MAKE_TO_STRING_FUNC(enumType)                  \
    static inline const char* to_string(enumType e) {         \
    switch (e) {                                   \
    XR_LIST_ENUM_##enumType(ENUM_CASE_STR)     \
    default: return "Unknown " #enumType;      \
    }                                              \
    }

MAKE_TO_STRING_FUNC(XrReferenceSpaceType)
MAKE_TO_STRING_FUNC(XrViewConfigurationType)
MAKE_TO_STRING_FUNC(XrEnvironmentBlendMode)
MAKE_TO_STRING_FUNC(XrSessionState)
MAKE_TO_STRING_FUNC(XrResult)

static bool isExtensionSupported(const char *extensionName, const QVector<XrExtensionProperties> &instanceExtensionProperties, uint32_t *extensionVersion = nullptr)
{
    for (const auto &extensionProperty : instanceExtensionProperties) {
        if (!strcmp(extensionName, extensionProperty.extensionName)) {
            if (extensionVersion)
                *extensionVersion = extensionProperty.extensionVersion;
            return true;
        }
    }
    return false;
}

static bool isApiLayerSupported(const char *layerName, const QVector<XrApiLayerProperties> &apiLayerProperties)
{
    for (const auto &prop : apiLayerProperties) {
        if (!strcmp(layerName, prop.layerName))
            return true;
    }
    return false;
}

// OpenXR's debug messenger stuff is a carbon copy of the Vulkan one, hence we
// replicate the same behavior on Qt side as well, i.e. route by default
// everything to qDebug. Filtering or further control (that is supported with
// the C++ APIS in the QVulkan* stuff) is not provided here for now.
#ifdef XR_EXT_debug_utils
static XRAPI_ATTR XrBool32 XRAPI_CALL defaultDebugCallbackFunc(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                                                               XrDebugUtilsMessageTypeFlagsEXT messageType,
                                                               const XrDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                               void *userData)
{
    Q_UNUSED(messageSeverity);
    Q_UNUSED(messageType);
    QQuick3DXrManager *self = static_cast<QQuick3DXrManager *>(userData);
    // this is qDebug intentionally, not categorized
    qDebug("xrDebug [QOpenXRManager %p] %s", self, callbackData->message);
    return XR_FALSE;
}
#endif // XR_EXT_debug_utils

static std::pair<XrResult, XrFrameState> waitForNextFrame(XrSession session)
{
    XrFrameWaitInfo frameWaitInfo {};
    frameWaitInfo.type = XR_TYPE_FRAME_WAIT_INFO;
    XrFrameState frameState {};
    frameState.type = XR_TYPE_FRAME_STATE;
    auto result = xrWaitFrame(session, &frameWaitInfo, &frameState);
    return { result, frameState };
}

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

static const char s_workerThreadName[] = "QQuick3DXrWorkerThread";

class QQuick3DOpenXRThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit QQuick3DOpenXRThreadWorker(XrSession session, QObject *parent = nullptr)
        : QObject(parent), m_session(session)
    {
    }

    using EventT = std::underlying_type_t<QEvent::Type>;
    enum Event : EventT {
        WaitForFrame = QEvent::User + 1,
    };

    static constexpr QEvent::Type asQEvent(QQuick3DOpenXRThreadWorker::Event event)
    {
        return static_cast<QEvent::Type>(event);
    }

    void startWaitingForFrame()
    {
        QCoreApplication::postEvent(this, new QEvent(QQuick3DOpenXRThreadWorker::asQEvent(QQuick3DOpenXRThreadWorker::Event::WaitForFrame)));
    }

protected:
    bool event(QEvent *event) override
    {
        switch (static_cast<Event>(event->type())) {
        case Event::WaitForFrame: {
            auto result = waitForNextFrame(m_session);
            emit frameWaited(result.first, result.second);
            return true;
        }
        default:
            return QObject::event(event);
        }
    }

signals:
    void frameWaited(XrResult result, const XrFrameState &frameState);

private:
    XrSession m_session = XR_NULL_HANDLE;
};

void QQuick3DXrManagerPrivate::setErrorString(XrResult result, const char *callName)
{
    if (m_runtimeName.isEmpty())
        m_errorString = QObject::tr("%1 failed: no OpenXR runtime found").arg(QLatin1StringView(callName));
    else
        m_errorString = QObject::tr("%1 for runtime %2 %3 failed with %4.")
                            .arg(QLatin1StringView(callName),
                                 m_runtimeName,
                                 m_runtimeVersion.toString(),
                                 OpenXRHelpers::getXrResultAsString(result, m_instance));
    if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE) // this is very common
        m_errorString += QObject::tr("\nThe OpenXR runtime has no connection to the headset; check if connection is active and functional.");
}

QQuick3DXrManagerPrivate::QQuick3DXrManagerPrivate(QQuick3DXrManager &manager)
    : q_ptr(&manager)
{
    m_multiviewRendering = !QQuick3DXrManager::isMultiviewRenderingDisabled();
}

QQuick3DXrManagerPrivate::~QQuick3DXrManagerPrivate()
{
    delete m_graphics; // last, with Vulkan this may own the VkInstance
}

QQuick3DXrManagerPrivate *QQuick3DXrManagerPrivate::get(QQuick3DXrManager *manager)
{
    QSSG_ASSERT(manager != nullptr, return nullptr);
    return manager->d_func();
}

void QQuick3DXrManagerPrivate::updateCameraHelper(QQuick3DXrEyeCamera *camera, const XrCompositionLayerProjectionView &layerView)
{
    camera->setLeftTangent(qTan(layerView.fov.angleLeft));
    camera->setRightTangent(qTan(layerView.fov.angleRight));
    camera->setUpTangent(qTan(layerView.fov.angleUp));
    camera->setDownTangent(qTan(layerView.fov.angleDown));

    camera->setPosition(QVector3D(layerView.pose.position.x,
                                  layerView.pose.position.y,
                                  layerView.pose.position.z) * 100.0f); // convert m to cm

    camera->setRotation(QQuaternion(layerView.pose.orientation.w,
                                    layerView.pose.orientation.x,
                                    layerView.pose.orientation.y,
                                    layerView.pose.orientation.z));
}

// Set the active camera for the view to the camera for the eye value
// This is set right before updateing/rendering for that eye's view
void QQuick3DXrManagerPrivate::updateCameraNonMultiview(int eye, const XrCompositionLayerProjectionView &layerView)
{
    Q_Q(QQuick3DXrManager);

    QQuick3DXrOrigin *xrOrigin = q->m_xrOrigin;

    QQuick3DXrEyeCamera *eyeCamera = xrOrigin ? xrOrigin->eyeCamera(eye) : nullptr;

    if (eyeCamera)
        updateCameraHelper(eyeCamera, layerView);

    q->m_vrViewport->setCamera(eyeCamera);
}

// The multiview version sets multiple cameras.
void QQuick3DXrManagerPrivate::updateCameraMultiview(int projectionLayerViewStartIndex, int count)
{
    Q_Q(QQuick3DXrManager);

    QQuick3DXrOrigin *xrOrigin = q->m_xrOrigin;
    QQuick3DViewport *vrViewport = q->m_vrViewport;

    QVarLengthArray<QQuick3DCamera *, 4> cameras;
    for (int i = projectionLayerViewStartIndex; i < projectionLayerViewStartIndex + count; ++i) {
        QQuick3DXrEyeCamera *eyeCamera = xrOrigin ? xrOrigin->eyeCamera(i) : nullptr;
        if (eyeCamera)
            updateCameraHelper(eyeCamera, m_projectionLayerViews[i]);
        cameras.append(eyeCamera);
    }
    vrViewport->setMultiViewCameras(cameras.data(), cameras.count());
}

bool QQuick3DXrManagerPrivate::supportsTransparentBlendMode() const
{
    return m_blendModes.contains(XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND)
        || m_blendModes.contains(XR_ENVIRONMENT_BLEND_MODE_ADDITIVE);
}

bool QQuick3DXrManagerPrivate::supportsFBPassthrough() const
{
    bool supported = false;
    XrSystemPassthroughProperties2FB passthroughSystemProperties{};
    passthroughSystemProperties.type = XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES2_FB;

    XrSystemProperties systemProperties{};
    systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
    systemProperties.next = &passthroughSystemProperties;

    XrSystemGetInfo systemGetInfo{};
    systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemGetInfo.formFactor = m_formFactor;

    XrSystemId systemId = XR_NULL_SYSTEM_ID;
    xrGetSystem(m_instance, &systemGetInfo, &systemId);
    xrGetSystemProperties(m_instance, systemId, &systemProperties);

    supported = (passthroughSystemProperties.capabilities & XR_PASSTHROUGH_CAPABILITY_BIT_FB) == XR_PASSTHROUGH_CAPABILITY_BIT_FB;

    if (!supported) {
        // Try the old one. (the Simulator reports spec version 3 for
        // XR_FB_passthrough, yet the capabilities in
        // XrSystemPassthroughProperties2FB are 0)
        XrSystemPassthroughPropertiesFB oldPassthroughSystemProperties{};
        oldPassthroughSystemProperties.type = XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES_FB;
        systemProperties.next = &oldPassthroughSystemProperties;
        xrGetSystemProperties(m_instance, systemId, &systemProperties);
        supported = oldPassthroughSystemProperties.supportsPassthrough;
    }

    return supported;
}

bool QQuick3DXrManagerPrivate::supportsPassthrough() const
{
    // Passthrough is supported if either XR_FB_passthrough extension is available
    // or if transparent blend modes (ALPHA_BLEND/ADDITIVE) are supported
    return supportsFBPassthrough() || supportsTransparentBlendMode();
}

void QQuick3DXrManagerPrivate::setupWindow(QQuickWindow *window)
{
    QSSG_ASSERT(window != nullptr, return);
    if (m_graphics)
        m_graphics->setupWindow(window);
}

bool QQuick3DXrManagerPrivate::isGraphicsInitialized() const
{
    return m_graphics && m_graphics->rhi();
}

bool QQuick3DXrManagerPrivate::setupGraphics(QQuickWindow *window)
{
    QSSG_ASSERT(window != nullptr, return false);
    QSSG_ASSERT(m_graphics != nullptr, return false);

    return m_graphics->setupGraphics(m_instance, m_systemId, window->graphicsConfiguration());
}

void QQuick3DXrManagerPrivate::update()
{
    Q_Q(QQuick3DXrManager);
    if (m_waitingForFrame) {
        m_wantUpdate = true;
        return;
    }

    QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
}

void QQuick3DXrManagerPrivate::processXrEvents()
{
    Q_Q(QQuick3DXrManager);

    bool exitRenderLoop = false;
    bool requestrestart = false;
    pollEvents(&exitRenderLoop, &requestrestart);

    if (exitRenderLoop)
        emit q->sessionEnded();

    if (m_sessionRunning && m_inputManager) {
        QQuick3DXrInputManagerPrivate::get(m_inputManager)->pollActions();
        q->renderFrame();
    }
}

void QQuick3DXrManagerPrivate::destroySwapchain()
{
    for (const Swapchain &swapchain : m_swapchains)
        xrDestroySwapchain(swapchain.handle);

    m_swapchains.clear();
    m_swapchainImages.clear();
    m_configViews.clear();

    for (const Swapchain &swapchain : m_depthSwapchains)
        xrDestroySwapchain(swapchain.handle);

    m_depthSwapchains.clear();
    m_depthSwapchainImages.clear();
}

void QQuick3DXrManagerPrivate::doRenderFrame()
{
    Q_ASSERT(m_session != XR_NULL_HANDLE);

    if (m_waitingForFrame)
        return;
    m_waitingForFrame = true;

    if (m_worker) {
        m_worker->startWaitingForFrame();
        // onFrameWaitCompleted() will be called asynchronoiusly
    } else {
        auto result = waitForNextFrame(m_session);
        onFrameWaitCompleted(result.first, result.second);
    }
}

void QQuick3DXrManagerPrivate::onFrameWaitCompleted(XrResult result, const XrFrameState &frameState)
{
    if (!m_waitingForFrame)
        return;
    m_waitingForFrame = false;

    if (!checkXrResult(result)) {
        qWarning("xrWaitFrame failed");
    } else {
        doRenderFrameAferWait(frameState);
    }

    if (m_wantUpdate) {
        m_wantUpdate = false;
        update();
    }
}

void QQuick3DXrManagerPrivate::doRenderFrameAferWait(const XrFrameState &frameState)
{
    Q_ASSERT(m_session != XR_NULL_HANDLE);

    XrFrameBeginInfo frameBeginInfo{};
    frameBeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
    if (!checkXrResult(xrBeginFrame(m_session, &frameBeginInfo))) {
        qWarning("xrBeginFrame failed");
        return;
    }

    QVector<XrCompositionLayerBaseHeader*> layers;

    XrCompositionLayerPassthroughFB passthroughCompLayer{};
    passthroughCompLayer.type = XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB;
    if (m_fbPassthroughEnabled) {
        if (m_passthroughLayer == XR_NULL_HANDLE)
            createMetaQuestPassthroughLayer();
        passthroughCompLayer.layerHandle = m_passthroughLayer;
        passthroughCompLayer.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        passthroughCompLayer.space = XR_NULL_HANDLE;
        layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&passthroughCompLayer));
    }

    XrCompositionLayerProjection layer{};
    layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
    layer.layerFlags |= XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
    layer.layerFlags |= XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT;

    if (frameState.shouldRender == XR_TRUE) {
        if (renderLayer(frameState.predictedDisplayTime, frameState.predictedDisplayPeriod, layer)) {
            layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer));
        }
    }

    XrFrameEndInfo frameEndInfo{};
    frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
    frameEndInfo.displayTime = frameState.predictedDisplayTime;
    if (!m_fbPassthroughEnabled)
        frameEndInfo.environmentBlendMode = m_environmentBlendMode;
    else
        frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    frameEndInfo.layerCount = (uint32_t)layers.size();
    frameEndInfo.layers = layers.data();
    if (!checkXrResult(xrEndFrame(m_session, &frameEndInfo)))
        qWarning("xrEndFrame failed");
}

bool QQuick3DXrManagerPrivate::finalizeGraphics(QRhi *rhi)
{
    QSSG_ASSERT(rhi != nullptr && m_graphics != nullptr, return false);

    if (m_multiviewRendering && !rhi->isFeatureSupported(QRhi::MultiView)) {
        qCDebug(lcQuick3DXr) << "Multiview rendering is not supported with the current graphics API";
        m_multiviewRendering = false;
    }

#if QT_CONFIG(graphicsframecapture)
    if (m_frameCapture) {
        m_frameCapture->setCapturePath(QLatin1String("."));
        m_frameCapture->setCapturePrefix(QLatin1String("quick3dxr"));
        m_frameCapture->setRhi(rhi);
        if (!m_frameCapture->isLoaded()) {
            qWarning("Quick 3D XR: Frame capture was requested but QGraphicsFrameCapture is not initialized"
                     " (or has no backends enabled in the Qt build)");
        } else {
            qCDebug(lcQuick3DXr, "Quick 3D XR: Frame capture initialized");
        }
    }
#endif

    m_isGraphicsInitialized = m_graphics->finializeGraphics(rhi);
    return m_isGraphicsInitialized;
}

bool QQuick3DXrManagerPrivate::initialize()
{
    Q_Q(QQuick3DXrManager);
    // This, meaning constructing the QGraphicsFrameCapture if we'll want it,
    // must be done as early as possible, before initalizing graphics. In fact
    // in hybrid apps it might be too late at this point if Qt Quick (so someone
    // outside our control) has initialized graphics which then makes
    // RenderDoc's hooking mechanisms disfunctional.
    if (qEnvironmentVariableIntValue("QT_QUICK3D_XR_FRAME_CAPTURE")) {
#if QT_CONFIG(graphicsframecapture)
        m_frameCapture.reset(new QGraphicsFrameCapture);
#else
        qWarning("Quick 3D XR: Frame capture was requested, but Qt is built without QGraphicsFrameCapture");
#endif
    }

#ifdef XR_USE_PLATFORM_ANDROID
    // Initialize the Loader
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
    xrGetInstanceProcAddr(
        XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);
    if (xrInitializeLoaderKHR != NULL) {
        m_javaVM = QJniEnvironment::javaVM();
        m_androidActivity = QNativeInterface::QAndroidApplication::context();

        XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid;
        memset(&loaderInitializeInfoAndroid, 0, sizeof(loaderInitializeInfoAndroid));
        loaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
        loaderInitializeInfoAndroid.next = NULL;
        loaderInitializeInfoAndroid.applicationVM = m_javaVM;
        loaderInitializeInfoAndroid.applicationContext = m_androidActivity.object();
        XrResult xrResult = xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid);
        if (xrResult != XR_SUCCESS) {
            qWarning("Failed to initialize OpenXR Loader: %s", to_string(xrResult));
            return false;
        }
    }
#endif

    // Init the Graphics Backend
    auto graphicsAPI = QQuickWindow::graphicsApi();

    m_graphics = nullptr;
#ifdef XR_USE_GRAPHICS_API_VULKAN
    if (graphicsAPI == QSGRendererInterface::Vulkan)
        m_graphics = new QOpenXRGraphicsVulkan;
#endif
#ifdef XR_USE_GRAPHICS_API_D3D11
    if (graphicsAPI == QSGRendererInterface::Direct3D11)
        m_graphics = new QOpenXRGraphicsD3D11;
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
    if (graphicsAPI == QSGRendererInterface::Direct3D12)
        m_graphics = new QOpenXRGraphicsD3D12;
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
    if (graphicsAPI == QSGRendererInterface::OpenGL)
        m_graphics = new QOpenXRGraphicsOpenGL;
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL_ES
    if (graphicsAPI == QSGRendererInterface::OpenGL)
        m_graphics = new QOpenXRGraphicsOpenGLES;
#endif
#ifdef XR_USE_GRAPHICS_API_METAL
    if (graphicsAPI == QSGRendererInterface::Metal)
        m_graphics = new QOpenXRGraphicsMetal;
#endif

    if (!m_graphics) {
        qWarning() << "The Qt Quick Scenegraph is not using a supported RHI mode:" << graphicsAPI;
        return false;
    }

    // Print out extension and layer information
    checkXrExtensions(nullptr);
    checkXrLayers();

    m_spaceExtension = QQuick3DXrAnchorManager::instance();

    // Create Instance
    XrResult result = createXrInstance();
    if (result != XR_SUCCESS) {
        setErrorString(result, "xrCreateInstance");
        delete m_graphics;
        m_graphics = nullptr;
        return false;
    } else {
        checkXrInstance();
    }

    // Catch OpenXR runtime messages via XR_EXT_debug_utils and route them to qDebug
    setupDebugMessenger();

    // Load System
    result = initializeSystem();
    if (result != XR_SUCCESS) {
        setErrorString(result, "xrGetSystem");
        delete m_graphics;
        m_graphics = nullptr;
        return false;
    }

    // Setup Graphics
    if (!q->setupGraphics()) {
        m_errorString = QObject::tr("Failed to set up 3D API integration");
        delete m_graphics;
        m_graphics = nullptr;
        return false;
    }

    // Create Session
    XrSessionCreateInfo xrSessionInfo{};
    xrSessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    xrSessionInfo.systemId = m_systemId;

    auto overlayLayerPlacement = qEnvironmentVariableIntValue("QT_QUICK3D_XR_OVERLAY_PLACEMENT");
    if (m_overlayExtensionSupported && overlayLayerPlacement > 0) {
        XrSessionCreateInfoOverlayEXTX overlayInfo {};
        overlayInfo.type = XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX;
        overlayInfo.sessionLayersPlacement = overlayLayerPlacement;
        overlayInfo.next = m_graphics->handle();
        xrSessionInfo.next = &overlayInfo;
        qCDebug(lcQuick3DXr, "requesting overlay placement: %d", overlayLayerPlacement);
    } else {
        xrSessionInfo.next = m_graphics->handle();
    }

    result = xrCreateSession(m_instance, &xrSessionInfo, &m_session);
    if (result != XR_SUCCESS) {
        setErrorString(result, "xrCreateSession");
        delete m_graphics;
        m_graphics = nullptr;
        return false;
    }

    const bool disableAsyncWait = qEnvironmentVariableIntValue("QT_QUICK3D_XR_DISABLE_ASYNC_WAIT") != 0;
    if (Q_LIKELY(!disableAsyncWait))
        initWorkerThread();

    // Meta Quest Specific Setup
    if (m_colorspaceExtensionSupported)
        setupMetaQuestColorSpaces();
    if (m_displayRefreshRateExtensionSupported)
        setupMetaQuestRefreshRates();
    if (m_spaceExtensionSupported)
        m_spaceExtension->initialize(m_instance, m_session);

    checkReferenceSpaces();

    // Setup Input
    m_inputManager = QQuick3DXrInputManager::instance();
    if (QSSG_GUARD(m_inputManager != nullptr))
        QQuick3DXrInputManagerPrivate::get(m_inputManager)->init(m_instance, m_session);

    if (!setupAppSpace())
        return false;
    if (!setupViewSpace())
        return false;

    if (!createSwapchains())
        return false;

    return true;
}

void QQuick3DXrManagerPrivate::teardown()
{
    m_waitingForFrame = false;
    m_wantUpdate = false;
    destroyWorkerThread();

    if (m_inputManager) {
        QQuick3DXrInputManagerPrivate::get(m_inputManager)->teardown();
        m_inputManager = nullptr;
    }

    if (m_spaceExtension) {
        m_spaceExtension->teardown();
        m_spaceExtension = nullptr;
    }

    if (m_passthroughLayer)
        destroyMetaQuestPassthroughLayer();
    if (m_passthroughFeature)
        destroyMetaQuestPassthrough();

    destroySwapchain();

    if (m_appSpace != XR_NULL_HANDLE) {
        xrDestroySpace(m_appSpace);
    }

    if (m_viewSpace != XR_NULL_HANDLE)
        xrDestroySpace(m_viewSpace);

    xrDestroySession(m_session);

#ifdef XR_EXT_debug_utils
    if (m_debugMessenger) {
        m_xrDestroyDebugUtilsMessengerEXT(m_debugMessenger);
        m_debugMessenger = XR_NULL_HANDLE;
    }
#endif // XR_EXT_debug_utils

    xrDestroyInstance(m_instance);

    // early deinit for graphics, so it can destroy owned QRhi resources
    // Note: Used to be part of the XRmanager dtor.
    if (m_graphics)
        m_graphics->releaseResources();
}

void QQuick3DXrManagerPrivate::initWorkerThread()
{
    Q_Q(QQuick3DXrManager);

    qCDebug(lcQuick3DXr, "Initializing OpenXR worker thread");
    m_workerThread = new QThread();
    m_workerThread->setObjectName(QLatin1StringView(s_workerThreadName));

    m_worker = new QQuick3DOpenXRThreadWorker(m_session);
    m_worker->moveToThread(m_workerThread);

    QObject::connect(
            m_worker,
            &QQuick3DOpenXRThreadWorker::frameWaited,
            q,
            [this](XrResult result, const XrFrameState &frameState) { onFrameWaitCompleted(result, frameState); },
            Qt::QueuedConnection);

    QObject::connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();
}

void QQuick3DXrManagerPrivate::destroyWorkerThread()
{
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        delete m_workerThread;
        m_workerThread = nullptr;
        m_worker = nullptr;
        qCDebug(lcQuick3DXr, "OpenXR worker thread destroyed");
    }
}

void QQuick3DXrManagerPrivate::checkReferenceSpaces()
{
    Q_ASSERT(m_session != XR_NULL_HANDLE);

    uint32_t spaceCount;
    if (!checkXrResult(xrEnumerateReferenceSpaces(m_session, 0, &spaceCount, nullptr))) {
        qWarning("Failed to enumerate reference spaces");
        return;
    }
    m_availableReferenceSpace.resize(spaceCount);
    if (!checkXrResult(xrEnumerateReferenceSpaces(m_session, spaceCount, &spaceCount, m_availableReferenceSpace.data()))) {
        qWarning("Failed to enumerate reference spaces");
        return;
    }

    qCDebug(lcQuick3DXr, "Available reference spaces: %d", spaceCount);
    for (XrReferenceSpaceType space : m_availableReferenceSpace) {
        qCDebug(lcQuick3DXr, "  Name: %s", to_string(space));
    }
}

bool QQuick3DXrManagerPrivate::isReferenceSpaceAvailable(XrReferenceSpaceType type)
{
    return m_availableReferenceSpace.contains(type);
}

bool QQuick3DXrManagerPrivate::setupAppSpace()
{
    Q_Q(QQuick3DXrManager);

    Q_ASSERT(m_session != XR_NULL_HANDLE);

    XrPosef identityPose;
    identityPose.orientation.w = 1;
    identityPose.orientation.x = 0;
    identityPose.orientation.y = 0;
    identityPose.orientation.z = 0;
    identityPose.position.x = 0;
    identityPose.position.y = 0;
    identityPose.position.z = 0;

    XrReferenceSpaceType newReferenceSpace;
    XrSpace newAppSpace = XR_NULL_HANDLE;
    m_isEmulatingLocalFloor = false;

    if (isReferenceSpaceAvailable(m_requestedReferenceSpace)) {
        newReferenceSpace = m_requestedReferenceSpace;
    } else if (m_requestedReferenceSpace == XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT &&
               isReferenceSpaceAvailable(XR_REFERENCE_SPACE_TYPE_STAGE)) {
        m_isEmulatingLocalFloor = true;
        m_isFloorResetPending = true;
        newReferenceSpace = XR_REFERENCE_SPACE_TYPE_LOCAL;
    } else {
        qWarning("Requested reference space is not available");
        newReferenceSpace = XR_REFERENCE_SPACE_TYPE_LOCAL;
    }

    // App Space
    qCDebug(lcQuick3DXr, "Creating new reference space for app space: %s", to_string(newReferenceSpace));
    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{};
    referenceSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    referenceSpaceCreateInfo.poseInReferenceSpace = identityPose;
    referenceSpaceCreateInfo.referenceSpaceType = newReferenceSpace;
    if (!checkXrResult(xrCreateReferenceSpace(m_session, &referenceSpaceCreateInfo, &newAppSpace))) {
        qWarning("Failed to create app space");
        return false;
    }

    if (m_appSpace)
        xrDestroySpace(m_appSpace);

    m_appSpace = newAppSpace;
    m_referenceSpace = newReferenceSpace;
    // only broadcast the reference space change if we are not emulating the local floor
    // since we'll try and change the referenceSpace again once we have tracking
    if (!m_isFloorResetPending)
        emit q->referenceSpaceChanged();

    return true;

}

void QQuick3DXrManagerPrivate::updateAppSpace(XrTime predictedDisplayTime)
{
    Q_Q(QQuick3DXrManager);

    // If the requested reference space is not the current one, we need to
    // re-create the app space now
    if (m_requestedReferenceSpace != m_referenceSpace && !m_isFloorResetPending) {
        if (!setupAppSpace()) {
            // If we can't set the requested reference space, use the current one
            qWarning("Setting requested reference space failed");
            m_requestedReferenceSpace = m_referenceSpace;
            return;
        }
    }

           // This happens when we setup the emulated LOCAL_FLOOR mode
           // We may have requested it on app setup, but we need to have
           // some tracking information to calculate the floor height so
           // that will only happen once we get here.
    if (m_isFloorResetPending) {
        if (!resetEmulatedFloorHeight(predictedDisplayTime)) {
            // It didn't work, so give up and use local space (which is already setup).
            m_requestedReferenceSpace = XR_REFERENCE_SPACE_TYPE_LOCAL;
            emit q->referenceSpaceChanged();
        }
        return;
    }

}

bool QQuick3DXrManagerPrivate::setupViewSpace()
{
    Q_ASSERT(m_session != XR_NULL_HANDLE);

    XrPosef identityPose;
    identityPose.orientation.w = 1;
    identityPose.orientation.x = 0;
    identityPose.orientation.y = 0;
    identityPose.orientation.z = 0;
    identityPose.position.x = 0;
    identityPose.position.y = 0;
    identityPose.position.z = 0;

    XrSpace newViewSpace = XR_NULL_HANDLE;

    XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{};
    referenceSpaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    referenceSpaceCreateInfo.poseInReferenceSpace = identityPose;
    referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
    if (!checkXrResult(xrCreateReferenceSpace(m_session, &referenceSpaceCreateInfo, &newViewSpace))) {
        qWarning("Failed to create view space");
        return false;
    }

    if (m_viewSpace != XR_NULL_HANDLE)
        xrDestroySpace(m_viewSpace);

    m_viewSpace = newViewSpace;

    return true;
}

bool QQuick3DXrManagerPrivate::resetEmulatedFloorHeight(XrTime predictedDisplayTime)
{
    Q_Q(QQuick3DXrManager);

    Q_ASSERT(m_isEmulatingLocalFloor);

    m_isFloorResetPending = false;

    XrPosef identityPose;
    identityPose.orientation.w = 1;
    identityPose.orientation.x = 0;
    identityPose.orientation.y = 0;
    identityPose.orientation.z = 0;
    identityPose.position.x = 0;
    identityPose.position.y = 0;
    identityPose.position.z = 0;

    XrSpace localSpace = XR_NULL_HANDLE;
    XrSpace stageSpace = XR_NULL_HANDLE;

    XrReferenceSpaceCreateInfo createInfo{};
    createInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    createInfo.poseInReferenceSpace = identityPose;

    if (!checkXrResult(xrCreateReferenceSpace(m_session, &createInfo, &localSpace))) {
        qWarning("Failed to create local space (for emulated LOCAL_FLOOR space)");
        return false;
    }

    createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
    if (!checkXrResult(xrCreateReferenceSpace(m_session, &createInfo, &stageSpace))) {
        qWarning("Failed to create stage space (for emulated LOCAL_FLOOR space)");
        xrDestroySpace(localSpace);
        return false;
    }

    XrSpaceLocation stageLocation{};
    stageLocation.type = XR_TYPE_SPACE_LOCATION;
    stageLocation.pose = identityPose;

    if (!checkXrResult(xrLocateSpace(stageSpace, localSpace, predictedDisplayTime, &stageLocation))) {
        qWarning("Failed to locate STAGE space in LOCAL space, in order to emulate LOCAL_FLOOR");
        xrDestroySpace(localSpace);
        xrDestroySpace(stageSpace);
        return false;
    }

    xrDestroySpace(localSpace);
    xrDestroySpace(stageSpace);

    XrSpace newAppSpace = XR_NULL_HANDLE;
    createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    createInfo.poseInReferenceSpace.position.y = stageLocation.pose.position.y;
    if (!checkXrResult(xrCreateReferenceSpace(m_session, &createInfo, &newAppSpace))) {
        qWarning("Failed to recreate emulated LOCAL_FLOOR play space with latest floor estimate");
        return false;
    }

    xrDestroySpace(m_appSpace);
    m_appSpace = newAppSpace;
    m_referenceSpace = XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT;
    emit q->referenceSpaceChanged();

    return true;
}

bool QQuick3DXrManagerPrivate::createSwapchains()
{
    Q_ASSERT(m_session != XR_NULL_HANDLE);
    Q_ASSERT(m_configViews.isEmpty());
    Q_ASSERT(m_swapchains.isEmpty());

    XrSystemProperties systemProperties{};
    systemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;

    XrSystemHandTrackingPropertiesEXT handTrackingSystemProperties{};
    handTrackingSystemProperties.type = XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT;
    systemProperties.next = &handTrackingSystemProperties;

    if (!checkXrResult(xrGetSystemProperties(m_instance, m_systemId, &systemProperties))) {
        qWarning("Failed to get OpenXR system properties");
        return false;
    }
    qCDebug(lcQuick3DXr, "System Properties: Name=%s VendorId=%d", systemProperties.systemName, systemProperties.vendorId);
    qCDebug(lcQuick3DXr, "System Graphics Properties: MaxWidth=%d MaxHeight=%d MaxLayers=%d",
           systemProperties.graphicsProperties.maxSwapchainImageWidth,
           systemProperties.graphicsProperties.maxSwapchainImageHeight,
           systemProperties.graphicsProperties.maxLayerCount);
    qCDebug(lcQuick3DXr, "System Tracking Properties: OrientationTracking=%s PositionTracking=%s",
           systemProperties.trackingProperties.orientationTracking == XR_TRUE ? "True" : "False",
           systemProperties.trackingProperties.positionTracking == XR_TRUE ? "True" : "False");
    qCDebug(lcQuick3DXr, "System Hand Tracking Properties: handTracking=%s",
           handTrackingSystemProperties.supportsHandTracking == XR_TRUE ? "True" : "False");

           // View Config type has to be Stereo, because OpenXR doesn't support any other mode yet.
    quint32 viewCount;
    if (!checkXrResult(xrEnumerateViewConfigurationViews(m_instance,
                                                         m_systemId,
                                                         m_viewConfigType,
                                                         0,
                                                         &viewCount,
                                                         nullptr)))
    {
        qWarning("Failed to enumerate view configurations");
        return false;
    }
    m_configViews.resize(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr, 0, 0, 0, 0, 0, 0});
    if (!checkXrResult(xrEnumerateViewConfigurationViews(m_instance,
                                                         m_systemId,
                                                         m_viewConfigType,
                                                         viewCount,
                                                         &viewCount,
                                                         m_configViews.data())))
    {
        qWarning("Failed to enumerate view configurations");
        return false;
    }
    m_views.resize(viewCount, {XR_TYPE_VIEW, nullptr, {}, {}});
    m_projectionLayerViews.resize(viewCount, {});
    m_layerDepthInfos.resize(viewCount, {});

    // Create the swapchain and get the images.
    if (viewCount > 0) {
        // Select a swapchain format.
        uint32_t swapchainFormatCount;
        if (!checkXrResult(xrEnumerateSwapchainFormats(m_session, 0, &swapchainFormatCount, nullptr))) {
            qWarning("Failed to enumerate swapchain formats");
            return false;
        }
        QVector<int64_t> swapchainFormats(swapchainFormatCount);
        if (!checkXrResult(xrEnumerateSwapchainFormats(m_session,
                                                       swapchainFormats.size(),
                                                       &swapchainFormatCount,
                                                       swapchainFormats.data())))
        {
            qWarning("Failed to enumerate swapchain formats");
            return false;
        }

        Q_ASSERT(static_cast<qsizetype>(swapchainFormatCount) == swapchainFormats.size());
        m_colorSwapchainFormat = m_graphics->colorSwapchainFormat(swapchainFormats);
        if (m_compositionLayerDepthSupported)
            m_depthSwapchainFormat = m_graphics->depthSwapchainFormat(swapchainFormats);

        // Print swapchain formats and the selected one.
        {
            QString swapchainFormatsString;
            for (int64_t format : swapchainFormats) {
                const bool selectedColor = format == m_colorSwapchainFormat;
                const bool selectedDepth = format == m_depthSwapchainFormat;
                swapchainFormatsString += u" ";
                if (selectedColor)
                    swapchainFormatsString += u"[";
                else if (selectedDepth)
                    swapchainFormatsString += u"<";
                swapchainFormatsString += QString::number(format);
                if (selectedColor)
                    swapchainFormatsString += u"]";
                else if (selectedDepth)
                    swapchainFormatsString += u">";
            }
            qCDebug(lcQuick3DXr, "Swapchain formats: %s", qPrintable(swapchainFormatsString));
        }

        const XrViewConfigurationView &vp = m_configViews[0]; // use the first view for all views, the sizes should be the same

        // sampleCount for the XrSwapchain is always 1. We could take m_samples
        // here, clamp it to vp.maxSwapchainSampleCount, and pass it in to the
        // swapchain to get multisample textures (or a multisample texture
        // array) out of the swapchain. This we do not do, because it was only
        // supported with 1 out of 5 OpenXR(+streaming) combination tested on
        // the Quest 3. In most cases, incl. Quest 3 native Android,
        // maxSwapchainSampleCount is 1. Therefore, we do MSAA on our own, and
        // do not rely on the XrSwapchain for this.

        if (m_multiviewRendering) {
            // Create a single swapchain with array size > 1
            XrSwapchainCreateInfo swapchainCreateInfo{};
            swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
            swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_MUTABLE_FORMAT_BIT;
            swapchainCreateInfo.format = m_colorSwapchainFormat;
            swapchainCreateInfo.sampleCount = 1; // we do MSAA on our own, do not need ms textures from the swapchain
            swapchainCreateInfo.width = vp.recommendedImageRectWidth;
            swapchainCreateInfo.height = vp.recommendedImageRectHeight;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.arraySize = viewCount;
            swapchainCreateInfo.mipCount = 1;

            qCDebug(lcQuick3DXr, "Creating multiview swapchain for %u view(s) with dimensions Width=%d Height=%d SampleCount=%d Format=%llx",
                   viewCount,
                   vp.recommendedImageRectWidth,
                   vp.recommendedImageRectHeight,
                   1,
                   static_cast<long long unsigned int>(m_colorSwapchainFormat));

            Swapchain swapchain;
            swapchain.width = swapchainCreateInfo.width;
            swapchain.height = swapchainCreateInfo.height;
            swapchain.arraySize = swapchainCreateInfo.arraySize;
            if (checkXrResult(xrCreateSwapchain(m_session, &swapchainCreateInfo, &swapchain.handle))) {
                uint32_t imageCount = 0;
                if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr))) {
                    qWarning("Failed to enumerate swapchain images");
                    return false;
                }

                auto swapchainImages = m_graphics->allocateSwapchainImages(imageCount, swapchain.handle);
                if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, swapchainImages[0]))) {
                    qWarning("Failed to enumerate swapchain images");
                    return false;
                }

                m_swapchains.append(swapchain);
                m_swapchainImages.insert(swapchain.handle, swapchainImages);
            } else {
                qWarning("xrCreateSwapchain failed (multiview)");
                return false;
            }

            // Create the depth swapchain always when
            // XR_KHR_composition_layer_depth is supported. If we are going to
            // submit (use the depth image), that's a different question, and is
            // dynamically controlled by the user.
            if (m_compositionLayerDepthSupported && m_depthSwapchainFormat > 0) {
                swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                swapchainCreateInfo.format = m_depthSwapchainFormat;
                if (checkXrResult(xrCreateSwapchain(m_session, &swapchainCreateInfo, &swapchain.handle))) {
                    uint32_t imageCount = 0;
                    if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr))) {
                        qWarning("Failed to enumerate depth swapchain images");
                        return false;
                    }

                    auto swapchainImages = m_graphics->allocateSwapchainImages(imageCount, swapchain.handle);
                    if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, swapchainImages[0]))) {
                        qWarning("Failed to enumerate depth swapchain images");
                        return false;
                    }

                    m_depthSwapchains.append(swapchain);
                    m_depthSwapchainImages.insert(swapchain.handle, swapchainImages);
                } else {
                    qWarning("xrCreateSwapchain failed for depth swapchain (multiview)");
                    return false;
                }
            }
        } else {
            // Create a swapchain for each view.
            for (uint32_t i = 0; i < viewCount; i++) {
                qCDebug(lcQuick3DXr, "Creating swapchain for view %u with dimensions Width=%d Height=%d SampleCount=%d Format=%llx",
                       i,
                       vp.recommendedImageRectWidth,
                       vp.recommendedImageRectHeight,
                       1,
                       static_cast<long long unsigned int>(m_colorSwapchainFormat));

                // Create the swapchain.
                XrSwapchainCreateInfo swapchainCreateInfo{};
                swapchainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
                swapchainCreateInfo.arraySize = 1;
                swapchainCreateInfo.format = m_colorSwapchainFormat;
                swapchainCreateInfo.width = vp.recommendedImageRectWidth;
                swapchainCreateInfo.height = vp.recommendedImageRectHeight;
                swapchainCreateInfo.mipCount = 1;
                swapchainCreateInfo.faceCount = 1;
                swapchainCreateInfo.sampleCount = 1; // we do MSAA on our own, do not need ms textures from the swapchain
                swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
                Swapchain swapchain;
                swapchain.width = swapchainCreateInfo.width;
                swapchain.height = swapchainCreateInfo.height;
                if (checkXrResult(xrCreateSwapchain(m_session, &swapchainCreateInfo, &swapchain.handle))) {
                    uint32_t imageCount = 0;
                    if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr))) {
                        qWarning("Failed to enumerate swapchain images");
                        return false;
                    }

                    auto swapchainImages = m_graphics->allocateSwapchainImages(imageCount, swapchain.handle);
                    if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, swapchainImages[0]))) {
                        qWarning("Failed to enumerate swapchain images");
                        return false;
                    }

                    m_swapchains.append(swapchain);
                    m_swapchainImages.insert(swapchain.handle, swapchainImages);
                } else {
                    qWarning("xrCreateSwapchain failed (view %u)", viewCount);
                    return false;
                }

                if (m_compositionLayerDepthSupported && m_depthSwapchainFormat > 0) {
                    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    swapchainCreateInfo.format = m_depthSwapchainFormat;
                    if (checkXrResult(xrCreateSwapchain(m_session, &swapchainCreateInfo, &swapchain.handle))) {
                        uint32_t imageCount = 0;
                        if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr))) {
                            qWarning("Failed to enumerate depth swapchain images");
                            return false;
                        }

                        auto swapchainImages = m_graphics->allocateSwapchainImages(imageCount, swapchain.handle);
                        if (!checkXrResult(xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, swapchainImages[0]))) {
                            qWarning("Failed to enumerate depth swapchain images");
                            return false;
                        }

                        m_depthSwapchains.append(swapchain);
                        m_depthSwapchainImages.insert(swapchain.handle, swapchainImages);
                    } else {
                        qWarning("xrCreateSwapchain failed for depth swapchain (view %u)", viewCount);
                        return false;
                    }
                }
            }
        }

        if (m_multiviewRendering) {
            if (m_swapchains.isEmpty())
                return false;
            if (m_compositionLayerDepthSupported && m_depthSwapchains.isEmpty())
                return false;
        } else {
            if (m_swapchains.count() != qsizetype(viewCount))
                return false;
            if (m_compositionLayerDepthSupported && m_depthSwapchains.count() != qsizetype(viewCount))
                return false;
        }

               // Setup the projection layer views.
        for (uint32_t i = 0; i < viewCount; ++i) {
            m_projectionLayerViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
            m_projectionLayerViews[i].next = nullptr;
            m_projectionLayerViews[i].subImage.swapchain = m_swapchains[0].handle; // for non-multiview this gets overwritten later
            m_projectionLayerViews[i].subImage.imageArrayIndex = i; // this too
            m_projectionLayerViews[i].subImage.imageRect.offset.x = 0;
            m_projectionLayerViews[i].subImage.imageRect.offset.y = 0;
            m_projectionLayerViews[i].subImage.imageRect.extent.width = vp.recommendedImageRectWidth;
            m_projectionLayerViews[i].subImage.imageRect.extent.height = vp.recommendedImageRectHeight;

            if (m_compositionLayerDepthSupported) {
                m_layerDepthInfos[i].type = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR;
                m_layerDepthInfos[i].next = nullptr;
                m_layerDepthInfos[i].subImage.swapchain = m_depthSwapchains[0].handle; // for non-multiview this gets overwritten later
                m_layerDepthInfos[i].subImage.imageArrayIndex = i; // this too
                m_layerDepthInfos[i].subImage.imageRect.offset.x = 0;
                m_layerDepthInfos[i].subImage.imageRect.offset.y = 0;
                m_layerDepthInfos[i].subImage.imageRect.extent.width = vp.recommendedImageRectWidth;
                m_layerDepthInfos[i].subImage.imageRect.extent.height = vp.recommendedImageRectHeight;
            }
        }
    }

    if (m_foveationExtensionSupported)
        setupMetaQuestFoveation();

    return true;
}

void QQuick3DXrManagerPrivate::setSamples(int samples)
{
    if (m_samples == samples)
        return;

    m_samples = samples;

    // No need to do anything more here (such as destroying and recreating the
    // XrSwapchain) since we do not do MSAA through the swapchain.
}

QStringList QQuick3DXrManagerPrivate::enabledExtensions() const
{
    return m_enabledExtensions;
}

QString QQuick3DXrManagerPrivate::runtimeName() const
{
    return m_runtimeName;
}

QVersionNumber QQuick3DXrManagerPrivate::runtimeVersion() const
{
    return m_runtimeVersion;
}

void QQuick3DXrManagerPrivate::setMultiViewRenderingEnabled(bool enable)
{
    Q_Q(QQuick3DXrManager);
    QRhi *rhi = q->m_renderControl->rhi();
    if (m_multiviewRendering == enable || !rhi)
        return;
    if (enable && !rhi->isFeatureSupported(QRhi::MultiView)) {
        qWarning("Quick 3D XR: Multiview rendering was enabled, but is reported as unsupported from the current QRhi backend (%s)",
                 rhi->backendName());
        return;
    }
    m_multiviewRendering = enable;
    qCDebug(lcQuick3DXr, "Multiview rendering %s", m_multiviewRendering ? "enabled" : "disabled");
    if (!m_swapchains.isEmpty()) {
        qCDebug(lcQuick3DXr, "OpenXR swapchain already exists, creating new one due to change in multiview mode");
        destroySwapchain();
        createSwapchains();

        emit q->multiViewRenderingEnabledChanged();
    }
}

bool QQuick3DXrManagerPrivate::isPassthroughEnabled() const
{
    // FB passthrough is enabled
    if (m_fbPassthroughEnabled)
        return true;

    // Blend mode passthrough is enabled (ALPHA_BLEND or ADDITIVE)
    return m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND
        || m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;
}

void QQuick3DXrManagerPrivate::setFBPassthroughEnabled(bool enable)
{
    m_fbPassthroughEnabled = enable;
    if (enable) {
        if (m_passthroughFeature == XR_NULL_HANDLE)
            createMetaQuestPassthrough();
        else
            startMetaQuestPassthrough();

        if (m_passthroughLayer == XR_NULL_HANDLE)
            createMetaQuestPassthroughLayer();
        else
            resumeMetaQuestPassthroughLayer();
    } else {
        if (m_passthroughLayer)
            pauseMetaQuestPassthroughLayer();
        if (m_passthroughFeature)
            pauseMetaQuestPassthrough();
    }
}

bool QQuick3DXrManagerPrivate::setTransparentBlendMode(bool enable)
{
    if (enable) {
        if (m_blendModes.contains(XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND))
            m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND;
        else if (m_blendModes.contains(XR_ENVIRONMENT_BLEND_MODE_ADDITIVE))
            m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;
        else
            return false;
    } else {
        if (m_blendModes.contains(XR_ENVIRONMENT_BLEND_MODE_OPAQUE))
            m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        else
            return false;
    }
    qCDebug(lcQuick3DXr, "Environment blend mode set to: %s", to_string(m_environmentBlendMode));
    return true;
}

bool QQuick3DXrManagerPrivate::setPassthroughEnabled(bool enable)
{
    if (supportsFBPassthrough()) {
        setFBPassthroughEnabled(enable);
        return true;
    }

    return setTransparentBlendMode(enable);
}

void QQuick3DXrManagerPrivate::setDepthSubmissionEnabled(bool enable)
{
    if (m_submitLayerDepth == enable)
        return;

    if (m_compositionLayerDepthSupported) {
        if (enable)
            qCDebug(lcQuick3DXr, "Enabling submitLayerDepth");

        m_submitLayerDepth = enable;
    }
}

void QQuick3DXrManagerPrivate::setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace)
{
    XrReferenceSpaceType referenceSpace = getXrReferenceSpaceType(newReferenceSpace);
    if (m_referenceSpace == referenceSpace)
        return;

    m_requestedReferenceSpace = referenceSpace;

    // we do not emit a changed signal here because it hasn't
    // changed yet.
}

QtQuick3DXr::ReferenceSpace QQuick3DXrManagerPrivate::getReferenceSpace() const
{
    return getReferenceSpaceType(m_referenceSpace);
}

void QQuick3DXrManagerPrivate::getDefaultClipDistances(float &nearClip, float &farClip) const
{
    // Hardcoded defaults
    nearClip = 1.0f;
    farClip = 10000.0f;
}

void QQuick3DXrManagerPrivate::pollEvents(bool *exitRenderLoop, bool *requestRestart) {
    *exitRenderLoop = false;
    *requestRestart = false;

    auto readNextEvent = [this]() {
        // It is sufficient to clear the just the XrEventDataBuffer header to
        // XR_TYPE_EVENT_DATA_BUFFER
        XrEventDataBaseHeader* baseHeader = reinterpret_cast<XrEventDataBaseHeader*>(&m_eventDataBuffer);
        *baseHeader = {XR_TYPE_EVENT_DATA_BUFFER, nullptr};
        const XrResult xr = xrPollEvent(m_instance, &m_eventDataBuffer);
        if (xr == XR_SUCCESS) {
            if (baseHeader->type == XR_TYPE_EVENT_DATA_EVENTS_LOST) {
                const XrEventDataEventsLost* const eventsLost = reinterpret_cast<const XrEventDataEventsLost*>(baseHeader);
                qCDebug(lcQuick3DXr, "%d events lost", eventsLost->lostEventCount);
            }

            return baseHeader;
        }

        return static_cast<XrEventDataBaseHeader*>(nullptr);
    };

    auto handleSessionStateChangedEvent = [this](const XrEventDataSessionStateChanged& stateChangedEvent,
                                                 bool* exitRenderLoop,
                                                 bool* requestRestart) {
        const XrSessionState oldState = m_sessionState;
        m_sessionState = stateChangedEvent.state;

        qCDebug(lcQuick3DXr, "XrEventDataSessionStateChanged: state %s->%s time=%lld",
               to_string(oldState),
               to_string(m_sessionState),
               static_cast<long long int>(stateChangedEvent.time));

        if ((stateChangedEvent.session != XR_NULL_HANDLE) && (stateChangedEvent.session != m_session)) {
            qCDebug(lcQuick3DXr, "XrEventDataSessionStateChanged for unknown session");
            return;
        }

        switch (m_sessionState) {
        case XR_SESSION_STATE_READY: {
            Q_ASSERT(m_session != XR_NULL_HANDLE);
            XrSessionBeginInfo sessionBeginInfo{};
            sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
            sessionBeginInfo.primaryViewConfigurationType = m_viewConfigType;
            if (!checkXrResult(xrBeginSession(m_session, &sessionBeginInfo))) {
                qWarning("xrBeginSession failed");
                break;
            }
            m_sessionRunning = true;
            break;
        }
        case XR_SESSION_STATE_STOPPING: {
            Q_ASSERT(m_session != XR_NULL_HANDLE);
            m_sessionRunning = false;
            if (!checkXrResult(xrEndSession(m_session)))
                qWarning("xrEndSession failed");
            break;
        }
        case XR_SESSION_STATE_EXITING: {
            *exitRenderLoop = true;
            // Do not attempt to restart because user closed this session.
            *requestRestart = false;
            break;
        }
        case XR_SESSION_STATE_LOSS_PENDING: {
            *exitRenderLoop = true;
            // Poll for a new instance.
            *requestRestart = true;
            break;
        }
        default:
            break;
        }
    };

    // Process all pending messages.
    while (const XrEventDataBaseHeader* event = readNextEvent()) {
        switch (event->type) {
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
            const auto& instanceLossPending = *reinterpret_cast<const XrEventDataInstanceLossPending*>(event);
            qCDebug(lcQuick3DXr, "XrEventDataInstanceLossPending by %lld", static_cast<long long int>(instanceLossPending.lossTime));
            *exitRenderLoop = true;
            *requestRestart = true;
            return;
        }
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            auto sessionStateChangedEvent = *reinterpret_cast<const XrEventDataSessionStateChanged*>(event);
            handleSessionStateChangedEvent(sessionStateChangedEvent, exitRenderLoop, requestRestart);
            break;
        }
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
            break;
        case XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB:
        case XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB:
        case XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB:
        case XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB:
            // Handle these events in the space extension
            if (m_spaceExtension)
                m_spaceExtension->handleEvent(event);
            break;
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
        default: {
            qCDebug(lcQuick3DXr, "Ignoring event type %d", event->type);
            break;
        }
        }
    }
}

bool QQuick3DXrManagerPrivate::renderLayer(XrTime predictedDisplayTime,
                                           XrDuration predictedDisplayPeriod,
                                           XrCompositionLayerProjection &layer)
{
    Q_Q(QQuick3DXrManager);
    auto *xrOrigin = q->m_xrOrigin;
    auto *animationDriver = q->m_animationDriver;
    auto *renderControl = q->m_renderControl;

    XrResult res;

    XrViewState viewState{};
    viewState.type = XR_TYPE_VIEW_STATE;
    quint32 viewCapacityInput = m_views.size();
    quint32 viewCountOutput;

    // Check if we need to update the app space before we use it
    updateAppSpace(predictedDisplayTime);

    XrViewLocateInfo viewLocateInfo{};
    viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
    viewLocateInfo.viewConfigurationType = m_viewConfigType;
    viewLocateInfo.displayTime = predictedDisplayTime;
    viewLocateInfo.space = m_appSpace;

    res = xrLocateViews(m_session, &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, m_views.data());
    if (XR_UNQUALIFIED_SUCCESS(res)) {
        Q_ASSERT(viewCountOutput == viewCapacityInput);
        Q_ASSERT(static_cast<qsizetype>(viewCountOutput) == m_configViews.size());
        Q_ASSERT(static_cast<qsizetype>(viewCountOutput) == m_projectionLayerViews.size());
        Q_ASSERT(m_multiviewRendering ? viewCountOutput == m_swapchains[0].arraySize : static_cast<qsizetype>(viewCountOutput) == m_swapchains.size());

        // Update the camera/head position
        XrSpaceLocation location{};
        location.type = XR_TYPE_SPACE_LOCATION;
        if (checkXrResult(xrLocateSpace(m_viewSpace, m_appSpace, predictedDisplayTime, &location))) {
            QVector3D position = QVector3D(location.pose.position.x,
                                           location.pose.position.y,
                                           location.pose.position.z) * 100.0f; // convert m to cm
            QQuaternion rotation(location.pose.orientation.w,
                                 location.pose.orientation.x,
                                 location.pose.orientation.y,
                                 location.pose.orientation.z);

            xrOrigin->updateTrackedCamera(position, rotation);
        }

        // Set the hand positions
        if (QSSG_GUARD(m_inputManager != nullptr))
            QQuick3DXrInputManagerPrivate::get(m_inputManager)->updatePoses(predictedDisplayTime, m_appSpace);

        // Spatial Anchors
        if (m_spaceExtension)
            m_spaceExtension->updateAnchors(predictedDisplayTime, m_appSpace);

        if (m_handtrackingExtensionSupported && m_inputManager)
            QQuick3DXrInputManagerPrivate::get(m_inputManager)->updateHandtracking(predictedDisplayTime, m_appSpace, m_handtrackingAimExtensionSupported);

        // Before rendering individual views, advance the animation driver once according
        // to the expected display time

        const qint64 displayPeriodMS = predictedDisplayPeriod / 1000000;
        const qint64 displayDeltaMS = (predictedDisplayTime - m_previousTime) / 1000000;

        if (m_previousTime == 0 || !animationDriver->isRunning()) {
            animationDriver->setStep(displayPeriodMS);
        } else {
            if (displayDeltaMS > displayPeriodMS)
                animationDriver->setStep(displayPeriodMS);
            else
                animationDriver->setStep(displayDeltaMS);
            animationDriver->advance();
        }
        m_previousTime = predictedDisplayTime;

#if QT_CONFIG(graphicsframecapture)
        if (m_frameCapture)
            m_frameCapture->startCaptureFrame();
#endif

        if (m_submitLayerDepth && m_samples > 1) {
            if (!renderControl->rhi()->isFeatureSupported(QRhi::ResolveDepthStencil)) {
                static bool warned = false;
                if (!warned) {
                    warned = true;
                    qWarning("Quick3D XR: Submitting depth buffer with MSAA cannot be enabled"
                             " when depth-stencil resolve is not supported by the underlying 3D API (%s)",
                             renderControl->rhi()->backendName());
                }
                m_submitLayerDepth = false;
            }
        }

        if (m_multiviewRendering) {
            const Swapchain swapchain = m_swapchains[0];

            // Acquire the swapchain image array
            XrSwapchainImageAcquireInfo acquireInfo{};
            acquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
            uint32_t swapchainImageIndex = 0;
            if (!checkXrResult(xrAcquireSwapchainImage(swapchain.handle, &acquireInfo, &swapchainImageIndex))) {
                qWarning("Failed to acquire swapchain image (multiview)");
                return false;
            }
            XrSwapchainImageWaitInfo waitInfo{};
            waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
            waitInfo.timeout = XR_INFINITE_DURATION;
            if (!checkXrResult(xrWaitSwapchainImage(swapchain.handle, &waitInfo))) {
                qWarning("Failed to wait for swapchain image (multiview)");
                return false;
            }
            XrSwapchainImageBaseHeader *swapchainImage = m_swapchainImages[swapchain.handle][swapchainImageIndex];

            XrSwapchainImageBaseHeader *depthSwapchainImage = nullptr;
            if (m_submitLayerDepth && !m_depthSwapchains.isEmpty()) {
                if (checkXrResult(xrAcquireSwapchainImage(m_depthSwapchains[0].handle, &acquireInfo, &swapchainImageIndex))) {
                    if (checkXrResult(xrWaitSwapchainImage(m_depthSwapchains[0].handle, &waitInfo)))
                        depthSwapchainImage = m_depthSwapchainImages[m_depthSwapchains[0].handle][swapchainImageIndex];
                    else
                        qWarning("Failed to wait for depth swapchain image (multiview)");
                } else {
                    qWarning("Failed to acquire depth swapchain image (multiview)");
                }
            }

            // First update both cameras with the latest view information and
            // then set them on the viewport (since this is going to be
            // multiview rendering).
            for (uint32_t i = 0; i < viewCountOutput; i++) {
                // subImage.swapchain and imageArrayIndex are already set and correct
                m_projectionLayerViews[i].pose = m_views[i].pose;
                m_projectionLayerViews[i].fov = m_views[i].fov;
            }
            updateCameraMultiview(0, viewCountOutput);

            // Perform the rendering. In multiview mode it is done just once,
            // targeting all the views (outputting simultaneously to all texture
            // array layers). The subImage dimensions are the same, that's why
            // passing in the first layerView's subImage works.
            doRender(m_projectionLayerViews[0].subImage,
                     swapchainImage,
                     depthSwapchainImage);

            for (uint32_t i = 0; i < viewCountOutput; i++) {
                if (m_submitLayerDepth) {
                    m_layerDepthInfos[i].minDepth = 0;
                    m_layerDepthInfos[i].maxDepth = 1;
                    QQuick3DXrEyeCamera *cam = xrOrigin ? xrOrigin->eyeCamera(i) : nullptr;
                    m_layerDepthInfos[i].nearZ = cam ? cam->clipNear() : 1.0f;
                    m_layerDepthInfos[i].farZ = cam ? cam->clipFar() : 10000.0f;
                    m_projectionLayerViews[i].next = &m_layerDepthInfos[i];
                } else {
                    m_projectionLayerViews[i].next = nullptr;
                }
            }

            // release the swapchain image array
            XrSwapchainImageReleaseInfo releaseInfo{};
            releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
            if (!checkXrResult(xrReleaseSwapchainImage(swapchain.handle, &releaseInfo)))
                qWarning("Failed to release swapchain image");
            if (depthSwapchainImage) {
                if (!checkXrResult(xrReleaseSwapchainImage(m_depthSwapchains[0].handle, &releaseInfo)))
                    qWarning("Failed to release depth swapchain image");
            }
        } else {
            for (uint32_t i = 0; i < viewCountOutput; i++) {
                // Each view has a separate swapchain which is acquired, rendered to, and released.
                const Swapchain viewSwapchain = m_swapchains[i];

                // Render view to the appropriate part of the swapchain image.
                XrSwapchainImageAcquireInfo acquireInfo{};
                acquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
                uint32_t swapchainImageIndex = 0;
                if (!checkXrResult(xrAcquireSwapchainImage(viewSwapchain.handle, &acquireInfo, &swapchainImageIndex))) {
                    qWarning("Failed to acquire swapchain image");
                    return false;
                }
                XrSwapchainImageWaitInfo waitInfo{};
                waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
                waitInfo.timeout = XR_INFINITE_DURATION;
                if (!checkXrResult(xrWaitSwapchainImage(viewSwapchain.handle, &waitInfo))) {
                    qWarning("Failed to wait for swapchain image");
                    return false;
                }
                XrSwapchainImageBaseHeader *swapchainImage = m_swapchainImages[viewSwapchain.handle][swapchainImageIndex];

                XrSwapchainImageBaseHeader *depthSwapchainImage = nullptr;
                if (m_submitLayerDepth && !m_depthSwapchains.isEmpty()) {
                    if (checkXrResult(xrAcquireSwapchainImage(m_depthSwapchains[i].handle, &acquireInfo, &swapchainImageIndex))) {
                        if (checkXrResult(xrWaitSwapchainImage(m_depthSwapchains[i].handle, &waitInfo)))
                            depthSwapchainImage = m_depthSwapchainImages[m_depthSwapchains[i].handle][swapchainImageIndex];
                        else
                            qWarning("Failed to wait for depth swapchain image");
                    } else {
                        qWarning("Failed to acquire depth swapchain image");
                    }
                }

                m_projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
                m_projectionLayerViews[i].subImage.imageArrayIndex = 0;
                m_projectionLayerViews[i].pose = m_views[i].pose;
                m_projectionLayerViews[i].fov = m_views[i].fov;

                updateCameraNonMultiview(i, m_projectionLayerViews[i]);

                doRender(m_projectionLayerViews[i].subImage,
                         swapchainImage,
                         depthSwapchainImage);

                if (depthSwapchainImage) {
                    m_layerDepthInfos[i].subImage.swapchain = m_depthSwapchains[i].handle;
                    m_layerDepthInfos[i].subImage.imageArrayIndex = 0;
                    m_layerDepthInfos[i].minDepth = 0;
                    m_layerDepthInfos[i].maxDepth = 1;
                    QQuick3DXrEyeCamera *cam = xrOrigin ? xrOrigin->eyeCamera(i) : nullptr;
                    m_layerDepthInfos[i].nearZ = cam ? cam->clipNear() : 1.0f;
                    m_layerDepthInfos[i].farZ = cam ? cam->clipFar() : 10000.0f;
                    m_projectionLayerViews[i].next = &m_layerDepthInfos[i];
                } else {
                    m_projectionLayerViews[i].next = nullptr;
                }

                XrSwapchainImageReleaseInfo releaseInfo{};
                releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
                if (!checkXrResult(xrReleaseSwapchainImage(viewSwapchain.handle, &releaseInfo)))
                    qWarning("Failed to release swapchain image");
                if (depthSwapchainImage) {
                    if (!checkXrResult(xrReleaseSwapchainImage(m_depthSwapchains[i].handle, &releaseInfo)))
                        qWarning("Failed to release depth swapchain image");
                }
            }
        }

#if QT_CONFIG(graphicsframecapture)
        if (m_frameCapture)
            m_frameCapture->endCaptureFrame();
#endif

        layer.space = m_appSpace;
        layer.viewCount = (uint32_t)m_projectionLayerViews.size();
        layer.views = m_projectionLayerViews.data();
        return true;
    }

    qCDebug(lcQuick3DXr, "xrLocateViews returned qualified success code: %s", to_string(res));
    return false;
}

void QQuick3DXrManagerPrivate::doRender(const XrSwapchainSubImage &subImage,
                                        const XrSwapchainImageBaseHeader *swapchainImage,
                                        const XrSwapchainImageBaseHeader *depthSwapchainImage)
{
    Q_Q(QQuick3DXrManager);

    auto *quickWindow = q->m_quickWindow;
    auto *renderControl = q->m_renderControl;

    const int arraySize = m_multiviewRendering ? m_swapchains[0].arraySize : 1;
    quickWindow->setRenderTarget(m_graphics->renderTarget(subImage,
                                                            swapchainImage,
                                                            m_colorSwapchainFormat,
                                                            m_samples,
                                                            arraySize,
                                                            depthSwapchainImage,
                                                            m_depthSwapchainFormat));

    quickWindow->setGeometry(0,
                               0,
                               subImage.imageRect.extent.width,
                               subImage.imageRect.extent.height);
    quickWindow->contentItem()->setSize(QSizeF(subImage.imageRect.extent.width,
                                                 subImage.imageRect.extent.height));

    renderControl->polishItems();
    renderControl->beginFrame();
    renderControl->sync();
    renderControl->render();
    renderControl->endFrame();

    // With multiview this indicates that the frame with both eyes is ready from
    // the 3D APIs perspective. Without multiview this is done - and so the
    // signal is emitted - multiple times (twice) per "frame" (eye).
    QRhiRenderTarget *rt = QQuickWindowPrivate::get(quickWindow)->activeCustomRhiRenderTarget();
    if (rt->resourceType() == QRhiResource::TextureRenderTarget && static_cast<QRhiTextureRenderTarget *>(rt)->description().colorAttachmentAt(0)->texture())
        emit q->frameReady();
}

void QQuick3DXrManagerPrivate::setupMetaQuestColorSpaces()
{
    PFN_xrEnumerateColorSpacesFB pfnxrEnumerateColorSpacesFB = NULL;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrEnumerateColorSpacesFB", (PFN_xrVoidFunction*)(&pfnxrEnumerateColorSpacesFB));
    if (!pfnxrEnumerateColorSpacesFB) // simulator
        return;

    uint32_t colorSpaceCountOutput = 0;
    if (!checkXrResult(pfnxrEnumerateColorSpacesFB(m_session, 0, &colorSpaceCountOutput, nullptr))) {
        qWarning("Failed to enumerate color spaces");
        return;
    }

    XrColorSpaceFB* colorSpaces =
            (XrColorSpaceFB*)malloc(colorSpaceCountOutput * sizeof(XrColorSpaceFB));

    if (!checkXrResult(pfnxrEnumerateColorSpacesFB(m_session, colorSpaceCountOutput, &colorSpaceCountOutput, colorSpaces))) {
        qWarning("Failed to enumerate color spaces");
        return;
    }

    qCDebug(lcQuick3DXr, "Supported color spaces:");
    for (uint32_t i = 0; i < colorSpaceCountOutput; i++) {
        qCDebug(lcQuick3DXr,"%d:%d", i, colorSpaces[i]);
    }

    const XrColorSpaceFB requestColorSpace = XR_COLOR_SPACE_QUEST_FB;

    PFN_xrSetColorSpaceFB pfnxrSetColorSpaceFB = NULL;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrSetColorSpaceFB", (PFN_xrVoidFunction*)(&pfnxrSetColorSpaceFB));

    if (!checkXrResult(pfnxrSetColorSpaceFB(m_session, requestColorSpace)))
        qWarning("Failed to set color space");

    free(colorSpaces);
}

void QQuick3DXrManagerPrivate::setupMetaQuestRefreshRates()
{
    PFN_xrEnumerateDisplayRefreshRatesFB pfnxrEnumerateDisplayRefreshRatesFB = NULL;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrEnumerateDisplayRefreshRatesFB", (PFN_xrVoidFunction*)(&pfnxrEnumerateDisplayRefreshRatesFB));
    if (!pfnxrEnumerateDisplayRefreshRatesFB)
        return;

    uint32_t numSupportedDisplayRefreshRates;
    QVector<float> supportedDisplayRefreshRates;

    if (!checkXrResult(pfnxrEnumerateDisplayRefreshRatesFB(m_session, 0, &numSupportedDisplayRefreshRates, nullptr))) {
        qWarning("Failed to enumerate display refresh rates");
        return;
    }

    supportedDisplayRefreshRates.resize(numSupportedDisplayRefreshRates);

    if (!checkXrResult(pfnxrEnumerateDisplayRefreshRatesFB(
            m_session,
            numSupportedDisplayRefreshRates,
            &numSupportedDisplayRefreshRates,
            supportedDisplayRefreshRates.data())))
    {
        qWarning("Failed to enumerate display refresh rates");
        return;
    }

    qCDebug(lcQuick3DXr, "Supported Refresh Rates:");
    for (uint32_t i = 0; i < numSupportedDisplayRefreshRates; i++) {
        qCDebug(lcQuick3DXr, "%d:%f", i, supportedDisplayRefreshRates[i]);
    }

    PFN_xrGetDisplayRefreshRateFB pfnGetDisplayRefreshRate;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrGetDisplayRefreshRateFB", (PFN_xrVoidFunction*)(&pfnGetDisplayRefreshRate));

    float currentDisplayRefreshRate = 0.0f;
    if (!checkXrResult(pfnGetDisplayRefreshRate(m_session, &currentDisplayRefreshRate)))
        qWarning("Failed to get display refresh rate");

    qCDebug(lcQuick3DXr, "Current System Display Refresh Rate: %f", currentDisplayRefreshRate);

    PFN_xrRequestDisplayRefreshRateFB pfnRequestDisplayRefreshRate;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrRequestDisplayRefreshRateFB", (PFN_xrVoidFunction*)(&pfnRequestDisplayRefreshRate));

    // Test requesting the system default.
    if (!checkXrResult(pfnRequestDisplayRefreshRate(m_session, 0.0f)))
        qWarning("Failed to request display refresh rate");

    qCDebug(lcQuick3DXr, "Requesting system default display refresh rate");
}

void QQuick3DXrManagerPrivate::setupMetaQuestFoveation()
{
    PFN_xrCreateFoveationProfileFB pfnCreateFoveationProfileFB;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrCreateFoveationProfileFB", (PFN_xrVoidFunction*)(&pfnCreateFoveationProfileFB));
    if (!pfnCreateFoveationProfileFB) // simulator
        return;

    PFN_xrDestroyFoveationProfileFB pfnDestroyFoveationProfileFB;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrDestroyFoveationProfileFB", (PFN_xrVoidFunction*)(&pfnDestroyFoveationProfileFB));

    PFN_xrUpdateSwapchainFB pfnUpdateSwapchainFB;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrUpdateSwapchainFB", (PFN_xrVoidFunction*)(&pfnUpdateSwapchainFB));

    for (auto swapchain : m_swapchains) {
        XrFoveationLevelProfileCreateInfoFB levelProfileCreateInfo = {};
        levelProfileCreateInfo.type = XR_TYPE_FOVEATION_LEVEL_PROFILE_CREATE_INFO_FB;
        levelProfileCreateInfo.level = m_foveationLevel;
        levelProfileCreateInfo.verticalOffset = 0;
        levelProfileCreateInfo.dynamic = XR_FOVEATION_DYNAMIC_DISABLED_FB;

        XrFoveationProfileCreateInfoFB profileCreateInfo = {};
        profileCreateInfo.type = XR_TYPE_FOVEATION_PROFILE_CREATE_INFO_FB;
        profileCreateInfo.next = &levelProfileCreateInfo;

        XrFoveationProfileFB foveationProfile;
        pfnCreateFoveationProfileFB(m_session, &profileCreateInfo, &foveationProfile);

        XrSwapchainStateFoveationFB foveationUpdateState = {};
        memset(&foveationUpdateState, 0, sizeof(foveationUpdateState));
        foveationUpdateState.type = XR_TYPE_SWAPCHAIN_STATE_FOVEATION_FB;
        foveationUpdateState.profile = foveationProfile;

        XrResult updateSwapchainFBResult = OpenXRHelpers::safeCall(pfnUpdateSwapchainFB, swapchain.handle, (const XrSwapchainStateBaseHeaderFB*)(&foveationUpdateState));
        if (updateSwapchainFBResult == XR_ERROR_FUNCTION_UNSUPPORTED)
            return;

        XrResult destroyFoveationProfileFBResult = OpenXRHelpers::safeCall(pfnDestroyFoveationProfileFB, foveationProfile);
        if (destroyFoveationProfileFBResult == XR_ERROR_FUNCTION_UNSUPPORTED)
            return;

        qCDebug(lcQuick3DXr, "Fixed foveated rendering requested with level %d", int(m_foveationLevel));
    }
}

void QQuick3DXrManagerPrivate::createMetaQuestPassthrough()
{
    // According to the validation layer 'flags' cannot be 0, thus we make sure
    // this function is only ever called when we know passthrough is actually
    // enabled by the app.
    Q_ASSERT(m_fbPassthroughEnabled);

    PFN_xrCreatePassthroughFB pfnXrCreatePassthroughFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrCreatePassthroughFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughFBX));

    XrPassthroughCreateInfoFB passthroughCreateInfo{};
    passthroughCreateInfo.type = XR_TYPE_PASSTHROUGH_CREATE_INFO_FB;
    passthroughCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;

    XrResult xrCreatePassthroughFBXResult = OpenXRHelpers::safeCall(pfnXrCreatePassthroughFBX, m_session, static_cast<const XrPassthroughCreateInfoFB*>(&passthroughCreateInfo), &m_passthroughFeature);
    if (!checkXrResult(xrCreatePassthroughFBXResult))
        qWarning("Failed to create passthrough object");
}

void QQuick3DXrManagerPrivate::destroyMetaQuestPassthrough()
{
    PFN_xrDestroyPassthroughFB pfnXrDestroyPassthroughFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrDestroyPassthroughFB", (PFN_xrVoidFunction*)(&pfnXrDestroyPassthroughFBX));

    XrResult xrDestroyPassthroughFBXResult = OpenXRHelpers::safeCall(pfnXrDestroyPassthroughFBX, m_passthroughFeature);
    if (!checkXrResult(xrDestroyPassthroughFBXResult))
        qWarning("Failed to destroy passthrough object");

    m_passthroughFeature = XR_NULL_HANDLE;
}

void QQuick3DXrManagerPrivate::startMetaQuestPassthrough()
{
    PFN_xrPassthroughStartFB pfnXrPassthroughStartFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrPassthroughStartFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughStartFBX));

    XrResult xrPassthroughStartFBXResult = OpenXRHelpers::safeCall(pfnXrPassthroughStartFBX, m_passthroughFeature);
    if (!checkXrResult(xrPassthroughStartFBXResult))
        qWarning("Failed to start passthrough");
}

void QQuick3DXrManagerPrivate::pauseMetaQuestPassthrough()
{
    PFN_xrPassthroughPauseFB pfnXrPassthroughPauseFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrPassthroughPauseFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughPauseFBX));

    XrResult xrPassthroughPauseFBXResult = OpenXRHelpers::safeCall(pfnXrPassthroughPauseFBX, m_passthroughFeature);
    if (!checkXrResult(xrPassthroughPauseFBXResult))
        qWarning("Failed to pause passthrough");
}

void QQuick3DXrManagerPrivate::createMetaQuestPassthroughLayer()
{
    PFN_xrCreatePassthroughLayerFB pfnXrCreatePassthroughLayerFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrCreatePassthroughLayerFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughLayerFBX));

    XrPassthroughLayerCreateInfoFB layerCreateInfo{};
    layerCreateInfo.type = XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB;
    layerCreateInfo.passthrough = m_passthroughFeature;
    layerCreateInfo.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
    if (m_fbPassthroughEnabled)
        layerCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;

    XrResult xrCreatePassthroughLayerFBXResult = OpenXRHelpers::safeCall(pfnXrCreatePassthroughLayerFBX, m_session, static_cast<const XrPassthroughLayerCreateInfoFB*>(&layerCreateInfo), &m_passthroughLayer);
    if (!checkXrResult(xrCreatePassthroughLayerFBXResult))
        qWarning("Failed to create passthrough layer");
}

void QQuick3DXrManagerPrivate::destroyMetaQuestPassthroughLayer()
{
    PFN_xrDestroyPassthroughLayerFB pfnXrDestroyPassthroughLayerFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrDestroyPassthroughLayerFB", (PFN_xrVoidFunction*)(&pfnXrDestroyPassthroughLayerFBX));

    XrResult xrDestroyPassthroughLayerFBXResult = OpenXRHelpers::safeCall(pfnXrDestroyPassthroughLayerFBX, m_passthroughLayer);
    if (!checkXrResult(xrDestroyPassthroughLayerFBXResult))
        qWarning("Failed to destroy passthrough layer");

    m_passthroughLayer = XR_NULL_HANDLE;
}

void QQuick3DXrManagerPrivate::pauseMetaQuestPassthroughLayer()
{
    PFN_xrPassthroughLayerPauseFB pfnXrPassthroughLayerPauseFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrPassthroughLayerPauseFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughLayerPauseFBX));

    XrResult xrPassthroughLayerPauseFBXResult = OpenXRHelpers::safeCall(pfnXrPassthroughLayerPauseFBX, m_passthroughLayer);
    if (!checkXrResult(xrPassthroughLayerPauseFBXResult))
        qWarning("Failed to pause passthrough layer");
}

void QQuick3DXrManagerPrivate::resumeMetaQuestPassthroughLayer()
{
    PFN_xrPassthroughLayerResumeFB pfnXrPassthroughLayerResumeFBX = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrPassthroughLayerResumeFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughLayerResumeFBX));

    XrResult xrPassthroughLayerResumeFBXResult = OpenXRHelpers::safeCall(pfnXrPassthroughLayerResumeFBX, m_passthroughLayer);
    if (!checkXrResult(xrPassthroughLayerResumeFBXResult))
        qWarning("Failed to resume passthrough layer");
}

void QQuick3DXrManagerPrivate::checkXrExtensions(const char *layerName, int indent)
{
    quint32 instanceExtensionCount;
    if (!checkXrResult(xrEnumerateInstanceExtensionProperties(layerName, 0, &instanceExtensionCount, nullptr))) {
        qWarning("Failed to enumerate instance extension properties");
        return;
    }

    QVector<XrExtensionProperties> extensions(instanceExtensionCount);
    for (XrExtensionProperties& extension : extensions) {
        extension.type = XR_TYPE_EXTENSION_PROPERTIES;
        extension.next = nullptr;
    }

    if (!checkXrResult(xrEnumerateInstanceExtensionProperties(layerName,
                                                              quint32(extensions.size()),
                                                              &instanceExtensionCount,
                                                              extensions.data())))
    {
        qWarning("Failed to enumerate instance extension properties");
    }

    const QByteArray indentStr(indent, ' ');
    qCDebug(lcQuick3DXr, "%sAvailable Extensions: (%d)", indentStr.data(), instanceExtensionCount);
    for (const XrExtensionProperties& extension : extensions) {
        qCDebug(lcQuick3DXr, "%s  Name=%s Version=%d.%d.%d",
               indentStr.data(),
               extension.extensionName,
               XR_VERSION_MAJOR(extension.extensionVersion),
               XR_VERSION_MINOR(extension.extensionVersion),
               XR_VERSION_PATCH(extension.extensionVersion));
    }
}

void QQuick3DXrManagerPrivate::checkXrLayers()
{
    quint32 layerCount;
    if (!checkXrResult(xrEnumerateApiLayerProperties(0, &layerCount, nullptr))) {
        qWarning("Failed to enumerate API layer properties");
        return;
    }

    QVector<XrApiLayerProperties> layers(layerCount);
    for (XrApiLayerProperties& layer : layers) {
        layer.type = XR_TYPE_API_LAYER_PROPERTIES;
        layer.next = nullptr;
    }

    if (!checkXrResult(xrEnumerateApiLayerProperties(quint32(layers.size()), &layerCount, layers.data()))) {
        qWarning("Failed to enumerate API layer properties");
        return;
    }

    qCDebug(lcQuick3DXr, "Available Layers: (%d)", layerCount);
    for (const XrApiLayerProperties& layer : layers) {
        qCDebug(lcQuick3DXr, "  Name=%s SpecVersion=%d.%d.%d LayerVersion=%d.%d.%d Description=%s",
               layer.layerName,
               XR_VERSION_MAJOR(layer.specVersion),
               XR_VERSION_MINOR(layer.specVersion),
               XR_VERSION_PATCH(layer.specVersion),
               XR_VERSION_MAJOR(layer.layerVersion),
               XR_VERSION_MINOR(layer.layerVersion),
               XR_VERSION_PATCH(layer.layerVersion),
               layer.description);
        checkXrExtensions(layer.layerName, 4);
    }
}

XrResult QQuick3DXrManagerPrivate::createXrInstance()
{
    // Setup Info
    XrApplicationInfo appInfo;
    strcpy(appInfo.applicationName, QCoreApplication::applicationName().toUtf8());
    appInfo.applicationVersion = 7;
    strcpy(appInfo.engineName, QStringLiteral("Qt").toUtf8());
    appInfo.engineVersion = 6;

    // apiVersion must not be XR_CURRENT_API_VERSION. Consider what happens when
    // building against 1.1 headers and running on an 1.0-only runtime. (it all
    // breaks down) For now, use a known, fixed version: the last 1.0 release.
    appInfo.apiVersion = XR_MAKE_VERSION(1, 0, 34);

    // Query available API layers
    uint32_t apiLayerCount = 0;
    xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr);
    QVector<XrApiLayerProperties> apiLayerProperties(apiLayerCount);
    for (uint32_t i = 0; i < apiLayerCount; i++) {
        apiLayerProperties[i].type = XR_TYPE_API_LAYER_PROPERTIES;
        apiLayerProperties[i].next = nullptr;
    }
    xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data());

    // Decide which API layers to enable
    QVector<const char*> enabledApiLayers;

    // Now it would be nice if we could use
    // QQuickGraphicsConfiguration::isDebugLayerEnabled() but the quickWindow is
    // nowhere yet, so just replicate the env.var. for now.
    const bool wantsValidationLayer = qEnvironmentVariableIntValue("QSG_RHI_DEBUG_LAYER");
    if (wantsValidationLayer) {
        if (isApiLayerSupported("XR_APILAYER_LUNARG_core_validation", apiLayerProperties))
            enabledApiLayers.append("XR_APILAYER_LUNARG_core_validation");
        else
            qCDebug(lcQuick3DXr, "OpenXR validation layer requested, but not available");
    }

    qCDebug(lcQuick3DXr) << "Requesting to enable XR API layers:" << enabledApiLayers;

    m_enabledApiLayers.clear();
    for (const char *layer : enabledApiLayers)
        m_enabledApiLayers.append(QString::fromLatin1(layer));

    // Load extensions
    uint32_t extensionCount = 0;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr);
    QVector<XrExtensionProperties> extensionProperties(extensionCount);
    for (uint32_t i = 0; i < extensionCount; i++) {
        // we usually have to fill in the type (for validation) and set
        // next to NULL (or a pointer to an extension specific struct)
        extensionProperties[i].type = XR_TYPE_EXTENSION_PROPERTIES;
        extensionProperties[i].next = nullptr;
    }
    xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data());

    QVector<const char*> enabledExtensions;
    if (!m_graphics->initialize(extensionProperties)) {
        qWarning("Failed to initialize OpenXR graphics module");
        return XR_ERROR_GRAPHICS_DEVICE_INVALID;
    }
    enabledExtensions.append(m_graphics->getRequiredExtensions());

    if (isExtensionSupported("XR_EXT_debug_utils", extensionProperties))
        enabledExtensions.append("XR_EXT_debug_utils");

    if (isExtensionSupported(XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME);

    m_handtrackingExtensionSupported = isExtensionSupported(XR_EXT_HAND_TRACKING_EXTENSION_NAME, extensionProperties);
    if (m_handtrackingExtensionSupported)
        enabledExtensions.append(XR_EXT_HAND_TRACKING_EXTENSION_NAME);

    m_compositionLayerDepthSupported = isExtensionSupported(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, extensionProperties);
    if (m_compositionLayerDepthSupported) {
        // The extension is enabled, whenever supported; however, if we actually
        // submit depth in xrEndFrame(), is a different question.
        enabledExtensions.append(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
        m_submitLayerDepth = qEnvironmentVariableIntValue("QT_QUICK3D_XR_SUBMIT_DEPTH");
        if (m_submitLayerDepth)
            qCDebug(lcQuick3DXr, "submitLayerDepth defaults to true due to env.var.");
    } else {
        m_submitLayerDepth = false;
    }

    // Oculus Quest Specific Extensions

    m_handtrackingAimExtensionSupported = isExtensionSupported(XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME, extensionProperties);
    if (m_handtrackingAimExtensionSupported)
        enabledExtensions.append(XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME);

    if (isExtensionSupported(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);

    if (isExtensionSupported(XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME);

    // Passthrough extensions (require manifest feature to work)
    // <uses-feature android:name="com.oculus.feature.PASSTHROUGH" android:required="true" />
    uint32_t passthroughSpecVersion = 0;
    if (isExtensionSupported(XR_FB_PASSTHROUGH_EXTENSION_NAME, extensionProperties, &passthroughSpecVersion)) {
        qCDebug(lcQuick3DXr, "Passthrough extension is supported, spec version %u", passthroughSpecVersion);
        enabledExtensions.append(XR_FB_PASSTHROUGH_EXTENSION_NAME);
    } else {
        qCDebug(lcQuick3DXr, "Passthrough extension is NOT supported");
    }

    if (isExtensionSupported(XR_FB_TRIANGLE_MESH_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_FB_TRIANGLE_MESH_EXTENSION_NAME);

    m_displayRefreshRateExtensionSupported = isExtensionSupported(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME, extensionProperties);
    if (m_displayRefreshRateExtensionSupported)
        enabledExtensions.append(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME);

    m_colorspaceExtensionSupported = isExtensionSupported(XR_FB_COLOR_SPACE_EXTENSION_NAME, extensionProperties);
    if (m_colorspaceExtensionSupported)
        enabledExtensions.append(XR_FB_COLOR_SPACE_EXTENSION_NAME);

    if (isExtensionSupported(XR_FB_SWAPCHAIN_UPDATE_STATE_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_FB_SWAPCHAIN_UPDATE_STATE_EXTENSION_NAME);

    m_foveationExtensionSupported = isExtensionSupported(XR_FB_FOVEATION_EXTENSION_NAME, extensionProperties);
    if (m_foveationExtensionSupported)
        enabledExtensions.append(XR_FB_FOVEATION_EXTENSION_NAME);

    if (isExtensionSupported(XR_FB_FOVEATION_CONFIGURATION_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_FB_FOVEATION_CONFIGURATION_EXTENSION_NAME);

    if (m_spaceExtension) {
        const auto requiredExtensions = m_spaceExtension->requiredExtensions();
        bool isSupported = true;
        for (const auto extension : requiredExtensions) {
            isSupported = isExtensionSupported(extension, extensionProperties) && isSupported;
            if (!isSupported)
                break;
        }
        m_spaceExtensionSupported = isSupported;
        if (isSupported)
            enabledExtensions.append(requiredExtensions);
    }

    if (isExtensionSupported(XR_EXTX_OVERLAY_EXTENSION_NAME, extensionProperties)) {
        m_overlayExtensionSupported = true;
        enabledExtensions.append(XR_EXTX_OVERLAY_EXTENSION_NAME);
    }

#ifdef Q_OS_ANDROID
    if (isExtensionSupported(XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME, extensionProperties))
        enabledExtensions.append(XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME);

    m_androidCreateInstanceExtensionSupported = isExtensionSupported(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME, extensionProperties);
    if (m_androidCreateInstanceExtensionSupported)
        enabledExtensions.append(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);

    auto graphicsAPI = QQuickWindow::graphicsApi();
    if (graphicsAPI == QSGRendererInterface::Vulkan) {
        if (isExtensionSupported(XR_FB_SWAPCHAIN_UPDATE_STATE_VULKAN_EXTENSION_NAME, extensionProperties))
            enabledExtensions.append(XR_FB_SWAPCHAIN_UPDATE_STATE_VULKAN_EXTENSION_NAME);
    } else if (graphicsAPI == QSGRendererInterface::OpenGL) {
        if (isExtensionSupported(XR_FB_SWAPCHAIN_UPDATE_STATE_OPENGL_ES_EXTENSION_NAME, extensionProperties))
            enabledExtensions.append(XR_FB_SWAPCHAIN_UPDATE_STATE_OPENGL_ES_EXTENSION_NAME);
    }
#endif

    qCDebug(lcQuick3DXr) << "Requesting to enable XR extensions:" << enabledExtensions;

    m_enabledExtensions.clear();
    for (const char *extension : enabledExtensions)
        m_enabledExtensions.append(QString::fromLatin1(extension));

    // Create Instance
    XrInstanceCreateInfo xrInstanceInfo{};
    xrInstanceInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;

#ifdef Q_OS_ANDROID
    XrInstanceCreateInfoAndroidKHR xrInstanceCreateInfoAndroid {};
    if (m_androidCreateInstanceExtensionSupported) {
        xrInstanceCreateInfoAndroid.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
        xrInstanceCreateInfoAndroid.applicationVM = m_javaVM;
        xrInstanceCreateInfoAndroid.applicationActivity = m_androidActivity.object();

        xrInstanceInfo.next = &xrInstanceCreateInfoAndroid;
    }
#endif


    xrInstanceInfo.createFlags = 0;
    xrInstanceInfo.applicationInfo = appInfo;
    xrInstanceInfo.enabledApiLayerCount = enabledApiLayers.count();
    xrInstanceInfo.enabledApiLayerNames = enabledApiLayers.constData();
    xrInstanceInfo.enabledExtensionCount = enabledExtensions.count();
    xrInstanceInfo.enabledExtensionNames = enabledExtensions.constData();

    return xrCreateInstance(&xrInstanceInfo, &m_instance);
}

void QQuick3DXrManagerPrivate::checkXrInstance()
{
    Q_ASSERT(m_instance != XR_NULL_HANDLE);
    XrInstanceProperties instanceProperties{};
    instanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
    if (!checkXrResult(xrGetInstanceProperties(m_instance, &instanceProperties))) {
        qWarning("Failed to get instance properties");
        return;
    }

    m_runtimeName = QString::fromUtf8(instanceProperties.runtimeName);
    m_runtimeVersion = QVersionNumber(XR_VERSION_MAJOR(instanceProperties.runtimeVersion),
                                      XR_VERSION_MINOR(instanceProperties.runtimeVersion),
                                      XR_VERSION_PATCH(instanceProperties.runtimeVersion));

    qCDebug(lcQuick3DXr, "Instance RuntimeName=%s RuntimeVersion=%d.%d.%d",
           qPrintable(m_runtimeName),
           m_runtimeVersion.majorVersion(),
           m_runtimeVersion.minorVersion(),
           m_runtimeVersion.microVersion());
}

void QQuick3DXrManagerPrivate::setupDebugMessenger()
{
    if (!m_enabledExtensions.contains(QString::fromUtf8("XR_EXT_debug_utils"))) {
        qCDebug(lcQuick3DXr, "No debug utils extension, message redirection not set up");
        return;
    }

#ifdef XR_EXT_debug_utils
    PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT = nullptr;
    OpenXRHelpers::resolveXrFunction(m_instance, "xrCreateDebugUtilsMessengerEXT", reinterpret_cast<PFN_xrVoidFunction *>(&xrCreateDebugUtilsMessengerEXT));
    if (!xrCreateDebugUtilsMessengerEXT)
        return;

    OpenXRHelpers::resolveXrFunction(m_instance, "xrDestroyDebugUtilsMessengerEXT", reinterpret_cast<PFN_xrVoidFunction *>(&m_xrDestroyDebugUtilsMessengerEXT));

    XrDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    messengerInfo.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
    messengerInfo.userCallback = defaultDebugCallbackFunc;
    messengerInfo.userData = this;

    XrResult err = xrCreateDebugUtilsMessengerEXT(m_instance, &messengerInfo, &m_debugMessenger);
    if (!checkXrResult(err))
        qWarning("Quick 3D XR: Failed to create debug report callback, OpenXR messages will not get redirected (%d)", err);
#endif // XR_EXT_debug_utils
}

XrResult QQuick3DXrManagerPrivate::initializeSystem()
{
    Q_ASSERT(m_instance != XR_NULL_HANDLE);
    Q_ASSERT(m_systemId == XR_NULL_SYSTEM_ID);

    XrSystemGetInfo hmdInfo{};
    hmdInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    hmdInfo.next = nullptr;
    hmdInfo.formFactor = m_formFactor;

    const XrResult result = xrGetSystem(m_instance, &hmdInfo, &m_systemId);
    const bool success = checkXrResult(result);

    if (!success)
        return result;

    // Check View Configuration
    checkViewConfiguration();

    return result;
}

void QQuick3DXrManagerPrivate::checkViewConfiguration()
{
    quint32 viewConfigTypeCount;
    if (!checkXrResult(xrEnumerateViewConfigurations(m_instance,
                                                     m_systemId,
                                                     0,
                                                     &viewConfigTypeCount,
                                                     nullptr)))
    {
        qWarning("Failed to enumerate view configurations");
        return;
    }
    QVector<XrViewConfigurationType> viewConfigTypes(viewConfigTypeCount);
    if (!checkXrResult(xrEnumerateViewConfigurations(m_instance,
                                                     m_systemId,
                                                     viewConfigTypeCount,
                                                     &viewConfigTypeCount,
                                                     viewConfigTypes.data())))
    {
        qWarning("Failed to enumerate view configurations");
        return;
    }

    qCDebug(lcQuick3DXr, "Available View Configuration Types: (%d)", viewConfigTypeCount);
    for (XrViewConfigurationType viewConfigType : viewConfigTypes) {
        qCDebug(lcQuick3DXr, "  View Configuration Type: %s %s", to_string(viewConfigType), viewConfigType == m_viewConfigType ? "(Selected)" : "");
        XrViewConfigurationProperties viewConfigProperties{};
        viewConfigProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
        if (!checkXrResult(xrGetViewConfigurationProperties(m_instance,
                                                            m_systemId,
                                                            viewConfigType,
                                                            &viewConfigProperties)))
        {
            qWarning("Failed to get view configuration properties");
            return;
        }

        qCDebug(lcQuick3DXr, "  View configuration FovMutable=%s", viewConfigProperties.fovMutable == XR_TRUE ? "True" : "False");

        uint32_t viewCount;
        if (!checkXrResult(xrEnumerateViewConfigurationViews(m_instance,
                                                             m_systemId,
                                                             viewConfigType,
                                                             0,
                                                             &viewCount,
                                                             nullptr)))
        {
            qWarning("Failed to enumerate configuration views");
            return;
        }

        if (viewCount > 0) {
            QVector<XrViewConfigurationView> views(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr, 0, 0, 0, 0, 0, 0});
            if (!checkXrResult(xrEnumerateViewConfigurationViews(m_instance,
                                                                 m_systemId,
                                                                 viewConfigType,
                                                                 viewCount,
                                                                 &viewCount,
                                                                 views.data())))
            {
                qWarning("Failed to enumerate configuration views");
                return;
            }

            for (int i = 0; i < views.size(); ++i) {
                const XrViewConfigurationView& view = views[i];
                qCDebug(lcQuick3DXr, "    View [%d]: Recommended Width=%d Height=%d SampleCount=%d",
                       i,
                       view.recommendedImageRectWidth,
                       view.recommendedImageRectHeight,
                       view.recommendedSwapchainSampleCount);
                qCDebug(lcQuick3DXr, "    View [%d]:     Maximum Width=%d Height=%d SampleCount=%d",
                       i,
                       view.maxImageRectWidth,
                       view.maxImageRectHeight,
                       view.maxSwapchainSampleCount);
            }
        } else {
            qCDebug(lcQuick3DXr, "Empty view configuration type");
        }
        checkEnvironmentBlendMode(viewConfigType);
    }
}

bool QQuick3DXrManagerPrivate::checkXrResult(const XrResult &result)
{
    return OpenXRHelpers::checkXrResult(result, m_instance);
}

void QQuick3DXrManagerPrivate::checkEnvironmentBlendMode(XrViewConfigurationType type)
{
    uint32_t count;
    if (!checkXrResult(xrEnumerateEnvironmentBlendModes(m_instance,
                                                        m_systemId,
                                                        type,
                                                        0,
                                                        &count,
                                                        nullptr)))
    {
        qWarning("Failed to enumerate blend modes");
        return;
    }

    qCDebug(lcQuick3DXr, "Available Environment Blend Mode count : (%d)", count);

    QVector<XrEnvironmentBlendMode> blendModes(count);
    if (!checkXrResult(xrEnumerateEnvironmentBlendModes(m_instance,
                                                        m_systemId,
                                                        type,
                                                        count,
                                                        &count,
                                                        blendModes.data())))
    {
        qWarning("Failed to enumerate blend modes");
        return;
    }

    m_blendModes = blendModes;

    // Set initial blend mode, prefer opaque (passthrough disabled)
    if (!setTransparentBlendMode(false))
        setTransparentBlendMode(true);
}

QT_END_NAMESPACE

#include "qquick3dxrmanager_openxr.moc"
