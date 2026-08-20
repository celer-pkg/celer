// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include "qssgrendernode_p.h"

#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qssgrendermodel_p.h"
#include "qssgrenderroot_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>

#include <QtQuick3DUtils/private/qssgplane_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderNode::QSSGRenderNode()
    : QSSGRenderNode(Type::Node)
{
}

QSSGRenderNode::QSSGRenderNode(Type type, QSSGRenderGraphObject::FlagT flags)
    : QSSGRenderGraphObject(type, flags)
{
    localTransform = calculateTransformMatrix({}, initScale, {}, {});
}

QSSGRenderNode::~QSSGRenderNode()
    = default;

void QSSGRenderNode::markDirty(DirtyFlag dirtyFlag)
{
    if ((flags & FlagT(dirtyFlag)) == 0) { // If not already marked
        flags |= FlagT(dirtyFlag);
        const bool markSubtreeDirty = ((FlagT(dirtyFlag) & FlagT(DirtyFlag::SubtreeUpdateMask)) != 0);
        if (markSubtreeDirty) {
            for (auto &child : children)
                child.markDirty(dirtyFlag);
        }
    }
}

void QSSGRenderNode::clearDirty(DirtyFlag dirtyFlag)
{
    flags &= ~FlagT(dirtyFlag);
}

void QSSGRenderNode::setState(LocalState state, bool on)
{
    const bool changed = (getLocalState(state) != on);
    if (changed) { // Mark state dirty
        flags = on ? (flags | FlagT(state)) : (flags & ~FlagT(state));

        // Mark state dirty
        switch (state) {
        case QSSGRenderNode::LocalState::Active:
            markDirty(DirtyFlag::ActiveDirty);
            if (QSSGRenderRoot *rootNode = QSSGRenderRoot::get(rootNodeRef))
                rootNode->markDirty(QSSGRenderRoot::DirtyFlag::TreeDirty);
            break;
        case QSSGRenderNode::LocalState::Pickable:
            markDirty(DirtyFlag::PickableDirty);
            break;
        case QSSGRenderNode::LocalState::Imported:
            markDirty(DirtyFlag::ImportDirty);
            break;
        }
    }

}

QMatrix4x4 QSSGRenderNode::calculateTransformMatrix(QVector3D position, QVector3D scale, QVector3D pivot, QQuaternion rotation)
{
    QMatrix4x4 transform;

    // Offset the origin (this is our pivot point)
    auto offset = (-pivot * scale);

    // Scale
    transform(0, 0) = scale[0];
    transform(1, 1) = scale[1];
    transform(2, 2) = scale[2];

    // Offset (before rotation)
    transform(0, 3) = offset[0];
    transform(1, 3) = offset[1];
    transform(2, 3) = offset[2];

    // rotate
    transform = QMatrix4x4{rotation.toRotationMatrix()} * transform;

    // translate
    transform(0, 3) += position[0];
    transform(1, 3) += position[1];
    transform(2, 3) += position[2];

    return transform;
}

void QSSGRenderNode::addChild(QSSGRenderNode &inChild)
{
    QSSG_ASSERT_X(inChild.parent != this, "Already a child of this node!", return);

    // Adding children to a layer does not reset parent
    // because layers can share children over with other layers
    if (type != QSSGRenderNode::Type::Layer && type != QSSGRenderNode::Type::ImportScene) {
        if (inChild.parent && inChild.parent != this)
            inChild.parent->removeChild(inChild);
        inChild.parent = this;
    }

    QSSG_ASSERT_X(type != QSSGRenderNode::Type::Root || inChild.type == QSSGRenderNode::Type::Layer,
                  "Only layers can be added to the root!", return);

    if (inChild.type != QSSGRenderNode::Type::Root)
        inChild.rootNodeRef = rootNodeRef;

    children.push_back(inChild);
    // Don't mark dirty if this is a layer, because layers won't have any global values
    // calculated and therefore will not have the dirty flag cleared!
    if (inChild.type != QSSGRenderNode::Type::Layer)
        inChild.markDirty(DirtyFlag::GlobalValuesDirty);
    if (QSSGRenderRoot *rootNode = QSSGRenderRoot::get(rootNodeRef))
        rootNode->markDirty(QSSGRenderRoot::DirtyFlag::TreeDirty);
}

