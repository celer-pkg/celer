// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/QCustom3DItem>

#include <QtGui/qquaternion.h>

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

private:
    QCustom3DItem *m_custom;
};

void tst_custom::initTestCase()
{
}

void tst_custom::cleanupTestCase()
{
}

void tst_custom::init()
{
    m_custom = new QCustom3DItem();
}

void tst_custom::cleanup()
{
    delete m_custom;
}

void tst_custom::construct()
{
    QCustom3DItem *custom = new QCustom3DItem();
    QVERIFY(custom);
    delete custom;

    custom = new QCustom3DItem(":/customitem.obj", QVector3D(1.0f, 1.0f, 1.0f),
                               QVector3D(1.0f, 1.0f, 1.0f), QQuaternion(1.0f, 1.0f, 10.0f, 100.0f),
                               QImage(":/customtexture.jpg"));
    QVERIFY(custom);
    QCOMPARE(custom->meshFile(), QString(":/customitem.obj"));
    QCOMPARE(custom->position(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(custom->isPositionAbsolute(), false);
    QCOMPARE(custom->rotation(), QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    QCOMPARE(custom->scaling(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(custom->isScalingAbsolute(), true);
    QCOMPARE(custom->isShadowCasting(), true);
    QCOMPARE(custom->textureFile(), QString());
    QCOMPARE(custom->isVisible(), true);
    delete custom;
}

void tst_custom::initialProperties()
{
    QVERIFY(m_custom);

    QCOMPARE(m_custom->meshFile(), QString());
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

    m_custom->setMeshFile(":/customitem.obj");
    m_custom->setPosition(QVector3D(1.0f, 1.0f, 1.0f));
    m_custom->setPositionAbsolute(true);
    m_custom->setRotation(QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    m_custom->setScaling(QVector3D(1.0f, 1.0f, 1.0f));
    m_custom->setScalingAbsolute(false);
    m_custom->setShadowCasting(false);
    m_custom->setTextureFile(":/customtexture.jpg");
    m_custom->setVisible(false);

    QCOMPARE(m_custom->meshFile(), QString(":/customitem.obj"));
    QCOMPARE(m_custom->position(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(m_custom->isPositionAbsolute(), true);
    QCOMPARE(m_custom->rotation(), QQuaternion(1.0f, 1.0f, 10.0f, 100.0f));
    QCOMPARE(m_custom->scaling(), QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(m_custom->isScalingAbsolute(), false);
    QCOMPARE(m_custom->isShadowCasting(), false);
    QCOMPARE(m_custom->textureFile(), QString(":/customtexture.jpg"));
    QCOMPARE(m_custom->isVisible(), false);

    m_custom->setTextureImage(QImage(QSize(10, 10), QImage::Format_ARGB32));
    QCOMPARE(m_custom->textureFile(), QString());
}

QTEST_MAIN(tst_custom)
#include "tst_custom.moc"
