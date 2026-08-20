// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottiefreeformshape_p.h"

using namespace Qt::StringLiterals;

class tst_LottiePath: public QObject
{
    Q_OBJECT

public:
    tst_LottiePath();
    ~tst_LottiePath();

private:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticCurveInitialStartPoint();
    void testStaticCurveInitialSecondPoint();
    void testStaticCurveInitialThirdPoint();
    void testStaticCurveInitialFourthPoint();
    void testStaticCurveInitialFifthPoint();
    void testStaticCurveInitialSixthPoint();
    void testStaticCurveUpdatedStartPoint();
    void testStaticCurveUpdatedSecondPoint();
    void testStaticCurveUpdatedThirdPoint();
    void testStaticCurveUpdatedFourthPoint();
    void testStaticCurveUpdatedFifthPoint();
    void testStaticCurveUpdatedSixthPoint();

    void testStaticTriangleInitialStartPoint();
    void testStaticTriangleInitialSecondPoint();
    void testStaticTriangleInitialThirdPoint();
    void testStaticTriangleInitialClosed();
    void testStaticTriangleUpdatedStartPoint();
    void testStaticTriangleUpdatedSecondPoint();
    void testStaticTriangleUpdatedThirdPoint();
    void testStaticTriangleUpdatedClosed();

    void testStaticRotoInitialStartPoint();
    void testStaticRotoInitialSecondPoint();
    void testStaticRotoInitialThirdPoint();
    void testStaticRotoInitialFourthPoint();
    void testStaticRotoInitialFifthPoint();
    void testStaticRotoInitialSixthPoint();
    void testStaticRotoInitialSeventhPoint();
    void testStaticRotoInitialEighthPoint();
    void testStaticRotoInitialNinthPoint();
    void testStaticRotoInitialTenthPoint();
    void testStaticRotoInitialEleventhPoint();
    void testStaticRotoInitialTwelvthPoint();
    void testStaticRotoInitialClosed();
    void testStaticRotoUpdatedStartPoint();
    void testStaticRotoUpdatedSecondPoint();
    void testStaticRotoUpdatedThirdPoint();
    void testStaticRotoUpdatedFourthPoint();
    void testStaticRotoUpdatedFifthPoint();
    void testStaticRotoUpdatedSixthPoint();
    void testStaticRotoUpdatedSeventhPoint();
    void testStaticRotoUpdatedEighthPoint();
    void testStaticRotoUpdatedNinthPoint();
    void testStaticRotoUpdatedTenthPoint();
    void testStaticRotoUpdatedEleventhPoint();
    void testStaticRotoUpdatedTwelvthPoint();
    void testStaticRotoUpdatedClosed();

    void testAnimatedCurveInitialStartPoint();
    void testAnimatedCurveInitialSecondPoint();
    void testAnimatedCurveInitialThirdPoint();
    void testAnimatedCurveInitialFourthPoint();
    void testAnimatedCurveInitialFifthPoint();
    void testAnimatedCurveInitialSixthPoint();
    void testAnimatedCurveUpdatedStartPoint();
    void testAnimatedCurveUpdatedSecondPoint();
    void testAnimatedCurveUpdatedThirdPoint();
    void testAnimatedCurveUpdatedFourthPoint();
    void testAnimatedCurveUpdatedFifthPoint();
    void testAnimatedCurveUpdatedSixthPoint();

    void testAnimatedTriangleInitialStartPoint();
    void testAnimatedTriangleInitialSecondPoint();
    void testAnimatedTriangleInitialThirdPoint();
    void testAnimatedTriangleInitialClosed();
    void testAnimatedTriangleUpdatedStartPoint();
    void testAnimatedTriangleUpdatedSecondPoint();
    void testAnimatedTriangleUpdatedThirdPoint();
    void testAnimatedTriangleUpdatedClosed();

