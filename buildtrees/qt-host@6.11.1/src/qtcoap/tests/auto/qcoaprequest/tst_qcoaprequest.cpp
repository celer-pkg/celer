// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoaprequest.h>
#include <private/qcoaprequest_p.h>

class tst_QCoapRequest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void ctor_data();
    void ctor();
    void adjustUrl_data();
    void adjustUrl();
    void setUrl_data();
    void setUrl();
    void enableObserve();
    void copyAndDetach();
};

void tst_QCoapRequest::ctor_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("empty") << QUrl();
    QTest::newRow("coap") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
}

void tst_QCoapRequest::ctor()
{
    QFETCH(QUrl, url);

    QCoapRequest request(url);
    QCOMPARE(request.url(), url);
}

void tst_QCoapRequest::adjustUrl_data()
{
    QTest::addColumn<QUrl>("inputUrl");
    QTest::addColumn<QUrl>("expectedUrl");
    QTest::addColumn<bool>("secure");

    QTest::newRow("empty") << QUrl() << QUrl() << false;
    QTest::newRow("empty_secure") << QUrl() << QUrl() << true;
    QTest::newRow("scheme_and_port_known") << QUrl("coap://10.11.12.13:1234/test")
                                           << QUrl("coap://10.11.12.13:1234/test") << false;
    QTest::newRow("scheme_and_port_known_secure") << QUrl("coaps://10.11.12.13:1234/test")
                                                  << QUrl("coaps://10.11.12.13:1234/test") << true;
    QTest::newRow("no_port") << QUrl("coap://vs0.inf.ethz.ch/test")
                             << QUrl("coap://vs0.inf.ethz.ch:5683/test") << false;
    QTest::newRow("no_port_secure") << QUrl("coaps://vs0.inf.ethz.ch/test")
                                    << QUrl("coaps://vs0.inf.ethz.ch:5684/test") << true;
    QTest::newRow("no_scheme_no_port") << QUrl("vs0.inf.ethz.ch/test")
                                       << QUrl("coap://vs0.inf.ethz.ch:5683/test") << false;
    QTest::newRow("no_scheme_no_port_secure") << QUrl("vs0.inf.ethz.ch/test")
                                              << QUrl("coaps://vs0.inf.ethz.ch:5684/test") << true;

    QUrl ipv6Host;
    ipv6Host.setHost("::1");
    ipv6Host.setPath("/path");
    QTest::newRow("no_scheme_no_port_ipv6") << ipv6Host << QUrl("coap://[::1]:5683/path")
                                            << false;
    QTest::newRow("no_scheme_no_port_ipv6_secure") << ipv6Host << QUrl("coaps://[::1]:5684/path")
                                                   << true;
}

void tst_QCoapRequest::adjustUrl()
{
#ifdef QT_BUILD_INTERNAL
    QFETCH(QUrl, inputUrl);
    QFETCH(QUrl, expectedUrl);
    QFETCH(bool, secure);

    // createRequest() will adjust the url
    QCoapRequest request = QCoapRequestPrivate::createRequest(QCoapRequest(inputUrl),
                                                              QtCoap::Method::Get, secure);
    QCOMPARE(request.url(), expectedUrl);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapRequest::setUrl_data()
{
    QTest::addColumn<QUrl>("inputUrl");
    QTest::addColumn<QUrl>("expectedUrl");

    QTest::newRow("empty") << QUrl() << QUrl();
    QTest::newRow("coap") << QUrl("coap://10.11.12.13:5683/test")
                          << QUrl("coap://10.11.12.13:5683/test");
    QTest::newRow("coaps") << QUrl("coaps://10.11.12.13:5683/test")
                           << QUrl("coaps://10.11.12.13:5683/test");
    QTest::newRow("other_port") << QUrl("coap://10.11.12.13:8888/test")
                                << QUrl("coap://10.11.12.13:8888/test");
    QTest::newRow("no_port") << QUrl("coap://vs0.inf.ethz.ch/test")
                             << QUrl("coap://vs0.inf.ethz.ch:5683/test");
    QTest::newRow("no_port_scure") << QUrl("coaps://vs0.inf.ethz.ch/test")
                                   << QUrl("coaps://vs0.inf.ethz.ch:5684/test");
    QTest::newRow("no_scheme_no_port") << QUrl("vs0.inf.ethz.ch/test")
                                       << QUrl("vs0.inf.ethz.ch/test");
    QTest::newRow("incorrect_scheme") << QUrl("http://vs0.inf.ethz.ch:5683/test") << QUrl();
    QTest::newRow("invalid") << QUrl("-coap://vs0.inf.ethz.ch:5683/test") << QUrl();
}

void tst_QCoapRequest::setUrl()
{
    QFETCH(QUrl, inputUrl);
    QFETCH(QUrl, expectedUrl);

    QCoapRequest request;
    request.setUrl(inputUrl);
    QCOMPARE(request.url(), expectedUrl);
}

void tst_QCoapRequest::enableObserve()
{
    QCoapRequest request;

    QCOMPARE(request.isObserve(), false);
    request.enableObserve();

    QCOMPARE(request.isObserve(), true);
}

void tst_QCoapRequest::copyAndDetach()
{
#ifdef QT_BUILD_INTERNAL
    QCoapRequest a = QCoapRequestPrivate::createRequest(QCoapRequest(), QtCoap::Method::Delete);
    a.setMessageId(3);
    a.setPayload("payload");
    a.setToken("token");
    a.setType(QCoapMessage::Type::Acknowledgment);
    a.setVersion(5);
    QUrl testUrl("coap://url:500/resource");
    a.setUrl(testUrl);
    QUrl testProxyUrl("test://proxyurl");
    a.setProxyUrl(testProxyUrl);

    // Test the QCoapMessage copy
    QCoapMessage b(a);
    QVERIFY2(b.messageId() == 3, "Message not copied correctly");
    QVERIFY2(b.payload() == "payload", "Message not copied correctly");
    QVERIFY2(b.token() == "token", "Message not copied correctly");
    QVERIFY2(b.type() == QCoapMessage::Type::Acknowledgment, "Message not copied correctly");
    QVERIFY2(b.version() == 5, "Message not copied correctly");

    // Test the QCoapRequest copy
    QCoapRequest c(a);
    QVERIFY2(c.method() == QtCoap::Method::Delete, "Request not copied correctly");
    QVERIFY2(c.url() == testUrl, "Request not copied correctly");
    QVERIFY2(c.proxyUrl() == testProxyUrl, "Request not copied correctly");

    // Detach
    c.setMessageId(9);
    QCOMPARE(c.messageId(), 9);
    QCOMPARE(a.messageId(), 3);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

QTEST_APPLESS_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
