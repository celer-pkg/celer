// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QDateTimeAxis>
#include <QtTest/QtTest>
#include "qtestcase.h"

class tst_datetimeaxis : public QObject
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
    QDateTimeAxis *m_axis;
};

void tst_datetimeaxis::initTestCase() {}

void tst_datetimeaxis::cleanupTestCase() {}

void tst_datetimeaxis::init()
{
    m_axis = new QDateTimeAxis();
}

void tst_datetimeaxis::cleanup()
{
    delete m_axis;
}

void tst_datetimeaxis::construct()
{
    QDateTimeAxis *axis = new QDateTimeAxis();
    QVERIFY(axis);
    delete axis;
}

void tst_datetimeaxis::initialProperties()
{
    QVERIFY(m_axis);

    QCOMPARE(m_axis->min(),
             QDateTime(QDate(1970, 1, 1), QTime::fromMSecsSinceStartOfDay(0), QTimeZone::UTC));
    QCOMPARE(m_axis->max(),
             QDateTime(QDate(1970, 1, 1), QTime::fromMSecsSinceStartOfDay(0), QTimeZone::UTC)
                 .addYears(10));
    QCOMPARE(m_axis->labelFormat(), "dd-MMMM-yy");
    QCOMPARE(m_axis->subTickCount(), 0);
    QCOMPARE(m_axis->tickInterval(), 0.0);
    QCOMPARE(m_axis->timeZoneAsString(), "UTC");
}

void tst_datetimeaxis::initializeProperties()
{
    QVERIFY(m_axis);

#ifdef QT_FEATURE_timezone
    QString timeZoneString = "Europe/Helsinki";
#else
    QString tzone = "UTC";
#endif

    QSignalSpy minSpy(m_axis, &QDateTimeAxis::minChanged);
    QSignalSpy maxSpy(m_axis, &QDateTimeAxis::maxChanged);
    QSignalSpy labelFormatSpy(m_axis, &QDateTimeAxis::labelFormatChanged);
    QSignalSpy tickIntervalSpy(m_axis, &QDateTimeAxis::tickIntervalChanged);
    QSignalSpy subTickCountSpy(m_axis, &QDateTimeAxis::subTickCountChanged);
    QSignalSpy timeZoneSpy(m_axis, &QDateTimeAxis::timeZoneChanged);

    m_axis->setMin(QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0)));
    m_axis->setMax(
        QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0), QTimeZone::UTC)
            .addYears(20));
    m_axis->setLabelFormat("yyyy");
    m_axis->setSubTickCount(2);
    m_axis->setTickInterval(0.5);

    QCOMPARE(m_axis->min(), QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0)));
    QCOMPARE(m_axis->max(),
             QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0), QTimeZone::UTC)
                 .addYears(20));
    QCOMPARE(m_axis->min().timeZone(), QTimeZone::utc());
    QCOMPARE(m_axis->max().timeZone(), QTimeZone::utc());
    QCOMPARE(m_axis->labelFormat(), "yyyy");
    QCOMPARE(m_axis->subTickCount(), 2);
    QCOMPARE(m_axis->tickInterval(), 0.5);

    auto tz = QString(timeZoneString).toUtf8();
    m_axis->setTimeZone(QTimeZone(tz));

    auto max = QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0), QTimeZone::UTC)
                 .addYears(20);
    auto min = QDateTime(QDate::currentDate(), QTime::fromMSecsSinceStartOfDay(0));
    max.toTimeZone(QTimeZone(tz));
    min.toTimeZone(QTimeZone(tz));

    QCOMPARE(m_axis->timeZoneAsString(), timeZoneString);
    QCOMPARE(m_axis->min(), min);
    QCOMPARE(m_axis->max(), max);
    QCOMPARE(m_axis->min().timeZone().offsetFromUtc(m_axis->min()), QTimeZone(tz).offsetFromUtc(m_axis->min()));
    QCOMPARE(m_axis->max().timeZone().offsetFromUtc(m_axis->max()), QTimeZone(tz).offsetFromUtc(m_axis->max()));

    QCOMPARE(minSpy.size(), 1);
    QCOMPARE(maxSpy.size(), 1);
    QCOMPARE(labelFormatSpy.size(), 1);
    QCOMPARE(tickIntervalSpy.size(), 1);
    QCOMPARE(subTickCountSpy.size(), 1);
    QCOMPARE(timeZoneSpy.size(), 1);
}

void tst_datetimeaxis::invalidProperties()
{
    QVERIFY(m_axis);

    m_axis->setSubTickCount(-1);
    m_axis->setTickInterval(-1);

    QCOMPARE(m_axis->subTickCount(), 0);
    QCOMPARE(m_axis->tickInterval(), 0);
}

QTEST_MAIN(tst_datetimeaxis)
#include "tst_datetimeaxis.moc"
