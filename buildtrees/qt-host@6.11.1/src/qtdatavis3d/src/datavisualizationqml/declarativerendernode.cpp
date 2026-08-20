// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "declarativerendernode_p.h"
#include "abstractdeclarative_p.h"
#include <QtOpenGL/QOpenGLFramebufferObject>
#include <QtCore/QMutexLocker>

QT_BEGIN_NAMESPACE

DeclarativeRenderNode::DeclarativeRenderNode(AbstractDeclarative *declarative,
                                             const QSharedPointer<QMutex> &nodeMutex)
    : QSGGeometryNode(),
      m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4),
      m_texture(0),
      m_declarative(declarative),
      m_controller(0),
      m_fbo(0),
      m_multisampledFBO(0),
      m_window(0),
      m_samples(0),
      m_dirtyFBO(false)
{
    m_nodeMutex = nodeMutex;
    setMaterial(&m_material);
    setOpaqueMaterial(&m_materialO);
    setGeometry(&m_geometry);
    setFlag(UsePreprocess);
}

DeclarativeRenderNode::~DeclarativeRenderNode()
{
    delete m_fbo;
    delete m_multisampledFBO;
    delete m_texture;

    m_nodeMutex.clear();
}

void DeclarativeRenderNode::setSize(const QSize &size)
{
    if (size == m_size)
        return;

    m_size = size;
    m_dirtyFBO = true;
    markDirty(DirtyGeometry);
}

void DeclarativeRenderNode::update()
{
    if (m_dirtyFBO) {
        updateFBO();
        m_dirtyFBO = false;
    }
}

void DeclarativeRenderNode::updateFBO()
{
    m_declarative->activateOpenGLContext(m_window);

    if (m_fbo)
        delete m_fbo;

    m_fbo = new QOpenGLFramebufferObject(m_size);
    m_fbo->setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    // Multisampled
    if (m_multisampledFBO) {
        delete m_multisampledFBO;
        m_multisampledFBO = 0;
    }
    if (m_samples > 0) {
        QOpenGLFramebufferObjectFormat multisampledFrambufferFormat;
        multisampledFrambufferFormat.setSamples(m_samples);
        multisampledFrambufferFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

        m_multisampledFBO = new QOpenGLFramebufferObject(m_size, multisampledFrambufferFormat);
    }

    QSGGeometry::updateTexturedRectGeometry(&m_geometry,
                                            QRectF(0, 0,
                                                   m_size.width()
                                                   / m_controller->scene()->devicePixelRatio(),
                                                   m_size.height()
                                                   / m_controller->scene()->devicePixelRatio()),
                                            QRectF(0, 1, 1, -1));

    delete m_texture;
    const uint id = m_fbo->texture();
    m_texture = QNativeInterface::QSGOpenGLTexture::fromNative(id, m_window, m_size);
    m_material.setTexture(m_texture);
    m_materialO.setTexture(m_texture);

    m_declarative->doneOpenGLContext(m_window);
}

void DeclarativeRenderNode::setQuickWindow(QQuickWindow *window)
{
    Q_ASSERT(window);

    m_window = window;
}

void DeclarativeRenderNode::setController(Abstract3DController *controller)
{
    QMutexLocker locker(m_nodeMutex.data());
    m_controller = controller;
    if (m_controller) {
        connect(m_controller, &QObject::destroyed,
                this, &DeclarativeRenderNode::handleControllerDestroyed, Qt::DirectConnection);
    }
}

void DeclarativeRenderNode::setSamples(int samples)
{
    if (m_samples == samples)
        return;

    m_samples = samples;
    m_dirtyFBO = true;
}

void DeclarativeRenderNode::preprocess()
{
    QMutexLocker locker(m_nodeMutex.data());

    if (!m_controller)
        return;

    QOpenGLFramebufferObject *targetFBO;
    if (m_samples > 0)
        targetFBO = m_multisampledFBO;
    else
        targetFBO = m_fbo;

    m_declarative->activateOpenGLContext(m_window);

    targetFBO->bind();
    // Render scene here
    m_controller->render(targetFBO->handle());

    targetFBO->release();

    if (m_samples > 0)
        QOpenGLFramebufferObject::blitFramebuffer(m_fbo, m_multisampledFBO);

    m_declarative->doneOpenGLContext(m_window);
}

// This function is called within m_nodeMutex lock
void DeclarativeRenderNode::handleControllerDestroyed()
{
    m_controller = 0;
}

QT_END_NAMESPACE
