// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "repeatednonpackedmessages.qpb.h"

#include <QTest>
#include <QProtobufSerializer>

class QtProtobufNonPackedRepeatedTypesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }
    void repeatedIntNonPackedMessageSerializerTest();
    void repeatedSIntNonPackedMessageSerializerTest();
    void repeatedUIntNonPackedMessageSerializerTest();
    void repeatedInt64NonPackedMessageSerializerTest();
    void repeatedSInt64NonPackedMessageSerializerTest();
    void repeatedUInt64NonPackedMessageSerializerTest();
    void repeatedFixedIntNonPackedMessageSerializerTest();
    void repeatedSFixedIntNonPackedMessageSerializerTest();
    void repeatedFixedInt64NonPackedMessageSerializerTest();
    void repeatedSFixedInt64NonPackedMessageSerializerTest();
    void repeatedFloatNonPackedMessageSerializerTest();
    void repeatedDoubleNonPackedMessageSerializerTest();
    void repeatedBoolNonPackedMessageSerializerTest();
    void repeatedEnumNonPackedMessageSerializerTest();

    void repeatedIntNonPackedMessageDeserializerTest();
    void nonPackedIntWithInterleavedExtra();
    void repeatedSIntNonPackedMessageDeserializerTest();
    void repeatedUIntNonPackedMessageDeserializerTest();
    void repeatedInt64NonPackedMessageDeserializerTest();
    void repeatedSInt64NonPackedMessageDeserializerTest();
    void repeatedUInt64NonPackedMessageDeserializerTest();
    void repeatedFixedIntNonPackedMessageDeserializerTest();
    void repeatedSFixedIntNonPackedMessageDeserializerTest();
    void repeatedFixedInt64NonPackedMessageDeserializerTest();
    void repeatedSFixedInt64NonPackedMessageDeserializerTest();
    void repeatedFloatNonPackedMessageDeserializerTest();
    void repeatedDoubleNonPackedMessageDeserializerTest();
    void repeatedBoolNonPackedMessageDeserializerTest();
    void repeatedEnumNonPackedMessageDeserializerTest();

private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

