// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRMANAGER_OPENXR_P_H
#define QQUICK3DXRMANAGER_OPENXR_P_H

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

#include <QtQuick3DXr/private/qabstractopenxrgraphics_p.h>
#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qpointer.h>

#include <QtGui/private/qtgui-config_p.h>
#if QT_CONFIG(graphicsframecapture)
#include <QtGui/private/qgraphicsframecapture_p.h>
#endif

#ifdef XR_USE_PLATFORM_ANDROID
# include <QtCore/QJniObject>
#endif

QT_BEGIN_NAMESPACE

class QQuick3DXrEyeCamera;
class QQuick3DXrOrigin;
class QQuick3DViewport;
class QQuick3DXrAnchorManager;
class QQuick3DXrManager;
class QQuick3DXrInputManager;
class QQuick3DOpenXRThreadWorker;
class QThread;

class QQuick3DXrManagerPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DXrManager)
public:
    explicit QQuick3DXrManagerPrivate(QQuick3DXrManager &manager);
    ~QQuick3DXrManagerPrivate();

    static QQuick3DXrManagerPrivate *get(QQuick3DXrManager *manager);

    [[nodiscard]] bool supportsPassthrough() const;
    bool supportsTransparentBlendMode() const;
    bool supportsFBPassthrough() const;

    bool isValid() const { return m_graphics != nullptr; }

    bool isReady() const { return true; }

    void setupWindow(QQuickWindow *window);

    bool isGraphicsInitialized() const;
    bool setupGraphics(QQuickWindow *window);

    void update();

    void processXrEvents();

    void doRenderFrame();

    bool finalizeGraphics(QRhi *rhi);

    void setSamples(int samples);

    QStringList enabledExtensions() const;
    QString runtimeName() const;
    QVersionNumber runtimeVersion() const;

    void setMultiViewRenderingEnabled(bool enable);
    bool isMultiViewRenderingEnabled() const { return m_multiviewRendering; }

    [[nodiscard]] bool setPassthroughEnabled(bool enable);
    bool isPassthroughEnabled() const;

    void setFBPassthroughEnabled(bool enable);
    bool setTransparentBlendMode(bool enable);

    void setDepthSubmissionEnabled(bool enable);
    bool isDepthSubmissionEnabled() const { return m_compositionLayerDepthSupported && m_submitLayerDepth; }

    void setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace);
    QtQuick3DXr::ReferenceSpace getReferenceSpace() const;

    void getDefaultClipDistances(float &nearClip, float &farClip) const;

    QString errorString() const { return m_errorString; }