    void testAnimatedRotoInitialStartPoint();
    void testAnimatedRotoInitialSecondPoint();
    void testAnimatedRotoInitialThirdPoint();
    void testAnimatedRotoInitialFourthPoint();
    void testAnimatedRotoInitialFifthPoint();
    void testAnimatedRotoInitialSixthPoint();
    void testAnimatedRotoInitialSeventhPoint();
    void testAnimatedRotoInitialEighthPoint();
    void testAnimatedRotoInitialNinthPoint();
    void testAnimatedRotoInitialTenthPoint();
    void testAnimatedRotoInitialEleventhPoint();
    void testAnimatedRotoInitialTwelvthPoint();
    void testAnimatedRotoInitialClosed();
    void testAnimatedRotoUpdatedStartPoint();
    void testAnimatedRotoUpdatedSecondPoint();
    void testAnimatedRotoUpdatedThirdPoint();
    void testAnimatedRotoUpdatedFourthPoint();
    void testAnimatedRotoUpdatedFifthPoint();
    void testAnimatedRotoUpdatedSixthPoint();
    void testAnimatedRotoUpdatedSeventhPoint();
    void testAnimatedRotoUpdatedEighthPoint();
    void testAnimatedRotoUpdatedNinthPoint();
    void testAnimatedRotoUpdatedTenthPoint();
    void testAnimatedRotoUpdatedEleventhPoint();
    void testAnimatedRotoUpdatedTwelvthPoint();
    void testAnimatedRotoUpdatedClosed();

    void testName();
    void testType();
    void testActive();
    void testHidden();
    void testDirection();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieFreeFormShape *m_path = nullptr;
};

tst_LottiePath::tst_LottiePath()
{

}

tst_LottiePath::~tst_LottiePath()
{

}

void tst_LottiePath::initTestCase()
{
}

void tst_LottiePath::cleanupTestCase()
{
    if (m_path)
        delete m_path;
}

void tst_LottiePath::testStaticCurveInitialStartPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 15);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticCurveInitialSecondPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 28);
    QVERIFY(qRound(el.y) == 85);
}

void tst_LottiePath::testStaticCurveInitialThirdPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticCurveInitialFourthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 51);
    QVERIFY(qRound(el.y) == 38);
}

void tst_LottiePath::testStaticCurveInitialFifthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 90);
    QVERIFY(qRound(el.y) == 25);
}

void tst_LottiePath::testStaticCurveInitialSixthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 85);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticCurveUpdatedStartPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 15);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticCurveUpdatedSecondPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 28);
    QVERIFY(qRound(el.y) == 85);
}

void tst_LottiePath::testStaticCurveUpdatedThirdPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticCurveUpdatedFourthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 51);
    QVERIFY(qRound(el.y) == 38);
}

void tst_LottiePath::testStaticCurveUpdatedFifthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 90);
    QVERIFY(qRound(el.y) == 25);
}

void tst_LottiePath::testStaticCurveUpdatedSixthPoint()
{
    loadTestData("freeform_curve_static.json");

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 85);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticTriangleInitialStartPoint()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 50);
    QVERIFY(qRound(el.y) == 30);
}

void tst_LottiePath::testStaticTriangleInitialSecondPoint()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticTriangleInitialThirdPoint()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticTriangleInitialClosed()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testStaticTriangleUpdatedStartPoint()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 50);
    QVERIFY(qRound(el.y) == 30);
}

