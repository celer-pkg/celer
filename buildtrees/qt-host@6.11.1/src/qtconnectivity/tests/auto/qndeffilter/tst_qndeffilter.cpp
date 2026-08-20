// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>

#include <QNdefFilter>
#include <QNdefNfcSmartPosterRecord>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include <QNdefMessage>

QT_USE_NAMESPACE

class tst_QNdefFilter : public QObject
{
    Q_OBJECT
private slots:
    void construct();
    void copyConstruct();
    void assingmentOperator();

    void clearFilter();

    void orderMatch();

    void appendRecord();
    void appendRecord_data();

    void appendRecordParameters();
    void appendRecordParameters_data();

    void appendRecordTemplate();
    void appendRecordTemplate_data();

    void match();
    void match_data();
};

void tst_QNdefFilter::construct()
{
    QNdefFilter filter;
    QCOMPARE(filter.recordCount(), 0);
    QCOMPARE(filter.orderMatch(), false);
}

void tst_QNdefFilter::copyConstruct()
{
    QNdefFilter filter;
    filter.setOrderMatch(true);
    filter.appendRecord<QNdefNfcTextRecord>(1, 2);
    filter.appendRecord(QNdefRecord::Empty, "", 0, 1);

    QNdefFilter filterCopy(filter);
    QCOMPARE(filterCopy.orderMatch(), true);
    QCOMPARE(filterCopy.recordCount(), 2);
    QNdefFilter::Record rec = filterCopy.recordAt(1);
    QCOMPARE(rec.typeNameFormat, QNdefRecord::Empty);
    QCOMPARE(rec.type, QByteArray());
    QCOMPARE(rec.minimum, 0U);
    QCOMPARE(rec.maximum, 1U);
}

void tst_QNdefFilter::assingmentOperator()
{
    QNdefFilter filter;
    filter.setOrderMatch(true);
    filter.appendRecord<QNdefNfcTextRecord>(1, 2);
    filter.appendRecord(QNdefRecord::Empty, "", 0, 1);

    QNdefFilter filterCopy;
    filterCopy = filter;

    QCOMPARE(filterCopy.orderMatch(), true);
    QCOMPARE(filterCopy.recordCount(), 2);
    QNdefFilter::Record rec = filterCopy.recordAt(1);
    QCOMPARE(rec.typeNameFormat, QNdefRecord::Empty);
    QCOMPARE(rec.type, QByteArray());
    QCOMPARE(rec.minimum, 0U);
    QCOMPARE(rec.maximum, 1U);
}

void tst_QNdefFilter::clearFilter()
{
    QNdefFilter filter;
    filter.setOrderMatch(true);
    filter.appendRecord<QNdefNfcTextRecord>(1, 2);
    filter.appendRecord(QNdefRecord::Empty, "", 0, 1);

    filter.clear();

    QCOMPARE(filter.orderMatch(), false);
    QCOMPARE(filter.recordCount(), 0);
}

void tst_QNdefFilter::orderMatch()
{
    QNdefFilter filter;
    QCOMPARE(filter.orderMatch(), false);

    filter.setOrderMatch(true);
    QCOMPARE(filter.orderMatch(), true);

    filter.setOrderMatch(false);
    QCOMPARE(filter.orderMatch(), false);
}

