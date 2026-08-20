// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QString>
#include <QtTest/QtTest>

#include <QtGraphs/QLineSeries>
#include <QtGraphs/QXYModelMapper>
#include <QtGraphs/QXYSeries>
#include <QtGui/QStandardItemModel>

QT_USE_NAMESPACE

class tst_qgxymodelmapper : public QObject
{
    Q_OBJECT

public:
    tst_qgxymodelmapper();
    void createVerticalMapper();
    void createHorizontalMapper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void verticalMapper_data();
    void verticalMapper();
    void verticalMapperCustomMapping_data();
    void verticalMapperCustomMapping();
    void horizontalMapper_data();
    void horizontalMapper();
    void horizontalMapperCustomMapping_data();
    void horizontalMapperCustomMapping();
    void seriesUpdated();
    void verticalModelInsertRows();
    void verticalModelRemoveRows();
    void verticalModelInsertColumns();
    void verticalModelRemoveColumns();
    void horizontalModelInsertRows();
    void horizontalModelRemoveRows();
    void horizontalModelInsertColumns();
    void horizontalModelRemoveColumns();
    void modelUpdateCell();
    void verticalMapperSignals();
    void horizontalMapperSignals();

private:
    QStandardItemModel *m_model;
    int m_modelRowCount;
    int m_modelColumnCount;

    QXYModelMapper *m_hMapper;
    QXYModelMapper *m_vMapper;

    QXYSeries *m_series;
};

tst_qgxymodelmapper::tst_qgxymodelmapper()
    : m_model(0)
    , m_modelRowCount(10)
    , m_modelColumnCount(8)
    , m_hMapper(0)
    , m_vMapper(0)
    , m_series(0)
{}

void tst_qgxymodelmapper::createVerticalMapper()
{
    m_vMapper = new QXYModelMapper;
    QVERIFY(m_vMapper->model() == 0);
    m_vMapper->setXSection(0);
    m_vMapper->setYSection(1);
    m_vMapper->setModel(m_model);
    m_vMapper->setSeries(m_series);
}

void tst_qgxymodelmapper::createHorizontalMapper()
{
    m_hMapper = new QXYModelMapper;
    QVERIFY(m_hMapper->model() == 0);
    m_hMapper->setOrientation(Qt::Horizontal);
    m_hMapper->setXSection(0);
    m_hMapper->setYSection(1);
    m_hMapper->setModel(m_model);
    m_hMapper->setSeries(m_series);
}

void tst_qgxymodelmapper::init()
{
    m_series = new QLineSeries;

    m_model = new QStandardItemModel(m_modelRowCount, m_modelColumnCount, this);
    for (int row = 0; row < m_modelRowCount; ++row) {
        for (int column = 0; column < m_modelColumnCount; column++) {
            m_model->setData(m_model->index(row, column), row * column);
        }
    }
}

void tst_qgxymodelmapper::cleanup()
{
    m_series->deleteLater();
    m_series = 0;

    m_model->clear();
    m_model->deleteLater();
    m_model = 0;

    if (m_vMapper) {
        m_vMapper->deleteLater();
        m_vMapper = 0;
    }

    if (m_hMapper) {
        m_hMapper->deleteLater();
        m_hMapper = 0;
    }
}

void tst_qgxymodelmapper::initTestCase() {}

void tst_qgxymodelmapper::cleanupTestCase()
{
    QTest::qWait(1); // Allow final deleteLaters to run
}

void tst_qgxymodelmapper::verticalMapper_data()
{
    QTest::addColumn<int>("xColumn");
    QTest::addColumn<int>("yColumn");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("different x and y columns") << 0 << 1 << m_modelRowCount;
    QTest::newRow("same x and y columns") << 1 << 1 << m_modelRowCount;
    QTest::newRow("invalid x column and correct y column") << -3 << 1 << 0;
    QTest::newRow("x column beyond the size of model and correct y column")
        << m_modelColumnCount << 1 << 0;
    QTest::newRow("x column beyond the size of model and invalid y column")
        << m_modelColumnCount << -1 << 0;
}

void tst_qgxymodelmapper::verticalMapper()
{
    QFETCH(int, xColumn);
    QFETCH(int, yColumn);
    QFETCH(int, expectedCount);

    QXYModelMapper *mapper = new QXYModelMapper;
    QVERIFY(mapper->model() == 0);

    mapper->setXSection(xColumn);
    mapper->setYSection(yColumn);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);

    QCOMPARE(m_series->count(), expectedCount);
    QCOMPARE(mapper->xSection(), qMax(-1, xColumn));
    QCOMPARE(mapper->ySection(), qMax(-1, yColumn));

    delete mapper;
    mapper = 0;
}

