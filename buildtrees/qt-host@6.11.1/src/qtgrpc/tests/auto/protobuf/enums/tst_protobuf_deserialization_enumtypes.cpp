// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2022 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "enummessages.qpb.h"

#include <QTest>
#include <QProtobufSerializer>

class QtProtobufEnumTypesDeserializationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        m_serializer.reset(new QProtobufSerializer);
    }
    void simpleEnumMessageDeserializeTest();
    void repeatedEnumMessageTest();
private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

using namespace qtprotobufnamespace::tests;

void QtProtobufEnumTypesDeserializationTest::simpleEnumMessageDeserializeTest()
{
    SimpleEnumMessage test;
    test.deserialize(m_serializer.get(), QByteArray::fromHex("0803"));
    QCOMPARE(test.localEnum(), SimpleEnumMessage::LocalEnum::LOCAL_ENUM_VALUE3);
}

void QtProtobufEnumTypesDeserializationTest::repeatedEnumMessageTest()
{
    RepeatedEnumMessage msg;

    msg.deserialize(m_serializer.get(), QByteArray());
    QVERIFY(msg.localEnumList().isEmpty());

    msg.deserialize(m_serializer.get(), QByteArray::fromHex("0a06000102010203"));
    const auto expected = QList<RepeatedEnumMessage::LocalEnum>{
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE0,
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE1,
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE2,
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE1,
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE2,
        RepeatedEnumMessage::LocalEnum::LOCAL_ENUM_VALUE3
    };
    QCOMPARE_EQ(msg.localEnumList(), expected);
}

QTEST_MAIN(QtProtobufEnumTypesDeserializationTest)
#include "tst_protobuf_deserialization_enumtypes.moc"
