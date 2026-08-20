// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtPositioning/qgeocoordinate.h>
#include <QtPositioning/qgeopath.h>
#include <QtPositioning/qgeorectangle.h>

#include <QtPositioning/private/qgeopath_p.h>

#include <QtTest/qtest.h>

QT_USE_NAMESPACE

class tst_QGeoPath : public QObject
{
    Q_OBJECT

private:
    enum class Type : quint8 {
        Lazy,
        Eager,
    };

    QGeoPath constructPath(const QList<QGeoCoordinate> &coords, qreal width = 0.0);

private slots:
    void initTestCase_data();
    void defaultConstructor();
    void listConstructor();
    void assignment();

    void comparison();
    void type();

    void path();
    void width();
    void size();

    void translate_data();
    void translate();

    void valid_data();
    void valid();

    void contains_data();
    void contains();

    void boundingGeoRectangle_data();
    void boundingGeoRectangle();

    void boundingGeoRectangleWithCopy();

    void hashing();
};

QGeoPath tst_QGeoPath::constructPath(const QList<QGeoCoordinate> &coords, qreal width)
{
    QFETCH_GLOBAL(const Type, type);

    switch (type) {
    case Type::Lazy:
        return QGeoPath(coords, width);
    case Type::Eager:
        return QGeoPathEager(coords, width);
    }
}

void tst_QGeoPath::initTestCase_data()
{
    QTest::addColumn<Type>("type");
    QTest::newRow("Lazy") << Type::Lazy;
    QTest::newRow("Eager") << Type::Eager;
}

void tst_QGeoPath::defaultConstructor()
{
    QGeoPath p;
    QVERIFY(!p.path().size());
    QCOMPARE(p.width(), qreal(0.0));
    QCOMPARE(p.length(), double(0.0));
}

void tst_QGeoPath::listConstructor()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));

    QGeoPath p = constructPath(coords, 1.0);
    QCOMPARE(p.width(), qreal(1.0));
    QCOMPARE(p.path().size(), 3);

    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.path().contains(c), true);
    }
}

void tst_QGeoPath::assignment()
{
    QGeoPath p1;
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QGeoPath p2 = constructPath(coords, 1.0);

    QVERIFY(p1 != p2);

    p1 = p2;
    QCOMPARE(p1.path(), coords);
    QCOMPARE(p1.width(), 1.0);
    QCOMPARE(p1, p2);

    // Assign c1 to an area
    QGeoShape area = p1;
    QCOMPARE(area.type(), p1.type());
    QVERIFY(area == p1);

    // Assign the area back to a bounding circle
    QGeoPath p3 = area;
    QCOMPARE(p3.path(), coords);
    QCOMPARE(p3.width(), 1.0);

    // Check that the copy is not modified when modifying the original.
    p1.setWidth(2.0);
    QVERIFY(p3.width() != p1.width());
    QVERIFY(p3 != p1);
}

void tst_QGeoPath::comparison()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QList<QGeoCoordinate> coords2;
    coords2.append(QGeoCoordinate(3,1));
    coords2.append(QGeoCoordinate(4,2));
    coords2.append(QGeoCoordinate(3,0));
    QGeoPath c1 = constructPath(coords, qreal(50.0));
    QGeoPath c2 = constructPath(coords, qreal(50.0));
    QGeoPath c3 = constructPath(coords, qreal(35.0));
    QGeoPath c4 = constructPath(coords2, qreal(50.0));

    QVERIFY(c1 == c2);
    QVERIFY(!(c1 != c2));

    QVERIFY(!(c1 == c3));
    QVERIFY(c1 != c3);

    QVERIFY(!(c1 == c4));
    QVERIFY(c1 != c4);

    QVERIFY(!(c2 == c3));
    QVERIFY(c2 != c3);

    QGeoRectangle b1(QGeoCoordinate(20,20),QGeoCoordinate(10,30));
    QVERIFY(!(c1 == b1));
    QVERIFY(c1 != b1);

    QGeoShape *c2Ptr = &c2;
    QVERIFY(c1 == *c2Ptr);
    QVERIFY(!(c1 != *c2Ptr));

    QGeoShape *c3Ptr = &c3;
    QVERIFY(!(c1 == *c3Ptr));
    QVERIFY(c1 != *c3Ptr);
}

void tst_QGeoPath::type()
{
    QGeoPath c;
    QCOMPARE(c.type(), QGeoShape::PathType);
}

void tst_QGeoPath::path()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));

    QGeoPath p;
    p.setPath(coords);
    QCOMPARE(p.path().size(), 3);

    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.path().contains(c), true);
    }

    p.clearPath();
    QCOMPARE(p.path().size(), 0);
    QVERIFY(p.boundingGeoRectangle().isEmpty());
}

