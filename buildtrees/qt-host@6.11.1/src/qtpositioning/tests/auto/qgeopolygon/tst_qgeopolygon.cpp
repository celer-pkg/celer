// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/qgeocoordinate.h>
#include <QtPositioning/qgeopath.h>
#include <QtPositioning/qgeopolygon.h>
#include <QtPositioning/qgeorectangle.h>

#include <QtPositioning/private/qgeopolygon_p.h>

#include <QtTest/qtest.h>

QT_USE_NAMESPACE

class tst_QGeoPolygon : public QObject
{
    Q_OBJECT

private:
    enum class Type : quint8 {
        Lazy,
        Eager,
    };

    QGeoPolygon constructPolygon(const QList<QGeoCoordinate> &coords);

private slots:
    void initTestCase_data();

    void defaultConstructor();
    void listConstructor();
    void assignment();

    void comparison();
    void type();

    void path();
    void size();

    void translate_data();
    void translate();

    void valid_data();
    void valid();

    void contains_data();
    void contains();

    void containsAfterCopy();

    void boundingGeoRectangle_data();
    void boundingGeoRectangle();

    void boundingGeoRectangleWithCopy();

    void hashing();

    void toString();
};

QGeoPolygon tst_QGeoPolygon::constructPolygon(const QList<QGeoCoordinate> &coords)
{
    QFETCH_GLOBAL(const Type, type);

    switch (type) {
    case Type::Lazy:
        return QGeoPolygon(coords);
    case Type::Eager:
        return QGeoPolygonEager(coords);
    }
}

void tst_QGeoPolygon::initTestCase_data()
{
    QTest::addColumn<Type>("type");
    QTest::newRow("Lazy") << Type::Lazy;
    QTest::newRow("Eager") << Type::Eager;
}

void tst_QGeoPolygon::defaultConstructor()
{
    QGeoPolygon p;
    QVERIFY(!p.perimeter().size());
    QVERIFY(!p.size());
    QVERIFY(!p.isValid());
    QVERIFY(p.isEmpty());
}

void tst_QGeoPolygon::listConstructor()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    QGeoPolygon p2 = constructPolygon(coords);
    QCOMPARE(p2.perimeter().size(), 2);
    QCOMPARE(p2.size(), 2);
    QVERIFY(!p2.isValid()); // a polygon can't have only 2 coords
    QVERIFY(!p2.isEmpty());

    coords.append(QGeoCoordinate(3,0));

    QGeoPolygon p = constructPolygon(coords);
    QCOMPARE(p.perimeter().size(), 3);
    QCOMPARE(p.size(), 3);
    QVERIFY(p.isValid());
    QVERIFY(!p.isEmpty());


    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.perimeter().contains(c), true);
        QCOMPARE(p.containsCoordinate(c), true);
    }
}

void tst_QGeoPolygon::assignment()
{
    QGeoPolygon p1;
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QGeoPolygon p2 = constructPolygon(coords);

    QVERIFY(p1 != p2);

    p1 = p2;
    QCOMPARE(p1.perimeter(), coords);
    QCOMPARE(p1, p2);

    // Assign c1 to an area
    QGeoShape area = p1;
    QCOMPARE(area.type(), p1.type());
    QVERIFY(area == p1);

    // Assign the area back to a polygon
    QGeoPolygon p3 = area;
    QCOMPARE(p3.perimeter(), coords);
    QVERIFY(p3 == p1);

    // Check that the copy is not modified when modifying the original.
    p1.addCoordinate(QGeoCoordinate(4,0));
    QVERIFY(p3 != p1);
}

void tst_QGeoPolygon::comparison()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QList<QGeoCoordinate> coords2;
    coords2.append(QGeoCoordinate(3,1));
    coords2.append(QGeoCoordinate(4,2));
    coords2.append(QGeoCoordinate(3,0));
    QGeoPolygon c1 = constructPolygon(coords);
    QGeoPolygon c2 = constructPolygon(coords);
    QGeoPolygon c3 = constructPolygon(coords2);

    QVERIFY(c1 == c2);
    QVERIFY(!(c1 != c2));

    QVERIFY(!(c1 == c3));
    QVERIFY(c1 != c3);

    QVERIFY(!(c2 == c3));
    QVERIFY(c2 != c3);

    QGeoRectangle b1(QGeoCoordinate(20,20),QGeoCoordinate(10,30));
    QVERIFY(!(c1 == b1));
    QVERIFY(c1 != b1);

    // Create a QGeoPolygon with the same vertices as the rectangle
    QGeoPolygon sameAsB1 = constructPolygon({ b1.topLeft(), b1.topRight(),
                                              b1.bottomRight(), b1.bottomLeft() });
    // It still should not be equal to b1, because the types are different
    QCOMPARE_NE(sameAsB1, b1);

    // Now construct from b1
    QGeoPolygon fromB1(b1);
    QCOMPARE_EQ(fromB1, sameAsB1);

    QGeoShape *c2Ptr = &c2;
    QVERIFY(c1 == *c2Ptr);
    QVERIFY(!(c1 != *c2Ptr));

    QGeoShape *c3Ptr = &c3;
    QVERIFY(!(c1 == *c3Ptr));
    QVERIFY(c1 != *c3Ptr);

    // Check comparison with QGeoPath with the same coordinates.
    // Objects should not match, because the types are different.
    QGeoPath path(coords, 1.0f);
    QCOMPARE_NE(path, c1);
}

