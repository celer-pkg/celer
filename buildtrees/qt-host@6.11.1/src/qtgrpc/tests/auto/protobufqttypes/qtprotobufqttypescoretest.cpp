// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2020 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtcoretypesnested.qpb.h"
#include "qtcoretypes.qpb.h"

#include <qtprotobuftestscommon.h>
#include <QProtobufSerializer>

#include <QObject>
#include <QtTest/QtTest>
#include <private/qtenvironmentvariables_p.h>

using namespace Qt::StringLiterals;

constexpr char conversionErrorMessage[] = "Qt Proto Type conversion error.";

const QTime testTime = QTime(7, 30, 18, 321);
const QDate testDate = QDate(1856, 6, 10);
const char *emptyValue = "";

class QtProtobufQtTypesQtCoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void qUrl();
    void qChar();
    void qUuid();
    void qTime();
    void qDate();
    void qTimeZone_data();
    void qTimeZone();
    void qDateTime_data();
    void qDateTime();
    void qSize();
    void qPoint();
    void qPointF();
    void qSizeF();
    void qRect();
    void qRectF();
    void qVersionNumber();

    void nestedQUrlMessageDefined();

private:
    QProtobufSerializer serializer;

#if QT_CONFIG(timezone)
    class TimeZoneRollback
    {
        const QByteArray prior;
    public:
        explicit TimeZoneRollback(const QByteArray &zone) : prior(qgetenv("TZ"))
        { reset(zone); }
        void reset(const QByteArray &zone)
        {
            qputenv("TZ", zone);
            qTzSet();
        }
        ~TimeZoneRollback()
        {
            if (prior.isNull())
                qunsetenv("TZ");
            else
                qputenv("TZ", prior);
            qTzSet();
        }
    };
#endif // timezone
};

void QtProtobufQtTypesQtCoreTest::initTestCase()
{
    QtProtobuf::registerProtobufQtCoreTypes();
}

using namespace qtprotobufnamespace::qttypes::tests;

void QtProtobufQtTypesQtCoreTest::qUrl()
{
    qProtobufAssertMessagePropertyRegistered<QUrlMessage, QUrl>(1, "QUrl", "testField");

    QUrlMessage msg;
    const char *qtUrl = "https://www.qt.io/product/framework";
    const char *qtUrlInvalid = "%https://www.qt.io/"; // '%' symbol is not allowed.
    const char *hexUrlValue
            = "0a250a2368747470733a2f2f7777772e71742e696f2f70726f647563742f6672616d65776f726b";
    msg.setTestField(QUrl(qtUrl));

    QCOMPARE(QByteArray::fromHex(hexUrlValue), msg.serialize(&serializer));

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexUrlValue));

    QVERIFY(msg.testField().isValid());
    QCOMPARE(QString::fromLatin1(qtUrl), msg.testField().url());

    msg.setTestField(QUrl(qtUrlInvalid));
    QVERIFY(!msg.testField().isValid());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    QByteArray result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QUrl(qtUrl));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qChar()
{
    qProtobufAssertMessagePropertyRegistered<QCharMessage, QChar>(1, "QChar", "testField");

    QCharMessage msg;
    msg.setTestField(QChar('q'));
    QCOMPARE(QByteArray::fromHex("0a020871"), msg.serialize(&serializer));

    msg.setTestField({});
    msg.setTestField({QChar(8364)});

    msg.deserialize(&serializer, QByteArray::fromHex("0a0308ac41"));
    QCOMPARE(QChar(8364), msg.testField());
}

