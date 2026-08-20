// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QHostInfo>
#include <QSslError>
#include <QDebug>
#include <QtWebSockets/QWebSocket>

class tst_ComplianceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanupTestCase();

    void autobahnTest_data();
    void autobahnTest();
};

static const QUrl baseUrl { "ws://localhost:9001" };
static const auto agent = QStringLiteral("QtWebSockets/" QT_VERSION_STR);

void tst_ComplianceTest::cleanupTestCase()
{
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);
    auto url = baseUrl;
    url.setPath(QStringLiteral("/updateReports"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("agent"), agent);
    url.setQuery(query);
    webSocket.open(url);
    QVERIFY(spy.wait());
}

void tst_ComplianceTest::autobahnTest_data()
{
    QTest::addColumn<int>("testCase");

    // Ask /getCaseCount how many tests we have
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);

    connect(&webSocket, &QWebSocket::textMessageReceived, [](QString message) {
        bool ok;
        const auto numberOfTestCases = message.toInt(&ok);
        if (!ok)
            QSKIP("Unable to parse /getCaseCount result");
        for (auto i = 1; i <= numberOfTestCases; ++i)
            QTest::addRow("%d", i) << i;
    });

    auto url = baseUrl;
    url.setPath(QStringLiteral("/getCaseCount"));
    webSocket.open(url);
    if (!spy.wait())
        QSKIP("AutoBahn test server didn't deliver case-count");
}

void tst_ComplianceTest::autobahnTest()
{
    QFETCH(int, testCase);
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);
    connect(&webSocket, &QWebSocket::textMessageReceived,
            &webSocket, &QWebSocket::sendTextMessage);
    connect(&webSocket, &QWebSocket::binaryMessageReceived,
            &webSocket, &QWebSocket::sendBinaryMessage);

    // Ask /runCase?case=<number>&agent=<agent> to run the test-case.
    auto url = baseUrl;
    url.setPath(QStringLiteral("/runCase"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("case"), QString::number(testCase));
    query.addQueryItem(QStringLiteral("agent"), agent);
    url.setQuery(query);
    webSocket.open(url);
    QVERIFY(spy.wait());
}

QTEST_MAIN(tst_ComplianceTest)

#include "tst_compliance.moc"
