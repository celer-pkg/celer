// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QObject>

#include <QtProtobuf/qprotobufjsonserializer.h>
#include <QtProtobuf/qprotobufserializer.h>
#include <QtProtobuf/qprotobufregistration.h>

#include <optional.qpb.h>

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

class QtProtobufOptionalSerializationTest : public QObject
{
    Q_OBJECT

    enum Format { Protobuf, JSON };

private Q_SLOTS:
    void initTestCase_data();
    void initTestCase() { }
    void init();

    void serializeEmptyOptional();
    void serializeOptionalInt_data();
    void serializeOptionalInt();
    void serializeOptionalBool_data();
    void serializeOptionalBool();
    void serializeOptionalString_data();
    void serializeOptionalString();
    void serializeOptionalBytes_data();
    void serializeOptionalBytes();
    void serializeOptionalMessage_data();
    void serializeOptionalMessage();

private:
    std::shared_ptr<QAbstractProtobufSerializer> m_serializer;
    Format m_format;
};

void QtProtobufOptionalSerializationTest::initTestCase_data()
{
    QTest::addColumn<QtProtobufOptionalSerializationTest::Format>("format");
    QTest::addColumn<std::shared_ptr<QAbstractProtobufSerializer>>("serializer");

    QTest::newRow("Protobuf")
        << Protobuf << std::shared_ptr<QAbstractProtobufSerializer>(new QProtobufSerializer);
    QTest::newRow("JSON")
        << JSON << std::shared_ptr<QAbstractProtobufSerializer>(new QProtobufJsonSerializer);
}

void QtProtobufOptionalSerializationTest::init()
{
    QFETCH_GLOBAL(QtProtobufOptionalSerializationTest::Format, format);
    m_format = format;
    QFETCH_GLOBAL(std::shared_ptr<QAbstractProtobufSerializer>, serializer);
    m_serializer = std::move(serializer);
}

void QtProtobufOptionalSerializationTest::serializeEmptyOptional()
{
    qtprotobufnamespace::optional::tests::OptionalMessage msg1;
    QCOMPARE(msg1.serialize(m_serializer.get()), m_format == Protobuf ? ""_ba : "{}"_ba);
}

void QtProtobufOptionalSerializationTest::serializeOptionalInt_data()
{
    QTest::addColumn<QtProtobuf::sint32>("value");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<QByteArray>("expectedDataJson");

    QTest::newRow("Zero") << 0 << "1000"_ba
                          << "{\"testFieldOpt\":0}"_ba;
    QTest::newRow("Valid") << 84 << "10a801"_ba
                           << "{\"testFieldOpt\":84}"_ba;
}

void QtProtobufOptionalSerializationTest::serializeOptionalInt()
{
    QFETCH(const QtProtobuf::sint32, value);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const QByteArray, expectedDataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldOpt(value);
    QByteArray result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);

    msg.clearTestFieldOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);
}

void QtProtobufOptionalSerializationTest::serializeOptionalBool_data()
{
    QTest::addColumn<bool>("value");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<QByteArray>("expectedDataJson");

    QTest::newRow("False") << false << "2000"_ba
                           << "{\"testFieldBoolOpt\":false}"_ba;
    QTest::newRow("True") << true << "2001"_ba
                          << "{\"testFieldBoolOpt\":true}"_ba;
}

void QtProtobufOptionalSerializationTest::serializeOptionalBool()
{
    QFETCH(const bool, value);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const QByteArray, expectedDataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldBoolOpt(value);
    QByteArray result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);

    msg.clearTestFieldBoolOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);
}

void QtProtobufOptionalSerializationTest::serializeOptionalString_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<QByteArray>("expectedDataJson");
    QTest::newRow("EmptyString") << QString::fromLatin1(""_L1) << "4200"_ba
                                 << "{\"testFieldStringOpt\":\"\"}"_ba;
    QTest::newRow("Valid") << QString::fromLatin1("qwerty"_L1) << "4206717765727479"_ba
                           << "{\"testFieldStringOpt\":\"qwerty\"}"_ba;
}

void QtProtobufOptionalSerializationTest::serializeOptionalString()
{
    QFETCH(const QString, value);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const QByteArray, expectedDataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldStringOpt(value);
    QByteArray result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);

    msg.clearTestFieldStringOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);
}

void QtProtobufOptionalSerializationTest::serializeOptionalBytes_data()
{
    QTest::addColumn<QByteArray>("value");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<QByteArray>("expectedDataJson");

    QTest::newRow("EmptyBytes") << ""_ba
                                << "3200"_ba
                                << "{\"testFieldBytesOpt\":\"\"}"_ba;
    QTest::newRow("Valid") << "qwerty"_ba
                           << "3206717765727479"_ba
                           << "{\"testFieldBytesOpt\":\"cXdlcnR5\"}"_ba;
}

void QtProtobufOptionalSerializationTest::serializeOptionalBytes()
{
    QFETCH(const QByteArray, value);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const QByteArray, expectedDataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldBytesOpt(value);
    QByteArray result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);

    msg.clearTestFieldBytesOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);
}

void QtProtobufOptionalSerializationTest::serializeOptionalMessage_data()
{
    QTest::addColumn<qtprotobufnamespace::optional::tests::TestStringMessage>("value");
    QTest::addColumn<QByteArray>("expectedData");
    QTest::addColumn<QByteArray>("expectedDataJson");

    QTest::newRow("EmptyMessage") << qtprotobufnamespace::optional::tests::TestStringMessage()
                                  << "5200"_ba
                                  << "{\"testFieldMessageOpt\":{}}"_ba;
}

void QtProtobufOptionalSerializationTest::serializeOptionalMessage()
{
    QFETCH(const qtprotobufnamespace::optional::tests::TestStringMessage, value);
    QFETCH(const QByteArray, expectedData);
    QFETCH(const QByteArray, expectedDataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldMessageOpt(value);
    QByteArray result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);

    msg.clearTestFieldMessageOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);

    // Accessing the field of message type initializes it.
    msg.testFieldMessageOpt();
    result = msg.serialize(m_serializer.get());
    if (m_format == Protobuf)
        QCOMPARE(result.toHex(), expectedData);
    else
        QCOMPARE(result, expectedDataJson);
    msg.clearTestFieldMessageOpt();
    result = msg.serialize(m_serializer.get());
    QCOMPARE(result, m_format == Protobuf ? ""_ba : "{}"_ba);
}

QTEST_MAIN(QtProtobufOptionalSerializationTest)

#include "tst_protobuf_serialization_optional.moc"
