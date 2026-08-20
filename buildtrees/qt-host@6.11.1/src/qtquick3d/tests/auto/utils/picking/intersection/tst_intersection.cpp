// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DRuntimeRender/private/qssgrenderray_p.h>

class intersection : public QObject
{
    Q_OBJECT

public:
    intersection() = default;
    ~intersection() = default;

private slots:
    void test_aabbIntersectionv2();
    void test_aabbIntersectionScaledv2();
    void test_aabbIntersectionTranslatedv2();
    void test_aabbIntersectionRotatedv2();

private:
    static QSSGRenderRay::IntersectionResult intersectWithAABBv2_proxy(const QMatrix4x4 &inGlobalTransform,
                                                                       const QSSGBounds3 &inBounds,
                                                                       const QSSGRenderRay &ray)
    {
        QSSGRenderRay::RayData data = QSSGRenderRay::createRayData(inGlobalTransform, ray);
        const auto hit = QSSGRenderRay::intersectWithAABBv2(data, inBounds);
        return hit.intersects() ? QSSGRenderRay::createIntersectionResult(data, hit)
                                : QSSGRenderRay::IntersectionResult();
    }
    using IntersectionFunc = std::function<QSSGRenderRay::IntersectionResult(const QMatrix4x4 &, const QSSGBounds3 &, const QSSGRenderRay &)>;

    void aabbIntersection(const IntersectionFunc &intersectFunc);
    void aabbIntersectionScaled(const IntersectionFunc &intersectFunc);
    void aabbIntersectionTranslated(const IntersectionFunc &intersectFunc);
    void aabbIntersectionRotated(const IntersectionFunc &intersectFunc);
};

void intersection::test_aabbIntersectionv2()
{
    aabbIntersection(&intersection::intersectWithAABBv2_proxy);
}

void intersection::test_aabbIntersectionScaledv2()
{
    aabbIntersectionScaled(&intersection::intersectWithAABBv2_proxy);
}

void intersection::test_aabbIntersectionTranslatedv2()
{
    aabbIntersectionTranslated(&intersection::intersectWithAABBv2_proxy);
}

void intersection::test_aabbIntersectionRotatedv2()
{
    aabbIntersectionRotated(&intersection::intersectWithAABBv2_proxy);
}

