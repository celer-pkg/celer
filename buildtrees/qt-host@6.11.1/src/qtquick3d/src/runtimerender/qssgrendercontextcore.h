// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_CONTEXT_CORE_H
#define QSSG_RENDER_CONTEXT_CORE_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QtQuick3D API, with limited compatibility guarantees.
// Usage of this API may make your code source and binary incompatible with
// future versions of Qt.
//

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>

#include <QtCore/QPair>
#include <QtCore/QSize>
#include <memory>

QT_BEGIN_NAMESPACE

class QSSGRhiContext;
class QSSGBufferManager;
class QSSGRenderer;
class QSSGShaderLibraryManager;
class QSSGShaderCache;
class QSSGProgramGenerator;
class QSSGCustomMaterialSystem;
class QSSGRendererInterface;
class QSSGDebugDrawSystem;
class QSSGPerFrameAllocator;

class QQuickWindow;
class QRhi;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderContextInterface
{
    Q_DISABLE_COPY(QSSGRenderContextInterface)
public:
    // The commonly used version (from QQuick3DSceneRenderer). There is one
    // rendercontext per QQuickWindow (and so scenegraph render thread).
    explicit QSSGRenderContextInterface(QRhi *rhi);

    // One of these overloads must only be used in special cases, e.g. by the genshaders tool.
    QSSGRenderContextInterface(std::unique_ptr<QSSGBufferManager> bufferManager,
                               std::unique_ptr<QSSGRenderer> renderer,
                               std::shared_ptr<QSSGShaderLibraryManager> shaderLibraryManager,
                               std::unique_ptr<QSSGShaderCache> shaderCache,
                               std::unique_ptr<QSSGCustomMaterialSystem> customMaterialSystem,
                               std::unique_ptr<QSSGProgramGenerator> shaderProgramGenerator,
                               std::unique_ptr<QSSGRhiContext> ctx);
    QSSGRenderContextInterface(std::unique_ptr<QSSGBufferManager> bufferManager,
                               std::unique_ptr<QSSGRenderer> renderer,
                               std::shared_ptr<QSSGShaderLibraryManager> shaderLibraryManager,
                               std::unique_ptr<QSSGShaderCache> shaderCache,
                               std::unique_ptr<QSSGCustomMaterialSystem> customMaterialSystem,
                               std::unique_ptr<QSSGProgramGenerator> shaderProgramGenerator,
                               std::unique_ptr<QSSGRhiContext> ctx,
                               std::unique_ptr<QSSGDebugDrawSystem> debugDrawSystem);

    ~QSSGRenderContextInterface();

    const std::unique_ptr<QSSGRenderer> &renderer() const;
    const std::unique_ptr<QSSGBufferManager> &bufferManager() const;
    const std::unique_ptr<QSSGRhiContext> &rhiContext() const;
    const std::unique_ptr<QSSGShaderCache> &shaderCache() const;
    const std::shared_ptr<QSSGShaderLibraryManager> &shaderLibraryManager() const;
    const std::unique_ptr<QSSGCustomMaterialSystem> &customMaterialSystem() const;
    const std::unique_ptr<QSSGProgramGenerator> &shaderProgramGenerator() const;
    const std::unique_ptr<QSSGDebugDrawSystem> &debugDrawSystem() const;

private:
    friend class QQuick3DSceneRenderer;
    friend class QQuick3DWindowAttachment;
    friend class QSSGLayerRenderData;
    friend class QSSGRenderer;

    QRhi *rhi() const; // Internal convenience function
    void init();
    void releaseCachedResources();

    // The memory used for the per frame allocator is released as the first step in BeginFrame.
    // This is useful for short lived objects and datastructures.
    // NOTE: Only used internally and not replaceable for now.
    const std::unique_ptr<QSSGPerFrameAllocator> &perFrameAllocator() const;

    std::unique_ptr<QSSGRhiContext> m_rhiContext;
    std::unique_ptr<QSSGShaderCache> m_shaderCache;
    std::unique_ptr<QSSGBufferManager> m_bufferManager;
    std::unique_ptr<QSSGRenderer> m_renderer;
    std::shared_ptr<QSSGShaderLibraryManager> m_shaderLibraryManager;
    std::unique_ptr<QSSGCustomMaterialSystem> m_customMaterialSystem;
    std::unique_ptr<QSSGProgramGenerator> m_shaderProgramGenerator;
    std::unique_ptr<QSSGDebugDrawSystem> m_debugDrawSystem;
    std::unique_ptr<QSSGPerFrameAllocator> m_perFrameAllocator;
};
QT_END_NAMESPACE

#endif
