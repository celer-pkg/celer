// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "extranamespace.qpb.h"

#include <QMetaProperty>

#include <QTest>
#include <qtprotobuftestscommon.h>

using namespace MyTestNamespace::qtprotobufnamespace::tests;

class QtProtobufExtraNamespaceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void emptyMessageTest();
    void complexMessageTest();
};

void QtProtobufExtraNamespaceTest::emptyMessageTest()
{
    QCOMPARE(EmptyMessage::staticPropertyOrdering.fieldCount(), 0);
    QCOMPARE(EmptyMessage::staticMetaObject.propertyCount(), 0);
}

void QtProtobufExtraNamespaceTest::complexMessageTest()
{
    const char *propertyName = "testComplexField_p";
    qProtobufAssertMessagePropertyRegistered<ComplexMessage, SimpleStringMessage*>(
                2, "MyTestNamespace::qtprotobufnamespace::tests::SimpleStringMessage*", propertyName);

    SimpleStringMessage stringMsg;
    stringMsg.setTestFieldString({ "test qwerty" });

    ComplexMessage test;
    QVERIFY(test.setProperty(
            propertyName,
            QVariant::fromValue<SimpleStringMessage *>(new SimpleStringMessage(stringMsg))));
    QCOMPARE(*(test.property(propertyName).value<SimpleStringMessage *>()), stringMsg);
    QCOMPARE(test.testComplexField(), stringMsg);
}

QTEST_MAIN(QtProtobufExtraNamespaceTest)
#include "tst_protobuf_extranamespace.moc"
