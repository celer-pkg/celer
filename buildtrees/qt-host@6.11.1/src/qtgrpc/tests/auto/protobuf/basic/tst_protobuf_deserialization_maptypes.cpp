// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mapmessages.qpb.h"

#include <QObject>

#include <QProtobufSerializer>
#include <QTest>

#include <limits>

using namespace qtprotobufnamespace::tests;

class QtProtobufMapTypesDeserializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { serializer.reset(new QProtobufSerializer); }

    void simpleFixed32StringMapDeserializeTest();
    void simpleSFixed32StringMapDeserializeTest();
    void simpleInt32StringMapDeserializeTest();
    void simpleSInt32StringMapDeserializeTest();
    void simpleUInt32StringMapDeserializeTest();
    void simpleFixed64StringMapDeserializeTest();
    void simpleSFixed64StringMapDeserializeTest();
    void simpleInt64StringMapDeserializeTest();
    void simpleSInt64StringMapDeserializeTest();
    void simpleUInt64StringMapDeserializeTest();
    void simpleStringStringMapDeserializeTest();

private:
    std::unique_ptr<QProtobufSerializer> serializer;
};

void QtProtobufMapTypesDeserializationTest::simpleFixed32StringMapDeserializeTest()
{
    SimpleFixed32StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("3a0a0d0a000000120374656e3a0e0d0f00000012076669667465656e3"
                                         "a110d2a000000120a666f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleFixed32StringMapMessage::MapFieldEntry(
                     { { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleSFixed32StringMapDeserializeTest()
{
    SimpleSFixed32StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("4a170dd6ffffff12106d696e757320666f757274792074776f4a0a0d0"
                                         "a000000120374656e4a0e0d0f00000012076669667465656e"));
    QCOMPARE(test.mapField(),
             SimpleSFixed32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                             { -42, { "minus fourty two" } },
                                                             { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleInt32StringMapDeserializeTest()
{
    SimpleInt32StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("1a1608f6ffffffffffffffff0112096d696e75732074656e1a0b080f1"
                                         "2076669667465656e1a0e082a120a666f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleInt32StringMapMessage::MapFieldEntry({ { -10, { "minus ten" } },
                                                          { 42, { "fourty two" } },
                                                          { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleSInt32StringMapDeserializeTest()
{
    SimpleSInt32StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("0a14085312106d696e757320666f757274792074776f0a07081412037"
                                         "4656e0a0b081e12076669667465656e"));
    QCOMPARE(test.mapField(),
             SimpleSInt32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { -42, { "minus fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleUInt32StringMapDeserializeTest()
{
    SimpleUInt32StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("2a07080a120374656e2a0b080f12076669667465656e2a0e082a120a6"
                                         "66f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleUInt32StringMapMessage::MapFieldEntry(
                     { { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleFixed64StringMapDeserializeTest()
{
    SimpleFixed64StringMapMessage test;
    test.deserialize(
            serializer.get(),
            QByteArray::fromHex("420e090a00000000000000120374656e4212090f00000000000000120766696674"
                                "65656e4215092a00000000000000120a666f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleFixed64StringMapMessage::MapFieldEntry(
                     { { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleSFixed64StringMapDeserializeTest()
{
    SimpleSFixed64StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex(
                             "521b09d6ffffffffffffff12106d696e757320666f757274792074776f520e090a000"
                             "00000000000120374656e5212090f0000000000000012076669667465656e"));
    QCOMPARE(test.mapField(),
             SimpleSFixed64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                             { -42, { "minus fourty two" } },
                                                             { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleInt64StringMapDeserializeTest()
{
    SimpleInt64StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("221608f6ffffffffffffffff0112096d696e75732074656e220b080f1"
                                         "2076669667465656e220e082a120a666f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleInt64StringMapMessage::MapFieldEntry({ { -10, { "minus ten" } },
                                                          { 42, { "fourty two" } },
                                                          { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleSInt64StringMapDeserializeTest()
{
    SimpleSInt64StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("1214085312106d696e757320666f757274792074776f1207081412037"
                                         "4656e120b081e12076669667465656e"));
    QCOMPARE(test.mapField(),
             SimpleSInt64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { -42, { "minus fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleUInt64StringMapDeserializeTest()
{
    SimpleUInt64StringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex("3207080a120374656e320b080f12076669667465656e320e082a120a6"
                                         "66f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleUInt64StringMapMessage::MapFieldEntry(
                     { { 10, { "ten" } }, { 42, { "fourty two" } }, { 15, { "fifteen" } } }));
}

void QtProtobufMapTypesDeserializationTest::simpleStringStringMapDeserializeTest()
{
    SimpleStringStringMapMessage test;
    test.deserialize(serializer.get(),
                     QByteArray::fromHex(
                             "6a0a0a0362656e120374656e6a100a05737765657412076669667465656e6a210a137"
                             "76861742069732074686520616e737765723f120a666f757274792074776f"));
    QCOMPARE(test.mapField(),
             SimpleStringStringMapMessage::MapFieldEntry({ { "ben", "ten" },
                                                           { "what is the answer?", "fourty two" },
                                                           { "sweet", "fifteen" } }));
}

QTEST_MAIN(QtProtobufMapTypesDeserializationTest)
#include "tst_protobuf_deserialization_maptypes.moc"
