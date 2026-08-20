// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QCategory3DAxis>
#include <QtGraphs/QValue3DAxis>
#include <QtGraphs/QSurface3DSeries>

class tst_seriesaxis: public QObject
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
    void resetAxes();

private:
    QValue3DAxis *m_axisX;
    QValue3DAxis *m_axisY;
    QValue3DAxis *m_axisZ;

    QSurface3DSeries *m_surfaceSeries;
};

void tst_seriesaxis::initTestCase()
{
}

void tst_seriesaxis::cleanupTestCase()
{
}

void tst_seriesaxis::init()
{
    m_surfaceSeries = new QSurface3DSeries;

    m_axisX = new QValue3DAxis;
    m_axisY = new QValue3DAxis;
    m_axisZ = new QValue3DAxis;
}

void tst_seriesaxis::cleanup()
{
    delete m_surfaceSeries;
}

void tst_seriesaxis::construct()
{
    QCategory3DAxis *axis = new QCategory3DAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_seriesaxis::initialProperties()
{
    QVERIFY(m_surfaceSeries);
    QCOMPARE(m_surfaceSeries->axisX(), nullptr);
    QCOMPARE(m_surfaceSeries->axisY(), nullptr);
    QCOMPARE(m_surfaceSeries->axisZ(), nullptr);
}

void tst_seriesaxis::initializeProperties()
{
    QVERIFY(m_surfaceSeries);

    QSignalSpy axisXSpy(m_surfaceSeries, &QSurface3DSeries::axisXChanged);
    QSignalSpy axisYSpy(m_surfaceSeries, &QSurface3DSeries::axisYChanged);
    QSignalSpy axisZSpy(m_surfaceSeries, &QSurface3DSeries::axisZChanged);

    m_surfaceSeries->setAxisX(m_axisX);
    m_surfaceSeries->setAxisY(m_axisY);
    m_surfaceSeries->setAxisZ(m_axisZ);

    QCOMPARE(m_surfaceSeries->axisX(), m_axisX);
    QCOMPARE(m_surfaceSeries->axisY(), m_axisY);
    QCOMPARE(m_surfaceSeries->axisZ(), m_axisZ);

    QCOMPARE(axisXSpy.size(), 1);
    QCOMPARE(axisYSpy.size(), 1);
    QCOMPARE(axisZSpy.size(), 1);
}

void tst_seriesaxis::resetAxes()
{
    m_surfaceSeries->setAxisX(m_axisX);
    m_surfaceSeries->setAxisY(m_axisY);
    m_surfaceSeries->setAxisZ(m_axisZ);

    QVERIFY(m_surfaceSeries->axisX());
    QVERIFY(m_surfaceSeries->axisY());
    QVERIFY(m_surfaceSeries->axisZ());

    QSignalSpy axisXSpy(m_surfaceSeries, &QSurface3DSeries::axisXChanged);
    QSignalSpy axisYSpy(m_surfaceSeries, &QSurface3DSeries::axisYChanged);
    QSignalSpy axisZSpy(m_surfaceSeries, &QSurface3DSeries::axisZChanged);

    m_surfaceSeries->resetAxisX();
    m_surfaceSeries->resetAxisY();
    m_surfaceSeries->resetAxisZ();

    QCOMPARE(m_surfaceSeries->axisX(), nullptr);
    QCOMPARE(m_surfaceSeries->axisY(), nullptr);
    QCOMPARE(m_surfaceSeries->axisZ(), nullptr);

    QCOMPARE(axisXSpy.size(), 1);
    QCOMPARE(axisYSpy.size(), 1);
    QCOMPARE(axisZSpy.size(), 1);
}

QTEST_MAIN(tst_seriesaxis)
#include "tst_seriesaxis.moc"
