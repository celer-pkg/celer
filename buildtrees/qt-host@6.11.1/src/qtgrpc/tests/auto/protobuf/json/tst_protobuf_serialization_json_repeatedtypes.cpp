// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//#include "basicmessages.qpb.h"
#include "repeatedmessages.qpb.h"

#include <QProtobufJsonSerializer>
#include <QJsonDocument>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class QtProtobufRepeatedTypesJsonSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        m_serializer.reset(new QProtobufJsonSerializer);
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
    std::unique_ptr<QProtobufJsonSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedIntMessageTest()
{
    RepeatedIntMessage test;
    test.setTestRepeatedInt({0, 1, 321, -65999, 123245, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());

    QCOMPARE(result, "{\"testRepeatedInt\":[0,1,321,-65999,123245,-3,3]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::int32List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedSIntMessageTest()
{
    RepeatedSIntMessage test;
    test.setTestRepeatedInt({1, 321, -65999, 123245, -3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,321,-65999,123245,-3,3,0]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::sint32List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedUIntMessageTest()
{
    RepeatedUIntMessage test;
    test.setTestRepeatedInt({1, 0, 321, 65999, 123245, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,0,321,65999,123245,3]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::uint32List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedInt64MessageTest()
{
    RepeatedInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 12324523123123, -3, 0, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,321,-65999,12324523123123,-3,0,3]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::int64List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedSInt64MessageTest()
{
    RepeatedSInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 12324523123123, 0, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,321,-65999,12324523123123,0,-3,3]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::sint64List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedUInt64MessageTest()
{
    RepeatedUInt64Message test;
    test.setTestRepeatedInt({1, 321, 0, 65999, 123245, 123245324235425234, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(
        result,
        "{\"testRepeatedInt\":[\"1\",\"321\",\"0\",\"65999\",\"123245\",\"123245324235425234\",\"3\"]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::uint64List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedFixedIntMessageTest()
{
    RepeatedFixedIntMessage test;
    test.setTestRepeatedInt({1, 321, 65999, 12324523, 3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,321,65999,12324523,3,3,0]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::fixed32List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedSFixedIntMessageTest()
{
    RepeatedSFixedIntMessage test;
    test.setTestRepeatedInt({0, 1, 321, -65999, 12324523, -3, 3});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[0,1,321,-65999,12324523,-3,3]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::sfixed32List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedFixedInt64MessageTest()
{
    RepeatedFixedInt64Message test;
    test.setTestRepeatedInt({1, 321, 65999, 123245324235425234, 3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(
        result,
        "{\"testRepeatedInt\":[\"1\",\"321\",\"65999\",\"123245324235425234\",\"3\",\"3\",\"0\"]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::fixed64List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedSFixedInt64MessageTest()
{
    RepeatedSFixedInt64Message test;
    test.setTestRepeatedInt({1, 321, -65999, 123245324235425234, -3, 3, 0});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"testRepeatedInt\":[1,321,-65999,123245324235425234,-3,3,0]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedInt(QtProtobuf::sfixed64List());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedStringMessageTest()
{
    RepeatedStringMessage test;
    test.setTestRepeatedString({"aaaa","bbbbb","ccc","dddddd","eeeee", ""});
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result,
             "{\"testRepeatedString\":[\"aaaa\",\"bbbbb\",\"ccc\",\"dddddd\",\"eeeee\",\"\"]}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());

    test.setTestRepeatedString(QStringList());
    result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedFloatMessageTest()
{
    RepeatedFloatMessage test;
    test.setTestRepeatedFloat({ 0.4f, 1.2f, 0.5f, 3.91235f, 0.6f });
    QByteArray result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(
        result,
        "{\"testRepeatedFloat\":[0.4000000059604645,1.2000000476837158,0.5,3.9123499393463135,0.6000000238418579]}"_ba);

    test.setTestRepeatedFloat(QtProtobuf::floatList());
    result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{}"_ba);
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedDoubleMessageTest()
{
    RepeatedDoubleMessage test;
    test.setTestRepeatedDouble({0.1, 0.2, 0.3, 3.912348239293, 0.5});
    QByteArray result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{\"testRepeatedDouble\":[0.1,0.2,0.3,3.912348239293,0.5]}"_ba);

    test.setTestRepeatedDouble(QtProtobuf::doubleList());
    result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{}"_ba);
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedBytesMessageTest()
{
    RepeatedBytesMessage test;
    test.setTestRepeatedBytes({QByteArray::fromHex("010203040506"),
                                QByteArray::fromHex("ffffffff"),
                                QByteArray::fromHex("eaeaeaeaea"),
                                QByteArray::fromHex("010203040506")});
    QByteArray result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result,
             "{\"testRepeatedBytes\":[\"AQIDBAUG\",\"/////w==\",\"6urq6uo=\",\"AQIDBAUG\"]}"_ba);

    test.setTestRepeatedBytes(QByteArrayList());
    result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{}"_ba);

    test.setTestRepeatedBytes({QByteArray::fromHex("010203040506"),
                                QByteArray::fromHex(""),
                                QByteArray::fromHex("eaeaeaeaea"),
                                QByteArray::fromHex("010203040506")});
    result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result,
             "{\"testRepeatedBytes\":[\"AQIDBAUG\",\"\",\"6urq6uo=\",\"AQIDBAUG\"]}"_ba);
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedComplexMessageTest()
{
    SimpleStringMessage stringMsg;
    stringMsg.setTestFieldString("qwerty");
    ComplexMessage msg;
    msg.setTestFieldInt(25);
    msg.setTestComplexField(stringMsg);
    RepeatedComplexMessage test;
    test.setTestRepeatedComplex({msg, msg, msg});
    QByteArray result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(
        result,
        "{\"testRepeatedComplex\":[{\"testComplexField\":{\"testFieldString\":\"qwerty\"},\"testFieldInt\":25},{\"testComplexField\":{\"testFieldString\":\"qwerty\"},\"testFieldInt\":25},{\"testComplexField\":{\"testFieldString\":\"qwerty\"},\"testFieldInt\":25}]}"_ba);

    test.setTestRepeatedComplex({});
    result = test.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{}"_ba);
}

void QtProtobufRepeatedTypesJsonSerializationTest::repeatedBoolMessageTest()
{
    RepeatedBoolMessage boolMsg;
    boolMsg.setTestRepeatedBool({ true, true, true, false, false, false, false, false,
                                  false, false, false, false, true });
    QByteArray result = boolMsg.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{\"testRepeatedBool\":[true,true,true,false,false,false,false,"
                     "false,false,false,false,false,true]}"_ba);

    boolMsg.setTestRepeatedBool(QtProtobuf::boolList());
    result = boolMsg.serialize(m_serializer.get());
    QVERIFY(!QJsonDocument::fromJson(result).isNull());
    QCOMPARE(result, "{}"_ba);
}

QTEST_MAIN(QtProtobufRepeatedTypesJsonSerializationTest)
#include "tst_protobuf_serialization_json_repeatedtypes.moc"
