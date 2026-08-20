// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dscenerootnode_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>


QT_BEGIN_NAMESPACE

QQuick3DSceneRootNode::QQuick3DSceneRootNode(QQuick3DViewport *view3D, QQuick3DNode *parent)
    : QQuick3DNode(parent)
    , m_view3D(view3D)
{
}

QQuick3DSceneRootNode::~QQuick3DSceneRootNode()
{
}


QQuick3DViewport *QQuick3DSceneRootNode::view3D()
{
    return m_view3D;
}

QSSGRenderGraphObject *QQuick3DSceneRootNode::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderNode(QSSGRenderGraphObject::Type::SceneRoot);
    }

    return QQuick3DNode::updateSpatialNode(node);
}

QT_END_NAMESPACE
