// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "nestedmessages.qpb.h"
#include "externalpackage.qpb.h"

#include <QTest>
#include <qtprotobuftestscommon.h>
#include <QProtobufSerializer>

using namespace qtprotobufnamespace::tests::nested;
using namespace Qt::Literals::StringLiterals;

class QtProtobufNestedTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_serializer.reset(new QProtobufSerializer);
    }
    void nestedMessageTest();
    void simpleTest();
    void serializationTest();
    void deserializationTest();
    void neighborTest();
    void nestedNoFieldsTest();
    void nestedCyclingTest();
    void unusedNestedMessageTest();
private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

void QtProtobufNestedTest::nestedMessageTest()
{
    qProtobufAssertMessagePropertyRegistered<NestedFieldMessage::NestedMessage,
            QtProtobuf::sint32>(1, "QtProtobuf::sint32", "testFieldInt");
}

void QtProtobufNestedTest::simpleTest()
{
    const char *propertyName = "nested_p";
    qProtobufAssertMessagePropertyRegistered<NestedFieldMessage, NestedFieldMessage::NestedMessage*>(2, "qtprotobufnamespace::tests::nested::NestedFieldMessage::NestedMessage*", "nested_p");

    NestedFieldMessage::NestedMessage nestedMsg;
    nestedMsg.setTestFieldInt(15);

    NestedFieldMessage test;
    test.setNested(nestedMsg);
    QCOMPARE(test.nested().testFieldInt(), 15);

    nestedMsg.setTestFieldInt(65555);
    QVERIFY(test.setProperty(propertyName,
                             QVariant::fromValue<NestedFieldMessage::NestedMessage *>(
                                     new NestedFieldMessage::NestedMessage(nestedMsg))));
    QCOMPARE(*(test.property(propertyName).value<NestedFieldMessage::NestedMessage *>()),
             nestedMsg);
    QCOMPARE(test.nested(), nestedMsg);

    qProtobufAssertMessagePropertyRegistered<NestedFieldMessage2::NestedMessageLevel1, NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*>(1, "qtprotobufnamespace::tests::nested::NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*", "nested_p");

    NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 level2;
    level2.setTestFieldInt(20);
    NestedFieldMessage2::NestedMessageLevel1 level1;
    level1.setNested(level2);
    QCOMPARE(level1.nested().testFieldInt(), 20);

    level2.setTestFieldInt(55);
    QVERIFY(level1.setProperty(
            propertyName,
            QVariant::fromValue<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>(
                    new NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2(level2))));
    QCOMPARE(*(level1.property(propertyName)
                       .value<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>()),
             level2);
    QCOMPARE(level1.nested(), level2);

    qProtobufAssertMessagePropertyRegistered<NestedFieldMessage2, NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*>(3, "qtprotobufnamespace::tests::nested::NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*", "nested2_p");

    NestedFieldMessage2 test2;

    level2.setTestFieldInt(55);
    level1.setNested(level2);
    test2.setNested1(level1);

    level2.setTestFieldInt(20);
    test2.setNested2(level2);

    QCOMPARE(test2.nested1().nested().testFieldInt(), 55);
    QCOMPARE(test2.nested2().testFieldInt(), 20);

    level2.setTestFieldInt(65);
    level1.setNested(level2);

    QVERIFY(test2.setProperty("nested1_p",
                              QVariant::fromValue<NestedFieldMessage2::NestedMessageLevel1 *>(
                                      new NestedFieldMessage2::NestedMessageLevel1(level1))));
    QCOMPARE(*(test2.property("nested1_p").value<NestedFieldMessage2::NestedMessageLevel1 *>()),
             level1);
    QCOMPARE(test2.nested1(), level1);

    level2.setTestFieldInt(75);
    QVERIFY(test2.setProperty(
            "nested2_p",
            QVariant::fromValue<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>(
                    new NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2(level2))));
    QCOMPARE(*(test2.property("nested2_p")
                       .value<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>()),
             level2);
    QCOMPARE(test2.nested2(), level2);

    QCOMPARE(test.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.nested.NestedFieldMessage");
    QCOMPARE(nestedMsg.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.nested.NestedFieldMessage.NestedMessage");
}

void QtProtobufNestedTest::serializationTest()
{
    NestedFieldMessage::NestedMessage nested;
    nested.setTestFieldInt(15);

    QByteArray result = nested.serialize(m_serializer.get());
    QCOMPARE(result.size(), 2);
    QCOMPARE(result.toHex().toStdString().c_str(), "081e");

    NestedFieldMessage test;
    test.setTestFieldInt(10);
    test.setNested(nested);

    result = test.serialize(m_serializer.get());

    QCOMPARE(QLatin1StringView(result.toHex()), "08141202081e"_L1);

    NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 level2;
    level2.setTestFieldInt(10);
    NestedFieldMessage2::NestedMessageLevel1 level1;
    level1.setNested(level2);
    result = level1.serialize(m_serializer.get());
    QCOMPARE(result.toHex().toStdString().c_str(), "0a020814");

    NestedFieldMessage2 test2;
    level2.setTestFieldInt(10);
    level1.setNested(level2);
    test2.setNested1(level1);
    level2.setTestFieldInt(15);
    test2.setNested2(level2);
    result = test2.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "12040a0208141a02081e"_L1);

    NeighborNested neigbor;
    nested.setTestFieldInt(15);
    level2.setTestFieldInt(20);
    neigbor.setNeighborNested(nested);
    neigbor.setNeighborNested2(level2);
    result = neigbor.serialize(m_serializer.get());
    QCOMPARE(QLatin1StringView(result.toHex()), "0a02081e12020828"_L1);
}

void QtProtobufNestedTest::deserializationTest()
{
    NestedFieldMessage::NestedMessage nested;

    nested.deserialize(m_serializer.get(), QByteArray::fromHex("081e"));
    QCOMPARE(nested.testFieldInt(), 15);

    NestedFieldMessage test;
    test.deserialize(m_serializer.get(), QByteArray::fromHex("1202081e0814"));
    QCOMPARE(test.nested().testFieldInt(), 15);

    NestedFieldMessage2::NestedMessageLevel1 level1;
    level1.deserialize(m_serializer.get(), QByteArray::fromHex("0a020814"));
    QCOMPARE(level1.nested().testFieldInt(), 10);

    NestedFieldMessage2 test2;
    test2.deserialize(m_serializer.get(), QByteArray::fromHex("1a02081e12040a020814"));
    QCOMPARE(test2.nested1().nested().testFieldInt(), 10);
    QCOMPARE(test2.nested2().testFieldInt(), 15);

    NeighborNested neigbor;
    neigbor.deserialize(m_serializer.get(), QByteArray::fromHex("120208280a02081e"));
    QCOMPARE(neigbor.neighborNested().testFieldInt(), 15);
    QCOMPARE(neigbor.neighborNested2().testFieldInt(), 20);
}


void QtProtobufNestedTest::neighborTest()
{
    qProtobufAssertMessagePropertyRegistered<NeighborNested, NestedFieldMessage::NestedMessage*>(1, "qtprotobufnamespace::tests::nested::NestedFieldMessage::NestedMessage*", "neighborNested_p");
    qProtobufAssertMessagePropertyRegistered<NeighborNested, NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*>(2, "qtprotobufnamespace::tests::nested::NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2*", "neighborNested2_p");

    NestedFieldMessage::NestedMessage nested;
    nested.setTestFieldInt(15);
    NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 level2;
    level2.setTestFieldInt(20);

    NeighborNested test;
    test.setNeighborNested(nested);
    test.setNeighborNested2(level2);

    QCOMPARE(test.neighborNested().testFieldInt(), 15);
    QCOMPARE(test.neighborNested2().testFieldInt(), 20);

    const char *propertyName = "neighborNested_p";

    nested.setTestFieldInt(55);
    QVERIFY(test.setProperty(propertyName,
                             QVariant::fromValue<NestedFieldMessage::NestedMessage *>(
                                     new NestedFieldMessage::NestedMessage(nested))));
    QCOMPARE(*(test.property(propertyName).value<NestedFieldMessage::NestedMessage *>()), nested);
    QCOMPARE(test.neighborNested(), nested);

    propertyName = "neighborNested2_p";

    level2.setTestFieldInt(75);
    QVERIFY(test.setProperty(
            propertyName,
            QVariant::fromValue<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>(
                    new NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2(level2))));
    QCOMPARE(*(test.property(propertyName)
                       .value<NestedFieldMessage2::NestedMessageLevel1::NestedMessageLevel2 *>()),
             level2);
    QCOMPARE(test.neighborNested2(), level2);

    QCOMPARE(level2.staticPropertyOrdering.messageFullName(),
             "qtprotobufnamespace.tests.nested.NestedFieldMessage2.NestedMessageLevel1."
             "NestedMessageLevel2");
}

void QtProtobufNestedTest::nestedNoFieldsTest()
{
    qProtobufAssertMessagePropertyRegistered<NestedNoFields::Nested, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "testFieldInt");

    NestedNoFields::Nested test;
    test.setTestFieldInt(15);
    QCOMPARE(test.testFieldInt(), 15);

    const char *propertyName = "testFieldInt";

    QVERIFY(test.setProperty(propertyName, 55));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::sint32>(), 55);
    QCOMPARE(test.testFieldInt(), 55);
}

void QtProtobufNestedTest::nestedCyclingTest()
{
    qProtobufAssertMessagePropertyRegistered<NestedCyclingA::NestedCyclingB, qtprotobufnamespace::tests::nested::NestedCyclingAA::NestedCyclingBB*>(1, "qtprotobufnamespace::tests::nested::NestedCyclingAA::NestedCyclingBB*", "testField_p");
    qProtobufAssertMessagePropertyRegistered<NestedCyclingAA::NestedCyclingBB, qtprotobufnamespace::tests::nested::NestedCyclingA::NestedCyclingB*>(1, "qtprotobufnamespace::tests::nested::NestedCyclingA::NestedCyclingB*", "testField_p");

    NestedCyclingA::NestedCyclingB test;
    NestedCyclingAA::NestedCyclingBB test2;
    test.setTestField(test2);
    test2.setTestField(test);
}

void QtProtobufNestedTest::unusedNestedMessageTest()
{
    qProtobufAssertMessagePropertyRegistered<NestedFieldMessage::UnusedNestedMessage,
                                             QtProtobuf::sint32>(1, "QtProtobuf::sint32",
                                                                 "testFieldInt");
}

QTEST_MAIN(QtProtobufNestedTest)
#include "tst_protobuf_nestedtypes.moc"