private:
    friend class QQuick3DXrRuntimeInfo;

    bool initialize();
    void teardown();

    void initWorkerThread();
    void destroyWorkerThread();
    void onFrameWaitCompleted(XrResult result, const XrFrameState &frameState);
    void doRenderFrameAferWait(const XrFrameState &frameState);

    void destroySwapchain();
    void setErrorString(XrResult result, const char *callName);
    void checkXrExtensions(const char* layerName, int indent = 0);
    void checkXrLayers();

    XrResult createXrInstance();
    void checkXrInstance();

    void setupDebugMessenger();

    XrResult initializeSystem();

    void checkViewConfiguration();
    [[nodiscard]] bool checkXrResult(const XrResult &result);
    void checkEnvironmentBlendMode(XrViewConfigurationType type);

    void pollEvents(bool *exitRenderLoop, bool *requestRestart);
    void pollActions();

    void checkReferenceSpaces();
    bool isReferenceSpaceAvailable(XrReferenceSpaceType type);

    bool setupAppSpace();
    void updateAppSpace(XrTime predictedDisplayTime);
    bool setupViewSpace();
    bool resetEmulatedFloorHeight(XrTime predictedDisplayTime);

    bool createSwapchains();

    bool renderLayer(XrTime predictedDisplayTime,
                     XrDuration predictedDisplayPeriod,
                     XrCompositionLayerProjection &layer);
    void doRender(const XrSwapchainSubImage &subImage,
                  const XrSwapchainImageBaseHeader *swapchainImage,
                  const XrSwapchainImageBaseHeader *depthSwapchainImage = nullptr);

    void updateCameraHelper(QQuick3DXrEyeCamera *camera, const XrCompositionLayerProjectionView &layerView);
    void updateCameraNonMultiview(int eye, const XrCompositionLayerProjectionView &layerView);
    void updateCameraMultiview(int projectionLayerViewStartIndex, int count);

    void setupMetaQuestColorSpaces();
    void setupMetaQuestRefreshRates();
    void setupMetaQuestFoveation();

    // Passthrough
    XrPassthroughFB m_passthroughFeature{XR_NULL_HANDLE};
    void createMetaQuestPassthrough();
    void destroyMetaQuestPassthrough();
    void startMetaQuestPassthrough();
    void pauseMetaQuestPassthrough();

    XrPassthroughLayerFB m_passthroughLayer{XR_NULL_HANDLE};
    void createMetaQuestPassthroughLayer();
    void destroyMetaQuestPassthroughLayer();
    void pauseMetaQuestPassthroughLayer();
    void resumeMetaQuestPassthroughLayer();

    XrTime m_previousTime = 0;

    struct Swapchain {
        XrSwapchain handle;
        quint32 width;
        quint32 height;
        quint32 arraySize;
    };

    QVector<XrViewConfigurationView> m_configViews;
    QVector<XrCompositionLayerProjectionView> m_projectionLayerViews;
    QVector<XrCompositionLayerDepthInfoKHR> m_layerDepthInfos;
    QVector<Swapchain> m_swapchains;
    QVector<Swapchain> m_depthSwapchains;
    QMap<XrSwapchain, QVector<XrSwapchainImageBaseHeader*>> m_swapchainImages;
    QMap<XrSwapchain, QVector<XrSwapchainImageBaseHeader*>> m_depthSwapchainImages;
    QVector<XrView> m_views;

    QString m_errorString;
    QString m_runtimeName;
    QVersionNumber m_runtimeVersion;
    QStringList m_enabledApiLayers;
    QStringList m_enabledExtensions;

    XrEventDataBuffer m_eventDataBuffer;

    XrInstance m_instance{XR_NULL_HANDLE};
    XrSession m_session{XR_NULL_HANDLE};
    XrSpace m_appSpace{XR_NULL_HANDLE};
    XrReferenceSpaceType m_requestedReferenceSpace = XR_REFERENCE_SPACE_TYPE_STAGE;
    XrReferenceSpaceType m_referenceSpace = XR_REFERENCE_SPACE_TYPE_LOCAL;
    bool m_isEmulatingLocalFloor = false;
    bool m_isFloorResetPending = false;
    XrSpace m_viewSpace{XR_NULL_HANDLE};
    XrFormFactor m_formFactor{XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY};
    XrViewConfigurationType m_viewConfigType{XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO};
    XrEnvironmentBlendMode m_environmentBlendMode{XR_ENVIRONMENT_BLEND_MODE_OPAQUE};
    XrSystemId m_systemId{XR_NULL_SYSTEM_ID};

    // Application's current lifecycle state according to the runtime
    XrSessionState m_sessionState{XR_SESSION_STATE_UNKNOWN};

    QVector<XrReferenceSpaceType> m_availableReferenceSpace;

    QPointer<QQuick3DXrInputManager> m_inputManager;

    int64_t m_colorSwapchainFormat = -1;
    int64_t m_depthSwapchainFormat = -1;
    int m_samples = 1;

    bool m_fbPassthroughEnabled = false;
    bool m_multiviewRendering = true;
    bool m_spaceExtensionSupported = false;
    QQuick3DXrAnchorManager *m_spaceExtension = nullptr;
    bool m_colorspaceExtensionSupported = false;
    bool m_displayRefreshRateExtensionSupported = false;
    bool m_foveationExtensionSupported = false;
#ifdef Q_OS_ANDROID
    bool m_androidCreateInstanceExtensionSupported = false;
#endif
    XrFoveationLevelFB m_foveationLevel = XR_FOVEATION_LEVEL_HIGH_FB;
    bool m_compositionLayerDepthSupported = false;
    bool m_submitLayerDepth = false;
    bool m_handtrackingExtensionSupported = false;
    bool m_handtrackingAimExtensionSupported = false;
    bool m_overlayExtensionSupported = false;
    bool m_isGraphicsInitialized = false;

    bool m_sessionRunning{false};

    QQuick3DXrManager *q_ptr = nullptr;
#ifdef XR_USE_PLATFORM_ANDROID
    QJniObject m_androidActivity;
    JavaVM* m_javaVM = nullptr;
#endif // XR_USE_PLATFORM_ANDROID

#ifdef XR_EXT_debug_utils
    XrDebugUtilsMessengerEXT m_debugMessenger = XR_NULL_HANDLE;
    PFN_xrDestroyDebugUtilsMessengerEXT m_xrDestroyDebugUtilsMessengerEXT = nullptr;
#endif

#if QT_CONFIG(graphicsframecapture)
    std::unique_ptr<QGraphicsFrameCapture> m_frameCapture;
#endif

    QAbstractOpenXRGraphics *m_graphics = nullptr;

    bool m_waitingForFrame = false;
    bool m_wantUpdate = false;
    QQuick3DOpenXRThreadWorker *m_worker = nullptr;
    QThread *m_workerThread = nullptr;
    QVector<XrEnvironmentBlendMode> m_blendModes;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRMANAGER_OPENXR_P_H
