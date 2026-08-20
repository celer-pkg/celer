// Copyright (C) 2016 Jolla Ltd.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

#include <QtCore/QDateTime>
#include <QtCore/QElapsedTimer>
#include <QtCore/QtNumeric>

#ifdef Q_OS_WIN

// Windows seems to require longer timeouts and step length
// We override the standard QTestCase related macros

#ifdef QTRY_COMPARE_WITH_TIMEOUT
#undef  QTRY_COMPARE_WITH_TIMEOUT
#endif
#define QTRY_COMPARE_WITH_TIMEOUT(__expr, __expected, __timeout) \
do { \
    const int __step = 100; \
    const int __timeoutValue = __timeout; \
    if ((__expr) != (__expected)) { \
        QTest::qWait(0); \
    } \
    for (int __i = 0; __i < __timeoutValue && ((__expr) != (__expected)); __i+=__step) { \
        QTest::qWait(__step); \
    } \
    QCOMPARE(__expr, __expected); \
} while (0)

#ifdef QTRY_COMPARE
#undef  QTRY_COMPARE
#endif
#define QTRY_COMPARE(__expr, __expected) QTRY_COMPARE_WITH_TIMEOUT(__expr, __expected, 10000)

#endif

tst_QNmeaPositionInfoSource::tst_QNmeaPositionInfoSource(QNmeaPositionInfoSource::UpdateMode mode, QObject *parent)
    : QObject(parent),
      m_mode(mode)
{
}

void tst_QNmeaPositionInfoSource::initTestCase()
{
    qRegisterMetaType<QNmeaPositionInfoSource::UpdateMode>();
}

void tst_QNmeaPositionInfoSource::constructor()
{
    QObject o;
    QNmeaPositionInfoSource source(m_mode, &o);
    QCOMPARE(source.updateMode(), m_mode);
    QCOMPARE(source.parent(), &o);
}

void tst_QNmeaPositionInfoSource::supportedPositioningMethods()
{
    QNmeaPositionInfoSource source(m_mode);
    QCOMPARE(source.supportedPositioningMethods(), QNmeaPositionInfoSource::SatellitePositioningMethods);
}

void tst_QNmeaPositionInfoSource::minimumUpdateInterval()
{
    QNmeaPositionInfoSource source(m_mode);
    QCOMPARE(source.minimumUpdateInterval(), 2);
}

void tst_QNmeaPositionInfoSource::userEquivalentRangeError()
{
    QNmeaPositionInfoSource source(m_mode);
    QVERIFY(qIsNaN(source.userEquivalentRangeError()));
    source.setUserEquivalentRangeError(5.1);
    QCOMPARE(source.userEquivalentRangeError(), 5.1);
}

void tst_QNmeaPositionInfoSource::setUpdateInterval_delayedUpdate()
{
    // If an update interval is set, and an update is not available at a
    // particular interval, the source should emit the next update as soon
    // as it becomes available

    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    // Android emulator is slow, so increase the update interval and all
    // timeouts for it.
#ifndef Q_OS_ANDROID
    constexpr int updateInterval = 500;
    constexpr int waitInterval = 600;
    constexpr int maxDelay = 400;
#else
    constexpr int updateInterval = 1000;
    constexpr int waitInterval = 1100;
    constexpr int maxDelay = 900;
#endif

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    proxy->source()->setUpdateInterval(updateInterval);
    proxy->source()->startUpdates();

    QTest::qWait(waitInterval);
    QDateTime now = QDateTime::currentDateTime();
    proxy->feedUpdate(now);
    QTRY_COMPARE(spyUpdate.size(), 1);

    // should have gotten the update immediately, and not have needed to
    // wait until the next interval
    QVERIFY(now.time().msecsTo(QDateTime::currentDateTime().time()) < maxDelay);
}

