// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>

#include <memory>

#include <qtprotobuftestscommon.h>

using namespace Qt::Literals::StringLiterals;

class QtProtobufTypesGenerationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void emptyMessageTest();
    void boolMessageTest();
    void intMessageTest();
    void sIntMessageTest();
    void uIntMessageTest();
    void int64MessageTest();
    void sInt64MessageTest();
    void uInt64MessageTest();
    void fixedInt32MessageTest();
    void fixedInt64MessageTest();
    void sFixedInt32MessageTest();
    void sFixedInt64MessageTest();
    void stringMessageTest();
    void floatMessageTest();
    void doubleMessageTest();
    void complexMessageTest();
    void bytesMessageTest();
    void oneofMessageEmptyTest();
    void oneofMessageTest();
    void oneofMessageCopyComplexValueTest();

    void assignmentOperatorTest();
    void moveOperatorTest();
    void rvalueSettersTest();
    void rvalueOneOfSettersTest();
    void mutableGetterTest();

    void invalidMessageConstructorTest();
};

using namespace qtprotobufnamespace::tests;

void QtProtobufTypesGenerationTest::emptyMessageTest()
{
    QCOMPARE(qtprotobufnamespace::tests::EmptyMessage::staticPropertyOrdering.fieldCount(), 0);
    QCOMPARE(qtprotobufnamespace::tests::EmptyMessage::staticMetaObject.propertyCount(), 0);
    QCOMPARE(qtprotobufnamespace::tests::EmptyMessage::staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.EmptyMessage");

    QProtobufMessagePointer rawMessage(
            QProtobufMessage::constructByName("qtprotobufnamespace.tests.EmptyMessage"));
    QVERIFY(reinterpret_cast<qtprotobufnamespace::tests::EmptyMessage*>(rawMessage.get()) != nullptr);

    // Move from and reuse. This should compile and run:
    qtprotobufnamespace::tests::EmptyMessage from;
    qtprotobufnamespace::tests::EmptyMessage to = std::move(from);
    from = to;
    QCOMPARE(from, to);

    qtprotobufnamespace::tests::EmptyMessage bucket = std::move(to);
    bucket = std::move(from);

    from = to;
    QCOMPARE(from, to);
}

void QtProtobufTypesGenerationTest::boolMessageTest()
{
    const char *propertyName = "testFieldBool";
    qProtobufAssertMessagePropertyRegistered<SimpleBoolMessage, bool>(1, "bool", propertyName);

    SimpleBoolMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue(true)));
    QCOMPARE(test.property(propertyName).value<bool>(), true);
    QCOMPARE(test.testFieldBool(), true);

    QCOMPARE(int(SimpleBoolMessage::QtProtobufFieldEnum::TestFieldBoolProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleBoolMessage");

    // Move from and reuse
    qtprotobufnamespace::tests::SimpleBoolMessage from;
    qtprotobufnamespace::tests::SimpleBoolMessage to = std::move(from);
    from = to;
    QCOMPARE(from.testFieldBool(), to.testFieldBool());
    // Changes in one should not be visible in the other:
    to.setTestFieldBool(!to.testFieldBool());
    QCOMPARE_NE(from.testFieldBool(), to.testFieldBool());

    from = to;
    to.setProperty(propertyName, QVariant::fromValue(!to.testFieldBool()));
    QCOMPARE_NE(from.testFieldBool(), to.testFieldBool());

}

void QtProtobufTypesGenerationTest::intMessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<qtprotobufnamespace::tests::SimpleIntMessage, QtProtobuf::int32>(1, "QtProtobuf::int32", propertyName);

    qtprotobufnamespace::tests::SimpleIntMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::int32>(), 1);
    QCOMPARE(test.testFieldInt(), 1);

    QCOMPARE(int(qtprotobufnamespace::tests::SimpleIntMessage::QtProtobufFieldEnum::
                         TestFieldIntProtoFieldNumber),
             1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleIntMessage");
}

