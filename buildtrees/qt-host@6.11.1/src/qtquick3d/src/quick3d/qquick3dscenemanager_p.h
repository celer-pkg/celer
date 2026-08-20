// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSCENEMANAGER_P_H
#define QSSGSCENEMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QObject>
#include <QtCore/QSet>

#include <QtQuick3D/private/qtquick3dglobal_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>

#include "qquick3dnode_p.h"

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QSGDynamicTexture;
class QQuickWindow;
class QSSGBufferManager;
class QSSGRenderContextInterface;
class QSSGRenderRoot;
struct QSSGRenderLayer;
class QSSGRenderExtension;
class QSSGRenderUserPass;

class Q_QUICK3D_EXPORT QQuick3DWindowAttachment : public QObject
{
    Q_OBJECT
public:
    enum SyncResultFlag : quint32
    {
        None,
        SharedResourcesDirty = 0x1,
        ExtensionsDiry = 0x2,
    };

    using SyncResult = std::underlying_type_t<SyncResultFlag>;

    explicit QQuick3DWindowAttachment(QQuickWindow *window);
    ~QQuick3DWindowAttachment() override;

    Q_INVOKABLE void preSync();
    Q_INVOKABLE void cleanupResources();
    Q_INVOKABLE SyncResult synchronize(QSet<QSSGRenderGraphObject *> &resourceLoaders);
    Q_INVOKABLE void requestUpdate();
    Q_INVOKABLE void evaluateEol();

    QQuickWindow *window() const;

    QSSGRenderRoot *rootNode() const { return m_rootNode; }

    const std::shared_ptr<QSSGRenderContextInterface> &rci() const { return m_rci; }
    void setRci(const std::shared_ptr<QSSGRenderContextInterface> &rciptr);

    void registerSceneManager(QQuick3DSceneManager &manager);
    void unregisterSceneManager(QQuick3DSceneManager &manager);

    void queueForCleanup(QSSGRenderGraphObject *obj);
    void queueForCleanup(QQuick3DSceneManager *manager);
    void queueForCleanup(QSSGRenderLayer *layer);

Q_SIGNALS:
    void releaseCachedResources();
    void renderContextInterfaceChanged();

private:
    Q_INVOKABLE void onReleaseCachedResources();
    Q_INVOKABLE void onInvalidated();

    QPointer<QQuickWindow> m_window;
    QSSGRenderRoot *m_rootNode = nullptr;
    std::shared_ptr<QSSGRenderContextInterface> m_rci;
    QList<QQuick3DSceneManager *> sceneManagers;
    QList<QQuick3DSceneManager *> sceneManagerCleanupQueue;
    QList<QSSGRenderGraphObject *> pendingResourceCleanupQueue;
    QSet<QSSGRenderGraphObject *> resourceCleanupQueue;
    QList<QSSGRenderLayer *> pendingLayerCleanupQueue;
};

class Q_QUICK3D_EXPORT QQuick3DSceneManager : public QObject
{
    Q_OBJECT
public:
    using SyncResultFlag = QQuick3DWindowAttachment::SyncResultFlag;
    using SyncResult = QQuick3DWindowAttachment::SyncResult;

    explicit QQuick3DSceneManager(QObject *parent = nullptr);
    ~QQuick3DSceneManager() override;

    void setWindow(QQuickWindow *window);
    QQuickWindow *window();

    void dirtyItem(QQuick3DObject *item);
    void requestUpdate();
    void cleanup(QSSGRenderGraphObject *item);

    void polishItems();
    void forcePolish();
    void sync();
    void preSync();

    SyncResult cleanupNodes();
    SyncResult updateDirtyResourceNodes();
    void updateDirtySpatialNodes();
    SyncResult updateDiryExtensions();
    SyncResult updateDirtyResourceSecondPass();

    void updateDirtyResource(QQuick3DObject *resourceObject);
    void updateDirtySpatialNode(QQuick3DNode *spatialNode);
    void updateBoundingBoxes(QSSGBufferManager &mgr);

    QQuick3DObject *lookUpNode(const QSSGRenderGraphObject *node) const;

