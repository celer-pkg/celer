// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QValue3DAxis>
#include <QtGraphs/QLogValue3DAxisFormatter>


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
    QValue3DAxis *m_axis;
};

void tst_axis::initTestCase()
{
}

void tst_axis::cleanupTestCase()
{
}

void tst_axis::init()
{
    m_axis = new QValue3DAxis();
}

void tst_axis::cleanup()
{
    delete m_axis;
}

void tst_axis::construct()
{
    QValue3DAxis *axis = new QValue3DAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_axis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->labelFormat(), QString("%.2f"));
    QCOMPARE(m_axis->reversed(), false);
    QCOMPARE(m_axis->segmentCount(), 5);
    QCOMPARE(m_axis->subSegmentCount(), 1);

    // Common (from QAbstract3DAxis)
    QCOMPARE(m_axis->isAutoAdjustRange(), true);
    QCOMPARE(m_axis->labelAutoAngle(), 0.0f);
    QCOMPARE(m_axis->labels().size(), 6);
    QCOMPARE(m_axis->labels().at(0), QString("0.00"));
    QCOMPARE(m_axis->labels().at(1), QString("2.00"));
    QCOMPARE(m_axis->labels().at(2), QString("4.00"));
    QCOMPARE(m_axis->labels().at(3), QString("6.00"));
    QCOMPARE(m_axis->labels().at(4), QString("8.00"));
    QCOMPARE(m_axis->labels().at(5), QString("10.00"));
    QCOMPARE(m_axis->max(), 10.0f);
    QCOMPARE(m_axis->min(), 0.0f);
    QCOMPARE(m_axis->orientation(), QAbstract3DAxis::AxisOrientation::None);
    QCOMPARE(m_axis->title(), QString(""));
    QCOMPARE(m_axis->isTitleFixed(), true);
    QCOMPARE(m_axis->isTitleVisible(), false);
    QCOMPARE(m_axis->labelsVisible(), true);
    QCOMPARE(m_axis->titleOffset(), 0.0f);
    QCOMPARE(m_axis->type(), QAbstract3DAxis::AxisType::Value);
}

void tst_axis::initializeProperties()
{
    QVERIFY(m_axis);

    QSignalSpy labelFormatSpy(m_axis, &QValue3DAxis::labelFormatChanged);
    QSignalSpy reversedSpy(m_axis, &QValue3DAxis::reversedChanged);
    QSignalSpy segmentCountSpy(m_axis, &QValue3DAxis::segmentCountChanged);
    QSignalSpy subSegmentCountSpy(m_axis, &QValue3DAxis::subSegmentCountChanged);
    QSignalSpy formatterSpy(m_axis, &QValue3DAxis::formatterChanged);
    QSignalSpy dirtyFormatterSpy(m_axis, &QValue3DAxis::formatterDirty);

    QSignalSpy adjustRangeSpy(m_axis, &QValue3DAxis::autoAdjustRangeChanged);
    QSignalSpy labelAngleSpy(m_axis, &QValue3DAxis::labelAutoAngleChanged);
    QSignalSpy maxSpy(m_axis, &QValue3DAxis::maxChanged);
    QSignalSpy minSpy(m_axis, &QValue3DAxis::minChanged);
    QSignalSpy titleSpy(m_axis, &QValue3DAxis::titleChanged);
    QSignalSpy titleFixedSpy(m_axis, &QValue3DAxis::titleFixedChanged);
    QSignalSpy titleVisibleSpy(m_axis, &QValue3DAxis::titleVisibleChanged);
    QSignalSpy labelVisibleSpy(m_axis, &QValue3DAxis::labelVisibleChanged);
    QSignalSpy titleOffsetSpy(m_axis, &QValue3DAxis::titleOffsetChanged);

    auto *formatter = new QLogValue3DAxisFormatter(this);

    m_axis->setLabelFormat("%.0fm");
    m_axis->setReversed(true);
    m_axis->setSegmentCount(2);
    m_axis->setSubSegmentCount(5);

    QCOMPARE(m_axis->labelFormat(), QString("%.0fm"));
    QCOMPARE(m_axis->reversed(), true);
    QCOMPARE(m_axis->segmentCount(), 2);
    QCOMPARE(m_axis->subSegmentCount(), 5);

    QCOMPARE(labelFormatSpy.size(), 1);
    QCOMPARE(reversedSpy.size(), 1);
    QCOMPARE(segmentCountSpy.size(), 1);
    QCOMPARE(subSegmentCountSpy.size(), 1);

    // Common (from QAbstract3DAxis)
    m_axis->setAutoAdjustRange(false);
    m_axis->setLabelAutoAngle(15.0f);
    m_axis->setMax(25.0f);
    m_axis->setMin(5.0f);
    m_axis->setTitle("title");
    m_axis->setTitleFixed(false);
    m_axis->setTitleVisible(true);
    m_axis->setLabelsVisible(false);
    m_axis->setTitleOffset(1.0f);

    QCOMPARE(m_axis->isAutoAdjustRange(), false);
    QCOMPARE(m_axis->labelAutoAngle(), 15.0f);
    QCOMPARE(m_axis->labels().size(), 3);
    QCOMPARE(m_axis->labels().at(0), QString("5m"));
    QCOMPARE(m_axis->labels().at(1), QString("15m"));
    QCOMPARE(m_axis->labels().at(2), QString("25m"));
    QCOMPARE(m_axis->max(), 25.0f);
    QCOMPARE(m_axis->min(), 5.0f);
    QCOMPARE(m_axis->title(), QString("title"));
    QCOMPARE(m_axis->isTitleFixed(), false);
    QCOMPARE(m_axis->isTitleVisible(), true);
    QCOMPARE(m_axis->labelsVisible(), false);
    QCOMPARE(m_axis->titleOffset(), 1.0f);

    m_axis->setFormatter(formatter);
    QCOMPARE(dirtyFormatterSpy.size(), 1);
    QCOMPARE(formatterSpy.size(), 1);

    QCOMPARE(adjustRangeSpy.size(), 1);
    QCOMPARE(labelAngleSpy.size(), 1);
    QCOMPARE(maxSpy.size(), 1);
    QCOMPARE(minSpy.size(), 1);
    QCOMPARE(titleSpy.size(), 1);
    QCOMPARE(titleFixedSpy.size(), 1);
    QCOMPARE(titleVisibleSpy.size(), 1);
    QCOMPARE(labelVisibleSpy.size(), 1);
    QCOMPARE(titleOffsetSpy.size(), 1);
}

void tst_axis::invalidProperties()
{
    m_axis->setSegmentCount(-1);
    QCOMPARE(m_axis->segmentCount(), 1);

    m_axis->setSubSegmentCount(-1);
    QCOMPARE(m_axis->subSegmentCount(), 1);

    m_axis->setLabelAutoAngle(-15.0f);
    QCOMPARE(m_axis->labelAutoAngle(), 0.0f);

    m_axis->setLabelAutoAngle(100.0f);
    QCOMPARE(m_axis->labelAutoAngle(), 90.0f);

    m_axis->setMax(-10.0f);
    QCOMPARE(m_axis->max(), -10.0f);
    QCOMPARE(m_axis->min(), -11.0f);

    m_axis->setMin(10.0f);
    QCOMPARE(m_axis->max(), 11.0f);
    QCOMPARE(m_axis->min(), 10.0f);

    m_axis->setTitleOffset(2.0f);
    QCOMPARE(m_axis->titleOffset(), 0.0f);
}

QTEST_MAIN(tst_axis)
#include "tst_axis.moc"
