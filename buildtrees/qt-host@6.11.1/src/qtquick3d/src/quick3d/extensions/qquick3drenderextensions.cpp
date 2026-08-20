// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderextensions.h"

#include <QtQuick3D/private/qquick3dobject_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QQuick3DRenderExtension
    \inmodule QtQuick3D
    \since 6.7

    \brief Abstract class for implementing user side render extensions.

    This is the front-end side of a render extension. The back-end side is implemented
    in \l QSSGRenderExtension. The QQuick3DRenderExtension class is used to create a custom
    render extension that can be used in the QtQuick3D scene graph by adding it to the list of
    extensions to be used with a \l View3D. The extension code will then be run as part of
    QtQuick3D's rendering pipeline execution.

    The QQuick3DRenderExtension class is an abstract class that should be subclassed and
    exposed to QML. The subclass should implement the \l QQuick3DRenderExtension::updateSpatialNode()
    function and return a QSSGRenderExtension instance that contains the code that should be run.

    \sa QSSGRenderExtension
*/

/*!
    \qmltype RenderExtension
    \nativetype QQuick3DRenderExtension
    \inqmlmodule QtQuick3D
    \inherits Object3D
    \since 6.7
    \brief An uncreatable abstract base type for render extensions.

    \sa QQuick3DRenderExtension, QSSGRenderExtension, View3D::extensions()
*/

QQuick3DRenderExtension::QQuick3DRenderExtension(QQuick3DObjectPrivate &dd, QQuick3DObject *parent)
    : QQuick3DObject(dd, parent)
{

}

QQuick3DRenderExtension::QQuick3DRenderExtension(QQuick3DObject *parent)
    : QQuick3DRenderExtension(*new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::RenderExtension), parent)
{

}

QQuick3DRenderExtension::~QQuick3DRenderExtension()
{

}

/*!
    \fn QSSGRenderGraphObject *QQuick3DRenderExtension::updateSpatialNode(QSSGRenderGraphObject *node)

    This function is called during the synchronization of the QtQuick3D scene graph when an item is
    created or when an update is requested, usually as a result of a change in the item's properties.
    The function should return a QSSGRenderExtension instance that contains the code that should be
    run during QtQuick3D's rendering pipeline execution.

    The \a node parameter is the previous QSSGRenderExtension instance that was returned from this
    function, or null if this is the first time the function is called. The function can return the
    same instance, a different instance, or null. If the function returns null, the extension will
    be removed from the rendering pipeline.

    \note The QSSGRenderExtension instance is a resource object and will be owned by the QtQuick3D
          scene graph. If a different instance, or null, is returned, the previous instance will be
          queued for deletion by the renderer.

    \sa QSSGRenderExtension
*/

QSSGRenderGraphObject *QQuick3DRenderExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    return QQuick3DObject::updateSpatialNode(node);
}





QT_END_NAMESPACE
