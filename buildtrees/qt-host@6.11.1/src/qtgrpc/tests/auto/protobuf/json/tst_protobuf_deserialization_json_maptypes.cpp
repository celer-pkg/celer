// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mapmessages.qpb.h"

#include <QObject>

#include <QProtobufJsonSerializer>
#include <QTest>

#include <limits>

using namespace qtprotobufnamespace::tests;
using namespace Qt::Literals::StringLiterals;

class QtProtobufJsonMapTypesDeserializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { serializer.reset(new QProtobufJsonSerializer); }

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
    void simpleFixed32ComplexMapDeserializeTest();
    void boolBoolMapDeserializeTest();
    void malformedJsonTest();
    void invalidTypeTest();

private:
    std::unique_ptr<QProtobufJsonSerializer> serializer;
};

void QtProtobufJsonMapTypesDeserializationTest::simpleFixed32StringMapDeserializeTest()
{
    SimpleFixed32StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty two\",\"0\":\"\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleFixed32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                            { 42, { "fourty two" } },
                                                            { 15, { "fifteen" } },
                                                            { 0, { "" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleSFixed32StringMapDeserializeTest()
{
    SimpleSFixed32StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-42\":\"minus fourty two\",\"10\":\"ten\",\"15\":\"fifteen\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleSFixed32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                             { -42, { "minus fourty two" } },
                                                             { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleInt32StringMapDeserializeTest()
{
    SimpleInt32StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-10\":\"minus ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleInt32StringMapMessage::MapFieldEntry({ { -10, { "minus ten" } },
                                                          { 42, { "fourty two" } },
                                                          { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleSInt32StringMapDeserializeTest()
{
    SimpleSInt32StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-42\":\"minus fourty two\",\"10\":\"ten\",\"15\":\"fifteen\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleSInt32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { -42, { "minus fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleUInt32StringMapDeserializeTest()
{
    SimpleUInt32StringMapMessage test;
    test.deserialize(serializer.get(),
                     "{\"mapField\":{\"10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleUInt32StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { 42, { "fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleFixed64StringMapDeserializeTest()
{
    SimpleFixed64StringMapMessage test;
    test.deserialize(serializer.get(),
                     "{\"mapField\":{\"10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleFixed64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                            { 42, { "fourty two" } },
                                                            { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleSFixed64StringMapDeserializeTest()
{
    SimpleSFixed64StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-42\":\"minus fourty two\",\"10\":\"ten\",\"15\":\"fifteen\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleSFixed64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                             { -42, { "minus fourty two" } },
                                                             { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleInt64StringMapDeserializeTest()
{
    SimpleInt64StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-10\":\"minus ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleInt64StringMapMessage::MapFieldEntry({ { -10, { "minus ten" } },
                                                          { 42, { "fourty two" } },
                                                          { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleSInt64StringMapDeserializeTest()
{
    SimpleSInt64StringMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-42\":\"minus fourty two\",\"10\":\"ten\",\"15\":\"fifteen\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleSInt64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { -42, { "minus fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleUInt64StringMapDeserializeTest()
{
    SimpleUInt64StringMapMessage test;
    test.deserialize(serializer.get(),
                     "{\"mapField\":{\"10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleUInt64StringMapMessage::MapFieldEntry({ { 10, { "ten" } },
                                                           { 42, { "fourty two" } },
                                                           { 15, { "fifteen" } } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleStringStringMapDeserializeTest()
{
    SimpleStringStringMapMessage test;
    test.deserialize(serializer.get(),
                     "{\"mapField\":{\"ben\":\"ten\",\"sweet\":\"fifteen\","
                     "\"what is the answer?\":\"fourty two\"}}"_ba);
    QCOMPARE(test.mapField(),
             SimpleStringStringMapMessage::MapFieldEntry({ { "ben", "ten" },
                                                           { "what is the answer?", "fourty two" },
                                                           { "sweet", "fifteen" } }));
}

void QtProtobufJsonMapTypesDeserializationTest::simpleFixed32ComplexMapDeserializeTest()
{
    SimpleFixed32ComplexMessageMapMessage test;
    test.deserialize(
        serializer.get(),
        "{\"mapField\":{\"10\":{\"testComplexField\":{\"testFieldString\":\"ten sixteen\"},"
        "\"testFieldInt\":16},\"42\":{\"testComplexField\":{\"testFieldString\":"
        "\"fourty two ten sixteen\"},\"testFieldInt\":10},\"65555\":{\"testComplexField\":"
        "{\"testFieldString\":\"WUT?\"},\"testFieldInt\":10}}}"_ba);
    QCOMPARE(QAbstractProtobufSerializer::Error::None, serializer->lastError());

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

    SimpleFixed32ComplexMessageMapMessage expected;
    expected.setMapField({ { 10, expected1 }, { 42, expected2 }, { 65555, expected3 } });

    QCOMPARE(test.mapField().value(10), expected1);
    QCOMPARE(test.mapField().value(42), expected2);
    QCOMPARE(test.mapField().value(65555), expected3);
}

void QtProtobufJsonMapTypesDeserializationTest::boolBoolMapDeserializeTest()
{
    BoolBoolMessageMapMessage test;
    test.deserialize(serializer.get(), "{\"mapField\":{\"true\":\"false\",\"false\":\"true\"}}");
    QCOMPARE(QAbstractProtobufSerializer::Error::None, serializer->lastError());

    QCOMPARE(test.mapField().value(true), false);
    QCOMPARE(test.mapField().value(false), true);
}

void QtProtobufJsonMapTypesDeserializationTest::malformedJsonTest()
{
    SimpleUInt64StringMapMessage test;
    // no brace
    test.deserialize(serializer.get(),
                     "{\"mapField\":{\"10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty two\"}"_ba);

    QVERIFY(test.mapField().empty());
    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::UnexpectedEndOfStream);

    // skipped ':'
    SimpleStringStringMapMessage test2;
    test2.deserialize(serializer.get(),
                     "{\"mapField\"{\"ben\":\"ten\",\"sweet\":\"fifteen\","
                     "\"what is the answer?\":\"fourty two\"}}"_ba);

    QVERIFY(test2.mapField().empty());
    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::UnexpectedEndOfStream);

    SimpleFixed32StringMapMessage test3;
    // skipped ','
    test3.deserialize(serializer.get(),
                     "{\"mapField\":{\"10\":\"ten\"\"15\":\"fifteen\",\"42\":\"fourty two\"}}"_ba);

    QVERIFY(test3.mapField().empty());
    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::UnexpectedEndOfStream);
}

void QtProtobufJsonMapTypesDeserializationTest::invalidTypeTest()
{
    // Expected int, but float is used
    SimpleInt32StringMapMessage fTest, bTest;
    fTest.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-10\":\"minus ten\",\"15\":\"fifteen\",\"42.3\":\"fourty two\"}}"_ba);

    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::InvalidFormat);

    // -10 for uint32 is used
    SimpleUInt32StringMapMessage uTest;
    uTest.deserialize(serializer.get(),
                     "{\"mapField\":{\"-10\":\"ten\",\"15\":\"fifteen\",\"42\":\"fourty"
                      " two\"}}"_ba);

    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::InvalidFormat);

    // expected int, but bool is used
    bTest.deserialize(
        serializer.get(),
        "{\"mapField\":{\"-10\":\"minus ten\",\"15\":\"fifteen\",\"false\":\"fourty two\"}}"_ba);

    QCOMPARE(serializer->lastError(),
             QAbstractProtobufSerializer::Error::InvalidFormat);
}

QTEST_MAIN(QtProtobufJsonMapTypesDeserializationTest)
#include "tst_protobuf_deserialization_json_maptypes.moc"
