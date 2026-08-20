// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QOPENXRGRAPHICSOPENGL_H
#define QOPENXRGRAPHICSOPENGL_H

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

QT_BEGIN_NAMESPACE

class QRhiTexture;

class QOpenXRGraphicsOpenGL : public QAbstractOpenXRGraphics
{
public:
    QOpenXRGraphicsOpenGL();

    enum PlatformType { PLATFORM_UNSUPPORTED, PLATFORM_EGL, PLATFORM_WAYLAND, PLATFORM_XLIB, PLATFORM_WIN32 };

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
    void setupWindow(QQuickWindow *window) override;
    QRhi *rhi() const override { return m_rhi; }
    void releaseResources() override;

private:
    enum PlatformType m_selectedPlatform = PLATFORM_UNSUPPORTED;
    QVector<const char *> m_requiredExtensions;
    std::unique_ptr<XrBaseInStructure, decltype(&std::free)> m_graphicsBinding = { nullptr, std::free };

    QMap<XrSwapchain, QVector<XrSwapchainImageOpenGLKHR>> m_swapchainImageBuffer;

    XrGraphicsRequirementsOpenGLKHR m_graphicsRequirements{};
    QWindow *m_window = nullptr;
    QRhi *m_rhi = nullptr;
    mutable QRhiTexture *m_depthTexture = nullptr;
};

QT_END_NAMESPACE

#endif // QOPENXRGRAPHICSOPENGL_H