void tst_QGeoPolygon::type()
{
    QGeoPolygon c;
    QCOMPARE(c.type(), QGeoShape::PolygonType);
}

void tst_QGeoPolygon::path()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));

    QGeoPolygon p;
    p.setPerimeter(coords);
    QCOMPARE(p.perimeter().size(), 3);
    QCOMPARE(p.size(), 3);

    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.perimeter().contains(c), true);
        QCOMPARE(p.containsCoordinate(c), true);
    }
}

void tst_QGeoPolygon::size()
{
    QList<QGeoCoordinate> coords;

    QGeoPolygon p1 = constructPolygon(coords);
    QCOMPARE(p1.size(), coords.size());

    coords.append(QGeoCoordinate(1,1));
    QGeoPolygon p2 = constructPolygon(coords);
    QCOMPARE(p2.size(), coords.size());

    coords.append(QGeoCoordinate(2,2));
    QGeoPolygon p3 = constructPolygon(coords);
    QCOMPARE(p3.size(), coords.size());

    coords.append(QGeoCoordinate(3,0));
    QGeoPolygon p4 = constructPolygon(coords);
    QCOMPARE(p4.size(), coords.size());

    p4.removeCoordinate(2);
    QCOMPARE(p4.size(), coords.size() - 1);

    p4.removeCoordinate(coords.first());
    QCOMPARE(p4.size(), coords.size() - 2);
}

void tst_QGeoPolygon::translate_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<double>("lat");
    QTest::addColumn<double>("lon");

    QTest::newRow("Simple") << QGeoCoordinate(1,1) << QGeoCoordinate(2,2) <<
                                 QGeoCoordinate(3,0) << 5.0 << 4.0;
    QTest::newRow("Backward") << QGeoCoordinate(1,1) << QGeoCoordinate(2,2) <<
                                 QGeoCoordinate(3,0) << -5.0 << -4.0;
}

void tst_QGeoPolygon::translate()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(double, lat);
    QFETCH(double, lon);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p = constructPolygon(coords);

    p.translate(lat, lon);

    for (int i = 0; i < p.perimeter().size(); i++) {
        QCOMPARE(coords[i].latitude(), p.perimeter()[i].latitude() - lat );
        QCOMPARE(coords[i].longitude(), p.perimeter()[i].longitude() - lon );
    }
}

void tst_QGeoPolygon::valid_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<bool>("valid");

    QTest::newRow("empty coords") << QGeoCoordinate() << QGeoCoordinate() << QGeoCoordinate() << false;
    QTest::newRow("invalid coord") << QGeoCoordinate(50, 50) << QGeoCoordinate(60, 60) << QGeoCoordinate(700, 700) << false;
    QTest::newRow("good") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12)  << true;
}

void tst_QGeoPolygon::valid()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(bool, valid);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p = constructPolygon(coords);

    QCOMPARE(p.isValid(), valid);

    QGeoShape area = p;
    QCOMPARE(area.isValid(), valid);
}

void tst_QGeoPolygon::contains_data()
{
    QTest::addColumn<QList<QGeoCoordinate>>("coords");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    const QList<QGeoCoordinate> c = { QGeoCoordinate(1, 1),
                                      QGeoCoordinate(2, 2),
                                      QGeoCoordinate(3, 0) };

    QTest::newRow("One of the points") << c <<  QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c << QGeoCoordinate(0.8, 0.8) << false;
    QTest::newRow("Inside") << c << QGeoCoordinate(2.0, 1.0) << true;

    const QList<QGeoCoordinate> wrapOver180 = { QGeoCoordinate(20, 175),
                                                QGeoCoordinate(20, -175),
                                                QGeoCoordinate(10, -175),
                                                QGeoCoordinate(10, 175) };
    QTest::newRow("Inside neg. long. with wrap") << wrapOver180 << QGeoCoordinate(15, -179) << true;
    QTest::newRow("Outside neg. long. with wrap") << wrapOver180 << QGeoCoordinate(15, -170) << false;
    QTest::newRow("Inside pos. long. with wrap") << wrapOver180 << QGeoCoordinate(15, 179) << true;
    QTest::newRow("Outside pos. long. with wrap") << wrapOver180 << QGeoCoordinate(15, 170) << false;
}

