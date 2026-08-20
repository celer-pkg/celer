// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QStandardItemModel>
#include <QtGraphs/QBarModelMapper>
#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtTest/QtTest>

class tst_barmodelmapper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void verticalMapper_data();
    void verticalMapper();
    void verticalMapperCustomMapping_data();
    void verticalMapperCustomMapping();
    void verticalModelInsertRows();
    void verticalModelRemoveRows();
    void verticalModelInsertColumns();
    void verticalModelRemoveColumns();
    void verticalMapperSignals();

    void horizontalMapper_data();
    void horizontalMapper();
    void horizontalMapperCustomMapping_data();
    void horizontalMapperCustomMapping();
    void horizontalModelInsertRows();
    void horizontalModelRemoveRows();
    void horizontalModelInsertColumns();
    void horizontalModelRemoveColumns();
    void horizontalMapperSignals();

    void construct();
    void seriesUpdated();
    void modelUpdateCell();

private:
    QBarSeries *m_series = nullptr;
    QStandardItemModel *m_model = nullptr;
    int m_modelRowCount = 10;
    int m_modelColumnCount = 8;
    QBarModelMapper *m_vMapper = nullptr;
    QBarModelMapper *m_hMapper = nullptr;

    void createVerticalMapper();
    void createHorizontalMapper();
};

void tst_barmodelmapper::initTestCase() {}

void tst_barmodelmapper::cleanupTestCase() {}

void tst_barmodelmapper::init()
{
    m_series = new QBarSeries();
    m_model = new QStandardItemModel(m_modelRowCount, m_modelColumnCount, this);
    for (int row = 0; row < m_modelRowCount; ++row) {
        for (int column = 0; column < m_modelColumnCount; column++) {
            m_model->setData(m_model->index(row, column), row * column);
        }
    }
}

void tst_barmodelmapper::cleanup() {}

void tst_barmodelmapper::verticalMapper_data()
{
    QTest::addColumn<int>("firstBarSetColumn");
    QTest::addColumn<int>("lastBarSetColumn");
    QTest::addColumn<int>("expectedBarSetCount");
    QTest::newRow("lastBarSetColumn greater than firstBarSetColumn") << 0 << 1 << 2;
    QTest::newRow("lastBarSetColumn equal to firstBarSetColumn") << 1 << 1 << 1;
    QTest::newRow("lastBarSetColumn lesser than firstBarSetColumn") << 1 << 0 << 0;
    QTest::newRow("invalid firstBarSetColumn and correct lastBarSetColumn") << -3 << 1 << 0;
    QTest::newRow("firstBarSetColumn beyond the size of model and correct lastBarSetColumn")
        << m_modelColumnCount << 1 << 0;
    QTest::newRow("firstBarSetColumn beyond the size of model and invalid lastBarSetColumn")
        << m_modelColumnCount << -1 << 0;
}

void tst_barmodelmapper::verticalMapper()
{
    QFETCH(int, firstBarSetColumn);
    QFETCH(int, lastBarSetColumn);
    QFETCH(int, expectedBarSetCount);

    QBarSeries *series = new QBarSeries;

    QBarModelMapper *mapper = new QBarModelMapper;
    mapper->setOrientation(Qt::Vertical);
    mapper->setFirstBarSetSection(firstBarSetColumn);
    mapper->setLastBarSetSection(lastBarSetColumn);
    mapper->setModel(m_model);
    mapper->setSeries(series);

    QCOMPARE(series->count(), expectedBarSetCount);
    QCOMPARE(mapper->firstBarSetSection(), qMax(-1, firstBarSetColumn));
    QCOMPARE(mapper->lastBarSetSection(), qMax(-1, lastBarSetColumn));

    delete mapper;
    delete series;
}

