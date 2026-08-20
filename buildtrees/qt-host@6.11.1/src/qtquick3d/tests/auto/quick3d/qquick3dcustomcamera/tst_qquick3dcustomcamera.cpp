// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick3D/private/qquick3dcustomcamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

class tst_QQuick3DCustomCamera : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Camera : public QQuick3DCustomCamera
    {
    public:
        using QQuick3DCamera::updateSpatialNode;
    };

private slots:
    void testCustomProjection();
};

void tst_QQuick3DCustomCamera::testCustomProjection()
{
    Camera camera;
    auto node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    // clang-format off
    const QMatrix4x4 customProjection = {
        2, 0, 0, 0,
        0, 4, 0, 0,
        0, 0, 2, 0,
        0, 0, 0, 1,
    };
    // clang-format on
    camera.setProjection(customProjection);
    node = static_cast<QSSGRenderCamera *>(camera.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(node->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));
    QCOMPARE(customProjection, node->projection);
}

QTEST_APPLESS_MAIN(tst_QQuick3DCustomCamera)
#include "tst_qquick3dcustomcamera.moc"
