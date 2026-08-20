// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "durationtest.qpb.h"

#include <QtProtobuf/qprotobufjsonserializer.h>

#include <QtTest/QtTest>

class QtProtobufDurationTest : public QObject
{
    Q_OBJECT

    void setupCommonData();

private Q_SLOTS:
    void validDuration_data();
    void validDuration();
    void invalidDuration_data();
    void invalidDuration();
    void serializeJson_data();
    void serializeJson();
    void serializeJsonOutOfRange_data();
    void serializeJsonOutOfRange();

    void deserializeJson_data();
    void deserializeJson();
    void deserializeMalformedJson_data();
    void deserializeMalformedJson();
};

using namespace qtproto::tests;
using namespace Qt::StringLiterals;

void QtProtobufDurationTest::setupCommonData()
{
    QTest::addRow("Zero") << 0ll << 0 << QString("0s"_L1);
    QTest::addRow("Positive") << 123ll << 123 << QString("123.000000123s"_L1);
    QTest::addRow("Negative") << -123ll << -123 << QString("-123.000000123s"_L1);
    QTest::addRow("Zero seconds positive") << 0ll << 123 << QString("0.000000123s"_L1);
    QTest::addRow("Zero seconds negative") << 0ll << -123 << QString("-0.000000123s"_L1);
    QTest::addRow("Nanos under 10") << 0ll << 1 << QString("0.000000001s"_L1);
    QTest::addRow("Nanos under 100") << 0ll << 21 << QString("0.000000021s"_L1);
    QTest::addRow("Nanos under 1000") << 0ll << 321 << QString("0.000000321s"_L1);
    QTest::addRow("Nanos under 10000") << 0ll << 4321 << QString("0.000004321s"_L1);
    QTest::addRow("Nanos under 100000") << 0ll << 54321 << QString("0.000054321s"_L1);
    QTest::addRow("Nanos under 1000000") << 0ll << 654321 << QString("0.000654321s"_L1);
    QTest::addRow("Nanos under 10000000") << 0ll << 7654321 << QString("0.007654321s"_L1);
    QTest::addRow("Nanos under 100000000") << 0ll << 87654321 << QString("0.087654321s"_L1);
    QTest::addRow("Nanos under 1000000000") << 0ll << 987654321 << QString("0.987654321s"_L1);
    QTest::addRow("Negative nanos under 10") << 0ll << -1 << QString("-0.000000001s"_L1);
    QTest::addRow("Negative nanos under 100") << 0ll << -21 << QString("-0.000000021s"_L1);
    QTest::addRow("Negative nanos under 1000") << 0ll << -321 << QString("-0.000000321s"_L1);
    QTest::addRow("Negative nanos under 10000") << 0ll << -4321 << QString("-0.000004321s"_L1);
    QTest::addRow("Negative nanos under 100000") << 0ll << -54321 << QString("-0.000054321s"_L1);
    QTest::addRow("Negative nanos under 1000000") << 0ll << -654321 << QString("-0.000654321s"_L1);
    QTest::addRow("Negative nanos under 10000000")
        << 0ll << -7654321 << QString("-0.007654321s"_L1);
    QTest::addRow("Negative nanos under 100000000")
        << 0ll << -87654321 << QString("-0.087654321s"_L1);
    QTest::addRow("Negative nanos under 1000000000")
        << 0ll << -987654321 << QString("-0.987654321s"_L1);
    QTest::addRow("Nanos 10") << 0ll << 10 << QString("0.00000001s"_L1);
    QTest::addRow("Nanos 100") << 0ll << 100 << QString("0.0000001s"_L1);
    QTest::addRow("Nanos 1000") << 0ll << 1000 << QString("0.000001s"_L1);
    QTest::addRow("Nanos 10000") << 0ll << 10000 << QString("0.00001s"_L1);
    QTest::addRow("Nanos 100000") << 0ll << 100000 << QString("0.0001s"_L1);
    QTest::addRow("Nanos 1000000") << 0ll << 1000000 << QString("0.001s"_L1);
    QTest::addRow("Nanos 10000000") << 0ll << 10000000 << QString("0.01s"_L1);
    QTest::addRow("Nanos 100000000") << 0ll << 100000000 << QString("0.1s"_L1);
    QTest::addRow("Nanos -10") << 0ll << -10 << QString("-0.00000001s"_L1);
    QTest::addRow("Nanos -100") << 0ll << -100 << QString("-0.0000001s"_L1);
    QTest::addRow("Nanos -1000") << 0ll << -1000 << QString("-0.000001s"_L1);
    QTest::addRow("Nanos -10000") << 0ll << -10000 << QString("-0.00001s"_L1);
    QTest::addRow("Nanos -100000") << 0ll << -100000 << QString("-0.0001s"_L1);
    QTest::addRow("Nanos -1000000") << 0ll << -1000000 << QString("-0.001s"_L1);
    QTest::addRow("Nanos -10000000") << 0ll << -10000000 << QString("-0.01s"_L1);
    QTest::addRow("Nanos -100000000") << 0ll << -100000000 << QString("-0.1s"_L1);
    QTest::addRow("Nanos min") << 0ll << -999999999 << QString("-0.999999999s"_L1);
    QTest::addRow("Nanos max") << 0ll << 999999999 << QString("0.999999999s"_L1);
    QTest::addRow("Seconds min") << -315576000000ll << 0 << QString("-315576000000s"_L1);
    QTest::addRow("Seconds max") << 315576000000ll << 0 << QString("315576000000s"_L1);
}

void QtProtobufDurationTest::validDuration_data()
{
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint32>("nanos");
    QTest::addColumn<QString>("result");

    // Common data has only valid ranges and sign compositions.
    setupCommonData();
}

