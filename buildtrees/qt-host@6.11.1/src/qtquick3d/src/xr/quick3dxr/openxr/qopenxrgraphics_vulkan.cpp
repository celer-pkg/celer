// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qopenxrgraphics_vulkan_p.h"

#include "qopenxrhelpers_p.h"
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickGraphicsDevice>
#include <QtQuick/QQuickGraphicsConfiguration>
#include <QtQuick/private/qquickrendertarget_p.h>

#include <rhi/qrhi.h>

//#define XR_USE_GRAPHICS_API_VULKAN

QT_BEGIN_NAMESPACE

QOpenXRGraphicsVulkan::QOpenXRGraphicsVulkan()
{
    m_graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
}

bool QOpenXRGraphicsVulkan::initialize(const QVector<XrExtensionProperties> &extensions)
{
    return hasExtension(extensions, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
}

QVector<const char *> QOpenXRGraphicsVulkan::getRequiredExtensions() const
{
    return { XR_KHR_VULKAN_ENABLE_EXTENSION_NAME };
}


const XrBaseInStructure *QOpenXRGraphicsVulkan::handle() const
{
    return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
}


bool QOpenXRGraphicsVulkan::setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &quickConfig)
{
    // Setup Vulkan Instance.

    // In hybrid applications that also show Qt Quick windows on the desktop, it
    // is not ideal to create multiple VkInstances (as the on-screen
    // QQuickWindow(s) will have another one), but there is nothing we can do
    // due to the forced upfront nature of Vulkan API design. And we need to do
    // OpenXR API calls to get the things we need to create the instance. This
    // is hard to reconcile with Quick, that knows nothing about XrView and
    // such, and cannot predict the future either (i.e., "guess" if the user is
    // ever going to instantiate an XRView, and so on).
    //
    // This has no relevance for XR-only apps, and even the hybrid case this
    // works in practice, so we might just live with this for now.

    PFN_xrGetVulkanGraphicsRequirementsKHR pfnGetVulkanGraphicsRequirementsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance,
                                                       "xrGetVulkanGraphicsRequirementsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanGraphicsRequirementsKHR)),
                                 instance);

    if (!pfnGetVulkanGraphicsRequirementsKHR) {
        qWarning("Could not resolve xrGetVulkanGraphicsRequirementsKHR; perhaps the OpenXR implementation does not support Vulkan?");
        return false;
    }

    PFN_xrGetVulkanInstanceExtensionsKHR pfnGetVulkanInstanceExtensionsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance,
                                                       "xrGetVulkanInstanceExtensionsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanInstanceExtensionsKHR)),
                                 instance);

    XrGraphicsRequirementsVulkanKHR graphicsRequirements{};
    graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;
    OpenXRHelpers::checkXrResult(pfnGetVulkanGraphicsRequirementsKHR(instance,
                                                                     systemId,
                                                                     &graphicsRequirements),
                                 instance);

    quint32 extensionNamesSize = 0;
    OpenXRHelpers::checkXrResult(pfnGetVulkanInstanceExtensionsKHR(instance,
                                                                   systemId,
                                                                   0,
                                                                   &extensionNamesSize,
                                                                   nullptr),
                                 instance);

    QByteArray extensionNames;
    extensionNames.resize(extensionNamesSize);
    OpenXRHelpers::checkXrResult(pfnGetVulkanInstanceExtensionsKHR(instance,
                                                                   systemId,
                                                                   extensionNamesSize,
                                                                   &extensionNamesSize,
                                                                   extensionNames.data()),
                                 instance);

    // The last extension could have extra null characters but
    // the way we handle extenions doesn't handle null terminated
    // strings well, so we have to strip them ourselves
    auto stripNullChars = [](const QByteArray &string) {
        auto begin = string.begin();
        auto end = string.end();
        while (begin < end && end[-1] == '\x00')
            --end;
        return QByteArray(begin, end - begin);
    };

    QByteArrayList extensions = extensionNames.split(' ');
    for (auto &ext : extensions)
        ext = stripNullChars(ext);

    for (auto &rhiExt : QRhiVulkanInitParams::preferredInstanceExtensions()) {
        if (!extensions.contains(rhiExt))
            extensions.append(rhiExt);
    }

    m_vulkanInstance.setExtensions(extensions);

    // Multiview is a Vulkan 1.1 feature and won't work without setting up the instance accordingly.
    const QVersionNumber supportedVersion = m_vulkanInstance.supportedApiVersion();
    if (supportedVersion >= QVersionNumber(1, 3))
        m_vulkanInstance.setApiVersion(QVersionNumber(1, 3));
    else if (supportedVersion >= QVersionNumber(1, 2))
        m_vulkanInstance.setApiVersion(QVersionNumber(1, 2));
    else if (supportedVersion >= QVersionNumber(1, 1))
        m_vulkanInstance.setApiVersion(QVersionNumber(1, 1));

    if (quickConfig.isDebugLayerEnabled())
        m_vulkanInstance.setLayers({ "VK_LAYER_LUNARG_standard_validation" });

    if (!m_vulkanInstance.create()) {
        qWarning("Quick 3D XR: Failed to create Vulkan instance");
        return false;
    }

    // Get Vulkan device extensions
    PFN_xrGetVulkanDeviceExtensionsKHR pfnGetVulkanDeviceExtensionsKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance,
                                                       "xrGetVulkanDeviceExtensionsKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanDeviceExtensionsKHR)),
                                 instance);

    uint32_t deviceExtensionNamesSize = 0;
    OpenXRHelpers::checkXrResult(pfnGetVulkanDeviceExtensionsKHR(instance,
                                                                 systemId,
                                                                 0,
                                                                 &deviceExtensionNamesSize,
                                                                 nullptr),
                                 instance);
    QByteArray deviceExtensionNames;
    deviceExtensionNames.resize(deviceExtensionNamesSize);
    OpenXRHelpers::checkXrResult(pfnGetVulkanDeviceExtensionsKHR(instance,
                                                                 systemId,
                                                                 deviceExtensionNamesSize,
                                                                 &deviceExtensionNamesSize,
                                                                 deviceExtensionNames.data()),
                                 instance);

    auto deviceExtensions = deviceExtensionNames.split(' ');
    for (auto &ext : deviceExtensions) {
        ext = stripNullChars(ext);
    }
    m_graphicsConfiguration.setDeviceExtensions(deviceExtensions);

    // Get the Vulkan Graphics Device
    PFN_xrGetVulkanGraphicsDeviceKHR pfnGetVulkanGraphicsDeviceKHR = nullptr;
    OpenXRHelpers::checkXrResult(xrGetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR",
                                                       reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetVulkanGraphicsDeviceKHR)), instance);

    OpenXRHelpers::checkXrResult(pfnGetVulkanGraphicsDeviceKHR(instance, systemId, m_vulkanInstance.vkInstance(), &m_vulkanPhysicalDevice), instance);

    return true;
}

