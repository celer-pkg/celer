// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QCustomSeries>
#include <QtGraphs/QValueAxis>
#include <QtQml/QQmlComponent>
#include <QtTest/QtTest>

class tst_custom : public QObject
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
    QCustomSeries *m_series;
};

void tst_custom::initTestCase() {}

void tst_custom::cleanupTestCase() {}

void tst_custom::init()
{
    m_series = new QCustomSeries();
}

void tst_custom::cleanup()
{
    delete m_series;
}

void tst_custom::construct()
{
    QCustomSeries *series = new QCustomSeries();
    QVERIFY(series);
    delete series;
}

void tst_custom::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QCustomSeries
    QCOMPARE(m_series->delegate(), nullptr);

    // Properties from QAbstractSeries
    QCOMPARE(m_series->name(), "");
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isSelectable(), false);
    QCOMPARE(m_series->isHoverable(), false);
    QCOMPARE(m_series->opacity(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);
}

void tst_custom::initializeProperties()
{
    QVERIFY(m_series);

    auto delegate = new QQmlComponent(this);

    QSignalSpy spy0(m_series, &QCustomSeries::delegateChanged);

    QSignalSpy spy1(m_series, &QCustomSeries::nameChanged);
    QSignalSpy spy2(m_series, &QCustomSeries::visibleChanged);
    QSignalSpy spy3(m_series, &QCustomSeries::selectableChanged);
    QSignalSpy spy4(m_series, &QCustomSeries::hoverableChanged);
    QSignalSpy spy5(m_series, &QCustomSeries::opacityChanged);
    QSignalSpy spy6(m_series, &QCustomSeries::valuesMultiplierChanged);

    m_series->setDelegate(delegate);

    m_series->setName("CustomSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    QCOMPARE(m_series->delegate(), delegate);

    QCOMPARE(m_series->name(), "CustomSeries");
    QCOMPARE(m_series->isVisible(), false);
    QCOMPARE(m_series->isSelectable(), true);
    QCOMPARE(m_series->isHoverable(), true);
    QCOMPARE(m_series->opacity(), 0.5);
    QCOMPARE(m_series->valuesMultiplier(), 0.5);

    QCOMPARE(spy0.size(), 1);

    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);
    QCOMPARE(spy6.size(), 1);
}

QTEST_MAIN(tst_custom)
#include "tst_custom.moc"
