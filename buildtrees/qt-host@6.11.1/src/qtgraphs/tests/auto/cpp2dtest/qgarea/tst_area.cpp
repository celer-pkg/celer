// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QAreaSeries>
#include <QtGraphs/QLineSeries>
#ifdef USE_SPLINEGRAPH
#include <QtGraphs/QSplineSeries>
#endif
#include <QtGraphs/QValueAxis>
#include <QtQml/QQmlComponent>
#include <QtTest/QtTest>

class tst_area : public QObject
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
#ifdef USE_SPLINEGRAPH
    void initializePropertiesWithSpline();
#endif
    void invalidProperties();

private:
    QAreaSeries *m_series;
};

void tst_area::initTestCase() {}

void tst_area::cleanupTestCase() {}

void tst_area::init()
{
    m_series = new QAreaSeries();
}

void tst_area::cleanup()
{
    delete m_series;
}

void tst_area::construct()
{
    QAreaSeries *series = new QAreaSeries();
    QVERIFY(series);
    delete series;
}

void tst_area::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QAreaSeries
    QCOMPARE(m_series->color(), QColor(Qt::transparent));
    QCOMPARE(m_series->selectedColor(), QColor(Qt::transparent));
    QCOMPARE(m_series->borderColor(), QColor(Qt::transparent));
    QCOMPARE(m_series->selectedBorderColor(), QColor(Qt::transparent));
    QCOMPARE(m_series->borderWidth(), -1.0);
    QCOMPARE(m_series->isSelected(), false);
    QCOMPARE(m_series->upperSeries(), nullptr);
    QCOMPARE(m_series->lowerSeries(), nullptr);

    // Properties from QAbstractSeries
    QCOMPARE(m_series->name(), "");
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isSelectable(), false);
    QCOMPARE(m_series->isHoverable(), false);
    QCOMPARE(m_series->opacity(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);
}

