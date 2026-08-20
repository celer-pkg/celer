// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtCore/QtEndian>

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanframeprocessor.h>
#include <QtSerialBus/qcanmessagedescription.h>
#include <QtSerialBus/qcansignaldescription.h>
#include <QtSerialBus/private/qcanframeprocessor_p.h>

QT_USE_NAMESPACE

class tst_QCanFrameProcessor : public QObject
{
    Q_OBJECT

private slots:
    /* basics */
    void construct();

    /* parse */
    void parseSignal_data();
    void parseSignal();

    void parseWithValueConversion_data();
    void parseWithValueConversion();

    void parseMultiplexedSignals_data();
    void parseMultiplexedSignals();

    void parseExtendedMultiplexedSignals_data();
    void parseExtendedMultiplexedSignals();

    void parseWithErrorsAndWarnings_data();
    void parseWithErrorsAndWarnings();

    void extractUniqueId_data();
    void extractUniqueId();

    /* generate */
    void prepareFrame_data();
    void prepareFrame();

    void prepareWithValueConversion_data();
    void prepareWithValueConversion();

    void prepareMultiplexedPayload_data();
    void prepareMultiplexedPayload();

    void prepareWithErrorsAndWarnings_data();
    void prepareWithErrorsAndWarnings();

    void prepareUniqueId_data();
    void prepareUniqueId();

    /* roundtrip */
    void roundtrip_data();
    void roundtrip();
};

void tst_QCanFrameProcessor::construct()
{
    QCanFrameProcessor p;

    QVERIFY(p.messageDescriptions().isEmpty());
    QCOMPARE(p.error(), QCanFrameProcessor::Error::None);
    QVERIFY(p.errorString().isEmpty());
    QVERIFY(p.warnings().isEmpty());
    QVERIFY(!p.uniqueIdDescription().isValid());
}

void tst_QCanFrameProcessor::parseSignal_data()
{
    QTest::addColumn<QtCanBus::DataSource>("source");
    QTest::addColumn<quint64>("data");
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QSysInfo::Endian>("endian");
    QTest::addColumn<QVariant>("expectedValue");

    struct SourceDesc {
        QtCanBus::DataSource source;
        const char name[8];
    };

    static constexpr SourceDesc sources[] = {
        {QtCanBus::DataSource::FrameId, "FrameId"},
        {QtCanBus::DataSource::Payload, "Payload"}
    };

    for (const auto &s : sources) {
        // LE, 16 bit, start 0
        QTest::addRow("%s, unsigned, 16bit, start 0, le", s.name)
                << s.source
                << quint64(0x1234FFEC) << quint16(0) << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(65516);

        QTest::addRow("%s, signed neg, 16bit, start 0, le", s.name)
                << s.source
                << quint64(0x1234FFEC) << quint16(0) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-20);

        QTest::addRow("%s, signed pos, 16bit, start 0, le", s.name)
                << s.source
                << quint64(0x12340FEC) << quint16(0) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(4076);

        // LE, 16 bit, start 5
        QTest::addRow("%s, unsigned, 16bit, start 5, le", s.name)
                << s.source
                << quint64(0x1234FFECULL << 5)
                << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(65516);

        QTest::addRow("%s, signed neg, 16bit, start 5, le", s.name)
                << s.source
                << quint64((0x1234FFECULL << 5) & 0x1FFFFFFFU)
                << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-20);

        QTest::addRow("%s, signed pos, 16bit, start 5, le", s.name)
                << s.source
                << quint64(0x12340FECULL << 5)
                << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(4076);

        // LE, 19 bit, start 0
        QTest::addRow("%s, unsigned, 19bit, start 0, le", s.name)
                << s.source
                << quint64(0x1234FFEC) << quint16(0) << quint16(19)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(327660);

        QTest::addRow("%s, signed neg, 19bit, start 0, le", s.name)
                << s.source
                << quint64(0x123D800A) << quint16(0) << quint16(19)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-163830);

        QTest::addRow("%s, signed pos, 19bit, start 0, le", s.name)
                << s.source
                << quint64(0x1231800A) << quint16(0) << quint16(19)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(98314);

        // LE, 19 bit, start 3
        QTest::addRow("%s, unsigned, 19bit, start 3, le", s.name)
                << s.source
                << quint64(0x1234FFECULL << 3)
                << quint16(3) << quint16(19)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(327660);

        QTest::addRow("%s, signed neg, 19bit, start 3, le", s.name)
                << s.source
                << quint64(0x123D800AULL << 3)
                << quint16(3) << quint16(19)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-163830);

        QTest::addRow("%s, signed pos, 19bit, start 3, le", s.name)
                << s.source
                << quint64(0x1231800AULL << 3)
                << quint16(3) << quint16(19)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(98314);

        // BE, 16 bit, start 7
        QTest::addRow("%s, unsigned, 16bit, start 7, be", s.name)
                << s.source
                << quint64(0x1234ECFF) << quint16(7) << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(65516);

        QTest::addRow("%s, signed neg, 16bit, start 7, be", s.name)
                << s.source
                << quint64(0x1234ECFF) << quint16(7) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-20);

        QTest::addRow("%s, signed pos, 16bit, start 7, be", s.name)
                << s.source
                << quint64(0x1234EC0F) << quint16(7) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(4076);

        // BE, 16 bit, start 5
        QTest::addRow("%s, unsigned, 16bit, start 5, be", s.name)
                << s.source
                << quint64(0x1234FB3F) << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(65516);

        QTest::addRow("%s, signed neg, 16bit, start 5, be", s.name)
                << s.source
                << quint64(0x1234FB3F) << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-20);

        QTest::addRow("%s, signed pos, 16bit, start 5, be", s.name)
                << s.source
                << quint64(0x1234FB03) << quint16(5) << quint16(16)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(4076);

        // BE, 21 bit, start 7
        QTest::addRow("%s, unsigned, 21bit, start 7, be", s.name)
                << s.source
                << quint64(0x1264FFA7) << quint16(7) << quint16(21)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(1376236);

        QTest::addRow("%s, signed neg, 21bit, start 7, be", s.name)
                << s.source
                << quint64(0x1264FFA7) << quint16(7) << quint16(21)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-720916);

        QTest::addRow("%s, signed pos, 21bit, start 7, be", s.name)
                << s.source
                << quint64(0x1264FF27) << quint16(7) << quint16(21)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(327660);

        // BE, 21 bit, start 4
        QTest::addRow("%s, unsigned, 21bit, start 4, be", s.name)
                << s.source
                << quint64(0x12ECFF14) << quint16(4) << quint16(21)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(1376236);

        QTest::addRow("%s, signed neg, 21bit, start 4, be", s.name)
                << s.source
                << quint64(0x12ECFF14) << quint16(4) << quint16(21)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-720916);

        QTest::addRow("%s, signed pos, 21bit, start 4, be", s.name)
                << s.source
                << quint64(0x12ECFF04) << quint16(4) << quint16(21)
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(327660);

        // Some LE tests for values less than 8 bit
        QTest::addRow("%s, unsinged, 4bit, start 0, le", s.name)
                << s.source
                << quint64(0x12345678) << quint16(0) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(8);

        QTest::addRow("%s, unsinged, 4bit, start 3, le", s.name)
                << s.source
                << quint64(0x12345678) << quint16(3) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(15);

        QTest::addRow("%s, unsinged, 4bit, start 7, le", s.name)
                << s.source
                << quint64(0x12345678) << quint16(7) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(12);

        // Some BE tests for values less than 8 bit
        QTest::addRow("%s, unsinged, 4bit, start 7, be", s.name)
                << s.source
                << quint64(0x12345678) << quint16(7) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(7);

        QTest::addRow("%s, unsinged, 4bit, start 5, be", s.name)
                << s.source
                << quint64(0x12345678) << quint16(5) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(14);

        QTest::addRow("%s, unsinged, 4bit, start 0, be", s.name)
                << s.source
                << quint64(0x12345678) << quint16(0) << quint16(4)
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(2);
    }

    // float data - payload only
    {
        const float floatVal = 123.456f;
        for (quint16 i = 0; i < 8; ++i) {
            quint64 value = 0;
            const float leFloatVal = qToLittleEndian(floatVal);
            memcpy(&value, &leFloatVal, sizeof(float));
            value = value << i;
            QTest::addRow("Payload, float, le, start %d", i)
                    << QtCanBus::DataSource::Payload
                    << value << i << quint16(32)
                    << QtCanBus::DataFormat::Float
                    << QSysInfo::Endian::LittleEndian
                    << QVariant(floatVal);

            // to get the proper BE value with an offset, we need to take an LE
            // value, shift it to the right, and then swap bytes
            value = 0;
            memcpy(&value, &leFloatVal, sizeof(float));
            value = value << 16; // so that we have some room for right shift
                                 // this means that default start bit is 23
            value = value >> i;
            value = qToBigEndian(value);
            QTest::addRow("Payload, float, be, start %d", 23 - i)
                    << QtCanBus::DataSource::Payload
                    << value << quint16(23 - i) << quint16(32)
                    << QtCanBus::DataFormat::Float
                    << QSysInfo::Endian::BigEndian
                    << QVariant(floatVal);
        }
    }

    // double data - payload only
    {
        const double doubleVal = 1.234e-15;
        const double leDoubleVal = qToLittleEndian(doubleVal);
        quint64 value = 0;
        memcpy(&value, &leDoubleVal, sizeof(leDoubleVal));
        QTest::addRow("Payload, double, le")
                << QtCanBus::DataSource::Payload
                << value << quint16(0) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QSysInfo::Endian::LittleEndian
                << QVariant(doubleVal);

        const double beDoubleVal = qToBigEndian(doubleVal);
        memcpy(&value, &beDoubleVal, sizeof(beDoubleVal));
        QTest::addRow("Payload, double, be")
                << QtCanBus::DataSource::Payload
                << value << quint16(7) << quint16(64)
                << QtCanBus::DataFormat::Double
                << QSysInfo::Endian::BigEndian
                << QVariant(doubleVal);
    }

    // ASCII
    for (const auto &s : sources) {
        const bool isPayload = s.source == QtCanBus::DataSource::Payload;
        const QByteArray expectedData = isPayload ? QByteArray("abc", 3) : QByteArray("ab", 2);
        const quint16 offset = isPayload ? 8 : 4;
        for (quint16 i = 0; i < 8; ++i) {
            quint64 value = 0;
            memcpy(&value, expectedData.constData(), expectedData.size());
            value = value << (offset + i);
            QTest::addRow("%s, ascii, offset %d", s.name, offset + i)
                    << s.source
                    << value << quint16(offset + i) << quint16(expectedData.size() * 8)
                    << QtCanBus::DataFormat::AsciiString
                    << QSysInfo::Endian::LittleEndian
                    << QVariant(expectedData);
        }
    }
}