void QtProtobufTypesGenerationTest::sIntMessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<SimpleSIntMessage, QtProtobuf::sint32>(1, "QtProtobuf::sint32", propertyName);

    SimpleSIntMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::sint32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::sint32>(), 1);
    QCOMPARE(test.testFieldInt(), 1);

    QCOMPARE(int(SimpleSIntMessage::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleSIntMessage");
}

void QtProtobufTypesGenerationTest::uIntMessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<SimpleUIntMessage, QtProtobuf::uint32>(1, "QtProtobuf::uint32", propertyName);

    SimpleUIntMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::uint32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::uint32>(), (uint32_t)1);
    QCOMPARE(test.testFieldInt(), 1u);

    QCOMPARE(int(SimpleUIntMessage::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleUIntMessage");
}

void QtProtobufTypesGenerationTest::int64MessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<SimpleInt64Message, QtProtobuf::int64>(1, "QtProtobuf::int64", propertyName);

    SimpleInt64Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int64>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::int64>(), 1);
    QCOMPARE(test.testFieldInt(), 1);

    QCOMPARE(int(SimpleInt64Message::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleInt64Message");
}

void QtProtobufTypesGenerationTest::sInt64MessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<SimpleSInt64Message, QtProtobuf::sint64>(1, "QtProtobuf::sint64", propertyName);

    SimpleSInt64Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::sint64>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::sint64>(), 1);
    QCOMPARE(test.testFieldInt(), 1);

    QCOMPARE(int(SimpleSInt64Message::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleSInt64Message");
}

void QtProtobufTypesGenerationTest::uInt64MessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<SimpleUInt64Message, QtProtobuf::uint64>(1, "QtProtobuf::uint64", propertyName);

    SimpleUInt64Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::uint64>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::uint64>(), 1u);
    QCOMPARE(test.testFieldInt(), 1u);

    QCOMPARE(uint(SimpleUInt64Message::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1u);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleUInt64Message");
}

void QtProtobufTypesGenerationTest::fixedInt32MessageTest()
{
    const char *propertyName = "testFieldFixedInt32";
    qProtobufAssertMessagePropertyRegistered<SimpleFixedInt32Message, QtProtobuf::fixed32>(1, "QtProtobuf::fixed32", propertyName);

    SimpleFixedInt32Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::fixed32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::fixed32>(), 1u);
    QCOMPARE(test.testFieldFixedInt32(), 1u);

    QCOMPARE(
            uint(SimpleFixedInt32Message::QtProtobufFieldEnum::TestFieldFixedInt32ProtoFieldNumber),
            1u);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleFixedInt32Message");
}

void QtProtobufTypesGenerationTest::fixedInt64MessageTest()
{
    const char *propertyName = "testFieldFixedInt64";
    qProtobufAssertMessagePropertyRegistered<SimpleFixedInt64Message, QtProtobuf::fixed64>(1, "QtProtobuf::fixed64", propertyName);

    SimpleFixedInt64Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::fixed64>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::fixed64>(), 1u);
    QCOMPARE(test.testFieldFixedInt64(), 1u);

    QCOMPARE(
            uint(SimpleFixedInt64Message::QtProtobufFieldEnum::TestFieldFixedInt64ProtoFieldNumber),
            1u);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleFixedInt64Message");
}

void QtProtobufTypesGenerationTest::sFixedInt32MessageTest()
{
    const char *propertyName = "testFieldFixedInt32";
    qProtobufAssertMessagePropertyRegistered<SimpleSFixedInt32Message, QtProtobuf::sfixed32>(1, "QtProtobuf::sfixed32", propertyName);

    SimpleSFixedInt32Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::sfixed32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::sfixed32>(), 1);
    QCOMPARE(test.testFieldFixedInt32(), 1);

    QCOMPARE(
            int(SimpleSFixedInt32Message::QtProtobufFieldEnum::TestFieldFixedInt32ProtoFieldNumber),
            1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleSFixedInt32Message");
}

void QtProtobufTypesGenerationTest::sFixedInt64MessageTest()
{
    const char *propertyName = "testFieldFixedInt64";
    qProtobufAssertMessagePropertyRegistered<SimpleSFixedInt64Message, QtProtobuf::sfixed64>(1, "QtProtobuf::sfixed64", propertyName);

    SimpleSFixedInt64Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::sfixed64>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::sfixed64>(), 1);
    QCOMPARE(test.testFieldFixedInt64(), 1);

    QCOMPARE(
            int(SimpleSFixedInt64Message::QtProtobufFieldEnum::TestFieldFixedInt64ProtoFieldNumber),
            1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleSFixedInt64Message");
}