void tst_QNmeaPositionInfoSource::lastKnownPosition()
{
    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QCOMPARE(proxy->source()->lastKnownPosition(), QGeoPositionInfo());

    // source may need requestUpdate() or startUpdates() to be called to
    // trigger reading of data channel
    QSignalSpy spyTimeout(proxy->source(), SIGNAL(errorOccurred(QGeoPositionInfoSource::Error)));
    proxy->source()->requestUpdate(proxy->source()->minimumUpdateInterval());
    QTRY_COMPARE(spyTimeout.size(), 1);
    const QList<QVariant> arguments = spyTimeout.takeFirst();
    const auto error = arguments.at(0).value<QGeoPositionInfoSource::Error>();
    QCOMPARE(error, QGeoPositionInfoSource::UpdateTimeoutError);

    // If an update is received and startUpdates() or requestUpdate() hasn't
    // been called, it should still be available through lastKnownPosition()
    QDateTime dt = QDateTime::currentDateTimeUtc();
    proxy->feedUpdate(dt);
    QTRY_COMPARE(proxy->source()->lastKnownPosition().timestamp(), dt);

    QList<QDateTime> dateTimes = createDateTimes(5);
    for (int i=0; i<dateTimes.size(); i++) {
        proxy->source()->requestUpdate(); // Irrelevant for this test
        proxy->feedUpdate(dateTimes[i]);
        QTRY_COMPARE(proxy->source()->lastKnownPosition().timestamp(), dateTimes[i]);
    }

    proxy->source()->startUpdates();
    // if dateTimes are older than before, they will be ignored.
    dateTimes = createDateTimes(dateTimes.last().addMSecs(100), 5);
    for (int i=0; i<dateTimes.size(); i++) {
        proxy->feedUpdate(dateTimes[i]);
        QTRY_COMPARE(proxy->source()->lastKnownPosition().timestamp(), dateTimes[i]);
    }
}

void tst_QNmeaPositionInfoSource::beginWithBufferedData()
{
    // In SimulationMode, data stored in the QIODevice is read when
    // startUpdates() or requestUpdate() is called.
    // In RealTimeMode, all existing data in the QIODevice is ignored -
    // only new data will be read.

    QFETCH(QList<QDateTime>, dateTimes);
    QFETCH(UpdateTriggerMethod, trigger);

    QByteArray bytes;
    for (int i=0; i<dateTimes.size(); i++)
        bytes += QLocationTestUtils::createRmcSentence(dateTimes[i]).toLatin1();
    QBuffer buffer;
    buffer.setData(bytes);

    QNmeaPositionInfoSource source(m_mode);
    QSignalSpy spy(&source, SIGNAL(positionUpdated(QGeoPositionInfo)));
    source.setDevice(&buffer);

    if (trigger == StartUpdatesMethod)
        source.startUpdates();
    else if (trigger == RequestUpdatesMethod)
        source.requestUpdate();

    if (m_mode == QNmeaPositionInfoSource::RealTimeMode) {
        QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 0, 300);
    } else {
        if (trigger == StartUpdatesMethod) {
            QTRY_COMPARE(spy.size(), dateTimes.size());
            for (int i=0; i<dateTimes.size(); i++)
                QCOMPARE(spy.at(i).at(0).value<QGeoPositionInfo>().timestamp(), dateTimes[i]);
        } else if (trigger == RequestUpdatesMethod) {
            QTRY_COMPARE(spy.size(), 1);
            QCOMPARE(spy.at(0).at(0).value<QGeoPositionInfo>().timestamp(), dateTimes.first());
        }
    }
}

void tst_QNmeaPositionInfoSource::beginWithBufferedData_data()
{
    QTest::addColumn<QList<QDateTime> >("dateTimes");
    QTest::addColumn<UpdateTriggerMethod>("trigger");

    QList<QDateTime> dateTimes;
    dateTimes << QDateTime::currentDateTime().toUTC();

    QTest::newRow("startUpdates(), 1 update in buffer") << dateTimes << StartUpdatesMethod;
    QTest::newRow("requestUpdate(), 1 update in buffer") << dateTimes << RequestUpdatesMethod;

    for (int i=1; i<3; i++)
        dateTimes << dateTimes[0].addMSecs(i * 100);
    QTest::newRow("startUpdates(), multiple updates in buffer") << dateTimes << StartUpdatesMethod;
    QTest::newRow("requestUpdate(), multiple updates in buffer") << dateTimes << RequestUpdatesMethod;
}

