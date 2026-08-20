// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qopenxrgraphics_d3d12_p.h"

#include "qopenxrhelpers_p.h"

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickGraphicsDevice>

#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

QOpenXRGraphicsD3D12::QOpenXRGraphicsD3D12()
{
    m_graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_D3D12_KHR;
    m_graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D12_KHR;
}

bool QOpenXRGraphicsD3D12::initialize(const QVector<XrExtensionProperties> &extensions)
{
    return hasExtension(extensions, XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
}

QVector<const char *> QOpenXRGraphicsD3D12::getRequiredExtensions() const
{
    return { XR_KHR_D3D12_ENABLE_EXTENSION_NAME };
}


const XrBaseInStructure *QOpenXRGraphicsD3D12::handle() const
{
    return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
}


bool QOpenXRGraphicsD3D12::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &)
{
    PFN_xrGetD3D12GraphicsRequirementsKHR pfnGetD3D12GraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetD3D12GraphicsRequirementsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetD3D12GraphicsRequirementsKHR)),
                                 instance);

    if (!pfnGetD3D12GraphicsRequirementsKHR) {
        qWarning("Could not resolve xrGetD3D12GraphicsRequirementsKHR; perhaps the OpenXR implementation does not support D3D12?");
        return false;
    }

    OpenXRHelpers::checkXrResult(pfnGetD3D12GraphicsRequirementsKHR(instance, systemId, &m_graphicsRequirements),
                                 instance);
    return true;
}

bool QOpenXRGraphicsD3D12::finializeGraphics(QRhi *rhi)
{
    const QRhiD3D12NativeHandles *d3d12Rhi = static_cast<const QRhiD3D12NativeHandles *>(rhi->nativeHandles());
    m_graphicsBinding.device = reinterpret_cast<ID3D12Device*>(d3d12Rhi->dev);
    m_graphicsBinding.queue = reinterpret_cast<ID3D12CommandQueue*>(d3d12Rhi->commandQueue);
    m_rhi = rhi;

    return true;
}


int64_t QOpenXRGraphicsD3D12::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // List of supported color swapchain formats.
    constexpr DXGI_FORMAT supportedColorSwapchainFormats[] = {
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM
    };

    auto swapchainFormatIt = std::find_first_of(std::begin(supportedColorSwapchainFormats),
                                                std::end(supportedColorSwapchainFormats),
                                                swapchainFormats.begin(),
                                                swapchainFormats.end());

    return *swapchainFormatIt;
}

int64_t QOpenXRGraphicsD3D12::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // in order of preference
    constexpr int64_t supportedDepthSwapchainFormats[] = {
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_D16_UNORM
    };

    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}

QVector<XrSwapchainImageBaseHeader*> QOpenXRGraphicsD3D12::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageD3D12KHR> swapchainImageBuffer(count);
    for (XrSwapchainImageD3D12KHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR;
        swapchainImages.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}


QQuickRenderTarget QOpenXRGraphicsD3D12::renderTarget(const XrSwapchainSubImage &subImage,
                                                      const XrSwapchainImageBaseHeader *swapchainImage,
                                                      quint64 swapchainFormat,
                                                      int samples,
                                                      int arraySize,
                                                      const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                      quint64 depthSwapchainFormat) const
{
    ID3D12Resource* const colorTexture = reinterpret_cast<const XrSwapchainImageD3D12KHR*>(swapchainImage)->texture;

    DXGI_FORMAT viewFormat = DXGI_FORMAT(swapchainFormat);
    switch (swapchainFormat) {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        viewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        viewFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        break;
    }

    QQuickRenderTarget::Flags flags;
    if (samples > 1)
        flags |= QQuickRenderTarget::Flag::MultisampleResolve;

    return QQuickRenderTarget::fromD3D12Texture(colorTexture,
                                                0,
                                                swapchainFormat,
                                                viewFormat,
                                                QSize(subImage.imageRect.extent.width, subImage.imageRect.extent.height),
                                                samples,
                                                arraySize,
                                                flags);

    // No depthSwapchainImage support because ResolveDepthStencil will be
    // unsupported with D3D11/12 no matter what.
    Q_UNUSED(depthSwapchainImage);
    Q_UNUSED(depthSwapchainFormat);
}


void QOpenXRGraphicsD3D12::setupWindow(QQuickWindow *quickWindow)
{
    quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromAdapter(m_graphicsRequirements.adapterLuid.LowPart,
                                                                     m_graphicsRequirements.adapterLuid.HighPart,
                                                                     m_graphicsRequirements.minFeatureLevel));
}

QT_END_NAMESPACE
