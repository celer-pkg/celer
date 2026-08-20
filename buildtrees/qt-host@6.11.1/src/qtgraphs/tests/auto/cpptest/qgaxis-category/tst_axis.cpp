// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QCategory3DAxis>

class tst_axis: public QObject
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
    QCategory3DAxis *m_axis;
};

void tst_axis::initTestCase()
{
}

void tst_axis::cleanupTestCase()
{
}

void tst_axis::init()
{
    m_axis = new QCategory3DAxis();
}

void tst_axis::cleanup()
{
    delete m_axis;
}

void tst_axis::construct()
{
    QCategory3DAxis *axis = new QCategory3DAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_axis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->labels().size(), 0);

    // Common (from QAbstract3DAxis)
    QCOMPARE(m_axis->isAutoAdjustRange(), true);
    QCOMPARE(m_axis->labelAutoAngle(), 0.0f);
    QCOMPARE(m_axis->max(), 10.0f);
    QCOMPARE(m_axis->min(), 0.0f);
    QCOMPARE(m_axis->orientation(), QAbstract3DAxis::AxisOrientation::None);
    QCOMPARE(m_axis->title(), QString(""));
    QCOMPARE(m_axis->isTitleFixed(), true);
    QCOMPARE(m_axis->isTitleVisible(), false);
    QCOMPARE(m_axis->isScaleLabelsByCount(), false);
    QCOMPARE(m_axis->labelSize(), 1.0f);
    QCOMPARE(m_axis->type(), QAbstract3DAxis::AxisType::Category);
}

void tst_axis::initializeProperties()
{
    QVERIFY(m_axis);

    QSignalSpy labelSpy(m_axis, &QCategory3DAxis::labelsChanged);
    QSignalSpy rowLabelSpy(m_axis, &QCategory3DAxis::rowLabelsChanged);
    QSignalSpy columnLabelSpy(m_axis, &QCategory3DAxis::columnLabelsChanged);

    QSignalSpy autoAdjustSpy(m_axis, &QCategory3DAxis::autoAdjustRangeChanged);
    QSignalSpy labelAutoAngleSpy(m_axis, &QCategory3DAxis::labelAutoAngleChanged);
    QSignalSpy maxSpy(m_axis, &QCategory3DAxis::maxChanged);
    QSignalSpy minSpy(m_axis, &QCategory3DAxis::minChanged);
    QSignalSpy titleSpy(m_axis, &QCategory3DAxis::titleChanged);
    QSignalSpy titleFixedSpy(m_axis, &QCategory3DAxis::titleFixedChanged);
    QSignalSpy titleVisibleSpy(m_axis, &QCategory3DAxis::titleVisibleChanged);

    m_axis->setLabels(QStringList() << "first" << "second");

    QCOMPARE(m_axis->labels().size(), 2);
    QCOMPARE(m_axis->labels().at(1), QString("second"));
    QCOMPARE(labelSpy.size(), 1);
    QCOMPARE(rowLabelSpy.size(), 0);
    QCOMPARE(columnLabelSpy.size(), 0);

    // Common (from QAbstract3DAxis)
    m_axis->setAutoAdjustRange(false);
    m_axis->setLabelAutoAngle(15.0f);
    m_axis->setMax(25.0f);
    m_axis->setMin(5.0f);
    m_axis->setTitle("title");
    m_axis->setTitleFixed(false);
    m_axis->setTitleVisible(true);
    m_axis->setScaleLabelsByCount(true);
    m_axis->setLabelSize(2.0f);

    QCOMPARE(m_axis->isAutoAdjustRange(), false);
    QCOMPARE(m_axis->labelAutoAngle(), 15.0f);
    QCOMPARE(m_axis->max(), 25.0f);
    QCOMPARE(m_axis->min(), 5.0f);
    QCOMPARE(m_axis->title(), QString("title"));
    QCOMPARE(m_axis->isTitleFixed(), false);
    QCOMPARE(m_axis->isTitleVisible(), true);
    QCOMPARE(m_axis->isScaleLabelsByCount(), true);
    QCOMPARE(m_axis->labelSize(), 2.0f);

    QCOMPARE(autoAdjustSpy.size(), 1);
    QCOMPARE(labelAutoAngleSpy.size(), 1);
    QCOMPARE(maxSpy.size(), 1);
    QCOMPARE(minSpy.size(), 1);
    QCOMPARE(titleSpy.size(), 1);
    QCOMPARE(titleFixedSpy.size(), 1);
    QCOMPARE(titleVisibleSpy.size(), 1);
}

void tst_axis::invalidProperties()
{
    m_axis->setLabelAutoAngle(-15.0f);
    QCOMPARE(m_axis->labelAutoAngle(), 0.0f);

    m_axis->setLabelAutoAngle(100.0f);
    QCOMPARE(m_axis->labelAutoAngle(), 90.0f);

    m_axis->setMax(-10.0f);
    QCOMPARE(m_axis->max(), 0.0f);
    QCOMPARE(m_axis->min(), 0.0f);

    m_axis->setMin(10.0f);
    QCOMPARE(m_axis->max(), 11.0f);
    QCOMPARE(m_axis->min(), 10.0f);
}

QTEST_MAIN(tst_axis)
#include "tst_axis.moc"
