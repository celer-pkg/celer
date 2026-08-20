// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "enummessages.qpb.h"

#include <QProtobufJsonSerializer>
#include <QTest>

class QtProtobufEnumTypesJsonSerializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void simpleEnumMessageSerializeTest();
    void repeatedEnumMessageTest();

    void init() { m_serializer.reset(new QProtobufJsonSerializer); }

private:
    std::unique_ptr<QProtobufJsonSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;
using namespace Qt::Literals::StringLiterals;

void QtProtobufEnumTypesJsonSerializationTest::simpleEnumMessageSerializeTest()
{
    SimpleEnumMessage test;
    test.setLocalEnum(SimpleEnumMessage::LocalEnum::LOCAL_ENUM_VALUE2);
    QByteArray result = test.serialize(m_serializer.get());
    QCOMPARE(result, "{\"localEnum\":\"LOCAL_ENUM_VALUE2\"}"_ba);
}

void QtProtobufEnumTypesJsonSerializationTest::repeatedEnumMessageTest()
{
    RepeatedEnumMessage msg;

    msg.setLocalEnumList({ RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE0,
                           RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE1,
                           RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE2,
                           RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE1,
                           RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE2,
                           RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE3 });
    QByteArray result = msg.serialize(m_serializer.get());
    QCOMPARE(result,
             "{\"localEnumList\":[\"LOCAL_ENUM_VALUE0\",\"LOCAL_ENUM_VALUE1\","
             "\"LOCAL_ENUM_VALUE2\",\"LOCAL_ENUM_VALUE1\",\"LOCAL_ENUM_VALUE2\","
             "\"LOCAL_ENUM_VALUE3\"]}"_ba);
    msg.setLocalEnumList({});
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, "{}"_ba);
}

QTEST_MAIN(QtProtobufEnumTypesJsonSerializationTest)
#include "tst_protobuf_serialization_json_enumtypes.moc"