void tst_QCanFrameProcessor::parseSignal()
{
    QFETCH(QtCanBus::DataSource, source);
    QFETCH(quint64, data);
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QSysInfo::Endian, endian);
    QFETCH(QVariant, expectedValue);

    QCanSignalDescription sig;
    sig.setName("test_signal");
    sig.setDataSource(source);
    sig.setDataFormat(format);
    sig.setDataEndian(endian);
    sig.setStartBit(startBit);
    sig.setBitLength(bitLength);
    QVERIFY(sig.isValid());

    const bool sourceIsPayload = source == QtCanBus::DataSource::Payload;

    // The unique id will be extracted from the 0-th payload byte
    const auto uidSource = sourceIsPayload ? QtCanBus::DataSource::FrameId
                                           : QtCanBus::DataSource::Payload;
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(uidSource);
    uidDesc.setStartBit(0);
    uidDesc.setBitLength(4);

    const QtCanBus::UniqueId expectedUid{0x0A};

    const auto messageSize = sourceIsPayload ? sizeof(data) : 1;
    QCanMessageDescription message;
    message.setName("test_message");
    message.setUniqueId(expectedUid);
    message.setSize(messageSize);
    message.addSignalDescription(sig);
    QVERIFY(message.isValid());

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ message });

    QCanBusFrame frame;
    if (sourceIsPayload) {
        frame.setFrameId(static_cast<QCanBusFrame::FrameId>(expectedUid));
        const QByteArray payload(reinterpret_cast<const char *>(&data), sizeof(data));
        frame.setPayload(payload);
    } else {
        frame.setFrameId(data & 0x1FFFFFFFU);
        frame.setPayload(QByteArray(1, 0x0A));
    }
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    const auto signalValues = result.signalValues;
    QCOMPARE(result.uniqueId, expectedUid);
    QVERIFY(!signalValues.isEmpty());

    const QVariantMap expectedSignalValues({qMakePair(sig.name(), expectedValue)});

    // The values can be doubles, so we need to compare them explicitly
    QCOMPARE(signalValues.keys(), expectedSignalValues.keys());
    for (const auto &key : expectedSignalValues.keys()) {
        const auto expectedValue = expectedSignalValues.value(key);
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::parseWithValueConversion_data()
{
    QTest::addColumn<double>("factor");
    QTest::addColumn<double>("offset");
    QTest::addColumn<double>("scaling");
    QTest::addColumn<quint8>("payloadValue");
    QTest::addColumn<QVariant>("expectedValue");

    // expectedValue = scaling * (payloadValue * factor + offset);
    QTest::newRow("nan, nan, nan") << qQNaN() << qQNaN() << qQNaN() << quint8(30) << QVariant(30);
    QTest::newRow("0.75, nan, nan") << 0.75 << qQNaN() << qQNaN() << quint8(30) << QVariant(22.5);
    QTest::newRow("0.75, 15.3, nan") << 0.75 << 15.3 << qQNaN() << quint8(30) << QVariant(37.8);
    QTest::newRow("0.75, 15.3, 100") << 0.75 << 15.3 << 100. << quint8(30) << QVariant(3780.);
    QTest::newRow("0.75, nan, 100") << 0.75 << qQNaN() << 100. << quint8(30) << QVariant(2250.);
    QTest::newRow("nan, 15.3, 100") << qQNaN() << 15.3 << 100. << quint8(30) << QVariant(4530.);
    // check that setting factor and/or scaling to 0 is equivalent to NaN
    QTest::newRow("0, 15.3, 100") << 0. << 15.3 << 100. << quint8(30) << QVariant(4530.);
    QTest::newRow("0, 15.3, 0") << 0. << 15.3 << 0. << quint8(30) << QVariant(45.3);
}

void tst_QCanFrameProcessor::parseWithValueConversion()
{
    QFETCH(double, factor);
    QFETCH(double, offset);
    QFETCH(double, scaling);
    QFETCH(quint8, payloadValue);
    QFETCH(QVariant, expectedValue);

    QCanSignalDescription sig;
    sig.setName("test_signal");
    sig.setStartBit(7);
    sig.setBitLength(8);
    sig.setFactor(factor);
    sig.setOffset(offset);
    sig.setScaling(scaling);

    const QtCanBus::UniqueId uniqueId{123};

    QCanMessageDescription message;
    message.setName("test_message");
    message.setUniqueId(uniqueId);
    message.setSize(1);
    message.addSignalDescription(sig);
    QVERIFY(message.isValid());

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ message });

    QCanBusFrame frame(static_cast<QCanBusFrame::FrameId>(uniqueId),
                       QByteArray(1, char(payloadValue)));
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    const auto signalValues = result.signalValues;
    QCOMPARE(result.uniqueId, uniqueId);
    QVERIFY(!signalValues.isEmpty());

    const QVariantMap expectedSignalValues({qMakePair(sig.name(), expectedValue)});

    // The values can be doubles, so we need to compare them explicitly
    QCOMPARE(signalValues.keys(), expectedSignalValues.keys());
    for (const auto &key : expectedSignalValues.keys()) {
        const auto expectedValue = expectedSignalValues.value(key);
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::parseMultiplexedSignals_data()
{
    QTest::addColumn<QString>("s0Name");
    QTest::addColumn<QString>("s1Name");
    QTest::addColumn<QString>("s2Name");
    QTest::addColumn<QVariant>("s1MuxValue");
    QTest::addColumn<QVariant>("s2MuxValue");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QVariantMap>("expectedResult");

    const QString s0("s0");
    const QString s1("s1");
    const QString s2("s2");
    const QVariant s1MuxValue(1);
    const QVariant s2MuxValue(2);

    QTest::newRow("s0 and s1") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                               << QByteArray(1, 0x29)
                               << QVariantMap({ qMakePair(s0, s1MuxValue), qMakePair(s1, 0x0A) });

    QTest::newRow("s0 and s2") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                               << QByteArray(1, 0x2E)
                               << QVariantMap({ qMakePair(s0, s2MuxValue), qMakePair(s2, 0x0B) });

    QTest::newRow("only s0") << s0 << s1 << s2 << s1MuxValue << s2MuxValue
                             << QByteArray(1, 0x73)
                             << QVariantMap({ qMakePair(s0, 0x03) });

}

void tst_QCanFrameProcessor::parseMultiplexedSignals()
{
    QFETCH(QString, s0Name);
    QFETCH(QString, s1Name);
    QFETCH(QString, s2Name);
    QFETCH(QVariant, s1MuxValue);
    QFETCH(QVariant, s2MuxValue);
    QFETCH(QByteArray, payload);
    QFETCH(QVariantMap, expectedResult);

    QCanSignalDescription s0; // multiplexor
    s0.setName(s0Name);
    s0.setStartBit(1);
    s0.setBitLength(2);
    s0.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexed signal 1, used when s0 == 1
    s1.setName(s1Name);
    s1.setStartBit(7);
    s1.setBitLength(6);
    s1.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s1.addMultiplexSignal(s0.name(), s1MuxValue);

    QCanSignalDescription s2; // multiplexed signal 2, used when s0 == 2
    s2.setName(s2Name);
    s2.setStartBit(7);
    s2.setBitLength(6);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s2.addMultiplexSignal(s0.name(), s2MuxValue);

    const QtCanBus::UniqueId uniqueId{123};

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(payload.size());
    msg.setSignalDescriptions({ s0, s1, s2 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ msg });

    QCanBusFrame frame(static_cast<QCanBusFrame::FrameId>(msg.uniqueId()), payload);
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    QCOMPARE(result.uniqueId, uniqueId);
    QCOMPARE(result.signalValues, expectedResult);
}

void tst_QCanFrameProcessor::parseExtendedMultiplexedSignals_data()
{
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QVariantMap>("expectedResult");

    const QString s0Name("s0");
    const QString s1Name("s1");
    const QString s2Name("s2");
    const QString s3Name("s3");
    const QString s4Name("s4");

    QTest::newRow("s0 and s2") << QByteArray::fromHex("2434")
                               << QVariantMap({ qMakePair(s0Name, 4),
                                                qMakePair(s2Name, 0x342) });

    QTest::newRow("s0, s1 and s3") << QByteArray::fromHex("1134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 1),
                                                    qMakePair(s3Name, 0x34) });

    QTest::newRow("s0, s1 and s4") << QByteArray::fromHex("2134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 2),
                                                    qMakePair(s4Name, 0x34) });

    QTest::newRow("s0 and s1") << QByteArray::fromHex("7134")
                               << QVariantMap({ qMakePair(s0Name, 1),
                                                qMakePair(s1Name, 7) });

    QTest::newRow("only s0") << QByteArray::fromHex("2534")
                             << QVariantMap({ qMakePair(s0Name, 5) });
}