void tst_QGeoPath::width()
{
    QGeoPath p;
    p.setWidth(10.0);
    QCOMPARE(p.width(), qreal(10.0));
}

void tst_QGeoPath::size()
{
    QList<QGeoCoordinate> coords;

    QGeoPath p1 = constructPath(coords, 3);
    QCOMPARE(p1.size(), coords.size());

    coords.append(QGeoCoordinate(1,1));
    QGeoPath p2 = constructPath(coords, 3);
    QCOMPARE(p2.size(), coords.size());

    coords.append(QGeoCoordinate(2,2));
    QGeoPath p3 = constructPath(coords, 3);
    QCOMPARE(p3.size(), coords.size());

    coords.append(QGeoCoordinate(3,0));
    QGeoPath p4 = constructPath(coords, 3);
    QCOMPARE(p4.size(), coords.size());

    p4.removeCoordinate(2);
    QCOMPARE(p4.size(), coords.size() - 1);

    p4.removeCoordinate(coords.first());
    QCOMPARE(p4.size(), coords.size() - 2);
}

void tst_QGeoPath::translate_data()
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

void tst_QGeoPath::translate()
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
    QGeoPath p = constructPath(coords);

    p.translate(lat, lon);

    for (int i = 0; i < p.path().size(); i++) {
        QCOMPARE(coords[i].latitude(), p.path()[i].latitude() - lat );
        QCOMPARE(coords[i].longitude(), p.path()[i].longitude() - lon );
    }
}

void tst_QGeoPath::valid_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<bool>("valid");

    QTest::newRow("empty coords") << QGeoCoordinate() << QGeoCoordinate() << QGeoCoordinate() << qreal(5.0) << false;
    QTest::newRow("invalid coord") << QGeoCoordinate(50, 50) << QGeoCoordinate(60, 60) << QGeoCoordinate(700, 700) << qreal(5.0) << false;
    QTest::newRow("bad width") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12) << qreal(-5.0) << true;
    QTest::newRow("NaN width") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12) << qreal(qQNaN()) << true;
    QTest::newRow("zero width") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12) << qreal(0) << true;
    QTest::newRow("good") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12) << qreal(5) << true;
}

void tst_QGeoPath::valid()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(qreal, width);
    QFETCH(bool, valid);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPath p = constructPath(coords, width);

    QCOMPARE(p.isValid(), valid);

    QGeoShape area = p;
    QCOMPARE(area.isValid(), valid);
}

void tst_QGeoPath::contains_data()
{
    QTest::addColumn<QList<QGeoCoordinate>>("coordinates");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    const QList<QGeoCoordinate> c {
        QGeoCoordinate(1, 1),
        QGeoCoordinate(2, 2),
        QGeoCoordinate(3, 0)
    };

    QTest::newRow("One of the points") << c << 0.0 << QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c << 0.0 << QGeoCoordinate(0.8, 0.8) << false;
    QTest::newRow("Not so far away and thick line")
            << c << 100000.0 << QGeoCoordinate(0.8, 0.8) << true;

    // Test case from QTBUG-142317
    QTest::newRow("coord-lon-left-from-path-no-wrap-needed")
            << QList{QGeoCoordinate(50, 7), QGeoCoordinate(52, 6.999)} << 10000.0
            << QGeoCoordinate(51, 6.95) << true;

    // Testing various cases:
    // - wrapped line, need to wrap the coord
    // - wrapped line, no need to wrap the coord
    // - non-wrapped line, need to wrap the coord
    // - non-wrapped line, no need to wrap the coord

    const qreal width{10000.0};
    const QList<QGeoCoordinate> wrappedPath {
        QGeoCoordinate(50, 179.95),
        QGeoCoordinate(52, -179.95)
    };

    QTest::newRow("wrap-path-no-wrap-coord")
            << wrappedPath << width << QGeoCoordinate(50.2, 179.94) << true;
    QTest::newRow("wrap-path-wrap-coord")
            << wrappedPath << width << QGeoCoordinate(52, -179.98) << true;

    // As close to the 180/-180 wrap line as possible
    const QList<QGeoCoordinate> notWrappedPathWest {
        QGeoCoordinate(50, -179.99),
        QGeoCoordinate(52, -179.98)
    };

    QTest::newRow("no-wrap-path-no-wrap-coord")
            << notWrappedPathWest << width << QGeoCoordinate(50.2, -180.0) << true;
    QTest::newRow("no-wrap-path-wrap-coord-subtract")
            << notWrappedPathWest << width << QGeoCoordinate(50.5, 179.99) << true;

    const QList<QGeoCoordinate> notWrappedPathEast {
        QGeoCoordinate(50, 179.99),
        QGeoCoordinate(52, 179.98)
    };

    QTest::newRow("no-wrap-path-wrap-coord-add")
            << notWrappedPathEast << width << QGeoCoordinate(50.5, -179.99) << true;

    const QList<QGeoCoordinate> cycledOverlap {
        QGeoCoordinate(-70, 171), // i.e. -189
        QGeoCoordinate(-50, -135), // Each step is + (20, 54)
        QGeoCoordinate(-30, -81),
        QGeoCoordinate(-10, -27),
        QGeoCoordinate(10, 27),
        QGeoCoordinate(30, 81),
        QGeoCoordinate(50, 135), // (58, 153) is near this last step
        QGeoCoordinate(70, -171), // i.e. 189
    };
    // Bounding box is -70 <= lat <= 70, 171 <= lon <= -171 = 189 ;^>

    QTest::newRow("cycled-wrap")
            << cycledOverlap << qreal(23000) << QGeoCoordinate(58, 153) << true;
    // Witness:
    // QWebMercator::coordinateInterpolation(QGC(50, 135), QGC(70, -171), 1./3); is QGC(58.094, 153)
    QCOMPARE_LT(QGeoCoordinate(58, 153).distanceTo(QGeoCoordinate(58.094, 153)), 11000);
}

