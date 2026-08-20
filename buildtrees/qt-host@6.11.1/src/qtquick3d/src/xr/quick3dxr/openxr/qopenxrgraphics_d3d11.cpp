// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qopenxrgraphics_d3d11_p.h"

#include "qopenxrhelpers_p.h"

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickGraphicsDevice>

#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

QOpenXRGraphicsD3D11::QOpenXRGraphicsD3D11()
{
    m_graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
    m_graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
}

bool QOpenXRGraphicsD3D11::initialize(const QVector<XrExtensionProperties> &extensions)
{
    return hasExtension(extensions, XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
}

QVector<const char *> QOpenXRGraphicsD3D11::getRequiredExtensions() const
{
    return { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };
}


const XrBaseInStructure *QOpenXRGraphicsD3D11::handle() const
{
    return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
}


bool QOpenXRGraphicsD3D11::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &)
{
    PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetD3D11GraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetD3D11GraphicsRequirementsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetD3D11GraphicsRequirementsKHR)),
                                 instance);

    if (!pfnGetD3D11GraphicsRequirementsKHR) {
        qWarning("Could not resolve xrGetD3D11GraphicsRequirementsKHR; perhaps the OpenXR implementation does not support D3D11?");
        return false;
    }

    // Create the D3D11 device for the adapter associated with the system.
    OpenXRHelpers::checkXrResult(pfnGetD3D11GraphicsRequirementsKHR(instance, systemId, &m_graphicsRequirements),
                                 instance);
    return true;
}

bool QOpenXRGraphicsD3D11::finializeGraphics(QRhi *rhi)
{
    const QRhiD3D11NativeHandles *d3d11Rhi = static_cast<const QRhiD3D11NativeHandles *>(rhi->nativeHandles());
    m_graphicsBinding.device = reinterpret_cast<ID3D11Device*>(d3d11Rhi->dev);
    m_rhi = rhi;

    return true;
}


int64_t QOpenXRGraphicsD3D11::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
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

int64_t QOpenXRGraphicsD3D11::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
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

QVector<XrSwapchainImageBaseHeader*> QOpenXRGraphicsD3D11::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageD3D11KHR> swapchainImageBuffer(count);
    for (XrSwapchainImageD3D11KHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
        swapchainImages.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}


QQuickRenderTarget QOpenXRGraphicsD3D11::renderTarget(const XrSwapchainSubImage &subImage,
                                                      const XrSwapchainImageBaseHeader *swapchainImage,
                                                      quint64 swapchainFormat,
                                                      int samples,
                                                      int arraySize,
                                                      const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                      quint64 depthSwapchainFormat) const
{
    ID3D11Texture2D* const colorTexture = reinterpret_cast<const XrSwapchainImageD3D11KHR*>(swapchainImage)->texture;

    // No real view format support for D3D11 in QRhi, but can strip off the
    // _SRGB and pass that in as the texture format, which in the end also
    // avoids incorrect (unwanted) linear->sRGB conversions.
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

    if (arraySize > 1)
        qWarning("Qt Quick 3D XR: The D3D11 integration has no support for multiview");

    QQuickRenderTarget::Flags flags;
    if (samples > 1)
        flags |= QQuickRenderTarget::Flag::MultisampleResolve;

    return QQuickRenderTarget::fromD3D11Texture(colorTexture,
                                                viewFormat,
                                                QSize(subImage.imageRect.extent.width, subImage.imageRect.extent.height),
                                                samples,
                                                flags);

    // No depthSwapchainImage support because ResolveDepthStencil will be
    // unsupported with D3D11/12 no matter what.
    Q_UNUSED(depthSwapchainImage);
    Q_UNUSED(depthSwapchainFormat);
}


void QOpenXRGraphicsD3D11::setupWindow(QQuickWindow *quickWindow)
{
    quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromAdapter(m_graphicsRequirements.adapterLuid.LowPart,
                                                                     m_graphicsRequirements.adapterLuid.HighPart,
                                                                     m_graphicsRequirements.minFeatureLevel));
}

QT_END_NAMESPACE