void tst_barmodelmapper::verticalMapperCustomMapping_data()
{
    QTest::addColumn<int>("first");
    QTest::addColumn<int>("countLimit");
    QTest::addColumn<int>("expectedBarSetCount");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("first: 0, unlimited count") << 0 << -1 << 2 << m_modelRowCount;
    QTest::newRow("first: 3, unlimited count") << 3 << -1 << 2 << m_modelRowCount - 3;
    QTest::newRow("first: 0, count: 5") << 0 << 5 << 2 << qMin(5, m_modelRowCount);
    QTest::newRow("first: 3, count: 5") << 3 << 5 << 2 << qMin(5, m_modelRowCount - 3);
    QTest::newRow("first: +1 greater then the number of rows in the model, unlimited count")
        << m_modelRowCount + 1 << -1 << 0 << 0;
    QTest::newRow("first: +1 greater then the number of rows in the model, count: 5")
        << m_modelRowCount + 1 << 5 << 0 << 0;
    QTest::newRow("first: 0, count: +3 greater than the number of rows in the model (should limit "
                  "to the size of model)")
        << 0 << m_modelRowCount + 3 << 2 << m_modelRowCount;
    QTest::newRow("first: -3(invalid - should default to 0), unlimited count")
        << -3 << -1 << 2 << m_modelRowCount;
    QTest::newRow("first: 0, count: -3 (invalid - shlould default to -1)")
        << 0 << -3 << 2 << m_modelRowCount;
    QTest::newRow(
        "first: -3(invalid - should default to 0), count: -3 (invalid - shlould default to -1)")
        << -3 << -3 << 2 << m_modelRowCount;
}

void tst_barmodelmapper::verticalMapperCustomMapping()
{
    QFETCH(int, first);
    QFETCH(int, countLimit);
    QFETCH(int, expectedBarSetCount);
    QFETCH(int, expectedCount);

    QBarSeries *series = new QBarSeries;

    QCOMPARE(series->count(), 0);

    QBarModelMapper *mapper = new QBarModelMapper;
    mapper->setFirstBarSetSection(0);
    mapper->setLastBarSetSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(series);
    mapper->setFirst(first);
    mapper->setCount(countLimit);

    QCOMPARE(series->count(), expectedBarSetCount);

    if (expectedBarSetCount > 0)
        QCOMPARE(series->barSets().first()->count(), expectedCount);

    // change values column mapping to invalid
    mapper->setFirstBarSetSection(-1);
    mapper->setLastBarSetSection(1);

    QCOMPARE(series->count(), 0);

    delete mapper;
    delete series;
}

void tst_barmodelmapper::verticalModelInsertRows()
{ // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int insertCount = 4;
    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount + insertCount);

    int first = 3;
    m_vMapper->setFirst(3);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount + insertCount - first);

    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount + 2 * insertCount - first);

    int countLimit = 6;
    m_vMapper->setCount(countLimit);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount + 2 * insertCount - first));

    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount + 3 * insertCount - first));

    m_vMapper->setFirst(0);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount + 3 * insertCount));

    m_vMapper->setCount(-1);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount + 3 * insertCount);
}

void tst_barmodelmapper::verticalModelRemoveRows()
{ // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int removeCount = 2;
    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount - removeCount);

    int first = 1;
    m_vMapper->setFirst(first);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount - removeCount - first);

    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount - 2 * removeCount - first);

    int countLimit = 3;
    m_vMapper->setCount(countLimit);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount - 2 * removeCount - first));

    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount - 3 * removeCount - first));

    m_vMapper->setFirst(0);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelRowCount - 3 * removeCount));

    m_vMapper->setCount(-1);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount - 3 * removeCount);
}

void tst_barmodelmapper::verticalModelInsertColumns()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int insertCount = 4;
    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(),
             m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
}

