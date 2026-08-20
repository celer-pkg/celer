// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3djoint_p.h"
#include "qquick3dskeleton_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dnode_p_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderjoint_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Joint
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief Defines a node in a skeletal animation hierarchy.

    A joint is a transformable node inside a \l {Skeleton}, used for \l {Vertex Skinning}
    {skeletal animation}. It is called a "joint" because it can be seen as a joint between the bones
    of a skeleton.

    All the joints must be contained inside a Skeleton, and each joint must have a \l skeletonRoot
    pointing back to that skeleton.

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

*/

QQuick3DJoint::QQuick3DJoint(QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::Joint)), parent)
{
}

QQuick3DJoint::~QQuick3DJoint()
{
    disconnect(m_skeletonConnection);
}

/*!
    \qmlproperty int Joint::index

    Specifies the index of this joint. This index value is used in the \c JointSemantic
    \l {QQuick3DGeometry::addAttribute}{custom geometry attribute}.

    \note Index values must be unique within the same \l {Skeleton}.
    \note Negative values cannot be assigned.

    \sa {QQuick3DGeometry::addAttribute}, {Qt Quick 3D - Simple Skinning Example}
*/

qint32 QQuick3DJoint::index() const
{
    return m_index;
}

/*!
    \qmlproperty Skeleton Joint::skeletonRoot

    Specifies the \l {Skeleton} that contains this joint.

    \note All the \l {Joint}s in the \l {Skeleton} must have the same skeletonRoot.
    If not, the animation will be broken.

    \sa {Skeleton}
*/

QQuick3DSkeleton *QQuick3DJoint::skeletonRoot() const
{
    return m_skeletonRoot;
}

void QQuick3DJoint::setIndex(qint32 index)
{
    if (m_index == index)
        return;
    if (index < 0)
        return;

    m_index = index;
    m_indexDirty = true;
    emit indexChanged();
}

void QQuick3DJoint::setSkeletonRoot(QQuick3DSkeleton *skeleton)
{
    if (skeleton == m_skeletonRoot)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DJoint::setSkeletonRoot, skeleton, m_skeletonRoot);
    if (m_skeletonRoot)
        QObject::disconnect(m_skeletonConnection);

    m_skeletonRoot = skeleton;

    if (m_skeletonRoot) {
        m_skeletonConnection = connect(this, &QQuick3DJoint::sceneTransformChanged,
                                       skeleton, [skeleton]() {
                                   auto skeletonNode = static_cast<QSSGRenderSkeleton *>(QQuick3DNodePrivate::get(skeleton)->spatialNode);
                                   if (skeletonNode)
                                       skeletonNode->skinningDirty = true;
                                });
    }
    m_skeletonRootDirty = true;
    emit skeletonRootChanged();
}


void QQuick3DJoint::markAllDirty()
{
    m_indexDirty = true;
    m_skeletonRootDirty = true;
    QQuick3DNode::markAllDirty();
}

QSSGRenderGraphObject *QQuick3DJoint::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!m_skeletonRoot)
        return node;

    if (!node) {
        markAllDirty();
        node = new QSSGRenderJoint();
    }

    QQuick3DNode::updateSpatialNode(node);

    auto jointNode = static_cast<QSSGRenderJoint *>(node);

    QQuick3DObjectPrivate *skeletonPriv = QQuick3DObjectPrivate::get(m_skeletonRoot);

    if (m_skeletonRootDirty) {
        if (skeletonPriv && skeletonPriv->spatialNode)
            jointNode->skeletonRoot = static_cast<QSSGRenderSkeleton *>(skeletonPriv->spatialNode);
    }

    if (m_indexDirty) {
        jointNode->index = m_index;
        m_indexDirty = false;

        if (jointNode->skeletonRoot) {
            Q_ASSERT(m_skeletonRoot);
            m_skeletonRoot->skeletonNodeDirty();

            if (jointNode->skeletonRoot->maxIndex < m_index) {
                jointNode->skeletonRoot->maxIndex = m_index;
            }
        }
    }
    return node;
}

QT_END_NAMESPACE
