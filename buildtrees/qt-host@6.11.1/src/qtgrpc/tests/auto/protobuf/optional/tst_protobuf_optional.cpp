// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QObject>

#include <QtProtobuf/qprotobufregistration.h>

#include <optional.qpb.h>

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

class QtProtobufOptionalTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copy();
    void move();
    void equal();
};

void QtProtobufOptionalTest::copy()
{
    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldBoolOpt(true);
    msg.setTestFieldBytesOpt(QByteArray::fromHex("00ff00ff"));
    msg.setTestFieldStringOpt("Hello Qt!"_L1);
    msg.setTestFieldOpt(42);

    qtprotobufnamespace::optional::tests::OptionalMessage other(msg);
    other.setTestField(15); //Force detach
    msg.setTestField(15); //Force detach

    QVERIFY(msg.hasTestFieldBoolOpt());
    QVERIFY(msg.hasTestFieldStringOpt());
    QVERIFY(msg.hasTestFieldBytesOpt());
    QVERIFY(msg.hasTestFieldOpt());

    QVERIFY(other.hasTestFieldBoolOpt());
    QVERIFY(other.hasTestFieldStringOpt());
    QVERIFY(other.hasTestFieldBytesOpt());
    QVERIFY(other.hasTestFieldOpt());

    QCOMPARE(other, msg);
    QCOMPARE(other.testFieldOpt(), msg.testFieldOpt());
    QCOMPARE(other.testFieldStringOpt(), msg.testFieldStringOpt());
    QCOMPARE(other.testFieldBytesOpt(), msg.testFieldBytesOpt());
    QCOMPARE(other.testFieldBytesOpt(), msg.testFieldBytesOpt());

    qtprotobufnamespace::optional::tests::OptionalMessage other2;
    other2 = other;

    QVERIFY(other2.hasTestFieldBoolOpt());
    QVERIFY(other2.hasTestFieldStringOpt());
    QVERIFY(other2.hasTestFieldBytesOpt());
    QVERIFY(other2.hasTestFieldOpt());

    QCOMPARE(other2, msg);
    QCOMPARE(other2.testFieldOpt(), msg.testFieldOpt());
    QCOMPARE(other2.testFieldStringOpt(), msg.testFieldStringOpt());
    QCOMPARE(other2.testFieldBytesOpt(), msg.testFieldBytesOpt());
    QCOMPARE(other2.testFieldBytesOpt(), msg.testFieldBytesOpt());
}

void QtProtobufOptionalTest::move()
{
    qtprotobufnamespace::optional::tests::OptionalMessage msg;
    msg.setTestFieldBoolOpt(true);
    msg.setTestFieldBytesOpt(QByteArray::fromHex("00ff00ff"));
    msg.setTestFieldStringOpt("Hello Qt!"_L1);
    msg.setTestFieldOpt(42);

    qtprotobufnamespace::optional::tests::OptionalMessage msgCopy = msg;
    msgCopy.setTestField(15); //Force detach
    msg.setTestField(15); //Force detach

    QVERIFY(msg.hasTestFieldBoolOpt());
    QVERIFY(msg.hasTestFieldStringOpt());
    QVERIFY(msg.hasTestFieldBytesOpt());
    QVERIFY(msg.hasTestFieldOpt());

    QVERIFY(msgCopy.hasTestFieldBoolOpt());
    QVERIFY(msgCopy.hasTestFieldStringOpt());
    QVERIFY(msgCopy.hasTestFieldBytesOpt());
    QVERIFY(msgCopy.hasTestFieldOpt());

    qtprotobufnamespace::optional::tests::OptionalMessage other(std::move(msgCopy));
    QVERIFY(other.hasTestFieldBoolOpt());
    QVERIFY(other.hasTestFieldStringOpt());
    QVERIFY(other.hasTestFieldBytesOpt());
    QVERIFY(other.hasTestFieldOpt());

    QCOMPARE(other, msg);
    QCOMPARE(other.testFieldOpt(), msg.testFieldOpt());
    QCOMPARE(other.testFieldStringOpt(), msg.testFieldStringOpt());
    QCOMPARE(other.testFieldBytesOpt(), msg.testFieldBytesOpt());
    QCOMPARE(other.testFieldBytesOpt(), msg.testFieldBytesOpt());

    qtprotobufnamespace::optional::tests::OptionalMessage other2;
    other2 = std::move(other);

    QVERIFY(other2.hasTestFieldBoolOpt());
    QVERIFY(other2.hasTestFieldStringOpt());
    QVERIFY(other2.hasTestFieldBytesOpt());
    QVERIFY(other2.hasTestFieldOpt());

    QCOMPARE(other2, msg);
    QCOMPARE(other2.testFieldOpt(), msg.testFieldOpt());
    QCOMPARE(other2.testFieldStringOpt(), msg.testFieldStringOpt());
    QCOMPARE(other2.testFieldBytesOpt(), msg.testFieldBytesOpt());
    QCOMPARE(other2.testFieldBytesOpt(), msg.testFieldBytesOpt());
}

void QtProtobufOptionalTest::equal()
{
    qtprotobufnamespace::optional::tests::OptionalMessage msg1;
    msg1.setTestFieldBoolOpt(true);
    msg1.setTestFieldBytesOpt(QByteArray::fromHex("00ff00ff"));
    msg1.setTestFieldStringOpt("Hello Qt!"_L1);
    msg1.setTestFieldOpt(42);
    msg1.setTestField(15);

    qtprotobufnamespace::optional::tests::OptionalMessage msg2;
    msg2.setTestFieldBoolOpt(true);
    msg2.setTestFieldBytesOpt(QByteArray::fromHex("00ff00ff"));
    msg2.setTestFieldStringOpt("Hello Qt!"_L1);
    msg2.setTestFieldOpt(15);
    msg2.setTestField(15);

    QVERIFY(msg1 != msg2);

    msg2.setTestFieldOpt(42);
    QVERIFY(msg1 == msg2);
}

QTEST_MAIN(QtProtobufOptionalTest)

#include "tst_protobuf_optional.moc"