void tst_barmodelmapper::verticalModelRemoveColumns()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(),
             qMin(m_model->columnCount(),
                  m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1));
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int removeCount = m_modelColumnCount - 2;
    m_model->removeColumns(0, removeCount);
    QCOMPARE(m_series->count(),
             qMin(m_model->columnCount(),
                  m_vMapper->lastBarSetSection() - m_vMapper->firstBarSetSection() + 1));
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);

    // leave all the columns
    m_model->removeColumns(0, m_modelColumnCount - removeCount);
    QCOMPARE(m_series->count(), 0);
}

void tst_barmodelmapper::verticalMapperSignals()
{
    QBarModelMapper *mapper = new QBarModelMapper;

    QSignalSpy spy0(mapper, SIGNAL(firstChanged()));
    QSignalSpy spy1(mapper, SIGNAL(countChanged()));
    QSignalSpy spy2(mapper, SIGNAL(firstBarSetSectionChanged()));
    QSignalSpy spy3(mapper, SIGNAL(lastBarSetSectionChanged()));
    QSignalSpy spy4(mapper, SIGNAL(modelChanged()));
    QSignalSpy spy5(mapper, SIGNAL(seriesChanged()));

    mapper->setFirstBarSetSection(0);
    mapper->setLastBarSetSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);
    mapper->setFirst(1);
    mapper->setCount(5);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);

    delete mapper;
}

void tst_barmodelmapper::horizontalMapper_data()
{
    QTest::addColumn<int>("firstBarSetRow");
    QTest::addColumn<int>("lastBarSetRow");
    QTest::addColumn<int>("expectedBarSetCount");
    QTest::newRow("lastBarSetRow greater than firstBarSetRow") << 0 << 1 << 2;
    QTest::newRow("lastBarSetRow equal to firstBarSetRow") << 1 << 1 << 1;
    QTest::newRow("lastBarSetRow lesser than firstBarSetRow") << 1 << 0 << 0;
    QTest::newRow("invalid firstBarSetRow and correct lastBarSetRow") << -3 << 1 << 0;
    QTest::newRow("firstBarSetRow beyond the size of model and correct lastBarSetRow")
        << m_modelRowCount << 1 << 0;
    QTest::newRow("firstBarSetRow beyond the size of model and invalid lastBarSetRow")
        << m_modelRowCount << -1 << 0;
}

void tst_barmodelmapper::horizontalMapper()
{
    QFETCH(int, firstBarSetRow);
    QFETCH(int, lastBarSetRow);
    QFETCH(int, expectedBarSetCount);

    QBarSeries *series = new QBarSeries;

    QBarModelMapper *mapper = new QBarModelMapper;
    mapper->setOrientation(Qt::Horizontal);
    mapper->setFirstBarSetSection(firstBarSetRow);
    mapper->setLastBarSetSection(lastBarSetRow);
    mapper->setModel(m_model);
    mapper->setSeries(series);

    QCOMPARE(series->count(), expectedBarSetCount);
    QCOMPARE(mapper->firstBarSetSection(), qMax(-1, firstBarSetRow));
    QCOMPARE(mapper->lastBarSetSection(), qMax(-1, lastBarSetRow));

    delete mapper;
    delete series;
}

void tst_barmodelmapper::horizontalMapperCustomMapping_data()
{
    QTest::addColumn<int>("first");
    QTest::addColumn<int>("countLimit");
    QTest::addColumn<int>("expectedBarSetCount");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("first: 0, unlimited count") << 0 << -1 << 2 << m_modelColumnCount;
    QTest::newRow("first: 3, unlimited count") << 3 << -1 << 2 << m_modelColumnCount - 3;
    QTest::newRow("first: 0, count: 5") << 0 << 5 << 2 << qMin(5, m_modelColumnCount);
    QTest::newRow("first: 3, count: 5") << 3 << 5 << 2 << qMin(5, m_modelColumnCount - 3);
    QTest::newRow("first: +1 greater then the number of rows in the model, unlimited count")
        << m_modelColumnCount + 1 << -1 << 0 << 0;
    QTest::newRow("first: +1 greater then the number of rows in the model, count: 5")
        << m_modelColumnCount + 1 << 5 << 0 << 0;
    QTest::newRow("first: 0, count: +3 greater than the number of rows in the model (should limit "
                  "to the size of model)")
        << 0 << m_modelColumnCount + 3 << 2 << m_modelColumnCount;
    QTest::newRow("first: -3(invalid - should default to 0), unlimited count")
        << -3 << -1 << 2 << m_modelColumnCount;
    QTest::newRow("first: 0, count: -3 (invalid - shlould default to -1)")
        << 0 << -3 << 2 << m_modelColumnCount;
    QTest::newRow(
        "first: -3(invalid - should default to 0), count: -3 (invalid - shlould default to -1)")
        << -3 << -3 << 2 << m_modelColumnCount;
}