void tst_QGeoPolygon::contains()
{
    QFETCH(const QList<QGeoCoordinate>, coords);
    QFETCH(const QGeoCoordinate, probe);
    QFETCH(const bool, result);

    QGeoPolygon p = constructPolygon(coords);

    QCOMPARE(p.contains(probe), result);

    QGeoShape area = p;
    QCOMPARE(area.contains(probe), result);
}

void tst_QGeoPolygon::containsAfterCopy()
{
    // This test is to make sure that we copy the QClipperUtils in the
    // QGeoPolygonPrivate correctly.
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1, 1));
    coords.append(QGeoCoordinate(2, 2));
    coords.append(QGeoCoordinate(3, 0));

    const QGeoCoordinate testPoint(2.0, 1.0);

    QGeoPolygon p1 = constructPolygon(coords);
    QVERIFY(p1.contains(testPoint));

    QGeoPolygon p2 = p1;
    QVERIFY(p2.contains(testPoint));

    p2.translate(10, 10); // does not contain testPoint any more
    QVERIFY(!p2.contains(testPoint));
    QVERIFY(p1.contains(testPoint));
    // This check is intentional! Needed to make sure that p1 does not modify
    // the internals of p2 somehow.
    QVERIFY(!p2.contains(testPoint));
}

void tst_QGeoPolygon::boundingGeoRectangle_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    QList<QGeoCoordinate> c;
    c.append(QGeoCoordinate(1,1));
    c.append(QGeoCoordinate(2,2));
    c.append(QGeoCoordinate(3,0));

    QTest::newRow("One of the points") << c[0] << c[1] << c[2] << QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c[0] << c[1] << c[2] <<  QGeoCoordinate(0, 0) << false;
    QTest::newRow("Inside the polygon") << c[0] << c[1] << c[2] <<  QGeoCoordinate(2, 1) << true;
    QTest::newRow("Inside the bounding rectangle") << c[0] << c[1] << c[2] <<  QGeoCoordinate(1.1, 0.1) << true;
}

void tst_QGeoPolygon::boundingGeoRectangle()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(QGeoCoordinate, probe);
    QFETCH(bool, result);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p = constructPolygon(coords);

    QGeoRectangle box = p.boundingGeoRectangle();
    QCOMPARE(box.contains(probe), result);
}

void tst_QGeoPolygon::boundingGeoRectangleWithCopy()
{
    const auto coordinates = QList{ QGeoCoordinate(1, 1),
                                    QGeoCoordinate(1, 2),
                                    QGeoCoordinate(2, 3) };
    QGeoPolygon p = constructPolygon(coordinates);

    QGeoRectangle box = p.boundingGeoRectangle();

    QGeoPolygon copy = p;
    QGeoRectangle copyBox = copy.boundingGeoRectangle();
    // matches the original
    QCOMPARE_EQ(copyBox, box);

    copy.addCoordinate(QGeoCoordinate(3, 5));
    copyBox = copy.boundingGeoRectangle();
    // does not match the original anymore
    QCOMPARE_NE(copyBox, box);
}

void tst_QGeoPolygon::hashing()
{
    const QGeoPolygon polygon = constructPolygon({ QGeoCoordinate(1, 1), QGeoCoordinate(2, 2),
                                                   QGeoCoordinate(3, 0) });
    const size_t polygonHash = qHash(polygon);

    QGeoPolygon otherCoordsPolygon = polygon;
    otherCoordsPolygon.addCoordinate(QGeoCoordinate(4, 1));
    QVERIFY(qHash(otherCoordsPolygon) != polygonHash);

    QGeoPolygon otherHolesPolygon = polygon;
    otherHolesPolygon.addHole({ QGeoCoordinate(1.1, 1), QGeoCoordinate(2, 1.8),
                                QGeoCoordinate(2, 1) });
    QVERIFY(qHash(otherHolesPolygon) != polygonHash);

    // Do not assign, so that they do not share same d_ptr
    QGeoPolygon similarPolygon = constructPolygon({ QGeoCoordinate(1, 1), QGeoCoordinate(2, 2),
                                                    QGeoCoordinate(3, 0) });
    QCOMPARE(qHash(similarPolygon), polygonHash);
}

void tst_QGeoPolygon::toString()
{
    const QGeoPolygon p = constructPolygon({ QGeoCoordinate(1, 1), QGeoCoordinate(2, 2),
                                             QGeoCoordinate(3, 0) });

    const QString str = p.toString();
    QCOMPARE(str, QStringLiteral("QGeoPolygon([ 1° 0' 0.0\" N, 1° 0' 0.0\" E; "
                                 "2° 0' 0.0\" N, 2° 0' 0.0\" E; "
                                 "3° 0' 0.0\" N, 0° 0' 0.0\" ])"));
}

QTEST_MAIN(tst_QGeoPolygon)
#include "tst_qgeopolygon.moc"