void QtProtobufTypesGenerationTest::stringMessageTest()
{
    const char *propertyName = "testFieldString";
    SimpleStringMessage test;
    int index = SimpleStringMessage::staticPropertyOrdering.indexOfFieldNumber(6);
    int propertyNumber = SimpleStringMessage::staticPropertyOrdering.propertyIndex(index); // See simpletest.proto
    QCOMPARE(SimpleStringMessage::staticMetaObject.property(propertyNumber).typeId(), QMetaType::QString);
    QCOMPARE(SimpleStringMessage::staticMetaObject.property(propertyNumber).name(), propertyName);
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue(QString("test1"))));
    QCOMPARE(test.property(propertyName).toString(), "test1"_L1);
    QCOMPARE(test.testFieldString(), "test1"_L1);

    QCOMPARE(int(SimpleStringMessage::QtProtobufFieldEnum::TestFieldStringProtoFieldNumber), 6);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleStringMessage");
}

void QtProtobufTypesGenerationTest::floatMessageTest()
{
    const char *propertyName = "testFieldFloat";
    SimpleFloatMessage test;
    int index = SimpleFloatMessage::staticPropertyOrdering.indexOfFieldNumber(7);
    int propertyNumber = SimpleFloatMessage::staticPropertyOrdering.propertyIndex(index); //See simpletest.proto
    QCOMPARE(SimpleFloatMessage::staticMetaObject.property(propertyNumber).typeId(), QMetaType::Float);
    QCOMPARE(SimpleFloatMessage::staticMetaObject.property(propertyNumber).name(),
             "testFieldFloat"_L1);

    float assignedValue = 1.55f;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<float>(assignedValue)));
    QCOMPARE(test.property(propertyName).toFloat(), assignedValue);
    QCOMPARE(test.testFieldFloat(), assignedValue);

    QCOMPARE(int(SimpleFloatMessage::QtProtobufFieldEnum::TestFieldFloatProtoFieldNumber), 7);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleFloatMessage");
}

void QtProtobufTypesGenerationTest::doubleMessageTest()
{
    const char *propertyName = "testFieldDouble";
    SimpleDoubleMessage test;
    int index = SimpleDoubleMessage::staticPropertyOrdering.indexOfFieldNumber(8);
    int propertyNumber = SimpleDoubleMessage::staticPropertyOrdering.propertyIndex(index); //See simpletest.proto
    QCOMPARE(SimpleDoubleMessage::staticMetaObject.property(propertyNumber).typeId(), QMetaType::Double);
    QCOMPARE(SimpleDoubleMessage::staticMetaObject.property(propertyNumber).name(), propertyName);

    double assignedValue = 0.55;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<double>(assignedValue)));
    QCOMPARE(test.property(propertyName).toDouble(), assignedValue);
    QCOMPARE(test.testFieldDouble(), assignedValue);

    QCOMPARE(int(SimpleDoubleMessage::QtProtobufFieldEnum::TestFieldDoubleProtoFieldNumber), 8);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleDoubleMessage");
}

void QtProtobufTypesGenerationTest::complexMessageTest()
{
    const char *propertyName = "testComplexField_p";
    qProtobufAssertMessagePropertyRegistered<qtprotobufnamespace::tests::ComplexMessage, qtprotobufnamespace::tests::SimpleStringMessage*>(
                2, "qtprotobufnamespace::tests::SimpleStringMessage*", propertyName);

    qtprotobufnamespace::tests::SimpleStringMessage stringMsg;
    stringMsg.setTestFieldString({ "test qwerty" });
    ComplexMessage test;
    QVERIFY(test.setProperty(
            propertyName,
            QVariant::fromValue<qtprotobufnamespace::tests::SimpleStringMessage *>(
                    new qtprotobufnamespace::tests::SimpleStringMessage(stringMsg))));
    QCOMPARE(*(test.property(propertyName)
                       .value<qtprotobufnamespace::tests::SimpleStringMessage *>()),
             stringMsg);
    QCOMPARE(test.testComplexField(), stringMsg);

    QProtobufMessagePointer rawObject(
            QProtobufMessage::constructByName("qtprotobufnamespace.tests.ComplexMessage"));
    auto *rawMessage = reinterpret_cast<qtprotobufnamespace::tests::ComplexMessage*>(rawObject.get());
    QVERIFY(rawMessage);
    QCOMPARE(rawMessage->testFieldInt(), 0);
    qtprotobufnamespace::tests::SimpleStringMessage embeddedStringMessage =
            rawMessage->testComplexField();
    QCOMPARE(embeddedStringMessage.testFieldString(), QString());
}

