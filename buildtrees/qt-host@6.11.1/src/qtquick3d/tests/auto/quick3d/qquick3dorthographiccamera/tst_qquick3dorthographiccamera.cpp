// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick3D/private/qquick3dorthographiccamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

class tst_QQuick3DOrthographicCamera : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Camera : public QQuick3DOrthographicCamera
    {
    public:
        using QQuick3DCamera::updateSpatialNode;
    };

private slots:
    void testClip();
};

void tst_QQuick3DOrthographicCamera::testClip()
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
}

QTEST_APPLESS_MAIN(tst_QQuick3DOrthographicCamera)
#include "tst_qquick3dorthographiccamera.moc"
