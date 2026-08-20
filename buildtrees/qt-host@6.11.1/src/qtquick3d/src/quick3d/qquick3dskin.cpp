// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dskin_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderskin_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Skin
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Defines a skinning animation.

    A skin defines how a model can be animated using \l {Vertex Skinning}
    {skeletal animation}. It contains a list of \l {Node}s and an optional list
    of the Inverse Bind Pose Matrices.
    Each \l {Node}'s transform becomes a transform of the bone with the
    corresponding index in the list.

    \qml
    Skin {
        id: skin0
        joints: [
            node0,
            node1,
            node2
        ]
        inverseBindPoses: [
            Qt.matrix4x4(...),
            Qt.matrix4x4(...),
            Qt.matrix4x4(...)
        ]
    }
    \endqml

    \note \l {Skeleton} and \l {Joint} will be deprecated.
*/

QQuick3DSkin::QQuick3DSkin(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::Skin)), parent)
{
}

QQuick3DSkin::~QQuick3DSkin()
{
    for (const auto &conn : m_jointsConnections) {
        disconnect(conn.first);
        disconnect(conn.second);
    }
}

/*!
    \qmlproperty List<QtQuick3D::Node> Skin::joints

    This property contains a list of nodes used for a hierarchy of joints.
    The order in the list becomes the index of the joint, which is used in the
    \c SkinSemantic \l {QQuick3DGeometry::addAttribute}{custom geometry attribute}.

    \note A value 'undefined' will be ignored and if a node which doesn't exist is
    described, the result is unpredictable.

    \sa {QQuick3DGeometry::addAttribute}, {Qt Quick 3D - Simple Skinning Example}
*/
QQmlListProperty<QQuick3DNode> QQuick3DSkin::joints()
{
    return QQmlListProperty<QQuick3DNode>(this,
                                            nullptr,
                                            QQuick3DSkin::qmlAppendJoint,
                                            QQuick3DSkin::qmlJointsCount,
                                            QQuick3DSkin::qmlJointAt,
                                            QQuick3DSkin::qmlClearJoints);
}

#define POS4BONETRANS(x)    (sizeof(float) * 16 * (x) * 2)
#define POS4BONENORM(x)     (sizeof(float) * 16 * ((x) * 2 + 1))

void QQuick3DSkin::qmlAppendJoint(QQmlListProperty<QQuick3DNode> *list, QQuick3DNode *joint)
{
    if (joint == nullptr)
        return;
    QQuick3DSkin *self = static_cast<QQuick3DSkin *>(list->object);
    if (!self->m_jointsConnections.contains(joint)) {
        self->m_jointsConnections[joint] =
            std::make_pair(
                connect(
                    joint, &QQuick3DNode::sceneTransformChanged,
                    self, [self, joint]() {
                        self->m_dirtyJoints.insert(joint);
                        self->update();
                    }),
                connect(
                    joint, &QQuick3DNode::destroyed,
                    self, [self, joint]() {
                        self->m_dirtyJoints.remove(joint);
                        self->m_removedJoints.insert(joint);
                        self->update();
                    })
            );
    }

    self->m_joints.push_back(joint);
    self->m_dirtyJoints.insert(joint);
    self->update();
}

QQuick3DNode *QQuick3DSkin::qmlJointAt(QQmlListProperty<QQuick3DNode> *list, qsizetype index)
{
    QQuick3DSkin *self = static_cast<QQuick3DSkin *>(list->object);
    return self->m_joints.at(index);
}

qsizetype QQuick3DSkin::qmlJointsCount(QQmlListProperty<QQuick3DNode> *list)
{
    QQuick3DSkin *self = static_cast<QQuick3DSkin *>(list->object);
    return self->m_joints.size();
}

void QQuick3DSkin::qmlClearJoints(QQmlListProperty<QQuick3DNode> *list)
{
    QQuick3DSkin *self = static_cast<QQuick3DSkin *>(list->object);
    for (const auto &conn : self->m_jointsConnections) {
        disconnect(conn.first);
        disconnect(conn.second);
    }
    self->m_jointsConnections.clear();

    self->m_joints.clear();
    self->m_boneData.clear();
    self->m_dirtyJoints.clear();
    self->m_removedJoints.clear();
    self->update();
}


/*!
    \qmlproperty List<matrix4x4> Skin::inverseBindPoses

    This property contains a list of Inverse Bind Pose matrixes used for the
    skinning animation. Each inverseBindPose matrix means the inverse of the
    global transform of the corresponding node in \l {Skin::joints},
    used initially.

    \note This property is an optional property. That is, if some or all of the
    matrices are not set, identity values will be used.
*/
QList<QMatrix4x4> QQuick3DSkin::inverseBindPoses() const
{
    return m_inverseBindPoses;
}

void QQuick3DSkin::setInverseBindPoses(const QList<QMatrix4x4> &poses)
{
    if (m_inverseBindPoses == poses)
        return;

    m_updatedByNewInverseBindPoses = qMax(poses.size(), m_inverseBindPoses.size());
    m_inverseBindPoses = poses;

    emit inverseBindPosesChanged();
    update();
}

QSSGRenderGraphObject *QQuick3DSkin::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderSkin();
    }
    QQuick3DObject::updateSpatialNode(node);
    auto skinNode = static_cast<QSSGRenderSkin *>(node);

    if (!m_removedJoints.empty()) {
        for (int i = m_joints.size() - 1; i >= 0; --i) {
            const auto &joint = m_joints.at(i);
            if (m_removedJoints.contains(joint)) {
                m_joints.removeAt(i);
                m_boneData.remove(POS4BONETRANS(i),
                                  sizeof(float) * 16 * 2);
            }
        }
        m_removedJoints.clear();
    }

    if (skinNode->boneCount != quint32(m_joints.size())) {
        skinNode->boneCount = quint32(m_joints.size());
        const int boneTexWidth = qCeil(qSqrt(skinNode->boneCount * 4 * 2));
        const int textureSizeInBytes = boneTexWidth * boneTexWidth * 16;  //NB: Assumes RGBA32F set above (16 bytes per color)
        m_boneData.resize(textureSizeInBytes);
        skinNode->setSize(QSize(boneTexWidth, boneTexWidth));
    }

    if (m_updatedByNewInverseBindPoses > 0 || !m_dirtyJoints.empty()) {
        for (int i = 0; i < m_joints.size(); ++i) {
            const auto &joint = m_joints.at(i);
            if (i < m_updatedByNewInverseBindPoses || m_dirtyJoints.contains(joint)) {
                QMatrix4x4 jointGlobal = joint->sceneTransform();
                if (m_inverseBindPoses.size() > i)
                    jointGlobal *= m_inverseBindPoses.at(i);
                memcpy(m_boneData.data() + POS4BONETRANS(i),
                       reinterpret_cast<const void *>(jointGlobal.constData()),
                       sizeof(float) * 16);
                memcpy(m_boneData.data() + POS4BONENORM(i),
                       reinterpret_cast<const void *>(QMatrix4x4(jointGlobal.normalMatrix()).constData()),
                       sizeof(float) * 11);
            }
        }
        m_dirtyJoints.clear();
        m_updatedByNewInverseBindPoses = 0;
    }

    skinNode->setTextureData(m_boneData);
    return node;
}

QT_END_NAMESPACE