void tst_LottiePath::testStaticTriangleUpdatedSecondPoint()
{
    loadTestData("freeform_triangle_static.json");

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticTriangleUpdatedThirdPoint()
{
    loadTestData("freeform_triangle_static.json");
    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testStaticTriangleUpdatedClosed()
{
    loadTestData("freeform_triangle_static.json");
    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testStaticRotoInitialStartPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testStaticRotoInitialSecondPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(1);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testStaticRotoInitialThirdPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testStaticRotoInitialFourthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testStaticRotoInitialFifthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testStaticRotoInitialSixthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testStaticRotoInitialSeventhPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testStaticRotoInitialEighthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(7);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testStaticRotoInitialNinthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(8);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testStaticRotoInitialTenthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(9);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testStaticRotoInitialEleventhPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(10);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testStaticRotoInitialTwelvthPoint()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el = m_path->path().elementAt(11);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testStaticRotoInitialClosed()
{
    loadTestData("freeform_roto_static.json");

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testStaticRotoUpdatedStartPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testStaticRotoUpdatedSecondPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(1);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testStaticRotoUpdatedThirdPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testStaticRotoUpdatedFourthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testStaticRotoUpdatedFifthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testStaticRotoUpdatedSixthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testStaticRotoUpdatedSeventhPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testStaticRotoUpdatedEighthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(7);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testStaticRotoUpdatedNinthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(8);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testStaticRotoUpdatedTenthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(9);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testStaticRotoUpdatedEleventhPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(10);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testStaticRotoUpdatedTwelvthPoint()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(11);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testStaticRotoUpdatedClosed()
{
    loadTestData("freeform_roto_static.json");
    updateProperty(179);

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testAnimatedCurveInitialStartPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 15);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedCurveInitialSecondPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 28);
    QVERIFY(qRound(el.y) == 85);
}

void tst_LottiePath::testAnimatedCurveInitialThirdPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedCurveInitialFourthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 51);
    QVERIFY(qRound(el.y) == 38);
}

void tst_LottiePath::testAnimatedCurveInitialFifthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 90);
    QVERIFY(qRound(el.y) == 25);
}

void tst_LottiePath::testAnimatedCurveInitialSixthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 85);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedCurveUpdatedStartPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 15);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedCurveUpdatedSecondPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 28);
    QVERIFY(qRound(el.y) == 5);
}

void tst_LottiePath::testAnimatedCurveUpdatedThirdPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedCurveUpdatedFourthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 90);
}

void tst_LottiePath::testAnimatedCurveUpdatedFifthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 12);
}

void tst_LottiePath::testAnimatedCurveUpdatedSixthPoint()
{
    loadTestData("freeform_curve_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 85);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedTriangleInitialStartPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 50);
    QVERIFY(qRound(el.y) == 30);
}

void tst_LottiePath::testAnimatedTriangleInitialSecondPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedTriangleInitialThirdPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedTriangleInitialClosed()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(0);

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testAnimatedTriangleUpdatedStartPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 50);
    QVERIFY(qRound(el.y) == 79);
}

void tst_LottiePath::testAnimatedTriangleUpdatedSecondPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedTriangleUpdatedThirdPoint()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedTriangleUpdatedClosed()
{
    loadTestData("freeform_triangle_animated.json");
    updateProperty(179);

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testAnimatedRotoInitialStartPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedRotoInitialSecondPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(1);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testAnimatedRotoInitialThirdPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testAnimatedRotoInitialFourthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedRotoInitialFifthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testAnimatedRotoInitialSixthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testAnimatedRotoInitialSeventhPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testAnimatedRotoInitialEighthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(7);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testAnimatedRotoInitialNinthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(8);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 35);
}

void tst_LottiePath::testAnimatedRotoInitialTenthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(9);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testAnimatedRotoInitialEleventhPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(10);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testAnimatedRotoInitialTwelvthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el = m_path->path().elementAt(11);
    QVERIFY(qRound(el.x) == 35);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testAnimatedRotoInitialClosed()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(0);

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testAnimatedRotoUpdatedStartPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(0);
    QVERIFY(qRound(el.x) == 40);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedRotoUpdatedSecondPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(1);
    QVERIFY(qRound(el.x) == 45);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testAnimatedRotoUpdatedThirdPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(2);
    QVERIFY(qRound(el.x) == 55);
    QVERIFY(qRound(el.y) == 65);
}

void tst_LottiePath::testAnimatedRotoUpdatedFourthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(3);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 60);
}

void tst_LottiePath::testAnimatedRotoUpdatedFifthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(4);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 55);
}

void tst_LottiePath::testAnimatedRotoUpdatedSixthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(5);
    QVERIFY(qRound(el.x) == 65);
    QVERIFY(qRound(el.y) == 45);
}

void tst_LottiePath::testAnimatedRotoUpdatedSeventhPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(6);
    QVERIFY(qRound(el.x) == 60);
    QVERIFY(qRound(el.y) == 40);
}