void tst_qgxymodelmapper::verticalMapperCustomMapping_data()
{
    QTest::addColumn<int>("first");
    QTest::addColumn<int>("countLimit");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("first: 0, unlimited count") << 0 << -1 << m_modelRowCount;
    QTest::newRow("first: 3, unlimited count") << 3 << -1 << m_modelRowCount - 3;
    QTest::newRow("first: 0, count: 5") << 0 << 5 << qMin(5, m_modelRowCount);
    QTest::newRow("first: 3, count: 5") << 3 << 5 << qMin(5, m_modelRowCount - 3);
    QTest::newRow("first: +1 greater then the number of rows in the model, unlimited count")
        << m_modelRowCount + 1 << -1 << 0;
    QTest::newRow("first: +1 greater then the number of rows in the model, count: 5")
        << m_modelRowCount + 1 << 5 << 0;
    QTest::newRow("first: 0, count: +3 greater than the number of rows in the model (should limit "
                  "to the size of model)")
        << 0 << m_modelRowCount + 3 << m_modelRowCount;
    QTest::newRow("first: -3(invalid - should default to 0), unlimited count")
        << -3 << -1 << m_modelRowCount;
    QTest::newRow("first: 0, count: -3 (invalid - shlould default to -1)")
        << 0 << -3 << m_modelRowCount;
    QTest::newRow(
        "first: -3(invalid - should default to 0), count: -3 (invalid - shlould default to -1)")
        << -3 << -3 << m_modelRowCount;
}

void tst_qgxymodelmapper::verticalMapperCustomMapping()
{
    QFETCH(int, first);
    QFETCH(int, countLimit);
    QFETCH(int, expectedCount);

    QCOMPARE(m_series->count(), 0);

    QXYModelMapper *mapper = new QXYModelMapper;
    mapper->setXSection(0);
    mapper->setYSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);
    mapper->setFirst(first);
    mapper->setCount(countLimit);

    QCOMPARE(m_series->count(), expectedCount);

    // change values column mapping to invalid
    mapper->setXSection(-1);
    mapper->setYSection(1);

    QCOMPARE(m_series->count(), 0);

    delete mapper;
    mapper = 0;
}

void tst_qgxymodelmapper::horizontalMapper_data()
{
    QTest::addColumn<int>("xRow");
    QTest::addColumn<int>("yRow");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("different x and y rows") << 0 << 1 << m_modelColumnCount;
    QTest::newRow("same x and y rows") << 1 << 1 << m_modelColumnCount;
    QTest::newRow("invalid x row and correct y row") << -3 << 1 << 0;
    QTest::newRow("x row beyond the size of model and correct y row") << m_modelRowCount << 1 << 0;
    QTest::newRow("x row beyond the size of model and invalid y row") << m_modelRowCount << -1 << 0;
}

void tst_qgxymodelmapper::horizontalMapper()
{
    QFETCH(int, xRow);
    QFETCH(int, yRow);
    QFETCH(int, expectedCount);

    QXYModelMapper *mapper = new QXYModelMapper;
    mapper->setOrientation(Qt::Horizontal);
    mapper->setXSection(xRow);
    mapper->setYSection(yRow);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);

    QCOMPARE(m_series->count(), expectedCount);
    QCOMPARE(mapper->xSection(), qMax(-1, xRow));
    QCOMPARE(mapper->ySection(), qMax(-1, yRow));

    delete mapper;
    mapper = 0;
}

void tst_qgxymodelmapper::horizontalMapperCustomMapping_data()
{
    QTest::addColumn<int>("first");
    QTest::addColumn<int>("countLimit");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("first: 0, unlimited count") << 0 << -1 << m_modelColumnCount;
    QTest::newRow("first: 3, unlimited count") << 3 << -1 << m_modelColumnCount - 3;
    QTest::newRow("first: 0, count: 5") << 0 << 5 << qMin(5, m_modelColumnCount);
    QTest::newRow("first: 3, count: 5") << 3 << 5 << qMin(5, m_modelColumnCount - 3);
    QTest::newRow("first: +1 greater then the number of columns in the model, unlimited count")
        << m_modelColumnCount + 1 << -1 << 0;
    QTest::newRow("first: +1 greater then the number of columns in the model, count: 5")
        << m_modelColumnCount + 1 << 5 << 0;
    QTest::newRow("first: 0, count: +3 greater than the number of columns in the model (should "
                  "limit to the size of model)")
        << 0 << m_modelColumnCount + 3 << m_modelColumnCount;
    QTest::newRow("first: -3(invalid - should default to 0), unlimited count")
        << -3 << -1 << m_modelColumnCount;
    QTest::newRow("first: 0, count: -3 (invalid - shlould default to -1)")
        << 0 << -3 << m_modelColumnCount;
    QTest::newRow(
        "first: -3(invalid - should default to 0), count: -3 (invalid - shlould default to -1)")
        << -3 << -3 << m_modelColumnCount;
}

