// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/Q3DLight>

class tst_light: public QObject
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
    Q3DLight *m_light;
};

void tst_light::initTestCase()
{
}

void tst_light::cleanupTestCase()
{
}

void tst_light::init()
{
    m_light = new Q3DLight();
}

void tst_light::cleanup()
{
    delete m_light;
}

void tst_light::construct()
{
    Q3DLight *light = new Q3DLight();
    QVERIFY(light);
    delete light;
}

void tst_light::initialProperties()
{
    QVERIFY(m_light);

    QCOMPARE(m_light->isAutoPosition(), false);

    // Common (from Q3DObject)
    QVERIFY(!m_light->parentScene());
    QCOMPARE(m_light->position(), QVector3D(0, 0, 0));
}

void tst_light::initializeProperties()
{
    QVERIFY(m_light);

    m_light->setAutoPosition(true);
    QCOMPARE(m_light->isAutoPosition(), true);

    // Common (from Q3DObject)
    m_light->setPosition(QVector3D(1.0f, 1.0f, 1.0f));
    QCOMPARE(m_light->position(), QVector3D(1.0f, 1.0f, 1.0f));
}

QTEST_MAIN(tst_light)
#include "tst_light.moc"
