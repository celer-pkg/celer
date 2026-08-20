// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dscenemanager_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dviewport_p.h"
#include "qquick3dmodel_p.h"

#include <QtQuick/QQuickWindow>

#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderroot_p.h>
#include <QtQuick3DRuntimeRender/ssg/qssgrenderextensions.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderuserpass_p.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>

#include "qquick3drenderextensions.h"

QT_BEGIN_NAMESPACE

// NOTE: Another indiraction to try to clean-up resource in a nice way.
QSSGCleanupObject::QSSGCleanupObject(std::shared_ptr<QSSGRenderContextInterface> rci, QList<QSSGRenderGraphObject *> resourceCleanupQueue, QQuickWindow *window)
    : QObject(window)
    , m_rci(rci)
    , m_window(window)
    , m_resourceCleanupQueue(resourceCleanupQueue)
{
    Q_ASSERT(window != nullptr && rci); // We need a window and a rci for this!

    if (QSGRenderContext *rc = QQuickWindowPrivate::get(window)->context) {
        connect(rc, &QSGRenderContext::releaseCachedResourcesRequested, this, &QSSGCleanupObject::cleanupResources, Qt::DirectConnection);
        connect(rc, &QSGRenderContext::invalidated, this, &QSSGCleanupObject::cleanupResources, Qt::DirectConnection);
    } else {
        qWarning() << "No QSGRenderContext, cannot cleanup resources!";
    }
}

QSSGCleanupObject::~QSSGCleanupObject()
{
    QSSG_CHECK(QSSG_DEBUG_COND(m_resourceCleanupQueue.isEmpty()));
}

void QSSGCleanupObject::cleanupResources()
{
    m_rci->renderer()->cleanupResources(m_resourceCleanupQueue);
    deleteLater();
}

static constexpr char qtQQ3DWAPropName[] { "_qtquick3dWindowAttachment" };

QQuick3DSceneManager::QQuick3DSceneManager(QObject *parent)
    : QObject(parent)
{
}

// Should be deleted by QQuick3DWindowAttachment to ensure it's done
// on the render thread.
QQuick3DSceneManager::~QQuick3DSceneManager()
{
    cleanupNodes();
    if (wattached)
        wattached->unregisterSceneManager(*this);
}

void QQuick3DSceneManager::setWindow(QQuickWindow *window)
{
    if (window == m_window)
        return;

    if (window != m_window) {
        if (wattached) {
            // Unregister from old windows attached object
            wattached->unregisterSceneManager(*this);
            wattached = nullptr;
        }
        m_window = window;
        if (m_window) {
            wattached = getOrSetWindowAttachment(*m_window);
            if (wattached)
                wattached->registerSceneManager(*this);
        }

        emit windowChanged();
    }
}

QQuickWindow *QQuick3DSceneManager::window()
{
    return m_window;
}

void QQuick3DSceneManager::dirtyItem(QQuick3DObject *item)
{
    Q_UNUSED(item);
    emit needsUpdate();
}

void QQuick3DSceneManager::requestUpdate()
{
    emit needsUpdate();
}

void QQuick3DSceneManager::cleanup(QSSGRenderGraphObject *item)
{
    cleanupNodeList.insert(item);

    if (auto front = m_nodeMap[item]) {
        auto *po = QQuick3DObjectPrivate::get(front);
        sharedResourceRemoved |= po->sharedResource;
        po->spatialNode = nullptr;

        // The front-end object is no longer reachable (destroyed) so make sure we don't return it
        // when doing a node look-up.
        m_nodeMap[item] = nullptr;
    }
}

void QQuick3DSceneManager::polishItems()
{

}

void QQuick3DSceneManager::forcePolish()
{

}

void QQuick3DSceneManager::sync()
{

}