void QSSGRenderNode::removeChild(QSSGRenderNode &inChild)
{
    if (Q_UNLIKELY(type != QSSGRenderNode::Type::Layer && inChild.parent != this)) {
        Q_ASSERT(inChild.parent == this);
        return;
    }

    inChild.parent = nullptr;
    if (inChild.type != QSSGRenderNode::Type::Root)
        inChild.rootNodeRef = nullptr;
    inChild.h = {};
    children.remove(inChild);
    if (QSSGRenderRoot *rootNode = QSSGRenderRoot::get(rootNodeRef))
        rootNode->markDirty(QSSGRenderRoot::DirtyFlag::TreeDirty);
    inChild.markDirty(DirtyFlag::GlobalValuesDirty);
}

void QSSGRenderNode::removeFromGraph()
{
    if (parent)
        parent->removeChild(*this);

    // Orphan all of my children.
    for (auto it = children.begin(), end = children.end(); it != end;) {
        auto &removedChild = *it++;
        children.remove(removedChild);
        removedChild.parent = nullptr;
    }
}

QSSGBounds3 QSSGRenderNode::getBounds(QSSGBufferManager &inManager,
                                      bool inIncludeChildren) const
{
    QSSGBounds3 retval;
    if (inIncludeChildren)
        retval = getChildBounds(inManager);

    if (type == QSSGRenderGraphObject::Type::Model) {
        auto model = static_cast<const QSSGRenderModel *>(this);
        retval.include(inManager.getModelBounds(model));
    }
    return retval;
}

QSSGBounds3 QSSGRenderNode::getChildBounds(QSSGBufferManager &inManager) const
{
    QSSGBounds3 retval;
    QSSGBounds3 childBounds;
    for (auto &child : children) {
        childBounds = child.getBounds(inManager);
        if (!childBounds.isEmpty()) {
            // Transform the bounds into our local space.
            childBounds.transform(child.localTransform);
            retval.include(childBounds);
        }
    }
    return retval;
}

QVector3D QSSGRenderNode::getDirection(const QMatrix4x4 &globalTransform)
{
    const float *dataPtr(globalTransform.data());
    QVector3D retval(dataPtr[8], dataPtr[9], dataPtr[10]);
    retval.normalize();
    return retval;
}

QVector3D QSSGRenderNode::getScalingCorrectDirection(const QMatrix4x4 &globalTransform)
{
    QMatrix3x3 theDirMatrix = globalTransform.normalMatrix();
    QVector3D theOriginalDir(0, 0, -1);
    QVector3D retval = QSSGUtils::mat33::transform(theDirMatrix, theOriginalDir);
    // Should already be normalized, but whatever
    retval.normalize();
    return retval;
}

void QSSGRenderNode::calculateMVP(const QMatrix4x4 &globalTransform, const QMatrix4x4 &inViewProjection, QMatrix4x4 &outMVP)
{
    outMVP = inViewProjection * globalTransform;
}

void QSSGRenderNode::calculateNormalMatrix(const QMatrix4x4 &globalTransform, QMatrix3x3 &outNormalMatrix)
{
    outNormalMatrix = globalTransform.normalMatrix();
}

void QSSGRenderNode::calculateMVPAndNormalMatrix(const QMatrix4x4 &globalTransform,
                                                 const QMatrix4x4 &inViewProjection,
                                                 QMatrix4x4 &outMVP,
                                                 QMatrix3x3 &outNormalMatrix)
{
    calculateMVP(globalTransform, inViewProjection, outMVP);
    calculateNormalMatrix(globalTransform, outNormalMatrix);
}

QT_END_NAMESPACE
