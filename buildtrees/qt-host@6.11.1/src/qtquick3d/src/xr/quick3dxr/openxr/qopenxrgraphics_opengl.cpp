// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qopenxrgraphics_opengl_p.h"
#include "qopenxrhelpers_p.h"

#include <QtGui/QOpenGLContext>
#include <QtQuick/QQuickWindow>
#include <QtQuick/private/qquickrendertarget_p.h>

#include <rhi/qrhi.h>

#if defined(XR_USE_PLATFORM_XLIB) || defined(XR_USE_PLATFORM_XCB)
#include <GL/glx.h>
#endif

#include <qpa/qplatformnativeinterface.h>

QT_BEGIN_NAMESPACE

#ifndef GL_RGBA8
#define GL_RGBA8                          0x8058
#endif

#ifndef GL_SRGB8_ALPHA8_EXT
#define GL_SRGB8_ALPHA8_EXT               0x8C43
#endif

#ifndef GL_DEPTH_COMPONENT16
#define GL_DEPTH_COMPONENT16              0x81A5
#endif

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24              0x81A6
#endif

#ifndef GL_DEPTH_COMPONENT32F
#define GL_DEPTH_COMPONENT32F             0x8CAC
#endif

#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8               0x88F0
#endif

template<typename T>
T *typed_calloc()
{
    return static_cast<T *>(calloc(1, sizeof(T)));
}

QOpenXRGraphicsOpenGL::QOpenXRGraphicsOpenGL()
{
    m_graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
}

bool QOpenXRGraphicsOpenGL::initialize(const QVector<XrExtensionProperties> &extensions)
{

#ifdef XR_USE_PLATFORM_EGL
    if (hasExtension(extensions, XR_MNDX_EGL_ENABLE_EXTENSION_NAME)) {

        // To choose XR_MNDX_egl_enable we need to be absolutely sure that
        // we will get access to EGLContext later and at this point we have no window/rhi yet.
        QOpenGLContext ctx;
        if (ctx.create() && ctx.nativeInterface<QNativeInterface::QEGLContext>()) {
            m_selectedPlatform = PLATFORM_EGL;
            m_requiredExtensions = { XR_MNDX_EGL_ENABLE_EXTENSION_NAME };

            // XR_MNDX_egl_enable does not require XR_KHR_opengl_enable
            // However, some OpenXR runtimes still want it (old Monado/SteamVR)
            if (hasExtension(extensions, XR_KHR_OPENGL_ENABLE_EXTENSION_NAME)) {
                m_requiredExtensions.append(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
            }
            return true;
        }
    }
#endif

    // Standard OpenGL platforms
    if (hasExtension(extensions, XR_KHR_OPENGL_ENABLE_EXTENSION_NAME)) {

#ifdef XR_USE_PLATFORM_WAYLAND
        auto wlApp = qApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (wlApp && wlApp->display()) {
            m_selectedPlatform = PLATFORM_WAYLAND;
            m_requiredExtensions = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
            return true;
        }
#endif

#ifdef XR_USE_PLATFORM_XLIB
        auto x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
        if (x11App && x11App->display()) {
            m_selectedPlatform = PLATFORM_XLIB;
            m_requiredExtensions = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
            return true;
        }
#endif
#ifdef XR_USE_PLATFORM_WIN32
        m_selectedPlatform = PLATFORM_WIN32;
        m_requiredExtensions = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
        return true;
#endif
    }

    qWarning("Qt Quick 3D XR (OpenGL): no supported platforms");
    return false;
}

QVector<const char *> QOpenXRGraphicsOpenGL::getRequiredExtensions() const
{
    return m_requiredExtensions;
}


const XrBaseInStructure *QOpenXRGraphicsOpenGL::handle() const
{
    return m_graphicsBinding.get();
}


bool QOpenXRGraphicsOpenGL::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &)
{
    // Extension function must be loaded by name
    PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLGraphicsRequirementsKHR)),
                                 instance);
    if (!pfnGetOpenGLGraphicsRequirementsKHR) {
        qWarning("Could not resolve xrGetOpenGLGraphicsRequirementsKHR; perhaps the OpenXR implementation does not support OpenGL?");
        return false;
    }
    OpenXRHelpers::checkXrResult(pfnGetOpenGLGraphicsRequirementsKHR(instance, systemId, &m_graphicsRequirements),
                                 instance);
    return true;
}

