// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtSerialBus/QCanDbcFileParser>
#include <QtSerialBus/QCanMessageDescription>
#include <QtSerialBus/QCanSignalDescription>
#include <QtSerialBus/QCanUniqueIdDescription>

#include "qcanmessagedescription_helpers.h"
#include "qcanuniqueiddescription_helpers.h"

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

class tst_QCanDbcFileParser : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase_data();
    void initTestCase();
    void construct();
    void uinqueId();
    void parseFile_data();
    void parseFile();
    void valueDescriptions();
    void resetState();

private:
    QString m_filesDir;
};

void tst_QCanDbcFileParser::initTestCase_data()
{
    QTest::addColumn<bool>("readFromFile");
    QTest::newRow("from_file") << true;
    QTest::newRow("from_input_data") << false;
}

void tst_QCanDbcFileParser::initTestCase()
{
    m_filesDir = QString::fromLatin1(QT_TEST_DATADIR) + QDir::separator();
}

void tst_QCanDbcFileParser::construct()
{
    QCanDbcFileParser parser;
    QCOMPARE(parser.error(), QCanDbcFileParser::Error::None);
    QVERIFY(parser.errorString().isEmpty());
    QVERIFY(parser.warnings().isEmpty());
    QVERIFY(parser.messageDescriptions().isEmpty());
    QVERIFY(parser.messageValueDescriptions().isEmpty());
}

void tst_QCanDbcFileParser::uinqueId()
{
    QCanUniqueIdDescription expectedDesc;
    expectedDesc.setSource(QtCanBus::DataSource::FrameId);
    expectedDesc.setEndian(QSysInfo::Endian::LittleEndian);
    expectedDesc.setStartBit(0);
    expectedDesc.setBitLength(29);

    QVERIFY(equals(QCanDbcFileParser::uniqueIdDescription(), expectedDesc));
}

