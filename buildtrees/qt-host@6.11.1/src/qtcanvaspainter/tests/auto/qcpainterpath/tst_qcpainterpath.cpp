// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <qdebug.h>
#include <QTransform>

#include "qcanvaspath.h"
#include "qcanvaspainter.h"

class tst_QCanvasPath : public QObject
{
    Q_OBJECT

private slots:
    // Path autotests
    void testConstructors();
    void testEqual();
    void testAppending();
    void testSlicing();
};

void tst_QCanvasPath::testConstructors()
{
    QCanvasPath p1;
    QVERIFY(p1.isEmpty());
    // Note: Currently there is no initial capasity.
    QCOMPARE(p1.commandsCapacity(), 0);
    QCOMPARE(p1.commandsDataCapacity(), 0);
    QCanvasPath p2(10);
    QCOMPARE(p2.commandsCapacity(), 10);
    QCOMPARE(p2.commandsDataCapacity(), 20);
    QCanvasPath p3(10, 40);
    QCOMPARE(p3.commandsCapacity(), 10);
    QCOMPARE(p3.commandsDataCapacity(), 40);
    QCanvasPath p4(p3);
    QCOMPARE(p4.commandsCapacity(), 10);
    QCOMPARE(p4.commandsDataCapacity(), 40);
    QCanvasPath p5 = p4;
    QCOMPARE(p5.commandsCapacity(), 10);
    QCOMPARE(p5.commandsDataCapacity(), 40);

    p5.reserve(23);
    QCOMPARE(p5.commandsCapacity(), 23);
    QCOMPARE(p5.commandsDataCapacity(), 2 * 23);
    p5.reserve(15, 51);
    QCOMPARE(p5.commandsCapacity(), 15);
    QCOMPARE(p5.commandsDataCapacity(), 51);
}

void tst_QCanvasPath::testEqual()
{
    QCanvasPath p1;
    QCanvasPath p2;
    QCOMPARE(p1, p2);
    QCOMPARE(p1.isEmpty(), true);

    p1.moveTo(10, 20);
    QCOMPARE_NE(p1, p2);
    p2.moveTo(10, 20);
    QCOMPARE(p1, p2);
    p1.lineTo(30, 40);
    QCOMPARE_NE(p1, p2);
    p2.lineTo(30, 40);
    QCOMPARE(p1, p2);
    p1.rect(1, 2, 3, 4);
    QCOMPARE_NE(p1, p2);
    p2 = p1;
    QCOMPARE(p1, p2);

    p2.clear();
    QCOMPARE(p2.isEmpty(), true);
    QCOMPARE_NE(p1, p2);
    p1.clear();
    QCOMPARE(p1.isEmpty(), true);
    QCOMPARE(p1, p2);
}

void tst_QCanvasPath::testAppending()
{
    {
        // addPath, full
        QCanvasPath p1;
        p1.moveTo(0, 100);
        p1.lineTo(50, 100);
        p1.lineTo(100, 50);
        p1.lineTo(150, 100);
        QCOMPARE(p1.commandsSize(), 4);

        QCanvasPath p2;
        p2.addPath(p1);
        QCOMPARE(p1, p2);
        p2.addPath(p1);
        QCOMPARE(p2.commandsSize(), 8);

        QCanvasPath p3;
        QTransform tIdentity;
        p3.addPath(p1, tIdentity);
        QCOMPARE(p1, p3);

        QCanvasPath p4;
        QTransform t2;
        t2.rotate(20);
        p4.addPath(p1, t2);
        QCOMPARE_NE(p1, p4);
    }

    {
        // addPath, parts
        QCanvasPath p2;
        p2.moveTo(1, 1);
        p2.lineTo(2, 1);
        p2.lineTo(3, 1);
        p2.lineTo(4, 1);
        QCanvasPath p3;
        p3.moveTo(1, 1);
        p3.lineTo(2, 1);
        p3.lineTo(3, 1);
        //p3.lineTo(4, 1); // Not adding this
        QCanvasPath p4;
        //p4.moveTo(1, 1); // Not adding this
        p4.lineTo(2, 1);
        p4.lineTo(3, 1);
        //p4.lineTo(4, 1); // Not adding this

        QCOMPARE_NE(p2, p3);
        QCanvasPath p5;
        p5.addPath(p2, 0, p2.commandsSize());
        QCOMPARE(p2, p5); // 1, 2, 3, 4
        QCanvasPath p6;
        p6.addPath(p2, 0, p2.commandsSize() - 1);
        QCOMPARE(p3, p6); // 1, 2, 3
        QCanvasPath p7;
        p7.addPath(p2, 1, 2);
        QCOMPARE(p4, p7); // 2, 3
    }
}

void tst_QCanvasPath::testSlicing()
{
    QCanvasPath p1;
    p1.moveTo(1, 1);
    p1.lineTo(2, 1);
    p1.lineTo(3, 1);
    p1.lineTo(4, 1);
    QCOMPARE(p1.commandsSize(), 4);
    QCOMPARE(p1.commandsDataSize(), 8);
    QCanvasPath p2 = p1.sliced(0, p1.commandsSize());
    QCOMPARE(p2.commandsSize(), 4);
    QCOMPARE(p2.commandsDataSize(), 8);
    QCOMPARE(p1, p2);
    QCanvasPath p3 = p1.sliced(0, p1.commandsSize() - 1);
    QCOMPARE(p3.commandsSize(), 3);
    QCOMPARE(p3.commandsDataSize(), 6);
    QCanvasPath p4 = p1.sliced(1, p1.commandsSize() - 1);
    QCOMPARE(p4.commandsSize(), 3);
    QCOMPARE(p4.commandsDataSize(), 6);
    QCanvasPath p5 = p1.sliced(1, p1.commandsSize() - 2);
    QCOMPARE(p5.commandsSize(), 2);
    QCOMPARE(p5.commandsDataSize(), 4);
}

QTEST_MAIN(tst_QCanvasPath)
#include <tst_qcpainterpath.moc>