void tst_QNmeaPositionInfoSource::startUpdates()
{
    QFETCH(QList<QDateTime>, dateTimes);

    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    proxy->source()->startUpdates();

    for (int i=0; i<dateTimes.size(); i++)
        proxy->feedUpdate(dateTimes[i]);
    QTRY_COMPARE(spyUpdate.size(), dateTimes.size());
}

void tst_QNmeaPositionInfoSource::startUpdates_data()
{
    QTest::addColumn<QList<QDateTime> >("dateTimes");

    QTest::newRow("1 update") << createDateTimes(1);
    QTest::newRow("2 updates") << createDateTimes(2);
    QTest::newRow("10 updates") << createDateTimes(10);
}

void tst_QNmeaPositionInfoSource::startUpdates_withTimeout()
{
    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    QSignalSpy spyTimeout(proxy->source(), SIGNAL(errorOccurred(QGeoPositionInfoSource::Error)));

    proxy->source()->setUpdateInterval(1000);
    proxy->source()->startUpdates();

    QDateTime dt = QDateTime::currentDateTimeUtc();

    if (m_mode == QNmeaPositionInfoSource::SimulationMode) {
        // the first sentence primes the simulation
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt).toLatin1());
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addMSecs(10)).toLatin1());
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addMSecs(1100)).toLatin1());
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addMSecs(2200)).toLatin1());
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addSecs(9)).toLatin1());

        QElapsedTimer t;
        t.start();

        for (int j = 1; j < 4; ++j) {
            QTRY_COMPARE(spyUpdate.size(), j);
            QCOMPARE(spyTimeout.size(), 0);
            int time = t.elapsed();
            QVERIFY((time > j*1000 - 300) && (time < j*1000 + 300));
        }

        spyUpdate.clear();

        QTRY_VERIFY_WITH_TIMEOUT((spyUpdate.size() == 0) && (spyTimeout.size() == 1), 7500);
        const QList<QVariant> arguments = spyTimeout.takeFirst();
        const auto error = arguments.at(0).value<QGeoPositionInfoSource::Error>();
        QCOMPARE(error, QGeoPositionInfoSource::UpdateTimeoutError);
        spyTimeout.clear();

        QTRY_VERIFY_WITH_TIMEOUT((spyUpdate.size() == 1) && (spyTimeout.size() == 0), 7500);

    } else {
        // dt + 900
        QTRY_VERIFY(spyUpdate.size() == 0 && spyTimeout.size() == 0);

        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addSecs(1)).toLatin1());
        // dt + 1200
        QTRY_VERIFY(spyUpdate.size() == 1 && spyTimeout.size() == 0);
        spyUpdate.clear();

        // dt + 1900
        QTRY_VERIFY(spyUpdate.size() == 0 && spyTimeout.size() == 0);
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addSecs(2)).toLatin1());

        // dt + 2200
        QTRY_VERIFY(spyUpdate.size() == 1 && spyTimeout.size() == 0);
        spyUpdate.clear();

        // dt + 2900
        QTRY_VERIFY(spyUpdate.size() == 0 && spyTimeout.size() == 0);
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addSecs(3)).toLatin1());

        // dt + 3200
        QTRY_VERIFY(spyUpdate.size() == 1 && spyTimeout.size() == 0);
        spyUpdate.clear();

        // dt + 6900
        QTRY_VERIFY(spyUpdate.size() == 0 && spyTimeout.size() == 1);
        const QList<QVariant> arguments = spyTimeout.takeFirst();
        const auto error = arguments.at(0).value<QGeoPositionInfoSource::Error>();
        QCOMPARE(error, QGeoPositionInfoSource::UpdateTimeoutError);
        spyTimeout.clear();
        proxy->feedBytes(QLocationTestUtils::createRmcSentence(dt.addSecs(7)).toLatin1());

        // dt + 7200
        QTRY_VERIFY(spyUpdate.size() == 1 && spyTimeout.size() == 0);
        spyUpdate.clear();
    }
}

