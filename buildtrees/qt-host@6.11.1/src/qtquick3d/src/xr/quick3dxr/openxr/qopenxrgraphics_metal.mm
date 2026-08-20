// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qopenxrgraphics_metal_p.h"

#include "qopenxrhelpers_p.h"

#include <rhi/qrhi.h>
#include <QQuickGraphicsDevice>
#include <QQuickWindow>

#include <Metal/Metal.h>

QT_BEGIN_NAMESPACE

QOpenXRGraphicsMetal::QOpenXRGraphicsMetal()
{

}

bool QOpenXRGraphicsMetal::initialize(const QVector<XrExtensionProperties> &extensions)
{
    return hasExtension(extensions, XR_KHR_METAL_ENABLE_EXTENSION_NAME);
}

QVector<const char *> QOpenXRGraphicsMetal::getRequiredExtensions() const
{
    return { XR_KHR_METAL_ENABLE_EXTENSION_NAME };
}

const XrBaseInStructure *QOpenXRGraphicsMetal::handle() const
{
    return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
}

bool QOpenXRGraphicsMetal::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &quickConfig)
{
    Q_UNUSED(quickConfig)
    PFN_xrGetMetalGraphicsRequirementsKHR pfnGetMetalGraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetMetalGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetMetalGraphicsRequirementsKHR)), instance);

    if (!pfnGetMetalGraphicsRequirementsKHR) {
        qWarning("Could not resolve pfnGetMetalGraphicsRequirementsKHR; perhaps the OpenXR implementation does not support Metal?");
        return false;
    }

    // Create the Metal device for the adapter associated with the system.
    if (!OpenXRHelpers::checkXrResult(pfnGetMetalGraphicsRequirementsKHR(instance, systemId, &m_graphicsRequirements), instance)) {
        qWarning("Failed to get Metal graphics requirements.");
        return false;
    }

    // From hello_xr/graphicsplugin_metal.cpp:
    m_device = static_cast<MTLDevice*>(m_graphicsRequirements.metalDevice);
    auto commandQueue = [m_device newCommandQueue];
    m_commandQueue = static_cast<MTLCommandQueue *>(commandQueue);
    m_graphicsBinding.commandQueue = m_commandQueue;

    return true;
}

bool QOpenXRGraphicsMetal::finializeGraphics(QRhi *rhi)
{
    // STUB
    Q_UNUSED(rhi)
    return true;
}

int64_t QOpenXRGraphicsMetal::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    constexpr MTLPixelFormat supportedDepthSwapchainFormats[] = {MTLPixelFormatRGBA8Unorm_sRGB, MTLPixelFormatRGBA8Unorm};
    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}

int64_t QOpenXRGraphicsMetal::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // in order of preference
    constexpr int64_t supportedDepthSwapchainFormats[] = {
        MTLPixelFormatDepth24Unorm_Stencil8,
        MTLPixelFormatDepth32Float,
        MTLPixelFormatDepth16Unorm
    };
    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}

QVector<XrSwapchainImageBaseHeader *> QOpenXRGraphicsMetal::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageMetalKHR> swapchainImageBuffer(count);
    for (XrSwapchainImageMetalKHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_METAL_KHR;
        swapchainImages.append(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}

QQuickRenderTarget QOpenXRGraphicsMetal::renderTarget(const XrSwapchainSubImage &subImage,
                                                      const XrSwapchainImageBaseHeader *swapchainImage,
                                                      quint64 swapchainFormat,
                                                      int samples,
                                                      int arraySize,
                                                      const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                      quint64 depthSwapchainFormat) const
{
    MTLTexture *const colorTexture = static_cast<MTLTexture * const>(reinterpret_cast<const XrSwapchainImageMetalKHR*>(swapchainImage)->texture);
    MTLPixelFormat colorFormat = static_cast<MTLPixelFormat>(swapchainFormat);

    QQuickRenderTarget::Flags flags;
    if (samples > 1)
        flags |= QQuickRenderTarget::Flag::MultisampleResolve;

    QQuickRenderTarget renderTarget = QQuickRenderTarget::fromMetalTexture(colorTexture, colorFormat, colorFormat, QSize(subImage.imageRect.extent.width, subImage.imageRect.extent.height), samples, arraySize, flags);

    Q_UNUSED(depthSwapchainImage)
    Q_UNUSED(depthSwapchainFormat)
    return renderTarget;
}

QRhi *QOpenXRGraphicsMetal::rhi() const
{
    return m_rhi;
}


void QOpenXRGraphicsMetal::setupWindow(QQuickWindow *window)
{
    auto qqGraphicsDevice = QQuickGraphicsDevice::fromDeviceAndCommandQueue(static_cast<MTLDevice*>(m_device), static_cast<MTLCommandQueue *>(m_commandQueue));
    window->setGraphicsDevice(qqGraphicsDevice);
}

QT_END_NAMESPACE
