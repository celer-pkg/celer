// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottierect_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieRect: public QObject
{
    Q_OBJECT

public:
    tst_QLottieRect();
    ~tst_QLottieRect();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialX();
    void testStaticInitialY();
    void testStaticInitialWidth();
    void testStaticInitialHeight();
    void testStaticInitialRoundness();
    void testStaticUpdatedX();
    void testStaticUpdatedY();
    void testStaticUpdatedWidth();
    void testStaticUpdatedHeight();
    void testStaticUpdatedRoundness();

    void testAnimatedInitialX();
    void testAnimatedInitialY();
    void testAnimatedInitialWidth();
    void testAnimatedInitialHeight();
    void testAnimatedInitialRoundness();
    void testAnimatedUpdatedX();
    void testAnimatedUpdatedY();
    void testAnimatedUpdatedWidth();
    void testAnimatedUpdatedHeight();
    void testAnimatedUpdatedRoundness();

    void testName();
    void testType();
    void testHidden();
    void testActive();
    void testDirection();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieRect *m_rect = nullptr;
};

tst_QLottieRect::tst_QLottieRect()
{

}

tst_QLottieRect::~tst_QLottieRect()
{

}

void tst_QLottieRect::initTestCase()
{
}

void tst_QLottieRect::cleanupTestCase()
{
    if (m_rect)
        delete m_rect;
}

void tst_QLottieRect::testStaticInitialX()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 50.0));
}

void tst_QLottieRect:: testStaticInitialY()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 50.0));
}

void tst_QLottieRect::testStaticInitialWidth()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_QLottieRect::testStaticInitialHeight()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_QLottieRect:: testStaticInitialRoundness()
{
    loadTestData("rect_static_30x30_5050_rad0.json");

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_QLottieRect::testStaticUpdatedX()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 50.0));
}

void tst_QLottieRect:: testStaticUpdatedY()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 50.0));
}

void tst_QLottieRect::testStaticUpdatedWidth()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_QLottieRect::testStaticUpdatedHeight()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_QLottieRect:: testStaticUpdatedRoundness()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_QLottieRect::testAnimatedInitialX()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 15.0));
}

void tst_QLottieRect:: testAnimatedInitialY()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 15.0));
}

void tst_QLottieRect::testAnimatedInitialWidth()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 30.0));
}

void tst_QLottieRect::testAnimatedInitialHeight()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 30.0));
}

void tst_QLottieRect::testAnimatedInitialRoundness()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 0.0));
}

void tst_QLottieRect::testAnimatedUpdatedX()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().x(), 75.0));
}

void tst_QLottieRect:: testAnimatedUpdatedY()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->position().y(), 75.0));
}

void tst_QLottieRect::testAnimatedUpdatedWidth()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().width(), 50.0));
}

void tst_QLottieRect::testAnimatedUpdatedHeight()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->size().height(), 50.0));
}

void tst_QLottieRect::testAnimatedUpdatedRoundness()
{
    loadTestData("rect_animated_30x30_1515_rad0_to_50x50_7575_rad25.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_rect->roundness(), 25.0));
}

void tst_QLottieRect::testName()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->name() == QString("Rectangle Path 1"));
}

void tst_QLottieRect::testType()
{
    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->type() == LOTTIE_SHAPE_RECT_IX);
}

void tst_QLottieRect::testHidden()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->hidden() == true);
}

void tst_QLottieRect::testActive()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->active(100) == false);

    loadTestData("rect_static_30x30_5050_rad0.json");
    QVERIFY(m_rect->active(100) == true);
}

void tst_QLottieRect::testDirection()
{
    loadTestData("rect_hidden.json");
    QVERIFY(m_rect->direction() == 0);

    loadTestData("rect_direction.json");
    QVERIFY(m_rect->direction() == 3);
}

void tst_QLottieRect::loadTestData(const QString &filename)
{
    if (m_rect) {
        delete m_rect;
        m_rect = nullptr;
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
        if (shape->type() == LOTTIE_SHAPE_RECT_IX)
            break;
        shapesIt++;
    }

    m_rect = static_cast<QLottieRect*>(shape);

    QVERIFY(m_rect != nullptr);
}

void tst_QLottieRect::updateProperty(int frame)
{
    m_rect->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieRect)
#include "tst_lottierect.moc"