void tst_QNmeaPositionInfoSource::startUpdates_expectLatestUpdateOnly()
{
    // If startUpdates() is called and an interval has been set, if multiple'
    // updates are in the buffer, only the latest update should be emitted

    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    proxy->source()->setUpdateInterval(500);
    proxy->source()->startUpdates();

    QList<QDateTime> dateTimes = createDateTimes(3);
    for (int i=0; i<dateTimes.size(); i++)
        proxy->feedUpdate(dateTimes[i]);

    QTRY_COMPARE(spyUpdate.size(), 1);
    QCOMPARE(spyUpdate[0][0].value<QGeoPositionInfo>().timestamp(), dateTimes.last());
}

void tst_QNmeaPositionInfoSource::startUpdates_waitForValidDateTime()
{
    // Tests that the class does not emit an update until it receives a
    // sentences with a valid date *and* time. All sentences before this
    // should be ignored, and any sentences received after this that do
    // not have a date should use the known date.

    QFETCH(QByteArray, bytes);
    QFETCH(QList<QDateTime>, dateTimes);
    QFETCH(QList<bool>, expectHorizontalAccuracy);
    QFETCH(QList<bool>, expectVerticalAccuracy);

    QNmeaPositionInfoSource source(m_mode);
    source.setUserEquivalentRangeError(5.1);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spy(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    QObject::connect(proxy->source(), &QNmeaPositionInfoSource::positionUpdated, this,
                     [](const QGeoPositionInfo &info) { qDebug() << info.timestamp(); });

    proxy->source()->startUpdates();
    proxy->feedBytes(bytes);
    QTest::qWait(1000); // default push delay is 20ms
    QTRY_COMPARE(spy.size(), dateTimes.size());

    for (int i=0; i<spy.size(); i++) {
        QGeoPositionInfo pInfo = spy[i][0].value<QGeoPositionInfo>();

        QCOMPARE(pInfo.timestamp(), dateTimes[i]);

        // Generated GGA/GSA sentences have hard coded HDOP of 3.5, which corrisponds to a
        // horizontal accuracy of 35.7, for the user equivalent range error of 5.1 set above.
        QCOMPARE(pInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy),
                 expectHorizontalAccuracy[i]);
        if (pInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
            QCOMPARE(pInfo.attribute(QGeoPositionInfo::HorizontalAccuracy), 35.7);

        // Generated GSA sentences have hard coded VDOP of 4.0, which corrisponds to a vertical
        // accuracy of 40.8, for the user equivalent range error of 5.1 set above.
        QCOMPARE(pInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy),
                 expectVerticalAccuracy[i]);
        if (pInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
            QCOMPARE(pInfo.attribute(QGeoPositionInfo::VerticalAccuracy), 40.8);
    }
}

void tst_QNmeaPositionInfoSource::startUpdates_waitForValidDateTime_data()
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::addColumn<QList<QDateTime> >("dateTimes");
    QTest::addColumn<QList<bool> >("expectHorizontalAccuracy");
    QTest::addColumn<QList<bool> >("expectVerticalAccuracy");

    QDateTime dt = QDateTime::currentDateTime().toUTC();
    QByteArray bytes;

    // should only receive RMC sentence and the GGA sentence *after* it
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(100).time()).toLatin1();
    bytes += QLocationTestUtils::createRmcSentence(dt.addMSecs(200)).toLatin1();
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(300).time()).toLatin1();
    // The first GGA does not have date, and there's no cached date to inject, so that update will be invalid
    QTest::newRow("Feed GGA,RMC,GGA; expect RMC, second GGA only")
            << bytes << (QList<QDateTime>() << dt.addMSecs(200) << dt.addMSecs(300))
            << (QList<bool>() << true << true) // accuracies are currently cached and injected in QGeoPositionInfos that do not have it
            << (QList<bool>() << false << false);

    // should not receive ZDA (has no coordinates) but should get the GGA
    // sentence after it since it got the date/time from ZDA
    bytes.clear();
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(100).time()).toLatin1();
    bytes += QLocationTestUtils::createZdaSentence(dt.addMSecs(200)).toLatin1();
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(300).time()).toLatin1();
    QTest::newRow("Feed GGA,ZDA,GGA; expect second GGA only")
            << bytes << (QList<QDateTime>() << dt.addMSecs(300))
            << (QList<bool>() << true)
            << (QList<bool>() << false);

    // Feed ZDA,GGA,GSA,GGA; expect vertical accuracy from second GGA.
    bytes.clear();
    bytes += QLocationTestUtils::createZdaSentence(dt.addMSecs(100)).toLatin1();
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(200).time()).toLatin1();
    bytes += QLocationTestUtils::createGsaSentence().toLatin1();
    bytes += QLocationTestUtils::createGgaSentence(dt.addMSecs(300).time()).toLatin1();
    if (m_mode == QNmeaPositionInfoSource::SimulationMode) {
        QTest::newRow("Feed ZDA,GGA,GSA,GGA; expect vertical accuracy from second GGA")
                << bytes << (QList<QDateTime>() << dt.addMSecs(200) << dt.addMSecs(300))
                << (QList<bool>() << true << true)
                << (QList<bool>() << true << true); // First GGA gets VDOP from GSA bundled into previous, as it has no timestamp, second GGA gets the cached value.
    }

    if (m_mode == QNmeaPositionInfoSource::SimulationMode) {
        // In sim m_mode, should ignore sentence with a date/time before the known date/time
        // (in real time m_mode, everything is passed on regardless)
        bytes.clear();
        bytes += QLocationTestUtils::createRmcSentence(dt.addMSecs(100)).toLatin1();
        bytes += QLocationTestUtils::createRmcSentence(dt.addMSecs(-200)).toLatin1();
        bytes += QLocationTestUtils::createRmcSentence(dt.addMSecs(200)).toLatin1();
        QTest::newRow("Feed good RMC, RMC with bad date/time, good RMC; expect first and third RMC only")
                << bytes << (QList<QDateTime>() << dt.addMSecs(100) << dt.addMSecs(200))
                << (QList<bool>() << false << false)
                << (QList<bool>() << false << false);
    }
}