void tst_QCanFrameProcessor::parseExtendedMultiplexedSignals()
{
    QFETCH(QByteArray, payload);
    QFETCH(QVariantMap, expectedResult);

    QCanSignalDescription s0; // multiplexor 1
    s0.setName("s0");
    s0.setDataEndian(QSysInfo::Endian::LittleEndian);
    s0.setStartBit(0);
    s0.setBitLength(4);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexor 2, also depends on the value of s0
    s1.setName("s1");
    s1.setDataEndian(QSysInfo::Endian::LittleEndian);
    s1.setStartBit(4);
    s1.setBitLength(4);
    s1.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    s1.addMultiplexSignal(s0.name(), 1);

    QCanSignalDescription s2; // depends on mux1 only, contains 12 bits of data
    s2.setName("s2");
    s2.setDataEndian(QSysInfo::Endian::LittleEndian);
    s2.setStartBit(4);
    s2.setBitLength(12);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { {2, 4} };
    s2.addMultiplexSignal(s0.name(), muxValuesS0);

    QCanSignalDescription s3; // depends on mux2 (so indirectly on mux1)
    s3.setName("s3");
    s3.setDataEndian(QSysInfo::Endian::LittleEndian);
    s3.setStartBit(8);
    s3.setBitLength(8);
    s3.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s3.addMultiplexSignal(s1.name(), 1);

    QCanSignalDescription s4; // depends on mux2 (so indirectly on mux1)
    s4.setName("s4");
    s4.setDataEndian(QSysInfo::Endian::LittleEndian);
    s4.setStartBit(8);
    s4.setBitLength(8);
    s4.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS1 { {2, 3}, {5, 5} };
    s4.addMultiplexSignal(s1.name(), muxValuesS1);

    const QtCanBus::UniqueId uniqueId{123};

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(payload.size());
    msg.setSignalDescriptions({ s0, s1, s2, s3, s4 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uidDesc);
    parser.addMessageDescriptions({ msg });

    QCanBusFrame frame(static_cast<QCanBusFrame::FrameId>(msg.uniqueId()), payload);
    QVERIFY(frame.isValid());

    const auto result = parser.parseFrame(frame);
    QCOMPARE(result.uniqueId, uniqueId);
    QCOMPARE(result.signalValues, expectedResult);
}

void tst_QCanFrameProcessor::parseWithErrorsAndWarnings_data()
{
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanBusFrame>("frame");
    QTest::addColumn<QCanFrameProcessor::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorDescription");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QtCanBus::UniqueId>("expectedUniqueId");
    QTest::addColumn<QVariantMap>("expectedResult");

    QTest::addRow("Invalid frame") << QCanMessageDescription()
                                   << QCanUniqueIdDescription()
                                   << QCanBusFrame(0x2345ABCD, QByteArray())
                                   << QCanFrameProcessor::Error::InvalidFrame
                                   << tr("Invalid frame.")
                                   << QStringList()
                                   << QtCanBus::UniqueId(0)
                                   << QVariantMap();

    QTest::addRow("Unsupported frame") << QCanMessageDescription()
                                       << QCanUniqueIdDescription()
                                       << QCanBusFrame(QCanBusFrame::RemoteRequestFrame)
                                       << QCanFrameProcessor::Error::UnsupportedFrameFormat
                                       << tr("Unsupported frame format.")
                                       << QStringList()
                                       << QtCanBus::UniqueId(0)
                                       << QVariantMap();

    QTest::addRow("No UID description")
            << QCanMessageDescription() << QCanUniqueIdDescription()
            << QCanBusFrame(123, QByteArray(2, 0x01))
            << QCanFrameProcessor::Error::Decoding
            << tr("No valid unique identifier description is specified.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    uidDesc.setStartBit(8);
    uidDesc.setBitLength(16); // for extended FrameId

    QTest::addRow("UID extraction failed")
            << QCanMessageDescription() << uidDesc
            << QCanBusFrame(123, QByteArray(2, 0x01))
            << QCanFrameProcessor::Error::Decoding
            << tr("Failed to extract unique id from the frame.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    uidDesc = QCanUniqueIdDescription(); // reset
    uidDesc.setBitLength(29);

    QTest::addRow("Unknown unique id")
            << QCanMessageDescription() << uidDesc
            << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::Decoding
            << tr("Could not find a message description for unique id 123.")
            << QStringList() << QtCanBus::UniqueId(0) << QVariantMap();

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(7);
    signalDesc.setBitLength(16);

    QCanMessageDescription messageDesc;
    messageDesc.setName("testMessage");
    messageDesc.setUniqueId(QtCanBus::UniqueId{123});
    messageDesc.setSize(2);
    messageDesc.addSignalDescription(signalDesc);

    QTest::addRow("Invalid payload size")
            << messageDesc << uidDesc
            << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::Decoding
            << tr("Payload size does not match message description. "
                  "Actual size = 8, expected size = 2.")
            << QStringList()
            << QtCanBus::UniqueId(0)
            << QVariantMap();

    // format float, but length != 32
    QCanSignalDescription invalidSignalDesc;
    invalidSignalDesc.setName("invalidSignal");
    invalidSignalDesc.setStartBit(23);
    invalidSignalDesc.setBitLength(8);
    invalidSignalDesc.setDataFormat(QtCanBus::DataFormat::Float);

    messageDesc.setSize(8);
    messageDesc.addSignalDescription(invalidSignalDesc);

    QStringList expectedWarnings = { tr("Skipping signal invalidSignal in message with "
                                        "unique id 123 because its description is invalid.") };

    // Invalid signal skipped, and we still get results from other signals
    QTest::addRow("invalid signal desc warning")
            << messageDesc << uidDesc << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::None << QString()
            << expectedWarnings << QtCanBus::UniqueId(123)
            << QVariantMap({ qMakePair("s0", 0x0101) });

    // other valid signal
    signalDesc.setName("s1");
    signalDesc.setStartBit(39);
    signalDesc.setBitLength(8);

    messageDesc.addSignalDescription(signalDesc);

    // valid signal, but length exceeds frame length
    signalDesc.setName("tooLong");
    signalDesc.setStartBit(47);
    signalDesc.setBitLength(32);

    messageDesc.addSignalDescription(signalDesc);

    expectedWarnings.push_back(tr("Skipping signal tooLong in message with unique id 123. "
                                  "Its expected length exceeds the data length."));

    // Two invalid signals skipped, two valid results are received
    QTest::addRow("invalid signal desc warning")
            << messageDesc << uidDesc << QCanBusFrame(123, QByteArray(8, 0x01))
            << QCanFrameProcessor::Error::None << QString()
            << expectedWarnings << QtCanBus::UniqueId(123)
            << QVariantMap({ qMakePair("s0", 0x0101), qMakePair("s1", 0x01) });
}

void tst_QCanFrameProcessor::parseWithErrorsAndWarnings()
{
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanBusFrame, frame);
    QFETCH(QCanFrameProcessor::Error, expectedError);
    QFETCH(QString, expectedErrorDescription);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QtCanBus::UniqueId, expectedUniqueId);
    QFETCH(QVariantMap, expectedResult);

    QCanFrameProcessor parser;
    parser.setUniqueIdDescription(uniqueIdDescription);
    parser.addMessageDescriptions({ messageDescription });

    const auto result = parser.parseFrame(frame);

    QCOMPARE(parser.error(), expectedError);
    QCOMPARE(parser.errorString(), expectedErrorDescription);

    // signal descriptions are stored as a QHash internally, so they are
    // processed in a hash-dependent order. Because of that the order of
    // warnings cannot be defined. So we sort them alphabetically before
    // comparing.
    std::sort(expectedWarnings.begin(), expectedWarnings.end());
    QStringList warnings = parser.warnings();
    std::sort(warnings.begin(), warnings.end());
    QCOMPARE(warnings, expectedWarnings);

    QCOMPARE(result.uniqueId, expectedUniqueId);

    // The values can be doubles, so we need to compare them explicitly
    const auto signalValues = result.signalValues;
    QCOMPARE(signalValues.keys(), expectedResult.keys());
    for (auto it = expectedResult.cbegin(); it != expectedResult.cend(); ++it) {
        const auto &expectedValue = it.value();
        const auto &key = it.key();
        if (expectedValue.canConvert(QMetaType::fromType<double>()))
            QCOMPARE(signalValues.value(key).toDouble(), expectedValue.toDouble());
        else
            QCOMPARE(signalValues.value(key), expectedValue);
    }
}

void tst_QCanFrameProcessor::extractUniqueId_data()
{
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanBusFrame>("frame");
    QTest::addColumn<QtCanBus::UniqueId>("expectedUniqueId");

    // unique id in frame id
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::FrameId);
    uidDesc.setBitLength(12);

    const quint32 uniqueId = 0x0965;
    for (quint16 startBit = 0; startBit < 8; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
        const QCanBusFrame::FrameId frameIdLe = qToLittleEndian(uniqueId) << startBit;
        QTest::addRow("frameId, LE, start %d", startBit)
                << uidDesc
                << QCanBusFrame(frameIdLe, QByteArray(1, 0x01))
                << QtCanBus::UniqueId{uniqueId};
    }

    static constexpr quint16 startBitsFrame[] = { 3, 2, 1, 0, 15, 14, 13, 12 };
    static constexpr qsizetype startBitsFrameLen =
            sizeof(startBitsFrame)/sizeof(startBitsFrame[0]);

    for (qsizetype idx = 0; idx < startBitsFrameLen; ++idx) {
        const auto val = (qToLittleEndian(uniqueId) << 16) >> idx;
        const auto startBit = startBitsFrame[idx];
        const QCanBusFrame::FrameId frameIdBe = qToBigEndian(val);
        uidDesc.setEndian(QSysInfo::Endian::BigEndian);
        uidDesc.setStartBit(startBit);
        QTest::addRow("frameId, BE, start %d", startBit)
                << uidDesc
                << QCanBusFrame(frameIdBe, QByteArray(1, 0x01))
                << QtCanBus::UniqueId{uniqueId};
    }

    // unique id in payload, the 1st byte of the payload is used for signal,
    // so we start from the 2nd byte
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    for (quint16 startBit = 8; startBit < 16; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
        const quint32 payload = 0x01 | (qToLittleEndian(uniqueId) << startBit);
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        QTest::addRow("payload, LE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(0, payloadData)
                << QtCanBus::UniqueId{uniqueId};
    }

    static constexpr quint16 startBitsPayload[] = { 11, 10, 9, 8, 23, 22, 21, 20 };
    static constexpr qsizetype startBitsPayloadLen =
            sizeof(startBitsPayload)/sizeof(startBitsPayload[0]);

    for (qsizetype idx = 0; idx < startBitsPayloadLen; ++idx) {
        const auto startBit = startBitsPayload[idx];
        const auto val = (qToLittleEndian(uniqueId) << 8) >> idx;
        quint32 payload = 0x01 | qToBigEndian(val);
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        uidDesc.setEndian(QSysInfo::Endian::BigEndian);
        uidDesc.setStartBit(startBit);
        QTest::addRow("payload, BE, offset %d", startBit)
                << uidDesc
                << QCanBusFrame(0, payloadData)
                << QtCanBus::UniqueId{uniqueId};
    }
}

void tst_QCanFrameProcessor::extractUniqueId()
{
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanBusFrame, frame);
    QFETCH(QtCanBus::UniqueId, expectedUniqueId);

    // Some valid message description.
    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(7);
    signalDesc.setBitLength(8);

    QCanMessageDescription messageDesc;
    messageDesc.setName("test");
    messageDesc.setUniqueId(expectedUniqueId);
    messageDesc.setSize(frame.payload().size());
    messageDesc.addSignalDescription(signalDesc);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDesc });
    auto result = processor.parseFrame(frame);
    QCOMPARE(result.uniqueId, expectedUniqueId);
}

