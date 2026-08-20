// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dextensionhelpers.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>

QT_BEGIN_NAMESPACE

/*!
    \typedef QSSGNodeId
    \relates QtQuick3D

    The QSSGNodeId is a handle to a QtQuick3D node object. \l Node objects are all
    objects that inherits from \l Node, like the \l Model item.
*/

/*!
    \typedef QSSGResourceId
    \relates QtQuick3D

    The QSSGResourceId is a handle to a QtQuick3D object. Resources are usually all
    \l {Node}{none-node} types.
*/

/*!
    \typedef QSSGCameraId
    \relates QtQuick3D

    The QSSGCameraId is a handle to a QtQuick3D \l Camera object, like the \l PerspectiveCamera item.

    \note Cameras are also nodes.
*/

/*!
    \typedef QSSGExtensionId
    \relates QtQuick3D

    The QSSGExtensionId is a handle to a QtQuick3D extension type, like items that inherits from the
    \l RenderExtension type.

    \sa QQuick3DRenderExtension, QSSGRenderExtension
*/

/*!
    \class QQuick3DExtensionHelpers
    \inmodule QtQuick3D
    \since 6.6

    \brief Helper functions for the Extensions APIs.
*/

QQuick3DExtensionHelpers::QQuick3DExtensionHelpers()
{

}

/*!
    \return a \c QSSGNodeId that can be used to retrieve the object in the engine
    corresponding to \a node.

    //! \sa QSSGFrameData::getNode()
*/
QSSGNodeId QQuick3DExtensionHelpers::getNodeId(const QQuick3DObject &node)
{
    auto *po = QQuick3DObjectPrivate::get(&node);
    QSSG_ASSERT_X(QSSGRenderGraphObject::isNodeType(po->type), "Type is not a node", return QSSGNodeId::Invalid);
    // NOTE: Implementation detail (don't rely on this in user code).
    return static_cast<QSSGNodeId>(quintptr(QQuick3DObjectPrivate::get(&node)->spatialNode));
}

/*!
    \return a \c QSSGResourceId that can be used to retrieve the corresponding \a resource object
    in the engine.

    //! \sa QSSGFrameData::getResource()
*/
QSSGResourceId QQuick3DExtensionHelpers::getResourceId(const QQuick3DObject &resource)
{
    auto *po = QQuick3DObjectPrivate::get(&resource);
    QSSG_ASSERT_X(QSSGRenderGraphObject::isResource(po->type), "Type is not a resource", return QSSGResourceId::Invalid);
    // NOTE: Implementation detail (don't rely on this in user code).
    return static_cast<QSSGResourceId>(quintptr(QQuick3DObjectPrivate::get(&resource)->spatialNode));
}

/*!
    \return a \c QSSGCameraId that can be used to retrieve the corresponding \a camera object
    in the engine.

    //! \sa QSSGFrameData::getNode()
*/
QSSGCameraId QQuick3DExtensionHelpers::getCameraId(const QQuick3DObject &camera)
{
    auto *po = QQuick3DObjectPrivate::get(&camera);
    QSSG_ASSERT_X(QSSGRenderGraphObject::isCamera(po->type), "Type is not a camera", return QSSGCameraId::Invalid);
    // NOTE: Implementation detail (don't rely on this in user code).
    return static_cast<QSSGCameraId>(quintptr(po->spatialNode));
}

QSSGExtensionId QQuick3DExtensionHelpers::getExtensionId(const QQuick3DObject &extension)
{
    auto *po = QQuick3DObjectPrivate::get(&extension);
    QSSG_ASSERT_X(QSSGRenderGraphObject::isExtension(po->type), "Type is not an extension", return QSSGExtensionId::Invalid);
    // NOTE: Implementation detail (don't rely on this in user code).
    return static_cast<QSSGExtensionId>(quintptr(po->spatialNode));
}


/*!
    \return the type of the node with the given \a nodeId.

    This function is useful to determine the type of a \a nodeId.

    \sa QSSGRenderGraphObject::Type
*/
QSSGRenderGraphObject::Type QQuick3DExtensionHelpers::getNodeIdType(QSSGNodeId nodeId)
{
    auto *node = QSSGRenderGraphObjectUtils::getNode(nodeId);
    return node ? node->type : QSSGRenderGraphObject::Type::Unknown;
}

QT_END_NAMESPACE
