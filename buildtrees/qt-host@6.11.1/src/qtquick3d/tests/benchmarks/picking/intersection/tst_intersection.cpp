// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DRuntimeRender/private/qssgrenderray_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

class intersection : public QObject
{
    Q_OBJECT

public:
    intersection() = default;
    ~intersection() = default;

private slots:
    void bench_aabbIntersectionv2();
};

void intersection::bench_aabbIntersectionv2()
{
    QMatrix4x4 globalTransform; // Identity
    // Bounding box of size 1, placed at origin
    QSSGBounds3 bounds { /*min=*/ QVector3D{ -0.5f, -0.5f, -0.5f }, /*max=*/ QVector3D{ 0.5f, 0.5f, 0.5f } };

    // Pick ray goes directly down the z-axis and should hit directly in the middle of the box
    const auto pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});

    QSSGRenderRay::RayData data = QSSGRenderRay::createRayData(globalTransform, pickRay);

    const auto hit = QSSGRenderRay::intersectWithAABBv2(data, bounds);
    QCOMPARE(hit.intersects(), true);

    QBENCHMARK {
        QSSGRenderRay::intersectWithAABBv2(data, bounds);
    }
}

QTEST_APPLESS_MAIN(intersection)

#include "tst_intersection.moc"