void QtProtobufQtTypesQtCoreTest::qUuid()
{
    qProtobufAssertMessagePropertyRegistered<QUuidMessage, QUuid>(1, "QUuid", "testField");

    const char *hexUuidValue = "0a120a104bcbcdc3c5b34d3497feaf78c825cc7d";
    const char *uuidValue = "{4bcbcdc3-c5b3-4d34-97fe-af78c825cc7d}";

    QUuidMessage msg;
    msg.setTestField(QUuid(uuidValue));
    QCOMPARE(QByteArray::fromHex(hexUuidValue), msg.serialize(&serializer));

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexUuidValue));

    QCOMPARE(QUuid(uuidValue), msg.testField());

    const char *uuidInvalidHex = "0a120a1000000000000000000000000000000000";
    msg.setTestField(QUuid());
    QVERIFY(msg.testField().isNull());
    QByteArray result = msg.serialize(&serializer);
    QCOMPARE(QByteArray::fromHex(uuidInvalidHex), result);

    msg.setTestField(QUuid(uuidValue));
    msg.deserialize(&serializer, QByteArray::fromHex(uuidInvalidHex));
    QVERIFY(msg.testField().isNull());
}

void QtProtobufQtTypesQtCoreTest::qTime()
{
    qProtobufAssertMessagePropertyRegistered<QTimeMessage, QTime>(1, "QTime", "testField");
    QTimeMessage msg;
    msg.setTestField(QTime(5, 30, 48, 123));
    QByteArray result = msg.serialize(&serializer);

    QCOMPARE(QByteArray::fromHex("0a0508bbb7bb09"), result);

    msg.deserialize(&serializer, QByteArray::fromHex("0a0508d188f10c"));
    QCOMPARE(msg.testField().hour(), testTime.hour());
    QCOMPARE(msg.testField().minute(), testTime.minute());
    QCOMPARE(msg.testField().second(), testTime.second());
    QCOMPARE(msg.testField().msec(), testTime.msec());

    msg.setTestField(QTime());
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer); // Error message is generated
    QVERIFY(result.isEmpty());

    msg.setTestField(QTime(5, 30, 48, 123));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(msg.testField().isNull());
}

void QtProtobufQtTypesQtCoreTest::qDate()
{
    qProtobufAssertMessagePropertyRegistered<QDateMessage, QDate>(1, "QDate", "testField");
    QDateMessage msg;
    msg.setTestField(testDate);
    QByteArray result = msg.serialize(&serializer);

    QCOMPARE(QByteArray::fromHex("0a050887b79201"), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex("0a0508aeab9301"));
    QCOMPARE(msg.testField().year(), 1897);

    msg.setTestField(QDate());
    QVERIFY(!msg.testField().isValid());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer); // Error message is generated
    QVERIFY(result.isEmpty());

    msg.setTestField(testDate);
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qTimeZone_data()
{
    QTest::addColumn<QTimeZone>("zone");
    QTest::addColumn<QByteArray>("zoneHex");
    QTest::addColumn<bool>("isValid");
#if QT_CONFIG(timezone)
    if (QTimeZone oz("Australia/Darwin"); oz.isValid()) {
        QTest::addRow("Oz/Darwin")
                << oz
                << QByteArray("0a1212104175737472616c69612f44617277696e")
                << true;
    }
#endif
    QTest::addRow("UTC")
            << QTimeZone(QTimeZone::UTC)
            << QByteArray("0a021801")
            << true;
    QTest::addRow("LocalTime")
            << QTimeZone(QTimeZone::LocalTime)
            << QByteArray("0a021800")
            << true;
    QTest::addRow("OffsetFromUTC")
            << QTimeZone::fromSecondsAheadOfUtc(3141)
            << QByteArray("0a0308c518")
            << true;
    QTest::addRow("invalid") << QTimeZone() << QByteArray() << false;
}