void tst_QGeoPath::contains()
{
    QFETCH(const QList<QGeoCoordinate>, coordinates);
    QFETCH(const qreal, width);
    QFETCH(const QGeoCoordinate, probe);
    QFETCH(const bool, result);

    const QGeoPath p = constructPath(coordinates, width);

    QCOMPARE(p.contains(probe), result);

    const QGeoShape area = p;
    QCOMPARE(area.contains(probe), result);
}

void tst_QGeoPath::boundingGeoRectangle_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    QList<QGeoCoordinate> c;
    c.append(QGeoCoordinate(1,1));
    c.append(QGeoCoordinate(2,2));
    c.append(QGeoCoordinate(3,0));

    QTest::newRow("One of the points") << c[0] << c[1] << c[2] << 0.0 << QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c[0] << c[1] << c[2] << 0.0 << QGeoCoordinate(0, 0) << false;
    QTest::newRow("Inside the bounds") << c[0] << c[1] << c[2] << 0.0 << QGeoCoordinate(2, 1) << true;
    QTest::newRow("Inside the bounds considering width") << c[0] << c[1] << c[2] << 100.0 << QGeoCoordinate(1.1, 0.1) << true;
}

void tst_QGeoPath::boundingGeoRectangle()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(qreal, width);
    QFETCH(QGeoCoordinate, probe);
    QFETCH(bool, result);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPath p = constructPath(coords, width);

    QGeoRectangle box = p.boundingGeoRectangle();
    QCOMPARE(box.contains(probe), result);
}

void tst_QGeoPath::boundingGeoRectangleWithCopy()
{
    const auto coordinates = QList{ QGeoCoordinate(1, 1),
                                    QGeoCoordinate(1, 2),
                                    QGeoCoordinate(2, 3) };
    QGeoPath p = constructPath(coordinates, 1.0);

    QGeoRectangle box = p.boundingGeoRectangle();

    QGeoPath copy = p;
    QGeoRectangle copyBox = copy.boundingGeoRectangle();
    // matches the original
    QCOMPARE_EQ(copyBox, box);

    copy.addCoordinate(QGeoCoordinate(3, 5));
    copyBox = copy.boundingGeoRectangle();
    // does not match the original anymore
    QCOMPARE_NE(copyBox, box);
    // verify that the original path is not affected
    QCOMPARE_EQ(p.boundingGeoRectangle(), box);
}

void tst_QGeoPath::hashing()
{
    const QGeoPath path = constructPath({ QGeoCoordinate(1, 1),
                                          QGeoCoordinate(1, 2),
                                          QGeoCoordinate(2, 5) },
                                        1.0);
    const size_t pathHash = qHash(path);

    QGeoPath otherCoordsPath = path;
    otherCoordsPath.addCoordinate(QGeoCoordinate(3, 5));
    QVERIFY(qHash(otherCoordsPath) != pathHash);

    QGeoPath otherWidthPath = path;
    otherWidthPath.setWidth(1.5);
    QVERIFY(qHash(otherWidthPath) != pathHash);

    // Do not assign, so that they do not share same d_ptr
    QGeoPath similarPath = constructPath({ QGeoCoordinate(1, 1),
                                           QGeoCoordinate(1, 2),
                                           QGeoCoordinate(2, 5) },
                                         1.0);
    QCOMPARE(qHash(similarPath), pathHash);
}

QTEST_MAIN(tst_QGeoPath)
#include "tst_qgeopath.moc"
