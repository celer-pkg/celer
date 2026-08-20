// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QTest>

#include <QtQuick/qsggeometry.h>

class GeometryTest : public QObject
{
    Q_OBJECT

public:

private Q_SLOTS:
    void testPoint2D();
    void testTexturedPoint2D();
    void testCustomGeometry();
    void testEmptyGeometry();
    void testSetVertexCount();
    void testSetIndexCount();
    void testPreallocBufferCornercases();

private:
};

void GeometryTest::testPoint2D()
{
    QSGGeometry geometry(QSGGeometry::defaultAttributes_Point2D(), 4, 0);

    QCOMPARE(geometry.attributeCount(), 1);
    QCOMPARE(geometry.sizeOfVertex(), (int) sizeof(float) * 2);
    QCOMPARE(geometry.vertexCount(), 4);
    QCOMPARE(geometry.indexCount(), 0);
    QVERIFY(!geometry.indexData());

    QSGGeometry::updateRectGeometry(&geometry, QRectF(1, 2, 3, 4));

    QSGGeometry::Point2D *pts = geometry.vertexDataAsPoint2D();
    QVERIFY(pts != nullptr);

    QCOMPARE(pts[0].x, (float) 1);
    QCOMPARE(pts[0].y, (float) 2);
    QCOMPARE(pts[3].x, (float) 4);
    QCOMPARE(pts[3].y, (float) 6);

    // Verify that resize gives me enough allocated data without crashing...
    geometry.allocate(100, 100);
    pts = geometry.vertexDataAsPoint2D();
    quint16 *is = geometry.indexDataAsUShort();
    for (int i=0; i<100; ++i) {
        pts[i].x = i;
        pts[i].y = i + 100;
        is[i] = i;
    }
    QVERIFY(true);
}


void GeometryTest::testTexturedPoint2D()
{
    QSGGeometry geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4, 0);

    QCOMPARE(geometry.attributeCount(), 2);
    QCOMPARE(geometry.sizeOfVertex(), (int) sizeof(float) * 4);
    QCOMPARE(geometry.vertexCount(), 4);
    QCOMPARE(geometry.indexCount(), 0);
    QVERIFY(!geometry.indexData());

    QSGGeometry::updateTexturedRectGeometry(&geometry, QRectF(1, 2, 3, 4), QRectF(5, 6, 7, 8));

    QSGGeometry::TexturedPoint2D *pts = geometry.vertexDataAsTexturedPoint2D();
    QVERIFY(pts != nullptr);

    QCOMPARE(pts[0].x, (float) 1);
    QCOMPARE(pts[0].y, (float) 2);
    QCOMPARE(pts[0].tx, (float) 5);
    QCOMPARE(pts[0].ty, (float) 6);

    QCOMPARE(pts[3].x, (float) 4);
    QCOMPARE(pts[3].y, (float) 6);
    QCOMPARE(pts[3].tx, (float) 12);
    QCOMPARE(pts[3].ty, (float) 14);

    // Verify that resize gives me enough allocated data without crashing...
    geometry.allocate(100, 100);
    pts = geometry.vertexDataAsTexturedPoint2D();
    quint16 *is = geometry.indexDataAsUShort();
    for (int i=0; i<100; ++i) {
        pts[i].x = i;
        pts[i].y = i + 100;
        pts[i].tx = i + 200;
        pts[i].ty = i + 300;
        is[i] = i;
    }
    QVERIFY(true);
}

void GeometryTest::testCustomGeometry()
{
    struct V {
        float x, y;
        unsigned char r, g, b, a;
        float v1, v2, v3, v4;
    };

    static QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, false),
        QSGGeometry::Attribute::create(1, 4, QSGGeometry::UnsignedByteType, false),
        QSGGeometry::Attribute::create(2, 4, QSGGeometry::FloatType, false)
    };
    static QSGGeometry::AttributeSet set = { 4, 6 * sizeof(float) + 4 * sizeof(unsigned char), attributes };

    QSGGeometry geometry(set, 1000, 4000);

    // Verify that space has been allocated.
    quint16 *ii = geometry.indexDataAsUShort();
    for (int i=0; i<geometry.indexCount(); ++i) {
        ii[i] = i;
    }

    V *v = (V *) geometry.vertexData();
    for (int i=0; i<geometry.vertexCount(); ++i) {
        v[i].x = 0;
        v[i].y = 1;
        v[i].r = 2;
        v[i].g = 3;
        v[i].b = 4;
        v[i].a = 5;
        v[i].v1 = 6;
        v[i].v2 = 7;
        v[i].v3 = 8;
        v[i].v4 = 9;
    }

    // Verify the data's integrity
    for (int i=0; i<4000; ++i)
        QCOMPARE(ii[i], (quint16) i);
    for (int i=0; i<1000; ++i)
        QCOMPARE(v[i].v1, float(6));
}

