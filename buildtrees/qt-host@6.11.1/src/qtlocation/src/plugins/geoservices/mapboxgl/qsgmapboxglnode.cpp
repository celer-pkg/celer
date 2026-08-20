// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qsgmapboxglnode.h"
#include "qgeomapmapboxgl.h"

#if __has_include(<QtQuick/private/qsgplaintexture_p.h>)
#include <QtQuick/private/qsgplaintexture_p.h>
#endif

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

// QSGMapboxGLTextureNode

static const QSize minTextureSize = QSize(64, 64);

QSGMapboxGLTextureNode::QSGMapboxGLTextureNode(const QMapboxGLSettings &settings, const QSize &size, qreal pixelRatio, QGeoMapMapboxGL *geoMap)
        : QSGSimpleTextureNode()
{
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    setFiltering(QSGTexture::Linear);

    m_map.reset(new QMapboxGL(nullptr, settings, size.expandedTo(minTextureSize), pixelRatio));

    QObject::connect(m_map.data(), &QMapboxGL::needsRendering, geoMap, &QGeoMap::sgNodeChanged);
    QObject::connect(m_map.data(), &QMapboxGL::copyrightsChanged, geoMap,
            static_cast<void (QGeoMap::*)(const QString &)>(&QGeoMapMapboxGL::copyrightsChanged));
}

void QSGMapboxGLTextureNode::resize(const QSize &size, qreal pixelRatio)
{
    const QSize& minSize = size.expandedTo(minTextureSize);
    const QSize fbSize = minSize * pixelRatio;
    m_map->resize(minSize);

    m_fbo.reset(new QOpenGLFramebufferObject(fbSize, QOpenGLFramebufferObject::CombinedDepthStencil));
    m_map->setFramebufferObject(m_fbo->handle(), fbSize);

    QSGPlainTexture *fboTexture = static_cast<QSGPlainTexture *>(texture());
    if (!fboTexture) {
        fboTexture = new QSGPlainTexture;
        fboTexture->setHasAlphaChannel(true);
    }

    fboTexture->setTextureId(m_fbo->texture());
    fboTexture->setTextureSize(fbSize);

    if (!texture()) {
        setTexture(fboTexture);
        setOwnsTexture(true);
    }

    setRect(QRectF(QPointF(), minSize));
    markDirty(QSGNode::DirtyGeometry);
}

void QSGMapboxGLTextureNode::render(QQuickWindow *window)
{
    QOpenGLFunctions *f = window->openglContext()->functions();
    f->glViewport(0, 0, m_fbo->width(), m_fbo->height());

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_fbo->bind();

    f->glClearColor(0.f, 0.f, 0.f, 0.f);
    f->glColorMask(true, true, true, true);
    f->glClear(GL_COLOR_BUFFER_BIT);

    m_map->render();
    m_fbo->release();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    window->resetOpenGLState();
    markDirty(QSGNode::DirtyMaterial);
}

QMapboxGL* QSGMapboxGLTextureNode::map() const
{
    return m_map.data();
}

// QSGMapboxGLRenderNode

QSGMapboxGLRenderNode::QSGMapboxGLRenderNode(const QMapboxGLSettings &settings, const QSize &size, qreal pixelRatio, QGeoMapMapboxGL *geoMap)
        : QSGRenderNode()
{
    m_map.reset(new QMapboxGL(nullptr, settings, size, pixelRatio));
    QObject::connect(m_map.data(), &QMapboxGL::needsRendering, geoMap, &QGeoMap::sgNodeChanged);
    QObject::connect(m_map.data(), &QMapboxGL::copyrightsChanged, geoMap,
            static_cast<void (QGeoMap::*)(const QString &)>(&QGeoMapMapboxGL::copyrightsChanged));
}

QMapboxGL* QSGMapboxGLRenderNode::map() const
{
    return m_map.data();
}

void QSGMapboxGLRenderNode::render(const RenderState *state)
{
    // QMapboxGL assumes we've prepared the viewport prior to render().
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
    f->glScissor(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
    f->glEnable(GL_SCISSOR_TEST);

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_map->render();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}

QSGRenderNode::StateFlags QSGMapboxGLRenderNode::changedStates() const
{
    return QSGRenderNode::DepthState
         | QSGRenderNode::StencilState
         | QSGRenderNode::ScissorState
         | QSGRenderNode::ColorState
         | QSGRenderNode::BlendState
         | QSGRenderNode::ViewportState
         | QSGRenderNode::RenderTargetState;
}

