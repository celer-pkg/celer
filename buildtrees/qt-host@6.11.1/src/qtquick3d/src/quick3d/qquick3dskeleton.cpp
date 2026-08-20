// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dskeleton_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dnode_p_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderskeleton_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Skeleton
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief Defines a skeletal animation hierarchy.

    A skeleton defines how a model can be animated using \l {Vertex Skinning}
    {skeletal animation}. It contains a hierarchy of \l {Joint} nodes. Each joint can be
    transformed for a skinning animation.

    \qml
    Skeleton {
        id: qmlskeleton
        Joint {
            id: joint0
            index: 0
            skeletonRoot: qmlskeleton
            Joint {
                id: joint1
                index: 1
                skeletonRoot: qmlskeleton
            }
        }
    }
    \endqml

    \sa {Joint::skeletonRoot}, {Model::skeleton}, {Qt Quick 3D - Simple Skinning Example#skeleton-and-joint-hierarchy}
*/

QQuick3DSkeleton::QQuick3DSkeleton(QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::Skeleton)), parent)
{
}

QQuick3DSkeleton::~QQuick3DSkeleton()
{
}

QSSGRenderGraphObject *QQuick3DSkeleton::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new QSSGRenderSkeleton();

    QQuick3DNode::updateSpatialNode(node);

    auto skeletonNode = static_cast<QSSGRenderSkeleton *>(node);

    return skeletonNode;
}

QT_END_NAMESPACE
