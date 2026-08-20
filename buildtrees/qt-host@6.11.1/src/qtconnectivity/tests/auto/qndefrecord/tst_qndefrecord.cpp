// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <qndefrecord.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QNdefRecord::TypeNameFormat)

class tst_QNdefRecord : public QObject
{
    Q_OBJECT

public:
    tst_QNdefRecord();
    ~tst_QNdefRecord();

private slots:
    void tst_record();
    void tst_comparison();
    void tst_comparison_data();
    void tst_isRecordType();

    void tst_textRecord_data();
    void tst_textRecord();

    void tst_uriRecord_data();
    void tst_uriRecord();

    void tst_ndefRecord_data();
    void tst_ndefRecord();
};

tst_QNdefRecord::tst_QNdefRecord()
{
}

tst_QNdefRecord::~tst_QNdefRecord()
{
}

void tst_QNdefRecord::tst_record()
{
    // test empty record
    {
        QNdefRecord record;

        QVERIFY(record.isEmpty());
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Empty);
        QVERIFY(record.type().isEmpty());
        QVERIFY(record.id().isEmpty());
        QVERIFY(record.payload().isEmpty());

        QVERIFY(!record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcUriRecord>());

        QCOMPARE(record, QNdefRecord());
        QVERIFY(!(record != QNdefRecord()));
    }

    // test type name format
    {
        QNdefRecord record;

        record.setTypeNameFormat(QNdefRecord::Empty);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Empty);

        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::NfcRtd);

        record.setTypeNameFormat(QNdefRecord::Mime);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Mime);

        record.setTypeNameFormat(QNdefRecord::Uri);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Uri);

        record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::ExternalRtd);

        record.setTypeNameFormat(QNdefRecord::Unknown);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);

        record.setTypeNameFormat(QNdefRecord::TypeNameFormat(6));
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);

        record.setTypeNameFormat(QNdefRecord::TypeNameFormat(7));
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);
    }

    // test type
    {
        QNdefRecord record;

        record.setType("test type");
        QCOMPARE(record.type(), QByteArray("test type"));
    }

    // test id
    {
        QNdefRecord record;

        record.setId("test id");
        QCOMPARE(record.id(), QByteArray("test id"));
    }

    // test payload
    {
        QNdefRecord record;

        record.setPayload("test payload");
        QVERIFY(!record.isEmpty());
        QVERIFY(!record.payload().isEmpty());
        QCOMPARE(record.payload(), QByteArray("test payload"));
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        record.setType("qt-project.org:test-rtd");
        record.setId("test id");
        record.setPayload("test payload");

        QNdefRecord ccopy(record);

        QCOMPARE(record.typeNameFormat(), ccopy.typeNameFormat());
        QCOMPARE(record.type(), ccopy.type());
        QCOMPARE(record.id(), ccopy.id());
        QCOMPARE(record.payload(), ccopy.payload());

        QVERIFY(record == ccopy);
        QVERIFY(!(record != ccopy));

        QNdefRecord acopy;
        acopy = record;

        QCOMPARE(record.typeNameFormat(), acopy.typeNameFormat());
        QCOMPARE(record.type(), acopy.type());
        QCOMPARE(record.id(), acopy.id());
        QCOMPARE(record.payload(), acopy.payload());

        QVERIFY(record == acopy);
        QVERIFY(!(record != acopy));

        QVERIFY(record != QNdefRecord());
    }

    // test clear
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setId("id");
        record.setType("type");
        record.setPayload("Some random data");

        record.clear();
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Empty);
        QVERIFY(record.id().isEmpty());
        QVERIFY(record.type().isEmpty());
        QVERIFY(record.payload().isEmpty());
        QVERIFY(record.isEmpty());
    }
}

void tst_QNdefRecord::tst_comparison()
{
    QFETCH(QNdefRecord, lhs);
    QFETCH(QNdefRecord, rhs);
    QFETCH(bool, result);

    QCOMPARE(lhs == rhs, result);
    QCOMPARE(lhs != rhs, !result);

    QCOMPARE(rhs == lhs, result);
    QCOMPARE(rhs != lhs, !result);
}