bool QOpenXRGraphicsOpenGL::finializeGraphics(QRhi *rhi)
{
    const QRhiGles2NativeHandles *openglRhi = static_cast<const QRhiGles2NativeHandles *>(rhi->nativeHandles());

    QOpenGLContext *openGLContext = openglRhi->context;

    const XrVersion desiredApiVersion = XR_MAKE_VERSION(openGLContext->format().majorVersion(),
                                                        openGLContext->format().minorVersion(),
                                                        0);
    if (m_graphicsRequirements.minApiVersionSupported > desiredApiVersion) {
        qWarning("Qt Quick 3D XR (OpenGL): Runtime does not support desired graphics API and/or version");
        return false;
    }

    switch (m_selectedPlatform) {

#ifdef XR_USE_PLATFORM_EGL
    case PLATFORM_EGL: {
        auto eglContext = openGLContext->nativeInterface<QNativeInterface::QEGLContext>();
        if (!eglContext) {
            qWarning("Qt Quick 3D XR (OpenGL): Failed to get QNativeInterface::QEGLContext");
            return false;
        }
        auto binding = typed_calloc<XrGraphicsBindingEGLMNDX>();
        binding->type = XR_TYPE_GRAPHICS_BINDING_EGL_MNDX;
        binding->getProcAddress = &eglGetProcAddress;
        binding->display = eglContext->display();
        binding->config = eglContext->config();
        binding->context = eglContext->nativeContext();
        m_graphicsBinding.reset(reinterpret_cast<XrBaseInStructure *>(binding));
        break;
    }
#endif

#ifdef XR_USE_PLATFORM_WAYLAND
    case PLATFORM_WAYLAND: {
        auto wlApp = qApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        auto wlDisplay = wlApp->display();
        auto binding = typed_calloc<XrGraphicsBindingOpenGLWaylandKHR>();
        binding->type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR;
        binding->display = wlDisplay;
        m_graphicsBinding.reset(reinterpret_cast<XrBaseInStructure *>(binding));
        break;
    }
#endif

#ifdef XR_USE_PLATFORM_XLIB
    case PLATFORM_XLIB: {
        auto glxContext = openGLContext->nativeInterface<QNativeInterface::QGLXContext>();
        if (!glxContext) {
            qWarning("Qt Quick 3D XR (OpenGL): Failed to get QNativeInterface::QGLXContext");
            return false;
        }

        auto x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
        auto xDisplay = x11App->display();
        auto glxDrawable = m_window->winId();

        QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
        auto glxFBConfig = static_cast<GLXFBConfig>(native->nativeResourceForContext("glxconfig", openGLContext));
        if (!glxFBConfig) {
            qWarning("Qt Quick 3D XR (OpenGL): Failed to get GLXFBConfig");
            return false;
        }

        int visualId = 0;
        if (glXGetFBConfigAttrib(xDisplay, glxFBConfig, GLX_VISUAL_ID, &visualId) != Success) {
            qWarning("Qt Quick 3D XR (OpenGL): Failed to get visualId");
            return false;
        }

        auto binding = typed_calloc<XrGraphicsBindingOpenGLXlibKHR>();
        binding->type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR;
        binding->xDisplay = xDisplay;
        binding->visualid = visualId;
        binding->glxFBConfig = glxFBConfig;
        binding->glxDrawable = glxDrawable;
        binding->glxContext = glxContext->nativeContext();
        m_graphicsBinding.reset(reinterpret_cast<XrBaseInStructure *>(binding));
        break;
    }
#endif

#ifdef XR_USE_PLATFORM_WIN32
    case PLATFORM_WIN32: {
        auto wglContext = openGLContext->nativeInterface<QNativeInterface::QWGLContext>();
        if (!wglContext) {
            qWarning("Qt Quick 3D XR (OpenGL): Failed to get QNativeInterface::QWGLContext");
            return false;
        }
        auto binding = typed_calloc<XrGraphicsBindingOpenGLWin32KHR>();
        binding->type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
        binding->hDC = GetDC(reinterpret_cast<HWND>(m_window->winId()));
        binding->hGLRC = wglContext->nativeContext();
        m_graphicsBinding.reset(reinterpret_cast<XrBaseInStructure *>(binding));
        break;
    }
#endif

    default:
        return false;
        break;
    }

    m_rhi = rhi;

    return true;
}


