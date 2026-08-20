// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtGraphs/QPieSlice>
#include <QtGraphs/QPieSeries>

QT_USE_NAMESPACE

class tst_qgpieslice : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void construct();
    void customize();

private:
    QPieSlice *m_slice;
};

void tst_qgpieslice::initTestCase() {}

void tst_qgpieslice::cleanupTestCase() {}

void tst_qgpieslice::init()
{
    m_slice = new QPieSlice();
}

void tst_qgpieslice::cleanup()
{
    delete m_slice;
}

void tst_qgpieslice::construct()
{
    // no params
    QPieSlice slice1;
    QCOMPARE(slice1.value(), 0.0);
    QVERIFY(slice1.label().isEmpty());
    QVERIFY(!slice1.isLabelVisible());
    QVERIFY(!slice1.isExploded());
    QCOMPARE(slice1.labelFont(), QFont());
    QCOMPARE(slice1.labelArmLengthFactor(), 0.15); // default value
    QCOMPARE(slice1.explodeDistanceFactor(), 0.15); // default value
    QCOMPARE(slice1.percentage(), 0.0);
    QCOMPARE(slice1.startAngle(), 0.0);
    QCOMPARE(slice1.angleSpan(), 0.0);

    // value and label params
    QPieSlice slice2("foobar", 1.0);
    QCOMPARE(slice2.value(), 1.0);
    QCOMPARE(slice2.label(), QString("foobar"));
    QVERIFY(!slice2.isLabelVisible());
    QVERIFY(!slice2.isExploded());
    QCOMPARE(slice2.labelFont(), QFont());
    QCOMPARE(slice2.labelArmLengthFactor(), 0.15); // default value
    QCOMPARE(slice2.explodeDistanceFactor(), 0.15); // default value
    QCOMPARE(slice2.percentage(), 0.0);
    QCOMPARE(slice2.startAngle(), 0.0);
    QCOMPARE(slice2.angleSpan(), 0.0);
}

void tst_qgpieslice::customize()
{
    // create a pie series
    QPieSeries *series = new QPieSeries();
    QPieSlice *s1 = series->append("slice 1", 1);
    QPieSlice *s2 = series->append("slice 2", 2);
    series->append("slice 3", 3);

    QSignalSpy spy0(s1, &QPieSlice::colorChanged);
    QSignalSpy spy1(s1, &QPieSlice::borderColorChanged);
    QSignalSpy spy2(s1, &QPieSlice::labelColorChanged);
    QSignalSpy spy3(s1, &QPieSlice::labelFontChanged);

    QColor color = Qt::red;
    // customize a slice
    s1->setColor(color);
    s1->setBorderColor(color);
    s1->setLabelColor(color);
    QFont f1("Consolas");
    s1->setLabelFont(f1);

    // check that customizations persist
    QCOMPARE(s1->color(), color);
    QCOMPARE(s1->borderColor(), color);
    QCOMPARE(s1->labelColor(), color);
    QCOMPARE(s1->labelFont(), f1);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);

    // remove a slice
    series->remove(s2);
    QCOMPARE(s1->color(), color);
    QCOMPARE(s1->borderColor(), color);
    QCOMPARE(s1->labelColor(), color);
    QCOMPARE(s1->labelFont(), f1);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);

    // add a slice
    series->append("slice 4", 4);
    QCOMPARE(s1->color(), color);
    QCOMPARE(s1->borderColor(), color);
    QCOMPARE(s1->labelColor(), color);
    QCOMPARE(s1->labelFont(), f1);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);

    // insert a slice
    series->insert(0, new QPieSlice("slice 0", 5));
    QCOMPARE(s1->color(), color);
    QCOMPARE(s1->borderColor(), color);
    QCOMPARE(s1->labelColor(), color);
    QCOMPARE(s1->labelFont(), f1);

    QCOMPARE(spy0.size(), 1);
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(spy3.size(), 1);
}

QTEST_MAIN(tst_qgpieslice)
#include "tst_qgpieslice.moc"
