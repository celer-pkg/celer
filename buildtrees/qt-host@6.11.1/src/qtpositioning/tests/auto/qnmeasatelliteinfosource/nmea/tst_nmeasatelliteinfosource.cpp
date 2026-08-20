// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QTimer>
#include <QSignalSpy>
#include <QtPositioning/QNmeaSatelliteInfoSource>
#include "../../utils/qlocationtestutils_p.h"

class DataFeeder : public QIODevice
{
    Q_OBJECT
public:
    DataFeeder(QNmeaSatelliteInfoSource *parent)
        : QIODevice(parent), m_parentMode(parent->updateMode())
    {
    }

    void setMessages(const QList<QByteArray> &messages)
    {
        m_messages = messages;
        m_canReadLine = true;
        m_messageIndex = 0;
        if (m_parentMode == QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode)
            emit readyRead();
    }

signals:
    void messageSent();

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (maxSize == 0 || m_messageIndex < 0 || m_messageIndex >= m_messages.size())
            return 0;

        m_canReadLine = false;

        QByteArray bytes = m_messages.at(m_messageIndex);
        qint64 sz = qMin(qint64(bytes.size()), maxSize);
        memcpy(data, bytes.constData(), sz);

        m_messageIndex++;

        emit messageSent();
        QTimer::singleShot(10, this, &DataFeeder::onTimer);

        return sz;
    }

    qint64 writeData(const char *, qint64) override { return -1; }

    qint64 bytesAvailable() const override { return 1024 + QIODevice::bytesAvailable(); }

    bool canReadLine() const override
    {
        return m_canReadLine && (m_messageIndex >= 0) && (m_messageIndex < m_messages.size());
    }

private slots:
    void onTimer()
    {
        m_canReadLine = true;
        if (m_parentMode == QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode)
            emit readyRead();
    }

private:
    QNmeaSatelliteInfoSource::UpdateMode m_parentMode;
    QList<QByteArray> m_messages;
    qsizetype m_messageIndex = 0;
    bool m_canReadLine = true; // To read each line separately
};

class tst_QNmeaSatelliteInfoSource : public QObject
{
    Q_OBJECT
private slots:
    void backendProperty();
    void backendProperty_data();

    void parseDataStream();
    void parseDataStream_data();

private:
    QGeoSatelliteInfo createSatelliteInfo(QGeoSatelliteInfo::SatelliteSystem system, int id,
                                          int snr);
};

void tst_QNmeaSatelliteInfoSource::backendProperty()
{
    QFETCH(int, simulationRate);
    QFETCH(int, updateInterval);
    QFETCH(QList<QByteArray>, messages);
    QFETCH(int, timeout);
    QFETCH(int, desiredMessagesSent);

    QNmeaSatelliteInfoSource source(QNmeaSatelliteInfoSource::UpdateMode::SimulationMode);
    auto feeder = new DataFeeder(&source);
    source.setDevice(feeder);

    QSignalSpy messageSentSpy(feeder, &DataFeeder::messageSent);
    QSignalSpy inViewSpy(&source, &QNmeaSatelliteInfoSource::satellitesInViewUpdated);

    source.setBackendProperty(QNmeaSatelliteInfoSource::SimulationUpdateInterval, simulationRate);
    source.setUpdateInterval(updateInterval);

    source.startUpdates();
    feeder->setMessages(messages);

    QTRY_VERIFY_WITH_TIMEOUT(inViewSpy.size() == 1, timeout);

    QCOMPARE(source.backendProperty(QNmeaSatelliteInfoSource::SimulationUpdateInterval).toInt(),
             simulationRate);
    QCOMPARE(source.updateInterval(), updateInterval);

    QCOMPARE(messageSentSpy.size(), desiredMessagesSent);
}

void tst_QNmeaSatelliteInfoSource::backendProperty_data()
{
    QTest::addColumn<int>("simulationRate");
    QTest::addColumn<int>("updateInterval");
    QTest::addColumn<QList<QByteArray>>("messages");
    QTest::addColumn<int>("timeout");
    QTest::addColumn<int>("desiredMessagesSent");

    const auto simpleGpsGsvMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                             "$GPGSV,1,1,4,05,,,25,07,,,,08,,,,13,,,36*")
                                             .toLatin1();

    const QList<QByteArray> msgs(10, simpleGpsGsvMessage);

    // here updateInterval should not be a multiple of simulationRate because
    // this will make the test flacky due to timer precision.
    QTest::addRow("Simulation rate smaller than updateInterval") << 200 << 500 << msgs << 550 << 2;

    QTest::addRow("Simulation rate larger than updateInterval") << 200 << 100 << msgs << 300 << 1;

    // Here we do not really have problems with timer precision, because even if
    // the updateInterval() timer expires earlier, we will not get any update
    // until the real data comes.
    QTest::addRow("Simulation rate equals updateInterval") << 200 << 200 << msgs << 300 << 1;
}

