// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtPositioning/QGeoShape>
#include <QtCore/QDebug>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>

QString tst_qgeoshape_debug;

void tst_qgeoshape_messageHandler(QtMsgType type, const QMessageLogContext&,
                                  const QString &msg)
{
    switch (type) {
        case QtDebugMsg :
            tst_qgeoshape_debug = msg;
            break;
        default:
            break;
    }
}

class tst_qgeoshape : public QObject
{
    Q_OBJECT

private slots:
    void testArea();
    void debug_data();
    void debug();
    void conversions();
    void serialization();
    void hashing();
};

void tst_qgeoshape::testArea()
{
    QGeoShape area;
    QVERIFY(!area.isValid());
    QVERIFY(area.isEmpty());
    QCOMPARE(area.type(), QGeoShape::UnknownType);
    QVERIFY(!area.contains(QGeoCoordinate()));

    // QGeoShape never constructs a QGeoShapePrivate.  Hence d_ptr is always 0.

    QGeoShape area2;

    QCOMPARE(area, area2);

    area = area2;

    QCOMPARE(area, area2);

    QGeoShape area3(area2);

    QCOMPARE(area2, area3);
}

void tst_qgeoshape::debug_data()
{
    QTest::addColumn<QGeoShape>("shape");
    QTest::addColumn<int>("nextValue");
    QTest::addColumn<QString>("debugString");

    QTest::newRow("uninitialized") << QGeoShape() << 45
            << QString("QGeoShape(Unknown) 45");
    QTest::newRow("rectangle") << QGeoShape(QGeoRectangle()) << 45
            << QString("QGeoShape(Rectangle) 45");
    QTest::newRow("circle") << QGeoShape(QGeoCircle()) << 45
            << QString("QGeoShape(Circle) 45");
    QTest::newRow("polygon") << QGeoShape(QGeoPolygon()) << 45
            << QString("QGeoShape(Polygon) 45");
    QTest::newRow("path") << QGeoShape(QGeoPath()) << 45
            << QString("QGeoShape(Path) 45");
}


void tst_qgeoshape::debug()
{
    QFETCH(QGeoShape, shape);
    QFETCH(int, nextValue);
    QFETCH(QString, debugString);

    qInstallMessageHandler(tst_qgeoshape_messageHandler);
    qDebug() << shape << nextValue;
    qInstallMessageHandler(0);
    QCOMPARE(tst_qgeoshape_debug, debugString);
}

void tst_qgeoshape::conversions()
{
    QVariant varShape = QVariant::fromValue(QGeoShape());
    QVariant varRect = QVariant::fromValue(QGeoRectangle(
                                               QGeoCoordinate(1, 1),
                                               QGeoCoordinate(2, 2)));
    QVariant varCircle = QVariant::fromValue(QGeoCircle(QGeoCoordinate(3, 3), 1000));

    QVERIFY(varShape.canConvert<QGeoShape>());
    QVERIFY(varShape.canConvert<QGeoCircle>());
    QVERIFY(varShape.canConvert<QGeoRectangle>());
    QVERIFY(!varRect.canConvert<QGeoCircle>());
    QVERIFY(varRect.canConvert<QGeoRectangle>());
    QVERIFY(varRect.canConvert<QGeoShape>());
    QVERIFY(varCircle.canConvert<QGeoCircle>());
    QVERIFY(!varCircle.canConvert<QGeoRectangle>());
    QVERIFY(varCircle.canConvert<QGeoShape>());
}

