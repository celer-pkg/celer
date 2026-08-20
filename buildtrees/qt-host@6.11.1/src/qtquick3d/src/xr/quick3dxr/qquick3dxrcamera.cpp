// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrcamera_p.h"
#include "qquick3dxrorigin_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3D/private/qquick3dutils_p.h>
#include <QtQuick3D/private/qquick3dnode_p_p.h>

#include <cmath>

QT_BEGIN_NAMESPACE
/*!
    \qmltype XrCamera
    \inherits Node
    \inqmlmodule QtQuick3D.Xr
    \brief Tracks spatial position and orientation from which the user views an XR scene.

    The XrCamera is a tracked spatial node that tracks the spatial position and orientation
    of the users's view of an XR scene.

    Since this is a tracked node, its spatial properties should be considered read-only.
    Properties specific to the XrCamera, such as the \l{clipNear}{near} and \l{clipFar}{far} clip planes, are settable.
    When set, they override preferred values.


    \sa XrOrigin::camera
*/

QQuick3DXrEyeCamera::QQuick3DXrEyeCamera(QQuick3DXrOrigin *parent)
    : QQuick3DCamera(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::CustomCamera, QQuick3DContentLayer::LayerAll)), parent)
{
}

float QQuick3DXrEyeCamera::leftTangent() const
{
    return m_leftTangent;
}

float QQuick3DXrEyeCamera::rightTangent() const
{
    return m_rightTangent;
}

float QQuick3DXrEyeCamera::upTangent() const
{
    return m_upTangent;
}

float QQuick3DXrEyeCamera::downTangent() const
{
    return m_downTangent;
}

float QQuick3DXrEyeCamera::clipNear() const
{
    return m_clipNear;
}

float QQuick3DXrEyeCamera::clipFar() const
{
    return m_clipFar;
}