void QtProtobufDurationTest::validDuration()
{
    QFETCH(qint64, seconds);
    QFETCH(qint32, nanos);

    google::protobuf::Duration duration;
    duration.setSeconds(seconds);
    duration.setNanos(nanos);
    QVERIFY(duration.isValid());
    QVERIFY(google::protobuf::Duration::isValid(seconds, nanos));
}

void QtProtobufDurationTest::invalidDuration_data()
{
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint32>("nanos");
    QTest::addRow("Positive seconds, negative nanos")
        << 123ll << -123;
    QTest::addRow("Negative seconds, positive nanos")
        << -123ll << 123;
    QTest::addRow("Nanoseconds too large") << 0ll << 1000000000;
    QTest::addRow("Nanoseconds too small") << 0ll << -1000000000;
    QTest::addRow("Seconds too large") << -315576000001ll << 0;
    QTest::addRow("Seconds too small") << 315576000001ll << 0;
}

void QtProtobufDurationTest::invalidDuration()
{
    QFETCH(qint64, seconds);
    QFETCH(qint32, nanos);

    google::protobuf::Duration duration;
    duration.setSeconds(seconds);
    duration.setNanos(nanos);
    QVERIFY(!duration.isValid());
    QVERIFY(!google::protobuf::Duration::isValid(seconds, nanos));
}

void QtProtobufDurationTest::serializeJson_data()
{
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint32>("nanos");
    QTest::addColumn<QString>("result");

    setupCommonData();

    // These two tests are mostly corner case and misuse of the API, serializer should fail to
    // serialize nanos and seconds of different signs.
    QTest::addRow("Positive seconds, negative nanos")
        << 123ll << -123 << QString(""_L1);
    QTest::addRow("Negative seconds, positive nanos")
        << -123ll << 123 << QString(""_L1);
}

void QtProtobufDurationTest::serializeJson()
{
    QProtobufJsonSerializer serializer;

    QFETCH(qint64, seconds);
    QFETCH(qint32, nanos);
    QFETCH(QString, result);

    DurationMessage msg;
    google::protobuf::Duration duration;
    duration.setSeconds(seconds);
    duration.setNanos(nanos);

    msg.setField(duration);

    const auto doc = QJsonDocument::fromJson(msg.serialize(&serializer));
    QVERIFY(doc.isObject());

    QCOMPARE(doc.object().value("field"_L1).toString(), result);
}

void QtProtobufDurationTest::serializeJsonOutOfRange_data()
{
    // Duration has the following limitations:
    // - nanoseconds in range [-999999999:999999999]
    // - seconds in range [-315576000000:315576000000]
    // The rest shouldn't be serialized.
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint32>("nanos");

    QTest::addRow("Nanoseconds too large") << 0ll << 1000000000;
    QTest::addRow("Nanoseconds too small") << 0ll << -1000000000;
    QTest::addRow("Seconds too large") << -315576000001ll << 0;
    QTest::addRow("Seconds too small") << 315576000001ll << 0;
}

void QtProtobufDurationTest::serializeJsonOutOfRange()
{
    QProtobufJsonSerializer serializer;

    QFETCH(qint64, seconds);
    QFETCH(qint32, nanos);

    DurationMessage msg;
    google::protobuf::Duration duration;
    duration.setSeconds(seconds);
    duration.setNanos(nanos);

    msg.setField(duration);

    const auto doc = QJsonDocument::fromJson(msg.serialize(&serializer));
    QVERIFY(doc.isObject());

    QVERIFY(doc.object().isEmpty());
}

void QtProtobufDurationTest::deserializeJson_data()
{
    QTest::addColumn<qint64>("seconds");
    QTest::addColumn<qint32>("nanos");
    QTest::addColumn<QString>("input");

    setupCommonData();
}

void QtProtobufDurationTest::deserializeJson()
{
    QProtobufJsonSerializer serializer;

    QFETCH(qint64, seconds);
    QFETCH(qint32, nanos);
    QFETCH(QString, input);

    DurationMessage msg;
    QVERIFY(msg.deserialize(&serializer, QString("{\"field\":\"%1\"}"_L1).arg(input).toLatin1()));

    QCOMPARE(msg.field().seconds(), seconds);
    QCOMPARE(msg.field().nanos(), nanos);
}

void QtProtobufDurationTest::deserializeMalformedJson_data()
{
    QTest::addColumn<QString>("input");

    QTest::addRow("Missing traling 's'") << "123.123";
    QTest::addRow("Random text") << "Hello Qt!";
    QTest::addRow("Malformed floating point") << "123.123.123s";
    QTest::addRow("Missing seconds") << ".123s";
    QTest::addRow("Missing nanos") << "123.s";
    QTest::addRow("Missing seconds negative") << "-.123s";
    QTest::addRow("Missing nanos negative") << "-123.s";
    QTest::addRow("Double minus") << "--123.123s";
    QTest::addRow("Misplaces minus") << "123.-123s";
    QTest::addRow("Nanos too large") << "0.1000000000s";
    QTest::addRow("Nanos too small") << "-0.1000000000s";
    QTest::addRow("Seconds too large") << "315576000001s";
    QTest::addRow("Seconds too large") << "-315576000001s";
}

void QtProtobufDurationTest::deserializeMalformedJson()
{
    QProtobufJsonSerializer serializer;

    QFETCH(QString, input);

    DurationMessage msg;
    QVERIFY(!msg.deserialize(&serializer, QString("{\"field\":\"%1\"}"_L1).arg(input).toLatin1()));

    QCOMPARE(msg.field().seconds(), 0);
    QCOMPARE(msg.field().nanos(), 0);
}

QTEST_MAIN(QtProtobufDurationTest)
#include "tst_protobuf_duration.moc"