void tst_qgeoshape::serialization()
{
    QByteArray data;
    // empty shape
    {
        QGeoShape shape;
        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << shape;

        QGeoShape otherShape;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> otherShape;

        QCOMPARE(otherShape, shape);
    }
    // circle
    {
        QGeoCircle circle(QGeoCoordinate(1.1, 2.2), 10.5);
        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << circle;

        QGeoShape otherCircle;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> otherCircle;

        QCOMPARE(otherCircle, circle);
    }
    // rectangle
    {
        QGeoRectangle rectangle(QGeoCoordinate(30, 160), QGeoCoordinate(-30, 170));
        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << rectangle;

        QGeoShape otherRectangle;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> otherRectangle;

        QCOMPARE(otherRectangle, rectangle);
    }
    // polygon
    {
        QGeoPolygon polygon({ QGeoCoordinate(30, 160),
                              QGeoCoordinate(0, 170),
                              QGeoCoordinate(-30, 160) });
        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << polygon;

        QGeoShape otherPolygon;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> otherPolygon;

        QCOMPARE(otherPolygon, polygon);
    }
    // polygon with holes
    {
        QGeoPolygon polygon({ QGeoCoordinate(-50, 50),
                              QGeoCoordinate(50, 50),
                              QGeoCoordinate(50, -50),
                              QGeoCoordinate(-50, -50) });

        const QList<QGeoCoordinate> rectangle{ QGeoCoordinate(-20, 40),
                                               QGeoCoordinate(30, 40),
                                               QGeoCoordinate(30, 30),
                                               QGeoCoordinate(-20, 30) };
        const QList<QGeoCoordinate> triangle{ QGeoCoordinate(-10, -10),
                                              QGeoCoordinate(-40, -40),
                                              QGeoCoordinate(-20, -30) };

        polygon.addHole(rectangle);
        polygon.addHole(triangle);

        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << polygon;

        QGeoShape shape;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> shape;

        QCOMPARE(shape, polygon);
        QGeoPolygon polygonFromShape(shape);
        QCOMPARE(polygonFromShape.holesCount(), 2);
        QCOMPARE(polygonFromShape.holePath(0), rectangle);
        QCOMPARE(polygonFromShape.holePath(1), triangle);
    }
    // path
    {
        QGeoPath path({ QGeoCoordinate(30, 160), QGeoCoordinate(0, 170),
                        QGeoCoordinate(-30, 180) }, 0.5);
        QDataStream writeStream(&data, QDataStream::WriteOnly);
        writeStream << path;

        QGeoShape otherPath;
        QDataStream readStream(&data, QDataStream::ReadOnly);
        readStream >> otherPath;

        QCOMPARE(otherPath, path);
    }
}

void tst_qgeoshape::hashing()
{
    const size_t defaultShapeHash = qHash(QGeoShape());

    const QGeoCircle circle(QGeoCoordinate(1, 2), 10);
    const QGeoShape circleShape = circle;
    const size_t circleShapeHash = qHash(circleShape);
    QVERIFY(defaultShapeHash != circleShapeHash);
    QCOMPARE(qHash(circle), circleShapeHash);

    const QGeoRectangle rectangle(QGeoCoordinate(30, 160), QGeoCoordinate(-30, 170));
    const QGeoShape rectangleShape = rectangle;
    const size_t rectangleShapeHash = qHash(rectangleShape);
    QVERIFY(defaultShapeHash != rectangleShapeHash);
    QVERIFY(circleShapeHash != rectangleShapeHash);
    QCOMPARE(qHash(rectangle), rectangleShapeHash);

    const QGeoPolygon polygon({ QGeoCoordinate(30, 160), QGeoCoordinate(0, 170),
                                QGeoCoordinate(-30, 160) });
    const QGeoShape polygonShape = polygon;
    const size_t polygonShapeHash = qHash(polygonShape);
    QVERIFY(defaultShapeHash != polygonShapeHash);
    QVERIFY(circleShapeHash != polygonShapeHash);
    QVERIFY(rectangleShapeHash != polygonShapeHash);
    QCOMPARE(qHash(polygon), polygonShapeHash);

    const QGeoPath path({ QGeoCoordinate(30, 160), QGeoCoordinate(0, 170),
                          QGeoCoordinate(-30, 180) }, 0.5);
    const QGeoShape pathShape = path;
    const size_t pathShapeHash = qHash(pathShape);
    QVERIFY(defaultShapeHash != pathShapeHash);
    QVERIFY(circleShapeHash != pathShapeHash);
    QVERIFY(rectangleShapeHash != pathShapeHash);
    QVERIFY(polygonShapeHash != pathShapeHash);
    QCOMPARE(qHash(path), pathShapeHash);
}

QTEST_MAIN(tst_qgeoshape)
#include "tst_qgeoshape.moc"
