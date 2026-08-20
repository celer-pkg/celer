// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "private/qlottielayer_p.h"
#include "private/qlottieshapelayer_p.h"

using namespace Qt::StringLiterals;

class tst_QLottieShapeLayer: public QObject
{
    Q_OBJECT

public:
    tst_QLottieShapeLayer();
    ~tst_QLottieShapeLayer();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testName();
    void testType();
    void testWidth();
    void testHeight();
    void testActive();
    void testIsUsingMatteLayer();
    void testIsMatteLayer();
    void testMatteMode();

private:
    void loadTestData(const QString &filename);

    QLottieShapeLayer *m_layer;
    QLottieShapeLayer *m_clippedlayer;
    qreal m_width;
    qreal m_height;
};

tst_QLottieShapeLayer::tst_QLottieShapeLayer()
{
    m_layer = nullptr;
    m_clippedlayer = nullptr;
}

tst_QLottieShapeLayer::~tst_QLottieShapeLayer()
{

}

void tst_QLottieShapeLayer::initTestCase()
{

}

void tst_QLottieShapeLayer::cleanupTestCase()
{

}

void tst_QLottieShapeLayer::testName()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->name() == QString("Shape Layer 1"));
}

void tst_QLottieShapeLayer::testType()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->type() == LOTTIE_LAYER_SHAPE_IX);
}

void tst_QLottieShapeLayer::testActive()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->active(0) == false);
    QVERIFY(m_layer->active(100) == true);
    QVERIFY(m_layer->active(150) == false);
}

void tst_QLottieShapeLayer::testWidth()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_width == 100.0);
}

void tst_QLottieShapeLayer::testHeight()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_height == 100.0);
}

void tst_QLottieShapeLayer::testIsUsingMatteLayer()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->isUsingMatteLayer() == false);
    loadTestData("shape_matte_alphaclip.json");
    QVERIFY(m_clippedlayer->isUsingMatteLayer() == true);
}

void tst_QLottieShapeLayer::testIsMatteLayer()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->isMatteLayer() == false);
    loadTestData("shape_matte_alphaclip.json");
    QVERIFY(m_layer->isMatteLayer() == true);
}

void tst_QLottieShapeLayer::testMatteMode()
{
    loadTestData("shape_active_60to120.json");
    QVERIFY(m_layer->matteMode() == QLottieLayer::MatteClipMode::NoClip);
    loadTestData("shape_matte_alphaclip.json");
    QVERIFY(m_clippedlayer->matteMode() == QLottieLayer::MatteClipMode::Alpha);
    loadTestData("shape_matte_alphainvclip.json");
    QVERIFY(m_clippedlayer->matteMode() == QLottieLayer::MatteClipMode::InvertedAlpha);
    loadTestData("shape_matte_lumaclip.json");
    QVERIFY(m_clippedlayer->matteMode() == QLottieLayer::MatteClipMode::Luminence);
    loadTestData("shape_matte_lumainvclip.json");
    QVERIFY(m_clippedlayer->matteMode() == QLottieLayer::MatteClipMode::InvertedLuminence);
}

void tst_QLottieShapeLayer::loadTestData(const QString &filename)
{
    if (m_layer) {
        delete m_layer;
        m_layer = nullptr;
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

    m_width = rootObj.value(QLatin1String("w")).toVariant().toReal();
    m_height = rootObj.value(QLatin1String("h")).toVariant().toReal();

    QJsonArray layers = rootObj.value(QLatin1String("layers")).toArray();
    QJsonObject layerObj = layers[0].toObject();
    int type = layerObj.value(QLatin1String("ty")).toInt();
    if (type != 4)
        QFAIL("It's not shape layer");
    m_layer = new QLottieShapeLayer();
    int ret = m_layer->parse(layerObj);
    QVERIFY(m_layer != nullptr);
    QVERIFY(ret >= 0);

    if (layers.size() > 1) {
        layerObj = layers[1].toObject();
        type = layerObj.value(QLatin1String("ty")).toInt();
        if (type != 4)
            QFAIL("it's not shape layer");
        m_clippedlayer = new QLottieShapeLayer();
        m_clippedlayer->parse(layerObj);
        QVERIFY(m_clippedlayer != nullptr);
    }
}

QTEST_MAIN(tst_QLottieShapeLayer)
#include "tst_lottieshapelayer.moc"