void tst_LottiePath::testAnimatedRotoUpdatedEighthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(7);
    QVERIFY(qRound(el.x) == 57);
    QVERIFY(qRound(el.y) == 37);
}

void tst_LottiePath::testAnimatedRotoUpdatedNinthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(8);
    QVERIFY(qRound(el.x) == 53);
    QVERIFY(qRound(el.y) == 47);
}

void tst_LottiePath::testAnimatedRotoUpdatedTenthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(9);
    QVERIFY(qRound(el.x) == 50);
    QVERIFY(qRound(el.y) == 50);
}

void tst_LottiePath::testAnimatedRotoUpdatedEleventhPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(10);
    QVERIFY(qRound(el.x) == 47);
    QVERIFY(qRound(el.y) == 53);
}

void tst_LottiePath::testAnimatedRotoUpdatedTwelvthPoint()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el = m_path->path().elementAt(11);
    QVERIFY(qRound(el.x) == 37);
    QVERIFY(qRound(el.y) == 57);
}

void tst_LottiePath::testAnimatedRotoUpdatedClosed()
{
    loadTestData("freeform_roto_animated.json");
    updateProperty(179);

    QPainterPath::Element el1 = m_path->path().elementAt(0);
    QPainterPath::Element el2 = m_path->path().elementAt(m_path->path().elementCount()-1);
    QVERIFY(qFuzzyCompare(el1.x, el2.x));
    QVERIFY(qFuzzyCompare(el1.y, el2.y));
}

void tst_LottiePath::testName()
{
    loadTestData("freeform_curve_static.json");
    QVERIFY(m_path->name() == QString("Path 1"));
}

void tst_LottiePath::testType()
{
    loadTestData("freeform_curve_static.json");
    QVERIFY(m_path->type() == LOTTIE_SHAPE_SHAPE_IX);
}

void tst_LottiePath::testActive()
{
    loadTestData("freeform_curve_static.json");
    QVERIFY(m_path->active(100) == true);

    loadTestData("freeform_hidden.json");
    QVERIFY(m_path->active(100) == false);
}

void tst_LottiePath::testHidden()
{
    loadTestData("freeform_hidden.json");
    QVERIFY(m_path->hidden() == true);
}

void tst_LottiePath::testDirection()
{
    loadTestData("freeform_hidden.json");
    QVERIFY(m_path->direction() == 0);
    loadTestData("freeform_direction.json");
    QVERIFY(m_path->direction() == 0);
}

void tst_LottiePath::loadTestData(const QString &filename)
{
    if (m_path) {
        delete m_path;
        m_path = nullptr;
    }

    QFile sourceFile(QFINDTESTDATA(QLatin1String("data/") + filename));
    if (!sourceFile.exists())
        QFAIL("File does not exist");
    if (!sourceFile.open(QIODevice::ReadOnly))
        QFAIL("Cannot read test file");

    QByteArray json = sourceFile.readAll();

    sourceFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject rootObj = doc.object();
    if (rootObj.empty())
        QFAIL("Cannot parse test file");

    QJsonArray layers = rootObj.value(QLatin1String("layers")).toArray();
    QJsonObject layerObj = layers[0].toObject();
    int type = layerObj.value(QLatin1String("ty")).toInt();
    if (type != 4)
        QFAIL("It's not shape layer");

    QJsonArray shapes = layerObj.value(QLatin1String("shapes")).toArray();
    QJsonArray::const_iterator shapesIt = shapes.constBegin();
    QLottieShape* shape = nullptr;
    while (shapesIt != shapes.end()) {
        QJsonObject childObj = (*shapesIt).toObject();
        shape = QLottieShape::construct(childObj);
        QVERIFY(shape != nullptr);
        if (shape->type() == LOTTIE_SHAPE_SHAPE_IX)
            break;
        shapesIt++;
    }

    m_path = static_cast<QLottieFreeFormShape*>(shape);

    QVERIFY(m_path != nullptr);
}

void tst_LottiePath::updateProperty(int frame)
{
    m_path->updateProperties(frame);
}

QTEST_MAIN(tst_LottiePath)
#include "tst_lottiepath.moc"