void tst_barmodelmapper::horizontalMapperCustomMapping()
{
    QFETCH(int, first);
    QFETCH(int, countLimit);
    QFETCH(int, expectedBarSetCount);
    QFETCH(int, expectedCount);

    QBarSeries *series = new QBarSeries;

    QCOMPARE(series->count(), 0);

    QBarModelMapper *mapper = new QBarModelMapper;
    mapper->setOrientation(Qt::Horizontal);
    mapper->setFirstBarSetSection(0);
    mapper->setLastBarSetSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(series);
    mapper->setFirst(first);
    mapper->setCount(countLimit);

    QCOMPARE(series->count(), expectedBarSetCount);

    if (expectedBarSetCount > 0)
        QCOMPARE(series->barSets().first()->count(), expectedCount);

    // change values column mapping to invalid
    mapper->setFirstBarSetSection(-1);
    mapper->setLastBarSetSection(1);

    QCOMPARE(series->count(), 0);

    delete mapper;
    delete series;
}

void tst_barmodelmapper::horizontalModelInsertRows()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int insertCount = 4;
    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);
}

void tst_barmodelmapper::horizontalModelRemoveRows()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(),
             qMin(m_model->rowCount(),
                  m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1));
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int removeCount = m_modelRowCount - 2;
    m_model->removeRows(0, removeCount);
    QCOMPARE(m_series->count(),
             qMin(m_model->rowCount(),
                  m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1));
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);

    // leave all the columns
    m_model->removeRows(0, m_modelRowCount - removeCount);
    QCOMPARE(m_series->count(), 0);
}

void tst_barmodelmapper::horizontalModelInsertColumns()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int insertCount = 4;
    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount + insertCount);

    int first = 3;
    m_hMapper->setFirst(3);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount + insertCount - first);

    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount + 2 * insertCount - first);

    int countLimit = 6;
    m_hMapper->setCount(countLimit);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount + 2 * insertCount - first));

    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount + 3 * insertCount - first));

    m_hMapper->setFirst(0);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount + 3 * insertCount));

    m_hMapper->setCount(-1);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount + 3 * insertCount);
}

void tst_barmodelmapper::horizontalModelRemoveColumns()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int removeCount = 2;
    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount - removeCount);

    int first = 1;
    m_hMapper->setFirst(first);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount - removeCount - first);

    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount - 2 * removeCount - first);

    int countLimit = 3;
    m_hMapper->setCount(countLimit);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount - 2 * removeCount - first));

    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount - 3 * removeCount - first));

    m_hMapper->setFirst(0);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(),
             qMin(countLimit, m_modelColumnCount - 3 * removeCount));

    m_hMapper->setCount(-1);
    QCOMPARE(m_series->count(),
             m_hMapper->lastBarSetSection() - m_hMapper->firstBarSetSection() + 1);
    QCOMPARE(m_series->barSets().first()->count(), m_modelColumnCount - 3 * removeCount);
}

