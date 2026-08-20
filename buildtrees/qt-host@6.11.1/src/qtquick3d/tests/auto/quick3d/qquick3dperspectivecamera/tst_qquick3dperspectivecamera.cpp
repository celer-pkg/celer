// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtGui/qquaternion.h>

#include <limits>

class tst_QQuick3DPerspectiveCamera : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Camera : public QQuick3DPerspectiveCamera
    {
    public:
        using QQuick3DCamera::updateSpatialNode;
    };

    class NodeItem : public QQuick3DNode
    {
    public:
        using QQuick3DNode::updateSpatialNode;
    };

private slots:
    void testClipAndFov();
    void testLookAt();
    void mapToViewport();
};

void tst_QQuick3DPerspectiveCamera::testClipAndFov()
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
    QCOMPARE(fov, node->fov.degrees()); // fov is in degrees in the front-end API.

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

void tst_QQuick3DPerspectiveCamera::testLookAt()
{
    Camera cam1;
    Camera cam2;
    NodeItem nodeItem;

    nodeItem.setPosition(QVector3D(200.f, 0.f, 0.f));
    cam1.setPosition(QVector3D(100.f, 0.f, 100.f));
    cam2.setPosition(QVector3D(100.f, 0.f, 100.f));

    auto node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(nullptr));
    auto camNode1 = static_cast<QSSGRenderNode *>(cam1.updateSpatialNode(nullptr));
    auto camNode2 = static_cast<QSSGRenderNode *>(cam2.updateSpatialNode(nullptr));

    QVERIFY(node);
    QVERIFY(camNode1);
    QVERIFY(camNode2);

    // Verify that using lookAt method and lookAtNode property result in same rotation
    cam1.setLookAtNode(&nodeItem);
    cam2.lookAt(&nodeItem);

    camNode1 = static_cast<QSSGRenderNode *>(cam1.updateSpatialNode(camNode1));
    camNode2 = static_cast<QSSGRenderNode *>(cam2.updateSpatialNode(camNode2));

    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam1.eulerRotation().y(), -45.f));
    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().z()));
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam2.eulerRotation().y(), -45.f));
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().z()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode1->localTransform)), cam1.rotation()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode2->localTransform)), cam2.rotation()));

    // Verify that rotation automatically changes when moving camera and using lookAtNode property
    cam1.setPosition(QVector3D(300.f, 0.f, 100.f));
    cam2.setPosition(QVector3D(300.f, 0.f, 100.f));

    camNode1 = static_cast<QSSGRenderNode *>(cam1.updateSpatialNode(camNode1));
    camNode2 = static_cast<QSSGRenderNode *>(cam2.updateSpatialNode(camNode2));

    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam1.eulerRotation().y(), 45.f)); // rotation updates
    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().z()));
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam2.eulerRotation().y(), -45.f)); // rotation doesn't update
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().z()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode1->localTransform)), cam1.rotation()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode2->localTransform)), cam2.rotation()));

    // Verify that rotation automatically changs when moving the target node
    nodeItem.setPosition(QVector3D(200.f, 0.f, 100.f));

    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    camNode1 = static_cast<QSSGRenderNode *>(cam1.updateSpatialNode(camNode1));
    camNode2 = static_cast<QSSGRenderNode *>(cam2.updateSpatialNode(camNode2));

    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam1.eulerRotation().y(), 90.f)); // rotation updates
    QVERIFY(qFuzzyIsNull(cam1.eulerRotation().z()));
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().x()));
    QVERIFY(qFuzzyCompare(cam2.eulerRotation().y(), -45.f)); // rotation doesn't update
    QVERIFY(qFuzzyIsNull(cam2.eulerRotation().z()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode1->localTransform)), cam1.rotation()));
    QVERIFY(qFuzzyCompare(QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(camNode2->localTransform)), cam2.rotation()));
}

void tst_QQuick3DPerspectiveCamera::mapToViewport()
{
    Camera camera;
    auto node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(nullptr));
    QVERIFY(node);

    // QTBUG-100832
    const float floatMax = std::numeric_limits<float>::max();
    const QVector3D largeValue(-floatMax, floatMax, floatMax);
    QVector3D result = camera.mapToViewport(largeValue);
    QVERIFY(result.isNull());
}

QTEST_APPLESS_MAIN(tst_QQuick3DPerspectiveCamera)
#include "tst_qquick3dperspectivecamera.moc"