void tst_QNmeaPositionInfoSource::requestUpdate_waitForValidDateTime()
{
    QFETCH(QByteArray, bytes);
    QFETCH(QList<QDateTime>, dateTimes);

    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spy(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    proxy->source()->requestUpdate();

    proxy->feedBytes(bytes);
    QTRY_COMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].value<QGeoPositionInfo>().timestamp(), dateTimes[0]);
}

void tst_QNmeaPositionInfoSource::requestUpdate_waitForValidDateTime_data()
{
    startUpdates_waitForValidDateTime_data();
}

void tst_QNmeaPositionInfoSource::requestUpdate()
{
    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    QSignalSpy spyTimeout(proxy->source(), SIGNAL(errorOccurred(QGeoPositionInfoSource::Error)));
    QDateTime dt;

    proxy->source()->requestUpdate(100);
    QTRY_COMPARE(spyTimeout.size(), 1);
    auto error = spyTimeout[0][0].value<QGeoPositionInfoSource::Error>();
    QCOMPARE(error, QGeoPositionInfoSource::UpdateTimeoutError);
    spyTimeout.clear();

    dt = QDateTime::currentDateTimeUtc();
    proxy->feedUpdate(dt);
    proxy->source()->requestUpdate();
    QTRY_COMPARE(spyUpdate.size(), 1);
    QCOMPARE(spyUpdate[0][0].value<QGeoPositionInfo>().timestamp(), dt);
    QCOMPARE(spyTimeout.size(), 0);
    spyUpdate.clear();

    // delay the update and expect it to be emitted after 300ms
    dt = QDateTime::currentDateTimeUtc();
    proxy->source()->requestUpdate(1000);
    QTest::qWait(300);
    proxy->feedUpdate(dt);
    QTRY_COMPARE(spyUpdate.size(), 1);
    QCOMPARE(spyUpdate[0][0].value<QGeoPositionInfo>().timestamp(), dt);
    QCOMPARE(spyTimeout.size(), 0);
    spyUpdate.clear();

    // delay the update and expect errorOccurred() to be emitted
    dt = QDateTime::currentDateTimeUtc();
    proxy->source()->requestUpdate(500);
    QTest::qWait(1000);
    proxy->feedUpdate(dt);
    QCOMPARE(spyTimeout.size(), 1);
    error = spyTimeout[0][0].value<QGeoPositionInfoSource::Error>();
    QCOMPARE(error, QGeoPositionInfoSource::UpdateTimeoutError);
    QCOMPARE(spyUpdate.size(), 0);
    spyUpdate.clear();
}