void QtProtobufQtTypesQtCoreTest::qTimeZone()
{
    QFETCH(const QTimeZone, zone);
    QFETCH(const QByteArray, zoneHex);
    QFETCH(const bool, isValid);

    qProtobufAssertMessagePropertyRegistered<QTimeZoneMessage, QTimeZone>(1,
                                                                          "QTimeZone",
                                                                          "testField");
    QTimeZoneMessage msg;
    msg.setTestField(zone);

    if (isValid) {
        QByteArray result = msg.serialize(&serializer);
        QCOMPARE(result.toHex(), zoneHex);

        msg.setTestField({});
        msg.deserialize(&serializer, QByteArray::fromHex(zoneHex.constData()));
        QCOMPARE(msg.testField(), zone);
    } else {
        QVERIFY(!msg.testField().isValid());
        QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
        QByteArray result = msg.serialize(&serializer); // Error message is generated
        QVERIFY(result.isEmpty());

        msg.setTestField(QTimeZone(QTimeZone::UTC));
        msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
        QVERIFY(!msg.testField().isValid());
    }
}

void QtProtobufQtTypesQtCoreTest::qDateTime_data()
{
    QTest::addColumn<QTimeZone>("zone");
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QByteArray>("zoneHex");
    QTest::addColumn<bool>("isValid");
#if QT_CONFIG(timezone)
    if (QTimeZone rus("Asia/Magadan"); rus.isValid()) {
        QTest::addRow("Asia/Magadan")
                << rus
                << QDate(2011, 3, 14)
                << QByteArray("0a1708d191a687eb25120e120c417369612f4d61676164616e")
                << true;
    }
    if (QTimeZone oz("Australia/Darwin"); oz.isValid()) {
        QTest::addRow("Australia/Darwin")
                << oz
                << QDate(2011, 3, 14)
                << QByteArray("0a1b0891ddef89eb25121212104175737472616c69612f44617277696e")
                << true;
    }
    QTest::addRow("LocalTime")
            << QTimeZone(QTimeZone(QTimeZone::LocalTime))
            << QDate(2011, 3, 14)
            << QByteArray("0a0b08d190979aeb2512021800")
            << true;
#endif //QT_CONFIG(timezone)
    QTest::addRow("UTC")
            << QTimeZone(QTimeZone::UTC)
            << QDate(2011, 3, 14)
            << QByteArray("0a0b08d190979aeb2512021801")
            << true;
    QTest::addRow("OffsetFromUTC")
            << QTimeZone::fromSecondsAheadOfUtc(3141)
            << QDate(2011, 3, 14)
            << QByteArray("0a0c08c9b5d798eb25120308c518")
            << true;
    QTest::addRow("invalid") << QTimeZone() << QDate() << QByteArray() << false;
}

void QtProtobufQtTypesQtCoreTest::qDateTime()
{
#if QT_CONFIG(timezone)
#ifdef Q_OS_WIN
    TimeZoneRollback useZone("GMT Standard Time");
#else
    TimeZoneRollback useZone("GMT");
#endif //Q_OS_WIN
#endif //QT_CONFIG(timezone)
    QFETCH(const QTimeZone, zone);
    QFETCH(const QDate, date);
    QFETCH(const QByteArray, zoneHex);
    QFETCH(const bool, isValid);

    qProtobufAssertMessagePropertyRegistered<QDateTimeMessage, QDateTime>(1,
                                                                          "QDateTime",
                                                                          "testField");
    QDateTimeMessage msg;
    const QDateTime dateTime = {date, testTime, zone};
    msg.setTestField(dateTime);

    if (isValid) {
        QByteArray result = msg.serialize(&serializer);
        QCOMPARE(result.toHex(), zoneHex);

        msg.setTestField({});
        msg.deserialize(&serializer, QByteArray::fromHex(zoneHex));
        QCOMPARE(msg.testField(), dateTime);
    } else {
        QVERIFY(msg.testField().isNull());
        QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
        QByteArray result = msg.serialize(&serializer); // Error message is generated
        QVERIFY(result.isEmpty());

        msg.setTestField(dateTime);
        msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
        QVERIFY(msg.testField().isNull());
    }

    // rvalue test
    {
        QDateTime dateTime = {date, QTime(7, 30, 21, 321), zone};
        QDateTime dateTimeCopy = dateTime;
        msg.setTestField(std::move(dateTime));
        QDateTimeMessage msgCopy = msg;
        QCOMPARE_EQ(msg.testField(), dateTimeCopy);
        QCOMPARE_EQ(msg, msgCopy);

        msg.setTestField({ QDate(2011, 3, 14),
                           QTime(6, 30, 21, 321),
                           QTimeZone(QTimeZone::UTC) });
        QCOMPARE_NE(msg.testField(), dateTimeCopy);
        QCOMPARE_NE(msg, msgCopy);
    }
}

