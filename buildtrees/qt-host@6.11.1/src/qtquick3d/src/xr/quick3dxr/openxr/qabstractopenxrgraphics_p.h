// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QABSTRACTOPENXRGRAPHICS_H
#define QABSTRACTOPENXRGRAPHICS_H

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


#include <QtQuick3DXr/qtquick3dxrglobal.h>

#include <openxr/openxr.h>
#include <QtCore/QVector>
#include <QtQuick/QQuickRenderTarget>

QT_BEGIN_NAMESPACE

class QRhi;
class QQuickWindow;
class QQuickGraphicsConfiguration;

class QAbstractOpenXRGraphics
{
public:
    QAbstractOpenXRGraphics();
    virtual ~QAbstractOpenXRGraphics() { }

    virtual bool initialize(const QVector<XrExtensionProperties> &extensions) = 0;
    virtual QVector<const char *> getRequiredExtensions() const = 0;
    virtual const XrBaseInStructure* handle() const = 0;
    virtual bool setupGraphics(const XrInstance &instance,
                               XrSystemId &systemId,
                               const QQuickGraphicsConfiguration &quickConfig) = 0;
    virtual void setupWindow(QQuickWindow *);
    virtual bool finializeGraphics(QRhi *rhi) = 0;
    virtual int64_t colorSwapchainFormat(const QVector<int64_t> &swapchainFormats) const = 0;
    virtual int64_t depthSwapchainFormat(const QVector<int64_t> &swapchainFormats) const = 0;
    virtual QVector<XrSwapchainImageBaseHeader*> allocateSwapchainImages(int count,
                                                                         XrSwapchain swapchain) = 0;

    virtual QQuickRenderTarget renderTarget(const XrSwapchainSubImage &subImage,
                                            const XrSwapchainImageBaseHeader *swapchainImage,
                                            quint64 swapchainFormat,
                                            int samples,
                                            int arraySize,
                                            const XrSwapchainImageBaseHeader *depthSwapchainImage,
                                            quint64 depthSwapchainFormat) const = 0;

    virtual QRhi *rhi() const = 0;
    virtual void releaseResources() { }
    static bool hasExtension(const QVector<XrExtensionProperties> &extensions, const char *extensionName);
};

QT_END_NAMESPACE

#endif // QABSTRACTOPENXRGRAPHICS_H