int64_t QOpenXRGraphicsOpenGL::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // List of supported color swapchain formats.
    constexpr int64_t supportedColorSwapchainFormats[] = { GL_SRGB8_ALPHA8_EXT, GL_RGBA8, GL_RGBA8_SNORM };

    auto swapchainFormatIt = std::find_first_of(std::begin(supportedColorSwapchainFormats),
                                                std::end(supportedColorSwapchainFormats),
                                                swapchainFormats.begin(),
                                                swapchainFormats.end());
    return *swapchainFormatIt;
}

int64_t QOpenXRGraphicsOpenGL::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // in order of preference
    constexpr int64_t supportedDepthSwapchainFormats[] = {
        GL_DEPTH24_STENCIL8,
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT16
    };

    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}

QVector<XrSwapchainImageBaseHeader*> QOpenXRGraphicsOpenGL::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageOpenGLKHR> swapchainImageBuffer(count);
    for (XrSwapchainImageOpenGLKHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
        swapchainImages.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}


QQuickRenderTarget QOpenXRGraphicsOpenGL::renderTarget(const XrSwapchainSubImage &subImage,
                                                       const XrSwapchainImageBaseHeader *swapchainImage,
                                                       quint64 swapchainFormat,
                                                       int samples,
                                                       int arraySize,
                                                       const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                       quint64 depthSwapchainFormat) const
{
    const uint32_t colorTexture = reinterpret_cast<const XrSwapchainImageOpenGLKHR*>(swapchainImage)->image;

    switch (swapchainFormat) {
    case GL_SRGB8_ALPHA8_EXT:
        swapchainFormat = GL_RGBA8;
        break;
    default:
        break;
    }

    QQuickRenderTarget::Flags flags;
    if (samples > 1)
        flags |= QQuickRenderTarget::Flag::MultisampleResolve;

    const QSize pixelSize(subImage.imageRect.extent.width, subImage.imageRect.extent.height);
    QQuickRenderTarget rt = QQuickRenderTarget::fromOpenGLTexture(colorTexture,
                                                                  swapchainFormat,
                                                                  pixelSize,
                                                                  samples,
                                                                  arraySize,
                                                                  flags);
    if (depthSwapchainImage) {
        QRhiTexture::Format format = QRhiTexture::D24S8;
        switch (depthSwapchainFormat) {
        case GL_DEPTH_COMPONENT32F:
            format = QRhiTexture::D32F;
            break;
        case GL_DEPTH_COMPONENT24:
            format = QRhiTexture::D24;
            break;
        case GL_DEPTH_COMPONENT16:
            format = QRhiTexture::D16;
            break;
        }
        GLuint depthImage = reinterpret_cast<const XrSwapchainImageOpenGLKHR*>(depthSwapchainImage)->image;
        if (m_depthTexture && (m_depthTexture->format() != format || m_depthTexture->pixelSize() != pixelSize || m_depthTexture->arraySize() != arraySize)) {
            delete m_depthTexture;
            m_depthTexture = nullptr;
        }
        if (!m_depthTexture) {
            // this is never multisample, QQuickRt takes care of resolving depth-stencil
            if (arraySize > 1)
                m_depthTexture = m_rhi->newTextureArray(format, arraySize, pixelSize, 1, QRhiTexture::RenderTarget);
            else
                m_depthTexture = m_rhi->newTexture(format, pixelSize, 1, QRhiTexture::RenderTarget);
        }
        m_depthTexture->createFrom({ depthImage, 0 });
        rt.setDepthTexture(m_depthTexture);
    }
    return rt;
}

void QOpenXRGraphicsOpenGL::setupWindow(QQuickWindow *window)
{
    m_window = window;
}

void QOpenXRGraphicsOpenGL::releaseResources()
{
    delete m_depthTexture;
    m_depthTexture = nullptr;
}

QT_END_NAMESPACE
