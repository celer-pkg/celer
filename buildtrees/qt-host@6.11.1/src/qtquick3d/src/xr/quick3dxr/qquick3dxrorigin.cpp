// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrorigin_p.h"

#include "qquick3dxrview_p.h"

#include <QtQuick3D/private/qquick3dnode_p_p.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrOrigin
    \inherits Node
    \inqmlmodule QtQuick3D.Xr
    \brief The origin location for the XrView.

    The origin determines the center of the \l{XrView::referenceSpace}{reference space}.
    Changing the position of the origin will transport the user to a different position
    in the scene.

    Most XR platforms allow the user to move physically, changing the viewpoint. This
    does not change the position of the origin. To track the position of the headset, create
    an XrCamera and assign it to the \l camera property.

    \warning Ensure you follow best practices when changing the position/rotation of
    the origin. Not doing so may cause physical discomfort, nausea, or loss of balance.
    In the worst case, this could lead to injury or even death.
*/

QQuick3DXrOrigin::QQuick3DXrOrigin(QQuick3DNode *parent)
    : QQuick3DNode(parent)
{
    // These are the "real" cameras that are used for rendering.
    QQuick3DXrEyeCamera *leftEyeCamera = new QQuick3DXrEyeCamera(this);
    leftEyeCamera->setParentItem(this);

    QQuick3DXrEyeCamera *rightEyeCamera = new QQuick3DXrEyeCamera(this);
    rightEyeCamera->setParentItem(this);

    m_eyeCameras = { leftEyeCamera, rightEyeCamera };
}

QQuick3DXrOrigin::~QQuick3DXrOrigin()
{

}

/*!
    \qmlproperty XrCamera QtQuick3D.Xr::XrOrigin::camera
    \brief Property for adding a tracked camera node.

    \default null

    Holds an XrCamera, which is a tracked spatial node that tracks the position and orientation
    of the head-mounted display in the XR environment.

    \note This property is optional.

    \sa XrCamera
*/

QQuick3DXrCamera *QQuick3DXrOrigin::camera() const
{
    return m_camera;
}

void QQuick3DXrOrigin::setCamera(QQuick3DXrCamera *newCamera)
{
    if (m_camera == newCamera)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DXrOrigin::setCamera, m_camera, newCamera);

    m_camera = newCamera;

    if (m_camera) {
        // Ensure that the parent item is the XrOrigin
        QQuick3DObject *camParentItem = m_camera->parentItem();
        if (camParentItem != this) {
            m_camera->setParentItem(this);
            if (camParentItem != nullptr)
                qWarning() << "XrCamera needs to be a child of XrOrigin. Reparenting...";
        }

        // If there's a camera, it will call this function to update the camera settings
        // when its properties change.
        syncCameraSettings();
    } else {
        // Restore default values
        resetCameraSettings();
    }

    emit cameraChanged();
}

QQuick3DXrEyeCamera *QQuick3DXrOrigin::eyeCamera(int index) const
{
    return m_eyeCameras[index];
}

void QQuick3DXrOrigin::syncCameraSettings()
{
    QSSG_ASSERT(m_camera != nullptr, return);

    for (auto eyeCamera : m_eyeCameras) {
        eyeCamera->setClipNear(m_camera->clipNear());
        eyeCamera->setClipFar(m_camera->clipFar());
    }
}

void QQuick3DXrOrigin::resetCameraSettings()
{
    Q_ASSERT(!m_camera);

    if (QQuick3DXrView *xrView = qobject_cast<QQuick3DXrView *>(parentItem())) {
        // Use the default clip distances from the XrManager
        float nearClip, farClip;
        xrView->xrManager()->getDefaultClipDistances(nearClip, farClip);
        for (auto eyeCamera : m_eyeCameras) {
            eyeCamera->setClipNear(nearClip);
            eyeCamera->setClipFar(farClip);
        }
    }
}

void QQuick3DXrOrigin::updateTrackedCamera(const QMatrix4x4 &transform)
{
    if (m_camera)
        QQuick3DNodePrivate::get(m_camera)->setLocalTransform(transform);
}

void QQuick3DXrOrigin::updateTrackedCamera(QVector3D position, QQuaternion rotation)
{
    if (m_camera) {
        m_camera->setPosition(position);
        m_camera->setRotation(rotation);
    }
}

QT_END_NAMESPACE