void tst_area::initializeProperties()
{
    QVERIFY(m_series);

    // Signals from QAreasSeries
    QSignalSpy spy0(m_series, &QAreaSeries::colorChanged);
    QSignalSpy spy1(m_series, &QAreaSeries::selectedColorChanged);
    QSignalSpy spy2(m_series, &QAreaSeries::borderColorChanged);
    QSignalSpy spy3(m_series, &QAreaSeries::selectedBorderColorChanged);
    QSignalSpy spy4(m_series, &QAreaSeries::borderWidthChanged);
    QSignalSpy spy5(m_series, &QAreaSeries::selectedChanged);
    QSignalSpy spy6(m_series, &QAreaSeries::upperSeriesChanged);
    QSignalSpy spy7(m_series, &QAreaSeries::lowerSeriesChanged);

    // Signals from QAbstractSeries
    QSignalSpy spy8(m_series, &QAbstractSeries::nameChanged);
    QSignalSpy spy9(m_series, &QAbstractSeries::visibleChanged);
    QSignalSpy spy10(m_series, &QAbstractSeries::selectableChanged);
    QSignalSpy spy11(m_series, &QAbstractSeries::hoverableChanged);
    QSignalSpy spy12(m_series, &QAbstractSeries::opacityChanged);
    QSignalSpy spy13(m_series, &QAbstractSeries::valuesMultiplierChanged);

    auto upperSeries = new QLineSeries(this);
    auto lowerSeries = new QLineSeries(this);

    m_series->setColor("#ff0000");
    m_series->setSelectedColor("#0000ff");
    m_series->setBorderColor("#ff0000");
    m_series->setSelectedBorderColor("#0000ff");
    m_series->setBorderWidth(2.0);
    m_series->setSelected(true);
    m_series->setUpperSeries(upperSeries);
    m_series->setLowerSeries(lowerSeries);

    m_series->setName("AreaSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    QCOMPARE(m_series->color(), "#ff0000");
    QCOMPARE(m_series->selectedColor(), "#0000ff");
    QCOMPARE(m_series->borderColor(), "#ff0000");
    QCOMPARE(m_series->selectedBorderColor(), "#0000ff");
    QCOMPARE(m_series->borderWidth(), 2.0);
    QCOMPARE(m_series->isSelected(), true);
    QCOMPARE(m_series->upperSeries(), upperSeries);
    QCOMPARE(m_series->lowerSeries(), lowerSeries);

    QCOMPARE(m_series->name(), "AreaSeries");
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
    QCOMPARE(spy10.size(), 1);
    QCOMPARE(spy11.size(), 1);
    QCOMPARE(spy12.size(), 1);
    QCOMPARE(spy13.size(), 1);
}

#ifdef USE_SPLINEGRAPH
void tst_area::initializePropertiesWithSpline()
{
    QVERIFY(m_series);

    // Signals from QAreasSeries
    QSignalSpy spy0(m_series, &QAreaSeries::colorChanged);
    QSignalSpy spy1(m_series, &QAreaSeries::selectedColorChanged);
    QSignalSpy spy2(m_series, &QAreaSeries::borderColorChanged);
    QSignalSpy spy3(m_series, &QAreaSeries::selectedBorderColorChanged);
    QSignalSpy spy4(m_series, &QAreaSeries::borderWidthChanged);
    QSignalSpy spy5(m_series, &QAreaSeries::selectedChanged);
    QSignalSpy spy6(m_series, &QAreaSeries::upperSeriesChanged);
    QSignalSpy spy7(m_series, &QAreaSeries::lowerSeriesChanged);

    // Signals from QAbstractSeries
    QSignalSpy spy8(m_series, &QAbstractSeries::nameChanged);
    QSignalSpy spy9(m_series, &QAbstractSeries::visibleChanged);
    QSignalSpy spy10(m_series, &QAbstractSeries::selectableChanged);
    QSignalSpy spy11(m_series, &QAbstractSeries::hoverableChanged);
    QSignalSpy spy12(m_series, &QAbstractSeries::opacityChanged);
    QSignalSpy spy13(m_series, &QAbstractSeries::valuesMultiplierChanged);

    auto upperSeries = new QSplineSeries(this);
    auto lowerSeries = new QSplineSeries(this);

    m_series->setColor("#ff0000");
    m_series->setSelectedColor("#0000ff");
    m_series->setBorderColor("#ff0000");
    m_series->setSelectedBorderColor("#0000ff");
    m_series->setBorderWidth(2.0);
    m_series->setSelected(true);
    m_series->setUpperSeries(upperSeries);
    m_series->setLowerSeries(lowerSeries);

    m_series->setName("AreaSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    QCOMPARE(m_series->color(), "#ff0000");
    QCOMPARE(m_series->selectedColor(), "#0000ff");
    QCOMPARE(m_series->borderColor(), "#ff0000");
    QCOMPARE(m_series->selectedBorderColor(), "#0000ff");
    QCOMPARE(m_series->borderWidth(), 2.0);
    QCOMPARE(m_series->isSelected(), true);
    QCOMPARE(m_series->upperSeries(), upperSeries);
    QCOMPARE(m_series->lowerSeries(), lowerSeries);

    QCOMPARE(m_series->name(), "AreaSeries");
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
    QCOMPARE(spy10.size(), 1);
    QCOMPARE(spy11.size(), 1);
    QCOMPARE(spy12.size(), 1);
    QCOMPARE(spy13.size(), 1);
}
#endif

void tst_area::invalidProperties()
{
    QVERIFY(m_series);

    m_series->setValuesMultiplier(2.0); // range 0...1
    QCOMPARE(m_series->valuesMultiplier(), 1.0);

    m_series->setValuesMultiplier(-1.0); // range 0...1
    QCOMPARE(m_series->valuesMultiplier(), 0.0);
}

QTEST_MAIN(tst_area)
#include "tst_area.moc"