void intersection::aabbIntersection(const IntersectionFunc &intersectFunc)
{
    QSSGRenderRay::IntersectionResult res;
    QMatrix4x4 globalTransform; // Identity
    // Bounding box of size 1, placed at origin
    QSSGBounds3 bounds { /*min=*/ QVector3D{ -0.5f, -0.5f, -0.5f }, /*max=*/ QVector3D{ 0.5f, 0.5f, 0.5f } };

    // Pick ray goes directly down the z-axis and should hit directly in the middle of the box
    auto pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
    res = intersectFunc(globalTransform, bounds, pickRay);
    QCOMPARE(res.intersects, true);

    { // Test against a flat bounding volume
        QSSGBounds3 boundsFlat { /*min=*/ QVector3D{ -1.0f, 0.0f, -1.0f }, /*max=*/ QVector3D{ 1.0f, 0.0f, 1.0f } };
        res = intersectFunc(globalTransform, boundsFlat, QSSGRenderRay(/*Origin=*/{0.0f, 10.0f, 0.0f}, /*Direction=*/{0.0f, -1.0f, 0.0f}));
        QCOMPARE(res.intersects, true);
    }

    { // horizontal scan
        const int steps = 10;
        const float stepping = (bounds.maximum.x() - bounds.minimum.x()) / float(steps);
        float rayX = bounds.minimum.x() - stepping;
        const float rayY = (bounds.maximum.y() - bounds.minimum.y()) / 2.0f;

        // First pick should fail, as it's just outside the bound's x extents
        QVERIFY(rayX < bounds.maximum.x());
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        // scan through the bounding box along the x-axis
        rayX = bounds.minimum.x();
        for (int i = 0; i != steps; ++i, rayX += stepping) {
            pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
            res = intersectFunc(globalTransform, bounds, pickRay);
            QCOMPARE(res.intersects, true);
        }

        // Do one more step to go outside the box and test that we don't hit.
        rayX += stepping;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);
    }

    { // vertical scan
        const int steps = 10;
        const float stepping = (bounds.maximum.y() - bounds.minimum.y()) / float(steps);
        float rayY = bounds.minimum.y() - stepping;
        const float rayX = (bounds.maximum.x() - bounds.minimum.x()) / 2.0f;

        // First pick should fail, as it's just outside the bound's y extents
        QVERIFY(rayY < bounds.maximum.y());
        pickRay = QSSGRenderRay(/*Origin=*/{rayY, rayX, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        // scan through the bounding box along the y-axis
        rayY = bounds.minimum.y();
        for (int i = 0; i != steps; ++i, rayY += stepping) {
            pickRay = QSSGRenderRay(/*Origin=*/{rayY, rayX, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
            res = intersectFunc(globalTransform, bounds, pickRay);
            QCOMPARE(res.intersects, true);
        }

        // Do one more step to go outside the box and test that we don't hit.
        rayY += stepping;
        pickRay = QSSGRenderRay(/*Origin=*/{rayY, rayX, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);
    }
}

void intersection::aabbIntersectionScaled(const IntersectionFunc &intersectFunc)
{
    QSSGRenderRay::IntersectionResult res;
    QMatrix4x4 globalTransform; // Identity
    // Bounding box of size 1, placed at origin
    QSSGBounds3 bounds { /*min=*/ QVector3D{ -0.5f, -0.5f, -0.5f }, /*max=*/ QVector3D{ 0.5f, 0.5f, 0.5f } };

    // Pick ray goes directly down the z-axis and should hit directly in the middle of the box
    auto pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
    res = intersectFunc(globalTransform, bounds, pickRay);
    QCOMPARE(res.intersects, true);

    { // Scale +
        globalTransform.setToIdentity();
        const float scale = 2.0f;
        globalTransform.scale(scale);
        const float rayY = bounds.minimum.y() * scale;
        const float rayX = bounds.minimum.x() * scale;
        pickRay = QSSGRenderRay(/*Origin=*/{rayY, rayX, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }

    { // Scale -
        globalTransform.setToIdentity();
        const float scale = -2.0f;
        globalTransform.scale(scale);
        const float rayY = bounds.maximum.y() * scale;
        const float rayX = bounds.maximum.x() * scale;
        pickRay = QSSGRenderRay(/*Origin=*/{rayY, rayX, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }
}

void intersection::aabbIntersectionTranslated(const IntersectionFunc &intersectFunc)
{
    QSSGRenderRay::IntersectionResult res;
    QMatrix4x4 globalTransform; // Identity
    // Bounding box of size 1, placed at origin
    QSSGBounds3 bounds { /*min=*/ QVector3D{ -0.5f, -0.5f, -0.5f }, /*max=*/ QVector3D{ 0.5f, 0.5f, 0.5f } };

    // Pick ray goes directly down the z-axis and should hit directly in the middle of the box
    auto pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
    res = intersectFunc(globalTransform, bounds, pickRay);
    QCOMPARE(res.intersects, true);

    { // Translate 1
        globalTransform.setToIdentity();
        const float uniformTranslation = 1.0f;
        globalTransform.translate(uniformTranslation, uniformTranslation);

        // Verify that we can get a hit in the center of the bounding box
        pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        const float rayY = bounds.minimum.y() + uniformTranslation;
        const float rayX = bounds.minimum.x() + uniformTranslation;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }

    { // Translate 2
        globalTransform.setToIdentity();
        const float translationX = -1.0f;
        const float translationY = 1.0f;
        globalTransform.translate(translationX, translationY);

        // Verify that we can get a hit in the center of the bounding box
        pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        const float rayY = bounds.minimum.y() + translationY;
        const float rayX = bounds.minimum.x() + translationX;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }

    { // Translate 3
        globalTransform.setToIdentity();
        const float uniformTranslation = -1.0f;
        globalTransform.translate(uniformTranslation, uniformTranslation);

        // Verify that we can get a hit in the center of the bounding box
        pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        const float rayY = bounds.maximum.y() + uniformTranslation;
        const float rayX = bounds.maximum.x() + uniformTranslation;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }

    { // Translate 4
        globalTransform.setToIdentity();
        const float translationX = 1.0f;
        const float translationY = -1.0f;
        globalTransform.translate(translationX, translationY);

        // Verify that we can get a hit in the center of the bounding box
        pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        const float rayY = bounds.minimum.y() + translationY;
        const float rayX = bounds.minimum.x() + translationX;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }
}

void intersection::aabbIntersectionRotated(const IntersectionFunc &intersectFunc)
{
    QSSGRenderRay::IntersectionResult res;
    QMatrix4x4 globalTransform; // Identity
    // Bounding box of size 1, placed at origin
    QSSGBounds3 bounds { /*min=*/ QVector3D{ -0.5f, -0.5f, -0.5f }, /*max=*/ QVector3D{ 0.5f, 0.5f, 0.5f } };

    // Pick ray goes directly down the z-axis and should hit directly in the middle of the box
    auto pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
    res = intersectFunc(globalTransform, bounds, pickRay);
    QCOMPARE(res.intersects, true);

    { // Rotate
        globalTransform.setToIdentity();
        const float rotation = 45.0f;
        globalTransform.rotate(rotation, {0.0f, 0.0f, 1.0f});

        // Verify that we can get a hit in the center of the bounding box
        pickRay = QSSGRenderRay(/*Origin=*/{0.0f, 0.0f, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);

        float rayX = bounds.minimum.x();
        float rayY = bounds.minimum.y();
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        rayX = bounds.maximum.x();
        rayY = bounds.maximum.y();
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, false);

        const auto center = bounds.center();
        rayX = center.x();
        rayY = center.y() + std::sqrt(bounds.dimensions().x() + bounds.dimensions().y()) * 0.5f;
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);

        rayX = center.x() + std::sqrt(bounds.dimensions().x() + bounds.dimensions().y()) * 0.5f;
        rayY = center.y();
        pickRay = QSSGRenderRay(/*Origin=*/{rayX, rayY, 10.0f}, /*Direction=*/{0.0f, 0.0f, -1.0f});
        res = intersectFunc(globalTransform, bounds, pickRay);
        QCOMPARE(res.intersects, true);
    }
}

QTEST_APPLESS_MAIN(intersection)

#include "tst_intersection.moc"
