// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qopenxrgraphics_opengles_p.h"
#include "qopenxrhelpers_p.h"

#include <QtGui/QOpenGLContext>
#include <QtQuick/private/qquickrendertarget_p.h>
#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

#ifndef GL_DEPTH_COMPONENT32F
#define GL_DEPTH_COMPONENT32F             0x8CAC
#endif

QOpenXRGraphicsOpenGLES::QOpenXRGraphicsOpenGLES()
{
#ifdef XR_USE_PLATFORM_ANDROID
    m_graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR;
#endif
    m_graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR;
}

bool QOpenXRGraphicsOpenGLES::initialize(const QVector<XrExtensionProperties> &extensions)
{
    return hasExtension(extensions, XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME);
}

QVector<const char *> QOpenXRGraphicsOpenGLES::getRequiredExtensions() const
{
    return { XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME };
}


const XrBaseInStructure *QOpenXRGraphicsOpenGLES::handle() const
{
    return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
}


bool QOpenXRGraphicsOpenGLES::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &)
{
    // Extension function must be loaded by name
    PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnGetOpenGLESGraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetOpenGLESGraphicsRequirementsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLESGraphicsRequirementsKHR)),
                                 instance);

    if (!pfnGetOpenGLESGraphicsRequirementsKHR) {
        qWarning("Could not resolve xrGetOpenGLESGraphicsRequirementsKHR; perhaps the OpenXR implementation does not support OpenGL ES?");
        return false;
    }
    OpenXRHelpers::checkXrResult(pfnGetOpenGLESGraphicsRequirementsKHR(instance, systemId, &m_graphicsRequirements),
                                 instance);
    return true;
}

bool QOpenXRGraphicsOpenGLES::finializeGraphics(QRhi *rhi)
{
    const QRhiGles2NativeHandles *openglRhi = static_cast<const QRhiGles2NativeHandles *>(rhi->nativeHandles());

    auto context = openglRhi->context;
    const XrVersion desiredApiVersion = XR_MAKE_VERSION(context->format().majorVersion(), context->format().minorVersion(), 0);
    if (m_graphicsRequirements.minApiVersionSupported > desiredApiVersion) {
        qWarning("Qt Quick 3D XR (Open GL ES): Runtime does not support desired graphics API and/or version");
        return false;
    }

#ifdef XR_USE_PLATFORM_ANDROID
    auto nativeContext = context->nativeInterface<QNativeInterface::QEGLContext>();
    if (nativeContext) {
        m_graphicsBinding.display = nativeContext->display();
        m_graphicsBinding.config = nativeContext->config();
        m_graphicsBinding.context = nativeContext->nativeContext();
    }
#endif

    m_rhi = rhi;

    return true;
}


int64_t QOpenXRGraphicsOpenGLES::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // List of supported color swapchain formats.
    constexpr int64_t supportedColorSwapchainFormats[] = {
        GL_SRGB8_ALPHA8_EXT,
        GL_RGBA8_SNORM
    };

    auto swapchainFormatIt = std::find_first_of(std::begin(supportedColorSwapchainFormats),
                                                std::end(supportedColorSwapchainFormats),
                                                swapchainFormats.begin(),
                                                swapchainFormats.end());
    return *swapchainFormatIt;
}

int64_t QOpenXRGraphicsOpenGLES::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // in order of preference
    constexpr int64_t supportedDepthSwapchainFormats[] = {
        GL_DEPTH24_STENCIL8_OES,
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH_COMPONENT24_OES,
        GL_DEPTH_COMPONENT16_OES
    };

    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}


QVector<XrSwapchainImageBaseHeader*> QOpenXRGraphicsOpenGLES::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageOpenGLESKHR> swapchainImageBuffer(count);
    for (XrSwapchainImageOpenGLESKHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
        swapchainImages.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}


QQuickRenderTarget QOpenXRGraphicsOpenGLES::renderTarget(const XrSwapchainSubImage &subImage,
                                                         const XrSwapchainImageBaseHeader *swapchainImage,
                                                         quint64 swapchainFormat,
                                                         int samples,
                                                         int arraySize,
                                                         const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                         quint64 depthSwapchainFormat) const
{
    const uint32_t colorTexture = reinterpret_cast<const XrSwapchainImageOpenGLESKHR*>(swapchainImage)->image;

    switch (swapchainFormat) {
    case GL_SRGB8_ALPHA8_EXT:
        swapchainFormat = GL_RGBA8_OES;
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
        case GL_DEPTH_COMPONENT24_OES:
            format = QRhiTexture::D24;
            break;
        case GL_DEPTH_COMPONENT16_OES:
            format = QRhiTexture::D16;
            break;
        }
        GLuint depthImage = reinterpret_cast<const XrSwapchainImageOpenGLESKHR*>(depthSwapchainImage)->image;
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

void QOpenXRGraphicsOpenGLES::releaseResources()
{
    delete m_depthTexture;
    m_depthTexture = nullptr;
}

QT_END_NAMESPACE
