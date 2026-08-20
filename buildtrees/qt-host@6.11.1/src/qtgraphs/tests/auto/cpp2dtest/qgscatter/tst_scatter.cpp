// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QScatterSeries>
#include <QtGraphs/QValueAxis>
#include <QtQml/QQmlComponent>
#include <QtTest/QtTest>

class tst_scatter : public QObject
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
    QScatterSeries *m_series;
};

void tst_scatter::initTestCase() {}

void tst_scatter::cleanupTestCase() {}

void tst_scatter::init()
{
    m_series = new QScatterSeries();
}

void tst_scatter::cleanup()
{
    delete m_series;
}

void tst_scatter::construct()
{
    QScatterSeries *series = new QScatterSeries();
    QVERIFY(series);
    delete series;
}

void tst_scatter::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QScatterSeries
    QCOMPARE(m_series->pointDelegate(), nullptr);

    // Properties from QXYSeries
    QCOMPARE(m_series->color(), QColor(Qt::transparent));
    QCOMPARE(m_series->selectedColor(), QColor(Qt::transparent));
    QCOMPARE(m_series->isDraggable(), false);

    // Properties from QAbstractSeries
    QCOMPARE(m_series->name(), "");
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isSelectable(), false);
    QCOMPARE(m_series->isHoverable(), false);
    QCOMPARE(m_series->opacity(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);
}

void tst_scatter::initializeProperties()
{
    QVERIFY(m_series);

    auto marker = new QQmlComponent(this);


    QSignalSpy spy0(m_series, &QScatterSeries::pointDelegateChanged);
    QSignalSpy spy1(m_series, &QScatterSeries::colorChanged);
    QSignalSpy spy2(m_series, &QScatterSeries::selectedColorChanged);
    QSignalSpy spy3(m_series, &QScatterSeries::draggableChanged);

    QSignalSpy spy4(m_series, &QScatterSeries::nameChanged);
    QSignalSpy spy5(m_series, &QScatterSeries::visibleChanged);
    QSignalSpy spy6(m_series, &QScatterSeries::selectableChanged);
    QSignalSpy spy7(m_series, &QScatterSeries::hoverableChanged);
    QSignalSpy spy8(m_series, &QScatterSeries::opacityChanged);
    QSignalSpy spy9(m_series, &QScatterSeries::valuesMultiplierChanged);

    m_series->setPointDelegate(marker);

    m_series->setColor("#ff0000");
    m_series->setSelectedColor("#0000ff");
    m_series->setDraggable(true);

    m_series->setName("LineSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    QCOMPARE(m_series->pointDelegate(), marker);

    QCOMPARE(m_series->color(), "#ff0000");
    QCOMPARE(m_series->selectedColor(), "#0000ff");
    QCOMPARE(m_series->isDraggable(), true);

    QCOMPARE(m_series->name(), "LineSeries");
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
    QCOMPARE(spy7.size(), 1);
    QCOMPARE(spy8.size(), 1);
    QCOMPARE(spy9.size(), 1);
}

void tst_scatter::invalidProperties()
{
    QVERIFY(m_series);

    m_series->setValuesMultiplier(2.0); // range 0...1

    // TODO: QTBUG-121721
    // QCOMPARE(m_series->valuesMultiplier(), 1.0);

    m_series->setValuesMultiplier(-1.0); // range 0...1
    // TODO: QTBUG-121721
    // QCOMPARE(m_series->valuesMultiplier(), 0.0);
}

QTEST_MAIN(tst_scatter)
#include "tst_scatter.moc"