void QtProtobufTypesGenerationTest::bytesMessageTest()
{
    const char *propertyName = "testFieldBytes";
    SimpleBytesMessage test;
    int index = SimpleBytesMessage::staticPropertyOrdering.indexOfFieldNumber(1);
    int propertyNumber = SimpleBytesMessage::staticPropertyOrdering.propertyIndex(index); //See simpletest.proto
    QCOMPARE(SimpleBytesMessage::staticMetaObject.property(propertyNumber).typeId(), QMetaType::QByteArray);
    QCOMPARE(SimpleBytesMessage::staticMetaObject.property(propertyNumber).name(), propertyName);
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QByteArray>("\x01\x02\x03\x04\x05")));
    QCOMPARE(test.property(propertyName).toByteArray(), QByteArray("\x01\x02\x03\x04\x05"));
    QCOMPARE(test.testFieldBytes(), QByteArray("\x01\x02\x03\x04\x05"));
    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.SimpleBytesMessage");
}

void QtProtobufTypesGenerationTest::oneofMessageEmptyTest()
{
    OneofMessage test;
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(!test.hasTestOneofComplexField());
    QCOMPARE(test.testOneofField(), OneofMessage::TestOneofFields::UninitializedField);
}

void QtProtobufTypesGenerationTest::oneofMessageTest()
{
    OneofMessage test;

    test.setTestOneofFieldInt(10);
    QVERIFY(test.hasTestOneofFieldInt());
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofSecondComplexField());
    QCOMPARE(test.testOneofFieldInt(), 10);
    QCOMPARE(test.testOneofField(), OneofMessage::TestOneofFields::TestOneofFieldInt);

    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("Qt Test");
    complexField.setTestFieldInt(20);
    complexField.setTestComplexField(stringField);
    test.setTestOneofComplexField(complexField);
    QVERIFY(!test.hasTestOneofFieldInt());
    QVERIFY(test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofSecondComplexField());
    QCOMPARE(test.testOneofComplexField(), complexField);
    QCOMPARE(test.testOneofField(), OneofMessage::TestOneofFields::TestOneofComplexField);
}

void QtProtobufTypesGenerationTest::oneofMessageCopyComplexValueTest()
{
    OneofMessage test;
    ComplexMessage complexField;
    SimpleStringMessage stringField;
    stringField.setTestFieldString("Qt Test");
    complexField.setTestFieldInt(20);
    complexField.setTestComplexField(stringField);
    test.setTestOneofComplexField(complexField);
    QVERIFY(test.hasTestOneofComplexField());
    QVERIFY(!test.hasTestOneofSecondComplexField());
    QCOMPARE(test.testOneofComplexField(), complexField);
    QCOMPARE(test.testOneofField(), OneofMessage::TestOneofFields::TestOneofComplexField);

    test.setTestOneofSecondComplexField(test.testOneofComplexField());
    QVERIFY(!test.hasTestOneofComplexField());
    QVERIFY(test.hasTestOneofSecondComplexField());
    QCOMPARE(test.testOneofSecondComplexField(), complexField);
    QCOMPARE(test.testOneofField(), OneofMessage::TestOneofFields::TestOneofSecondComplexField);
}

void QtProtobufTypesGenerationTest::assignmentOperatorTest()
{
    const char *propertyName = "testFieldInt";
    qtprotobufnamespace::tests::SimpleIntMessage test;
    qtprotobufnamespace::tests::SimpleIntMessage test2;
    test2.setTestFieldInt(35);

    test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int32>(15));
    test.setTestFieldInt(25);
    test = test2;
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wself-assign-overloaded")
    test = test;
    QT_WARNING_POP
    test = test2;
    QCOMPARE(test2.testFieldInt(), test.testFieldInt());
}

void QtProtobufTypesGenerationTest::moveOperatorTest()
{
    const char *propertyName = "testFieldInt";
    qtprotobufnamespace::tests::SimpleIntMessage test;
    qtprotobufnamespace::tests::SimpleIntMessage test2;
    test2.setTestFieldInt(25);
    test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int32>(15));
    test = std::move(test2);
    QCOMPARE(test.testFieldInt(), 25);

    qtprotobufnamespace::tests::SimpleIntMessage test3;
    test3.setTestFieldInt(35);
    qtprotobufnamespace::tests::SimpleIntMessage test4(std::move(test3));
    QCOMPARE(test4.testFieldInt(), 35);
}

