// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QOPENXRGRAPHICSMETAL_H
#define QOPENXRGRAPHICSMETAL_H

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
#include <QtQuick3DXr/private/qopenxrplatform_p.h>
#include <QtCore/QMap>

Q_FORWARD_DECLARE_OBJC_CLASS(MTLDevice);
Q_FORWARD_DECLARE_OBJC_CLASS(MTLCommandQueue);

QT_BEGIN_NAMESPACE

class QOpenXRGraphicsMetal : public QAbstractOpenXRGraphics
{
public:
    QOpenXRGraphicsMetal();

    bool initialize(const QVector<XrExtensionProperties> &extensions) override;
    QVector<const char *> getRequiredExtensions() const override;
    const XrBaseInStructure *handle() const override;
    bool setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &quickConfig) override;
    bool finializeGraphics(QRhi *rhi) override;
    int64_t colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const override;
    int64_t depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const override;
    QVector<XrSwapchainImageBaseHeader *> allocateSwapchainImages(int count, XrSwapchain swapchain) override;
    QQuickRenderTarget renderTarget(const XrSwapchainSubImage &subImage, const XrSwapchainImageBaseHeader *swapchainImage, quint64 swapchainFormat, int samples, int arraySize, const XrSwapchainImageBaseHeader *depthSwapchainImage, quint64 depthSwapchainFormat) const override;
    QRhi *rhi() const override;
    void setupWindow(QQuickWindow *) override;
private:
    QRhi *m_rhi = nullptr;
    MTLDevice *m_device = nullptr;
    MTLCommandQueue *m_commandQueue = nullptr;
    XrGraphicsBindingMetalKHR m_graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_METAL_KHR, nullptr, nullptr};
    QMap<XrSwapchain, QVector<XrSwapchainImageMetalKHR>> m_swapchainImageBuffer;
    XrGraphicsRequirementsMetalKHR m_graphicsRequirements = {XR_TYPE_GRAPHICS_REQUIREMENTS_METAL_KHR, nullptr, nullptr};
};


QT_END_NAMESPACE

#endif // QOPENXRGRAPHICSMETAL_H