    // Where the enumerator is placed will decide the priority it gets.
    // NOTE: Place new list types before 'Count'.
    // NOTE: InstanceNodes are nodes that have an instance root set, we'll process these
    // after the other nodes but before light nodes; this implies that lights are not good candidates
    // for being instance roots...
    enum class NodePriority { Skeleton, Other, ModelWithInstanceRoot, Lights, Count };
    enum class ResourcePriority { TextureData, Texture, Other, Count };
    enum class ExtensionPriority { RenderExtension, Count };

    static inline size_t resourceListIndex(QSSGRenderGraphObject::Type type)
    {
        Q_ASSERT(!QSSGRenderGraphObject::isNodeType(type));

        if (QSSGRenderGraphObject::isTexture(type))
            return size_t(ResourcePriority::Texture);

        if (type == QSSGRenderGraphObject::Type::TextureData)
            return size_t(ResourcePriority::TextureData);

        return size_t(ResourcePriority::Other);
    }

    static inline size_t nodeListIndex(QSSGRenderGraphObject::Type type)
    {
        Q_ASSERT(QSSGRenderGraphObject::isNodeType(type));

        if (QSSGRenderGraphObject::isLight(type))
            return size_t(NodePriority::Lights);

        if (type == QSSGRenderGraphObject::Type::Skeleton)
            return size_t(NodePriority::Skeleton);

        return size_t(NodePriority::Other);
    }

    static constexpr size_t extensionListIndex(QSSGRenderGraphObject::Type type)
    {
        Q_ASSERT(QSSGRenderGraphObject::isExtension(type));

        return size_t(ExtensionPriority::RenderExtension);
    }

    static QQuick3DWindowAttachment *getOrSetWindowAttachment(QQuickWindow &window);

    QQuick3DObject *dirtyResources[size_t(ResourcePriority::Count)] {};
    QQuick3DObject *dirtyNodes[size_t(NodePriority::Count)] {};
    QQuick3DObject *dirtyExtensions[size_t(ExtensionPriority::Count)] {};
    // For exceptions to the norm we create a list of resources that
    // we get a second update.
    // In the case of the render extensions the resources are update first and for the
    // first time the extensions have not been run and therefore have no backend node, which
    // we'll need to use connect the render result from the extension with the texture.
    QSet<QQuick3DObject *> dirtySecondPassResources;

    QList<QQuick3DObject *> dirtyBoundingBoxList;
    QSet<QSSGRenderGraphObject *> cleanupNodeList;
    QList<QSSGRenderGraphObject *> resourceCleanupQueue;

    QSet<QQuick3DObject *> parentlessItems;
    QVector<QSGDynamicTexture *> qsgDynamicTextures;
    QHash<QSSGRenderGraphObject *, QQuick3DObject *> m_nodeMap;
    QSet<QSSGRenderGraphObject *> resourceLoaders;
    QList<QSSGRenderExtension *> autoRegisteredExtensions;
    QList<QSSGRenderUserPass *> userRenderPasses;
    QQuickWindow *m_window = nullptr;
    QPointer<QQuick3DWindowAttachment> wattached;
    int inputHandlingEnabled = 0; // Holds the count of active item2Ds, input disabled if zero.
    bool sharedResourceRemoved = false;
    bool autoRegisteredExtensionsDirty = false;
    friend QQuick3DObject;
    QString lightmapSource;

Q_SIGNALS:
    void needsUpdate();
    void windowChanged();

private Q_SLOTS:
    SyncResult updateResources(QQuick3DObject **listHead);
    void updateNodes(QQuick3DObject **listHead);
    SyncResult updateExtensions(QQuick3DObject **listHead);
};

class QSSGCleanupObject : public QObject
{
    Q_OBJECT
public:
    QSSGCleanupObject(std::shared_ptr<QSSGRenderContextInterface> rci,
                      QList<QSSGRenderGraphObject *> resourceCleanupQueue,
                      QQuickWindow *window);

    ~QSSGCleanupObject() override;

    Q_INVOKABLE void cleanupResources();

private:
    std::shared_ptr<QSSGRenderContextInterface> m_rci;
    QPointer<QQuickWindow> m_window;
    QList<QSSGRenderGraphObject *> m_resourceCleanupQueue;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuick3DSceneManager)

#endif // QSSGSCENEMANAGER_P_H
