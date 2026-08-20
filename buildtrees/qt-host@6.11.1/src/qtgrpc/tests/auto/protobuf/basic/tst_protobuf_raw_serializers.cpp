// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "basicmessages.qpb.h"

#include <QtTest/QtTest>
#include <QProtobufSerializer>

#include <limits>

class QtProtobufRawSerializersTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() { m_serializer.reset(new QProtobufSerializer); }
    void complexMessageSerializeTest();
    void complexMessageDeserializeTest();
private:
    std::unique_ptr<QProtobufSerializer> m_serializer;
};

class NonMessageQObject : public QObject
{
    Q_OBJECT
};

void QtProtobufRawSerializersTest::complexMessageSerializeTest()
{
    QProtobufMessagePointer rawMessage =
            QProtobufMessage::constructByName("qtprotobufnamespace.tests.ComplexMessage");
    m_serializer->deserialize(rawMessage.get(),
                              QByteArray::fromHex("1208320671776572747908d3ffffffffffffffff01"));
    auto *message = reinterpret_cast<qtprotobufnamespace::tests::ComplexMessage *>(rawMessage.get());
    QCOMPARE(message->testFieldInt(), -45);
    QCOMPARE(message->testComplexField().testFieldString(), QLatin1String("qwerty"));
}

void QtProtobufRawSerializersTest::complexMessageDeserializeTest()
{
    QProtobufMessagePointer rawMessage =
            QProtobufMessage::constructByName("qtprotobufnamespace.tests.ComplexMessage");
    auto *message = reinterpret_cast<qtprotobufnamespace::tests::ComplexMessage *>(rawMessage.get());
    message->setTestFieldInt(-45);
    auto stringMessage = message->testComplexField();
    stringMessage.setTestFieldString(QLatin1String("qwerty"));
    message->setTestComplexField(stringMessage);

    QByteArray buffer = m_serializer->serialize(rawMessage.get());
    QCOMPARE(buffer.toHex(), "08d3ffffffffffffffff0112083206717765727479");
}

QTEST_MAIN(QtProtobufRawSerializersTest)
#include "tst_protobuf_raw_serializers.moc"
