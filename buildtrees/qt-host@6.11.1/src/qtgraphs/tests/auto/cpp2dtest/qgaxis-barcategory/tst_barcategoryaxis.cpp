// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtestcase.h"
#include <QtGraphs/QBarCategoryAxis>
#include <QtTest/QtTest>

class tst_barcategoryaxis : public QObject
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

    void appendInsertRemove();
    void replaceClear();

private:
    QBarCategoryAxis *m_axis;
};

void tst_barcategoryaxis::initTestCase() {}

void tst_barcategoryaxis::cleanupTestCase() {}

void tst_barcategoryaxis::init()
{
    m_axis = new QBarCategoryAxis();
}

void tst_barcategoryaxis::cleanup()
{
    delete m_axis;
}

void tst_barcategoryaxis::construct()
{
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_barcategoryaxis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->categories(), {});
    QCOMPARE(m_axis->min(), "");
    QCOMPARE(m_axis->max(), "");
    QCOMPARE(m_axis->count(), 0);
    QCOMPARE(m_axis->labelPosition(), QBarCategoryAxis::LabelPosition::Center);
}

void tst_barcategoryaxis::initializeProperties()
{
    QVERIFY(m_axis);

    QSignalSpy spy0(m_axis, &QBarCategoryAxis::categoriesChanged);
    QSignalSpy spy1(m_axis, &QBarCategoryAxis::minChanged);
    QSignalSpy spy2(m_axis, &QBarCategoryAxis::maxChanged);
    QSignalSpy spy3(m_axis, &QBarCategoryAxis::categoryRangeChanged);
    QSignalSpy spy4(m_axis, &QBarCategoryAxis::countChanged);
    QSignalSpy spy5(m_axis, &QBarCategoryAxis::labelPositionChanged);

    QStringList cats = {"One", "Two", "Three"};

    m_axis->setCategories(cats);

    QCOMPARE(m_axis->categories(), cats);
    QCOMPARE(m_axis->min(), "One");
    QCOMPARE(m_axis->max(), "Three");
    QCOMPARE(m_axis->count(), 3);

    m_axis->setMin("Zero");
    m_axis->setMax("Ten");

    // TODO: QTBUG-121718
    // QCOMPARE(m_axis->min(), "Zero");
    // QCOMPARE(m_axis->max(), "Ten");

    m_axis->setRange("Zero", "Ten");

    m_axis->setLabelPosition(QBarCategoryAxis::LabelPosition::OnValue);
    QCOMPARE(m_axis->labelPosition(), QBarCategoryAxis::LabelPosition::OnValue);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
    QCOMPARE(spy4.size(), 1);
    QCOMPARE(spy5.size(), 1);

    // TODO: QTBUG-121718
    // QCOMPARE(m_axis->min(), "Zero");
    // QCOMPARE(m_axis->max(), "Ten");
}

void tst_barcategoryaxis::appendInsertRemove()
{
    QVERIFY(m_axis);

    QSignalSpy spy0(m_axis, &QBarCategoryAxis::categoriesChanged);
    QSignalSpy spy1(m_axis, &QBarCategoryAxis::minChanged);
    QSignalSpy spy2(m_axis, &QBarCategoryAxis::maxChanged);
    QSignalSpy spy3(m_axis, &QBarCategoryAxis::categoryRangeChanged);
    QSignalSpy spy4(m_axis, &QBarCategoryAxis::countChanged);

    QStringList cats = {"One", "Two", "Three"};
    QStringList morecats = {"Four", "Five", "Six"};
    QStringList allcats = cats + morecats;
    QStringList mixedcats = {"One", "Four", "Two", "Five", "Six", "Three"};

    // Append 3
    for (int i = 0; i < cats.count(); ++i)
        m_axis->append(cats[i]);

    QCOMPARE(m_axis->categories(), cats);
    QCOMPARE(m_axis->min(), cats[0]);
    QCOMPARE(m_axis->max(), cats[cats.length() - 1]);

    // Append 3 more
    for (int i = 0; i < morecats.count(); ++i)
        m_axis->append(morecats[i]);

    QCOMPARE(m_axis->categories(), allcats);
    QCOMPARE(m_axis->min(), cats[0]);
    QCOMPARE(m_axis->max(), morecats[morecats.length() - 1]);

    // Remove the first 3
    for (int i = 0; i < cats.count(); ++i)
        m_axis->remove(cats[i]);

    QCOMPARE(m_axis->categories(), morecats);

    // Insert them in between
    m_axis->insert(3, cats[2]);
    m_axis->insert(1, cats[1]);
    m_axis->insert(0, cats[0]);

    QCOMPARE(m_axis->categories(), mixedcats);
    QCOMPARE(m_axis->min(), mixedcats[0]);
    QCOMPARE(m_axis->max(), mixedcats[mixedcats.length() - 1]);

    QCOMPARE(spy0.size(), 12);
    QCOMPARE(spy1.size(), 5);
    QCOMPARE(spy2.size(), 7);
    QCOMPARE(spy3.size(), 11);
    QCOMPARE(spy4.size(), 12);
}

void tst_barcategoryaxis::replaceClear()
{
    QVERIFY(m_axis);

    QSignalSpy spy0(m_axis, &QBarCategoryAxis::categoriesChanged);
    QSignalSpy spy1(m_axis, &QBarCategoryAxis::minChanged);
    QSignalSpy spy2(m_axis, &QBarCategoryAxis::maxChanged);
    QSignalSpy spy3(m_axis, &QBarCategoryAxis::categoryRangeChanged);
    QSignalSpy spy4(m_axis, &QBarCategoryAxis::countChanged);

    QStringList mixedcats = {"One", "Four", "Two", "Five", "Six", "Three"};
    QStringList replacedcats = {"Ten", "Four", "Fifteen", "Five", "Six", "Twenty"};

    m_axis->setCategories(mixedcats);

    // Replace 3
    m_axis->replace("One", "Ten");
    m_axis->replace("Two", "Fifteen");
    m_axis->replace("Three", "Twenty");

    QCOMPARE(m_axis->categories(), replacedcats);
    QCOMPARE(m_axis->min(), "Ten");
    QCOMPARE(m_axis->max(), "Twenty");

    // Clear
    m_axis->clear();
    QCOMPARE(m_axis->count(), 0);

    QCOMPARE(spy0.size(), 5);
    QCOMPARE(spy1.size(), 3);
    QCOMPARE(spy2.size(), 3);
    QCOMPARE(spy3.size(), 4);
    QCOMPARE(spy4.size(), 5);
}

QTEST_MAIN(tst_barcategoryaxis)
#include "tst_barcategoryaxis.moc"