void tst_QNdefRecord::tst_comparison_data()
{
    QTest::addColumn<QNdefRecord>("lhs");
    QTest::addColumn<QNdefRecord>("rhs");
    QTest::addColumn<bool>("result");

    QTest::newRow("default-constructed records") << QNdefRecord() << QNdefRecord() << true;
    {
        QNdefRecord rec;
        rec.setTypeNameFormat(QNdefRecord::Empty);

        QTest::newRow("default-constructed vs empty") << QNdefRecord() << rec << false;
    }

    QNdefRecord record;
    record.setTypeNameFormat(QNdefRecord::NfcRtd);
    record.setType("T");
    record.setId("text-record");
    record.setPayload("Some random text");

    {
        QNdefRecord lhs = record;
        QNdefRecord rhs = record;
        rhs.setTypeNameFormat(QNdefRecord::ExternalRtd);

        QTest::newRow("typeNameFormat mismatch") << lhs << rhs << false;
    }
    {
        QNdefRecord lhs = record;
        QNdefRecord rhs = record;
        rhs.setType("t");

        QTest::newRow("type mismatch") << lhs << rhs << false;
    }
    {
        QNdefRecord lhs = record;
        QNdefRecord rhs = record;
        rhs.setId("random id");

        QTest::newRow("id mismatch") << lhs << rhs << false;
    }
    {
        QNdefRecord lhs = record;
        QNdefRecord rhs = record;
        rhs.setPayload("another random text");

        QTest::newRow("payload mismatch") << lhs << rhs << false;
    }
    {
        QNdefRecord lhs = record;
        QNdefRecord rhs = record;

        QTest::newRow("same records") << lhs << rhs << true;
    }
}

class PngNdefRecord : public QNdefRecord
{
public:
    PngNdefRecord() : QNdefRecord(QNdefRecord::Mime, "image/png") { }
};

class JpegNdefRecord : public QNdefRecord
{
public:
    JpegNdefRecord() : QNdefRecord(QNdefRecord::Mime, "image/jpeg") { }
};

void tst_QNdefRecord::tst_isRecordType()
{
    QNdefRecord rec;
    rec.setTypeNameFormat(QNdefRecord::Mime);
    rec.setType("image/png");

    QVERIFY(!rec.isRecordType<QNdefRecord>());
    QVERIFY(!rec.isRecordType<JpegNdefRecord>());
    QVERIFY(rec.isRecordType<PngNdefRecord>());
}

void tst_QNdefRecord::tst_textRecord_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("utf8");
    QTest::addColumn<QByteArray>("payload");


    QTest::newRow("en_US utf8") << QString::fromLatin1("en_US")
                                << QString::fromLatin1("Test String")
                                << true
                                << QByteArray::fromHex("05656E5F55535465737420537472696E67");
    QTest::newRow("en_US utf16") << QString::fromLatin1("en_US")
                                 << QString::fromLatin1("Test String")
                                 << false
                                 << QByteArray::fromHex("85656E5F5553FEFF005400650073007400200053007400720069006E0067");
    QTest::newRow("ja_JP utf8") << QString::fromLatin1("ja_JP")
                                << QString::fromUtf8("\343\203\206\343\202\271\343\203\210\346"
                                                     "\226\207\345\255\227\345\210\227")
                                << true
                                << QByteArray::fromHex("056A615F4A50E38386E382B9E38388E69687E5AD97E58897");
    QTest::newRow("ja_JP utf16") << QString::fromLatin1("ja_JP")
                                 << QString::fromUtf8("\343\203\206\343\202\271\343\203\210\346"
                                                      "\226\207\345\255\227\345\210\227")
                                 << false
                                 << QByteArray::fromHex("856A615F4A50FEFF30C630B930C865875B575217");
}