void tst_QCanFrameProcessor::prepareFrame_data()
{
    QTest::addColumn<quint16>("startBit");
    QTest::addColumn<quint16>("bitLength");
    QTest::addColumn<QtCanBus::DataSource>("source");
    QTest::addColumn<QtCanBus::DataFormat>("format");
    QTest::addColumn<QSysInfo::Endian>("endian");
    QTest::addColumn<QVariant>("signalValue");
    QTest::addColumn<quint64>("expectedData");

    struct SourceDesc {
        QtCanBus::DataSource source;
        const char name[8];
    };

    static constexpr SourceDesc sources[] = {
        {QtCanBus::DataSource::FrameId, "FrameId"},
        {QtCanBus::DataSource::Payload, "Payload"}
    };

    for (const auto &s : sources) {
        // LE, 16bit, start 0
        QTest::addRow("%s, unsigned, 16bit, start 0, le", s.name)
                << quint16(0) << quint16(16) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(65516)
                << quint64(0xFFEC);

        QTest::addRow("%s, signed neg, 16bit, start 0, le", s.name)
                << quint16(0) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-20)
                << quint64(0xFFEC);

        QTest::addRow("%s, signed pos, 16bit, start 0, le", s.name)
                << quint16(0) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(32748)
                << quint64(0x7FEC);

        // LE, 16bit, start 5
        QTest::addRow("%s, unsigned, 16bit, start 5, le", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(65516)
                << quint64(0xFFECULL << 5);

        QTest::addRow("%s, signed neg, 16bit, start 5, le", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-20)
                << quint64(0xFFECULL << 5);

        QTest::addRow("%s, signed pos, 16bit, start 5, le", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(32748)
                << quint64(0x7FECULL << 5);

        // LE, 19bit, start 0
        QTest::addRow("%s, unsigned, 19bit, start 0, le", s.name)
                << quint16(0) << quint16(19) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(327660)
                << quint64(0x4FFEC);

        QTest::addRow("%s, signed neg, 19bit, start 0, le", s.name)
                << quint16(0) << quint16(19) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-196628)
                << quint64(0x4FFEC);

        QTest::addRow("%s, signed pos, 19bit, start 0, le", s.name)
                << quint16(0) << quint16(19) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(196588)
                << quint64(0x2FFEC);

        // LE, 19bit, start 3
        QTest::addRow("%s, unsigned, 19bit, start 3, le", s.name)
                << quint16(3) << quint16(19) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(327660)
                << quint64(0x4FFECULL << 3);

        QTest::addRow("%s, signed neg, 19bit, start 3, le", s.name)
                << quint16(3) << quint16(19) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(-196628)
                << quint64(0x4FFECULL << 3);

        QTest::addRow("%s, signed pos, 19bit, start 3, le", s.name)
                << quint16(3) << quint16(19) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(196588)
                << quint64(0x2FFECULL << 3);

        // BE, 16bit, start 7
        QTest::addRow("%s, unsigned, 16bit, start 7, be", s.name)
                << quint16(7) << quint16(16) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(65516)
                << quint64(0xECFF);

        QTest::addRow("%s, signed neg, 16bit, start 7, be", s.name)
                << quint16(7) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-20)
                << quint64(0xECFF);

        QTest::addRow("%s, signed pos, 16bit, start 7, be", s.name)
                << quint16(7) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(32748)
                << quint64(0xEC7F);

        // BE, 16bit, start 5
        QTest::addRow("%s, unsigned, 16bit, start 5, be", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(65516)
                << quint64(0x00FB3F);

        QTest::addRow("%s, signed neg, 16bit, start 5, be", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-20)
                << quint64(0x00FB3F);

        QTest::addRow("%s, signed pos, 16bit, start 5, be", s.name)
                << quint16(5) << quint16(16) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(32748)
                << quint64(0x00FB1F);

        // BE, 21bit, start 7
        QTest::addRow("%s, unsigned, 21bit, start 7, be", s.name)
                << quint16(7) << quint16(21) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(1834988) /* 0x1BFFEC */
                << quint64(0x60FFDF);

        QTest::addRow("%s, signed neg, 21bit, start 7, be", s.name)
                << quint16(7) << quint16(21) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-262164)
                << quint64(0x60FFDF);

        QTest::addRow("%s, signed pos, 21bit, start 7, be", s.name)
                << quint16(7) << quint16(21) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(786412)
                << quint64(0x60FF5F);

        // BE, 21bit, start 4
        QTest::addRow("%s, unsigned, 21bit, start 4, be", s.name)
                << quint16(4) << quint16(21) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(1834988) /* 0x1BFFEC */
                << quint64(0xECFF1B);

        QTest::addRow("%s, signed neg, 21bit, start 4, be", s.name)
                << quint16(4) << quint16(21) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(-262164)
                << quint64(0xECFF1B);

        QTest::addRow("%s, signed pos, 21bit, start 4, be", s.name)
                << quint16(4) << quint16(21) << s.source
                << QtCanBus::DataFormat::SignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(786412)
                << quint64(0xECFF0B);

        // Some LE tests for values less than 8 bit
        QTest::addRow("%s, unsinged, 4bit, start 0, le", s.name)
                << quint16(0) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(8)
                << quint64(0x08);

        QTest::addRow("%s, unsinged, 4bit, start 3, le", s.name)
                << quint16(3) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(15)
                << quint64(0x78);

        QTest::addRow("%s, unsinged, 4bit, start 7, le", s.name)
                << quint16(7) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::LittleEndian
                << QVariant(12)
                << quint64(0x0600);

        // Some BE tests for values less than 8 bit
        QTest::addRow("%s, unsinged, 4bit, start 7, be", s.name)
                << quint16(7) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(8)
                << quint64(0x80);

        QTest::addRow("%s, unsinged, 4bit, start 5, be", s.name)
                << quint16(5) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(14)
                << quint64(0x38);

        QTest::addRow("%s, unsinged, 4bit, start 0, be", s.name)
                << quint16(0) << quint16(4) << s.source
                << QtCanBus::DataFormat::UnsignedInteger
                << QSysInfo::Endian::BigEndian
                << QVariant(3)
                << quint64(0x6000);
    }

    // float data - payload only
    {
        const float floatVal = 123.456f;
        for (quint16 i = 0; i < 8; ++i) {
            const float leFloatVal = qToLittleEndian(floatVal);
            quint64 value = 0;
            memcpy(&value, &leFloatVal, sizeof(leFloatVal));
            value = value << i;
            QTest::addRow("Payload, float, start %d, le", i)
                    << quint16(i) << quint16(32)
                    << QtCanBus::DataSource::Payload
                    << QtCanBus::DataFormat::Float
                    << QSysInfo::Endian::LittleEndian
                    << QVariant(floatVal)
                    << value;

            // to get the proper BE value with an offset, we need to take an LE
            // value, shift it to the right, and then swap bytes
            value = 0;
            memcpy(&value, &leFloatVal, sizeof(leFloatVal));
            value = value << 16; // so that we have some room for right shift
                                 // this means that default start bit is 23
            value = value >> i;
            value = qToBigEndian(value);
            QTest::addRow("Payload, float, start %d, be", 23 - i)
                    << quint16(23 - i) << quint16(32)
                    << QtCanBus::DataSource::Payload
                    << QtCanBus::DataFormat::Float
                    << QSysInfo::Endian::BigEndian
                    << QVariant(floatVal)
                    << value;
        }
    }

    // double data - payload only
    {
        const double doubleVal = 1.234e-15;
        const double leDoubleVal = qToLittleEndian(doubleVal);
        quint64 value = 0;
        memcpy(&value, &leDoubleVal, sizeof(leDoubleVal));
        QTest::addRow("Payload, double, le")
                << quint16(0) << quint16(64)
                << QtCanBus::DataSource::Payload
                << QtCanBus::DataFormat::Double
                << QSysInfo::Endian::LittleEndian
                << QVariant(doubleVal)
                << value;

        const double beDoubleVal = qToBigEndian(doubleVal);
        value = 0;
        memcpy(&value, &beDoubleVal, sizeof(beDoubleVal));
        QTest::addRow("Payload, double, be")
                << quint16(7) << quint16(64)
                << QtCanBus::DataSource::Payload
                << QtCanBus::DataFormat::Double
                << QSysInfo::Endian::BigEndian
                << QVariant(doubleVal)
                << value;
    }

    // ASCII
    for (const auto &s : sources) {
        const bool isPayload = s.source == QtCanBus::DataSource::Payload;
        const QByteArray asciiData = isPayload ? QByteArray("abc", 3) : QByteArray("ab", 2);
        const quint16 offset = isPayload ? 8 : 4;
        for (quint16 i = 0; i < 8; ++i) {
            quint64 value = 0;
            memcpy(&value, asciiData.constData(), asciiData.size());
            value = value << (offset + i);
            QTest::addRow("%s, ascii, start %d", s.name, offset + i)
                    << quint16(offset + i) << quint16(asciiData.size() * 8) << s.source
                    << QtCanBus::DataFormat::AsciiString
                    << QSysInfo::Endian::LittleEndian /* does not matter */
                    << QVariant(asciiData)
                    << value;
        }
    }
    // ASCII - not enough data
    {
        const QByteArray data("ab", 2);
        QByteArray expectedData = data;
        expectedData.push_back('\0'); // ab\0

        quint64 value = 0;
        memcpy(&value, expectedData.constData(), expectedData.size());

        QTest::addRow("ascii, not enough data")
                << quint16(0) << quint16(expectedData.size() * 8)
                << QtCanBus::DataSource::Payload
                << QtCanBus::DataFormat::AsciiString
                << QSysInfo::Endian::LittleEndian
                << QVariant(data)
                << value;
    }
    // ASCII - too much data
    {
        const QByteArray data("abcd", 4);
        const QByteArray expectedData = data.left(3); // abc

        quint64 value = 0;
        memcpy(&value, expectedData.constData(), expectedData.size());

        QTest::addRow("ascii, too much data")
                << quint16(0) << quint16(expectedData.size() * 8)
                << QtCanBus::DataSource::Payload
                << QtCanBus::DataFormat::AsciiString
                << QSysInfo::Endian::LittleEndian
                << QVariant(data)
                << value;
    }
}

