// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottierepeater_p.h"
#include "private/qlottierepeatertransform_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieRepeaterTransform: public QObject
{
    Q_OBJECT

public:
    tst_QLottieRepeaterTransform();
    ~tst_QLottieRepeaterTransform();

private:

    //    void testParseStaticRect();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStaticInitialAnchorX();
    void testStaticInitialAnchorY();
    void testStaticInitialPositionX();
    void testStaticInitialPositionY();
    void testStaticInitialScaleX();
    void testStaticInitialScaleY();
    void testStaticInitialRotation();
    void testStaticInitialStartOpacity();
    void testStaticInitialEndOpacity();
    void testStaticUpdatedAnchorX();
    void testStaticUpdatedAnchorY();
    void testStaticUpdatedPositionX();
    void testStaticUpdatedPositionY();
    void testStaticUpdatedScaleX();
    void testStaticUpdatedScaleY();
    void testStaticUpdatedRotation();
    void testStaticUpdatedStartOpacity();
    void testStaticUpdatedEndOpacity();

    void testAnimatedInitialAnchorX();
    void testAnimatedInitialAnchorY();
    void testAnimatedInitialPositionX();
    void testAnimatedInitialPositionY();
    void testAnimatedInitialScaleX();
    void testAnimatedInitialScaleY();
    void testAnimatedInitialRotation();
    void testAnimatedInitialStartOpacity();
    void testAnimatedInitialEndOpacity();
    void testAnimatedUpdatedAnchorX();
    void testAnimatedUpdatedAnchorY();
    void testAnimatedUpdatedPositionX();
    void testAnimatedUpdatedPositionY();
    void testAnimatedUpdatedScaleX();
    void testAnimatedUpdatedScaleY();
    void testAnimatedUpdatedRotation();
    void testAnimatedUpdatedStartOpacity();
    void testAnimatedUpdatedEndOpacity();

    void testActive();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieRepeaterTransform *m_transform = nullptr;
};

tst_QLottieRepeaterTransform::tst_QLottieRepeaterTransform()
{

}

tst_QLottieRepeaterTransform::~tst_QLottieRepeaterTransform()
{

}

void tst_QLottieRepeaterTransform::initTestCase()
{
}

void tst_QLottieRepeaterTransform::cleanupTestCase()
{
    if (m_transform)
        delete m_transform;
}

void tst_QLottieRepeaterTransform::testStaticInitialAnchorX()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialAnchorY()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialPositionX()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialPositionY()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialScaleX()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieRepeaterTransform::testStaticInitialScaleY()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieRepeaterTransform::testStaticInitialRotation()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialStartOpacity()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->startOpacity(), 100.0));
}

void tst_QLottieRepeaterTransform::testStaticInitialEndOpacity()
{
    loadTestData("repeater_transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->endOpacity(), 100.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedAnchorX()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedAnchorY()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedPositionX()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedPositionY()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedScaleX()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedScaleY()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedRotation()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedStartOpacity()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->startOpacity(), 100.0));
}

void tst_QLottieRepeaterTransform::testStaticUpdatedEndOpacity()
{
    loadTestData("repeater_transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->endOpacity(), 100.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialAnchorX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialAnchorY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialPositionX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialPositionY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialScaleX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialScaleY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialRotation()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialStartOpacity()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->startOpacity(), 0.0));
}

void tst_QLottieRepeaterTransform::testAnimatedInitialEndOpacity()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->endOpacity(), 100.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedAnchorX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 25.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedAnchorY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 25.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedPositionX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 75.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedPositionY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 75.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedScaleX()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 1.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedScaleY()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 1.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedRotation()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), (3 * 360 + 30.0)));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedStartOpacity()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->startOpacity(), 25.0));
}

void tst_QLottieRepeaterTransform::testAnimatedUpdatedEndOpacity()
{
    loadTestData("repeater_transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->endOpacity(), 75.0));
}

void tst_QLottieRepeaterTransform::testActive()
{
    loadTestData("repeater_transform_static.json");
    QVERIFY(m_transform->active(100) == true);
}

void tst_QLottieRepeaterTransform::loadTestData(const QString &filename)
{
    if (m_transform) {
        delete m_transform;
        m_transform = nullptr;
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
        if (shape->type() == LOTTIE_SHAPE_REPEATER_IX)
            break;
        shapesIt++;
    }

    QLottieRepeater *repeater = static_cast<QLottieRepeater*>(shape);
    m_transform = static_cast<QLottieRepeaterTransform*>(repeater->transform().clone());

    QVERIFY(m_transform != nullptr);
}

void tst_QLottieRepeaterTransform::updateProperty(int frame)
{
    m_transform->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieRepeaterTransform)
#include "tst_lottierepeatertransform.moc"
