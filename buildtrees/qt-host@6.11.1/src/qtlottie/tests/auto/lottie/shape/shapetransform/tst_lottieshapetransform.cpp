// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottiegroup_p.h"
#include "private/qlottieshapetransform_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieShapeTransform: public QObject
{
    Q_OBJECT

public:
    tst_QLottieShapeTransform();
    ~tst_QLottieShapeTransform();

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
    void testStaticInitialSkew();
    void testStaticInitialSkewAxis();
    void testStaticUpdatedAnchorX();
    void testStaticUpdatedAnchorY();
    void testStaticUpdatedPositionX();
    void testStaticUpdatedPositionY();
    void testStaticUpdatedScaleX();
    void testStaticUpdatedScaleY();
    void testStaticUpdatedRotation();
    void testStaticUpdatedOpacity();
    void testStaticUpdatedSkew();
    void testStaticUpdatedSkewAxis();


    void testAnimatedInitialAnchorX();
    void testAnimatedInitialAnchorY();
    void testAnimatedInitialPositionX();
    void testAnimatedInitialPositionY();
    void testAnimatedInitialScaleX();
    void testAnimatedInitialScaleY();
    void testAnimatedInitialRotation();
    void testAnimatedInitialOpacity();
    void testAnimatedInitialSkew();
    void testAnimatedInitialSkewAxis();
    void testAnimatedUpdatedAnchorX();
    void testAnimatedUpdatedAnchorY();
    void testAnimatedUpdatedPositionX();
    void testAnimatedUpdatedPositionY();
    void testAnimatedUpdatedScaleX();
    void testAnimatedUpdatedScaleY();
    void testAnimatedUpdatedRotation();
    void testAnimatedUpdatedOpacity();
    void testAnimatedUpdatedSkew();
    void testAnimatedUpdatedSkewAxis();

    void testName();
    void testType();
    void testActive();

private:
    void loadTestData(const QString &filename);
    void updateProperty(int frame);

    QLottieShapeTransform *m_transform = nullptr;
};

tst_QLottieShapeTransform::tst_QLottieShapeTransform()
{

}

tst_QLottieShapeTransform::~tst_QLottieShapeTransform()
{

}

void tst_QLottieShapeTransform::initTestCase()
{
}

void tst_QLottieShapeTransform::cleanupTestCase()
{
    if (m_transform)
        delete m_transform;
}

void tst_QLottieShapeTransform::testStaticInitialAnchorX()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieShapeTransform::testStaticInitialAnchorY()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieShapeTransform::testStaticInitialPositionX()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieShapeTransform::testStaticInitialPositionY()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieShapeTransform::testStaticInitialScaleX()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieShapeTransform::testStaticInitialScaleY()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieShapeTransform::testStaticInitialRotation()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieShapeTransform::testStaticInitialOpacity()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieShapeTransform::testStaticInitialSkew()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->skew(), 0.0));
}

void tst_QLottieShapeTransform::testStaticInitialSkewAxis()
{
    loadTestData("shapetransform_static.json");

    QVERIFY(qFuzzyCompare(m_transform->skewAxis(), 0.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedAnchorX()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedAnchorY()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedPositionX()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedPositionY()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedScaleX()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieShapeTransform::testStaticUpdatedScaleY()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieShapeTransform::testStaticUpdatedRotation()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedOpacity()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedSkew()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->skew(), 0.0));
}

void tst_QLottieShapeTransform::testStaticUpdatedSkewAxis()
{
    loadTestData("shapetransform_static.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->skewAxis(), 0.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialAnchorX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 50.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialAnchorY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 50.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialPositionX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 50.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialPositionY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 50.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialScaleX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 0.5));
}

void tst_QLottieShapeTransform::testAnimatedInitialScaleY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 0.5));
}

void tst_QLottieShapeTransform::testAnimatedInitialRotation()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), 0.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialOpacity()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 1.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialSkew()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->skew(), 0.0));
}

void tst_QLottieShapeTransform::testAnimatedInitialSkewAxis()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(0);

    QVERIFY(qFuzzyCompare(m_transform->skewAxis(), 0.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedAnchorX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().x(), 25.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedAnchorY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->anchorPoint().y(), 25.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedPositionX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().x(), 75.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedPositionY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->position().y(), 75.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedScaleX()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().x(), 1.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedScaleY()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->scale().y(), 1.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedRotation()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->rotation(), (3 * 360 + 30.0)));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedOpacity()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->opacity(), 0.25));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedSkew()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->skew(), 4.0));
}

void tst_QLottieShapeTransform::testAnimatedUpdatedSkewAxis()
{
    loadTestData("shapetransform_animated.json");
    updateProperty(180);

    QVERIFY(qFuzzyCompare(m_transform->skewAxis(), (2 * 360 + 25.0)));
}

void tst_QLottieShapeTransform::testName()
{
    loadTestData("shapetransform_static.json");
    QVERIFY(m_transform->name() == QString("Transform"));
}

void tst_QLottieShapeTransform::testType()
{
    loadTestData("shapetransform_static.json");
    QVERIFY(m_transform->type() == LOTTIE_SHAPE_TRANS_IX);
}

void tst_QLottieShapeTransform::testActive()
{
    loadTestData("shapetransform_static.json");
    QVERIFY(m_transform->active(100) == true);
}

void tst_QLottieShapeTransform::loadTestData(const QString &filename)
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
    QLottieGroup* group = nullptr;
    while (shapesIt != shapes.end()) {
        QJsonObject childObj = (*shapesIt).toObject();
        QLottieShape *shape = QLottieShape::construct(childObj);
        QVERIFY(shape != nullptr);
        if (shape->type() == LOTTIE_SHAPE_GROUP_IX) {
            group = static_cast<QLottieGroup *>(shape);
            break;
        }
        shapesIt++;
    }
    QVERIFY(group != nullptr);

    m_transform = static_cast<QLottieShapeTransform*>(group->findChild("Transform"));

    QVERIFY(m_transform != nullptr);
}

void tst_QLottieShapeTransform::updateProperty(int frame)
{
    m_transform->updateProperties(frame);
}

QTEST_MAIN(tst_QLottieShapeTransform)
#include "tst_lottieshapetransform.moc"