void QQuick3DSceneManager::updateBoundingBoxes(QSSGBufferManager &mgr)
{
    mgr.setLightmapSource(lightmapSource);
    const QList<QQuick3DObject *> dirtyList = dirtyBoundingBoxList;
    for (auto object : dirtyList) {
        QQuick3DObjectPrivate *itemPriv = QQuick3DObjectPrivate::get(object);
        if (itemPriv->sceneManager == nullptr)
            continue;
        auto model = static_cast<QSSGRenderModel *>(itemPriv->spatialNode);
        if (model) {
            QSSGBounds3 bounds = mgr.getModelBounds(model);
            static_cast<QQuick3DModel *>(object)->setBounds(bounds.minimum, bounds.maximum);
        }
        dirtyBoundingBoxList.removeOne(object);
    }
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::updateDirtyResourceNodes()
{
    SyncResult ret = SyncResultFlag::None;

    auto it = std::begin(dirtyResources);
    const auto end = std::end(dirtyResources);
    for (; it != end; ++it)
        ret |= updateResources(it);

    return ret;
}

void QQuick3DSceneManager::updateDirtySpatialNodes()
{
    auto it = std::begin(dirtyNodes);
    const auto end = std::end(dirtyNodes);
    for (; it != end; ++it)
        updateNodes(it);
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::updateDiryExtensions()
{
    SyncResult ret = SyncResultFlag::None;

    auto it = std::begin(dirtyExtensions);
    const auto end = std::end(dirtyExtensions);
    for (; it != end; ++it)
        ret |= updateExtensions(it);

    return ret;
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::updateDirtyResourceSecondPass()
{
    const auto updateDirtyResourceNode = [this](QQuick3DObject *resource) {
        QQuick3DObjectPrivate *po = QQuick3DObjectPrivate::get(resource);
        po->dirtyAttributes = 0; // Not used, but we should still reset it.
        po->secondaryUpdateRequested = false;
        QSSGRenderGraphObject *node = po->spatialNode;
        po->spatialNode = resource->updateSpatialNode(node);
        if (po->spatialNode)
            m_nodeMap.insert(po->spatialNode, resource);
        return po->sharedResource ? SyncResultFlag::SharedResourcesDirty : SyncResultFlag::None;
    };

    SyncResult res = SyncResultFlag::None;
    auto it = dirtySecondPassResources.constBegin();
    const auto end = dirtySecondPassResources.constEnd();
    for (; it != end; ++it)
        res |= updateDirtyResourceNode(*it);

    // Expectation is that we won't get here often, for other updates the
    // backend nodes should have been realized and we won't get here, so
    // just release space used by the set.
    dirtySecondPassResources = {};

    return res;
}

void QQuick3DSceneManager::updateDirtyResource(QQuick3DObject *resourceObject)
{
    QQuick3DObjectPrivate *itemPriv = QQuick3DObjectPrivate::get(resourceObject);
    quint32 dirty = itemPriv->dirtyAttributes;
    Q_UNUSED(dirty);
    itemPriv->dirtyAttributes = 0;
    QSSGRenderGraphObject *oldNode = itemPriv->spatialNode;
    QSSGRenderGraphObject *newNode = resourceObject->updateSpatialNode(itemPriv->spatialNode);

    const bool backendNodeChanged = oldNode != newNode;

    // If the backend resource object changed, we need to clean up the old one.
    if (oldNode && backendNodeChanged)
        cleanup(oldNode);

    // NOTE: cleanup() will remove the item from the node map and set the spatial node to nullptr.
    //       so we need to set it here.
    itemPriv->spatialNode = newNode;

    if (itemPriv->spatialNode) {
        m_nodeMap.insert(itemPriv->spatialNode, resourceObject);
        if (itemPriv->spatialNode->type == QSSGRenderGraphObject::Type::ResourceLoader) {
            resourceLoaders.insert(itemPriv->spatialNode);
        } else if (itemPriv->spatialNode->type == QQuick3DObjectPrivate::Type::Image2D && backendNodeChanged) {
            ++inputHandlingEnabled;
        } else if (QSSGRenderGraphObjectUtils::isUserRenderPass(itemPriv->type) && itemPriv->spatialNode) {
            auto *userRenderPass = static_cast<QSSGRenderUserPass *>(itemPriv->spatialNode);
            if (const auto idx = userRenderPasses.indexOf(userRenderPass); idx == -1)
                userRenderPasses.push_back(userRenderPass);
        }
    }

    if (itemPriv->hasFlag(QQuick3DObjectPrivate::Flags::RequiresSecondaryUpdate) && itemPriv->secondaryUpdateRequested)
        dirtySecondPassResources.insert(resourceObject);

    // resource nodes dont go in the tree, so we dont need to parent them
}

void QQuick3DSceneManager::updateDirtySpatialNode(QQuick3DNode *spatialNode)
{
    const auto prepNewSpatialNode = [this](QSSGRenderNode *node) {
        if (node) {
            node->rootNodeRef = wattached->rootNode()->rootNodeRef;
            Q_ASSERT(QSSGRenderRoot::get(node->rootNodeRef) != nullptr);
        }
    };

    QQuick3DObjectPrivate *itemPriv = QQuick3DObjectPrivate::get(spatialNode);
    quint32 dirty = itemPriv->dirtyAttributes;
    itemPriv->dirtyAttributes = 0;
    QSSGRenderGraphObject *oldNode = itemPriv->spatialNode;
    QSSGRenderGraphObject *newNode = spatialNode->updateSpatialNode(oldNode);

    const bool backendNodeChanged = oldNode != newNode;

    // If the backend node changed, we need to clean up the old one.
    if (oldNode && backendNodeChanged)
        cleanup(oldNode);

    // NOTE: cleanup() will remove the item from the node map and set the spatial node to nullptr.
    //       so we need to set it here.
    itemPriv->spatialNode = newNode;
    prepNewSpatialNode(static_cast<QSSGRenderNode *>(itemPriv->spatialNode));

    // NOTE: We always update the node map, as we can end-up with the a node map where the mapping
    // has been 'disconnected', e.g., the front-end object removed from the scene only to be later
    // re-used.
    if (itemPriv->spatialNode) {
        m_nodeMap.insert(itemPriv->spatialNode, spatialNode);
        if (itemPriv->type == QQuick3DObjectPrivate::Type::Item2D && itemPriv->spatialNode != oldNode)
            ++inputHandlingEnabled;
    }

    QSSGRenderNode *graphNode = static_cast<QSSGRenderNode *>(itemPriv->spatialNode);

    if (graphNode && graphNode->parent && dirty & QQuick3DObjectPrivate::ParentChanged) {
        QQuick3DNode *nodeParent = qobject_cast<QQuick3DNode *>(spatialNode->parentItem());
        if (nodeParent) {
            QSSGRenderNode *parentGraphNode = static_cast<QSSGRenderNode *>(
                        QQuick3DObjectPrivate::get(nodeParent)->spatialNode);
            if (parentGraphNode) {
                graphNode->parent->removeChild(*graphNode);
                parentGraphNode->addChild(*graphNode);
            }
        }
    }

    if (graphNode && graphNode->parent == nullptr) {
        QQuick3DNode *nodeParent = qobject_cast<QQuick3DNode *>(spatialNode->parentItem());
        if (nodeParent) {
            QSSGRenderNode *parentGraphNode = static_cast<QSSGRenderNode *>(QQuick3DObjectPrivate::get(nodeParent)->spatialNode);
            if (!parentGraphNode) {
                // The parent spatial node hasn't been created yet
                auto parentNode = QQuick3DObjectPrivate::get(nodeParent);
                parentNode->spatialNode = nodeParent->updateSpatialNode(parentNode->spatialNode);
                if (parentNode->spatialNode)
                    m_nodeMap.insert(parentNode->spatialNode, nodeParent);
                parentGraphNode = static_cast<QSSGRenderNode *>(parentNode->spatialNode);
                prepNewSpatialNode(parentGraphNode);
            }
            if (parentGraphNode)
                parentGraphNode->addChild(*graphNode);
        } else {
            QQuick3DViewport *viewParent = qobject_cast<QQuick3DViewport *>(spatialNode->parent());
            if (viewParent) {
                auto sceneRoot = QQuick3DObjectPrivate::get(viewParent->scene());
                if (!sceneRoot->spatialNode) // must have a scene root spatial node first
                    sceneRoot->spatialNode = viewParent->scene()->updateSpatialNode(sceneRoot->spatialNode);
                if (sceneRoot->spatialNode) {
                    auto *sceneRootNode = static_cast<QSSGRenderNode *>(sceneRoot->spatialNode);
                    prepNewSpatialNode(sceneRootNode);
                    m_nodeMap.insert(sceneRootNode, viewParent->scene());
                    sceneRootNode->addChild(*graphNode);
                }
            }
        }
    }
}

QQuick3DObject *QQuick3DSceneManager::lookUpNode(const QSSGRenderGraphObject *node) const
{
    return m_nodeMap[const_cast<QSSGRenderGraphObject *>(node)];
}

QQuick3DWindowAttachment *QQuick3DSceneManager::getOrSetWindowAttachment(QQuickWindow &window)
{

    QQuick3DWindowAttachment *wa = nullptr;
    if (auto aProperty = window.property(qtQQ3DWAPropName); aProperty.isValid())
        wa = aProperty.value<QQuick3DWindowAttachment *>();

    if (!wa) {
        // WindowAttachment will not be created under 'window'.
        // It should be deleted after all the cleanups related with 'window',
        // otherwise some resourses deleted after it, will not be cleaned correctly.
        wa = new QQuick3DWindowAttachment(&window);
        window.setProperty(qtQQ3DWAPropName, QVariant::fromValue(wa));
    }

    return wa;
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::cleanupNodes()
{
    SyncResult res = sharedResourceRemoved ? SyncResultFlag::SharedResourcesDirty : SyncResultFlag::None;
    // Reset the shared resource removed value.
    sharedResourceRemoved = false;
    for (auto node : std::as_const(cleanupNodeList)) {
        // Remove "spatial" nodes from scenegraph
        if (QSSGRenderGraphObject::isNodeType(node->type)) {
            QSSGRenderNode *spatialNode = static_cast<QSSGRenderNode *>(node);
            spatialNode->removeFromGraph();
        }

        if (node->type == QQuick3DObjectPrivate::Type::Item2D) {
            --inputHandlingEnabled;
        } else if (node->type == QQuick3DObjectPrivate::Type::Image2D) {
            auto image = static_cast<QSSGRenderImage *>(node);
            if (image && image->m_qsgTexture != nullptr ) {
                --inputHandlingEnabled;
            }
        }

        // Remove all nodes from the node map because they will no
        // longer be usable from this point from the frontend
        m_nodeMap.remove(node);

        if (QSSGRenderGraphObjectUtils::hasInternalFlag(*node, QSSGRenderGraphObjectUtils::InternalFlags::AutoRegisterExtension)) {
            autoRegisteredExtensions.removeAll(node);
            autoRegisteredExtensionsDirty = true;
        }

        if (QSSGRenderGraphObjectUtils::isUserRenderPass(node->type)) {
            auto *userRenderPass = static_cast<QSSGRenderUserPass *>(node);
            if (qsizetype idx = userRenderPasses.indexOf(userRenderPass); idx != -1)
                userRenderPasses.remove(idx);
        }

        // Some nodes will trigger resource cleanups that need to
        // happen at a specified time (when graphics backend is active)
        // So build another queue for graphics assets marked for removal
        if (node->hasGraphicsResources()) {
            wattached->queueForCleanup(node);
            if (node->type == QSSGRenderGraphObject::Type::ResourceLoader)
                resourceLoaders.remove(node);
        } else {
            delete node;
        }
    }

    // Nodes are now "cleaned up" so clear the cleanup list
    cleanupNodeList.clear();

    return res;
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::updateResources(QQuick3DObject **listHead)
{
    SyncResult res = SyncResultFlag::None;

    // Detach the current list head first, and consume all reachable entries.
    // New entries may be added to the new list while traversing, which will be
    // visited on the next updateDirtyNodes() call.
    bool hasSharedResources = false;
    QQuick3DObject *updateList = *listHead;
    *listHead = nullptr;
    if (updateList)
        QQuick3DObjectPrivate::get(updateList)->prevDirtyItem = &updateList;

    QQuick3DObject *item = updateList;
    while (item) {
        // Different processing for resource nodes vs hierarchical nodes etc.
        Q_ASSERT(!QSSGRenderGraphObject::isNodeType(QQuick3DObjectPrivate::get(item)->type) || !QSSGRenderGraphObject::isExtension(QQuick3DObjectPrivate::get(item)->type));
        // handle hierarchical nodes
        updateDirtyResource(item);
        auto *po = QQuick3DObjectPrivate::get(item);
        hasSharedResources |= po->sharedResource;
        po->removeFromDirtyList();
        item = updateList;
    }

    if (hasSharedResources)
        res |= SyncResultFlag::SharedResourcesDirty;

    return res;
}

void QQuick3DSceneManager::updateNodes(QQuick3DObject **listHead)
{
    // Detach the current list head first, and consume all reachable entries.
    // New entries may be added to the new list while traversing, which will be
    // visited on the next updateDirtyNodes() call.
    QQuick3DObject *updateList = *listHead;
    *listHead = nullptr;
    if (updateList)
        QQuick3DObjectPrivate::get(updateList)->prevDirtyItem = &updateList;

    QQuick3DObject *item = updateList;
    while (item) {
        // Different processing for resource nodes vs hierarchical nodes (anything that's _not_ a resource)
        Q_ASSERT(QSSGRenderGraphObject::isNodeType(QQuick3DObjectPrivate::get(item)->type));
        // handle hierarchical nodes
        updateDirtySpatialNode(static_cast<QQuick3DNode *>(item));
        QQuick3DObjectPrivate::get(item)->removeFromDirtyList();
        item = updateList;
    }
}

QQuick3DSceneManager::SyncResult QQuick3DSceneManager::updateExtensions(QQuick3DObject **listHead)
{
    SyncResult ret = SyncResultFlag::None;

    const auto updateDirtyExtensionNode = [this, &ret](QQuick3DObject *extension) {
        QQuick3DObjectPrivate *po = QQuick3DObjectPrivate::get(extension);
        po->dirtyAttributes = 0; // Not used, but we should still reset it.
        QSSGRenderGraphObject *oldNode = po->spatialNode;
        QSSGRenderGraphObject *newNode = extension->updateSpatialNode(oldNode);

        const bool backendNodeChanged = oldNode != newNode;

        if (backendNodeChanged)
            ret |= SyncResultFlag::ExtensionsDiry;

        // If the backend node changed, we need to clean up the old one.
        if (oldNode && backendNodeChanged)
            cleanup(oldNode);

        // NOTE: cleanup() will remove the item from the node map and set the spatial node to nullptr.
        //       so we need to set it here.
        po->spatialNode = newNode;

        if (po->spatialNode)
            m_nodeMap.insert(po->spatialNode, extension);

        if (newNode && QSSGRenderGraphObjectUtils::hasInternalFlag(*newNode, QSSGRenderGraphObjectUtils::InternalFlags::AutoRegisterExtension)) {
            const bool shouldInsert = autoRegisteredExtensions.indexOf(static_cast<QSSGRenderExtension *>(newNode)) == -1;
            if (shouldInsert) {
                autoRegisteredExtensions.push_back(static_cast<QSSGRenderExtension *>(newNode));
                autoRegisteredExtensionsDirty = true;
            }
        }
    };

    // Detach the current list head first, and consume all reachable entries.
    // New entries may be added to the new list while traversing, which will be
    // visited on the next updateDirtyNodes() call.
    QQuick3DObject *updateList = *listHead;
    *listHead = nullptr;
    if (updateList)
        QQuick3DObjectPrivate::get(updateList)->prevDirtyItem = &updateList;

    QQuick3DObject *item = updateList;
    while (item) {
        // Different processing for resource nodes vs hierarchical nodes (anything that's _not_ a resource)
        Q_ASSERT(QSSGRenderGraphObject::isExtension(QQuick3DObjectPrivate::get(item)->type));
        // handle hierarchical nodes
        updateDirtyExtensionNode(item);
        QQuick3DObjectPrivate::get(item)->removeFromDirtyList();
        item = updateList;
    }

    return ret;
}

void QQuick3DSceneManager::preSync()
{
    for (auto it = std::begin(dirtyResources), end = std::end(dirtyResources); it != end; ++it) {
        QQuick3DObject *next = *it;
        while (next) {
            next->preSync();
            next = QQuick3DObjectPrivate::get(next)->nextDirtyItem;
        }
    }

    for (auto it = std::begin(dirtyNodes), end = std::end(dirtyNodes); it != end; ++it) {
        QQuick3DObject *next = *it;
        while (next) {
            next->preSync();
            next = QQuick3DObjectPrivate::get(next)->nextDirtyItem;
        }
    }

    for (auto it = std::begin(dirtyExtensions), end = std::end(dirtyExtensions); it != end; ++it) {
        QQuick3DObject *next = *it;
        while (next) {
            next->preSync();
            next = QQuick3DObjectPrivate::get(next)->nextDirtyItem;
        }
    }
}

////////
/// QQuick3DWindowAttachment
////////

QQuick3DWindowAttachment::QQuick3DWindowAttachment(QQuickWindow *window)
    : m_window(window)
    , m_rootNode(new QSSGRenderRoot)
{
    if (window) {
        // Act when the application calls window->releaseResources() and the
        // render loop emits the corresponding signal in order to forward the
        // event to us as well. (do not confuse with other release-resources
        // type of functions, this is about dropping pipeline and other resource
        // caches than can be automatically recreated if needed on the next frame)
        QQuickWindowPrivate *wd = QQuickWindowPrivate::get(window);
        QSGRenderContext *rc = wd->context;
        if (QSSG_GUARD_X(rc, "QQuickWindow has no QSGRenderContext, this should not happen")) {
            // QSGRenderContext signals are emitted on the render thread, if there is one; use DirectConnection
            connect(rc, &QSGRenderContext::releaseCachedResourcesRequested, this, &QQuick3DWindowAttachment::onReleaseCachedResources, Qt::DirectConnection);
            connect(rc, &QSGRenderContext::invalidated, this, &QQuick3DWindowAttachment::onInvalidated, Qt::DirectConnection);
        }

        // We put this in the back of the queue to allow any clean-up of resources to happen first.
        connect(window, &QQuickWindow::destroyed, this, &QObject::deleteLater);
        // afterAnimating is emitted on the main thread.
        connect(window, &QQuickWindow::afterAnimating, this, &QQuick3DWindowAttachment::preSync);
        // afterFrameEnd is emitted on render thread.
        connect(window, &QQuickWindow::afterFrameEnd, this, &QQuick3DWindowAttachment::cleanupResources, Qt::DirectConnection);
    }
}

QQuick3DWindowAttachment::~QQuick3DWindowAttachment()
{
    for (auto manager: sceneManagerCleanupQueue) {
        sceneManagers.removeOne(manager);
        delete manager;
    }
    // remaining sceneManagers should also be removed
    qDeleteAll(sceneManagers);
    QSSG_CHECK(QSSG_DEBUG_COND(resourceCleanupQueue.isEmpty()));

    if (!pendingResourceCleanupQueue.isEmpty()) {
        // Let's try to recover from this situation. Most likely this is some loader usage
        // situation, where all View3Ds have been destroyed while the window still lives and might
        // eventually just create a new View3D. We cannot release the resources here, as we'll need
        // to do that on the render thread. Instaed we'll transform the pendingResourceCleanupQueue
        // to a cleanup object that can be called on the render thread.
        if (m_rci && m_window) {
            new QSSGCleanupObject(m_rci, std::move(pendingResourceCleanupQueue), m_window);
            QMetaObject::invokeMethod(m_window, &QQuickWindow::releaseResources, Qt::QueuedConnection);
        } else {
            qWarning() << "Pending resource cleanup queue not empty, but no RCI or window to clean it up!";
        }
    }

    if (m_window)
        m_window->setProperty(qtQQ3DWAPropName, QVariant());

    delete m_rootNode;
}

void QQuick3DWindowAttachment::preSync()
{
    for (auto &sceneManager : std::as_const(sceneManagers))
        sceneManager->preSync();
}

// Called from the render thread
void QQuick3DWindowAttachment::cleanupResources()
{
    // Pass the scene managers list of resources marked for
    // removal to the render context for deletion
    // The render context will take ownership of the nodes
    // and clear the list

    // In special cases there is no rci because synchronize() is never called.
    // This can happen when running with the software backend of Qt Quick.
    // Handle this gracefully.
    if (!m_rci)
        return;

    // Check if there's orphaned resources that needs to be
    // cleaned out first.
    if (resourceCleanupQueue.size() != 0)
        m_rci->renderer()->cleanupResources(resourceCleanupQueue);
}

// Called on the render thread, if there is one
void QQuick3DWindowAttachment::onReleaseCachedResources()
{
    if (m_rci)
        m_rci->releaseCachedResources();
    Q_EMIT releaseCachedResources();
}

void QQuick3DWindowAttachment::onInvalidated()
{
    // Find all objects that have graphics resources and queue them
    // for cleanup.
    // 1. We'll need to manually go through the nodes of each scene manager and mark
    // objects with graphics resources for deletion.
    // The scene graph is invalidated so we need to release graphics resources now, on the render thread.
    for (auto &sceneManager : std::as_const(sceneManagers)) {
        const auto objects = sceneManager->m_nodeMap.keys();
        for (QSSGRenderGraphObject *obj : objects) {
            if (obj->hasGraphicsResources())
                sceneManager->cleanup(obj);
        }
    }

    // Start: follow the normal clean-up procedure
    for (auto &sceneManager : std::as_const(sceneManagers))
        sceneManager->cleanupNodes();

    for (QSSGRenderLayer *layers : std::as_const(pendingLayerCleanupQueue))
        delete layers;
    pendingLayerCleanupQueue.clear();

    for (const auto &pr : std::as_const(pendingResourceCleanupQueue))
        resourceCleanupQueue.insert(pr);
    pendingResourceCleanupQueue.clear();

    // NOTE!: It's essential that we release the cached resources before we
    // call cleanupResources(), to avoid the expensive bookkeeping code involved
    // for models. This is achieved by dropping the data to avoid expensive look-ups; it's going away anyways.
    // (In the future, if/when cleanupDrawCallData() is improved, then this step might not be necessary).
    onReleaseCachedResources();

    cleanupResources();
    // end

    // If the SG RenderContex is invalidated and we're the only one holding onto the SSG
    // RenderContextInterface then just release it. If the application is not going down
    // a new RCI will be created/set during the next sync.
    if (m_rci.use_count() == 1) {
        m_rci.reset();
        emit renderContextInterfaceChanged();
    }
}

QQuick3DWindowAttachment::SyncResult QQuick3DWindowAttachment::synchronize(QSet<QSSGRenderGraphObject *> &resourceLoaders)
{
    // Terminate old scene managers
    for (auto manager: sceneManagerCleanupQueue) {
        sceneManagers.removeOne(manager);
        delete manager;
    }
    // Terminate old scene managers
    sceneManagerCleanupQueue = {};

    SyncResult syncResult = SyncResultFlag::None;

    // Cleanup
    for (auto &sceneManager : std::as_const(sceneManagers))
        syncResult |= sceneManager->cleanupNodes();

    for (QSSGRenderLayer *layers : std::as_const(pendingLayerCleanupQueue))
        delete layers;
    pendingLayerCleanupQueue.clear();

    // Resources
    for (auto &sceneManager : std::as_const(sceneManagers))
        syncResult |= sceneManager->updateDirtyResourceNodes();
    // Spatial Nodes
    for (auto &sceneManager : std::as_const(sceneManagers))
        sceneManager->updateDirtySpatialNodes();
    for (auto &sceneManager : std::as_const(sceneManagers))
        syncResult |= sceneManager->updateDiryExtensions();
    for (auto &sceneManager : std::as_const(sceneManagers))
        syncResult |= sceneManager->updateDirtyResourceSecondPass();
    // Bounding Boxes
    for (auto &sceneManager : std::as_const(sceneManagers))
        sceneManager->updateBoundingBoxes(*m_rci->bufferManager());
    // Resource Loaders
    for (auto &sceneManager : std::as_const(sceneManagers))
        resourceLoaders.unite(sceneManager->resourceLoaders);

    if ((syncResult & SyncResultFlag::SharedResourcesDirty)) {
        // We know there are shared resources in the scene, so notify the "world".
        // Ideally we should be more targeted, but for now this will do the job.
        for (auto &sceneManager : std::as_const(sceneManagers))
            sceneManager->requestUpdate();
    }

    // Prepare pending (adopted) resources for clean-up (will happen as a result of afterFrameEnd()).
    for (const auto &pr : std::as_const(pendingResourceCleanupQueue))
        resourceCleanupQueue.insert(pr);
    pendingResourceCleanupQueue.clear();

    return syncResult;
}

void QQuick3DWindowAttachment::requestUpdate()
{
    for (const auto &sm : std::as_const(sceneManagers))
        sm->requestUpdate();
}

void QQuick3DWindowAttachment::evaluateEol()
{
    // NOTE: We'll re-iterate this list either on the next sync or if there's no sceneManagers left.
    // See: sync and dtor.
    for (QQuick3DSceneManager *manager : std::as_const(sceneManagerCleanupQueue))
        sceneManagers.removeOne(manager);

    if (sceneManagers.isEmpty())
        delete this;
}

QQuickWindow *QQuick3DWindowAttachment::window() const { return m_window; }

void QQuick3DWindowAttachment::setRci(const std::shared_ptr<QSSGRenderContextInterface> &rciptr)
{
    QSSG_CHECK_X(m_rci == nullptr || m_rci.use_count() == 1, "Old render context was not released!");
    m_rci = rciptr;

    // For convenience we'll also set the SG rendere context here.
    if (m_rci && m_window)
        QSSGRendererPrivate::setSgRenderContext(*m_rci->renderer(), QQuickWindowPrivate::get(m_window)->context);

    emit renderContextInterfaceChanged();
}

void QQuick3DWindowAttachment::registerSceneManager(QQuick3DSceneManager &manager)
{
    if (!sceneManagers.contains(&manager))
        sceneManagers.push_back(&manager);
}

void QQuick3DWindowAttachment::unregisterSceneManager(QQuick3DSceneManager &manager)
{
    sceneManagers.removeOne(&manager);
}

void QQuick3DWindowAttachment::queueForCleanup(QSSGRenderGraphObject *obj)
{
    Q_ASSERT(obj->hasGraphicsResources());
    pendingResourceCleanupQueue.push_back(obj);
}

void QQuick3DWindowAttachment::queueForCleanup(QQuick3DSceneManager *manager)
{
    if (!sceneManagerCleanupQueue.contains(manager))
        sceneManagerCleanupQueue.push_back(manager);
}

void QQuick3DWindowAttachment::queueForCleanup(QSSGRenderLayer *layer)
{
    if (QSSGRenderRoot *lr = QSSGRenderRoot::get(layer->rootNodeRef); lr != m_rootNode) {
        qWarning("Attempted to queue layer for cleanup that is not part of the current window!");
        return;
    }

    if (!pendingLayerCleanupQueue.contains(layer)) {
        layer->removeFromGraph();
        pendingLayerCleanupQueue.push_back(layer);
    }
}

QT_END_NAMESPACE
