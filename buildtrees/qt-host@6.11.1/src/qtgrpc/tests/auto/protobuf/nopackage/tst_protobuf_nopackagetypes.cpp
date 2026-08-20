// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "nopackage.qpb.h"
#include "nopackageexternal.qpb.h"

#include <QMetaProperty>
#include <QSignalSpy>
#include <QTest>

#include <qtprotobuftestscommon.h>

class QtProtobufNoPackageTypesGenerationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void noPackageEmptyMessageMessageTest();
    void noPackageSimpleIntMessageTest();
    void noPackageEnumTest();
    void noPackageExternalTest();
    void noPackageMessageTest();
};

void QtProtobufNoPackageTypesGenerationTest::noPackageEmptyMessageMessageTest()
{
    QCOMPARE(::EmptyMessage::staticPropertyOrdering.fieldCount(), 0);
    QCOMPARE(::EmptyMessage::staticMetaObject.propertyCount(), 0);
}

void QtProtobufNoPackageTypesGenerationTest::noPackageSimpleIntMessageTest()
{
    const char *propertyName = "testFieldInt";
    qProtobufAssertMessagePropertyRegistered<::SimpleIntMessage, QtProtobuf::int32>(1, "QtProtobuf::int32", propertyName);

    ::SimpleIntMessage test;
    QVERIFY(test.setProperty(propertyName, QVariant::fromValue<QtProtobuf::int32>(1)));
    QCOMPARE(test.property(propertyName).value<QtProtobuf::int32>(), 1);
    QCOMPARE(test.testFieldInt(), 1);

    QCOMPARE(int(::SimpleIntMessage::QtProtobufFieldEnum::TestFieldIntProtoFieldNumber), 1);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(), "SimpleIntMessage");
}

void QtProtobufNoPackageTypesGenerationTest::noPackageEnumTest()
{
    QVERIFY(::TestEnumGadget::staticMetaObject.enumeratorCount() > 0);
    QMetaEnum testEnum;
    for (int i = 0; i < ::TestEnumGadget::staticMetaObject.enumeratorCount(); i++) {
        QMetaEnum tmp = ::TestEnumGadget::staticMetaObject.enumerator(i);
        if (QString(tmp.name()) == QString("TestEnum")) {
            testEnum = tmp;
            break;
        }
    }
    QCOMPARE(testEnum.keyCount(), 4);
    QVERIFY(testEnum.isValid());
    QCOMPARE(testEnum.key(0), "LOCAL_ENUM_VALUE0");
    QCOMPARE(testEnum.key(1), "LOCAL_ENUM_VALUE1");
    QCOMPARE(testEnum.key(2), "LOCAL_ENUM_VALUE2");
    QCOMPARE(testEnum.key(3), "LOCAL_ENUM_VALUE3");

    QCOMPARE(testEnum.value(0), 0);
    QCOMPARE(testEnum.value(1), 1);
    QCOMPARE(testEnum.value(2), 2);
    QCOMPARE(testEnum.value(3), 5);
}

void QtProtobufNoPackageTypesGenerationTest::noPackageExternalTest()
{
    const char *propertyName = "testField_p";
    qProtobufAssertMessagePropertyRegistered<NoPackageExternalMessage, SimpleIntMessageExt*>(1, "SimpleIntMessageExt*", propertyName);

    SimpleIntMessageExt intMsg;
    intMsg.setTestFieldInt(42);

    NoPackageExternalMessage test;
    QVERIFY(test.setProperty(
            propertyName,
            QVariant::fromValue<SimpleIntMessageExt *>(new SimpleIntMessageExt(intMsg))));
    QCOMPARE(test.property(propertyName).value<SimpleIntMessageExt*>()->testFieldInt(), 42);
    QCOMPARE(test.testField().testFieldInt(), 42);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(), "NoPackageExternalMessage");
}

void QtProtobufNoPackageTypesGenerationTest::noPackageMessageTest()
{
    const char *propertyName = "testField_p";
    qProtobufAssertMessagePropertyRegistered<NoPackageMessage, SimpleIntMessage*>(1, "SimpleIntMessage*", propertyName);

    SimpleIntMessage intMsg;
    intMsg.setTestFieldInt(42);

    NoPackageMessage test;
    QVERIFY(test.setProperty(
            propertyName, QVariant::fromValue<SimpleIntMessage *>(new SimpleIntMessage(intMsg))));
    QCOMPARE(test.property(propertyName).value<SimpleIntMessage*>()->testFieldInt(), 42);
    QCOMPARE(test.testField().testFieldInt(), 42);
    QCOMPARE(test.staticPropertyOrdering.messageFullName(), "NoPackageMessage");
}

QTEST_MAIN(QtProtobufNoPackageTypesGenerationTest)
#include "tst_protobuf_nopackagetypes.moc"