void tst_barmodelmapper::horizontalMapperSignals()
{
    QBarModelMapper *mapper = new QBarModelMapper;

    QSignalSpy spy0(mapper, SIGNAL(firstChanged()));
    QSignalSpy spy1(mapper, SIGNAL(countChanged()));
    QSignalSpy spy2(mapper, SIGNAL(firstBarSetSectionChanged()));
    QSignalSpy spy3(mapper, SIGNAL(lastBarSetSectionChanged()));
    QSignalSpy spy4(mapper, SIGNAL(modelChanged()));
    QSignalSpy spy5(mapper, SIGNAL(seriesChanged()));

    mapper->setFirstBarSetSection(0);
    mapper->setLastBarSetSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);
    mapper->setFirst(1);
    mapper->setCount(5);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);

    delete mapper;
}

void tst_barmodelmapper::construct()
{
    auto mapper = new QBarModelMapper();
    QVERIFY(mapper);
    delete mapper;
}

void tst_barmodelmapper::createVerticalMapper()
{
    m_vMapper = new QBarModelMapper;
    QVERIFY(m_vMapper->model() == nullptr);
    m_vMapper->setFirstBarSetSection(0);
    m_vMapper->setLastBarSetSection(4);
    m_vMapper->setModel(m_model);
    m_vMapper->setSeries(m_series);
}

void tst_barmodelmapper::createHorizontalMapper()
{
    m_hMapper = new QBarModelMapper;
    QVERIFY(m_hMapper->model() == 0);
    m_hMapper->setOrientation(Qt::Horizontal);
    m_hMapper->setFirstBarSetSection(0);
    m_hMapper->setLastBarSetSection(4);
    m_hMapper->setModel(m_model);
    m_hMapper->setSeries(m_series);
}
void tst_barmodelmapper::seriesUpdated()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->barSets().first()->count(), m_modelRowCount);
    QCOMPARE(m_vMapper->count(), -1);

    m_series->barSets().first()->append(123);
    QCOMPARE(m_model->rowCount(), m_modelRowCount + 1);
    QCOMPARE(m_vMapper->count(),
             -1); // the value should not change as it indicates 'all' items there are in the model

    m_series->barSets().last()->remove(0, m_modelRowCount);
    QCOMPARE(m_model->rowCount(), 1);
    QCOMPARE(m_vMapper->count(),
             -1); // the value should not change as it indicates 'all' items there are in the model

    m_series->barSets().first()->replace(0, 444.0);
    QCOMPARE(m_model->data(m_model->index(0, 0)).toReal(), 444.0);

    m_series->barSets().first()->setLabel("Hello");
    QCOMPARE(m_model->headerData(0, Qt::Horizontal).toString(), QString("Hello"));

    QList<qreal> newValues;
    newValues << 15 << 27 << 35 << 49;
    m_series->barSets().first()->append(newValues);
    QCOMPARE(m_model->rowCount(), 1 + newValues.size());

    QList<QBarSet *> newBarSets;
    QBarSet *newBarSet_1 = new QBarSet("New_1");
    newBarSet_1->append(101);
    newBarSet_1->append(102);
    newBarSet_1->append(103);
    newBarSets.append(newBarSet_1);

    QBarSet *newBarSet_2 = new QBarSet("New_2");
    newBarSet_2->append(201);
    newBarSet_2->append(202);
    newBarSet_2->append(203);
    newBarSets.append(newBarSet_2);

    m_series->append(newBarSets);
    QCOMPARE(m_model->columnCount(), m_modelColumnCount + newBarSets.size());
}

void tst_barmodelmapper::modelUpdateCell()
{
    // setup the mapper
    createVerticalMapper();

    QVERIFY(m_model->setData(m_model->index(1, 0), 44));
    QCOMPARE(m_series->barSets().at(0)->at(1), 44.0);
    QCOMPARE(m_model->data(m_model->index(1, 0)).toReal(), 44.0);
}

QTEST_MAIN(tst_barmodelmapper)
#include "tst_barmodelmapper.moc"
