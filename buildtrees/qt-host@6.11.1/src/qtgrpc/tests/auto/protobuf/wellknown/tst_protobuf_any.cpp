// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "anymessages.qpb.h"

#include <QtProtobuf/qprotobufserializer.h>

#include <QMetaProperty>
#include <QSignalSpy>
#include <QtTest/QtTest>

class tst_protobuf_any : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void defaultConstructed();
    void simpleMessage();
    void anyMessage_data();
    void anyMessage();
    void repeatedAnyMessage();
    void twoAnyMessage_data();
    void twoAnyMessage();
    void fromMessage();
};

using namespace qtproto::tests;
using namespace Qt::StringLiterals;

void tst_protobuf_any::defaultConstructed()
{
    QProtobufSerializer serializer;

    AnyMessage message;
    // Serialize default-constructed message...
    QByteArray serialized = message.serialize(&serializer);
    QCOMPARE_EQ(serialized.toHex(), ""_ba);

    AnyMessage message2;
    // ... and then deserialize it. They should be equal.
    message2.deserialize(&serializer, serialized);
    QCOMPARE_EQ(message2, message);
}

void tst_protobuf_any::simpleMessage()
{
    QProtobufSerializer serializer;

    SimpleMessage payload;
    payload.setI(42);

    AnyMessage message;
    message.setField(QtProtobuf::Any::fromMessage(&serializer, payload));
    QByteArray serialized = message.serialize(&serializer);
    QCOMPARE_EQ(serialized.toHex(),
                "0a380a2f747970652e676f6f676c65617069732e636f6d2f717470726f74"
                "6f2e74657374732e53696d706c654d65737361676512058092f4012a"_ba);

    AnyMessage message2;
    message2.deserialize(&serializer, serialized);

    auto result = message2.field().unpack<SimpleMessage>(&serializer);
    QVERIFY(result.has_value());
    QCOMPARE_EQ(result.value(), payload);
}

void tst_protobuf_any::anyMessage_data()
{
    QTest::addColumn<QByteArray>("input");
    // Expected output:
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("i");
    QTest::addColumn<int>("j");
    QTest::addColumn<int>("h");
    QTest::addColumn<QString>("str2");

    QByteArray data = QByteArray::fromHex("0a2b0a29747970652e676f6f676c65617069732e636f6d2f71747072"
                                          "6f746f2e74657374732e4578616d706c65");
    QTest::addRow("DefaultedMessage") << data << "" << 0 << 0 << 0 << "";

    data = QByteArray::fromHex(
            "0a420a29747970652e676f6f676c65617069732e636f6d2f717470726f746f2e74657374732e4578616d70"
            "6c6512150a0548656c6c6f1002180420062a06576f726c6421");
    QTest::addRow("HelloWorld") << data << "Hello" << 1 << 2 << 3 << "World!";

    data = QByteArray::fromHex("0a3b0a29747970652e676f6f676c65617069732e636f6d2f717470726f746f2e746"
                               "57374732e4578616d706c65120e0a024e6f2a084e756d6265727321");
    QTest::addRow("NoNumbers") << data << "No" << 0 << 0 << 0 << "Numbers!";
}

void tst_protobuf_any::anyMessage()
{
    QFETCH(QByteArray, input);

    QProtobufSerializer serializer;

    AnyMessage message;
    message.deserialize(&serializer, input);
    QCOMPARE_EQ(serializer.lastError(), QAbstractProtobufSerializer::Error::None);

    std::optional<Example> opt = message.field().unpack<Example>(&serializer);
    QVERIFY(opt.has_value());
    Example ex = std::move(opt).value();
    QTEST(ex.str(), "str");
    QTEST(ex.i(), "i");
    QTEST(ex.j(), "j");
    QTEST(ex.h(), "h");
    QTEST(ex.str2(), "str2");
}