bool QOpenXRGraphicsVulkan::finializeGraphics(QRhi *rhi)
{
    const QRhiVulkanNativeHandles *vulkanRhi = static_cast<const QRhiVulkanNativeHandles *>(rhi->nativeHandles());
    m_vulkanDevice = vulkanRhi->dev;
    Q_ASSERT(m_vulkanPhysicalDevice == vulkanRhi->physDev);
    m_vulkanCommandQueue = vulkanRhi->gfxQueue;
    m_queueFamilyIndex = vulkanRhi->gfxQueueFamilyIdx;

    m_graphicsBinding.instance = m_vulkanInstance.vkInstance();
    m_graphicsBinding.physicalDevice = m_vulkanPhysicalDevice;
    m_graphicsBinding.device = m_vulkanDevice;
    m_graphicsBinding.queueFamilyIndex = m_queueFamilyIndex;
    m_graphicsBinding.queueIndex = 0;

    m_rhi = rhi;

    return true;
}


int64_t QOpenXRGraphicsVulkan::colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // List of supported color swapchain formats.
    constexpr int64_t supportedColorSwapchainFormats[] = {
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM
    };

    auto swapchainFormatIt = std::find_first_of(std::begin(supportedColorSwapchainFormats),
                                                std::end(supportedColorSwapchainFormats),
                                                swapchainFormats.begin(),
                                                swapchainFormats.end());
    return *swapchainFormatIt;
}

