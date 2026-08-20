// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3D/qquick3dobject.h>
#include <QtQuick3D/private/qquick3ditem2d_p.h>

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qsgrenderer_p.h>
#include <QtQuick/private/qquickwindow_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderitem2d_p.h>
#include "qquick3dnode_p_p.h"

#include <QtGui/rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*
internal
*/

QQuick3DItem2D::QQuick3DItem2D(QQuickItem *item, QQuick3DNode *parent)
    : QQuick3DNode(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::Item2D)), parent)
{
    m_contentItem = new QQuickItem();
    m_contentItem->setObjectName(QLatin1String("parent of ") + item->objectName()); // for debugging
    // No size is set for m_contentItem. This is intentional, otherwise item2d anchoring breaks.
    QQuickItemPrivate::get(m_contentItem)->ensureSubsceneDeliveryAgent();
    QQmlEngine::setObjectOwnership(m_contentItem, QQmlEngine::CppOwnership);

    connect(m_contentItem, &QQuickItem::childrenChanged, this, &QQuick3DObject::update);
    addChildItem(item);
}

QQuick3DItem2D::~QQuick3DItem2D()
{
    delete m_contentItem;
}

void QQuick3DItem2D::addChildItem(QQuickItem *item)
{
    item->setParent(m_contentItem);
    item->setParentItem(m_contentItem);
    QQuickItemPrivate::get(item)->addItemChangeListener(this, QQuickItemPrivate::ChangeType::Destroyed);
    connect(item, &QQuickItem::enabledChanged, this, &QQuick3DItem2D::updatePicking);
    connect(item, &QQuickItem::visibleChanged, this, &QQuick3DItem2D::updatePicking);
    m_sourceItems.append(item);
    update();
}
void QQuick3DItem2D::removeChildItem(QQuickItem *item)
{
    m_sourceItems.removeOne(item);
    if (item)
        QQuickItemPrivate::get(item)->removeItemChangeListener(this, QQuickItemPrivate::ChangeType::Destroyed);
    if (m_sourceItems.isEmpty())
        emit allChildrenRemoved();
    else
        update();
}

QQuickItem *QQuick3DItem2D::contentItem() const
{
    return m_contentItem;
}

void QQuick3DItem2D::itemDestroyed(QQuickItem *item)
{
    removeChildItem(item);
}

void QQuick3DItem2D::updatePicking()
{
    m_pickingDirty = true;
    update();
}

QSSGRenderGraphObject *QQuick3DItem2D::updateSpatialNode(QSSGRenderGraphObject *node)
{
    auto *sourceItemPrivate = QQuickItemPrivate::get(m_contentItem);
    QQuickWindow *window = m_contentItem->window();

    if (!window) {
        const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager;
        window = manager->window();
    }

    if (!node) {
        markAllDirty();
        auto *backEndNode = new QSSGRenderItem2D();
        backEndNode->m_frontEndObject = this;
        node = backEndNode;
    }

    QQuick3DNode::updateSpatialNode(node);

    auto itemNode = static_cast<QSSGRenderItem2D *>(node);

    itemNode->m_rootNode = sourceItemPrivate->rootNode();
    if (!itemNode->m_rootNode) {
        QQuickWindowPrivate::get(window)->updateDirtyNode(m_contentItem);
        itemNode->m_rootNode = sourceItemPrivate->rootNode();
        if (!itemNode->m_rootNode) {
            qWarning() << "Item2D is not initialized. It will not be shown.";
            return nullptr;
        }
    }

    if (m_pickingDirty) {
        m_pickingDirty = false;
        bool isPickable = false;
        for (const auto &item : std::as_const(m_sourceItems)) {
            // Enable picking for Item2D if any of its child is visible and enabled.
            if (item->isVisible() && item->isEnabled()) {
                isPickable = true;
                break;
            }
        }
        itemNode->setState(QSSGRenderNode::LocalState::Pickable, isPickable);
    }

    return node;
}

void QQuick3DItem2D::markAllDirty()
{
    m_pickingDirty = true;

    QQuick3DNode::markAllDirty();
}

void QQuick3DItem2D::preSync()
{
    const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager;
    auto *sourcePrivate = QQuickItemPrivate::get(m_contentItem);
    auto *window = manager->window();
    if (m_window != window) {
        update(); // Just schedule an upate immediately.
        if (m_window) {
            disconnect(m_window, SIGNAL(destroyed(QObject*)), this, SLOT(derefWindow(QObject*)));
            sourcePrivate->derefWindow();
        }
        m_window = window;
        sourcePrivate->refWindow(window);
        connect(window, SIGNAL(destroyed(QObject*)), this, SLOT(derefWindow(QObject*)));
        sourcePrivate->refFromEffectItem(true);
    }
}

static void detachWindow(QQuickItem *item, QObject *win)
{
    auto *itemPriv = QQuickItemPrivate::get(item);

    if (win == itemPriv->window) {
        itemPriv->window = nullptr;
        itemPriv->windowRefCount = 0;

        itemPriv->prevDirtyItem = nullptr;
        itemPriv->nextDirtyItem = nullptr;
    }

    for (auto *child: itemPriv->childItems)
        detachWindow(child, win);
}

void QQuick3DItem2D::derefWindow(QObject *win)
{
    detachWindow(m_contentItem, win);
}

QT_END_NAMESPACE