void tst_protobuf_any::repeatedAnyMessage()
{
    // [Hello 0 0 0 World!] * 3:
    QByteArray input = QByteArray::fromHex(
            "0a3c0a29747970652e676f6f676c65617069732e636f6d2f717470726f746f2e74657374732e4578616d70"
            "6c65120f0a0548656c6c6f2a06576f726c64210a3c0a29747970652e676f6f676c65617069732e636f6d2f"
            "717470726f746f2e74657374732e4578616d706c65120f0a0548656c6c6f2a06576f726c64210a3c0a2974"
            "7970652e676f6f676c65617069732e636f6d2f717470726f746f2e74657374732e4578616d706c65120f0a"
            "0548656c6c6f2a06576f726c6421");

    QProtobufSerializer serializer;
    RepeatedAnyMessage message;
    message.deserialize(&serializer, input);

    QCOMPARE_EQ(message.anys().size(), 3);
    for (const QtProtobuf::Any &any : message.anys()) {
        std::optional<Example> opt = any.unpack<Example>(&serializer);
        QVERIFY(opt.has_value());
        Example ex = std::move(opt).value();
        QCOMPARE_EQ(ex.str(), "Hello");
        QCOMPARE_EQ(ex.i(), 0);
        QCOMPARE_EQ(ex.j(), 0);
        QCOMPARE_EQ(ex.h(), 0);
        QCOMPARE_EQ(ex.str2(), "World!");
    }

    input = message.serialize(&serializer);
    QCOMPARE_GT(input.size(), 0);
    // `input` may have changed order so we're not going to compare it, but
    // let's try to deserialize it again
    RepeatedAnyMessage message2;
    message2.deserialize(&serializer, input);
    if (serializer.lastError() != QAbstractProtobufSerializer::Error::None)
        QFAIL(qPrintable(serializer.lastErrorString()));
    QCOMPARE_EQ(message2.anys().size(), message.anys().size());

    const auto anys = message.anys();
    const auto anys2 = message2.anys();
    for (int i = 0; i < anys2.size(); ++i) {
        QtProtobuf::Any lhs = anys2.at(i);
        QtProtobuf::Any rhs = anys.at(i);

        QCOMPARE_EQ(lhs, rhs);
    }
}

void tst_protobuf_any::twoAnyMessage_data()
{
    QTest::addColumn<QByteArray>("serializedData");
    QTest::addColumn<bool>("nested");

    const QByteArray emptyAndHelloWorld =
            QByteArray::fromHex("123c0a29747970652e676f6f676c65617069732e636f6d2f717470726f746f2"
                                "e74657374732e4578616d706c65120f0a0548656c6c6f2a06576f726c6421");
    QTest::newRow("Empty_and_Hello_World") << emptyAndHelloWorld << false;

    const QByteArray nestedEmptyAndHelloWorld = QByteArray::fromHex(
            "0a290a27747970652e676f6f676c65617069732e636f6d2f676f6f676c652e70726f746f6275662e416e79"
            "123c0a29747970652e676f6f676c65617069732e636f6d2f717470726f746f2e74657374732e4578616d70"
            "6c65120f0a0548656c6c6f2a06576f726c6421");
    QTest::newRow("Nested_empty_and_Hello_World") << nestedEmptyAndHelloWorld << true;
}

void tst_protobuf_any::twoAnyMessage()
{
    QFETCH(const QByteArray, serializedData);
    QFETCH(const bool, nested);

    QProtobufSerializer serializer;
    TwoAnyMessage message;
    message.deserialize(&serializer, serializedData);

    std::optional<Example> opt = message.two().unpack<Example>(&serializer);
    QVERIFY(opt);
    const Example &ex = opt.value();
    QCOMPARE_EQ(ex.str(), "Hello");
    QCOMPARE_EQ(ex.str2(), "World!");
    QCOMPARE_EQ(ex.i(), 0);
    QCOMPARE_EQ(ex.j(), 0);
    QCOMPARE_EQ(ex.h(), 0);

    if (nested) {
        // The Any-message contains another Any inside:
        std::optional<QtProtobuf::Any> anyOpt = message.one().unpack<QtProtobuf::Any>(&serializer);
        QVERIFY(anyOpt);
        // But the nested Any-message is empty:
        QCOMPARE_EQ(anyOpt->value(), QByteArray());
    } else {
        std::optional<QtProtobuf::Any> nestedAny = message.one()
                                                       .unpack<QtProtobuf::Any>(&serializer);
        QVERIFY(!nestedAny); // not nested
        // and the value of the field is empty:
        QCOMPARE_EQ(message.one().value(), QByteArray());
    }
}

void tst_protobuf_any::fromMessage()
{
    QProtobufSerializer serializer;
    Example ex;
    ex.setH(242);
    AnyMessage message;
    message.setField(QtProtobuf::Any::fromMessage(&serializer, ex));
    std::optional<Example> exop = message.field().unpack<Example>(&serializer);
    QVERIFY(exop.has_value());
    QCOMPARE(exop->h(), 242);
    QCOMPARE(*exop, ex);
}

QTEST_MAIN(tst_protobuf_any)
#include "tst_protobuf_any.moc"
