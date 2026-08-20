// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcustomcamera_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

#include <QtMath>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qquick3dnode_p_p.h"

#include "qquick3dutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CustomCamera
    \inherits Camera
    \inqmlmodule QtQuick3D
    \brief Defines a Camera with a custom projection matrix.

    A \l Camera defines how the content of the 3D scene is projected onto a 2D surface,
    such as a View3D. A scene needs at least one \l Camera in order to visualize its
    contents.

    It is possible to position and rotate the \l Camera like any other spatial \l{QtQuick3D::Node}{Node} in
    the scene. The \l{QtQuick3D::Node}{Node}'s location and orientation determines where the \l Camera is in
    the scene, and what direction it is facing. The default orientation of the \l Camera
    has its forward vector pointing along the negative Z axis and its up vector along
    the positive Y axis.

    The CustomCamera type provides a \l Camera where the projection matrix can be customized
    freely.

    The following example creates a CustomCamera at position [0, 200, 300] in the scene, with
    a 30 degree downward pitch, and a custom projection matrix based on custom near and far plane
    distances, and a custom field of view.
    \code
    CustomCamera {
        position: Qt.vector3d(0, 200, 300)
        eulerRotation.x: -30

        property real near: 10.0
        property real far: 10000.0
        property real fov: 60.0 * Math.PI / 180.0
        projection: Qt.matrix4x4(Math.cos(fov / 2) / Math.sin(fov / 2) * (window.height / window.width), 0, 0, 0,
                                 0, Math.cos(fov / 2) / Math.sin(fov / 2), 0, 0,
                                 0, 0, -(near + far) / (far - near), -(2.0 * near * far) / (far - near),
                                 0, 0, -1, 0);
    }
    \endcode

    \note When using CustomCamera, some anti-aliasing modes(Temporal AA and Progressive AA) cannot be applied correctly.

    \sa PerspectiveCamera, OrthographicCamera, FrustumCamera
*/

/*!
 * \internal
 */
QQuick3DCustomCamera::QQuick3DCustomCamera(QQuick3DNode *parent)
    : QQuick3DCamera(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::CustomCamera, QQuick3DContentLayer::LayerAll)), parent){}

/*!
    \qmlproperty matrix4x4 CustomCamera::projection

    This property defines the CustomCamera's projection matrix.
*/
QMatrix4x4 QQuick3DCustomCamera::projection() const
{
    return m_projection;
}

void QQuick3DCustomCamera::setProjection(const QMatrix4x4 &projection)
{
    if (m_projection == projection)
        return;

    m_projection = projection;
    emit projectionChanged();
    update();
}

/*!
 * \internal
 */
QSSGRenderGraphObject *QQuick3DCustomCamera::updateSpatialNode(QSSGRenderGraphObject *node)
{
    QSSGRenderCamera *camera = static_cast<QSSGRenderCamera *>(QQuick3DCamera::updateSpatialNode(node));
    if (camera) {
        if (qUpdateIfNeeded(camera->projection, m_projection))
            camera->markDirty(QSSGRenderCamera::DirtyFlag::CameraDirty);
    }

    return camera;
}

QT_END_NAMESPACE
