// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "repeatedmessages.qpb.h"

#include <QTest>
#include <QProtobufSerializer>

using namespace Qt::Literals::StringLiterals;

class QtProtobufRepeatedTypesSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        m_serializer.reset(new QProtobufSerializer);
    }
    void repeatedIntMessageTest();
    void repeatedSIntMessageTest();
    void repeatedUIntMessageTest();
    void repeatedInt64MessageTest();
    void repeatedSInt64MessageTest();
    void repeatedUInt64MessageTest();
    void repeatedFixedIntMessageTest();
    void repeatedSFixedIntMessageTest();
    void repeatedFixedInt64MessageTest();
    void repeatedSFixedInt64MessageTest();
    void repeatedStringMessageTest();
    void repeatedFloatMessageTest();
    void repeatedDoubleMessageTest();
    void repeatedBytesMessageTest();
    void repeatedComplexMessageTest();
    void repeatedBoolMessageTest();
private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

void QtProtobufRepeatedTypesSerializationTest::repeatedIntMessageTest()
{
    RepeatedIntMessage test;
    test.setTestRepeatedInt({0, 1, 321, -65999, 123245, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a1c0001c102b1fcfbffffffffffff01edc207fdffffffffffffffff0103"_L1);
    test.setTestRepeatedInt(QtProtobuf::int32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedSIntMessageTest()
{
    RepeatedSIntMessage test;
    test.setTestRepeatedInt({1, 321, -65999, 123245, -3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a0c0282059d8708da850f050600"_L1);

    test.setTestRepeatedInt(QtProtobuf::sint32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedUIntMessageTest()
{
    RepeatedUIntMessage test;
    test.setTestRepeatedInt({1, 0, 321, 65999, 123245, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a0b0100c102cf8304edc20703"_L1);

    test.setTestRepeatedInt(QtProtobuf::uint32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedInt64MessageTest()
{
    RepeatedInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 12324523123123, -3, 0, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a2001c102b1fcfbffffffffffff01b3c3cab6d8e602fdffffffffffffffff010003"_L1);

    test.setTestRepeatedInt(QtProtobuf::int64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedSInt64MessageTest()
{
    RepeatedSInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 12324523123123, 0, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a100282059d8708e68695edb0cd05000506"_L1);

    test.setTestRepeatedInt(QtProtobuf::sint64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedUInt64MessageTest()
{
    RepeatedUInt64Message test;
    test.setTestRepeatedInt({1, 321, 0, 65999, 123245, 123245324235425234, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a1401c10200cf8304edc207d28b9fda82dff6da0103"_L1);

    test.setTestRepeatedInt(QtProtobuf::uint64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedFixedIntMessageTest()
{
    RepeatedFixedIntMessage test;
    test.setTestRepeatedInt({1, 321, 65999, 12324523, 3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a1c0100000041010000cf010100ab0ebc00030000000300000000000000"_L1);
    test.setTestRepeatedInt(QtProtobuf::fixed32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedSFixedIntMessageTest()
{
    RepeatedSFixedIntMessage test;
    test.setTestRepeatedInt({0, 1, 321, -65999, 12324523, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a1c00000000010000004101000031fefeffab0ebc00fdffffff03000000"_L1);

    test.setTestRepeatedInt(QtProtobuf::sfixed32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedFixedInt64MessageTest()
{
    RepeatedFixedInt64Message test;
    test.setTestRepeatedInt({1, 321, 65999, 123245324235425234, 3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a3801000000000000004101000000000000cf01010000000000d2c5472bf8dab"
             "501030000000000000003000000000000000000000000000000"_L1);

    test.setTestRepeatedInt(QtProtobuf::fixed64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedSFixedInt64MessageTest()
{
    RepeatedSFixedInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 123245324235425234, -3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a380100000000000000410100000000000031fefeffffffffffd2c5472bf8"
             "dab501fdffffffffffffff03000000000000000000000000000000"_L1);

    test.setTestRepeatedInt(QtProtobuf::sfixed64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedStringMessageTest()
{
    RepeatedStringMessage test;
    test.setTestRepeatedString({"aaaa","bbbbb","ccc","dddddd","eeeee", ""});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a04616161610a0562626262620a036363630a066464646464640a0565656565650a00"_L1);

    test.setTestRepeatedString(QStringList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedFloatMessageTest()
{
    RepeatedFloatMessage test;
    test.setTestRepeatedFloat({0.4f, 1.2f, 0.5f, 1.4f, 0.6f});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a14cdcccc3e9a99993f0000003f3333b33f9a99193f"_L1);

    test.setTestRepeatedFloat(QtProtobuf::floatList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedDoubleMessageTest()
{
    RepeatedDoubleMessage test;
    test.setTestRepeatedDouble({0.1, 0.2, 0.3, 0.4, 0.5});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a289a9999999999b93f9a9999999999c93f333333333333d33f9a99999"
             "99999d93f000000000000e03f"_L1);

    test.setTestRepeatedDouble(QtProtobuf::doubleList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedBytesMessageTest()
{
    RepeatedBytesMessage test;
    test.setTestRepeatedBytes({QByteArray::fromHex("010203040506"),
                               QByteArray::fromHex("ffffffff"),
                               QByteArray::fromHex("eaeaeaeaea"),
                               QByteArray::fromHex("010203040506")});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a060102030405060a04ffffffff0a05eaeaeaeaea0a06010203040506"_L1);

    test.setTestRepeatedBytes(QByteArrayList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());

    test.setTestRepeatedBytes({QByteArray::fromHex("010203040506"),
                               QByteArray::fromHex(""),
                               QByteArray::fromHex("eaeaeaeaea"),
                               QByteArray::fromHex("010203040506")});
    result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()),
             "0a060102030405060a000a05eaeaeaeaea0a06010203040506"_L1);
}

void QtProtobufRepeatedTypesSerializationTest::repeatedComplexMessageTest()
{
    SimpleStringMessage stringMsg;
    stringMsg.setTestFieldString("qwerty");
    ComplexMessage msg;
    msg.setTestFieldInt(25);
    msg.setTestComplexField(stringMsg);
    RepeatedComplexMessage test;
    test.setTestRepeatedComplex({msg, msg, msg});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a0c0819120832067177657274790a0c081912083"
                                                "2067177657274790a0c081912083206717765727479"_L1);

    test.setTestRepeatedComplex({});
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufRepeatedTypesSerializationTest::repeatedBoolMessageTest()
{
    RepeatedBoolMessage boolMsg;
    boolMsg.setTestRepeatedBool({ true, true, true, false, false, false, false, false,
                                  false, false, false, false, true });
    QByteArray result = boolMsg.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a0d01010100000000000000000001"_L1);
}

QTEST_MAIN(QtProtobufRepeatedTypesSerializationTest)
#include "tst_protobuf_serialization_repeatedtypes.moc"
