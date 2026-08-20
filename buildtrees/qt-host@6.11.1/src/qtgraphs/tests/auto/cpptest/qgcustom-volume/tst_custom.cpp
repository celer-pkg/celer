// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QCustom3DVolume>

class tst_custom: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();

    void initialProperties();
    void initializeProperties();
    void invalidProperties();

private:
    QCustom3DVolume *m_custom;
};

void tst_custom::initTestCase()
{
}

void tst_custom::cleanupTestCase()
{
}

void tst_custom::init()
{
    m_custom = new QCustom3DVolume();
}

void tst_custom::cleanup()
{
    delete m_custom;
}

void tst_custom::construct()
{
    QCustom3DVolume *custom = new QCustom3DVolume();
    QVERIFY(custom);
    delete custom;

    QList<uchar> *tdata = new QList<uchar>(1000);

    QList<QRgb> table;
    table << QRgb(0xff00ff) << QRgb(0x00ff00);

    custom = new QCustom3DVolume(QVector3D(1.0f, 1.0f, 1.0f), QVector3D(1.0f, 1.0f, 1.0f),
                                 QQuaternion(1.0f, 1.0f, 10.0f, 100.0f), 10, 10, 10,
                                 tdata, QImage::Format_ARGB32, table);
    QVERIFY(custom);
    QCOMPARE(custom->alphaMultiplier(), 1.0f);
    QCOMPARE(custom->drawSliceFrames(), false);
    QCOMPARE(custom->drawSliceFrames(), false);
    QCOMPARE(custom->preserveOpacity(), true);
    QCOMPARE(custom->sliceFrameColor(), QColor(Qt::black));
    QCOMPARE(custom->sliceFrameGaps(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(custom->sliceFrameThicknesses(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(custom->sliceFrameWidths(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(custom->sliceIndexX(), -1);
    QCOMPARE(custom->sliceIndexY(), -1);
    QCOMPARE(custom->sliceIndexZ(), -1);
    QCOMPARE(custom->useHighDefShader(), true);
    QCOMPARE(custom->textureData()->size(), 1000);
    QCOMPARE(custom->textureDataWidth(), 40);
    QCOMPARE(custom->textureFormat(), QImage::Format_ARGB32);
    QCOMPARE(custom->textureHeight(), 10);
    QCOMPARE(custom->textureWidth(), 10);
    QCOMPARE(custom->textureDepth(), 10);
    QCOMPARE(custom->meshFile(), QString(":/defaultMeshes/barMeshFull"));
    QCOMPARE(custom->position(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(custom->isPositionAbsolute(), false);
    QCOMPARE(custom->rotation(), QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    QCOMPARE(custom->scaling(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(custom->isScalingAbsolute(), true);
    QCOMPARE(custom->isShadowCasting(), false);
    QCOMPARE(custom->textureFile(), QString());
    QCOMPARE(custom->isVisible(), true);
    delete custom;
}

void tst_custom::initialProperties()
{
    QVERIFY(m_custom);

    QCOMPARE(m_custom->alphaMultiplier(), 1.0f);
    QCOMPARE(m_custom->drawSliceFrames(), false);
    QCOMPARE(m_custom->drawSliceFrames(), false);
    QCOMPARE(m_custom->preserveOpacity(), true);
    QCOMPARE(m_custom->sliceFrameColor(), QColor(Qt::black));
    QCOMPARE(m_custom->sliceFrameGaps(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(m_custom->sliceFrameThicknesses(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(m_custom->sliceFrameWidths(), QVector3D(0.01f, 0.01f, 0.01f));
    QCOMPARE(m_custom->sliceIndexX(), -1);
    QCOMPARE(m_custom->sliceIndexY(), -1);
    QCOMPARE(m_custom->sliceIndexZ(), -1);
    QCOMPARE(m_custom->useHighDefShader(), true);

    // Common (from QCustom3DVolume)
    QCOMPARE(m_custom->meshFile(), QString(":/defaultMeshes/barMeshFull"));
    QCOMPARE(m_custom->position(), QVector3D());
    QCOMPARE(m_custom->isPositionAbsolute(), false);
    QCOMPARE(m_custom->rotation(), QQuaternion());
    QCOMPARE(m_custom->scaling(), QVector3D(0.1f, 0.1f, 0.1f));
    QCOMPARE(m_custom->isScalingAbsolute(), true);
    QCOMPARE(m_custom->isShadowCasting(), true);
    QCOMPARE(m_custom->textureFile(), QString());
    QCOMPARE(m_custom->isVisible(), true);
}

void tst_custom::initializeProperties()
{
    QVERIFY(m_custom);

    QSignalSpy textureWidthSpy(m_custom, &QCustom3DVolume::textureWidthChanged);
    QSignalSpy textureHeightSpy(m_custom, &QCustom3DVolume::textureHeightChanged);
    QSignalSpy textureDepthSpy(m_custom, &QCustom3DVolume::textureDepthChanged);
    QSignalSpy sliceIndexXSpy(m_custom, &QCustom3DVolume::sliceIndexXChanged);
    QSignalSpy sliceIndexYSpy(m_custom, &QCustom3DVolume::sliceIndexYChanged);
    QSignalSpy sliceIndexZSpy(m_custom, &QCustom3DVolume::sliceIndexZChanged);
    QSignalSpy colorTableSpy(m_custom, &QCustom3DVolume::colorTableChanged);
    QSignalSpy textureDataSpy(m_custom, &QCustom3DVolume::textureDataChanged);
    QSignalSpy textureFormatSpy(m_custom, &QCustom3DVolume::textureFormatChanged);
    QSignalSpy alphaMultiplierSpy(m_custom, &QCustom3DVolume::alphaMultiplierChanged);
    QSignalSpy preserveOpacitySpy(m_custom, &QCustom3DVolume::preserveOpacityChanged);
    QSignalSpy useHighDefShaderSpy(m_custom, &QCustom3DVolume::useHighDefShaderChanged);
    QSignalSpy drawSlicesSpy(m_custom, &QCustom3DVolume::drawSlicesChanged);
    QSignalSpy drawSliceFramesSpy(m_custom, &QCustom3DVolume::drawSliceFramesChanged);
    QSignalSpy sliceFrameColorSpy(m_custom, &QCustom3DVolume::sliceFrameColorChanged);
    QSignalSpy sliceFrameWidthSpy(m_custom, &QCustom3DVolume::sliceFrameWidthsChanged);
    QSignalSpy sliceFrameGapsSpy(m_custom, &QCustom3DVolume::sliceFrameGapsChanged);
    QSignalSpy sliceFrameThicknessSpy(m_custom, &QCustom3DVolume::sliceFrameThicknessesChanged);

    m_custom->setAlphaMultiplier(0.1f);
    m_custom->setDrawSliceFrames(true);
    m_custom->setDrawSliceFrames(true);
    m_custom->setPreserveOpacity(false);
    m_custom->setSliceFrameColor(QColor(Qt::red));
    m_custom->setSliceFrameGaps(QVector3D(2.0f, 2.0f, 2.0f));
    m_custom->setSliceFrameThicknesses(QVector3D(2.0f, 2.0f, 2.0f));
    m_custom->setSliceFrameWidths(QVector3D(2.0f, 2.0f, 2.0f));
    m_custom->setSliceIndexX(0);
    m_custom->setSliceIndexY(0);
    m_custom->setSliceIndexZ(0);
    m_custom->setUseHighDefShader(false);

    QCOMPARE(m_custom->alphaMultiplier(), 0.1f);
    QCOMPARE(m_custom->drawSliceFrames(), true);
    QCOMPARE(m_custom->drawSliceFrames(), true);
    QCOMPARE(m_custom->preserveOpacity(), false);
    QCOMPARE(m_custom->sliceFrameColor(), QColor(Qt::red));
    QCOMPARE(m_custom->sliceFrameGaps(), QVector3D(2.0f, 2.0f, 2.0f));
    QCOMPARE(m_custom->sliceFrameThicknesses(), QVector3D(2.0f, 2.0f, 2.0f));
    QCOMPARE(m_custom->sliceFrameWidths(), QVector3D(2.0f, 2.0f, 2.0f));
    QCOMPARE(m_custom->sliceIndexX(), 0);
    QCOMPARE(m_custom->sliceIndexY(), 0);
    QCOMPARE(m_custom->sliceIndexZ(), 0);
    QCOMPARE(m_custom->useHighDefShader(), false);


    // Common (from QCustom3DVolume)
    m_custom->setPosition(QVector3D(1.0f, 1.0f, 1.0f));
    m_custom->setPositionAbsolute(true);
    m_custom->setRotation(QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    m_custom->setScaling(QVector3D(1.0f, 1.0f, 1.0f));
    m_custom->setScalingAbsolute(false);
    m_custom->setShadowCasting(false);
    m_custom->setVisible(false);
    m_custom->setTextureWidth(10);
    m_custom->setTextureHeight(10);
    m_custom->setTextureDepth(10);
    m_custom->setDrawSlices(true);

    QList<QRgb> colors = { 0x88112233 };
    m_custom->setColorTable(colors);

    QCOMPARE(m_custom->position(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(m_custom->isPositionAbsolute(), true);
    QCOMPARE(m_custom->rotation(), QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    QCOMPARE(m_custom->scaling(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(m_custom->isScalingAbsolute(), false);
    QCOMPARE(m_custom->isShadowCasting(), false);
    QCOMPARE(m_custom->isVisible(), false);

    QCOMPARE(textureWidthSpy.size(), 1);
    QCOMPARE(textureHeightSpy.size(), 1);
    QCOMPARE(textureDepthSpy.size(), 1);
    QCOMPARE(sliceIndexXSpy.size(), 1);
    QCOMPARE(sliceIndexYSpy.size(), 1);
    QCOMPARE(sliceIndexZSpy.size(), 1);
    QCOMPARE(colorTableSpy.size(), 1);
    QCOMPARE(textureDataSpy.size(), 0);
    QCOMPARE(textureFormatSpy.size(), 0);
    QCOMPARE(alphaMultiplierSpy.size(), 1);
    QCOMPARE(preserveOpacitySpy.size(), 1);
    QCOMPARE(useHighDefShaderSpy.size(), 1);
    QCOMPARE(drawSlicesSpy.size(), 1);
    QCOMPARE(drawSliceFramesSpy.size(), 1);
    QCOMPARE(sliceFrameColorSpy.size(), 1);
    QCOMPARE(sliceFrameWidthSpy.size(), 1);
    QCOMPARE(sliceFrameGapsSpy.size(), 1);
    QCOMPARE(sliceFrameThicknessSpy.size(), 1);
}

void tst_custom::invalidProperties()
{
    m_custom->setAlphaMultiplier(-1.0f);
    QCOMPARE(m_custom->alphaMultiplier(), 1.0f);

    m_custom->setSliceFrameGaps(QVector3D(-0.1f, -0.1f, -0.1f));
    QCOMPARE(m_custom->sliceFrameGaps(), QVector3D(0.01f, 0.01f, 0.01f));

    m_custom->setSliceFrameThicknesses(QVector3D(-0.1f, -0.1f, -0.1f));
    QCOMPARE(m_custom->sliceFrameThicknesses(), QVector3D(0.01f, 0.01f, 0.01f));

    m_custom->setSliceFrameWidths(QVector3D(-0.1f, -0.1f, -0.1f));
    QCOMPARE(m_custom->sliceFrameWidths(), QVector3D(0.01f, 0.01f, 0.01f));

    m_custom->setTextureFormat(QImage::Format_ARGB8555_Premultiplied);
    QCOMPARE(m_custom->textureFormat(), QImage::Format_ARGB32);
}

QTEST_MAIN(tst_custom)
#include "tst_custom.moc"