void tst_QNdefRecord::tst_textRecord()
{
    QFETCH(QString, locale);
    QFETCH(QString, text);
    QFETCH(bool, utf8);
    QFETCH(QByteArray, payload);

    // test setters
    {
        QNdefNfcTextRecord record;
        record.setLocale(locale);
        record.setText(text);
        record.setEncoding(utf8 ? QNdefNfcTextRecord::Utf8 : QNdefNfcTextRecord::Utf16);

        QCOMPARE(record.typeNameFormat(), QNdefRecord::NfcRtd);
        QCOMPARE(record.type(), QByteArray("T"));
        QCOMPARE(record.payload(), payload);

        QVERIFY(record != QNdefRecord());
    }

    // test getters
    {
        QNdefNfcTextRecord record;
        record.setPayload(payload);

        QCOMPARE(record.locale(), locale);
        QCOMPARE(record.text(), text);
        QCOMPARE(record.encoding() == QNdefNfcTextRecord::Utf8, utf8);
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("T");
        record.setPayload(payload);

        QVERIFY(!record.isRecordType<QNdefRecord>());
        QVERIFY(record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcUriRecord>());

        QNdefNfcTextRecord textRecord(record);

        QVERIFY(!textRecord.isEmpty());

        QVERIFY(record == textRecord);

        QCOMPARE(textRecord.locale(), locale);
        QCOMPARE(textRecord.text(), text);
        QCOMPARE(textRecord.encoding() == QNdefNfcTextRecord::Utf8, utf8);
    }
}

void tst_QNdefRecord::tst_uriRecord_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("payload");


    QTest::newRow("http") << QString::fromLatin1("http://qt-project.org/")
                                << QByteArray::fromHex("0371742d70726f6a6563742e6f72672f");
    QTest::newRow("tel") << QString::fromLatin1("tel:+1234567890")
                         << QByteArray::fromHex("052B31323334353637383930");
    QTest::newRow("mailto") << QString::fromLatin1("mailto:test@example.com")
                            << QByteArray::fromHex("0674657374406578616D706C652E636F6D");
    QTest::newRow("urn") << QString::fromLatin1("urn:nfc:ext:qt-project.org:test")
                         << QByteArray::fromHex("136E66633A6578743A71742D70726F6A6563742E6F72673A74657374");
}

void tst_QNdefRecord::tst_uriRecord()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, payload);

    // test setters
    {
        QNdefNfcUriRecord record;
        record.setUri(QUrl(url));

        QCOMPARE(record.typeNameFormat(), QNdefRecord::NfcRtd);
        QCOMPARE(record.type(), QByteArray("U"));
        QCOMPARE(record.payload(), payload);

        QVERIFY(record != QNdefRecord());
    }

    // test getters
    {
        QNdefNfcUriRecord record;
        record.setPayload(payload);

        QCOMPARE(record.uri(), QUrl(url));
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("U");
        record.setPayload(payload);

        QVERIFY(!record.isRecordType<QNdefRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(record.isRecordType<QNdefNfcUriRecord>());

        QNdefNfcUriRecord uriRecord(record);

        QVERIFY(!uriRecord.isEmpty());

        QVERIFY(record == uriRecord);

        QCOMPARE(uriRecord.uri(), QUrl(url));
    }
}

void tst_QNdefRecord::tst_ndefRecord_data()
{
    QTest::addColumn<QNdefRecord::TypeNameFormat>("typeNameFormat");
    QTest::addColumn<QByteArray>("type");

    QTest::newRow("NfcRtd:U") << QNdefRecord::NfcRtd << QByteArray("U");
    QTest::newRow("NfcRtd:T") << QNdefRecord::NfcRtd << QByteArray("T");
    QTest::newRow("Empty:BLAH") << QNdefRecord::Empty << QByteArray("BLAH");
    QTest::newRow("Empty") << QNdefRecord::Empty << QByteArray("");
    QTest::newRow("Unknown") << QNdefRecord::Unknown << QByteArray("BLAHfoo");
    QTest::newRow("Mime") << QNdefRecord::Mime << QByteArray("foobar");
    QTest::newRow("ExternalRtd") << QNdefRecord::ExternalRtd << QByteArray("");
    QTest::newRow("Uri") << QNdefRecord::Uri << QByteArray("example.com/uri");
}

void tst_QNdefRecord::tst_ndefRecord()
{
    QFETCH(QNdefRecord::TypeNameFormat, typeNameFormat);
    QFETCH(QByteArray, type);

    {
        QNdefRecord record;
        record.setTypeNameFormat(typeNameFormat);
        record.setType(type);
        QCOMPARE(record.typeNameFormat(), typeNameFormat);
        QCOMPARE(record.type(), type);
    }
}

QTEST_MAIN(tst_QNdefRecord)

#include "tst_qndefrecord.moc"

