// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QOPENXRGRAPHICSD3D12_H
#define QOPENXRGRAPHICSD3D12_H

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

QT_BEGIN_NAMESPACE

class QOpenXRGraphicsD3D12 : public QAbstractOpenXRGraphics
{
public:
    QOpenXRGraphicsD3D12();

    bool initialize(const QVector<XrExtensionProperties> &extensions) override;
    QVector<const char *> getRequiredExtensions() const override;
    const XrBaseInStructure *handle() const override;
    bool setupGraphics(const XrInstance &instance, XrSystemId &systemId, const QQuickGraphicsConfiguration &quickConfig) override;
    bool finializeGraphics(QRhi *rhi) override;
    int64_t colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const override;
    int64_t depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const override;
    QVector<XrSwapchainImageBaseHeader*> allocateSwapchainImages(int count, XrSwapchain swapchain) override;
    QQuickRenderTarget renderTarget(const XrSwapchainSubImage &subImage, const XrSwapchainImageBaseHeader *swapchainImage,
                                    quint64 swapchainFormat, int samples, int arraySize,
                                    const XrSwapchainImageBaseHeader *depthSwapchainImage, quint64 depthSwapchainFormat) const override;
    QRhi *rhi() const override { return m_rhi; }
    void setupWindow(QQuickWindow *quickWindow) override;

private:
    QRhi *m_rhi = nullptr;
    XrGraphicsBindingD3D12KHR m_graphicsBinding = {};
    QMap<XrSwapchain, QVector<XrSwapchainImageD3D12KHR>> m_swapchainImageBuffer;
    XrGraphicsRequirementsD3D12KHR m_graphicsRequirements = {};
};

QT_END_NAMESPACE

#endif // QOPENXRGRAPHICSD3D12_H