void tst_qgxymodelmapper::horizontalMapperCustomMapping()
{
    QFETCH(int, first);
    QFETCH(int, countLimit);
    QFETCH(int, expectedCount);

    QCOMPARE(m_series->count(), 0);

    QXYModelMapper *mapper = new QXYModelMapper;
    mapper->setOrientation(Qt::Horizontal);
    mapper->setXSection(0);
    mapper->setYSection(1);
    mapper->setModel(m_model);
    mapper->setSeries(m_series);
    mapper->setFirst(first);
    mapper->setCount(countLimit);

    QCOMPARE(m_series->count(), expectedCount);

    // change values row mapping to invalid
    mapper->setXSection(-1);
    mapper->setYSection(1);

    QCOMPARE(m_series->count(), 0);

    delete mapper;
    mapper = 0;
}

void tst_qgxymodelmapper::seriesUpdated()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(), m_modelRowCount);
    QCOMPARE(m_vMapper->count(), -1);

    m_series->append(QPointF(100, 100));
    QCOMPARE(m_series->count(), m_modelRowCount + 1);
    QCOMPARE(m_vMapper->count(),
             -1); // the value should not change as it indicates 'all' items there are in the model

    m_series->remove(m_series->points().last());
    QCOMPARE(m_series->count(), m_modelRowCount);
    QCOMPARE(m_vMapper->count(),
             -1); // the value should not change as it indicates 'all' items there are in the model

    m_series->removeMultiple(1, m_modelRowCount - 4);
    QCOMPARE(m_series->count(), 4);
    QCOMPARE(m_vMapper->count(),
             -1); // the value should not change as it indicates 'all' items there are in the model

    m_series->replace(m_series->points().first(), QPointF(25.0, 75.0));
    QCOMPARE(m_model->data(m_model->index(0, 0)).toReal(), 25.0);
    QCOMPARE(m_model->data(m_model->index(0, 1)).toReal(), 75.0);
}

void tst_qgxymodelmapper::verticalModelInsertRows()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int insertCount = 4;
    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(), m_modelRowCount + insertCount);

    int first = 3;
    m_vMapper->setFirst(3);
    QCOMPARE(m_series->count(), m_modelRowCount + insertCount - first);

    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(), m_modelRowCount + 2 * insertCount - first);

    int countLimit = 6;
    m_vMapper->setCount(countLimit);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount + 2 * insertCount - first));

    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount + 3 * insertCount - first));

    m_vMapper->setFirst(0);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount + 3 * insertCount));

    m_vMapper->setCount(-1);
    QCOMPARE(m_series->count(), m_modelRowCount + 3 * insertCount);
}

void tst_qgxymodelmapper::verticalModelRemoveRows()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int removeCount = 2;
    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(), m_modelRowCount - removeCount);

    int first = 1;
    m_vMapper->setFirst(first);
    QCOMPARE(m_series->count(), m_modelRowCount - removeCount - first);

    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(), m_modelRowCount - 2 * removeCount - first);

    int countLimit = 3;
    m_vMapper->setCount(countLimit);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount - 2 * removeCount - first));

    m_model->removeRows(1, removeCount);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount - 3 * removeCount - first));

    m_vMapper->setFirst(0);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelRowCount - 3 * removeCount));

    m_vMapper->setCount(-1);
    QCOMPARE(m_series->count(), m_modelRowCount - 3 * removeCount);
}

void tst_qgxymodelmapper::verticalModelInsertColumns()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int insertCount = 4;
    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(), m_modelRowCount);
}

void tst_qgxymodelmapper::verticalModelRemoveColumns()
{
    // setup the mapper
    createVerticalMapper();
    QCOMPARE(m_series->count(), m_modelRowCount);
    QVERIFY(m_vMapper->model() != 0);

    int removeCount = m_modelColumnCount - 2;
    m_model->removeColumns(0, removeCount);
    QCOMPARE(m_series->count(), m_modelRowCount);

    // leave only one column
    m_model->removeColumns(0, m_modelColumnCount - removeCount - 1);
    QCOMPARE(m_series->count(), 0);
}