void tst_QCanFrameProcessor::prepareFrame()
{
    QFETCH(quint16, startBit);
    QFETCH(quint16, bitLength);
    QFETCH(QtCanBus::DataSource, source);
    QFETCH(QtCanBus::DataFormat, format);
    QFETCH(QSysInfo::Endian, endian);
    QFETCH(QVariant, signalValue);
    QFETCH(quint64, expectedData);

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(startBit);
    signalDesc.setBitLength(bitLength);
    signalDesc.setDataSource(source);
    signalDesc.setDataFormat(format);
    signalDesc.setDataEndian(endian);

    // Unique Id will be located in a different place from the signal data.
    // If it's in a payload, we assume that it's in the first byte.
    // If it's in the frame id, we assume that it takes the whole frame id.
    const bool sourceInPayload = source == QtCanBus::DataSource::Payload;
    const auto uidSource = sourceInPayload ? QtCanBus::DataSource::FrameId
                                           : QtCanBus::DataSource::Payload;
    const auto uidLength = sourceInPayload ? 29 : 8;

    static constexpr quint32 uniqueId = 123;

    QCanUniqueIdDescription uidDesc;
    uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
    uidDesc.setSource(uidSource);
    uidDesc.setStartBit(0);
    uidDesc.setBitLength(uidLength);

    const auto messageSize = sourceInPayload ? sizeof(expectedData) : 1;
    QCanMessageDescription messageDesc;
    messageDesc.setName("test");
    messageDesc.setUniqueId(QtCanBus::UniqueId{uniqueId});
    messageDesc.setSize(messageSize);
    messageDesc.setSignalDescriptions({ signalDesc });

    const QVariantMap signalValues = { qMakePair(signalDesc.name(), signalValue) };

    QCanBusFrame expectedFrame;
    if (sourceInPayload) {
        expectedFrame.setFrameId(uniqueId & 0x1FFFFFFFU);
        const QByteArray payload(reinterpret_cast<const char *>(&expectedData),
                                 sizeof(expectedData));
        expectedFrame.setPayload(payload);
    } else {
        expectedFrame.setFrameId(expectedData & 0x1FFFFFFFU);
        // Explicitly truncating uniqueId to 1 byte. Fine, because it equals 123.
        expectedFrame.setPayload(QByteArray(1, static_cast<char>(uniqueId)));
    }
    QVERIFY(expectedFrame.isValid());

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ messageDesc });
    const QCanBusFrame frame = processor.prepareFrame(QtCanBus::UniqueId{uniqueId}, signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), expectedFrame.frameId());
    QCOMPARE(frame.payload(), expectedFrame.payload());
}