int64_t QOpenXRGraphicsVulkan::depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const
{
    // in order of preference
    QVarLengthArray<int64_t, 4> supportedDepthSwapchainFormats;
    constexpr std::pair<int64_t, QRhiTexture::Format> nativeDepthSwapchainFormats[] = {
        { VK_FORMAT_D24_UNORM_S8_UINT, QRhiTexture::D24S8 },
        { VK_FORMAT_D32_SFLOAT_S8_UINT, QRhiTexture::D32F },
        { VK_FORMAT_D32_SFLOAT, QRhiTexture::D32F },
        { VK_FORMAT_D16_UNORM, QRhiTexture::D16 }
    };
    for (const auto &format : nativeDepthSwapchainFormats) {
        if (m_rhi->isTextureFormatSupported(format.second))
            supportedDepthSwapchainFormats.append(format.first);
    }

    if (supportedDepthSwapchainFormats.isEmpty())
        return 0;

    // order matters, we prefer D24S8 above all the others
    return *std::find_first_of(std::begin(supportedDepthSwapchainFormats),
                               std::end(supportedDepthSwapchainFormats),
                               swapchainFormats.begin(),
                               swapchainFormats.end());
}

QVector<XrSwapchainImageBaseHeader*> QOpenXRGraphicsVulkan::allocateSwapchainImages(int count, XrSwapchain swapchain)
{
    QVector<XrSwapchainImageBaseHeader*> swapchainImages;
    QVector<XrSwapchainImageVulkanKHR> swapchainImageBuffer(count);
    for (XrSwapchainImageVulkanKHR& image : swapchainImageBuffer) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
        swapchainImages.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
    }
    m_swapchainImageBuffer.insert(swapchain, swapchainImageBuffer);
    return swapchainImages;
}


QQuickRenderTarget QOpenXRGraphicsVulkan::renderTarget(const XrSwapchainSubImage &subImage,
                                                       const XrSwapchainImageBaseHeader *swapchainImage,
                                                       quint64 swapchainFormat,
                                                       int samples,
                                                       int arraySize,
                                                       const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                                       quint64 depthSwapchainFormat) const
{
    VkImage colorTexture = reinterpret_cast<const XrSwapchainImageVulkanKHR*>(swapchainImage)->image;

    VkFormat viewFormat = VkFormat(swapchainFormat);
    switch (swapchainFormat) {
    case VK_FORMAT_R8G8B8A8_SRGB:
        viewFormat = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    case VK_FORMAT_B8G8R8A8_SRGB:
        viewFormat = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    default:
        break;
    }

    QQuickRenderTarget::Flags flags;
    if (samples > 1)
        flags |= QQuickRenderTarget::Flag::MultisampleResolve;

    const QSize pixelSize(subImage.imageRect.extent.width, subImage.imageRect.extent.height);
    QQuickRenderTarget rt = QQuickRenderTarget::fromVulkanImage(colorTexture,
                                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                                VkFormat(swapchainFormat),
                                                                viewFormat,
                                                                pixelSize,
                                                                samples,
                                                                arraySize,
                                                                flags);
    if (depthSwapchainImage) {
        // There might be issues with stencil when MSAA is not used and the
        // format is D16 or D32F or the half-unsupported D32FS8 (because then
        // the OpenXR-provided texture is the one and only depth-stencil buffer,
        // perhaps without stencil). But we prefer D24S8 whenever that's
        // available, so hopefully this problem won't come up in practice.
        QRhiTexture::Format format = QRhiTexture::D24S8;
        switch (depthSwapchainFormat) {
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
            format = QRhiTexture::D32F;
            break;
        case VK_FORMAT_D16_UNORM:
            format = QRhiTexture::D16;
            break;
        }
        VkImage depthImage = reinterpret_cast<const XrSwapchainImageVulkanKHR*>(depthSwapchainImage)->image;
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
        m_depthTexture->createFrom({ quint64(depthImage), VK_IMAGE_LAYOUT_UNDEFINED });
        rt.setDepthTexture(m_depthTexture);
    }
    return rt;
}

void QOpenXRGraphicsVulkan::setupWindow(QQuickWindow *quickWindow)
{
    quickWindow->setGraphicsDevice(QQuickGraphicsDevice::fromPhysicalDevice(m_vulkanPhysicalDevice));
    quickWindow->setGraphicsConfiguration(m_graphicsConfiguration);
    quickWindow->setVulkanInstance(&m_vulkanInstance);
}

void QOpenXRGraphicsVulkan::releaseResources()
{
    delete m_depthTexture;
    m_depthTexture = nullptr;
}

QT_END_NAMESPACE
