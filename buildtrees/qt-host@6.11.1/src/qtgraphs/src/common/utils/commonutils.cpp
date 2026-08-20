// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGui/qtguiglobal.h>
#if QT_CONFIG(opengl)
#include <QtGui/qoffscreensurface.h>
#endif

#include "commonutils_p.h"

#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

static qreal s_maxTextureSize = 0.;

Q_LOGGING_CATEGORY(lcGraphsUtils, "qt.graphs.common.utils")

qreal CommonUtils::maxTextureSize()
{
    // Query maximum texture size only once
    if (!s_maxTextureSize) {
        std::unique_ptr<QRhi> rhi;
#if defined(Q_OS_WIN)
        QRhiD3D12InitParams params;
        rhi.reset(QRhi::create(QRhi::D3D12, &params));
#elif defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        QRhiMetalInitParams params;
        rhi.reset(QRhi::create(QRhi::Metal, &params));
#elif QT_CONFIG(opengl)
        QRhiGles2InitParams params;
        params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
        rhi.reset(QRhi::create(QRhi::OpenGLES2, &params));
#elif QT_CONFIG(vulkan)
        if (!qEnvironmentVariable("QSG_RHI_BACKEND").compare("vulkan")) {
            QVulkanInstance inst;
            inst.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
            if (inst.create()) {
                QRhiVulkanInitParams params;
                params.inst = &inst;
                rhi.reset(QRhi::create(QRhi::Vulkan, &params));
            } else {
                qCWarning(lcGraphsUtils, "Failed to create Vulkan instance");
            }
        }
#endif
        if (rhi)
            s_maxTextureSize = qreal(rhi->resourceLimit(QRhi::TextureSizeMax));
        else
            s_maxTextureSize = gradientTextureWidth;
    }

    return s_maxTextureSize;
}

QT_END_NAMESPACE