void QtProtobufQtTypesQtCoreTest::qSize()
{
    qProtobufAssertMessagePropertyRegistered<QSizeMessage, QSize>(1, "QSize", "testField");
    QSizeMessage msg;
    const QSize size({1024, 768});
    msg.setTestField(size);

    QByteArray result = msg.serialize(&serializer);
    const char *hexValue = "0a06088008108006";

    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField().width(), size.width());
    QCOMPARE(msg.testField().height(), size.height());

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex("0a06088006108008"));
    QCOMPARE(msg.testField().width(), size.height());
    QCOMPARE(msg.testField().height(), size.width());

    msg.setTestField(QSize(0, 0));
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QSize(20, 67));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qSizeF()
{
    qProtobufAssertMessagePropertyRegistered<QSizeFMessage, QSizeF>(1, "QSizeF", "testField");
    QSizeFMessage msg;

    const QSizeF sizeF = {1024.0, 768.0};
    msg.setTestField(sizeF);

    QByteArray result = msg.serialize(&serializer);
    const char *hexValue = "0a12090000000000009040110000000000008840";
    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField().width(), sizeF.width());
    QCOMPARE(msg.testField().height(), sizeF.height());

    msg.deserialize(&serializer,
                    QByteArray::fromHex("0a12090000000000008840110000000000009040"));
    QCOMPARE(msg.testField().width(), sizeF.height());
    QCOMPARE(msg.testField().height(), sizeF.width());

    msg.setTestField(QSizeF(0.0, 0.0));
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QSizeF(30.0, 0.2));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qPoint()
{
    qProtobufAssertMessagePropertyRegistered<QPointMessage, QPoint>(1, "QPoint", "testField");
    QPointMessage msg;
    const QPoint point = {1024, 768};
    msg.setTestField(point);

    QByteArray result = msg.serialize(&serializer);
    QCOMPARE(QByteArray::fromHex("0a0608801010800c"), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex("0a0608801010800c"));
    QCOMPARE(msg.testField().x(), point.x());
    QCOMPARE(msg.testField().y(), point.y());

    msg.deserialize(&serializer, QByteArray::fromHex("0a0608800c108010"));
    QCOMPARE(msg.testField().x(), point.y());
    QCOMPARE(msg.testField().y(), point.x());

    msg.setTestField(QPoint(0, 0));
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QPoint(2, 9));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(msg.testField().isNull());
}

void QtProtobufQtTypesQtCoreTest::qPointF()
{
    qProtobufAssertMessagePropertyRegistered<QPointFMessage, QPointF>(1, "QPointF", "testField");
    QPointFMessage msg;

    const QPointF pointF = {1024.0, 768.0};
    msg.setTestField(pointF);

    QByteArray result = msg.serialize(&serializer);
    const char *hexValue = "0a12090000000000009040110000000000008840";
    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField().x(), pointF.x());
    QCOMPARE(msg.testField().y(), pointF.y());

    msg.deserialize(&serializer,
                    QByteArray::fromHex("0a12090000000000008840110000000000009040"));
    QCOMPARE(msg.testField().x(), pointF.y());
    QCOMPARE(msg.testField().y(), pointF.x());

    msg.setTestField(QPointF(0, 0));
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QPointF(0.25, 10));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(msg.testField().isNull());
}

