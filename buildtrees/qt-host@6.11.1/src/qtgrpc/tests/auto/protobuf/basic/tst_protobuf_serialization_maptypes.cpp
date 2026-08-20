// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mapmessages.qpb.h"

#include <qtprotobuftestscommon.h>

#include <QObject>
#include <QProtobufSerializer>
#include <QTest>

using namespace qtprotobufnamespace::tests;

class QtProtobufMapTypesSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }
    void simpleFixed32StringMapSerializeTest();
    void simpleSFixed32StringMapSerializeTest();
    void simpleInt32StringMapSerializeTest();
    void simpleSInt32StringMapSerializeTest();
    void simpleUInt32StringMapSerializeTest();
    void simpleFixed64StringMapSerializeTest();
    void simpleSFixed64StringMapSerializeTest();
    void simpleInt64StringMapSerializeTest();
    void simpleSInt64StringMapSerializeTest();
    void simpleUInt64StringMapSerializeTest();
    void simpleStringStringMapSerializeTest();

private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

void QtProtobufMapTypesSerializationTest::simpleFixed32StringMapSerializeTest()
{
    SimpleFixed32StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(), "3a0a0d0a000000120374656e",
                                     "3a0e0d0f00000012076669667465656e",
                                     "3a110d2a000000120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSFixed32StringMapSerializeTest()
{
    SimpleSFixed32StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { -42, { "minus fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "4a170dd6ffffff12106d696e757320666f757274792074776f",
                                     "4a0a0d0a000000120374656e",
                                     "4a0e0d0f00000012076669667465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleInt32StringMapSerializeTest()
{
    SimpleInt32StringMapMessage test;
    test.setMapField({ { -10, { "minus ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "1a1608f6ffffffffffffffff0112096d696e75732074656e",
                                     "1a0b080f12076669667465656e",
                                     "1a0e082a120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSInt32StringMapSerializeTest()
{
    SimpleSInt32StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { -42, { "minus fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(), "0a14085312106d696e757320666f757274792074776f",
                                     "0a0b081e12076669667465656e", "0a070814120374656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleUInt32StringMapSerializeTest()
{
    SimpleUInt32StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(), "2a07080a120374656e",
                                     "2a0b080f12076669667465656e",
                                     "2a0e082a120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleFixed64StringMapSerializeTest()
{
    SimpleFixed64StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(), "420e090a00000000000000120374656e",
                                     "4212090f0000000000000012076669667465656e",
                                     "4215092a00000000000000120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSFixed64StringMapSerializeTest()
{
    SimpleSFixed64StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { -42, { "minus fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "521b09d6ffffffffffffff12106d696e757320666f757274792074776f",
                                     "520e090a00000000000000120374656e",
                                     "5212090f0000000000000012076669667465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleInt64StringMapSerializeTest()
{
    SimpleInt64StringMapMessage test;
    test.setMapField({ { -10, { "minus ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "221608f6ffffffffffffffff0112096d696e75732074656e",
                                     "220b080f12076669667465656e",
                                     "220e082a120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSInt64StringMapSerializeTest()
{
    SimpleSInt64StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { -42, { "minus fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());
    QVERIFY2(compareSerializedChunks(result.toHex(), "1214085312106d696e757320666f757274792074776f",
                                     "12070814120374656e", "120b081e12076669667465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleUInt64StringMapSerializeTest()
{
    SimpleUInt64StringMapMessage test;
    test.setMapField({ { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(), "3207080a120374656e",
                                     "320b080f12076669667465656e",
                                     "320e082a120a666f757274792074776f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleStringStringMapSerializeTest()
{
    SimpleStringStringMapMessage test;
    test.setMapField({ { "ben", "ten" },
                       { "what is the answer?", "fourty two" },
                       { "sweet", "fifteen" } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(
                     result.toHex(), "6a0a0a0362656e120374656e",
                     "6a100a05737765657412076669667465656e",
                     "6a210a13776861742069732074686520616e737765723f120a666f757274792074776f"),
             result.toHex());
}

QTEST_MAIN(QtProtobufMapTypesSerializationTest)
#include "tst_protobuf_serialization_maptypes.moc"
