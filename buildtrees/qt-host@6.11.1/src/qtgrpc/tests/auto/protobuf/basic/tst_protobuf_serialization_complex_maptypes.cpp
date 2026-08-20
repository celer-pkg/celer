// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"
#include "mapmessages.qpb.h"
#include <QObject>
#include <QProtobufSerializer>
#include <QTest>

#include <qtprotobuftestscommon.h>

using namespace qtprotobufnamespace::tests;

class QtProtobufMapTypesSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }
    void simpleFixed32ComplexMapSerializeTest();
    void simpleSFixed32ComplexMapSerializeTest();
    void simpleInt32ComplexMapSerializeTest();
    void simpleSInt32ComplexMapSerializeTest();
    void simpleUInt32ComplexMapSerializeTest();
    void simpleFixed64ComplexMapSerializeTest();
    void simpleSFixed64ComplexMapSerializeTest();
    void simpleInt64ComplexMapSerializeTest();
    void simpleSInt64ComplexMapSerializeTest();
    void simpleUInt64ComplexMapSerializeTest();
    void simpleStringComplexMapSerializeTest();

private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

//Complex map
void QtProtobufMapTypesSerializationTest::simpleFixed32ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleFixed32ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { 42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(
                     result.toHex(), "3a180d0a00000012110810120d320b74656e207369787465656e",
                     "3a230d2a000000121c080a12183216666f757274792074776f2074656e207369787465656e",
                     "3a110d13000100120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSFixed32ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleSFixed32ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { -42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "4a290dd6ffffff1222080a121e321c6d696e757320666f757274792074776"
                                     "f2074656e207369787465656e",
                                     "4a180d0a00000012110810120d320b74656e207369787465656e",
                                     "4a110d13000100120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleInt32ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleInt32ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { -42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "1a2f08d6ffffffffffffffff011222080a121e321c6d696e757320666f757"
                                     "274792074776f2074656e207369787465656e",
                                     "1a15080a12110810120d320b74656e207369787465656e",
                                     "1a1008938004120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSInt32ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleSInt32ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { 42, ComplexMessage(expected2) },
                       { -65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(
                     result.toHex(),
                     "0a200854121c080a12183216666f757274792074776f2074656e207369787465656e",
                     "0a1608a580081210080a120c320a6d696e7573205755543f",
                     "0a15081412110810120d320b74656e207369787465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleUInt32ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleUInt32ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { 42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(
                     result.toHex(), "2a1008938004120a080a120632045755543f",
                     "2a20082a121c080a12183216666f757274792074776f2074656e207369787465656e",
                     "2a15080a12110810120d320b74656e207369787465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleFixed64ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten MAAAX" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(42);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleFixed64ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { UINT64_MAX, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "421c090a0000000000000012110810120d320b74656e207369787465656e",
                                     "422b09ffffffffffffffff1220082a121c321a6d696e757320666f7572747"
                                     "92074776f2074656e204d41414158",
                                     "4215091300010000000000120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSFixed64ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleSFixed64ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { -42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "521c090a0000000000000012110810120d320b74656e207369787465656e",
                                     "522d09d6ffffffffffffff1222080a121e321c6d696e757320666f7572747"
                                     "92074776f2074656e207369787465656e",
                                     "5215091300010000000000120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleInt64ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleInt64ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { -42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "2215080a12110810120d320b74656e207369787465656e",
                                     "222f08d6ffffffffffffffff011222080a121e321c6d696e757320666f757"
                                     "274792074776f2074656e207369787465656e",
                                     "221008938004120a080a120632045755543f"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleSInt64ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten sixteen" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(16);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "minus fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleSInt64ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { -42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "1215081412110810120d320b74656e207369787465656e",
                                     "121008a68008120a080a120632045755543f",
                                     "122608531222080a121e321c6d696e757320666f757274792074776f20746"
                                     "56e207369787465656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleUInt64ComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten eleven" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(11);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleUInt64ComplexMessageMapMessage test;
    test.setMapField({ { 10, ComplexMessage(expected1) },
                       { 42, ComplexMessage(expected2) },
                       { 65555, ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(
                     result.toHex(),
                     "3220082a121c080a12183216666f757274792074776f2074656e207369787465656e",
                     "321008938004120a080a120632045755543f",
                     "3214080a1210080b120c320a74656e20656c6576656e"),
             result.toHex());
}

void QtProtobufMapTypesSerializationTest::simpleStringComplexMapSerializeTest()
{
    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;

    stringMsg.setTestFieldString({ "ten eleven" });
    ComplexMessage expected1;
    expected1.setTestFieldInt(11);
    expected1.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "fourty two ten sixteen" });
    ComplexMessage expected2;
    expected2.setTestFieldInt(10);
    expected2.setTestComplexField(stringMsg);

    stringMsg.setTestFieldString({ "?WUT?" });
    ComplexMessage expected3;
    expected3.setTestFieldInt(10);
    expected3.setTestComplexField(stringMsg);

    SimpleStringComplexMessageMapMessage test;
    test.setMapField({ { "ben", ComplexMessage(expected1) },
                       { "where is my car dude?", ComplexMessage(expected2) },
                       { "WUT??", ComplexMessage(expected3) } });
    QByteArray result = test.serialize(m_serializer.get());

    QVERIFY2(compareSerializedChunks(result.toHex(),
                                     "6a170a0362656e1210080b120c320a74656e20656c6576656e",
                                     "6a140a055755543f3f120b080a120732053f5755543f",
                                     "6a350a157768657265206973206d792063617220647564653f121c080a121"
                                     "83216666f757274792074776f2074656e207369787465656e"),
             result.toHex());
}

QTEST_MAIN(QtProtobufMapTypesSerializationTest)
#include "tst_protobuf_serialization_complex_maptypes.moc"
