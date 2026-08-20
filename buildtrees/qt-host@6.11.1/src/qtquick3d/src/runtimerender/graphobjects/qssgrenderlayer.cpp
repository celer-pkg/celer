// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include "../rendererimpl/qssglayerrenderdata_p.h"
#include "qssgrenderroot_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

void QSSGRenderLayer::markDirty(DirtyFlag dirtyFlag)
{
    m_layerDirtyFlags |= FlagT(dirtyFlag);
    QSSGRenderNode::markDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

void QSSGRenderLayer::clearDirty(DirtyFlag dirtyFlag)
{
    m_layerDirtyFlags &= ~FlagT(dirtyFlag);
    QSSGRenderNode::clearDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

QSSGRenderLayer::QSSGRenderLayer()
    : QSSGRenderNode(QSSGRenderNode::Type::Layer)
    , firstEffect(nullptr)
    , antialiasingMode(QSSGRenderLayer::AAMode::NoAA)
    , antialiasingQuality(QSSGRenderLayer::AAQuality::High)
    , background(QSSGRenderLayer::Background::Transparent)
    , temporalAAStrength(0.3f)
    , ssaaMultiplier(1.5f)
    , specularAAEnabled(false)
    , oitMethod(OITMethod::None)
    , oitMethodDirty(false)
    , tonemapMode(TonemapMode::Linear)
{
    flags = { FlagT(LocalState::Active) | FlagT(GlobalState::Active) }; // The layer node is alway active and not dirty.
}

QSSGRenderLayer::~QSSGRenderLayer()
{
    rootNode = nullptr;
    rootNodeRef = nullptr;

    delete importSceneNode;
    importSceneNode = nullptr;

    delete renderData;
    renderData = nullptr;
}

void QSSGRenderLayer::setProbeOrientation(const QVector3D &angles)
{
    if (angles != lightProbeSettings.probeOrientationAngles) {
        lightProbeSettings.probeOrientationAngles = angles;
        lightProbeSettings.probeOrientation = QQuaternion::fromEulerAngles(lightProbeSettings.probeOrientationAngles).toRotationMatrix();
    }
}

void QSSGRenderLayer::addEffect(QSSGRenderEffect &inEffect)
{
    // Effects need to be rendered in reverse order as described in the file.
    inEffect.m_nextEffect = firstEffect;
    firstEffect = &inEffect;
}

bool QSSGRenderLayer::hasEffect(QSSGRenderEffect *inEffect) const
{
    for (auto currentEffect = firstEffect; currentEffect != nullptr; currentEffect = currentEffect->m_nextEffect) {
        if (currentEffect == inEffect)
            return true;
    }
    return false;
}

void QSSGRenderLayer::setImportScene(QSSGRenderNode &importedNode)
{
    if (importedNode.parent != nullptr && !QSSGRenderGraphObjectUtils::isSceneRoot(importedNode.parent->type)) {
        qWarning("The root of the imported scene node is already part of another scene graph.\n"
                 "Importing a sub-scene is unsupported and may not work as expected!");
    }

    // We create a dummy node to represent the imported scene tree, as we
    // do absolutely not want to change the node links in that tree!
    if (importSceneNode == nullptr) {
        importSceneNode = new QSSGRenderNode(QSSGRenderGraphObject::Type::ImportScene);
        // Now we can add the dummy node to the layers child list
        // NOTE: We push front because this way we'll always index the imported scene first...
        children.push_front(*importSceneNode);
    } else {
        importSceneNode->children.clear(); // Clear the list (or the list will modify the rootNode)
    }

    // The imported scene root node is now a child of the dummy node
    auto &importChildren = importSceneNode->children;
    Q_ASSERT(importChildren.isEmpty());
    // We don't want the list to modify our node, so we set the tail and head manually.
    importChildren.m_head = importChildren.m_tail = &importedNode;

    // Mark all nodes as imported, this allows us to detect imported nodes later on.
    importedNode.setState(QSSGRenderNode::LocalState::Imported);

    // Now try to detect if the imported scene is used in a different window. This is not
    // supported and might not function correctly, so we warn the user and try to be nice.
    const bool warnAboutCrossWindowSharing = importedNode.rootNodeRef && QSSGRenderRoot::get(importedNode.rootNodeRef) != rootNode;
    if (warnAboutCrossWindowSharing) {
        qWarning("Sharing nodes across different windows is not supported and may lead to unexpected behavior!");
        // NOTE: If there are multiple windows sharing the same imported scene, then we don't, and don't want
        //       to, keep track of that, so we mark the whole imported scene as dirty to force every window to
        //       update their data for the imported scene.
        importSceneNode->markDirty(QSSGRenderNode::DirtyFlag::StickyDirty);
    }
}

void QSSGRenderLayer::removeImportScene(QSSGRenderNode &rootNode)
{
    if (importSceneNode && !importSceneNode->children.isEmpty()) {
        if (&importSceneNode->children.back() == &rootNode)
            importSceneNode->children.clear();
    }
}

QT_END_NAMESPACE