void tst_QNmeaSatelliteInfoSource::parseDataStream()
{
    QFETCH(QNmeaSatelliteInfoSource::UpdateMode, mode);
    QFETCH(QList<QByteArray>, messages);
    QFETCH(QList<QGeoSatelliteInfo>, desiredInView);
    QFETCH(QList<QGeoSatelliteInfo>, desiredInUse);

    QNmeaSatelliteInfoSource source(mode);
    auto feeder = new DataFeeder(&source);
    source.setDevice(feeder);

    QSignalSpy messageSentSpy(feeder, &DataFeeder::messageSent);
    QSignalSpy inViewSpy(&source, &QNmeaSatelliteInfoSource::satellitesInViewUpdated);
    QSignalSpy inUseSpy(&source, &QNmeaSatelliteInfoSource::satellitesInUseUpdated);

    source.startUpdates();
    feeder->setMessages(messages);

    QTRY_VERIFY_WITH_TIMEOUT(messageSentSpy.size() == messages.size(), 2000);
    QVERIFY(!inViewSpy.isEmpty());
    QVERIFY(!inUseSpy.isEmpty());

    const auto inView = inViewSpy.back().at(0).value<QList<QGeoSatelliteInfo>>();
    QCOMPARE(inView, desiredInView);

    const auto inUse = inUseSpy.back().at(0).value<QList<QGeoSatelliteInfo>>();
    QCOMPARE(inUse, desiredInUse);
}

