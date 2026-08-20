// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "repeatedmessages.qpb.h"

#include <QObject>

#include <QTest>
#include <QProtobufSerializer>

#include <limits>

class QtProtobufRepeatedTypesDeserializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        serializer.reset(new QProtobufSerializer);
    }

    void repeatedStringMessageTest();
    void repeatedBytesMessageTest();
    void repeatedFloatMessageTest();
    void repeatedDoubleMessageTest();
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
    void repeatedComplexMessageTest();
    void repeatedBoolMessageTest();
private:
    std::unique_ptr<QProtobufSerializer> serializer;
};

using namespace qtprotobufnamespace::tests;

void QtProtobufRepeatedTypesDeserializationTest::repeatedStringMessageTest()
{
    RepeatedStringMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a04616161610a0562626262620a036363630a066464646464640a056565656565"));
    QCOMPARE(test.testRepeatedString().count(), 5);
    QCOMPARE(test.testRepeatedString(), QStringList({"aaaa","bbbbb","ccc","dddddd","eeeee"}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedBytesMessageTest()
{
    RepeatedBytesMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a060102030405060a04ffffffff0a05eaeaeaeaea0a06010203040506"));
    QCOMPARE(test.testRepeatedBytes().count(), 4);
    QCOMPARE(test.testRepeatedBytes(), QByteArrayList({QByteArray::fromHex("010203040506"),
                                                             QByteArray::fromHex("ffffffff"),
                                                             QByteArray::fromHex("eaeaeaeaea"),
                                                             QByteArray::fromHex("010203040506")}));
    //TODO: Serialization for list types works partially incorrect because of appending of values to existing
    //Need to make decision if deserialize should reset all protobuf properties or not
    RepeatedBytesMessage test2;
    test2.deserialize(serializer.get(), QByteArray::fromHex("0a060102030405060a000a05eaeaeaeaea0a06010203040506"));
    QCOMPARE(test2.testRepeatedBytes().count(), 4);
    QCOMPARE(test2.testRepeatedBytes(), QByteArrayList({QByteArray::fromHex("010203040506"),
                                                             QByteArray::fromHex(""),
                                                             QByteArray::fromHex("eaeaeaeaea"),
                                                             QByteArray::fromHex("010203040506")}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedFloatMessageTest()
{
    RepeatedFloatMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a14cdcccc3e9a99993f0000003f3333b33f9a99193f"));
    QCOMPARE(test.testRepeatedFloat().count(), 5);
    QCOMPARE(test.testRepeatedFloat(), QtProtobuf::floatList({0.4f, 1.2f, 0.5f, 1.4f, 0.6f}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedDoubleMessageTest()
{
    RepeatedDoubleMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a289a9999999999b93f9a9999999999c93f333333333333d33f9a9999999999d93f000000000000e03f"));
    QCOMPARE(test.testRepeatedDouble().count(), 5);
    QCOMPARE(test.testRepeatedDouble(), QtProtobuf::doubleList({0.1, 0.2, 0.3, 0.4, 0.5}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedIntMessageTest()
{
    RepeatedIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a1b01c102b1fcfbffffffffffff01edc207fdffffffffffffffff0103"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::int32List({1, 321, -65999, 123245, -3, 3}));

    RepeatedIntMessage test2;
    test2.deserialize(serializer.get(), QByteArray::fromHex("0a1b01c102b1fcfbffffffffffff01edc207fdffffffffffffffff0103"));
    QCOMPARE(test2.testRepeatedInt().count(), 6);
    QCOMPARE(test2.testRepeatedInt(), QtProtobuf::int32List({1, 321, -65999, 123245, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedSIntMessageTest()
{
    RepeatedSIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a0b0282059d8708da850f0506"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::sint32List({1, 321, -65999, 123245, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedUIntMessageTest()
{
    RepeatedUIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a0a01c102cf8304edc20703"));
    QCOMPARE(test.testRepeatedInt().count(), 5);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::uint32List({1, 321, 65999, 123245, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedInt64MessageTest()
{
    RepeatedInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a1f01c102b1fcfbffffffffffff01b3c3cab6d8e602fdffffffffffffffff0103"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::int64List({1, 321, -65999, 12324523123123, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedSInt64MessageTest()
{
    RepeatedSInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a0f0282059d8708e68695edb0cd050506"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::sint64List({1, 321, -65999, 12324523123123, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedUInt64MessageTest()
{
    RepeatedUInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a1301c102cf8304edc207d28b9fda82dff6da0103"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::uint64List({1, 321, 65999, 123245, 123245324235425234, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedFixedIntMessageTest()
{
    RepeatedFixedIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a180100000041010000cf010100ab0ebc000300000003000000"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::fixed32List({1, 321, 65999, 12324523, 3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedSFixedIntMessageTest()
{
    RepeatedSFixedIntMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a18010000004101000031fefeffab0ebc00fdffffff03000000"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::sfixed32List({1, 321, -65999, 12324523, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedFixedInt64MessageTest()
{
    RepeatedFixedInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a3001000000000000004101000000000000cf01010000000000d2c5472bf8dab50103000000000000000300000000000000"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::fixed64List({1, 321, 65999, 123245324235425234, 3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedSFixedInt64MessageTest()
{
    RepeatedSFixedInt64Message test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a300100000000000000410100000000000031fefeffffffffffd2c5472bf8dab501fdffffffffffffff0300000000000000"));
    QCOMPARE(test.testRepeatedInt().count(), 6);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::sfixed64List({1, 321, -65999, 123245324235425234, -3, 3}));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedComplexMessageTest()
{
    RepeatedComplexMessage test;
    test.deserialize(serializer.get(), QByteArray::fromHex("0a0c0819120832067177657274790a0c0819120832067177657274790a0c081912083206717765727479"));
    QCOMPARE(test.testRepeatedComplex().count(), 3);
    QCOMPARE(test.testRepeatedComplex().at(0).testFieldInt(), 25);
    QCOMPARE(test.testRepeatedComplex().at(0).testComplexField().testFieldString(), QString("qwerty"));
    QCOMPARE(test.testRepeatedComplex().at(1).testFieldInt(), 25);
    QCOMPARE(test.testRepeatedComplex().at(1).testComplexField().testFieldString(), QString("qwerty"));
    QCOMPARE(test.testRepeatedComplex().at(2).testFieldInt(), 25);
    QCOMPARE(test.testRepeatedComplex().at(2).testComplexField().testFieldString(), QString("qwerty"));

    test.deserialize(serializer.get(), QByteArray::fromHex("0a1508d3feffffffffffffff0112083206717765727479"));
    QVERIFY(!test.testRepeatedComplex().isEmpty());
    QCOMPARE(test.testRepeatedComplex().at(0).testFieldInt(), -173);
    QCOMPARE(test.testRepeatedComplex().at(0).testComplexField().testFieldString(), QString("qwerty"));
}

void QtProtobufRepeatedTypesDeserializationTest::repeatedBoolMessageTest()
{
    RepeatedBoolMessage boolMsg;
    boolMsg.deserialize(serializer.get(), QByteArray::fromHex("0a0d01010100000000000000000001"));
    QCOMPARE(serializer->lastError(), QAbstractProtobufSerializer::Error::None);
    QtProtobuf::boolList expected{ true,  true,  true,  false, false, false, false,
                                   false, false, false, false, false, true };
    QCOMPARE(boolMsg.testRepeatedBool(), expected);
}

QTEST_MAIN(QtProtobufRepeatedTypesDeserializationTest)
#include "tst_protobuf_deserialization_repeatedtypes.moc"