void tst_QCanFrameProcessor::prepareWithValueConversion_data()
{
    QTest::addColumn<double>("factor");
    QTest::addColumn<double>("offset");
    QTest::addColumn<double>("scaling");
    QTest::addColumn<QVariant>("signalValue");
    QTest::addColumn<QByteArray>("expectedPayload");

    // payloadValue = (signalValue / scaling - offset) / factor;
    QTest::newRow("nan, nan, nan") << qQNaN() << qQNaN() << qQNaN() << QVariant(30) << QByteArray(1, 30);
    QTest::newRow("0.75, nan, nan") << 0.75 << qQNaN() << qQNaN() << QVariant(22.5) << QByteArray(1, 30);
    QTest::newRow("0.75, 15.3, nan") << 0.75 << 15.3 << qQNaN() << QVariant(37.8) << QByteArray(1, 30);
    QTest::newRow("0.75, 15.3, 100") << 0.75 << 15.3 << 100. << QVariant(3780.) << QByteArray(1, 30);
    QTest::newRow("0.75, nan, 100") << 0.75 << qQNaN() << 100. << QVariant(2250.) << QByteArray(1, 30);
    QTest::newRow("nan, 15.3, 100") << qQNaN() << 15.3 << 100. << QVariant(4530.) << QByteArray(1, 30);
    // check that setting factor and/or scaling to 0 is equivalent to NaN
    QTest::newRow("0, 15.3, 100") << 0. << 15.3 << 100. << QVariant(4530.) << QByteArray(1, 30);
    QTest::newRow("0, 15.3, 0") << 0. << 15.3 << 0. << QVariant(45.3) << QByteArray(1, 30);
}

void tst_QCanFrameProcessor::prepareWithValueConversion()
{
    QFETCH(double, factor);
    QFETCH(double, offset);
    QFETCH(double, scaling);
    QFETCH(QVariant, signalValue);
    QFETCH(QByteArray, expectedPayload);

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(7);
    signalDesc.setBitLength(8);
    signalDesc.setFactor(factor);
    signalDesc.setOffset(offset);
    signalDesc.setScaling(scaling);

    const QtCanBus::UniqueId uniqueId{123};

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(uniqueId);
    messageDesc.setSize(expectedPayload.size());
    messageDesc.addSignalDescription(signalDesc);

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    const QVariantMap signalValues = { qMakePair(signalDesc.name(), signalValue) };

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ messageDesc });
    const QCanBusFrame frame = processor.prepareFrame(uniqueId, signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), static_cast<QCanBusFrame::FrameId>(uniqueId));
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareMultiplexedPayload_data()
{
    QTest::addColumn<QByteArray>("expectedPayload");
    QTest::addColumn<QVariantMap>("signalValues");

    const QString s0Name("s0");
    const QString s1Name("s1");
    const QString s2Name("s2");
    const QString s3Name("s3");
    const QString s4Name("s4");

    QTest::newRow("s0 and s2") << QByteArray::fromHex("2334")
                               << QVariantMap({ qMakePair(s0Name, 3),
                                                qMakePair(s2Name, 0x342) });

    QTest::newRow("s0, s1 and s3") << QByteArray::fromHex("1134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 1),
                                                    qMakePair(s3Name, 0x34) });

    QTest::newRow("s0, s1 and s4") << QByteArray::fromHex("5134")
                                   << QVariantMap({ qMakePair(s0Name, 1),
                                                    qMakePair(s1Name, 5),
                                                    qMakePair(s4Name, 0x34) });

    QTest::newRow("s0 and s1") << QByteArray::fromHex("3100")
                               << QVariantMap({ qMakePair(s0Name, 1),
                                                qMakePair(s1Name, 3) });

    QTest::newRow("only s0") << QByteArray::fromHex("0300")
                             << QVariantMap({ qMakePair(s0Name, 3) });
}

