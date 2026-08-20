// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QCategory3DAxis>
#include <QtGraphs/QValue3DAxis>
#include <QtGraphs/QBar3DSeries>

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

    QCategory3DAxis *m_rowAxis;
    QCategory3DAxis *m_columnAxis;
    QValue3DAxis *m_valueAxis;

    QBar3DSeries *m_barSeries;
};

void tst_seriesaxis::initTestCase()
{
}

void tst_seriesaxis::cleanupTestCase()
{
}

void tst_seriesaxis::init()
{
    m_barSeries = new QBar3DSeries;

    m_rowAxis = new QCategory3DAxis;
    m_columnAxis = new QCategory3DAxis;
    m_valueAxis = new QValue3DAxis;
}

void tst_seriesaxis::cleanup()
{
    delete m_barSeries;
}

void tst_seriesaxis::construct()
{
    QCategory3DAxis *axis = new QCategory3DAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_seriesaxis::initialProperties()
{
    QVERIFY(m_barSeries);

    QCOMPARE(m_barSeries->rowAxis(), nullptr);
    QCOMPARE(m_barSeries->columnAxis(), nullptr);
    QCOMPARE(m_barSeries->valueAxis(), nullptr);
}

void tst_seriesaxis::initializeProperties()
{
    QVERIFY(m_barSeries);

    QSignalSpy rowAxisSpy(m_barSeries, &QBar3DSeries::rowAxisChanged);
    QSignalSpy columnAxisSpy(m_barSeries, &QBar3DSeries::columnAxisChanged);
    QSignalSpy valueAxisSpy(m_barSeries, &QBar3DSeries::valueAxisChanged);

    m_barSeries->setRowAxis(m_rowAxis);
    m_barSeries->setColumnAxis(m_columnAxis);
    m_barSeries->setValueAxis(m_valueAxis);

    QCOMPARE(m_barSeries->rowAxis(), m_rowAxis);
    QCOMPARE(m_barSeries->columnAxis(), m_columnAxis);
    QCOMPARE(m_barSeries->valueAxis(), m_valueAxis);

    QCOMPARE(rowAxisSpy.size(), 1);
    QCOMPARE(columnAxisSpy.size(), 1);
    QCOMPARE(valueAxisSpy.size(), 1);
}

void tst_seriesaxis::resetAxes()
{
    m_barSeries->setRowAxis(m_rowAxis);
    m_barSeries->setColumnAxis(m_columnAxis);
    m_barSeries->setValueAxis(m_valueAxis);

    QVERIFY(m_barSeries->rowAxis());
    QVERIFY(m_barSeries->columnAxis());
    QVERIFY(m_barSeries->valueAxis());

    QSignalSpy rowAxisSpy(m_barSeries, &QBar3DSeries::rowAxisChanged);
    QSignalSpy columnAxisSpy(m_barSeries, &QBar3DSeries::columnAxisChanged);
    QSignalSpy valueAxisSpy(m_barSeries, &QBar3DSeries::valueAxisChanged);

    m_barSeries->resetRowAxis();
    m_barSeries->resetColumnAxis();
    m_barSeries->resetValueAxis();

    QCOMPARE(m_barSeries->rowAxis(), nullptr);
    QCOMPARE(m_barSeries->columnAxis(), nullptr);
    QCOMPARE(m_barSeries->valueAxis(), nullptr);

    QCOMPARE(rowAxisSpy.size(), 1);
    QCOMPARE(columnAxisSpy.size(), 1);
    QCOMPARE(valueAxisSpy.size(), 1);
}

QTEST_MAIN(tst_seriesaxis)
#include "tst_seriesaxis.moc"