void tst_QNmeaSatelliteInfoSource::parseDataStream_data()
{
    QTest::addColumn<QNmeaSatelliteInfoSource::UpdateMode>("mode");
    QTest::addColumn<QList<QByteArray>>("messages");
    QTest::addColumn<QList<QGeoSatelliteInfo>>("desiredInView");
    QTest::addColumn<QList<QGeoSatelliteInfo>>("desiredInUse");

    // We will use only satId and SNR, to simplify objects for comparison

    // one line GPS
    const auto simpleGpsGsvMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                             "$GPGSV,1,1,4,05,,,25,07,,,,08,,,,13,,,36*")
                                             .toLatin1();
    const auto simpleGpsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                             "$GPGSA,A,3,05,13,,,,,,,,,,,50.95,50.94,1.00*")
                                             .toLatin1();

    const auto simpleGpsInView =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GPS, 5, 25),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 7, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 8, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 13, 36) };
    const auto simpleGpsInUse =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GPS, 5, 25),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 13, 36) };

    QTest::newRow("realtime GPS") << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
                                  << QList<QByteArray> { simpleGpsGsvMessage, simpleGpsGsaMessage }
                                  << simpleGpsInView << simpleGpsInUse;

    QTest::newRow("simulation GPS")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { simpleGpsGsvMessage, simpleGpsGsaMessage } << simpleGpsInView
            << simpleGpsInUse;

    // multi line GPS
    const auto complexGpsGsvMessage1 = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                               "$GPGSV,2,1,8,05,,,22,07,,,,08,,,,13,,,18*")
                                               .toLatin1();
    const auto complexGpsGsvMessage2 =
            QLocationTestUtils::addNmeaChecksumAndBreaks("$GPGSV,2,2,8,14,,,,15,,,18,18,,,,20,,,*")
                    .toLatin1();
    const auto complexGpsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                              "$GPGSA,A,3,05,13,15,,,,,,,,,,50.95,50.94,1.00*")
                                              .toLatin1();

    const auto complexGpsInView =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GPS, 5, 22),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 7, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 8, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 13, 18),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 14, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 15, 18),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 18, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 20, -1) };
    const auto complexGpsInUse =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GPS, 5, 22),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 13, 18),
                                       createSatelliteInfo(QGeoSatelliteInfo::GPS, 15, 18) };

    QTest::newRow("realtime multi-line GPS")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { complexGpsGsvMessage1, complexGpsGsvMessage2,
                                   complexGpsGsaMessage }
            << complexGpsInView << complexGpsInUse;

    QTest::newRow("simulation multi-line GPS")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { complexGpsGsvMessage1, complexGpsGsvMessage2,
                                   complexGpsGsaMessage }
            << complexGpsInView << complexGpsInUse;

    // one line GPS & GLONASS
    const auto simpleGlnsGsvMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                              "$GLGSV,1,1,4,65,,,,66,,,,71,,,20,72,,,28*")
                                              .toLatin1();
    const auto gnSimpleGlnsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                                "$GNGSA,A,3,71,72,,,,,,,,,,,50.95,50.94,1.00*")
                                                .toLatin1();
    const auto gnSimpleGpsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                               "$GNGSA,A,3,05,13,,,,,,,,,,,50.95,50.94,1.00*")
                                               .toLatin1();

    const auto simpleGlnsInView =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 65, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 66, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 71, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 72, 28) };
    const auto simpleGlnsInUse =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 71, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 72, 28) };

    const auto simpleGpsGlnsInView = simpleGpsInView + simpleGlnsInView;
    const auto simpleGpsGlnsInUse = simpleGpsInUse + simpleGlnsInUse;

    QTest::newRow("realtime GPS & GLONASS")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { simpleGpsGsvMessage, simpleGlnsGsvMessage, gnSimpleGpsGsaMessage,
                                   gnSimpleGlnsGsaMessage }
            << simpleGpsGlnsInView << simpleGpsGlnsInUse;

    QTest::newRow("simulation GPS & GLONASS")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { simpleGpsGsvMessage, simpleGlnsGsvMessage, gnSimpleGpsGsaMessage,
                                   gnSimpleGlnsGsaMessage }
            << simpleGpsGlnsInView << simpleGpsGlnsInUse;

    // multi line GPS & GLONASS
    const auto complexGlnsGsvMessage1 = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                                "$GLGSV,3,1,10,65,,,,66,,,,71,,,20,72,,,28*")
                                                .toLatin1();
    const auto complexGlnsGsvMessage2 =
            QLocationTestUtils::addNmeaChecksumAndBreaks("$GLGSV,3,2,10,73,,,,74,,,,75,,,33,81,,,*")
                    .toLatin1();
    const auto complexGlnsGsvMessage3 =
            QLocationTestUtils::addNmeaChecksumAndBreaks("$GLGSV,3,3,10,82,,,,83,,,*").toLatin1();
    const auto gnComplexGlnsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                                 "$GNGSA,A,3,71,72,75,,,,,,,,,,50.95,50.94,1.00*")
                                                 .toLatin1();
    const auto gnComplexGpsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                                "$GNGSA,A,3,05,13,15,,,,,,,,,,50.95,50.94,1.00*")
                                                .toLatin1();

    const auto complexGlnsInView =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 65, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 66, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 71, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 72, 28),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 73, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 74, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 75, 33),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 81, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 82, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 83, -1) };
    const auto complexGlnsInUse =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 71, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 72, 28),
                                       createSatelliteInfo(QGeoSatelliteInfo::GLONASS, 75, 33) };

    const auto complexGpsGlnsInView = complexGpsInView + complexGlnsInView;
    const auto complexGpsGlnsInUse = complexGpsInUse + complexGlnsInUse;

    QTest::newRow("realtime multi-line GPS & GLONASS")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { complexGpsGsvMessage1,  complexGpsGsvMessage2,
                                   complexGlnsGsvMessage1, complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3, gnComplexGpsGsaMessage,
                                   gnComplexGlnsGsaMessage }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    QTest::newRow("simulation multi-line GPS & GLONASS")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { complexGpsGsvMessage1,  complexGpsGsvMessage2,
                                   complexGlnsGsvMessage1, complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3, gnComplexGpsGsaMessage,
                                   gnComplexGlnsGsaMessage }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    // multi-line GPS & GLONASS: GSA before GSV
    QTest::newRow("realtime multi-line GPS & GLONASS reverse")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { gnComplexGlnsGsaMessage, gnComplexGpsGsaMessage,
                                   complexGlnsGsvMessage1,  complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3,  complexGpsGsvMessage1,
                                   complexGpsGsvMessage2 }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    QTest::newRow("simulation multi-line GPS & GLONASS reverse")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { gnComplexGlnsGsaMessage, gnComplexGpsGsaMessage,
                                   complexGlnsGsvMessage1,  complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3,  complexGpsGsvMessage1,
                                   complexGpsGsvMessage2 }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    // multi-line GSP & GLONASS: mixed order
    QTest::newRow("realtime multi-line GPS & GLONASS mixed")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { gnComplexGlnsGsaMessage, complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2,  complexGlnsGsvMessage3,
                                   complexGpsGsvMessage1,   complexGpsGsvMessage2,
                                   gnComplexGpsGsaMessage }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    QTest::newRow("simulation multi-line GPS & GLONASS mixed")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { gnComplexGlnsGsaMessage, complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2,  complexGlnsGsvMessage3,
                                   complexGpsGsvMessage1,   complexGpsGsvMessage2,
                                   gnComplexGpsGsaMessage }
            << complexGpsGlnsInView << complexGpsGlnsInUse;

    // one line GPS & GLONASS: empty GNGSA for GPS
    const auto gnEmptyGpsGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                              "$GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*")
                                              .toLatin1();

    QTest::newRow("realtime GSP & GLONASS: empty GSA for GPS")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { simpleGpsGsvMessage, simpleGlnsGsvMessage,
                                   gnSimpleGlnsGsaMessage, gnEmptyGpsGsaMessage }
            << simpleGpsGlnsInView << simpleGlnsInUse;

    QTest::newRow("simulation GSP & GLONASS: empty GSA for GPS")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { simpleGpsGsvMessage, simpleGlnsGsvMessage,
                                   gnSimpleGlnsGsaMessage, gnEmptyGpsGsaMessage }
            << simpleGpsGlnsInView << simpleGlnsInUse;

    // one line GPS & GLONASS: empty $GPGSV
    const auto emptyGpsGsvMessage =
            QLocationTestUtils::addNmeaChecksumAndBreaks("$GPGSV,1,1,0,,,,,,,,,,,,,,,,*")
                    .toLatin1();

    QTest::newRow("realtime GPS & GLONASS: empty $GPGSV")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { emptyGpsGsvMessage, simpleGlnsGsvMessage, gnSimpleGlnsGsaMessage,
                                   gnSimpleGpsGsaMessage }
            << simpleGlnsInView << simpleGlnsInUse;

    QTest::newRow("simulation GPS & GLONASS: empty $GPGSV")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { emptyGpsGsvMessage, simpleGlnsGsvMessage, gnSimpleGlnsGsaMessage,
                                   gnSimpleGpsGsaMessage }
            << simpleGlnsInView << simpleGlnsInUse;

    // one line GPS & GLONASS: empty GNGSA in the middle
    // In this test we send the following sequence:
    // 1. $GPGSV - valid GPS in view
    // 2. $GLGSV - valid GLONASS in view
    // 3. $GNGSA - valid GPS in use
    // 4. $GNGSA - valid GLONASS in use
    // 5. $GPGSV - valid GPS in view
    // 6. $GNGSA - empty GNGSA (meaning empty GPS)
    // 7. $GPGSV - valid GPS in view: at this point GPS in use will be cleared
    // As a result, we will see valid GPS & GLONASS in view, but only GLONASS
    // in use.

    QTest::newRow("realtime GPS & GLONASS: empty $GNGSA in the middle")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { simpleGpsGsvMessage,   simpleGlnsGsvMessage,
                                   gnSimpleGpsGsaMessage, gnSimpleGlnsGsaMessage,
                                   simpleGpsGsvMessage,   gnEmptyGpsGsaMessage,
                                   simpleGpsGsvMessage }
            << simpleGpsGlnsInView << simpleGlnsInUse;

    QTest::newRow("simulation GPS & GLONASS: empty $GNGSA in the middle")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { simpleGpsGsvMessage,   simpleGlnsGsvMessage,
                                   gnSimpleGpsGsaMessage, gnSimpleGlnsGsaMessage,
                                   simpleGpsGsvMessage,   gnEmptyGpsGsaMessage,
                                   simpleGpsGsvMessage }
            << simpleGpsGlnsInView << simpleGlnsInUse;

    // BEIDOU signals for below test cases are just synthesized based on the
    // NMEA protocol description. They were NEVER checked on real hardware,
    // as we do not have one.

    // multi-line GPS, GLONASS & BEIDOU
    const auto complexBduGsvMessage1 = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                               "$GBGSV,2,1,7,201,,,,202,,,,203,,,20,204,,,28*")
                                               .toLatin1();
    const auto complexBduGsvMessage2 =
            QLocationTestUtils::addNmeaChecksumAndBreaks("$GBGSV,2,2,7,205,,,,206,,,,207,,,33,,,,*")
                    .toLatin1();
    const auto gnComplexBduGsaMessage = QLocationTestUtils::addNmeaChecksumAndBreaks(
                                                "$GNGSA,A,3,203,204,207,,,,,,,,,,50.95,50.94,1.00*")
                                                .toLatin1();
    const auto complexBduInView =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 201, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 202, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 203, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 204, 28),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 205, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 206, -1),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 207, 33) };
    const auto complexBduInUse =
            QList<QGeoSatelliteInfo> { createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 203, 20),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 204, 28),
                                       createSatelliteInfo(QGeoSatelliteInfo::BEIDOU, 207, 33) };

    const auto complexGpsGlnsBduInView = complexGpsInView + complexGlnsInView + complexBduInView;
    const auto complexGpsGlnsBduInUse = complexGpsInUse + complexGlnsInUse + complexBduInUse;

    QTest::newRow("realtime GPS, GLONASS & BEIDOU")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { complexGpsGsvMessage1,   complexGpsGsvMessage2,
                                   complexGlnsGsvMessage1,  complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3,  complexBduGsvMessage1,
                                   complexBduGsvMessage2,   gnComplexGpsGsaMessage,
                                   gnComplexGlnsGsaMessage, gnComplexBduGsaMessage }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;

    QTest::newRow("simulation GPS, GLONASS & BEIDOU")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { complexGpsGsvMessage1,   complexGpsGsvMessage2,
                                   complexGlnsGsvMessage1,  complexGlnsGsvMessage2,
                                   complexGlnsGsvMessage3,  complexBduGsvMessage1,
                                   complexBduGsvMessage2,   gnComplexGpsGsaMessage,
                                   gnComplexGlnsGsaMessage, gnComplexBduGsaMessage }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;

    // multi-line GPS, GLONASS & BEIDOU: GSA before GSV
    QTest::newRow("realtime GPS, GLONASS & BEIDOU reverse")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { gnComplexGpsGsaMessage, gnComplexGlnsGsaMessage,
                                   gnComplexBduGsaMessage, complexGpsGsvMessage1,
                                   complexGpsGsvMessage2,  complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2, complexGlnsGsvMessage3,
                                   complexBduGsvMessage1,  complexBduGsvMessage2 }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;

    QTest::newRow("simulation GPS, GLONASS & BEIDOU reverse")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { gnComplexGpsGsaMessage, gnComplexGlnsGsaMessage,
                                   gnComplexBduGsaMessage, complexGpsGsvMessage1,
                                   complexGpsGsvMessage2,  complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2, complexGlnsGsvMessage3,
                                   complexBduGsvMessage1,  complexBduGsvMessage2 }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;

    // multi-line GPS, GLONASS & BEIDOU: mixed order
    QTest::newRow("realtime GPS, GLONASS & BEIDOU mixed")
            << QNmeaSatelliteInfoSource::UpdateMode::RealTimeMode
            << QList<QByteArray> { gnComplexGpsGsaMessage,  complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2,  complexGlnsGsvMessage3,
                                   complexBduGsvMessage1,   complexBduGsvMessage2,
                                   gnComplexGlnsGsaMessage, gnComplexBduGsaMessage,
                                   complexGpsGsvMessage1,   complexGpsGsvMessage2 }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;

    QTest::newRow("simulation GPS, GLONASS & BEIDOU mixed")
            << QNmeaSatelliteInfoSource::UpdateMode::SimulationMode
            << QList<QByteArray> { gnComplexGpsGsaMessage,  complexGlnsGsvMessage1,
                                   complexGlnsGsvMessage2,  complexGlnsGsvMessage3,
                                   complexBduGsvMessage1,   complexBduGsvMessage2,
                                   gnComplexGlnsGsaMessage, gnComplexBduGsaMessage,
                                   complexGpsGsvMessage1,   complexGpsGsvMessage2 }
            << complexGpsGlnsBduInView << complexGpsGlnsBduInUse;
}

QGeoSatelliteInfo
tst_QNmeaSatelliteInfoSource::createSatelliteInfo(QGeoSatelliteInfo::SatelliteSystem system, int id,
                                                  int snr)
{
    QGeoSatelliteInfo info;
    info.setSatelliteSystem(system);
    info.setSatelliteIdentifier(id);
    info.setSignalStrength(snr);
    info.setAttribute(QGeoSatelliteInfo::Azimuth, 0.0);
    info.setAttribute(QGeoSatelliteInfo::Elevation, 0.0);
    return info;
}

#include "tst_nmeasatelliteinfosource.moc"

QTEST_GUILESS_MAIN(tst_QNmeaSatelliteInfoSource)
