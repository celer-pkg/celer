// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottiebasictransform_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieBasicTransform: public QObject
{
    Q_OBJECT

public:
    tst_QLottieBasicTransform();
    ~tst_QLottieBasicTransform();

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
    void testStaticInitialOpacity();
    void testStaticUpdatedAnchorX();
    void testStaticUpdatedAnchorY();
    void testStaticUpdatedPositionX();
    void testStaticUpdatedPositionY();
    void testStaticUpdatedScaleX();
    void testStaticUpdatedScaleY();
    void testStaticUpdatedRotation();
    void testStaticUpdatedOpacity();

    void testAnimatedInitialAnchorX();
    void testAnimatedInitialAnchorY();
    void testAnimatedInitialPositionX();
    void testAnimatedInitialPositionY();
    void testAnimatedInitialScaleX();
    void testAnimatedInitialScaleY();
    void testAnimatedInitialRotation();
    void testAnimatedInitialOpacity();
    void testAnimatedUpdatedAnchorX();
    void testAnimatedUpdatedAnchorY();
    void testAnimatedUpdatedPositionX();
    void testAnimatedUpdatedPositionY();
    void testAnimatedUpdatedScaleX();
    void testAnimatedUpdatedScaleY();
    void testAnimatedUpdatedRotation();
    void testAnimatedUpdatedOpacity();

    void testActive();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieBasicTransform *m_transform = nullptr;
};

tst_QLottieBasicTransform::tst_QLottieBasicTransform()
{

}

tst_QLottieBasicTransform::~tst_QLottieBasicTransform()
{

}

void tst_QLottieBasicTransform::initTestCase()
{
}

void tst_QLottieBasicTransform::cleanupTestCase()
{
    if (m_transform)
        delete m_transform;
}

void tst_QLottieBasicTransform::testStaticInitialAnchorX()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieBasicTransform::testStaticInitialAnchorY()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieBasicTransform::testStaticInitialPositionX()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieBasicTransform::testStaticInitialPositionY()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieBasicTransform::testStaticInitialScaleX()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieBasicTransform::testStaticInitialScaleY()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieBasicTransform::testStaticInitialRotation()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieBasicTransform::testStaticInitialOpacity()
{
    loadTestData("transform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedAnchorX()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedAnchorY()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedPositionX()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedPositionY()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedScaleX()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieBasicTransform::testStaticUpdatedScaleY()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieBasicTransform::testStaticUpdatedRotation()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieBasicTransform::testStaticUpdatedOpacity()
{
    loadTestData("transform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialAnchorX()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialAnchorY()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialPositionX()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialPositionY()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialScaleX()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieBasicTransform::testAnimatedInitialScaleY()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieBasicTransform::testAnimatedInitialRotation()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieBasicTransform::testAnimatedInitialOpacity()
{
    loadTestData("transform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedAnchorX()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 25.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedAnchorY()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 25.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedPositionX()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 75.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedPositionY()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 75.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedScaleX()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 1.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedScaleY()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 1.0));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedRotation()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), (3 * 360 + 30.0)));
}

void tst_QLottieBasicTransform::testAnimatedUpdatedOpacity()
{
    loadTestData("transform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 0.25));
}

void tst_QLottieBasicTransform::testActive()
{
    loadTestData("transform_static.json");
    QVERIFY(m_transform->active(100) == true);
}

void tst_QLottieBasicTransform::loadTestData(const QString &filename)
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

    QJsonObject transformObj = layerObj.value(QLatin1String("ks")).toObject();
    m_transform = new QLottieBasicTransform();
    int ret = m_transform->parse(transformObj);

    QVERIFY(m_transform != nullptr);
    QVERIFY(ret >= 0);
}

void tst_QLottieBasicTransform::updateProperty(int frame)
{
    m_transform->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieBasicTransform)
#include "tst_lottiebasictransform.moc"
