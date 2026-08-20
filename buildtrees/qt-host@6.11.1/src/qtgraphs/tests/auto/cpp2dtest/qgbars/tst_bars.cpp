// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QValueAxis>
#include <QtGraphs/QBarCategoryAxis>
#include <QtTest/QtTest>

class tst_bars : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();
    void modifySeries();
    void initialProperties();
    void initializeProperties();
    void invalidProperties();

private:
    QBarSeries *m_series;
};

void tst_bars::initTestCase() {}

void tst_bars::cleanupTestCase() {}

void tst_bars::init()
{
    m_series = new QBarSeries();
}

void tst_bars::cleanup()
{
    delete m_series;
}

void tst_bars::construct()
{
    QBarSeries *series = new QBarSeries();
    QVERIFY(series);
    delete series;
}

void tst_bars::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QBarSeries
    QCOMPARE(m_series->barWidth(), 0.5);
    QCOMPARE(m_series->count(), 0);
    QCOMPARE(m_series->labelsVisible(), false);
    QCOMPARE(m_series->labelsFormat(), "");
    QCOMPARE(m_series->labelsPosition(), QBarSeries::LabelsPosition::Center);
    QCOMPARE(m_series->labelsMargin(), 0);
    QCOMPARE(m_series->labelsAngle(), 0);
    QCOMPARE(m_series->labelsPrecision(), 6);
    QCOMPARE(m_series->seriesColors(), {});

    // Properties from QAbstractSeries
    QCOMPARE(m_series->name(), "");
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isSelectable(), false);
    QCOMPARE(m_series->isHoverable(), false);
    QCOMPARE(m_series->opacity(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);
}

void tst_bars::initializeProperties()
{
    QVERIFY(m_series);

    QSignalSpy spy0(m_series, &QBarSeries::barWidthChanged);
    QSignalSpy spy1(m_series, &QBarSeries::labelsVisibleChanged);
    QSignalSpy spy2(m_series, &QBarSeries::labelsFormatChanged);
    QSignalSpy spy3(m_series, &QBarSeries::labelsPositionChanged);
    QSignalSpy spy4(m_series, &QBarSeries::labelsMarginChanged);
    QSignalSpy spy5(m_series, &QBarSeries::labelsAngleChanged);
    QSignalSpy spy6(m_series, &QBarSeries::labelsPrecisionChanged);
    QSignalSpy spy7(m_series, &QBarSeries::seriesColorsChanged);
    QSignalSpy spy8(m_series, &QBarSeries::borderColorsChanged);
    QSignalSpy spy9(m_series, &QBarSeries::nameChanged);
    QSignalSpy spy10(m_series, &QBarSeries::visibleChanged);
    QSignalSpy spy11(m_series, &QBarSeries::selectableChanged);
    QSignalSpy spy12(m_series, &QBarSeries::hoverableChanged);
    QSignalSpy spy13(m_series, &QBarSeries::opacityChanged);
    QSignalSpy spy14(m_series, &QBarSeries::valuesMultiplierChanged);

    auto colors1 = QList<QColor> {QColor(255, 255, 255)};
    auto colors2 = QList<QColor> {QColor(0, 0, 0)};
    auto set = new QBarSet(this);

    m_series->setBarWidth(0.75);
    m_series->setLabelsVisible(true);
    m_series->setLabelsFormat("i");
    m_series->setLabelsPosition(QBarSeries::LabelsPosition::InsideBase);
    m_series->setLabelsMargin(10.0);
    m_series->setLabelsAngle(45.0);
    m_series->setLabelsPrecision(10);
    m_series->append(set);

    m_series->setSeriesColors(colors1);
    m_series->setBorderColors(colors2);
    m_series->setName("BarSeries");
    m_series->setVisible(false);
    m_series->setSelectable(true);
    m_series->setHoverable(true);
    m_series->setOpacity(0.5);
    m_series->setValuesMultiplier(0.5);

    QCOMPARE(m_series->barWidth(), 0.75);
    QCOMPARE(m_series->count(), 1);
    QCOMPARE(m_series->labelsVisible(), true);
    QCOMPARE(m_series->labelsFormat(), "i");
    QCOMPARE(m_series->labelsPosition(), QBarSeries::LabelsPosition::InsideBase);
    QCOMPARE(m_series->labelsMargin(), 10.0);
    QCOMPARE(m_series->labelsAngle(), 45.0);
    QCOMPARE(m_series->labelsPrecision(), 10);

    QCOMPARE(m_series->seriesColors(), colors1);
    QCOMPARE(m_series->borderColors(), colors2);
    QCOMPARE(m_series->name(), "BarSeries");
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
    QCOMPARE(spy14.size(), 1);
}

