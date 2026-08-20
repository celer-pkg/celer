// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSGMAPBOXGLNODE_H
#define QSGMAPBOXGLNODE_H

#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRenderNode>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtOpenGL/QOpenGLFramebufferObject>

#include <QMapboxGL>

class QGeoMapMapboxGL;

class QSGMapboxGLTextureNode : public QSGSimpleTextureNode
{
public:
    QSGMapboxGLTextureNode(const QMapboxGLSettings &, const QSize &, qreal pixelRatio, QGeoMapMapboxGL *geoMap);

    QMapboxGL* map() const;

    void resize(const QSize &size, qreal pixelRatio);
    void render(QQuickWindow *);

private:
    std::unique_ptr<QMapboxGL> m_map;
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
};

class QSGMapboxGLRenderNode : public QSGRenderNode
{
public:
    QSGMapboxGLRenderNode(const QMapboxGLSettings &, const QSize &, qreal pixelRatio, QGeoMapMapboxGL *geoMap);

    QMapboxGL* map() const;

    // QSGRenderNode
    void render(const RenderState *state) override;
    StateFlags changedStates() const override;

private:
    std::unique_ptr<QMapboxGL> m_map;
};

#endif // QSGMAPBOXGLNODE_H