void QQuick3DXrEyeCamera::setLeftTangent(float leftTangent)
{
    if (qFuzzyCompare(m_leftTangent, leftTangent))
        return;

    m_leftTangent = leftTangent;
    emit leftTangentChanged(m_leftTangent);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setRightTangent(float rightTangent)
{
    if (qFuzzyCompare(m_rightTangent, rightTangent))
        return;

    m_rightTangent = rightTangent;
    emit rightTangentChanged(m_rightTangent);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setUpTangent(float upTangent)
{
    if (qFuzzyCompare(m_upTangent, upTangent))
        return;

    m_upTangent = upTangent;
    emit upTangentChanged(m_upTangent);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setDownTangent(float downTangent)
{
    if (qFuzzyCompare(m_downTangent, downTangent))
        return;

    m_downTangent = downTangent;
    emit downTangentChanged(m_downTangent);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setClipNear(float clipNear)
{
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;
    emit clipNearChanged(m_clipNear);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setClipFar(float clipFar)
{
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;
    emit clipFarChanged(m_clipFar);
    markDirty(DirtyFlag::ProjectionDirty);
}

void QQuick3DXrEyeCamera::setProjection(const QMatrix4x4 &projection)
{
    m_projection = projection;
    markDirty(DirtyFlag::ProjectionChanged);
}

QSSGRenderGraphObject *QQuick3DXrEyeCamera::updateSpatialNode(QSSGRenderGraphObject *node)
{
    QSSGRenderCamera *camera = static_cast<QSSGRenderCamera *>(QQuick3DCamera::updateSpatialNode(node));
    if (camera) {
        // Projection changed takes precedence over projection dirty
        bool changed = false;
        if (m_dirtyFlags & DirtyFlag::ProjectionChanged) {
            camera->projection = m_projection;
            qUpdateIfNeeded(camera->clipPlanes, { m_clipNear, m_clipFar });
            m_dirtyFlags &= ~DirtyFlag::ProjectionChanged;
            changed = true;
        } else if (m_dirtyFlags & DirtyFlag::ProjectionDirty) {
            maybeUpdateProjection();
            changed |= qUpdateIfNeeded(camera->projection, m_projection);
            qUpdateIfNeeded(camera->clipPlanes, { m_clipNear, m_clipFar });
            m_dirtyFlags &= ~DirtyFlag::ProjectionDirty;
        }

        if (m_dirtyFlags & DirtyFlag::ClipChanged) {
            qUpdateIfNeeded(camera->clipPlanes, { m_clipNear, m_clipFar });
            m_dirtyFlags &= ~DirtyFlag::ClipChanged;
        }

        // Reset the dirty flag after all updates have been done
        m_dirtyFlags = 0;

        if (changed)
            camera->markDirty(QSSGRenderCamera::DirtyFlag::CameraDirty);
    }
    return camera;
}

void QQuick3DXrEyeCamera::markDirty(DirtyFlag flag)
{
    if (m_dirtyFlags & flag)
        return;

    m_dirtyFlags |= flag;
    update();
}

void QQuick3DXrEyeCamera::maybeUpdateProjection()
{
    QSSG_ASSERT(m_dirtyFlags & DirtyFlag::ProjectionDirty, return);

    const float right = m_rightTangent * m_clipNear;
    const float top = m_upTangent * m_clipNear;
#if defined(Q_OS_VISIONOS)
    // cp_view_get_tangents always returns positive values
    // so we need to negate the left and down tangents
    const float left = -m_leftTangent * m_clipNear;
    const float bottom = -m_downTangent * m_clipNear;
#else
    const float left = m_leftTangent * m_clipNear;
    const float bottom = m_downTangent * m_clipNear;
#endif

    float *m = m_projection.data();

    m[0] = 2 * m_clipNear / (right - left);
    m[4] = 0;
    m[8] = (right + left) / (right - left);
    m[12] = 0;

    m[1] = 0;
    m[5] = 2 * m_clipNear / (top - bottom);
    m[9] = (top + bottom) / (top - bottom);
    m[13] = 0;

    m[2] = 0;
    m[6] = 0;
    m[10] = m_clipFar / (m_clipNear - m_clipFar);
    m[14] = m_clipFar * m_clipNear / (m_clipNear - m_clipFar);


    m[3] = 0;
    m[7] = 0;
    m[11] = -1;
    m[15] = 0;

    const bool isReverseZ = false; // placeholder
    if (isReverseZ) {
        if (std::isinf(m_clipFar)) {
            m[10] = 0;
            m[14] = m_clipNear;
        } else {
            m[10] = -m[10] - 1;
            m[14] = -m[14];
        }
    } else if (std::isinf(m_clipFar)) {
        m[10] = -1;
        m[14] = -m_clipNear;
    }
}

QQuick3DXrCamera::QQuick3DXrCamera(QQuick3DXrOrigin *parent)
    : QQuick3DNode(parent)
{
}

QQuick3DXrCamera::~QQuick3DXrCamera()
{
}

/*!
    \qmlproperty real QtQuick3D.Xr::XrCamera::clipNear
    \brief The start of the distance range, with reference to the camera position, in which objects will appear.

    \note Unless set explicitly, the clipNear value will be set to the device's preferred value.
*/

float QQuick3DXrCamera::clipNear() const
{
    return m_clipNear;
}

/*!
    \qmlproperty real QtQuick3D.Xr::XrCamera::clipFar
    \brief The end of the distance range, with reference to the camera position, in which objects will appear.

    \note Unless set explicitly, the clipFar value will be set to the device's preferred value.
*/

float QQuick3DXrCamera::clipFar() const
{
    return m_clipFar;
}

void QQuick3DXrCamera::setClipNear(float clipNear)
{
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;

    syncCameraSettings();

    emit clipNearChanged(m_clipNear);
}

void QQuick3DXrCamera::setClipFar(float clipFar)
{
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;

    syncCameraSettings();

    emit clipFarChanged(m_clipFar);
}

void QQuick3DXrCamera::itemChange(ItemChange change, const ItemChangeData &data)
{
    // Sanity check (If we get a tracked item we'll do this there instead).
    if (change == QQuick3DObject::ItemParentHasChanged) {
        if (data.item != nullptr) {
            if (QQuick3DXrOrigin *xrOrigin = qobject_cast<QQuick3DXrOrigin *>(data.item)) {
                xrOrigin->setCamera(this);
            } else {
                qWarning() << "XrCamera must be a child of an XrOrigin!";
                setParentItem(nullptr);
            }
        } else {
            QQuick3DNode::itemChange(change, data);
        }
    }
}

void QQuick3DXrCamera::syncCameraSettings()
{
    QQuick3DXrOrigin *xrOrigin = qobject_cast<QQuick3DXrOrigin *>(parentItem());
    if (xrOrigin && xrOrigin->camera() == this)
        xrOrigin->syncCameraSettings();
}

QT_END_NAMESPACE
