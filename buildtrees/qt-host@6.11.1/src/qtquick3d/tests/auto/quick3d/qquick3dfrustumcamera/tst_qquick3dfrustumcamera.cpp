// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick3D/private/qquick3dfrustumcamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

class tst_QQuick3DFrustumCamera : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Camera : public QQuick3DFrustumCamera
    {
    public:
        using QQuick3DCamera::updateSpatialNode;
    };

private slots:
    void testClipAndFov();
    void testFrustum();
};

void tst_QQuick3DFrustumCamera::testClipAndFov()
{
    Camera camera;
    auto node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    const float clipNear = 0.2f;
    camera.setClipNear(clipNear);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QCOMPARE(clipNear, node->clipPlanes.clipNear());

    const float clipFar = 0.4f;
    camera.setClipFar(clipFar);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QCOMPARE(clipFar, node->clipPlanes.clipFar());

    const float fov = 6.2f;
    camera.setFieldOfView(fov);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QCOMPARE(fov, node->fov.degrees()); // We use degrees in the front-end API

    const QQuick3DPerspectiveCamera::FieldOfViewOrientation fovOrientation
            = QQuick3DPerspectiveCamera::FieldOfViewOrientation::Horizontal;
    camera.setFieldOfViewOrientation(fovOrientation);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QVERIFY(node->fov.orientation() == QSSGRenderCamera::FieldOfView::Orientation::Horizontal);
    camera.setFieldOfViewOrientation(QQuick3DPerspectiveCamera::FieldOfViewOrientation::Vertical);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QVERIFY(node->fov.orientation() == QSSGRenderCamera::FieldOfView::Orientation::Vertical);
}

void tst_QQuick3DFrustumCamera::testFrustum()
{
    Camera camera;
    auto node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    const float frustumBottom = 0.54f;
    camera.setBottom(frustumBottom);
    const float frustumTop = 0.242f;
    camera.setTop(frustumTop);
    const float frustumLeft = 0.74f;
    camera.setLeft(frustumLeft);
    const float frustumRight = 1.0f;
    camera.setRight(frustumRight);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QCOMPARE(frustumBottom, node->customFrustum.bottom);
    QCOMPARE(frustumTop, node->customFrustum.top);
    QCOMPARE(frustumLeft, node->customFrustum.left);
    QCOMPARE(frustumRight, node->customFrustum.right);
}

QTEST_APPLESS_MAIN(tst_QQuick3DFrustumCamera)
#include "tst_qquick3dfrustumcamera.moc"
