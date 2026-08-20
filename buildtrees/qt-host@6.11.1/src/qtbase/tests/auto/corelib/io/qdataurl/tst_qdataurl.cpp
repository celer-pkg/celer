// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "private/qdataurl_p.h"
#include <QTest>
#include <QtCore/QDebug>

using namespace Qt::Literals;

class tst_QDataUrl : public QObject
{
    Q_OBJECT

private slots:
    void decode_data();
    void decode();
};

void tst_QDataUrl::decode_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QByteArray>("payload");

    auto row = [](const char *tag, const char *url, bool success, QString mimeType = {}, QByteArray payload = {}) {
        QTest::newRow(tag) << url << success <<mimeType << payload;
    };

    row("nonData", "http://test.com", false);
    row("malformed-host", "data://test.com", false);
    row("malformed-host2", "data://text/plain;charset=ISO-8859-1", false);
    row("malformed-host3", "data://test.com/,", false);
    row("malformed-no-comma", "data:text/plain", false);

    constexpr auto defaultMimeType = "text/plain;charset=US-ASCII"_L1;

    row("emptyData-default-mimetype", "data:,", true,
        "text/plain;charset=US-ASCII"_L1, "");
    row("emptyData-only-charset", "data:charset=ISO-8859-1,", true,
        "text/plain;charset=ISO-8859-1"_L1, "");

    row("alreadyPercentageEncoded", "data:text/plain,%E2%88%9A", true,
        "text/plain"_L1, QByteArray::fromPercentEncoding("%E2%88%9A"));
    row("everythingIsCaseInsensitive", "Data:texT/PlaiN;charSet=iSo-8859-1;Base64,SGVsbG8=", true,
        "texT/PlaiN;charSet=iSo-8859-1"_L1, QByteArrayLiteral("Hello"));
    row("spacesAroundCharset", "data:%20charset%20%20=%20UTF-8,Hello", true,
        "text/plain;charset  = UTF-8"_L1, QByteArrayLiteral("Hello"));
    row("prematureCharsetEnd", "data:charset,", true,
        "text/plain;charset"_L1, ""); // nonsense result, but don't crash
    row("prematureCharsetEnd-no-encoding", "data:charset=,", true,
        "text/plain;charset="_L1, ""); // nonsense result, but don't crash
    row("charset-value-as-quoted-string", "data:charset=%22UTF-8%22,Hello", true,
        "text/plain;charset=\"UTF-8\""_L1, "Hello"_ba);

    row("extraparameter-before-charset", "data:;extraparameter=foo;charset=ISO-8859-1,Hello", true,
        "text/plain;charset=ISO-8859-1", "Hello");
    row("extraparameter-after-charset", "data:charset=ISO-8859-1;extraparameter=foo,Hello", true,
        "text/plain;charset=ISO-8859-1", "Hello");
    row("content-type-parsing-slash-in-mimetype",
        "data:charset=UTF-8;alternate=\"application/octet-stream\";bar=baz,", true,
        "text/plain;charset=UTF-8", "");
    row("not-real-charset", "data:incharsetter=true,", true, defaultMimeType, "");

    row("percent-encoded-comma-in-parameter-value",
        "data:;charset=%22UTF-8%22;x-bar=\"a%2Cb%2Cc\",Hello%2C%20world", true,
        "text/plain;charset=\"UTF-8\""_L1, "Hello, world");

    QString path = QFINDTESTDATA("arrow-down-16.png");
    QFile img(path);
    QVERIFY(img.open(QFile::ReadOnly));
    QByteArray imageData = img.readAll();
    QByteArray base64 = imageData.toBase64();
    row("image-png", "data:image/png;base64," + base64, true,
        "image/png", imageData);
}

void tst_QDataUrl::decode()
{
    QFETCH(const QString, input);
    QFETCH(const bool, result);
    QFETCH(const QString, mimeType);
    QFETCH(const QByteArray, payload);

    QString actualMimeType;
    QByteArray actualPayload;

    QUrl url(input);
    const bool actualResult = qDecodeDataUrl(url, actualMimeType, actualPayload);

    QCOMPARE(actualResult, result);
    QCOMPARE(actualMimeType, mimeType);
    QCOMPARE(actualPayload, payload);
    QCOMPARE(actualPayload.isNull(), payload.isNull()); // assume nullness is significant
}

QTEST_MAIN(tst_QDataUrl)
#include "tst_qdataurl.moc"