void tst_QNmeaPositionInfoSource::requestUpdate_after_start()
{
    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spyUpdate(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    QSignalSpy spyTimeout(proxy->source(), SIGNAL(errorOccurred(QGeoPositionInfoSource::Error)));

    // Start updates with 500ms interval and requestUpdate() with 100ms
    // timeout. Feed an update, and it should be emitted immediately due to
    // the requestUpdate(). The update should not be emitted again after that
    // (i.e. the startUpdates() interval should not cause it to be re-emitted).

    QDateTime dt = QDateTime::currentDateTimeUtc();
    proxy->source()->setUpdateInterval(500);
    proxy->source()->startUpdates();
    proxy->source()->requestUpdate(100);
    proxy->feedUpdate(dt);
    QTRY_COMPARE(spyUpdate.size(), 1);
    QCOMPARE(spyUpdate[0][0].value<QGeoPositionInfo>().timestamp(), dt);
    QCOMPARE(spyTimeout.size(), 0);
    spyUpdate.clear();

    // Update has been emitted for requestUpdate(), shouldn't be emitted for startUpdates()
    QTRY_COMPARE_WITH_TIMEOUT(spyUpdate.size(), 0, 1000);
}

void tst_QNmeaPositionInfoSource::testWithBadNmea()
{
    QFETCH(QByteArray, bytes);
    QFETCH(QList<QDateTime>, dateTimes);
    QFETCH(UpdateTriggerMethod, trigger);

    QNmeaPositionInfoSource source(m_mode);
    QNmeaProxyFactory factory;
    QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy *>(
            factory.createPositionInfoSourceProxy(&source));

    QSignalSpy spy(proxy->source(), SIGNAL(positionUpdated(QGeoPositionInfo)));
    if (trigger == StartUpdatesMethod)
        proxy->source()->startUpdates();
    else
        proxy->source()->requestUpdate();

    proxy->feedBytes(bytes);
    QTRY_COMPARE(spy.size(), dateTimes.size());
    for (int i=0; i<dateTimes.size(); i++)
        QCOMPARE(spy[i][0].value<QGeoPositionInfo>().timestamp(), dateTimes[i]);
}

void tst_QNmeaPositionInfoSource::testWithBadNmea_data()
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::addColumn<QList<QDateTime> >("dateTimes");
    QTest::addColumn<UpdateTriggerMethod>("trigger");

    QDateTime firstDateTime = QDateTime::currentDateTimeUtc();
    QByteArray bad = QLocationTestUtils::createRmcSentence(firstDateTime.addSecs(1)).toLatin1();
    bad = bad.mid(bad.size()/2);
    QDateTime lastDateTime = firstDateTime.addSecs(2);

    QByteArray bytes;
    bytes += QLocationTestUtils::createRmcSentence(firstDateTime).toLatin1();
    bytes += bad;
    bytes += QLocationTestUtils::createRmcSentence(lastDateTime).toLatin1();
    QTest::newRow("requestUpdate(), bad second sentence") << bytes
            << (QList<QDateTime>() << firstDateTime) << RequestUpdatesMethod;
    QTest::newRow("startUpdates(), bad second sentence") << bytes
            << (QList<QDateTime>() << firstDateTime << lastDateTime) << StartUpdatesMethod;
}