void QtProtobufNonPackedRepeatedTypesTest::repeatedIntNonPackedMessageSerializerTest()
{
    RepeatedNonPackedIntMessage test;
    test.setTestRepeatedInt({ 0, 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int32_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("08000801080008c102080008b1fcfbffffffffffff0108edc20708fdffffffffffffffff010803"
                    "0800088080feffffffffffff010880808080f8ffffffff01");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::int32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSIntNonPackedMessageSerializerTest()
{
    RepeatedNonPackedSIntMessage test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int32_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("080208000882050800089d870808da850f08050806080008ffff0308ffffffff0f");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::sint32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedUIntNonPackedMessageSerializerTest()
{
    RepeatedNonPackedUIntMessage test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, 123245, 3, 0, std::numeric_limits<uint16_t>::max(),
                              std::numeric_limits<uint32_t>::max() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult = QString("0801080008c102080008edc2070803080008ffff0308ffffffff0f");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::uint32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedInt64NonPackedMessageSerializerTest()
{
    RepeatedNonPackedInt64Message test;
    test.setTestRepeatedInt({ 0, 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int64_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("08000801080008c102080008b1fcfbffffffffffff0108edc20708fdffffffffffffffff010803"
                    "0800088080feffffffffffff010880808080808080808001");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::int64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSInt64NonPackedMessageSerializerTest()
{
    RepeatedNonPackedSInt64Message test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int64_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("080208000882050800089d870808da850f08050806080008ffff0308ffffffffffffffffff01");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::sint64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedUInt64NonPackedMessageSerializerTest()
{
    RepeatedNonPackedUInt64Message test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, 123245, 3, 0, std::numeric_limits<uint16_t>::max(),
                              std::numeric_limits<uint64_t>::max() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("0801080008c102080008edc2070803080008ffff0308ffffffffffffffffff01");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::uint64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFixedIntNonPackedMessageSerializerTest()
{
    RepeatedNonPackedFixedIntMessage test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, 123245, 3, 0, std::numeric_limits<uint16_t>::max(),
                              std::numeric_limits<uint32_t>::max() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult = QString("0d010000000d000000000d410100000d000000000d6de101000d030000000"
                                     "d000000000dffff00000dffffffff");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::fixed32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSFixedIntNonPackedMessageSerializerTest()
{
    RepeatedNonPackedSFixedIntMessage test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int32_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult = QString("0d010000000d000000000d410100000d000000000d31fefeff0d6de101000"
                                     "dfdffffff0d030000000d000000000d0080ffff0d00000080");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::sfixed32List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFixedInt64NonPackedMessageSerializerTest()
{
    RepeatedNonPackedFixedInt64Message test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, 123245, 3, 0, std::numeric_limits<uint16_t>::max(),
                              std::numeric_limits<uint64_t>::max() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult = QString(
            "090100000000000000090000000000000000094101000000000000090000000000000000096de101000000"
            "000009030000000000000009000000000000000009ffff00000000000009ffffffffffffffff");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::fixed64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSFixedInt64NonPackedMessageSerializerTest()
{
    RepeatedNonPackedSFixedInt64Message test;
    test.setTestRepeatedInt({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                              std::numeric_limits<int16_t>::min(),
                              std::numeric_limits<int64_t>::min() });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("0901000000000000000900000000000000000941010000000000000900000000000000000931fe"
                    "feffffffffff096de101000000000009fdffffffffffffff090300000000000000090000000000"
                    "000000090080ffffffffffff090000000000000080");
    QCOMPARE(result.toHex(), expectedResult);
    test.setTestRepeatedInt(QtProtobuf::sfixed64List());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedDoubleNonPackedMessageSerializerTest()
{
    RepeatedNonPackedDoubleMessage test;
    test.setTestRepeatedDouble({ 0.1, 0.2, 0.3, 0.4, 0.5, 0.0 });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult = QString("099a9999999999b93f099a9999999999c93f09333333333333d33f099a999"
                                     "9999999d93f09000000000000e03f090000000000000000");
    QCOMPARE(result.toHex(), expectedResult);

    test.setTestRepeatedDouble(QtProtobuf::doubleList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFloatNonPackedMessageSerializerTest()
{
    RepeatedNonPackedFloatMessage test;
    test.setTestRepeatedFloat({ 0.0, 0.4f, 1.2f, 0.5f, 1.4f, 0.6f });
    QByteArray result = test.serialize(m_serializer.get());
    QString expectedResult =
            QString("0d000000000dcdcccc3e0d9a99993f0d0000003f0d3333b33f0d9a99193f");
    QCOMPARE(result.toHex(), expectedResult);

    test.setTestRepeatedFloat(QtProtobuf::floatList());
    result = test.serialize(m_serializer.get());
    QVERIFY(result.isEmpty());
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedBoolNonPackedMessageSerializerTest()
{
    RepeatedNonPackedBoolMessage boolMsg;
    boolMsg.setTestRepeatedBool({ true, true, true, false, false, true, false, false, false, false,
                                  false, false, true });
    QByteArray result = boolMsg.serialize(m_serializer.get());
    QString expectedResult = QString("0801080108010800080008010800080008000800080008000801");
    QCOMPARE(result.toHex(), expectedResult);
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedEnumNonPackedMessageSerializerTest()
{
    RepeatedNonPackedEnumMessage enumMsg;
    enumMsg.setField({ RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE0,
                       RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE3,
                       RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE2 });
    QByteArray result = enumMsg.serialize(m_serializer.get());
    QString expectedResult = QString("080008030802");
    QCOMPARE(result.toHex(), expectedResult);
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedIntNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedIntMessage test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex(
                    "08000801080008c102080008b1fcfbffffffffffff0108edc20708fdffffffffffffffff010803"
                    "0800088080feffffffffffff010880808080f8ffffffff01"));
    QCOMPARE(test.testRepeatedInt().count(), 12);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::int32List({ 0, 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                     std::numeric_limits<int16_t>::min(),
                                     std::numeric_limits<int32_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::nonPackedIntWithInterleavedExtra()
{
    NonPackedIntMessageWithExtraMember test;
    // [0, 1], "242", [3] - the two arrays are actually the same,
    // but the entries are separated by a string.
    QByteArray input = QByteArray::fromHex("0800080112033234320803");
    test.deserialize(m_serializer.get(), input);
    QCOMPARE(test.testRepeatedInt().count(), 3);
    QCOMPARE(test.testRepeatedInt(), QtProtobuf::int32List({ 0, 1, 3 }));
    QCOMPARE(test.extra(), "242");
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSIntNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedSIntMessage test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex(
                             "080208000882050800089d870808da850f08050806080008ffff0308ffffffff0f"));
    QCOMPARE(test.testRepeatedInt().count(), 11);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::sint32List({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                      std::numeric_limits<int16_t>::min(),
                                      std::numeric_limits<int32_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedUIntNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedUIntMessage test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex("0801080008c102080008edc2070803080008ffff0308ffffffff0f"));
    QCOMPARE(test.testRepeatedInt().count(), 9);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::uint32List({ 1, 0, 321, 0, 123245, 3, 0,
                                      std::numeric_limits<uint16_t>::max(),
                                      std::numeric_limits<uint32_t>::max() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedInt64NonPackedMessageDeserializerTest()
{
    RepeatedNonPackedInt64Message test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex(
                    "08000801080008c102080008b1fcfbffffffffffff0108edc20708fdffffffffffffffff010803"
                    "0800088080feffffffffffff010880808080808080808001"));
    QCOMPARE(test.testRepeatedInt().count(), 12);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::int64List({ 0, 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                     std::numeric_limits<int16_t>::min(),
                                     std::numeric_limits<int64_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSInt64NonPackedMessageDeserializerTest()
{
    RepeatedNonPackedSInt64Message test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex("080208000882050800089d870808da850f08050806080008ffff0308f"
                                         "fffffffffffffffff01"));
    QCOMPARE(test.testRepeatedInt().count(), 11);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::sint64List({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                      std::numeric_limits<int16_t>::min(),
                                      std::numeric_limits<int64_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedUInt64NonPackedMessageDeserializerTest()
{
    RepeatedNonPackedUInt64Message test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex(
                             "0801080008c102080008edc2070803080008ffff0308ffffffffffffffffff01"));
    QCOMPARE(test.testRepeatedInt().count(), 9);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::uint64List({ 1, 0, 321, 0, 123245, 3, 0,
                                      std::numeric_limits<uint16_t>::max(),
                                      std::numeric_limits<uint64_t>::max() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFixedIntNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedFixedIntMessage test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex("0d010000000d000000000d410100000d000000000d6de101000d030000000"
                                "d000000000dffff00000dffffffff"));
    QCOMPARE(test.testRepeatedInt().count(), 9);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::fixed32List({ 1, 0, 321, 0, 123245, 3, 0,
                                       std::numeric_limits<uint16_t>::max(),
                                       std::numeric_limits<uint32_t>::max() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSFixedIntNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedSFixedIntMessage test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex("0d010000000d000000000d410100000d000000000d31fefeff0d6de101000"
                                "dfdffffff0d030000000d000000000d0080ffff0d00000080"));
    QCOMPARE(test.testRepeatedInt().count(), 11);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::sfixed32List({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                        std::numeric_limits<int16_t>::min(),
                                        std::numeric_limits<int32_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFixedInt64NonPackedMessageDeserializerTest()
{
    RepeatedNonPackedFixedInt64Message test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex("090100000000000000090000000000000000094101000000000000090"
                                         "000000000000000096de101000000"
                                         "000009030000000000000009000000000000000009ffff00000000000"
                                         "009ffffffffffffffff"));
    QCOMPARE(test.testRepeatedInt().count(), 9);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::fixed64List({ 1, 0, 321, 0, 123245, 3, 0,
                                       std::numeric_limits<uint16_t>::max(),
                                       std::numeric_limits<uint64_t>::max() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedSFixedInt64NonPackedMessageDeserializerTest()
{
    RepeatedNonPackedSFixedInt64Message test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex(
                    "0901000000000000000900000000000000000941010000000000000900000000000000000931fe"
                    "feffffffffff096de101000000000009fdffffffffffffff090300000000000000090000000000"
                    "000000090080ffffffffffff090000000000000080"));
    QCOMPARE(test.testRepeatedInt().count(), 11);
    QCOMPARE(test.testRepeatedInt(),
             QtProtobuf::sfixed64List({ 1, 0, 321, 0, -65999, 123245, -3, 3, 0,
                                        std::numeric_limits<int16_t>::min(),
                                        std::numeric_limits<int64_t>::min() }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedFloatNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedFloatMessage test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex("0d000000000dcdcccc3e0d9a99993f0d0000003f0d3333b33f0d9a99193f"));
    QCOMPARE(test.testRepeatedFloat().count(), 6);
    QCOMPARE(test.testRepeatedFloat(),
             QtProtobuf::floatList({ 0.0, 0.4f, 1.2f, 0.5f, 1.4f, 0.6f }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedDoubleNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedDoubleMessage test;
    test.deserialize(
            m_serializer.get(),
            QByteArray::fromHex("099a9999999999b93f099a9999999999c93f09333333333333d33f099a999"
                                "9999999d93f09000000000000e03f090000000000000000"));
    QCOMPARE(test.testRepeatedDouble().count(), 6);
    QCOMPARE(test.testRepeatedDouble(), QtProtobuf::doubleList({ 0.1, 0.2, 0.3, 0.4, 0.5, 0.0 }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedBoolNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedBoolMessage test;
    test.deserialize(m_serializer.get(),
                     QByteArray::fromHex("0801080108010800080008010800080008000800080008000801"));
    QCOMPARE(test.testRepeatedBool().count(), 13);
    QCOMPARE(test.testRepeatedBool(),
             QtProtobuf::boolList({ true, true, true, false, false, true, false, false, false,
                                    false, false, false, true }));
}

void QtProtobufNonPackedRepeatedTypesTest::repeatedEnumNonPackedMessageDeserializerTest()
{
    RepeatedNonPackedEnumMessage test;
    test.deserialize(m_serializer.get(), QByteArray::fromHex("080308000802"));
    QCOMPARE(test.field().count(), 3);
    QCOMPARE(test.field(),
             QList<RepeatedNonPackedEnumMessage::
                       TestEnum>({ RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE3,
                                   RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE0,
                                   RepeatedNonPackedEnumMessage::TestEnum::ENUM_VALUE2 }));
}
QTEST_MAIN(QtProtobufNonPackedRepeatedTypesTest)
#include "tst_protobuf_non_packed_repeatedtypes.moc"
