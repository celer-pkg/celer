// Copyright (C) 2019 Tasuku Suzuki <tasuku.suzuki@qbc.io>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qhttpserverresponse.h>

#include <QtCore/qfile.h>
#include <QtTest/qtest.h>

#if QT_CONFIG(mimetype)
#include <QtCore/qmimedatabase.h>
#endif

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

class tst_QHttpServerResponse : public QObject
{
    Q_OBJECT

private slots:
    void mimeTypeDetection_data();
    void mimeTypeDetection();
    void mimeTypeDetectionFromFile_data();
    void mimeTypeDetectionFromFile();
    void headers();
};

void tst_QHttpServerResponse::mimeTypeDetection_data()
{
    QTest::addColumn<QString>("content");

    QTest::addRow("application/x-zerosize")
        << QFINDTESTDATA("data/empty");

    QTest::addRow("text/plain")
        << QFINDTESTDATA("data/text.plain");

    QTest::addRow("text/html")
        << QFINDTESTDATA("data/text.html");

    QTest::addRow("image/png")
        << QFINDTESTDATA("data/image.png");

    QTest::addRow("image/jpeg")
             << QFINDTESTDATA("data/image.jpeg");

    QTest::addRow("image/svg+xml")
             << QFINDTESTDATA("data/image.svg");
}

void tst_QHttpServerResponse::mimeTypeDetection()
{
#if !QT_CONFIG(mimetype)
    QSKIP("Test requires QMimeDatabase");
#else
    QFETCH(QString, content);

    QFile file(content);
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray data = file.readAll();
    QHttpServerResponse response(data);
    file.close();

    const QMimeType mimeType = QMimeDatabase().mimeTypeForData(data);
    QCOMPARE(response.mimeType(), mimeType.name());
#endif
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile_data()
{
    QTest::addColumn<QString>("content");

    QTest::addRow("application/x-zerosize")
            << QFINDTESTDATA("data/empty");

    QTest::addRow("text/plain")
            << QFINDTESTDATA("data/text.plain");

    QTest::addRow("text/html")
            << QFINDTESTDATA("data/text.html");

    QTest::addRow("image/png")
            << QFINDTESTDATA("data/image.png");

    QTest::addRow("image/jpeg")
            << QFINDTESTDATA("data/image.jpeg");

    QTest::addRow("image/svg+xml")
            << QFINDTESTDATA("data/image.svg");

    QTest::addRow("application/json")
            << QFINDTESTDATA("data/application.json");
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile()
{
#if !QT_CONFIG(mimetype)
    QSKIP("Test requires QMimeDatabase");
#else
    QFETCH(QString, content);
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(content);

    const QByteArray responseMimeType = QHttpServerResponse::fromFile(content).mimeType();
    QCOMPARE(responseMimeType, mimeType.name());
#endif
}

void tst_QHttpServerResponse::headers()
{
    QHttpServerResponse resp("");

    const QByteArray test1 = "test1"_ba;
    const QByteArray test2 = "test2"_ba;
    const QByteArray zero = "application/x-zerosize"_ba;

    QHttpHeaders h = resp.headers();
    QVERIFY(!h.contains(QHttpHeaders::WellKnownHeader::ContentLength));
    const auto contentTypeValues = h.values(QHttpHeaders::WellKnownHeader::ContentType);
    QCOMPARE(contentTypeValues.size(), 1);
    QCOMPARE(contentTypeValues.first(), zero);

    h.append(QHttpHeaders::WellKnownHeader::ContentType, test1);
    h.append(QHttpHeaders::WellKnownHeader::ContentLength, test2);
    resp.setHeaders(h);
    QCOMPARE(resp.headers().toListOfPairs(), h.toListOfPairs());

    resp.setHeaders({});
    QVERIFY(resp.headers().isEmpty());

    auto tmp = h;
    resp.setHeaders(std::move(tmp));
    QCOMPARE(resp.headers().toListOfPairs(), h.toListOfPairs());
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerResponse)

#include "tst_qhttpserverresponse.moc"