void tst_QNdefFilter::appendRecord()
{
    QFETCH(QNdefRecord::TypeNameFormat, typeNameFormat);
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;

    const QNdefFilter::Record record { typeNameFormat, type, minimum, maximum };
    QVERIFY(filter.appendRecord(record) == result);
    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecord_data()
{
    QTest::addColumn<QNdefRecord::TypeNameFormat>("typeNameFormat");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<unsigned int>("minimum");
    QTest::addColumn<unsigned int>("maximum");
    QTest::addColumn<bool>("result");

    constexpr struct {
        const char *type;
        QNdefRecord::TypeNameFormat format;
    } inputs[] = {
        { "Empty", QNdefRecord::Empty },
        { "NfcRtd", QNdefRecord::NfcRtd },
        { "Mime", QNdefRecord::Mime },
        { "Uri", QNdefRecord::Uri},
        { "ExternalRtd", QNdefRecord::ExternalRtd },
        { "Unknown", QNdefRecord::Unknown }
    };

    for (auto [typeC, format] : inputs) {
        const auto type = QByteArray::fromRawData(typeC, strlen(typeC));
        QTest::addRow("%s; min < max", typeC) << format << type << 1u << 2u << true;
        QTest::addRow("%s; min == max", typeC) << format << type << 2u << 2u << true;
        QTest::addRow("%s; min > max", typeC) << format << type << 2u << 1u << false;
    }
}

void tst_QNdefFilter::appendRecordParameters()
{
    QFETCH(QNdefRecord::TypeNameFormat, typeNameFormat);
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;

    QVERIFY(filter.appendRecord(typeNameFormat, type, minimum, maximum) == result);
    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecordParameters_data()
{
    appendRecord_data();
}

void tst_QNdefFilter::appendRecordTemplate()
{
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;
    if (type == QByteArray("SmartPoster"))
        QVERIFY(filter.appendRecord<QNdefNfcSmartPosterRecord>(minimum, maximum) == result);
    else if (type == QByteArray("Text"))
        QVERIFY(filter.appendRecord<QNdefNfcTextRecord>(minimum, maximum) == result);
    else if (type == QByteArray("Uri"))
        QVERIFY(filter.appendRecord<QNdefNfcUriRecord>(minimum, maximum) == result);

    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecordTemplate_data()
{
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<unsigned int>("minimum");
    QTest::addColumn<unsigned int>("maximum");
    QTest::addColumn<bool>("result");

    const QList<QByteArray> types {"SmartPoster", "Text", "Uri"};
    for (const auto &type : types) {
        QTest::newRow(type + "; min < max") << type << 1u << 2u << true;
        QTest::newRow(type + "; min == max") << type << 2u << 2u << true;
        QTest::newRow(type + "; min > max") << type << 2u << 1u << false;
    }
}

void tst_QNdefFilter::match()
{
    QFETCH(QNdefFilter, filter);
    QFETCH(QNdefMessage, message);
    QFETCH(bool, result);

    QCOMPARE(filter.match(message), result);
}

void tst_QNdefFilter::match_data()
{
    QTest::addColumn<QNdefFilter>("filter");
    QTest::addColumn<QNdefMessage>("message");
    QTest::addColumn<bool>("result");

    QNdefFilter filter;
    filter.appendRecord<QNdefNfcTextRecord>(1, 2);
    filter.appendRecord(QNdefRecord::Mime, "image/png", 1, 1);
    filter.appendRecord(QNdefRecord::Empty, "", 0, 100);

    QNdefNfcTextRecord textRec;
    textRec.setPayload("text");

    QNdefRecord mimeRec;
    mimeRec.setTypeNameFormat(QNdefRecord::Mime);
    mimeRec.setType("image/png");
    mimeRec.setPayload("some image should be here");

    QNdefRecord emptyRec;
    emptyRec.setTypeNameFormat(QNdefRecord::Empty);

    {
        QTest::addRow("empty filter, empty message, match")
                << QNdefFilter() << QNdefMessage() << true;

        QNdefMessage message;
        message.push_back(emptyRec);

        QTest::addRow("empty filter, non-empty message, no match")
                << QNdefFilter() << message << false;

        QTest::addRow("non-empty filter, empty message, no match")
                << filter << QNdefMessage() << false;

        QNdefFilter f;
        f.appendRecord<QNdefNfcUriRecord>(0, 1);

        QTest::addRow("filter with 0 or 1 rec, empty message, match")
                << f << QNdefMessage() << true;

        QNdefMessage uriMsg;
        uriMsg.push_back(QNdefNfcUriRecord());

        QTest::addRow("filter with 0 or 1 rec, one record, match") << f << uriMsg << true;

        uriMsg.push_back(QNdefNfcUriRecord());

        QTest::addRow("filter with 0 or 1 rec, too many records, no match") << f << uriMsg << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(mimeRec);

        QTest::addRow("No optional records, no ordering, match") << filter << message << true;

        message.push_back(emptyRec);
        message.push_back(emptyRec);

        QTest::addRow("Multiple records with optional, no ordering, match")
                << filter << message << true;

        filter.setOrderMatch(true);

        QTest::addRow("Multiple records with optional, with ordering, match")
                << filter << message << true;
    }

    {
        filter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(mimeRec);
        message.push_back(emptyRec);
        message.push_back(textRec);
        message.push_back(emptyRec);
        message.push_back(textRec);

        QTest::addRow("Random order, no ordering, match") << filter << message << true;

        filter.setOrderMatch(true);
        QTest::addRow("Random order, with ordering, no match") << filter << message << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefRecord rec;
        rec.setTypeNameFormat(QNdefRecord::NfcRtd);
        rec.setType("image/png");
        rec.setPayload("borken image with invalid format");

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(rec);

        QTest::addRow("TypeNameFormat mismatch, no ordering, no match")
                << filter << message << false;

        filter.setOrderMatch(true);
        QTest::addRow("TypeNameFormat mismatch, with ordering, no match")
                << filter << message << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefRecord rec;
        rec.setTypeNameFormat(QNdefRecord::Mime);
        rec.setType("image/jpeg");
        rec.setPayload("Good image with invalid type");

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(rec);

        QTest::addRow("Type mismatch, no ordering, no match") << filter << message << false;

        filter.setOrderMatch(true);
        QTest::addRow("Type mismatch, with ordering, no match") << filter << message << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(QNdefNfcUriRecord());
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(mimeRec);
        message.push_back(emptyRec);
        message.push_back(emptyRec);

        QTest::addRow("Type not from filter in the beginning, no ordering, no match")
                << filter << message << false;

        filter.setOrderMatch(true);

        QTest::addRow("Type not from filter in the beginning, with ordering, no match")
                << filter << message << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(mimeRec);
        message.push_back(emptyRec);
        message.push_back(QNdefNfcUriRecord());
        message.push_back(emptyRec);

        QTest::addRow("Type not from filter in the middle, no ordering, no match")
                << filter << message << false;

        filter.setOrderMatch(true);

        QTest::addRow("Type not from filter in the middle, with ordering, no match")
                << filter << message << false;
    }

    {
        filter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(mimeRec);
        message.push_back(emptyRec);
        message.push_back(emptyRec);
        message.push_back(QNdefNfcUriRecord());

        QTest::addRow("Type not from filter in the end, no ordering, no match")
                << filter << message << false;

        filter.setOrderMatch(true);

        QTest::addRow("Type not from filter in the end, with ordering, no match")
                << filter << message << false;
    }

    QNdefFilter repeatedFilter;
    // These 2 consecutive QNdefNfcTextRecord's are the same as
    // repeatedFilter.appendRecord<QNdefNfcTextRecord>(0, 2);
    repeatedFilter.appendRecord<QNdefNfcTextRecord>(0, 1);
    repeatedFilter.appendRecord<QNdefNfcTextRecord>(0, 1);
    repeatedFilter.appendRecord(QNdefRecord::Mime, "", 1, 1);
    repeatedFilter.appendRecord<QNdefNfcTextRecord>(1, 1);

    {
        repeatedFilter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(mimeRec);

        QTest::addRow("Filter with repeated type format, no ordering, match")
                << repeatedFilter << message << true;

        repeatedFilter.setOrderMatch(true);

        QTest::addRow("Filter with repeated type format, with ordering, no match")
                << repeatedFilter << message << false;
    }

    {
        repeatedFilter.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(mimeRec);
        message.push_back(textRec);

        QTest::addRow("Filter with repeated type format 2, no ordering, match")
                << repeatedFilter << message << true;

        repeatedFilter.setOrderMatch(true);

        QTest::addRow("Filter with repeated type format 2, with ordering, match")
                << repeatedFilter << message << true;
    }

    QNdefFilter repeatedFilter2;
    repeatedFilter2.appendRecord<QNdefNfcTextRecord>(0, 2);
    repeatedFilter2.appendRecord<QNdefNfcTextRecord>(1, 1);
    repeatedFilter2.appendRecord(QNdefRecord::Mime, "", 1, 1);

    {
        repeatedFilter2.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(mimeRec);

        QTest::addRow("Filter with repeated type format 3, no ordering, match")
                << repeatedFilter2 << message << true;

        repeatedFilter2.setOrderMatch(true);

        QTest::addRow("Filter with repeated type format 3, with ordering, match")
                << repeatedFilter2 << message << true;
    }

    {
        repeatedFilter2.setOrderMatch(false);

        QNdefMessage message;
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(textRec);
        message.push_back(mimeRec);

        QTest::addRow("Filter with repeated type format 4, no ordering, no match")
                << repeatedFilter2 << message << false;

        repeatedFilter2.setOrderMatch(true);

        QTest::addRow("Filter with repeated type format 4, with ordering, no match")
                << repeatedFilter2 << message << false;
    }
}

QTEST_MAIN(tst_QNdefFilter)

#include "tst_qndeffilter.moc"