void QtProtobufTypesGenerationTest::rvalueSettersTest()
{
    {
        // String and ComplexMessage tests
        ComplexMessage complexField;
        SimpleStringMessage stringField;
        stringField.setTestFieldString("Value1");
        complexField.setTestComplexField(std::move(stringField));

        stringField = complexField.testComplexField();
        auto stringFieldCopy = complexField.testComplexField();
        stringFieldCopy.setTestFieldString("Value2");
        complexField.setTestComplexField(std::move(stringFieldCopy));
        ComplexMessage complexFieldCopy = complexField;

        QCOMPARE_EQ(stringField.testFieldString(), "Value1");
        QCOMPARE_EQ(complexField.testComplexField().testFieldString(), "Value2");
        QCOMPARE_EQ(complexField, complexFieldCopy);

        stringFieldCopy = complexField.testComplexField();
        SimpleStringMessage newStringField;
        newStringField.setTestFieldString("Value3");
        complexField.setTestComplexField(std::move(newStringField));
        QCOMPARE_NE(complexField, complexFieldCopy);
    }

    {
        // BytesMessage test
        SimpleBytesMessage bytesMsg;
        bytesMsg.setTestFieldBytes("\x01\x02\x03\x04\x05");
        SimpleBytesMessage bytesMsgCopy = bytesMsg;
        QCOMPARE_EQ(bytesMsg.testFieldBytes(), "\x01\x02\x03\x04\x05");
        QCOMPARE_EQ(bytesMsg, bytesMsgCopy);

        bytesMsg.setTestFieldBytes("\x01\x02\x03\x04\x05\x05");
        QCOMPARE_NE(bytesMsg.testFieldBytes(), "\x01\x02\x03\x04\x05");
        QCOMPARE_NE(bytesMsg, bytesMsgCopy);
    }
}

void QtProtobufTypesGenerationTest::rvalueOneOfSettersTest()
{
    // OneOf test
    OneofMessage oneOfField;
    QVERIFY(!oneOfField.hasTestOneofComplexField());
    QVERIFY(!oneOfField.hasTestOneofSecondComplexField());
    QVERIFY(!oneOfField.hasSecondComplexField());
    QVERIFY(!oneOfField.hasSecondSecondComplexField());

    ComplexMessage complexField;
    {
        SimpleStringMessage stringField;
        stringField.setTestFieldString("Value1");
        complexField.setTestComplexField(std::move(stringField));

        ComplexMessage complexFieldCopy = complexField;
        oneOfField.setTestOneofComplexField(std::move(complexFieldCopy));
        OneofMessage oneOfFieldCopy = oneOfField;

        QVERIFY(oneOfField.hasTestOneofComplexField());
        QCOMPARE_EQ(oneOfField, oneOfFieldCopy);
        QCOMPARE_EQ(oneOfField.testOneofComplexField().testComplexField().testFieldString(),
                    "Value1");
        QVERIFY(!oneOfField.hasTestOneofSecondComplexField());
        QVERIFY(!oneOfField.hasSecondComplexField());
        QVERIFY(!oneOfField.hasSecondSecondComplexField());

        oneOfField.setTestOneofComplexField(ComplexMessage());
        QCOMPARE_NE(oneOfField, oneOfFieldCopy);
        QCOMPARE_NE(oneOfField.testOneofComplexField().testComplexField().testFieldString(),
                    "Value1");
    }
    {
        SimpleStringMessage stringField;
        stringField.setTestFieldString("Value2");
        complexField.setTestComplexField(std::move(stringField));

        ComplexMessage complexFieldCopy = complexField;
        oneOfField.setTestOneofSecondComplexField(std::move(complexFieldCopy));
        OneofMessage oneOfFieldCopy = oneOfField;

        QVERIFY(!oneOfField.hasTestOneofComplexField());
        QVERIFY(oneOfField.hasTestOneofSecondComplexField());
        QCOMPARE_EQ(oneOfField, oneOfFieldCopy);
        QCOMPARE_EQ(oneOfField.testOneofSecondComplexField().testComplexField().testFieldString(),
                    "Value2");
        QVERIFY(!oneOfField.hasSecondComplexField());
        QVERIFY(!oneOfField.hasSecondSecondComplexField());

        oneOfField.setTestOneofSecondComplexField(ComplexMessage());
        QCOMPARE_NE(oneOfField, oneOfFieldCopy);
        QCOMPARE_NE(oneOfField.testOneofSecondComplexField().testComplexField().testFieldString(),
                    "Value2");
    }

    {
        SimpleStringMessage stringField;
        stringField.setTestFieldString("Value3");
        complexField.setTestComplexField(std::move(stringField));

        ComplexMessage complexFieldCopy = complexField;
        oneOfField.setSecondComplexField(std::move(complexFieldCopy));
        OneofMessage oneOfFieldCopy = oneOfField;

        QCOMPARE_EQ(oneOfField, oneOfFieldCopy);
        QVERIFY(!oneOfField.hasTestOneofComplexField());
        QVERIFY(oneOfField.hasTestOneofSecondComplexField());
        QCOMPARE_NE(oneOfField.testOneofSecondComplexField().testComplexField().testFieldString(),
                    "Value2");
        QVERIFY(oneOfField.hasSecondComplexField());
        QCOMPARE_EQ(oneOfField.secondComplexField().testComplexField().testFieldString(),
                    "Value3");
        QVERIFY(!oneOfField.hasSecondSecondComplexField());

        oneOfField.setSecondComplexField(ComplexMessage());
        QCOMPARE_NE(oneOfField, oneOfFieldCopy);
        QCOMPARE_NE(oneOfField.secondComplexField().testComplexField().testFieldString(),
                    "Value3");
    }

    {
        SimpleStringMessage stringField;
        stringField.setTestFieldString("Value4");
        complexField.setTestComplexField(std::move(stringField));

        ComplexMessage complexFieldCopy = complexField;
        oneOfField.setSecondSecondComplexField(std::move(complexFieldCopy));
        OneofMessage oneOfFieldCopy = oneOfField;

        QCOMPARE_EQ(oneOfField, oneOfFieldCopy);
        QVERIFY(!oneOfField.hasTestOneofComplexField());
        QVERIFY(oneOfField.hasTestOneofSecondComplexField());
        QCOMPARE_NE(oneOfField.testOneofSecondComplexField().testComplexField().testFieldString(),
                    "Value2");
        QVERIFY(!oneOfField.hasSecondComplexField());
        QVERIFY(oneOfField.hasSecondSecondComplexField());
        QCOMPARE_EQ(oneOfField.secondSecondComplexField().testComplexField().testFieldString(),
                    "Value4");

        oneOfField.setSecondSecondComplexField(ComplexMessage());
        QCOMPARE_NE(oneOfField, oneOfFieldCopy);
        QCOMPARE_NE(oneOfField.secondSecondComplexField().testComplexField().testFieldString(),
                    "Value4");
    }
}

