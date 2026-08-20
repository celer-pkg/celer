// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVERENDERNODE_P_H
#define DECLARATIVERENDERNODE_P_H

#include <private/datavisualizationglobal_p.h>

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGTextureMaterial>
#include <QtQuick/QQuickWindow>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class Abstract3DController;
class AbstractDeclarative;

class DeclarativeRenderNode : public QObject, public QSGGeometryNode
{
    Q_OBJECT
public:
    DeclarativeRenderNode(AbstractDeclarative *declarative,
                          const QSharedPointer<QMutex> &nodeMutex);
    ~DeclarativeRenderNode();

    void setSize(const QSize &size);
    QSize size() const { return m_size; }

    void update();
    void updateFBO();

    void setController(Abstract3DController *controller);
    void setQuickWindow(QQuickWindow *window);
    void setSamples(int samples);

    void preprocess() override;

public Q_SLOTS:
    void handleControllerDestroyed();

private:
    QSGTextureMaterial m_material;
    QSGOpaqueTextureMaterial m_materialO;
    QSGGeometry m_geometry;
    QSGTexture *m_texture;
    QSize m_size;

    AbstractDeclarative *m_declarative;
    Abstract3DController *m_controller;
    QOpenGLFramebufferObject *m_fbo;
    QOpenGLFramebufferObject *m_multisampledFBO;
    QQuickWindow *m_window;
    int m_samples;

    bool m_dirtyFBO;

    QSharedPointer<QMutex> m_nodeMutex;

};

QT_END_NAMESPACE

#endif
