// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottiestroke_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieStroke: public QObject
{
    Q_OBJECT

public:
    tst_QLottieStroke();
    ~tst_QLottieStroke();

private:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialColor();
    void testStaticInitialOpacity();
    void testStaticInitialWidth();
    void testStaticInitialCapStyle();
    void testStaticInitialJoinStyle();
    void testStaticInitialMiterLimit();
    void testStaticUpdatedColor();
    void testStaticUpdatedOpacity();
    void testStaticUpdatedWidth();
    void testStaticUpdatedCapStyle();
    void testStaticUpdatedJoinStyle();
    void testStaticUpdatedMiterLimit();

    void testAnimatedInitialColor();
    void testAnimatedInitialOpacity();
    void testAnimatedInitialWidth();
    void testAnimatedInitialCapStyle();
    void testAnimatedInitialJoinStyle();
    void testAnimatedInitialMiterLimit();
    void testAnimatedUpdatedColor();
    void testAnimatedUpdatedOpacity();
    void testAnimatedUpdatedWidth();

    void testName();
    void testType();
    void testActive();
    void testHidden();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieStroke *m_stroke = nullptr;
};

tst_QLottieStroke::tst_QLottieStroke()
{

}

tst_QLottieStroke::~tst_QLottieStroke()
{

}

void tst_QLottieStroke::initTestCase()
{
}

void tst_QLottieStroke::cleanupTestCase()
{
    if (m_stroke)
        delete m_stroke;
}

void tst_QLottieStroke::testStaticInitialColor()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(m_stroke->pen().color() == QColor(Qt::blue));
}

void tst_QLottieStroke::testStaticInitialOpacity()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(qFuzzyCompare(m_stroke->opacity(), 100.0));
}

void tst_QLottieStroke::testStaticInitialWidth()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(qFuzzyCompare(m_stroke->pen().width(), 2.0));
}

void tst_QLottieStroke::testStaticInitialCapStyle()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(m_stroke->pen().capStyle() == Qt::FlatCap);
}

void tst_QLottieStroke::testStaticInitialJoinStyle()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(m_stroke->pen().joinStyle() == Qt::MiterJoin);
}

void tst_QLottieStroke::testStaticInitialMiterLimit()
{
    loadTestData("stroke_static_blue_2.json");

    QVERIFY(qFuzzyCompare(m_stroke->pen().miterLimit(), 4.0));
}

void tst_QLottieStroke::testStaticUpdatedColor()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(m_stroke->pen().color() == QColor(Qt::blue));
}

void tst_QLottieStroke::testStaticUpdatedOpacity()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_stroke->opacity(), 100.0));
}

void tst_QLottieStroke::testStaticUpdatedWidth()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_stroke->pen().width(), 2.0));
}

void tst_QLottieStroke::testStaticUpdatedCapStyle()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(m_stroke->pen().capStyle() == Qt::FlatCap);
}

void tst_QLottieStroke::testStaticUpdatedJoinStyle()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(m_stroke->pen().joinStyle() == Qt::MiterJoin);
}

void tst_QLottieStroke::testStaticUpdatedMiterLimit()
{
    loadTestData("stroke_static_blue_2.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_stroke->pen().miterLimit(), 4.0));
}

void tst_QLottieStroke::testAnimatedInitialColor()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(m_stroke->pen().color() == QColor(Qt::blue));
}

void tst_QLottieStroke::testAnimatedInitialOpacity()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_stroke->opacity(), 100.0));
}

void tst_QLottieStroke::testAnimatedInitialWidth()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_stroke->pen().width(), 5.0));
}

void tst_QLottieStroke::testAnimatedInitialCapStyle()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(m_stroke->pen().capStyle() == Qt::FlatCap);
}

void tst_QLottieStroke::testAnimatedInitialJoinStyle()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(m_stroke->pen().joinStyle() == Qt::MiterJoin);
}

void tst_QLottieStroke::testAnimatedInitialMiterLimit()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_stroke->pen().miterLimit(), 4.0));
}

void tst_QLottieStroke::testAnimatedUpdatedColor()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(179);

    QVERIFY(m_stroke->pen().color() == QColor(Qt::white));
}

void tst_QLottieStroke::testAnimatedUpdatedOpacity()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_stroke->opacity(), 10.0));
}

void tst_QLottieStroke::testAnimatedUpdatedWidth()
{
    loadTestData("stroke_animated_blue5_white1.json");
    updateProperty(179);

    QVERIFY(qFuzzyCompare(m_stroke->pen().width(), 1.0));
}

void tst_QLottieStroke::testName()
{
    loadTestData("stroke_static_blue_2.json");
    QVERIFY(m_stroke->name() == QString("Stroke 1"));
}

void tst_QLottieStroke::testType()
{
    loadTestData("stroke_static_blue_2.json");
    QVERIFY(m_stroke->type() == LOTTIE_SHAPE_STROKE_IX);
}

void tst_QLottieStroke::testActive()
{
    loadTestData("stroke_static_blue_2.json");
    QVERIFY(m_stroke->active(100) == true);

    loadTestData("stroke_hidden.json");
    QVERIFY(m_stroke->active(100) == false);
}

void tst_QLottieStroke::testHidden()
{
    loadTestData("stroke_hidden.json");
    QVERIFY(m_stroke->hidden() == true);
}

void tst_QLottieStroke::loadTestData(const QString &filename)
{
    if (m_stroke) {
        delete m_stroke;
        m_stroke = nullptr;
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
        if (shape->type() == LOTTIE_SHAPE_STROKE_IX)
            break;
        shapesIt++;
    }

    m_stroke = static_cast<QLottieStroke*>(shape);

    QVERIFY(m_stroke != nullptr);
}

void tst_QLottieStroke::updateProperty(int frame)
{
    m_stroke->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieStroke)
#include "tst_lottiestroke.moc"
