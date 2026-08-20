// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottieellipse_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieEllipse: public QObject
{
    Q_OBJECT

public:
    tst_QLottieEllipse();
    ~tst_QLottieEllipse();

private:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialX();
    void testStaticInitialY();
    void testStaticInitialWidth();
    void testStaticInitialHeight();
    void testStaticUpdatedX();
    void testStaticUpdatedY();
    void testStaticUpdatedWidth();
    void testStaticUpdatedHeight();

    void testAnimatedInitialX();
    void testAnimatedInitialY();
    void testAnimatedInitialWidth();
    void testAnimatedInitialHeight();
    void testAnimatedUpdatedX();
    void testAnimatedUpdatedY();
    void testAnimatedUpdatedWidth();
    void testAnimatedUpdatedHeight();

    void testName();
    void testType();
    void testActive();
    void testHidden();
    void testDirection();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieEllipse *m_ellipse = nullptr;
};

tst_QLottieEllipse::tst_QLottieEllipse()
{

}

tst_QLottieEllipse::~tst_QLottieEllipse()
{

}

void tst_QLottieEllipse::initTestCase()
{
}

void tst_QLottieEllipse::cleanupTestCase()
{
    if (m_ellipse)
        delete m_ellipse;
}

void tst_QLottieEllipse::testStaticInitialX()
{
    loadTestData("ellipse_static_100x80.json");

    QVERIFY(qFuzzyCompare(m_ellipse->position().x(), 0.0));
}

void tst_QLottieEllipse:: testStaticInitialY()
{
    loadTestData("ellipse_static_100x80.json");

    QVERIFY(qFuzzyCompare(m_ellipse->position().y(), 0.0));
}

void tst_QLottieEllipse:: testStaticInitialWidth()
{
    loadTestData("ellipse_static_100x80.json");

    QVERIFY(qFuzzyCompare(m_ellipse->size().width(), 100));
}

void tst_QLottieEllipse:: testStaticInitialHeight()
{
    loadTestData("ellipse_static_100x80.json");

    QVERIFY(qFuzzyCompare(m_ellipse->size().height(), 80));
}

void tst_QLottieEllipse::testStaticUpdatedX()
{
    loadTestData("ellipse_static_100x80.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->position().x(), 0.0));
}

void tst_QLottieEllipse:: testStaticUpdatedY()
{
    loadTestData("ellipse_static_100x80.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->position().y(), 0.0));
}

void tst_QLottieEllipse:: testStaticUpdatedWidth()
{
    loadTestData("ellipse_static_100x80.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->size().width(), 100));
}

void tst_QLottieEllipse:: testStaticUpdatedHeight()
{
    loadTestData("ellipse_static_100x80.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->size().height(), 80));
}

void tst_QLottieEllipse::testAnimatedInitialX()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_ellipse->position().x(), 0.0));
}

void tst_QLottieEllipse::testAnimatedInitialY()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_ellipse->position().y(), 0.0));
}

void tst_QLottieEllipse::testAnimatedInitialWidth()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_ellipse->size().width(), 100.0));
}

void tst_QLottieEllipse::testAnimatedInitialHeight()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_ellipse->size().height(), 80.0));
}

void tst_QLottieEllipse::testAnimatedUpdatedX()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->position().x(), 50.0));
}

void tst_QLottieEllipse::testAnimatedUpdatedY()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->position().y(), 100.0));
}

void tst_QLottieEllipse::testAnimatedUpdatedWidth()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->size().width(), 200.0));
}

void tst_QLottieEllipse::testAnimatedUpdatedHeight()
{
    loadTestData("ellipse_animated_100x80at00to200x40at50100.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_ellipse->size().height(), 40.0));
}

void tst_QLottieEllipse::testName()
{
    loadTestData("ellipse_static_100x80.json");
    QVERIFY(m_ellipse->name() == QString("Ellipse Path 1"));
}

void tst_QLottieEllipse::testType()
{
    loadTestData("ellipse_static_100x80.json");
    QVERIFY(m_ellipse->type() == LOTTIE_SHAPE_ELLIPSE_IX);
}

void tst_QLottieEllipse::testActive()
{
    loadTestData("ellipse_static_100x80.json");
    QVERIFY(m_ellipse->active(100) == true);

    loadTestData("ellipse_hidden.json");
    QVERIFY(m_ellipse->active(100) == false);
}

void tst_QLottieEllipse::testHidden()
{
    loadTestData("ellipse_hidden.json");
    QVERIFY(m_ellipse->hidden() == true);
}

void tst_QLottieEllipse::testDirection()
{
    loadTestData("ellipse_hidden.json");
    QVERIFY(m_ellipse->direction() == 0);
    loadTestData("ellipse_direction.json");
    QVERIFY(m_ellipse->direction() == 0);
}

void tst_QLottieEllipse::loadTestData(const QString &filename)
{
    if (m_ellipse) {
        delete m_ellipse;
        m_ellipse = nullptr;
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
        if (shape->type() == LOTTIE_SHAPE_ELLIPSE_IX)
            break;
        shapesIt++;
    }

    m_ellipse = static_cast<QLottieEllipse*>(shape);

    QVERIFY(m_ellipse != nullptr);
}

void tst_QLottieEllipse::updateProperty(int frame)
{
    m_ellipse->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieEllipse)
#include "tst_lottieellipse.moc"