void tst_QCanFrameProcessor::prepareMultiplexedPayload()
{
    QFETCH(QByteArray, expectedPayload);
    QFETCH(QVariantMap, signalValues);

    QCanSignalDescription s0; // multiplexor 1
    s0.setName("s0");
    s0.setDataEndian(QSysInfo::Endian::LittleEndian);
    s0.setStartBit(0);
    s0.setBitLength(4);
    s0.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);

    QCanSignalDescription s1; // multiplexor 2, also depends on the value of s0
    s1.setName("s1");
    s1.setDataEndian(QSysInfo::Endian::LittleEndian);
    s1.setStartBit(4);
    s1.setBitLength(4);
    s1.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    s1.addMultiplexSignal(s0.name(), 1);

    QCanSignalDescription s2; // depends on mux1 only, contains 12 bits of data
    s2.setName("s2");
    s2.setDataEndian(QSysInfo::Endian::LittleEndian);
    s2.setStartBit(4);
    s2.setBitLength(12);
    s2.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { {2, 4} };
    s2.addMultiplexSignal(s0.name(), muxValuesS0);

    QCanSignalDescription s3; // depends on mux2 (so indirectly on mux1)
    s3.setName("s3");
    s3.setDataEndian(QSysInfo::Endian::LittleEndian);
    s3.setStartBit(8);
    s3.setBitLength(8);
    s3.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    s3.addMultiplexSignal(s1.name(), 1);

    QCanSignalDescription s4; // depends on mux2 (so indirectly on mux1)
    s4.setName("s4");
    s4.setDataEndian(QSysInfo::Endian::LittleEndian);
    s4.setStartBit(8);
    s4.setBitLength(8);
    s4.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS1 { {2, 3}, {5, 5} };
    s4.addMultiplexSignal(s1.name(), muxValuesS1);

    const QtCanBus::UniqueId uniqueId{123};

    QCanMessageDescription msg;
    msg.setName("test");
    msg.setUniqueId(uniqueId);
    msg.setSize(expectedPayload.size());
    msg.setSignalDescriptions({ s0, s1, s2, s3, s4 });

    // Assume that our uniqueId is the whole FrameId
    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uidDesc);
    processor.addMessageDescriptions({ msg });

    const QCanBusFrame frame = processor.prepareFrame(msg.uniqueId(), signalValues);

    QVERIFY(frame.isValid());
    QCOMPARE(frame.frameId(), static_cast<QCanBusFrame::FrameId>(uniqueId));
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareWithErrorsAndWarnings_data()
{
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QtCanBus::UniqueId>("uniqueId");
    QTest::addColumn<QVariantMap>("signalValues");
    QTest::addColumn<QCanFrameProcessor::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorDescription");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QCanBusFrame::FrameId>("expectedFrameId");
    QTest::addColumn<QByteArray>("expectedPayload");

    QTest::addRow("no UID description")
            << QCanMessageDescription() << QCanUniqueIdDescription()
            << QtCanBus::UniqueId(0) << QVariantMap()
            << QCanFrameProcessor::Error::Encoding
            << tr("No valid unique identifier description is specified.")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    QCanUniqueIdDescription uidDesc;
    uidDesc.setBitLength(29);

    QTest::addRow("no message description")
            << QCanMessageDescription() << uidDesc << QtCanBus::UniqueId(123)
            << QVariantMap() << QCanFrameProcessor::Error::Encoding
            << tr("Failed to find message description for unique id 123.")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setStartBit(3);
    signalDesc.setBitLength(4);

    QCanMessageDescription messageDesc;
    messageDesc.setUniqueId(QtCanBus::UniqueId{123});
    messageDesc.setSize(2);
    messageDesc.addSignalDescription(signalDesc);

    uidDesc.setSource(QtCanBus::DataSource::Payload);
    uidDesc.setStartBit(8);
    uidDesc.setBitLength(16); // does not fit into the payload

    QTest::addRow("set UID fail")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap() << QCanFrameProcessor::Error::Encoding
            << tr("Failed to encode unique id 123 into the frame")
            << QStringList() << QCanBusFrame::FrameId(0) << QByteArray();

    // reset uidDesc
    uidDesc = QCanUniqueIdDescription();
    uidDesc.setBitLength(29);

    QStringList warnings = { tr("Skipping signal s1. It is not found in message "
                                "description for unique id 123.") };

    QTest::addRow("no signal description")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2) })
            << QCanFrameProcessor::Error::None << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // invalid signal desc - 0 length
    signalDesc.setName("s2");
    signalDesc.setStartBit(7);
    signalDesc.setBitLength(0);

    messageDesc.addSignalDescription(signalDesc);

    warnings.push_back(tr("Skipping signal s2. Its description is invalid."));

    QTest::addRow("invalid signal description")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2), qMakePair("s2", 3) })
            << QCanFrameProcessor::Error::None << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // max bit exceed data length
    signalDesc.setName("s3");
    signalDesc.setStartBit(15);
    signalDesc.setBitLength(10);

    messageDesc.addSignalDescription(signalDesc);

    warnings.push_back(tr("Skipping signal s3. Its length exceeds the expected message length."));

    QTest::addRow("invalid signal length")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 2),
                             qMakePair("s2", 3), qMakePair("s3", 4) })
            << QCanFrameProcessor::Error::None << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("0100");

    // Test that multiplexed signals are used only when proper multiplexor
    // values are provided.
    messageDesc.clearSignalDescriptions();

    // first multiplexor
    signalDesc.setName("s0");
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
    signalDesc.setStartBit(7);
    signalDesc.setBitLength(4);
    messageDesc.addSignalDescription(signalDesc);

    // s1 is used only when s0 is in range [2, 4] or 6
    signalDesc.setName("s1");
    signalDesc.setStartBit(3);
    signalDesc.setBitLength(12);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    const QCanSignalDescription::MultiplexValues muxValuesS0 { {2, 4}, {6, 6} };
    signalDesc.addMultiplexSignal("s0", muxValuesS0);
    messageDesc.addSignalDescription(signalDesc);

    // second multiplexor, used when s0 == 1
    signalDesc.setName("s2");
    signalDesc.setStartBit(3);
    signalDesc.setBitLength(4);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    messageDesc.addSignalDescription(signalDesc);

    // s3 depends on two multiplexors
    signalDesc.setName("s3");
    signalDesc.setStartBit(15);
    signalDesc.setBitLength(8);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    const QCanSignalDescription::MultiplexValues muxValuesS2 { {1, 3}, {5, 6} };
    signalDesc.addMultiplexSignal("s2", muxValuesS2);
    messageDesc.addSignalDescription(signalDesc);

    // s4 depends on two multiplexors
    signalDesc.setName("s4");
    signalDesc.setStartBit(15);
    signalDesc.setBitLength(8);
    signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
    signalDesc.addMultiplexSignal("s0", 1);
    signalDesc.addMultiplexSignal("s2", 4);
    messageDesc.addSignalDescription(signalDesc);

    warnings.clear();
    warnings.push_back(tr("Skipping signal s1. Proper multiplexor values not found."));
    warnings.push_back(tr("Skipping signal s4. Proper multiplexor values not found."));

    QTest::addRow("multiplexed signals")
            << messageDesc << uidDesc << messageDesc.uniqueId()
            << QVariantMap({ qMakePair("s0", 1), qMakePair("s1", 10), qMakePair("s2", 2),
                             qMakePair("s3", 5), qMakePair("s4", 6) })
            << QCanFrameProcessor::Error::None << QString()
            << warnings << QCanBusFrame::FrameId(messageDesc.uniqueId())
            << QByteArray::fromHex("1205");
}

void tst_QCanFrameProcessor::prepareWithErrorsAndWarnings()
{
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QtCanBus::UniqueId, uniqueId);
    QFETCH(QVariantMap, signalValues);
    QFETCH(QCanFrameProcessor::Error, expectedError);
    QFETCH(QString, expectedErrorDescription);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QCanBusFrame::FrameId, expectedFrameId);
    QFETCH(QByteArray, expectedPayload);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDescription });

    const QCanBusFrame frame = processor.prepareFrame(uniqueId, signalValues);

    QCOMPARE(processor.error(), expectedError);
    QCOMPARE(processor.errorString(), expectedErrorDescription);
    // signal descriptions are stored as a QHash internally, so they are
    // processed in a hash-dependent order. Because of that the order of
    // warnings cannot be defined. So we sort them alphabetically before
    // comparing.
    std::sort(expectedWarnings.begin(), expectedWarnings.end());
    QStringList warnings = processor.warnings();
    std::sort(warnings.begin(), warnings.end());
    QCOMPARE(warnings, expectedWarnings);

    QCOMPARE(frame.frameId(), expectedFrameId);
    QCOMPARE(frame.payload(), expectedPayload);
}

void tst_QCanFrameProcessor::prepareUniqueId_data()
{
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QtCanBus::UniqueId>("uniqueId");
    QTest::addColumn<QVariantMap>("signalValues");
    QTest::addColumn<QCanBusFrame>("expectedFrame");

    const quint32 uniqueId = 0x0965;

    QCanSignalDescription signalDesc;
    signalDesc.setName("s0");
    signalDesc.setDataSource(QtCanBus::DataSource::Payload);
    signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
    signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    signalDesc.setStartBit(0);
    signalDesc.setBitLength(8);

    QCanMessageDescription messageDesc;
    messageDesc.setName("test");
    messageDesc.setUniqueId(QtCanBus::UniqueId{uniqueId});
    messageDesc.setSize(1);
    messageDesc.addSignalDescription(signalDesc);

    const QVariantMap signalValues { qMakePair("s0", 1) };

    // Encode unique id into frame id.
    QCanUniqueIdDescription uidDesc;
    uidDesc.setSource(QtCanBus::DataSource::FrameId);
    uidDesc.setBitLength(12);
    for (quint8 startBit = 0; startBit < 8; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
        const QCanBusFrame::FrameId frameId = qFromLittleEndian(uniqueId) << startBit;
        QTest::addRow("frameId, LE, start %d", startBit)
                << uidDesc << messageDesc << QtCanBus::UniqueId{uniqueId}
                << signalValues << QCanBusFrame(frameId, QByteArray(1, 0x01));
    }

    static constexpr quint16 startBitsFrame[] = { 3, 2, 1, 0, 15, 14, 13, 12 };
    static constexpr quint16 startBitsFrameLen =
            sizeof(startBitsFrame)/sizeof(startBitsFrame[0]);

    for (quint16 idx = 0; idx < startBitsFrameLen; ++idx) {
        const auto val = (qToLittleEndian(uniqueId) << 16) >> idx;
        const QCanBusFrame::FrameId frameId = qToBigEndian(val);
        const auto startBit = startBitsFrame[idx];
        uidDesc.setEndian(QSysInfo::Endian::BigEndian);
        uidDesc.setStartBit(startBit);
        QTest::addRow("frameId, BE, offser %d", startBit)
                << uidDesc << messageDesc << QtCanBus::UniqueId{uniqueId}
                << signalValues << QCanBusFrame(frameId, QByteArray(1, 0x01));
    }

    // Encode unique id into payload. The 1st byte of the payload is already
    // populated with a signal value, so we start from the 2nd byte
    uidDesc.setSource(QtCanBus::DataSource::Payload);
    messageDesc.setSize(sizeof(quint32));

    for (quint8 startBit = 8; startBit < 16; ++startBit) {
        uidDesc.setStartBit(startBit);
        uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
        const quint32 payload = 0x01 | qFromLittleEndian(uniqueId) << startBit;
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        QTest::addRow("payload, LE, offser %d", startBit)
                << uidDesc << messageDesc << QtCanBus::UniqueId{uniqueId}
                << signalValues << QCanBusFrame(0, payloadData);
    }

    static constexpr quint16 startBitsPayload[] = { 11, 10, 9, 8, 23, 22, 21, 20 };
    static constexpr qsizetype startBitsPayloadLen =
            sizeof(startBitsPayload)/sizeof(startBitsPayload[0]);

    for (quint16 idx = 0; idx < startBitsPayloadLen; ++idx) {
        const auto val = (qToLittleEndian(uniqueId) << 8) >> idx;
        const auto startBit = startBitsPayload[idx];
        quint32 payload = 0x01 | qToBigEndian(val);
        const QByteArray payloadData(reinterpret_cast<const char *>(&payload), sizeof(payload));
        uidDesc.setEndian(QSysInfo::Endian::BigEndian);
        uidDesc.setStartBit(startBit);
        QTest::addRow("payload, BE, start %d", startBit)
                << uidDesc << messageDesc << QtCanBus::UniqueId{uniqueId}
                << signalValues << QCanBusFrame(0, payloadData);
    }
}