void tst_QCanDbcFileParser::parseFile_data()
{
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QCanDbcFileParser::Error>("expectedError");
    QTest::addColumn<QString>("expectedErrorDescription");
    QTest::addColumn<QStringList>("expectedWarnings");
    QTest::addColumn<QList<QCanMessageDescription>>("expectedMessageDescriptions");

    {
        // We need to get the exact same error string
        const QString fileName = u"nonexisting_file.dbc"_s;
        QFile f(m_filesDir + fileName);
        auto res = f.open(QIODevice::ReadOnly);
        QVERIFY(!res);
        const QString errorDesc = f.errorString();

        QTest::addRow("invalid filename")
                << QStringList{ u"nonexisting_file.dbc"_s }
                << QCanDbcFileParser::Error::FileReading << errorDesc
                << QStringList() << QList<QCanMessageDescription>();
    }

    QStringList expectedWarnings {
        u"Failed to find message description in string BO_ MessageName : 8 Vector__XXX"_s,
        u"Failed to find message description in string BO_ 1234  : 8 Vector__XXX"_s,
        u"Failed to find message description in string BO_ 1234 MessageName : Vector__XXX"_s,
        u"Failed to find message description in string BO_ 1234 MessageName : 8"_s
    };

    QTest::addRow("invalid message definitions")
            << QStringList{ u"invalid_messages.dbc"_s }
            << QCanDbcFileParser::Error::None << QString()
            << expectedWarnings
            << QList<QCanMessageDescription>();

    expectedWarnings = {
        u"Failed to find signal description in string SG_ : 0|8@1+ (1,0) [0|0] \"unit\" "
         "Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName 0|8@1+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : |8@1+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 08@1+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|@1+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|81+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@+ (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1 (1,0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ (0) [0|0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ (1,0)  "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ (1,0) [0] "
         "\"unit\" Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ (1,0) [0|0]  "
         "Vector__XXX"_s,
        u"Failed to find signal description in string SG_ SignalName : 0|8@1+ (1,0) [0|0] "
         "\"unit\""_s,
        u"Message description with unique id 1234 is skipped because it's not valid."_s
    };

    QTest::addRow("invalid signal definitions")
            << QStringList{ u"invalid_signals.dbc"_s }
            << QCanDbcFileParser::Error::None << QString()
            << expectedWarnings
            << QList<QCanMessageDescription>();

    QCanMessageDescription messageDesc;
    messageDesc.setName("Test");
    messageDesc.setSize(3);
    messageDesc.setUniqueId(QtCanBus::UniqueId{1234});
    messageDesc.setTransmitter("Vector__XXX");

    {
        messageDesc.clearSignalDescriptions();

        QCanSignalDescription signalDesc;
        signalDesc.setName("Signal0"); // correct
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("_sIgnal_1"); // correct
        signalDesc.setStartBit(8);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s_Ig_nAl2_"); // correct
        signalDesc.setStartBit(16);
        messageDesc.addSignalDescription(signalDesc);

        // other signal names are not correct DBC identifiers, so they will
        // result in warnings

        expectedWarnings = {
            u"Failed to find signal description in string SG_ Singal%1 : 0|8@1+ (1,0) [0|0] \"unit\" Vector__XXX"_s,
            u"Failed to find signal description in string SG_ 1Signal : 0|8@1+ (1,0) [0|0] \"unit\" Vector__XXX"_s,
            u"Failed to find signal description in string SG_ Signal name : 0|8@1+ (1,0) [0|0] \"unit\" Vector__XXX"_s,
            u"Failed to find signal description in string SG_ \"Signal name\" : 0|8@1+ (1,0) [0|0] \"unit\" Vector__XXX"_s,
        };

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("different signal names")
                << QStringList{ u"signal_names.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << expectedWarnings << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(12);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(12);
        signalDesc.setPhysicalUnit("unit1");
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("valid message and signals")
                << QStringList{ u"valid_message_and_signals.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(12);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setDataEndian(QSysInfo::Endian::BigEndian);
        signalDesc.setBitLength(8);
        signalDesc.setStartBit(23);
        signalDesc.setPhysicalUnit("");
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("different endian")
                << QStringList{ u"different_endian.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(7);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(12);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setStartBit(12);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setBitLength(32);
        signalDesc.setStartBit(24);
        messageDesc.addSignalDescription(signalDesc);

        QCanMessageDescription doubleMessage = messageDesc;
        doubleMessage.clearSignalDescriptions();
        doubleMessage.setName("Test1");
        doubleMessage.setUniqueId(QtCanBus::UniqueId{0x18fef1fe});
        doubleMessage.setSize(8);

        signalDesc.setName("s3");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Double);
        signalDesc.setBitLength(64);
        signalDesc.setStartBit(0);
        doubleMessage.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc, doubleMessage };

        QTest::addRow("different data types")
                << QStringList{ u"different_data_types.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(8);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(0.0); // not used - shouldn't really happen in DBC file
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(8);
        signalDesc.setFactor(1.0); // int values
        signalDesc.setOffset(-5.0);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setStartBit(16);
        signalDesc.setFactor(0.625); // float values
        signalDesc.setOffset(-5.567);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s3");
        signalDesc.setStartBit(24);
        signalDesc.setFactor(1.234e-05); // scientific representation
        signalDesc.setOffset(-1.056e02);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s4");
        signalDesc.setStartBit(32);
        signalDesc.setFactor(0.625); // float and int values
        signalDesc.setOffset(-5.0);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s5");
        signalDesc.setStartBit(40);
        signalDesc.setFactor(2.5); // float and scientific values
        signalDesc.setOffset(1.000123e01);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s6");
        signalDesc.setStartBit(48);
        signalDesc.setFactor(2e-003);
        signalDesc.setOffset(1e02);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s7");
        signalDesc.setStartBit(56);
        signalDesc.setFactor(1.e4);
        signalDesc.setOffset(2.E-02);
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("different factor and offset")
                << QStringList{ u"different_factor_offset.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(7);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(8);
        signalDesc.setRange(1, 5); // int values
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setStartBit(16);
        signalDesc.setRange(-0.625, 0.125); // float values
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s3");
        signalDesc.setStartBit(24);
        signalDesc.setRange(-1.234e-05, 1.234e05); // scientific
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s4");
        signalDesc.setStartBit(32);
        signalDesc.setRange(-1, 0.333); // int and float
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s5");
        signalDesc.setStartBit(40);
        signalDesc.setRange(2.5, 1.000123e01); // float and scientific values
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s6");
        signalDesc.setStartBit(48);
        signalDesc.setRange(0.333, -0.333); // invalid order
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("different ranges")
                << QStringList{ u"different_ranges.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(4);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Rcv_1");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(8);
        signalDesc.setReceiver("Rcv2");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setStartBit(16);
        signalDesc.setReceiver("RCV_3");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s3");
        signalDesc.setStartBit(24);
        signalDesc.setReceiver("rcv4");
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("multiple receivers")
                << QStringList{ u"multiple_receivers.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(7);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(12);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setStartBit(12);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setBitLength(32);
        signalDesc.setStartBit(24);
        messageDesc.addSignalDescription(signalDesc);

        QCanMessageDescription doubleMessage = messageDesc;
        doubleMessage.clearSignalDescriptions();
        doubleMessage.setName("Test1");
        doubleMessage.setUniqueId(QtCanBus::UniqueId{0x18fef1fe});
        doubleMessage.setSize(8);

        signalDesc.setName("s3");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Double);
        signalDesc.setBitLength(64);
        signalDesc.setStartBit(0);
        doubleMessage.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc, doubleMessage };

        QTest::addRow("message and signals in one line")
                << QStringList{ u"message_signals_in_one_line.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(5);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(8);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setStartBit(16);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s3");
        signalDesc.setStartBit(24);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s4");
        signalDesc.setStartBit(32);
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc };

        QTest::addRow("random whitespaces")
                << QStringList{ u"random_whitespaces.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(2);
        messageDesc.setComment("Comment for message Test");

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        signalDesc.setComment("Comment for s0");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setStartBit(8);
        signalDesc.setComment("comment for s1");
        messageDesc.addSignalDescription(signalDesc);

        QCanMessageDescription otherDesc = messageDesc;
        otherDesc.clearSignalDescriptions();
        otherDesc.setName("Test1");
        otherDesc.setUniqueId(QtCanBus::UniqueId{0x18fef1fe});
        otherDesc.setSize(1);
        otherDesc.setComment("comment for Test1.");

        signalDesc.setName("s2");
        signalDesc.setDataEndian(QSysInfo::Endian::BigEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setStartBit(7);
        signalDesc.setBitLength(8);
        signalDesc.setComment("comment for s2");
        otherDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc, otherDesc };

        QTest::addRow("comments processing")
                << QStringList{ u"messages_with_comments.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << QStringList() << descriptions;

        messageDesc.setComment("");
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(5);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(2);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s1");
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::None);
        signalDesc.setStartBit(2);
        signalDesc.setBitLength(6);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(32);
        signalDesc.addMultiplexSignal("s0", 1u);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.setName("s3");
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(32);
        signalDesc.addMultiplexSignal("s0", 2u);
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc};

        // The file also contains a message with simple multiplexing, but two
        // multiplexor signals. That is incorrect, so a warning is raised, and
        // this message description is discarded
        expectedWarnings = {
            u"Message description with unique id 1235 is skipped because it has invalid "
             "multiplexing description."_s
        };

        QTest::addRow("simple multiplexing")
                << QStringList{ u"simple_multiplexing.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << expectedWarnings << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(3);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(4);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s1");
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
        signalDesc.setStartBit(4);
        signalDesc.setBitLength(4);
        signalDesc.addMultiplexSignal("s0", { {1U, 1U}, {5U, 5U}, {9U, 9U} });
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
        signalDesc.setStartBit(4);
        signalDesc.setBitLength(12);
        signalDesc.addMultiplexSignal("s0", { {2U, 4U}, {6U, 8U}, {10U, 10U} });
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s3");
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(8);
        signalDesc.addMultiplexSignal("s1", 1U);
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s4");
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(8);
        signalDesc.addMultiplexSignal("s1", { {2U, 3U}, {5U, 5U} });
        messageDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s5");
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::None);
        signalDesc.setStartBit(16);
        signalDesc.setBitLength(8);
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc};

        // The file also contains messages with broken extended multiplexing,
        // so a warning is raised for every such message, and these message
        // descriptions are discarded
        expectedWarnings = {
            u"Failed to find extended multiplexing description in string "
             "SG_MUL_VAL_  1238 s2 s0 2-4, 6-8 10-10;"_s,
            u"Failed to find extended multiplexing description in string "
             "SG_MUL_VAL_  1238 s4 s1 2-3, 5-5"_s,
            u"Failed to find message description for unique id 12371. Skipping string "
             "SG_MUL_VAL_  12371 s4 s1 2-3, 5-5;"_s,
            u"Failed to find signal description for signal s11. Skipping string "
             "SG_MUL_VAL_  2566844926 s4 s11 2-3, 5-5;"_s,
            u"Failed to find signal description for signal s6. Skipping string "
             "SG_MUL_VAL_  1236 s6 s1 2-3, 5-5;"_s,
            u"Message description with unique id 419361278 is skipped because it has invalid "
             "multiplexing description."_s,
            u"Message description with unique id 1236 is skipped because it has invalid "
             "multiplexing description."_s,
            u"Message description with unique id 1237 is skipped because it has invalid "
             "multiplexing description."_s,
            u"Message description with unique id 1238 is skipped because it has invalid "
             "multiplexing description."_s,
        };

        QTest::addRow("extended multiplexing")
                << QStringList{ u"extended_multiplexing.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << expectedWarnings << descriptions;
    }

    {
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(8);

        QCanSignalDescription signalDesc;
        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(8);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit(" !#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "[]^_`abcdefghijklmnopqrstuvwxyz{|}~°");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        QList<QCanMessageDescription> descriptions { messageDesc};

        expectedWarnings = {
            u"Failed to find signal description in string SG_ s1 : 0|8@1+ (1,0) [0|0] "
             "\"double quote: \"\" Vector__XXX"_s,
            u"Failed to find signal description in string SG_ s2 : 0|8@1+ (1,0) [0|0] "
             "\"backslash: \\\" Vector__XXX"_s,
        };

        QTest::addRow("char string test")
                << QStringList{ u"char_string_test.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << expectedWarnings << descriptions;
    }

    {
        const QString fileName = u"invalid_message_pos.dbc"_s;
        const QString expectedError = u"Failed to parse file %1. Unexpected position "
                                       "of BO_  section."_s.arg(m_filesDir + fileName);

        QTest::addRow("invalid message desc pos")
                << QStringList{ fileName }
                << QCanDbcFileParser::Error::Parsing << expectedError
                << QStringList() << QList<QCanMessageDescription>();
    }

    {
        const QString fileName = u"invalid_signal_pos.dbc"_s;
        const QString expectedError = u"Failed to parse file %1. Unexpected position "
                                       "of SG_  section."_s.arg(m_filesDir + fileName);

        QTest::addRow("invalid signal desc pos")
                << QStringList{ fileName }
                << QCanDbcFileParser::Error::Parsing << expectedError
                << QStringList() << QList<QCanMessageDescription>();
    }

    {
        // Check that we get message descriptions and warnings from all files
        messageDesc.clearSignalDescriptions();
        messageDesc.setSize(4);

        QCanSignalDescription signalDesc;
        signalDesc.setName("Signal0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::Float);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(32);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("unit");
        signalDesc.setReceiver("Vector__XXX");
        messageDesc.addSignalDescription(signalDesc);

        QCanMessageDescription otherDesc;
        otherDesc.setName("Test1");
        otherDesc.setSize(3);
        otherDesc.setUniqueId(QtCanBus::UniqueId{1235});
        otherDesc.setTransmitter("Vector__XXX");

        signalDesc.setName("s0");
        signalDesc.setDataEndian(QSysInfo::Endian::LittleEndian);
        signalDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
        signalDesc.setDataSource(QtCanBus::DataSource::Payload);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
        signalDesc.setStartBit(0);
        signalDesc.setBitLength(4);
        signalDesc.setFactor(1.0);
        signalDesc.setOffset(0.0);
        signalDesc.setRange(0.0, 0.0);
        signalDesc.setPhysicalUnit("");
        signalDesc.setReceiver("Vector__XXX");
        otherDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s1");
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
        signalDesc.setStartBit(4);
        signalDesc.setBitLength(4);
        signalDesc.addMultiplexSignal("s0", { {1U, 1U}, {5U, 5U}, {9U, 9U} });
        otherDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s2");
        signalDesc.setDataFormat(QtCanBus::DataFormat::SignedInteger);
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
        signalDesc.setStartBit(4);
        signalDesc.setBitLength(12);
        signalDesc.addMultiplexSignal("s0", { {2U, 4U}, {6U, 8U}, {10U, 10U} });
        otherDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s3");
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(8);
        signalDesc.addMultiplexSignal("s1", 1U);
        otherDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s4");
        signalDesc.setStartBit(8);
        signalDesc.setBitLength(8);
        signalDesc.addMultiplexSignal("s1", { {2U, 3U}, {5U, 5U} });
        otherDesc.addSignalDescription(signalDesc);

        signalDesc.clearMultiplexSignals();
        signalDesc.setName("s5");
        signalDesc.setMultiplexState(QtCanBus::MultiplexState::None);
        signalDesc.setStartBit(16);
        signalDesc.setBitLength(8);
        otherDesc.addSignalDescription(signalDesc);

        expectedWarnings = {
            u"Failed to find signal description in string "
             "SG_ Singal%1 : 0|8@1+ (1,0) [0|0] \"unit\" Vector__XXX"_s,
            u"Failed to find signal description for signal s6. Skipping string "
             "SG_MUL_VAL_  1236 s6 s1 2-3, 5-5;"_s,
            u"Message description with unique id 1236 is skipped because it has invalid "
             "multiplexing description."_s,
        };

        QList<QCanMessageDescription> descriptions { messageDesc, otherDesc };

        QTest::addRow("multiple files")
                << QStringList{ u"multiple_files_1.dbc"_s, u"multiple_files_2.dbc"_s }
                << QCanDbcFileParser::Error::None << QString()
                << expectedWarnings << descriptions;
    }

    {
        const QString fileName = u"invalid_message_pos.dbc"_s;
        const QString expectedError = u"Failed to parse file %1. Unexpected position "
                                       "of BO_  section."_s.arg(m_filesDir + fileName);

        QTest::addRow("multiple files and error")
                << QStringList{ u"multiple_files_1.dbc"_s, fileName }
                << QCanDbcFileParser::Error::Parsing << expectedError
                << QStringList() << QList<QCanMessageDescription>();
    }
}

void tst_QCanDbcFileParser::parseFile()
{
    QFETCH(QStringList, fileNames);
    QFETCH(QCanDbcFileParser::Error, expectedError);
    QFETCH(QString, expectedErrorDescription);
    QFETCH(QStringList, expectedWarnings);
    QFETCH(QList<QCanMessageDescription>, expectedMessageDescriptions);
    QFETCH_GLOBAL(bool, readFromFile);

    QVERIFY(!fileNames.isEmpty());

    QCanDbcFileParser parser;
    if (readFromFile) {
        if (fileNames.size() > 1) {
            QStringList files;
            for (const auto &fn : fileNames)
                files.push_back(m_filesDir + fn);
            parser.parse(files);
        } else {
            parser.parse(m_filesDir + fileNames.first());
        }
    } else {
        // read the file outside of the parser
        if (fileNames.size() > 1)
            QSKIP("parseData() does not support multiple files");

        QByteArray allData;
        QFile f(m_filesDir + fileNames.first());
        if (f.open(QIODevice::ReadOnly))
            allData = f.readAll();
        f.close();
        if (allData.isEmpty())
            QSKIP("No valid input file provided");
        const QString inputData = QString::fromUtf8(allData);
        parser.parseData(inputData);
    }

    QCOMPARE_EQ(parser.error(), expectedError);
    if (!readFromFile && expectedErrorDescription.contains("Failed to parse file"))
        expectedErrorDescription.replace(QRegularExpression(u"file .+dbc"_s), u"input data"_s);
    QCOMPARE_EQ(parser.errorString(), expectedErrorDescription);

    if (expectedError != QCanDbcFileParser::Error::None)
        return; // no need to compare other data if we already had an error

    // Some warnings may come in unspecified order, so sort them before
    // comparing
    auto parserWarnings = parser.warnings();
    std::sort(parserWarnings.begin(), parserWarnings.end());
    std::sort(expectedWarnings.begin(), expectedWarnings.end());
    QCOMPARE_EQ(parserWarnings, expectedWarnings);

    // parser.messageDescriptions() generates a list from a QHash, so the order
    // is unspecified and we need to sort them before comparison.
    auto messageDescComparator =
            [](const QCanMessageDescription &lhs, const QCanMessageDescription &rhs) {
        return lhs.name() < rhs.name();
    };
    auto messageDescriptions = parser.messageDescriptions();
    std::sort(messageDescriptions.begin(), messageDescriptions.end(), messageDescComparator);
    std::sort(expectedMessageDescriptions.begin(), expectedMessageDescriptions.end(),
              messageDescComparator);
    QVERIFY(equals(messageDescriptions, expectedMessageDescriptions));
}

static bool parseHelper(QCanDbcFileParser *parser, const QString &name, bool readFromFile)
{
    if (readFromFile)
        return parser->parse(name);

    QString data;
    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) {
        data = QString::fromUtf8(f.readAll());
        f.close();
    }
    return parser->parseData(data);
}

void tst_QCanDbcFileParser::valueDescriptions()
{
    QFETCH_GLOBAL(bool, readFromFile);

    QCanDbcFileParser::ValueDescriptions s0_test_value_descriptions;
    s0_test_value_descriptions.insert(0, u"Description for the value '0x0'"_s);
    s0_test_value_descriptions.insert(1, u"Description for the value '0x1'"_s);
    s0_test_value_descriptions.insert(2, u"Description for the value '0x2'"_s);

    QCanDbcFileParser::ValueDescriptions s1_test_value_descriptions;
    s1_test_value_descriptions.insert(3, u"red"_s);
    s1_test_value_descriptions.insert(4, u"green"_s);
    s1_test_value_descriptions.insert(5, u"blue"_s);

    const QCanDbcFileParser::SignalValueDescriptions test_value_descriptions {
        qMakePair("s0", s0_test_value_descriptions), qMakePair("s1", s1_test_value_descriptions)
    };

    QCanDbcFileParser::ValueDescriptions s1_test1_value_descriptions;
    s1_test1_value_descriptions.insert(3, u"r"_s);
    s1_test1_value_descriptions.insert(4, u"g"_s);
    s1_test1_value_descriptions.insert(5, u"b"_s);

    QCanDbcFileParser::ValueDescriptions s2_test1_value_descriptions;
    s2_test1_value_descriptions.insert(0, u"Value0"_s);
    s2_test1_value_descriptions.insert(1, u"Value1"_s);
    s2_test1_value_descriptions.insert(2, u"Value2"_s);
    s2_test1_value_descriptions.insert(3, u"Value3"_s);
    s2_test1_value_descriptions.insert(4, u"Value4"_s);

    const QCanDbcFileParser::SignalValueDescriptions test1_value_descriptions {
        qMakePair("s1", s1_test1_value_descriptions), qMakePair("s2", s2_test1_value_descriptions)
    };

    QCanDbcFileParser::MessageValueDescriptions expectedDescriptions;
    expectedDescriptions.insert(QtCanBus::UniqueId{1234}, test_value_descriptions);
    expectedDescriptions.insert(QtCanBus::UniqueId{0x18fef1fe}, test1_value_descriptions);

    const QStringList expectedWarnings {
        u"Failed to find message description for unique id 1236. Skipping string "
         "VAL_ 1236 s2 4 \"Value4\" 3 \"Value3\" 2 \"Value2\" 1 \"Value1\" 0 \"Value0\" ;"_s,
        u"Failed to find signal description for signal s3. Skipping string "
         "VAL_ 2566844926 s3 4 \"Value4\" 3 \"Value3\" 2 \"Value2\" 1 \"Value1\" 0 \"Value0\" ;"_s,
        u"Failed to parse value description from string "
         "VAL_ 2566844926 s2 4 \"Value4\" 3 \"Value3\" 2 \"Value2\" 1 \"Value1\" 0 ;"_s
    };

    QCanDbcFileParser parser;
    const QString fileName = u"value_descriptions.dbc"_s;
    QVERIFY(parseHelper(&parser, m_filesDir + fileName, readFromFile));

    QCOMPARE(parser.error(), QCanDbcFileParser::Error::None);
    QCOMPARE(parser.messageValueDescriptions(), expectedDescriptions);
    QCOMPARE(parser.warnings(), expectedWarnings);
}

void tst_QCanDbcFileParser::resetState()
{
    QFETCH_GLOBAL(bool, readFromFile);

    // Test that the state is correctly reset between parsings
    const QString fileName = u"value_descriptions.dbc"_s;
    QCanDbcFileParser parser;
    QVERIFY(parseHelper(&parser, m_filesDir + fileName, readFromFile));

    QCOMPARE(parser.error(), QCanDbcFileParser::Error::None);
    QVERIFY(!parser.messageDescriptions().isEmpty());
    QVERIFY(!parser.warnings().isEmpty());
    QVERIFY(!parser.messageValueDescriptions().isEmpty());

    // Now when we parse an invalid file, we should get an error, and all
    // other getters should return default values
    const QString invalidName = u"invalid_file"_s;
    QVERIFY(!parseHelper(&parser, m_filesDir + invalidName, readFromFile));

    const auto expectedError = readFromFile ? QCanDbcFileParser::Error::FileReading
                                            : QCanDbcFileParser::Error::Parsing;
    QCOMPARE(parser.error(), expectedError);
    QVERIFY(parser.messageDescriptions().isEmpty());
    QVERIFY(parser.warnings().isEmpty());
    QVERIFY(parser.messageValueDescriptions().isEmpty());
}

QTEST_MAIN(tst_QCanDbcFileParser)

#include "tst_qcandbcfileparser.moc"
