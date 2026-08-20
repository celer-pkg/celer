// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottiefill_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieFill: public QObject
{
    Q_OBJECT

public:
    tst_QLottieFill();
    ~tst_QLottieFill();

private:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialColor();
    void testStaticeInitialOpacity();
    void testStaticUpdatedColor();
    void testStaticeUpdatedOpacity();

    void testAnimatedInitialColor();
    void testAnimatedInitialOpacity();
    void testAnimatedUpdatedColor();
    void testAnimatedUpdatedOpacity();

    void testName();
    void testType();
    void testActive();
    void testHidden();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieFill *m_fill = nullptr;
};

tst_QLottieFill::tst_QLottieFill()
{

}

tst_QLottieFill::~tst_QLottieFill()
{

}

void tst_QLottieFill::initTestCase()
{
}

void tst_QLottieFill::cleanupTestCase()
{
    if (m_fill)
        delete m_fill;
}

void tst_QLottieFill::testStaticInitialColor()
{
    loadTestData("fill_static_red_100.json");

    QVERIFY(m_fill->color() == QColor(Qt::red));
}

void tst_QLottieFill:: testStaticeInitialOpacity()
{
    loadTestData("fill_static_red_100.json");

    QVERIFY(qFuzzyCompare(m_fill->opacity(), 100.0));
}

void tst_QLottieFill::testStaticUpdatedColor()
{
    loadTestData("fill_static_red_100.json");
    updateProperty(179);

    QVERIFY(m_fill->color() == QColor(Qt::red));
}

void tst_QLottieFill:: testStaticeUpdatedOpacity()
{
    loadTestData("fill_static_red_100.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_fill->opacity(), 100.0));
}

void tst_QLottieFill::testAnimatedInitialColor()
{
    loadTestData("fill_animated_red100_green0.json");
    updateProperty(0);

    QVERIFY(m_fill->color() == QColor(Qt::red));
}

void tst_QLottieFill::testAnimatedInitialOpacity()
{
    loadTestData("fill_animated_red100_green0.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_fill->opacity(), 100.0));
}

void tst_QLottieFill::testAnimatedUpdatedColor()
{
    loadTestData("fill_animated_red100_green0.json");
    updateProperty(179);

    QVERIFY(m_fill->color() == QColor(Qt::green));
}

void tst_QLottieFill::testAnimatedUpdatedOpacity()
{
    loadTestData("fill_animated_red100_green0.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_fill->opacity(), 0.0));
}

void tst_QLottieFill::testName()
{
    loadTestData("fill_static_red_100.json");
    QVERIFY(m_fill->name() == QString("Fill 1"));
}

void tst_QLottieFill::testType()
{
    loadTestData("fill_static_red_100.json");
    QVERIFY(m_fill->type() == LOTTIE_SHAPE_FILL_IX);
}

void tst_QLottieFill::testActive()
{
    loadTestData("fill_static_red_100.json");
    QVERIFY(m_fill->active(100) == true);

    loadTestData("fill_hidden.json");
    QVERIFY(m_fill->active(100) == false);
}

void tst_QLottieFill::testHidden()
{
    loadTestData("fill_hidden.json");
    QVERIFY(m_fill->hidden() == true);
}

void tst_QLottieFill::loadTestData(const QString &filename)
{
    if (m_fill) {
        delete m_fill;
        m_fill = nullptr;
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
        if (shape->type() == LOTTIE_SHAPE_FILL_IX)
            break;
        shapesIt++;
    }

    m_fill = static_cast<QLottieFill*>(shape);

    QVERIFY(m_fill != nullptr);
}

void tst_QLottieFill::updateProperty(int frame)
{
    m_fill->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieFill)
#include "tst_lottiefill.moc"