void tst_QCanFrameProcessor::prepareUniqueId()
{
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QtCanBus::UniqueId, uniqueId);
    QFETCH(QVariantMap, signalValues);
    QFETCH(QCanBusFrame, expectedFrame);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDescription });

    const auto result = processor.prepareFrame(uniqueId, signalValues);
    QCOMPARE(processor.error(), QCanFrameProcessor::Error::None);
    QCOMPARE(result.frameId(), expectedFrame.frameId());
    QCOMPARE(result.payload(), expectedFrame.payload());
}

void tst_QCanFrameProcessor::roundtrip_data()
{
    QTest::addColumn<QCanMessageDescription>("messageDescription");
    QTest::addColumn<QCanUniqueIdDescription>("uniqueIdDescription");
    QTest::addColumn<QCanBusFrame>("inputFrame");
    QTest::addColumn<QtCanBus::UniqueId>("expectedUniqueId");
    QTest::addColumn<QVariantMap>("expectedSignalValues");

    {
        QCanUniqueIdDescription uidDesc;
        uidDesc.setSource(QtCanBus::DataSource::FrameId);
        uidDesc.setEndian(QSysInfo::Endian::LittleEndian);
        uidDesc.setStartBit(0);
        uidDesc.setBitLength(11);

        const quint32 uniqueId = 1467;

        QCanMessageDescription messageDesc;
        messageDesc.setName("test");
        messageDesc.setUniqueId(QtCanBus::UniqueId{uniqueId});
        messageDesc.setSize(8);

        // s0 - 4 bits [0:3], signed int
        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(4);
        messageDesc.addSignalDescription(signalDesc);

        // s1 - 12 bits [4:15], unsigned int
        signalDesc.setName("s1");
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setStartBit(4);
        signalDesc.setBitLength(12);
        messageDesc.addSignalDescription(signalDesc);

        // s2 - 32 bits [16:47], float
        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setStartBit(16);
        signalDesc.setBitLength(32);
        messageDesc.addSignalDescription(signalDesc);

        // gap - 6 bits [48:53]

        // s3 - 10 bits [54:63], unsinged int
        signalDesc.setName("s3");
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setStartBit(54);
        signalDesc.setBitLength(10);
        messageDesc.addSignalDescription(signalDesc);

        const qint8 s0Val = -3;
        const quint16 s1Val = 2890;
        const float s2Val = 123.456f;
        const quint16 s3Val = 963;
        QVariantMap signalValues = { qMakePair("s0", s0Val), qMakePair("s1", s1Val),
                                     qMakePair("s2", s2Val), qMakePair("s3", s3Val) };

        quint64 payloadValue = (static_cast<quint64>(s3Val) << 54) +
                (static_cast<quint64>(s1Val & 0x0FFF) << 4) + static_cast<quint64>(s0Val & 0x0F);
        QByteArray payload(reinterpret_cast<const char *>(&payloadValue), sizeof(quint64));
        memcpy(&payload[2], &s2Val, sizeof(float));

        QCanBusFrame inputFrame(uniqueId, payload);

        QTest::addRow("LE") << messageDesc << uidDesc << inputFrame
                            << QtCanBus::UniqueId{uniqueId} << signalValues;
    }
    {
        QCanUniqueIdDescription uidDesc;
        uidDesc.setSource(QtCanBus::DataSource::FrameId);
        uidDesc.setEndian(QSysInfo::Endian::BigEndian);
        uidDesc.setStartBit(7);
        uidDesc.setBitLength(11);

        const quint32 uniqueId = 1467; // 0x60B7 for the specified layout

        QCanMessageDescription messageDesc;
        messageDesc.setName("test");
        messageDesc.setUniqueId(QtCanBus::UniqueId{uniqueId});
        messageDesc.setSize(8);

        // s0 - 4 bits [7:4], signed int
        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setDataEndian(QSysInfo::Endian::BigEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setStartBit(7);
        signalDesc.setBitLength(4);
        messageDesc.addSignalDescription(signalDesc);

        // s1 - 12 bits [3:0]+[15:8], unsigned int
        signalDesc.setName("s1");
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setStartBit(3);
        signalDesc.setBitLength(12);
        messageDesc.addSignalDescription(signalDesc);

        // s2 - 32 bits [23:16]+[31:24]+[39:32]+[47:10], float
        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setStartBit(23);
        signalDesc.setBitLength(32);
        messageDesc.addSignalDescription(signalDesc);

        // gap - 6 bits [55:50]

        // s3 - 10 bits [49:48]+[63:56], unsinged int
        signalDesc.setName("s3");
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setStartBit(49);
        signalDesc.setBitLength(10);
        messageDesc.addSignalDescription(signalDesc);

        const qint8 s0Val = -3; // 0xD
        const quint16 s1Val = 2890; // 0xB4A
        const float s2Val = 123.456f;
        const quint16 s3Val = 963; // 0x3C3
        QVariantMap signalValues = { qMakePair("s0", s0Val), qMakePair("s1", s1Val),
                                     qMakePair("s2", s2Val), qMakePair("s3", s3Val) };

        quint64 payloadValue = 0xC303000000004ADBULL;
        QByteArray payload(reinterpret_cast<const char *>(&payloadValue), sizeof(quint64));
        const float s2BeVal = qToBigEndian(s2Val);
        memcpy(&payload[2], &s2BeVal, sizeof(float));

        QCanBusFrame inputFrame(0x60B7, payload);

        QTest::addRow("BE") << messageDesc << uidDesc << inputFrame
                            << QtCanBus::UniqueId{uniqueId} << signalValues;
    }
}

void tst_QCanFrameProcessor::roundtrip()
{
    QFETCH(QCanMessageDescription, messageDescription);
    QFETCH(QCanUniqueIdDescription, uniqueIdDescription);
    QFETCH(QCanBusFrame, inputFrame);
    QFETCH(QtCanBus::UniqueId, expectedUniqueId);
    QFETCH(QVariantMap, expectedSignalValues);

    QCanFrameProcessor processor;
    processor.setUniqueIdDescription(uniqueIdDescription);
    processor.setMessageDescriptions({ messageDescription });

    const auto result = processor.parseFrame(inputFrame);
    QCOMPARE(result.uniqueId, expectedUniqueId);
    QCOMPARE(result.signalValues, expectedSignalValues);

    const auto encodedFrame = processor.prepareFrame(result.uniqueId, result.signalValues);
    QCOMPARE(encodedFrame.frameId(), inputFrame.frameId());
    QCOMPARE(encodedFrame.payload(), inputFrame.payload());
}

QTEST_MAIN(tst_QCanFrameProcessor)

#include "tst_qcanframeprocessor.moc"
