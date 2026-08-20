// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgperframeallocator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DRuntimeRender/private/qssgdebugdrawsystem_p.h>

#include <QtQuick/private/qquickwindow_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QSSGRenderContextInterface
    \inmodule QtQuick3D
    \since 6.7

    \brief Aggregate class for sub-parts of the QtQuick3D rendering engine.

    The QSSGRenderContextInterface, and the objects owned by it are always
    per-QQuickWindow, and so per scenegraph render thread. Some resources might
    be shared, like the shader library, but that's all take care of internally
    by the QSSGRenderContextInterface.

    \note Some sub-components might not be exposed as semi-public, or their use might require
    private headers to be used. In those cases it's likely their APIs have a high likelihood
    of being changed in the near future. One the APIs for those class have stabilized they will
    be made available with the same guarantee as the rest of the semi-public APIs.
*/

static bool loadPregenratedShaders()
{
    return qEnvironmentVariableIntValue("QT_QUICK3D_DISABLE_GENSHADERS") == 0;
}

void QSSGRenderContextInterface::init()
{
    if (m_renderer)
        QSSGRendererPrivate::setRenderContextInterface(*m_renderer, this);

    if (m_bufferManager)
        m_bufferManager->setRenderContextInterface(this);

    if (m_customMaterialSystem)
        m_customMaterialSystem->setRenderContextInterface(this);
    if (m_shaderLibraryManager && loadPregenratedShaders())
        m_shaderLibraryManager->loadPregeneratedShaderInfo();
}

void QSSGRenderContextInterface::releaseCachedResources()
{
    if (m_renderer)
        m_renderer->releaseCachedResources();
    if (m_shaderCache)
        m_shaderCache->releaseCachedResources();
    if (m_customMaterialSystem)
        m_customMaterialSystem->releaseCachedResources();
    if (m_bufferManager)
        m_bufferManager->releaseCachedResources();
    if (m_rhiContext)
        QSSGRhiContextPrivate::get(m_rhiContext.get())->releaseCachedResources();
}

const std::unique_ptr<QSSGPerFrameAllocator> &QSSGRenderContextInterface::perFrameAllocator() const
{
    return m_perFrameAllocator;
}

/*!
    \internal
 */
QSSGRenderContextInterface::QSSGRenderContextInterface(std::unique_ptr<QSSGBufferManager> bufferManager,
                                                       std::unique_ptr<QSSGRenderer> renderer,
                                                       std::shared_ptr<QSSGShaderLibraryManager> shaderLibraryManager,
                                                       std::unique_ptr<QSSGShaderCache> shaderCache,
                                                       std::unique_ptr<QSSGCustomMaterialSystem> customMaterialSystem,
                                                       std::unique_ptr<QSSGProgramGenerator> shaderProgramGenerator,
                                                       std::unique_ptr<QSSGRhiContext> ctx)
    : QSSGRenderContextInterface {
        std::move(bufferManager),
        std::move(renderer),
        std::move(shaderLibraryManager),
        std::move(shaderCache),
        std::move(customMaterialSystem),
        std::move(shaderProgramGenerator),
        std::move(ctx),
        nullptr,
    }
{
}

/*!
    \internal
 */
QSSGRenderContextInterface::QSSGRenderContextInterface(std::unique_ptr<QSSGBufferManager> bufferManager,
                                                       std::unique_ptr<QSSGRenderer> renderer,
                                                       std::shared_ptr<QSSGShaderLibraryManager> shaderLibraryManager,
                                                       std::unique_ptr<QSSGShaderCache> shaderCache,
                                                       std::unique_ptr<QSSGCustomMaterialSystem> customMaterialSystem,
                                                       std::unique_ptr<QSSGProgramGenerator> shaderProgramGenerator,
                                                       std::unique_ptr<QSSGRhiContext> ctx,
                                                       std::unique_ptr<QSSGDebugDrawSystem> debugDrawSystem)
    : m_rhiContext(std::move(ctx))
    , m_shaderCache(std::move(shaderCache))
    , m_bufferManager(std::move(bufferManager))
    , m_renderer(std::move(renderer))
    , m_shaderLibraryManager(std::move(shaderLibraryManager))
    , m_customMaterialSystem(std::move(customMaterialSystem))
    , m_shaderProgramGenerator(std::move(shaderProgramGenerator))
    , m_debugDrawSystem(std::move(debugDrawSystem))
    , m_perFrameAllocator(new QSSGPerFrameAllocator)
{
    init();
}

// The shader library is a global object, not per-QQuickWindow, hence not owned
// by the QSSGRenderContextInterface.
static const std::shared_ptr<QSSGShaderLibraryManager> &q3ds_shaderLibraryManager()
{
    static auto shaderLibraryManager = std::make_shared<QSSGShaderLibraryManager>();
    return shaderLibraryManager;
}

/*!
    \internal
 */
QSSGRenderContextInterface::QSSGRenderContextInterface(QRhi *rhi)
    : m_rhiContext(new QSSGRhiContext(rhi))
    , m_shaderCache(new QSSGShaderCache(*m_rhiContext))
    , m_bufferManager(new QSSGBufferManager())
    , m_renderer(new QSSGRenderer())
    , m_shaderLibraryManager(q3ds_shaderLibraryManager())
    , m_customMaterialSystem(new QSSGCustomMaterialSystem())
    , m_shaderProgramGenerator(new QSSGProgramGenerator())
    , m_debugDrawSystem(new QSSGDebugDrawSystem())
    , m_perFrameAllocator(new QSSGPerFrameAllocator)
{
    init();
}

/*!
    \internal
 */
QSSGRenderContextInterface::~QSSGRenderContextInterface()
{
    m_renderer->releaseCachedResources();
}

/*!
    \internal
 */
const std::unique_ptr<QSSGRenderer> &QSSGRenderContextInterface::renderer() const
{
    return m_renderer;
}

/*!
    \internal
 */
const std::unique_ptr<QSSGBufferManager> &QSSGRenderContextInterface::bufferManager() const
{
    return m_bufferManager;
}

/*!
    \return the context object that wraps QRhi-related settings and helper functionality.
 */
const std::unique_ptr<QSSGRhiContext> &QSSGRenderContextInterface::rhiContext() const
{
    return m_rhiContext;
}

/*!
    \internal
 */
const std::unique_ptr<QSSGShaderCache> &QSSGRenderContextInterface::shaderCache() const
{
    return m_shaderCache;
}

/*!
    \internal
 */
const std::shared_ptr<QSSGShaderLibraryManager> &QSSGRenderContextInterface::shaderLibraryManager() const
{
    return m_shaderLibraryManager;
}

/*!
    \internal
 */
const std::unique_ptr<QSSGCustomMaterialSystem> &QSSGRenderContextInterface::customMaterialSystem() const
{
    return m_customMaterialSystem;
}

/*!
    \internal
 */
const std::unique_ptr<QSSGProgramGenerator> &QSSGRenderContextInterface::shaderProgramGenerator() const
{
    return m_shaderProgramGenerator;
}

/*!
    \internal
 */
const std::unique_ptr<QSSGDebugDrawSystem> &QSSGRenderContextInterface::debugDrawSystem() const
{
    return m_debugDrawSystem;
}

QRhi *QSSGRenderContextInterface::rhi() const
{
    return m_rhiContext->rhi();
}

QT_END_NAMESPACE

