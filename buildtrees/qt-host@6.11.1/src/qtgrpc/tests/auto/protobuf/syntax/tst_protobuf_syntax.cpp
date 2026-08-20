// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "syntax.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>
#include <QDebug>

#include <qtprotobuftestscommon.h>

class QtProtobufSyntaxTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void underscoresTest();
    void upperCaseTest();
    void reservedTest();
    void reservedUpperCaseTest();
    void reservedEnumTest();
    void lowerCaseEnumTest();
    void upperCaseEnumTest();

    void mutableGetterConflicts();
};

using namespace qtprotobufnamespace::tests;

void QtProtobufSyntaxTest::underscoresTest()
{
    //Sanity compilation checks
    Message_Uderscore_name msg1;
    MessageUderscorename msg2;
    MessageUnderscoreField msg3;
    PriorMessageUnderscoreField msg4;
    FollowingMessageUnderscoreField msg5;
    CombinedMessageUnderscoreField msg6;

    qProtobufAssertMessagePropertyRegistered<MessageUnderscoreField, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "underScoreMessageField");
    qProtobufAssertMessagePropertyRegistered<PriorMessageUnderscoreField, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "underScoreMessageField");
    qProtobufAssertMessagePropertyRegistered<PriorMessageUnderscoreField, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "underScoreMessageField");
    qProtobufAssertMessagePropertyRegistered<FollowingMessageUnderscoreField , QtProtobuf::sint32>(1, "QtProtobuf::sint32", "underScoreMessageField");
    qProtobufAssertMessagePropertyRegistered<CombinedMessageUnderscoreField , QtProtobuf::sint32>(1, "QtProtobuf::sint32", "underScoreMessageField");
}

void QtProtobufSyntaxTest::upperCaseTest()
{
    qProtobufAssertMessagePropertyRegistered<MessageUpperCase, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "testField");
}

void QtProtobufSyntaxTest::reservedTest()
{
    qProtobufAssertMessagePropertyRegistered<MessageReserved, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "import_proto");
    qProtobufAssertMessagePropertyRegistered<MessageReserved, QtProtobuf::sint32>(2, "QtProtobuf::sint32", "property_proto");
    qProtobufAssertMessagePropertyRegistered<MessageReserved, QtProtobuf::sint32>(3, "QtProtobuf::sint32", "id_proto");
}

void QtProtobufSyntaxTest::reservedUpperCaseTest()
{
    qProtobufAssertMessagePropertyRegistered<MessageUpperCaseReserved, QtProtobuf::sint32>(1, "QtProtobuf::sint32", "import_proto");
    qProtobufAssertMessagePropertyRegistered<MessageUpperCaseReserved, QtProtobuf::sint32>(2, "QtProtobuf::sint32", "property_proto");
    qProtobufAssertMessagePropertyRegistered<MessageUpperCaseReserved, QtProtobuf::sint32>(3, "QtProtobuf::sint32", "id_proto");
}

void QtProtobufSyntaxTest::reservedEnumTest()
{
    const auto &metaObject = MessageEnumReserved_QtProtobufNested::staticMetaObject;
    QVERIFY(metaObject.enumeratorCount() > 0);
    QMetaEnum simpleEnum;
    for (int i = 0; i < metaObject.enumeratorCount(); i++) {
        QMetaEnum tmp = metaObject.enumerator(i);
        if (QString(tmp.name()) == QString("ReservedEnum")) {
            simpleEnum = tmp;
            break;
        }
    }
    QVERIFY(simpleEnum.isValid());
    QCOMPARE(simpleEnum.key(0), "Import");
    QCOMPARE(simpleEnum.key(1), "Property");
    QCOMPARE(simpleEnum.key(2), "Id");

    QCOMPARE(simpleEnum.value(0), 0);
    QCOMPARE(simpleEnum.value(1), 1);
    QCOMPARE(simpleEnum.value(2), 2);
}

void QtProtobufSyntaxTest::lowerCaseEnumTest()
{
    const auto &metaObject = MessageEnumReserved_QtProtobufNested::staticMetaObject;
    QVERIFY(metaObject.enumeratorCount() > 0);
    QMetaEnum simpleEnum;
    for (int i = 0; i < metaObject.enumeratorCount(); i++) {
        QMetaEnum tmp = metaObject.enumerator(i);
        if (QString(tmp.name()) == QString("LowerCaseEnum")) {
            simpleEnum = tmp;
            break;
        }
    }
    QVERIFY(simpleEnum.isValid());
    QCOMPARE(simpleEnum.key(0), "enumValue0");
    QCOMPARE(simpleEnum.key(1), "enumValue1");
    QCOMPARE(simpleEnum.key(2), "enumValue2");
}

void QtProtobufSyntaxTest::upperCaseEnumTest()
{
    const auto &metaObject = MessageEnumReserved_QtProtobufNested::staticMetaObject;
    QVERIFY(metaObject.enumeratorCount() > 0);
    QMetaEnum simpleEnum;
    for (int i = 0; i < metaObject.enumeratorCount(); i++) {
        QMetaEnum tmp = metaObject.enumerator(i);
        if (QString(tmp.name()) == QString("UpperCaseEnum")) {
            simpleEnum = tmp;
            break;
        }
    }
    QVERIFY(simpleEnum.isValid());
    QCOMPARE(simpleEnum.key(0), "EnumValue0");
    QCOMPARE(simpleEnum.key(1), "EnumValue1");
    QCOMPARE(simpleEnum.key(2), "EnumValue2");
}

void QtProtobufSyntaxTest::mutableGetterConflicts()
{
    NameClashingMutableGetters msg;
    // Set 'data' field for the 'field' field using the mutable getter
    msg.mutField().setData(1);

    // Set 'data' field for the 'mutField' field using the mutable getter
    msg.mutMutField().setData(2);

    // Access the immutable 'field' field
    QVERIFY(msg.field().data() == 1);
    // Access the mutable 'field' field
    QVERIFY(msg.mutField().data() == 1);

    // Access the immutable 'mutField' field
    QVERIFY(std::as_const(msg).mutField().data() == 2);

    // Ensure we use the correct value in serialization
    QVERIFY(!msg.property("mutField_p").isNull());
    QVERIFY(msg.property("mutField_p").value<MutFieldMessage *>()->data() == 2);
}

QTEST_MAIN(QtProtobufSyntaxTest)
#include "tst_protobuf_syntax.moc"
