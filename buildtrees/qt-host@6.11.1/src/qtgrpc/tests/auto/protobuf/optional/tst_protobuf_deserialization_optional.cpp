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

class QtProtobufOptionalDeserializationTest : public QObject
{
    Q_OBJECT

    enum Format { Protobuf, JSON };

private Q_SLOTS:
    void initTestCase_data();
    void initTestCase() { }
    void init();

    void deserializeEmptyOptional();
    void deserializeOptionalInt_data();
    void deserializeOptionalInt();
    void deserializeOptionalBool_data();
    void deserializeOptionalBool();
    void deserializeOptionalString_data();
    void deserializeOptionalString();
    void deserializeOptionalBytes_data();
    void deserializeOptionalBytes();
    void deserializeOptionalMessage_data();
    void deserializeOptionalMessage();
private:
    std::shared_ptr<QAbstractProtobufSerializer> m_serializer;
    Format m_format;
};

void QtProtobufOptionalDeserializationTest::initTestCase_data()
{
    QTest::addColumn<QtProtobufOptionalDeserializationTest::Format>("format");
    QTest::addColumn<std::shared_ptr<QAbstractProtobufSerializer>>("serializer");

    QTest::newRow("Protobuf")
        << Protobuf << std::shared_ptr<QAbstractProtobufSerializer>(new QProtobufSerializer);
    QTest::newRow("JSON")
        << JSON << std::shared_ptr<QAbstractProtobufSerializer>(new QProtobufJsonSerializer);
}

void QtProtobufOptionalDeserializationTest::init()
{
    QFETCH_GLOBAL(QtProtobufOptionalDeserializationTest::Format, format);
    m_format = format;
    QFETCH_GLOBAL(std::shared_ptr<QAbstractProtobufSerializer>, serializer);
    m_serializer = std::move(serializer);
}

void QtProtobufOptionalDeserializationTest::deserializeEmptyOptional()
{
    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? ""_ba : "{}"_ba);
    QVERIFY(!msg.hasTestFieldOpt());
    QVERIFY(!msg.hasTestFieldBoolOpt());
    QVERIFY(!msg.hasTestFieldMessage());
    QVERIFY(!msg.hasTestFieldBytesOpt());
    QVERIFY(!msg.hasTestFieldStringOpt());
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalInt_data()
{
    QTest::addColumn<QtProtobuf::sint32>("expected");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataJson");

    QTest::newRow("Zero") << 0 << QByteArray::fromHex("1000"_ba) << "{\"testFieldOpt\":0}"_ba;
    QTest::newRow("Valid") << 84 << QByteArray::fromHex("10a801"_ba) << "{\"testFieldOpt\":84}"_ba;
}
void QtProtobufOptionalDeserializationTest::deserializeOptionalInt()
{
    QFETCH(const QtProtobuf::sint32, expected);
    QFETCH(const QByteArray, data);
    QFETCH(const QByteArray, dataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? data : dataJson);
    QVERIFY(msg.hasTestFieldOpt());
    QCOMPARE(msg.testFieldOpt(), expected);
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalBool_data()
{
    QTest::addColumn<bool>("expected");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataJson");

    QTest::newRow("False") << false << QByteArray::fromHex("2000"_ba)
                           << "{\"testFieldBoolOpt\":false}"_ba;
    QTest::newRow("True") << true << QByteArray::fromHex("2001"_ba)
                          << "{\"testFieldBoolOpt\":true}"_ba;
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalBool()
{
    QFETCH(const bool, expected);
    QFETCH(const QByteArray, data);
    QFETCH(const QByteArray, dataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? data : dataJson);
    QVERIFY(msg.hasTestFieldBoolOpt());
    QCOMPARE(msg.testFieldBoolOpt(), expected);
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalString_data()
{
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataJson");
    QTest::newRow("EmptyString") << QString::fromLatin1(""_L1) << QByteArray::fromHex("4200"_ba)
                                 << "{\"testFieldStringOpt\":\"\"}"_ba;
    QTest::newRow("Valid") << QString::fromLatin1("qwerty"_L1)
                           << QByteArray::fromHex("4206717765727479"_ba)
                           << "{\"testFieldStringOpt\":\"qwerty\"}"_ba;
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalString()
{
    QFETCH(const QString, expected);
    QFETCH(const QByteArray, data);
    QFETCH(const QByteArray, dataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? data : dataJson);
    QVERIFY(msg.hasTestFieldStringOpt());
    QCOMPARE(msg.testFieldStringOpt(), expected);
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalBytes_data()
{
    QTest::addColumn<QByteArray>("expected");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataJson");

    QTest::newRow("EmptyBytes") << ""_ba << QByteArray::fromHex("3200"_ba)
                                << "{\"testFieldBytesOpt\":\"\"}"_ba;
    QTest::newRow("Valid") << "qwerty"_ba << QByteArray::fromHex("3206717765727479"_ba)
                           << "{\"testFieldBytesOpt\":\"cXdlcnR5\"}"_ba;
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalBytes()
{
    QFETCH(const QByteArray, expected);
    QFETCH(const QByteArray, data);
    QFETCH(const QByteArray, dataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? data : dataJson);
    QVERIFY(msg.hasTestFieldBytesOpt());
    QCOMPARE(msg.testFieldBytesOpt(), expected);
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalMessage_data()
{
    QTest::addColumn<qtprotobufnamespace::optional::tests::TestStringMessage>("expected");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataJson");

    QTest::newRow("EmptyMessage") << qtprotobufnamespace::optional::tests::TestStringMessage()
                                  << QByteArray::fromHex("5200"_ba)
                                  << "{\"testFieldMessageOpt\":{}}"_ba;
}

void QtProtobufOptionalDeserializationTest::deserializeOptionalMessage()
{
    QFETCH(const qtprotobufnamespace::optional::tests::TestStringMessage, expected);
    QFETCH(const QByteArray, data);
    QFETCH(const QByteArray, dataJson);

    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.deserialize(m_serializer.get(), m_format == Protobuf ? data : dataJson);
    QVERIFY(msg.hasTestFieldMessageOpt());
    QCOMPARE(msg.testFieldMessageOpt(), expected);
}

QTEST_MAIN(QtProtobufOptionalDeserializationTest)

#include "tst_protobuf_deserialization_optional.moc"