void QtProtobufQtTypesQtCoreTest::qRect()
{
    qProtobufAssertMessagePropertyRegistered<QRectMessage, QRect>(1, "QRect", "testField");
    QRectMessage msg;
    QPoint point(768, 768);
    QSize size(500, 1212);
    msg.setTestField({point, size});

    QByteArray result = msg.serialize(&serializer);

    const char *hexValue = "0a0c08800c10800c18f40320bc09";
    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});

    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField().x(), point.x());
    QCOMPARE(msg.testField().y(), point.y());
    QCOMPARE(msg.testField().width(), size.width());
    QCOMPARE(msg.testField().height(), size.height());

    msg.setTestField({});

    msg.deserialize(&serializer, QByteArray::fromHex("0a06188008208006"));
    QCOMPARE(msg.testField().x(), 0);
    QCOMPARE(msg.testField().y(), 0);
    QCOMPARE(msg.testField().width(), 1024);
    QCOMPARE(msg.testField().height(), 768);

    msg.setTestField(QRect());
    QVERIFY(!msg.testField().isValid());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QRect(1, 2, 10, 10));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qRectF()
{
    qProtobufAssertMessagePropertyRegistered<QRectFMessage, QRectF>(1, "QRectF", "testField");
    QRectFMessage msg;
    QPointF pointF(768.0, 768.0);
    QSizeF sizeF(1024.0, 1024.0);
    msg.setTestField({pointF.x(), pointF.y(), sizeF.width(), sizeF.height()});

    QByteArray result = msg.serialize(&serializer);

    const char *hexValue
            = "0a24090000000000008840110000000000008840190000000000009040210000000000009040";
    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField().x(), pointF.x());
    QCOMPARE(msg.testField().y(), pointF.y());
    QCOMPARE(msg.testField().width(), sizeF.width());
    QCOMPARE(msg.testField().height(),sizeF.height());

    msg.setTestField({QPointF(0.0, 0.0), QSizeF(1024.0, 768.0)});
    msg.deserialize(&serializer,
                    QByteArray::fromHex("0a24090000000000000000110000000000000"
                                        "000190000000000009040210000000000008840"));
    QCOMPARE(msg.testField().x(), 0.0);
    QCOMPARE(msg.testField().y(), 0.0);
    QCOMPARE(msg.testField().width(), 1024.0);
    QCOMPARE(msg.testField().height(), 768.0);

    msg.setTestField(QRectF());
    QVERIFY(!msg.testField().isValid());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(QRectF(88.8, 87, 100, 101.2));
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(!msg.testField().isValid());
}

void QtProtobufQtTypesQtCoreTest::qVersionNumber()
{
    qProtobufAssertMessagePropertyRegistered<QVersionNumberFMessage,
            QVersionNumber>(1, "QVersionNumber", "testField");
    QVersionNumberFMessage msg;
    QVersionNumber version(1, 1, 0);
    msg.setTestField({version});

    QByteArray result = msg.serialize(&serializer);
    const char *hexValue = "0a050a03010100";
    QCOMPARE(QByteArray::fromHex(hexValue), result);

    msg.setTestField({});
    msg.deserialize(&serializer, QByteArray::fromHex(hexValue));
    QCOMPARE(msg.testField(), version);

    msg.setTestField(QVersionNumber());
    QVERIFY(msg.testField().isNull());
    QTest::ignoreMessage(QtWarningMsg, conversionErrorMessage);
    result = msg.serialize(&serializer);
    QVERIFY(result.isEmpty());

    msg.setTestField(version);
    msg.deserialize(&serializer, QByteArray::fromHex(emptyValue));
    QVERIFY(msg.testField().isNull());
}

void QtProtobufQtTypesQtCoreTest::nestedQUrlMessageDefined()
{
    const QUrl url{u"http://qt.io"_s};
    NestedQUrlMessage::QUrlMessage msg;
    msg.setTestField(url);
    QCOMPARE(msg.testField(), url);
}

QTEST_MAIN(QtProtobufQtTypesQtCoreTest)
#include "qtprotobufqttypescoretest.moc"

