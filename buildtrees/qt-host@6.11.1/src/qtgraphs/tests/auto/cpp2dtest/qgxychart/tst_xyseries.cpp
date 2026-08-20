// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QScatterSeries>
#include <QtTest/QtTest>

class tst_xyseries : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initialProperties();
    void initializeProperties();

    void selectDeselect();
    void appendInsertRemove();
    void replaceAtClear();
    void find();
    void take();

private:
    // QXYSeries is uncreatable, so testing is done through QScatterSeries
    QScatterSeries *m_series;
};

void tst_xyseries::initTestCase() {}

void tst_xyseries::cleanupTestCase() {}

void tst_xyseries::init()
{
    m_series = new QScatterSeries();
}

void tst_xyseries::cleanup()
{
    delete m_series;
}

void tst_xyseries::initialProperties()
{
    QVERIFY(m_series);

    // Properties from QXYSeries
    QCOMPARE(m_series->color(), QColor(Qt::transparent));
    QCOMPARE(m_series->selectedColor(), QColor(Qt::transparent));
}

void tst_xyseries::initializeProperties()
{
    QVERIFY(m_series);

    m_series->setColor("#ff0000");
    m_series->setSelectedColor("#0000ff");

    QCOMPARE(m_series->color(), "#ff0000");
    QCOMPARE(m_series->selectedColor(), "#0000ff");
}

void tst_xyseries::selectDeselect()
{
    QVERIFY(m_series);

    QList<QPointF> points = {{0, 0}, {1, 1}, {2, 2}};
    QList<qsizetype> allselected = {0, 1, 2};

    m_series->append(points);

    QCOMPARE(m_series->selectedPoints(), {});

    m_series->selectAllPoints();

    QCOMPARE(m_series->selectedPoints().size(), allselected.size());
    for (int i = 0; i < allselected.size(); i++) {
        QCOMPARE(m_series->selectedPoints().contains(allselected[i]), true);
    }

    m_series->deselectAllPoints();

    QCOMPARE(m_series->selectedPoints(), {});

    m_series->selectPoints(allselected);

    QCOMPARE(m_series->selectedPoints().size(), allselected.size());
    for (int i = 0; i < allselected.size(); i++) {
        QCOMPARE(m_series->selectedPoints().contains(allselected[i]), true);
    }

    m_series->toggleSelection(allselected);

    QCOMPARE(m_series->selectedPoints(), {});
}

void tst_xyseries::appendInsertRemove()
{
    QVERIFY(m_series);
    QSignalSpy updateSpy(m_series, &QXYSeries::update);
    QSignalSpy pointAddedSpy(m_series, &QXYSeries::pointAdded);
    QSignalSpy pointsAddedSpy(m_series, &QXYSeries::pointsAdded);
    QSignalSpy pointRemovedSpy(m_series, &QXYSeries::pointRemoved);
    QSignalSpy pointsRemovedSpy(m_series, &QXYSeries::pointsRemoved);

    QList<QPointF> points = {{0, 0}, {1, 1}, {2, 2}};
    QList<QPointF> morepoints = {{3, 3}, {4, 4}, {5, 5}};
    QList<QPointF> allpoints = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    QList<QPointF> mixedpoints = {{0, 0}, {3, 3}, {1, 1}, {4, 4}, {2, 2}, {5, 5}};

    QCOMPARE(updateSpy.count(), 0);

    // Append 3
    for (int i = 0; i < points.count(); ++i)
        m_series->append(points[i]);

    QCOMPARE(updateSpy.count(), 3);
    QCOMPARE(m_series->points(), points);
    QCOMPARE(pointAddedSpy.size(), 3);

    // Append 3 more
    m_series->append(morepoints);

    QCOMPARE(updateSpy.count(), 4);
    QCOMPARE(m_series->points(), allpoints);
    QCOMPARE(pointsAddedSpy.size(), 1);

    // Remove the first 3 one by one
    for (int i = 2; i >= 0; --i)
        m_series->remove(i);

    QCOMPARE(updateSpy.count(), 7);
    QCOMPARE(m_series->points(), morepoints);
    QCOMPARE(pointRemovedSpy.size(), 3);

    // Insert them in between
    m_series->insert(0, points[0]);
    m_series->insert(2, points[1]);
    m_series->insert(4, points[2]);

    QCOMPARE(updateSpy.count(), 10);
    QCOMPARE(m_series->points(), mixedpoints);
    QCOMPARE(pointAddedSpy.size(), 6);

    // Remove first 3
    m_series->removeMultiple(0, 3);

    QCOMPARE(updateSpy.count(), 11);
    QCOMPARE(m_series->count(), 3);
    QCOMPARE(pointsRemovedSpy.size(), 1);

    // Append 3 by qreals
    for (int i = 10; i < 13; ++i)
        m_series->append(i, i);

    QCOMPARE(updateSpy.count(), 14);
    QCOMPARE(m_series->count(), 6);
    QCOMPARE(pointAddedSpy.size(), 9);

    // Remove 3 by qreals
    for (int i = 10; i < 13; ++i)
        m_series->remove(i, i);

    QCOMPARE(updateSpy.count(), 17);
    QCOMPARE(m_series->count(), 3);
    QCOMPARE(pointRemovedSpy.size(), 6);
}

void tst_xyseries::replaceAtClear()
{
    QVERIFY(m_series);

    QList<QPointF> points = {{0, 0}, {1, 1}, {2, 2}};
    QList<QPointF> morepoints = {{3, 3}, {4, 4}, {5, 5}};

    m_series->append(points);

    for (int i = 0; i < m_series->count(); ++i)
        QCOMPARE(m_series->at(i), points[i]);

    for (int i = 0; i < m_series->count(); ++i)
        m_series->replace(i, morepoints[i]);

    for (int i = 0; i < m_series->count(); ++i)
        QCOMPARE(m_series->at(i), morepoints[i]);

    m_series->clear();

    QCOMPARE(m_series->count(), 0);
}

void tst_xyseries::find()
{
    QVERIFY(m_series);
    QList<QPointF> points = {{1, 4}, {9, 2}, {3, 7}, {9, 2}, {8, 8}};

    m_series->append(points);
    auto sPoints = m_series->points();

    QCOMPARE(sPoints, points);

    auto item1 = m_series->find({9, 2});
    auto item2 = m_series->find({1, 4});
    auto item3 = m_series->find({8, 8});
    auto item4 = m_series->find({300, 8});

    QCOMPARE(item1, 1);
    QCOMPARE(item2, 0);
    QCOMPARE(item3, 4);
    QCOMPARE(item4, -1);
}

void tst_xyseries::take()
{
    QVERIFY(m_series);
    QList<QPointF> points = {{1, 4}, {9, 2}, {3, 7}, {9, 2}, {8, 8}};

    m_series->append(points);
    QCOMPARE(m_series->count(), 5);

    QVERIFY(!m_series->take({100, 100}));
    QCOMPARE(m_series->count(), 5);
    QVERIFY(m_series->take({3, 7}));
    QCOMPARE(m_series->count(), 4);
}

QTEST_MAIN(tst_xyseries)
#include "tst_xyseries.moc"
