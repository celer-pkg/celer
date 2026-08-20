// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dorthographiccamera_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

#include <QtMath>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qquick3dutils_p.h"

#include "qquick3dnode_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype OrthographicCamera
    \inherits Camera
    \inqmlmodule QtQuick3D
    \brief Defines an Camera with an orthographic projection matrix.

    A \l Camera defines how the content of the 3D scene is projected onto a 2D surface,
    such as a View3D. A scene needs at least one \l Camera in order to visualize its
    contents.

    It is possible to position and rotate the \l Camera like any other spatial \l{QtQuick3D::Node}{Node} in
    the scene. The \l{QtQuick3D::Node}{Node}'s location and orientation determines where the \l Camera is in
    the scene, and what direction it is facing. The default orientation of the \l Camera
    has its forward vector pointing along the negative Z axis and its up vector along
    the positive Y axis.

    \image orthographiccamera.png {Orthographic camera projection diagram}

    The OrthographicCamera is a parallel projection \l Camera, in which parallel lines remain
    parallel and an object's perceived scale is unaffected by its distance from the \l Camera.
    Typical use cases for this type of \l Camera are CAD (Computer-Assisted Design) applications
    and cartography.

    The following example creates a OrthographicCamera at position [0, 200, 300] in the scene, and
    with a 30 degree downward pitch.
    \code
    OrthographicCamera {
        position: Qt.vector3d(0, 200, 300)
        eulerRotation.x: -30
    }
    \endcode

    \sa {Qt Quick 3D - View3D Example}, PerspectiveCamera, FrustumCamera, CustomCamera
*/

/*!
 * \internal
 */
QQuick3DOrthographicCamera::QQuick3DOrthographicCamera(QQuick3DNode *parent)
    : QQuick3DCamera(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::OrthographicCamera)), parent) {}

/*!
    \qmlproperty real OrthographicCamera::clipNear

    This property defines the near clip plane of the OrthographicCamera's frustum. Geometry which
    is closer to the \l Camera than the near clip plane will not be visible.

    The default value is 10.0.

    \sa clipFar
 */
float QQuick3DOrthographicCamera::clipNear() const
{
    return m_clipNear;
}

/*!
    \qmlproperty real OrthographicCamera::clipFar

    This property defines the far clip plane of the OrthographicCamera's frustum. Geometry which
    is further away from the \l Camera than the far clip plane will not be visible.

    The default value is 10000.0.

    \sa clipNear
 */
float QQuick3DOrthographicCamera::clipFar() const
{
    return m_clipFar;
}

/*!
    \qmlproperty real OrthographicCamera::horizontalMagnification

    This property holds the horizontal magnification of the OrthographicCamera's frustum.

    The default value is 1.0.

    \sa verticalMagnification
 */
float QQuick3DOrthographicCamera::horizontalMagnification() const
{
    return m_horizontalMagnification;
}

/*!
    \qmlproperty real OrthographicCamera::verticalMagnification

    This property holds the vertical magnification of the OrthographicCamera's frustum.

    The default value is 1.0.

    \sa horizontalMagnification
 */
float QQuick3DOrthographicCamera::verticalMagnification() const
{
    return m_verticalMagnification;
}

void QQuick3DOrthographicCamera::setClipNear(float clipNear)
{
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;
    emit clipNearChanged();
    update();
}

void QQuick3DOrthographicCamera::setClipFar(float clipFar)
{
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;
    emit clipFarChanged();
    update();
}

void QQuick3DOrthographicCamera::setHorizontalMagnification(float horizontalMagnification)
{
    if (horizontalMagnification <= 0.0) {
        qWarning("OrthographicCamera: magnification must be greater than zero.");
        return;
    }

    if (qFuzzyCompare(m_horizontalMagnification, horizontalMagnification))
        return;

    m_horizontalMagnification = horizontalMagnification;
    emit horizontalMagnificationChanged();
    update();
}

void QQuick3DOrthographicCamera::setVerticalMagnification(float verticalMagnification)
{
    if (verticalMagnification <= 0.0) {
        qWarning("OrthographicCamera: magnification must be greater than zero.");
        return;
    }

    if (qFuzzyCompare(m_verticalMagnification, verticalMagnification))
        return;

    m_verticalMagnification = verticalMagnification;
    emit verticalMagnificationChanged();
    update();
}

QSSGRenderGraphObject *QQuick3DOrthographicCamera::updateSpatialNode(QSSGRenderGraphObject *node)
{
    QSSGRenderCamera *camera = static_cast<QSSGRenderCamera *>(QQuick3DCamera::updateSpatialNode(node));
    if (camera) {
        const bool changed = ((int(qUpdateIfNeeded(camera->clipPlanes, { m_clipNear, m_clipFar }))
                               | int(qUpdateIfNeeded(camera->magnification, { m_horizontalMagnification, m_verticalMagnification }))) != 0);
        if (changed)
            camera->markDirty(QSSGRenderCamera::DirtyFlag::CameraDirty);
    }

    return camera;
}

QT_END_NAMESPACE
