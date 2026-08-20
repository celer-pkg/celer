// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "externalpackageconsumer.qpb.h"
#include "externalpackage.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>

#include <qtprotobuftestscommon.h>

class QtProtobufExternalPackageGenerationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void repeatedExternalComplexMessageTest();
    void externalEnumMessageTest();
    void externalComplexMessageTest();
    void nestedMessageTest();
};

using namespace qtprotobufnamespace::tests;

void QtProtobufExternalPackageGenerationTest::repeatedExternalComplexMessageTest()
{
    const char *propertyName = "testExternalComplexData";
    qProtobufAssertMessagePropertyRegistered<RepeatedExternalComplexMessage, QList<qtprotobufnamespace1::externaltests::ExternalComplexMessage>>(
                1, "QList<qtprotobufnamespace1::externaltests::ExternalComplexMessage>", propertyName);

    qtprotobufnamespace1::externaltests::ExternalInt32Message complexMessage;
    complexMessage.setLocalList({1, 2, 3, 4, 5});

    qtprotobufnamespace1::externaltests::ExternalComplexMessage externalMessage;
    externalMessage.setTestFieldInt(complexMessage);

    QList<qtprotobufnamespace1::externaltests::ExternalComplexMessage>complexMessageList;
    complexMessageList << externalMessage;

    RepeatedExternalComplexMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue(complexMessageList)));
    QCOMPARE(test.property(propertyName).value<QList<qtprotobufnamespace1::externaltests::ExternalComplexMessage>>(), complexMessageList);
    QCOMPARE(test.testExternalComplex(), complexMessageList);
}

void QtProtobufExternalPackageGenerationTest::externalEnumMessageTest()
{
    const char *propertyName = "externalEnum";
    qProtobufAssertMessagePropertyRegistered<
            SimpleExternalEnumMessage,
            qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::ExternalTestEnum>(
            1, "qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::ExternalTestEnum",
            propertyName);

    SimpleExternalEnumMessage test;
    QVERIFY(test.setProperty(propertyName,
                             QVariant::fromValue<qtprotobufnamespace1::externaltests::
                                                     ExternalTestEnumGadget::ExternalTestEnum>(
                                 qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::
                                     ExternalTestEnum::EXTERNAL_TEST_ENUM_VALUE4)));
    QCOMPARE(test.property(propertyName)
                 .value<qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::
                            ExternalTestEnum>(),
             qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::ExternalTestEnum::
                 EXTERNAL_TEST_ENUM_VALUE4);
    QCOMPARE(test.externalEnum(),
             qtprotobufnamespace1::externaltests::ExternalTestEnumGadget::ExternalTestEnum::
                 EXTERNAL_TEST_ENUM_VALUE4);
}

void QtProtobufExternalPackageGenerationTest::externalComplexMessageTest()
{
    const char *propertyName = "localList";
    qProtobufAssertMessagePropertyRegistered<qtprotobufnamespace1::externaltests::ExternalInt32Message, QtProtobuf::int32List>(
                1, "QtProtobuf::int32List", propertyName);

    qtprotobufnamespace1::externaltests::ExternalInt32Message test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int32List>({1, 2, 3, 4, 5})));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::int32List>(), QtProtobuf::int32List({1, 2, 3, 4, 5}));
    QCOMPARE(test.localList(), QtProtobuf::int32List({1, 2, 3, 4, 5}));
}

void QtProtobufExternalPackageGenerationTest::nestedMessageTest()
{
    qProtobufAssertMessagePropertyRegistered<NestedExternal, qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage*>(1, "qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage*", "externalNested_p");

    qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage nestedMsg;
    nestedMsg.setField(15);

    NestedExternal test;
    test.setExternalNested(nestedMsg);
    QCOMPARE(test.externalNested().field(), 15);

    nestedMsg.setField(55);

    const char *propertyName = "externalNested_p";
    QVERIFY(test.setProperty(
            propertyName,
            QVariant::fromValue<
                    qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage *>(
                    new qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage(
                            nestedMsg))));
    QCOMPARE(*(test.property(propertyName)
                       .value<qtprotobufnamespace1::externaltests::NestedFieldMessage::NestedMessage
                                      *>()),
             nestedMsg);
    QCOMPARE(test.externalNested(), nestedMsg);
}

QTEST_MAIN(QtProtobufExternalPackageGenerationTest)
#include "tst_protobuf_externalpackage.moc"
