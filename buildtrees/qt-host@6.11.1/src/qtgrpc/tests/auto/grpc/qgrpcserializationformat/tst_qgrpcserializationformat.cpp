// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qgrpcserializationformat.h>

#include <QtProtobuf/qprotobufjsonserializer.h>
#include <QtProtobuf/qprotobufserializer.h>

#include <QtTest/qtest.h>

using namespace QtGrpc;

class QGrpcSerializationFormatTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructFromEnum() const;
    void constructCustom() const;
    void hasSpecialMemberFunctions() const;
    void hasImplicitQVariant() const;
    void hasMemberSwap() const;
    void streamsToDebug() const;
};

void QGrpcSerializationFormatTest::constructFromEnum() const
{
    QGrpcSerializationFormat defaultFormat = SerializationFormat::Default;
    QCOMPARE(defaultFormat.suffix(), "");
    QVERIFY(dynamic_cast<QProtobufSerializer *>(defaultFormat.serializer().get()) != nullptr);

    QGrpcSerializationFormat jsonFormat = SerializationFormat::Json;
    QCOMPARE(jsonFormat.suffix(), "json");
    QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(jsonFormat.serializer().get()) != nullptr);

    QGrpcSerializationFormat protobufFormat = SerializationFormat::Protobuf;
    QCOMPARE(protobufFormat.suffix(), "proto");
    QVERIFY(dynamic_cast<QProtobufSerializer *>(protobufFormat.serializer().get()) != nullptr);
}

void QGrpcSerializationFormatTest::constructCustom() const
{
    QGrpcSerializationFormat customFormat("test", std::make_shared<QProtobufJsonSerializer>());
    QCOMPARE(customFormat.suffix(), "test");
    QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(customFormat.serializer().get()) != nullptr);
}

void QGrpcSerializationFormatTest::hasSpecialMemberFunctions() const
{
    QGrpcSerializationFormat f1(SerializationFormat::Json);

    {
        QGrpcSerializationFormat f2(f1);
        QCOMPARE(f2.suffix(), "json");
        QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(f2.serializer().get()) != nullptr);
        QVERIFY(f2.serializer() == f1.serializer());

        QGrpcSerializationFormat f3(std::move(f2));
        QCOMPARE(f3.suffix(), "json");
        QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(f3.serializer().get()) != nullptr);
        QVERIFY(f3.serializer() == f1.serializer());
    }

    {
        QGrpcSerializationFormat f2;
        f2 = f1;
        QCOMPARE(f2.suffix(), "json");
        QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(f2.serializer().get()) != nullptr);
        QVERIFY(f2.serializer() == f1.serializer());

        QGrpcSerializationFormat f3;
        f3 = std::move(f2);
        QCOMPARE(f3.suffix(), "json");
        QVERIFY(dynamic_cast<QProtobufJsonSerializer *>(f3.serializer().get()) != nullptr);
        QVERIFY(f3.serializer() == f1.serializer());
    }
}

void QGrpcSerializationFormatTest::hasImplicitQVariant() const
{
    QGrpcSerializationFormat sfmt1("test", std::make_shared<QProtobufJsonSerializer>());
    QVariant v = sfmt1;
    QCOMPARE_EQ(v.metaType(), QMetaType::fromType<QGrpcSerializationFormat>());
    const auto sfmt2 = v.value<QGrpcSerializationFormat>();
    QCOMPARE_EQ(sfmt1.suffix(), sfmt2.suffix());
    QCOMPARE_EQ(sfmt1.serializer(), sfmt2.serializer());
}

void QGrpcSerializationFormatTest::hasMemberSwap() const
{
    auto jsonSer = std::make_shared<QProtobufJsonSerializer>();
    auto pbSer = std::make_shared<QProtobufSerializer>();
    QGrpcSerializationFormat sfmt1("one", jsonSer);
    QGrpcSerializationFormat sfmt2("two", pbSer);
    sfmt1.swap(sfmt2);
    auto check = [&](auto a, auto b) {
        QCOMPARE_EQ(a.suffix(), QByteArray("two"));
        QCOMPARE_EQ(a.serializer(), pbSer);
        QCOMPARE_EQ(b.suffix(), QByteArray("one"));
        QCOMPARE_EQ(b.serializer(), jsonSer);
    };
    check(sfmt1, sfmt2);
    swap(sfmt1, sfmt2);
    check(sfmt2, sfmt1);
}

void QGrpcSerializationFormatTest::streamsToDebug() const
{
    QGrpcSerializationFormat sfmt("custom", std::make_shared<QProtobufJsonSerializer>());
    QString storage;
    QDebug dbg(&storage);
    dbg.noquote().nospace();

    dbg << sfmt;
    QVERIFY(!storage.isEmpty());

    std::unique_ptr<char[]> ustr(QTest::toString(sfmt));
    QCOMPARE_EQ(storage, QString::fromUtf8(ustr.get()));
}

QTEST_MAIN(QGrpcSerializationFormatTest)

#include "tst_qgrpcserializationformat.moc"