void QtProtobufTypesGenerationTest::mutableGetterTest()
{
    ComplexMessage complexMsg;

    const auto MutableGetterValue = "Value from mutable getter"_L1;
    complexMsg.mutTestComplexField().setTestFieldString(MutableGetterValue);
    QCOMPARE_EQ(MutableGetterValue, complexMsg.testComplexField().testFieldString());

    const auto InitialFieldValue = "Value from standalone field";
    SimpleStringMessage complexFieldValue;
    complexFieldValue.setTestFieldString(InitialFieldValue);

    complexMsg.setTestComplexField(complexFieldValue);
    QCOMPARE_EQ(InitialFieldValue, complexMsg.testComplexField().testFieldString());

    // Reset the field and ensure that it didn't affect the copied standalone
    // field value.
    complexMsg.mutTestComplexField().setTestFieldString(MutableGetterValue);
    QCOMPARE_EQ(MutableGetterValue, complexMsg.testComplexField().testFieldString());
    QCOMPARE_EQ(InitialFieldValue, complexFieldValue.testFieldString());

    // Make a shared copy of the complexMsg
    ComplexMessage complexMsgCopy(complexMsg);
    QCOMPARE_EQ(MutableGetterValue, complexMsgCopy.testComplexField().testFieldString());
    QCOMPARE_EQ(complexMsg.testComplexField().testFieldString(),
        complexMsgCopy.testComplexField().testFieldString());

    // Ensure that mutable getter detaches and the new value is not applied to
    // to a shared copy.
    const auto NewMutableGetterValue = "New value from mutable getter"_L1;
    complexMsg.mutTestComplexField().setTestFieldString(NewMutableGetterValue);
    QCOMPARE_EQ(MutableGetterValue, complexMsgCopy.testComplexField().testFieldString());
    QCOMPARE_EQ(NewMutableGetterValue, complexMsg.testComplexField().testFieldString());
}

void QtProtobufTypesGenerationTest::invalidMessageConstructorTest()
{
    QProtobufMessagePointer message(QProtobufMessage::constructByName(
            "qtprotobufnamespace.tests.InvalidMessageConstructorTestNotExists"));
    QCOMPARE(message, nullptr);
}


QTEST_MAIN(QtProtobufTypesGenerationTest)
#include "tst_protobuf_basictypes.moc"