void tst_bars::invalidProperties()
{
    QVERIFY(m_series);

    m_series->setBarWidth(2.0); // range 0...1
    m_series->setValuesMultiplier(2.0); // range 0...1

    QCOMPARE(m_series->barWidth(), 1.0);
    QCOMPARE(m_series->valuesMultiplier(), 1.0);

    m_series->setBarWidth(-1.0); // range 0...1
    m_series->setValuesMultiplier(-1.0); // range 0...1

    QCOMPARE(m_series->barWidth(), 0.0);
    QCOMPARE(m_series->valuesMultiplier(), 0.0);
}

void tst_bars::modifySeries()
{
    QVERIFY(m_series);

    QSignalSpy spy0(m_series, &QBarSeries::barsetsAdded);
    QSignalSpy spy1(m_series, &QBarSeries::barsetsReplaced);
    QSignalSpy spy2(m_series, &QBarSeries::barsetsRemoved);
    QSignalSpy spy3(m_series, &QBarSeries::countChanged);
    QSignalSpy spy4(m_series, &QBarSeries::setValueChanged);
    QSignalSpy spy5(m_series, &QBarSeries::setValueAdded);
    QSignalSpy spy6(m_series, &QBarSeries::setValueRemoved);
    QSignalSpy spy7(m_series, &QBarSeries::barSetsChanged); // unused

    auto set1 = new QBarSet("Set 1", m_series);
    auto set2 = new QBarSet("Set 2", m_series);
    auto set3 = new QBarSet("Set 3", m_series);
    auto set4 = new QBarSet("Set 4", m_series);
    auto set5 = new QBarSet("Set 5", m_series);
    auto set6 = new QBarSet("Set 6", m_series);
    auto set7 = new QBarSet("Set 7", m_series);
    auto newSet = new QBarSet("New Set", m_series);
    auto pointerBarset = new QBarSet("Set pointer", m_series);
    auto insertSet = new QBarSet("insertSet", m_series);

    // append
    m_series->append(set1);
    m_series->append(set2);
    m_series->append(set3);
    m_series->append(set4);
    m_series->append(set5);
    m_series->append(set6);
    m_series->append(set7);

    auto sets = m_series->barSets();

    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], set2);
    QCOMPARE(sets[2], set3);
    QCOMPARE(sets[3], set4);
    QCOMPARE(sets[4], set5);
    QCOMPARE(sets[5], set6);
    QCOMPARE(sets[6], set7);
    QCOMPARE(spy0.size(), 7);
    QCOMPARE(spy3.size(), 7);
    QCOMPARE(spy7.size(), 7);

    // pointer remove
    m_series->remove(set3);
    sets = m_series->barSets();
    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], set2);
    QCOMPARE(sets[2], set4);
    QCOMPARE(sets[3], set5);
    QCOMPARE(sets[4], set6);
    QCOMPARE(sets[5], set7);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 8);
    QCOMPARE(spy7.size(), 8);

    // index remove
    QVERIFY(!m_series->remove(-1));
    QVERIFY(!m_series->remove(100));
    QVERIFY(m_series->remove(2));
    sets = m_series->barSets();
    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], set2);
    QCOMPARE(sets[2], set5);
    QCOMPARE(sets[3], set6);
    QCOMPARE(sets[4], set7);
    QCOMPARE(spy2.size(), 2);
    QCOMPARE(spy3.size(), 9);
    QCOMPARE(spy7.size(), 9);

    // index replace
    m_series->replace(2, newSet);
    sets = m_series->barSets();
    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], set2);
    QCOMPARE(sets[2], newSet);
    QCOMPARE(sets[3], set6);
    QCOMPARE(sets[4], set7);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy7.size(), 10);

    // multiple remove
    m_series->removeMultiple(-1, -1);
    m_series->removeMultiple(0, 100);
    m_series->removeMultiple(1, 2);
    sets = m_series->barSets();
    QCOMPARE(sets.size(), 3);
    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], set6);
    QCOMPARE(sets[2], set7);
    QCOMPARE(spy2.size(), 5);
    QCOMPARE(spy3.size(), 12);
    QCOMPARE(spy7.size(), 12);

    // pointer replace
    QVERIFY(!m_series->replace(nullptr, nullptr));
    QVERIFY(!m_series->replace(pointerBarset, pointerBarset));
    QVERIFY(m_series->replace(set6, pointerBarset));
    sets = m_series->barSets();
    QCOMPARE(sets[0], set1);
    QCOMPARE(sets[1], pointerBarset);
    QCOMPARE(sets[2], set7);
    QCOMPARE(spy1.size(), 2);

    //at
    auto atSet1 = m_series->at(2);
    auto atSet2 = m_series->at(-10);
    auto atSet3 = m_series->at(100);
    QCOMPARE(atSet1, set7);
    QCOMPARE(atSet2, nullptr);
    QCOMPARE(atSet3, nullptr);

    //find
    auto fSet1 = m_series->find(newSet);
    auto fSet2 = m_series->find(set2);
    auto fSet3 = m_series->find(pointerBarset);
    auto fSet4 = m_series->find(set7);

    QCOMPARE(fSet4, 2);
    QCOMPARE(fSet1, -1);
    QCOMPARE(fSet2, -1);
    QCOMPARE(fSet3, 1);

    // take
    QVERIFY(m_series->take(pointerBarset));
    QCOMPARE(m_series->count(), 2);
    QCOMPARE(spy2.size(), 7);
    QCOMPARE(spy3.size(), 14);
    QCOMPARE(spy7.size(), 14);

    // full replace
    QList<QBarSet *> bars = {new QBarSet("new set 1", m_series),
                             new QBarSet("new set 2", m_series),
                             new QBarSet("new set 3", m_series)};
    QVERIFY(m_series->replace(bars));
    sets = m_series->barSets();
    QCOMPARE(sets[0], bars[0]);
    QCOMPARE(sets[1], bars[1]);
    QCOMPARE(sets[2], bars[2]);
    QCOMPARE(spy7.size(), 16);

    //append value to barset
    for (qsizetype i = 0; i < m_series->barSets().size(); ++i)
        m_series->barSets().at(i)->append(10);
    QCOMPARE(spy5.size(), 3);

    //replace value from barset
    for (qsizetype i = 0; i < m_series->barSets().size(); ++i)
        m_series->barSets().at(i)->replace(0, 20);
    QCOMPARE(spy4.size(), 3);

    //remove value from barset
    for (qsizetype i = 0; i < m_series->barSets().size(); ++i)
        m_series->barSets().at(i)->remove(0);
    QCOMPARE(spy6.size(), 3);

    //insert
    m_series->insert(0, insertSet);
    sets = m_series->barSets();
    QCOMPARE(sets[0], insertSet);
    QCOMPARE(spy7.size(), 17);

    m_series->barSets().at(0)->append(10);
    m_series->barSets().at(0)->replace(0, 20);
    m_series->barSets().at(0)->remove(0);

    QCOMPARE(spy0.size(), 8);
    QCOMPARE(spy1.size(), 3);
    QCOMPARE(spy2.size(), 9);
    QCOMPARE(spy3.size(), 17);
    QCOMPARE(spy4.size(), 4);
    QCOMPARE(spy5.size(), 4);
    QCOMPARE(spy6.size(), 4);
    QCOMPARE(spy7.size(), 17);
}

QTEST_MAIN(tst_bars)
#include "tst_bars.moc"