void tst_qgxymodelmapper::horizontalModelInsertRows()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int insertCount = 4;
    m_model->insertRows(3, insertCount);
    QCOMPARE(m_series->count(), m_modelColumnCount);
}

void tst_qgxymodelmapper::horizontalModelRemoveRows()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int removeCount = m_modelRowCount - 2;
    m_model->removeRows(0, removeCount);
    QCOMPARE(m_series->count(), m_modelColumnCount);

    // leave only one column
    m_model->removeRows(0, m_modelRowCount - removeCount - 1);
    QCOMPARE(m_series->count(), 0);
}

void tst_qgxymodelmapper::horizontalModelInsertColumns()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int insertCount = 4;
    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(), m_modelColumnCount + insertCount);

    int first = 3;
    m_hMapper->setFirst(3);
    QCOMPARE(m_series->count(), m_modelColumnCount + insertCount - first);

    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(), m_modelColumnCount + 2 * insertCount - first);

    int countLimit = 6;
    m_hMapper->setCount(countLimit);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount + 2 * insertCount - first));

    m_model->insertColumns(3, insertCount);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount + 3 * insertCount - first));

    m_hMapper->setFirst(0);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount + 3 * insertCount));

    m_hMapper->setCount(-1);
    QCOMPARE(m_series->count(), m_modelColumnCount + 3 * insertCount);
}

void tst_qgxymodelmapper::horizontalModelRemoveColumns()
{
    // setup the mapper
    createHorizontalMapper();
    QCOMPARE(m_series->count(), m_modelColumnCount);
    QVERIFY(m_hMapper->model() != 0);

    int removeCount = 2;
    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(), m_modelColumnCount - removeCount);

    int first = 1;
    m_hMapper->setFirst(first);
    QCOMPARE(m_series->count(), m_modelColumnCount - removeCount - first);

    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(), m_modelColumnCount - 2 * removeCount - first);

    int countLimit = 3;
    m_hMapper->setCount(countLimit);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount - 2 * removeCount - first));

    m_model->removeColumns(1, removeCount);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount - 3 * removeCount - first));

    m_hMapper->setFirst(0);
    QCOMPARE(m_series->count(), qMin(countLimit, m_modelColumnCount - 3 * removeCount));

    m_hMapper->setCount(-1);
    QCOMPARE(m_series->count(), m_modelColumnCount - 3 * removeCount);
}

void tst_qgxymodelmapper::modelUpdateCell()
{
    // setup the mapper
    createVerticalMapper();

    QVERIFY(m_model->setData(m_model->index(1, 0), 44));
    QCOMPARE(m_series->points().at(1).x(), 44.0);
    QCOMPARE(m_model->data(m_model->index(1, 0)).toReal(), 44.0);
}

void tst_qgxymodelmapper::verticalMapperSignals()
{
    QXYModelMapper *mapper = new QXYModelMapper;

    QSignalSpy spy0(mapper, SIGNAL(firstChanged()));
    QSignalSpy spy1(mapper, SIGNAL(countChanged()));
    QSignalSpy spy2(mapper, SIGNAL(xSectionChanged()));
    QSignalSpy spy3(mapper, SIGNAL(ySectionChanged()));
    QSignalSpy spy4(mapper, SIGNAL(modelChanged()));
    QSignalSpy spy5(mapper, SIGNAL(seriesChanged()));

    mapper->setXSection(0);
    mapper->setYSection(1);
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

void tst_qgxymodelmapper::horizontalMapperSignals()
{
    QXYModelMapper *mapper = new QXYModelMapper;

    QSignalSpy spy0(mapper, SIGNAL(firstChanged()));
    QSignalSpy spy1(mapper, SIGNAL(countChanged()));
    QSignalSpy spy2(mapper, SIGNAL(xSectionChanged()));
    QSignalSpy spy3(mapper, SIGNAL(ySectionChanged()));
    QSignalSpy spy4(mapper, SIGNAL(modelChanged()));
    QSignalSpy spy5(mapper, SIGNAL(seriesChanged()));

    mapper->setXSection(0);
    mapper->setYSection(1);
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

QTEST_MAIN(tst_qgxymodelmapper)

#include "tst_xymodelmapper.moc"