void GeometryTest::testEmptyGeometry()
{
    // This test verifies that behavior of vertexData() does not change with
    // new-to-6.9 functions: setVertexCount() and setIndexCount().
    //
    // Prior to Qt 6.9, vertexData() returns nullptr if geometry is first
    // created with zero vertices, but returns the address of the m_prealloc
    // buffer if created with non-zero \a vertexCount and is resized to 0.
    //
    // This behavior seems inconsistent, but it allows testing whether a
    // geometry ever had vertices. It is unknown whether this side-effect
    // is desired or used.

    QSGGeometry geometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);

    // when constructed with zero vertices, vertex data returns nulltpr
    QCOMPARE(geometry.vertexData(), nullptr);

    // Allocate storage for vertices
    geometry.allocate(16);

    auto *v = (QSGGeometry::ColoredPoint2D *) geometry.vertexData();

    QVERIFY(v != nullptr);

    // Clear storage for vertices
    geometry.allocate(0);

    // With zero vertices, vertex data points to m_prealloc buffer
    QVERIFY(geometry.vertexData() != nullptr);
    QVERIFY(geometry.vertexData() != v);
}

void GeometryTest::testSetVertexCount()
{
    QSGGeometry geometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 2000);

    QCOMPARE(geometry.attributeCount(), 2);
    QCOMPARE(geometry.sizeOfVertex(), (int) (sizeof(float) * 2 + sizeof(char) * 4));

    QCOMPARE(geometry.vertexCount(), 2000);

    auto *v = (QSGGeometry::ColoredPoint2D *) geometry.vertexData();

    geometry.setVertexCount(0);
    QCOMPARE(geometry.vertexCount(), 0);

    geometry.setVertexCount(2000);
    QCOMPARE(geometry.vertexCount(), 2000);

    // Verify same location in memory is still used
    QCOMPARE(geometry.vertexData(), v);
}

void GeometryTest::testSetIndexCount()
{
    QSGGeometry geometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 2000, 3000);

    QCOMPARE(geometry.attributeCount(), 2);

    // Verify that indexes are unaffected by changing the count
    quint16 *ii = geometry.indexDataAsUShort();
    for (int i=0; i<geometry.indexCount(); ++i) {
        ii[i] = i;
    }

    QCOMPARE(geometry.indexCount(), 3000);

    geometry.setIndexCount(0);
    QCOMPARE(geometry.indexCount(), 0);

    geometry.setIndexCount(3000);
    QCOMPARE(geometry.indexCount(), 3000);

    // Verify same location in memory is still used
    QCOMPARE(geometry.indexDataAsUShort(), ii);

    // Verify all indexes remain consistent
    for (int i=0; i<3000; ++i)
        QCOMPARE(ii[i], (quint16) i);
}

void GeometryTest::testPreallocBufferCornercases()
{
    // This test verifies m_prealloc behavior as it existed prior to adding
    // setVertexCount() and setIndexCount() for Qt 6.9. Given that it is
    // undocumented, this test will inform whoever breaks it in the future.

    struct V {
        float x, y, alpha;
    };

    static QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 3, QSGGeometry::FloatType, false),
    };
    static QSGGeometry::AttributeSet set = { 1, 3 * sizeof(float), attributes };

    QSGGeometry geometry(set, 1);

    // Store address of the m_prealloc buffer for buffer checks
    const auto *v = geometry.vertexData();

    QCOMPARE(geometry.vertexCount(), 1);
    QCOMPARE(geometry.indexCount(), 0);
    QCOMPARE(geometry.indexData(), nullptr);

    // Verify the maximum number of points that fit without reallocating
    geometry.allocate(5);
    QCOMPARE(geometry.vertexData(), v);

    // Verify "one more" requires allocating heap memory
    geometry.allocate(6);
    QVERIFY(v != geometry.vertexData());

    // Verify that resizing down fits uses m_prealloc buffer again
    geometry.allocate(2);
    QCOMPARE(geometry.vertexData(), v);
    geometry.allocate(5);
    QCOMPARE(geometry.vertexData(), v);
}

QTEST_MAIN(GeometryTest);

#include "tst_geometry.moc"
