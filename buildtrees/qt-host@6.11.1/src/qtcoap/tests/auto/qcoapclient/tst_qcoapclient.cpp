// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapclient.h>
#include <QtCoap/qcoaprequest.h>
#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapresourcediscoveryreply.h>
#include <QtCore/qbuffer.h>
#include <QtNetwork/qnetworkdatagram.h>
#include <QtNetwork/qsslcipher.h>
#include <private/qcoapclient_p.h>
#include <private/qcoapqudpconnection_p.h>
#include <private/qcoapprotocol_p.h>
#include <private/qcoaprequest_p.h>

#include "../coapnetworksettings.h"

using namespace QtCoapNetworkSettings;

class tst_QCoapClient : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void incorrectUrls_data();
    void incorrectUrls();
    void methods_data();
    void methods();
    void separateMethod();
    void socketError();
    void timeout_data();
    void timeout();
    void abort();
    void removeReply();
    void setBlockSize_data();
    void setBlockSize();
    void requestWithQIODevice_data();
    void requestWithQIODevice();
    void multipleRequests_data();
    void multipleRequests();
    void blockwiseReply_data();
    void blockwiseReply();
    void blockwiseRequest_data();
    void blockwiseRequest();
    void discover_data();
    void discover();
    void observe_data();
    void observe();
    void confirmableMulticast();
    void multicast();
    void multicast_blockwise();
    void setMinimumTokenSize_data();
    void setMinimumTokenSize();
};

class QCoapClientForSecurityTests : public QCoapClient
{
public:
    QCoapClientForSecurityTests(QtCoap::SecurityMode security)
        : QCoapClient(security)
        , securityMode(security)
    {
        if (security != QtCoap::SecurityMode::NoSecurity)
            setSecurityConfiguration(createConfiguration(security));
    }

    bool securitySetupMissing() const
    {
#if QT_CONFIG(dtls)
        if (securityMode == QtCoap::SecurityMode::PreSharedKey) {
            // The Californium test server accepts only PSK-AES128-CCM8 and PSK-AES128-CBC-SHA256
            // ciphers. Make sure that the required ciphers are present, otherwise the test
            // should be skipped.
            const auto ciphers = QSslConfiguration::defaultDtlsConfiguration().ciphers();
            const auto it = std::find_if(ciphers.cbegin(), ciphers.cend(),
                                         [](const QSslCipher &cipher) {
                                            return cipher.name() == "PSK-AES128-CCM8"
                                                    || cipher.name() == "PSK-AES128-CBC-SHA256";
                                         });
            return it == ciphers.cend();
        }
        // For all other modes the setup should be OK, return false.
        return false;
#else
        // If dtls is not configured, the setup for the secure modes is missing,
        // but it is OK if security is not used.
        return isSecure();
#endif
    }

    bool isSecure() const
    {
        return securityMode != QtCoap::SecurityMode::NoSecurity;
    }

private:
    QtCoap::SecurityMode securityMode;
};

#ifdef QT_BUILD_INTERNAL
class QCoapQUdpConnectionSocketTestsPrivate : public QCoapQUdpConnectionPrivate
{
    bool bind() override
    {
        // Force a socket binding error
        QUdpSocket anotherSocket;
        anotherSocket.bind(QHostAddress::Any, 6080);
        return socket()->bind(QHostAddress::Any, 6080);
    }
};

class QCoapQUdpConnectionSocketTests : public QCoapQUdpConnection
{
public:
    QCoapQUdpConnectionSocketTests() :
        QCoapQUdpConnection(*new QCoapQUdpConnectionSocketTestsPrivate)
    {
        createSocket();
    }

private:
    Q_DECLARE_PRIVATE(QCoapQUdpConnectionSocketTests)
};

class QCoapClientForSocketErrorTests : public QCoapClient
{
public:
    QCoapClientForSocketErrorTests()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        privateClient->setConnection(new QCoapQUdpConnectionSocketTests());
    }

    QCoapQUdpConnection *connection()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return qobject_cast<QCoapQUdpConnection*>(privateClient->connection);
    }
};

class QCoapClientForTests : public QCoapClient
{
public:
    QCoapClientForTests() {}

    QCoapProtocol *protocol()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return privateClient->protocol;
    }
    QCoapQUdpConnection *connection()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return qobject_cast<QCoapQUdpConnection*>(privateClient->connection);
    }
};

class QCoapConnectionMulticastTests : public QCoapConnection
{
public:
    ~QCoapConnectionMulticastTests() override = default;

    void bind(const QString &host, quint16 port) override
    {
        Q_UNUSED(host)
        Q_UNUSED(port)
        // Do nothing
    }

    void writeData(const QByteArray &data, const QString &host, quint16 port) override
    {
        Q_UNUSED(data)
        Q_UNUSED(host)
        Q_UNUSED(port)
        // Do nothing
    }

    void close() override {}
};

class QCoapClientForMulticastTests : public QCoapClient
{
public:
    QCoapClientForMulticastTests()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        privateClient->setConnection(new QCoapConnectionMulticastTests());
    }

    QCoapConnection *connection()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return privateClient->connection;
    }
};

#endif

class Helper : public QObject
{
    Q_OBJECT
public:
    Helper() {}

public slots:
    void onError(QCoapReply *, QtCoap::Error error)
    {
        qWarning() << "Network error" << error << "occurred";
    }
};

void tst_QCoapClient::initTestCase()
{
#if defined(COAP_TEST_SERVER_IP) || defined(QT_TEST_SERVER)
    QVERIFY2(waitForHost(QUrl{testServerHost()}), "Failed to connect to Californium plugtest server.");
#if QT_CONFIG(dtls)
    QVERIFY2(waitForHost(QUrl{timeServerUrl()}, QtCoap::SecurityMode::Certificate),
             "Failed to connect to FreeCoAP sample time server.");
#endif
#endif
}

void tst_QCoapClient::incorrectUrls_data()
{
    qWarning("Expect warnings here...");
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get")        << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("post")       << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("put")        << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("delete")     << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("discover")   << QUrl("wrong://10.20.30.40:5683/test");
}

void tst_QCoapClient::incorrectUrls()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QScopedPointer<QCoapReply> reply;

    if (qstrcmp(QTest::currentDataTag(), "get") == 0)
        reply.reset(client.get(url));
    else if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply.reset(client.post(url));
    else if (qstrcmp(QTest::currentDataTag(), "put") == 0)
        reply.reset(client.put(url));
    else if (qstrcmp(QTest::currentDataTag(), "delete") == 0)
        reply.reset(client.deleteResource(url));
    else if (qstrcmp(QTest::currentDataTag(), "discover") == 0)
        reply.reset(client.discover(url));
    else {
        QString error = QLatin1String("Unrecognized method '") + QTest::currentDataTag() + "'";
        QFAIL(qPrintable(error));
    }

    QVERIFY2(reply.isNull(), "Request did not fail as expected.");
}

void tst_QCoapClient::methods_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QtCoap::Method>("method");
    QTest::addColumn<QtCoap::SecurityMode>("security");

    QTest::newRow("get")
            << QUrl(testServerResource())
            << QtCoap::Method::Get
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_no_port")
            << QUrl("coap://" + testServerHost() + "/test")
            << QtCoap::Method::Get
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_no_scheme_no_port")
            << QUrl(testServerHost() + "/test")
            << QtCoap::Method::Get
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_psk")
            << QUrl(testServerResource()) << QtCoap::Method::Get
            << QtCoap::SecurityMode::PreSharedKey;
    QTest::newRow("get_cert")
            << QUrl(timeServerUrl()) << QtCoap::Method::Get
            << QtCoap::SecurityMode::Certificate;
    QTest::newRow("post")
            << QUrl(testServerResource())
            << QtCoap::Method::Post
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("post_no_scheme_no_port")
            << QUrl(testServerHost() + "/test")
            << QtCoap::Method::Post
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("post_psk")
            << QUrl(testServerResource()) << QtCoap::Method::Post
            << QtCoap::SecurityMode::PreSharedKey;
    QTest::newRow("put")
            << QUrl(testServerResource())
            << QtCoap::Method::Put
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("put_no_scheme_no_port")
            << QUrl(testServerHost() + "/test")
            << QtCoap::Method::Put
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("put_psk")
            << QUrl(testServerResource()) << QtCoap::Method::Put
            << QtCoap::SecurityMode::PreSharedKey;
    QTest::newRow("delete")
            << QUrl(testServerResource())
            << QtCoap::Method::Delete
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("delete_no_scheme_no_port")
            << QUrl(testServerHost() + "/test")
            << QtCoap::Method::Delete
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("delete_psk")
            << QUrl(testServerResource()) << QtCoap::Method::Delete
            << QtCoap::SecurityMode::PreSharedKey;
}

void tst_QCoapClient::methods()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);
    QFETCH(QtCoap::Method, method);
    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    QCoapRequest request(url);

    QSignalSpy spyClientFinished(&client, &QCoapClient::finished);

    const QByteArray payload = "test payload";
    QScopedPointer<QCoapReply> reply;
    if (qstrncmp(QTest::currentDataTag(), "get", 3) == 0) {
        reply.reset(client.get(request));
    } else if (qstrncmp(QTest::currentDataTag(), "post", 4) == 0) {
        request.setPayload(payload);
        reply.reset(client.post(request));
    } else if (qstrncmp(QTest::currentDataTag(), "put", 3) == 0) {
        request.setPayload(payload);
        reply.reset(client.put(request));
    } else if (qstrncmp(QTest::currentDataTag(), "delete", 6) == 0) {
        reply.reset(client.deleteResource(request));
    } else {
        QString error = QLatin1String("Unrecognized method '") + QTest::currentDataTag() + "'";
        QFAIL(qPrintable(error));
    }

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
#ifdef QT_BUILD_INTERNAL
    QCOMPARE(reply->url(),
             QCoapRequestPrivate::adjustedUrl(url, client.isSecure()));
#endif
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QTRY_COMPARE(spyReplyFinished.size(), 1);
    QTRY_COMPARE(spyClientFinished.size(), 1);

    QByteArray replyData;
    if (!reply.isNull()) {
        replyData = reply->readAll();
        if (qstrncmp(QTest::currentDataTag(), "get", 3) == 0) {
            QVERIFY(!replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Content);
        } else if (qstrncmp(QTest::currentDataTag(), "post", 4) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Created);
            QCOMPARE(reply->request().payload(), payload);
        } else if (qstrncmp(QTest::currentDataTag(), "put", 3) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Changed);
            QCOMPARE(reply->request().payload(), payload);
        } else if (qstrncmp(QTest::currentDataTag(), "delete", 6) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Deleted);
        } else {
            QString error = QLatin1String("Unrecognized method '") + QTest::currentDataTag() + "'";
            QFAIL(qPrintable(error));
        }
    }
    QCOMPARE(reply->request().method(), method);
}

void tst_QCoapClient::separateMethod()
{
    CHECK_FOR_COAP_SERVER;

    QCoapClient client;
    QScopedPointer<QCoapReply> reply(client.get(QUrl(testServerUrl() + "/separate")));

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QTRY_COMPARE(spyReplyFinished.size(), 1);

    QByteArray replyData = reply->readAll();

    QVERIFY(!replyData.isEmpty());
    QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Content);
}

void tst_QCoapClient::removeReply()
{
    CHECK_FOR_COAP_SERVER;

    QCoapClient client;
    QCoapReply *reply = client.get(QUrl(testServerResource()));
    QVERIFY2(reply != nullptr, "Request failed unexpectedly");
    connect(reply, &QCoapReply::finished, this, [reply]() { reply->deleteLater(); });

    try {
        QEventLoop eventLoop;
        QTimer::singleShot(2000, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    } catch (...) {
        QFAIL("Exception occurred after destroying the QCoapReply");
    }
}

void tst_QCoapClient::setBlockSize_data()
{
    QTest::addColumn<int>("blockSizeSet");
    QTest::addColumn<int>("blockSizeExpected");

    QTest::newRow("valid_size_0")       << 0 << 0;
    QTest::newRow("valid_size_16")      << 16 << 16;
    QTest::newRow("valid_size_1024")    << 1024 << 1024;
    QTest::newRow("invalid_size_8")     << 8 << 0;
    QTest::newRow("invalid_size_350")   << 350 << 0;
    QTest::newRow("invalid_size_2048")  << 2048 << 0;
}

void tst_QCoapClient::setBlockSize()
{
#ifdef QT_BUILD_INTERNAL
    QFETCH(int, blockSizeSet);
    QFETCH(int, blockSizeExpected);

    QCoapClientForTests client;
    client.setBlockSize(static_cast<quint16>(blockSizeSet));

    QEventLoop eventLoop;
    QTimer::singleShot(1000, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QCOMPARE(client.protocol()->blockSize(), blockSizeExpected);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapClient::requestWithQIODevice_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("post") << QUrl(testServerResource());
    QTest::newRow("put") << QUrl(testServerResource());
}

void tst_QCoapClient::requestWithQIODevice()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);

    QCoapClient client;
    QCoapRequest request(url);

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    buffer.write("Some data");

    QScopedPointer<QCoapReply> reply;
    if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply.reset(client.post(request, &buffer));
    else
        reply.reset(client.put(request, &buffer));

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QTRY_COMPARE(spyReplyFinished.size(), 1);

    QByteArray replyData = reply->readAll();

    if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Created);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->responseCode(), QtCoap::ResponseCode::Changed);
    }
}

void tst_QCoapClient::multipleRequests_data()
{
    QTest::addColumn<QtCoap::SecurityMode>("security");

    QTest::newRow("multiple_requests") << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("multiple_requests_secure") << QtCoap::SecurityMode::PreSharedKey;
}

void tst_QCoapClient::multipleRequests()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    QUrl url = QUrl(testServerResource());
    QSignalSpy spyClientFinished(&client, &QCoapClient::finished);

    const uint8_t requestCount = 4;
    QList<QSharedPointer<QCoapReply>> replies;
    QList<QSharedPointer<QSignalSpy>> signalSpies;
    for (uint8_t i = 0; i < requestCount; ++i) {
        QCoapRequest request;
        const auto token = "token" + QByteArray::number(i);
        request.setToken(token);
        request.setUrl(url);

        QSharedPointer<QCoapReply> reply(client.get(request));
        const auto errorMsg = QStringLiteral("Request number %1 failed unexpectedly").arg(i);
        QVERIFY2(!reply.isNull(), qPrintable(errorMsg));
        replies.push_back(reply);

        QSharedPointer<QSignalSpy> signalSpy(
                new QSignalSpy(reply.data(), &QCoapReply::finished));
        signalSpies.push_back(signalSpy);
    }

    for (const auto &signalSpy : signalSpies)
        QTRY_COMPARE(signalSpy->size(), 1);
    QTRY_COMPARE(spyClientFinished.size(), 4);

    for (uint8_t i = 0; i < requestCount; ++i) {
        QCOMPARE(replies[i]->responseCode(), QtCoap::ResponseCode::Content);
        QByteArray replyData = replies[i]->readAll();
        const auto token = "token" + QByteArray::number(i);
        // The californium server now returns the hex token in uppercase
        QVERIFY(replyData.contains(token.toHex().toUpper()));
    }
}

void tst_QCoapClient::socketError()
{
#ifdef QT_BUILD_INTERNAL
    CHECK_FOR_COAP_SERVER;

    QCoapClientForSocketErrorTests client;
    QUrl url = QUrl(testServerResource());

    const auto connection = client.connection();
    QVERIFY2(connection, "Failed to get coap connection!");
    QUdpSocket *socket = connection->socket();
    QVERIFY2(socket, "Socket not properly created with connection");
    QSignalSpy spySocketError(socket, &QUdpSocket::errorOccurred);
    QScopedPointer<QCoapReply> reply(client.get(url));
    QSignalSpy spyClientError(&client, &QCoapClient::error);

    QTRY_COMPARE_WITH_TIMEOUT(spySocketError.size(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientError.size(), 1, 1000);
    QCOMPARE(qvariant_cast<QtCoap::Error>(spyClientError.first().at(1)),
             QtCoap::Error::AddressInUse);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}
void tst_QCoapClient::timeout_data()
{
    QTest::addColumn<uint>("timeout");
    QTest::addColumn<uint>("maximumRetransmitCount");

    QTest::newRow("2000/0") << 2000u << 0u;
    QTest::newRow("2000/2") << 2000u << 2u;
    QTest::newRow("4000/0") << 4000u << 0u;
}

void tst_QCoapClient::timeout()
{
#ifdef QT_BUILD_INTERNAL
    CHECK_FOR_COAP_SERVER;

    QFETCH(uint, timeout);
    QFETCH(uint, maximumRetransmitCount);

    QCoapClientForTests client;
    // Trigger a network timeout
    client.protocol()->setAckTimeout(timeout);
    client.protocol()->setAckRandomFactor(1);
    client.protocol()->setMaximumRetransmitCount(maximumRetransmitCount);
    QUrl url = QUrl("coap://192.0.2.0:5683/"); // Need an url that returns nothing

    QElapsedTimer timeoutTimer;
    timeoutTimer.start();
    QScopedPointer<QCoapReply> reply(
                client.get(QCoapRequest(url, QCoapMessage::Type::Confirmable)));
    QSignalSpy spyClientError(&client, &QCoapClient::error);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);
    QSignalSpy spyReplyAborted(reply.data(), &QCoapReply::aborted);
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    // Check timeout upper limit
    uint transmissions = maximumRetransmitCount + 1;

    // 10% Precision expected at least, plus timer precision
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.size(), 1, static_cast<int>(
                                  1.1 * client.protocol()->maximumTransmitWait() + 20 * transmissions));

    // Check timeout lower limit
    qint64 elapsedTime = timeoutTimer.elapsed();
    QString errorMessage = QString("Timeout was triggered after %1ms, while expecting about %2ms")
                           .arg(QString::number(elapsedTime),
                                QString::number(client.protocol()->maximumTransmitWait()));

    // 10% Precision expected at least, minus timer precision
    QVERIFY2(elapsedTime > 0.9 * client.protocol()->maximumTransmitWait() - 20 * transmissions,
             qPrintable(errorMessage));

    QCOMPARE(qvariant_cast<QtCoap::Error>(spyReplyError.first().at(1)),
             QtCoap::Error::TimeOut);
    QCOMPARE(spyReplyFinished.size(), 1);
    QCOMPARE(spyReplyAborted.size(), 0);
    QCOMPARE(spyClientError.size(), 1);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapClient::abort()
{
    CHECK_FOR_COAP_SERVER;

    QCoapClient client;
    QUrl url = QUrl(testServerUrl() + "/large");

    QScopedPointer<QCoapReply> reply(client.get(url));
    QVERIFY(!reply.isNull());
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QSignalSpy spyReplyAborted(reply.data(), &QCoapReply::aborted);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);

    reply->abortRequest();

    QEventLoop eventLoop;
    QTimer::singleShot(1000, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QCOMPARE(spyReplyAborted.size(), 1);
    QCOMPARE(spyReplyFinished.size(), 1);
    QCOMPARE(spyReplyError.size(), 0);
}

void tst_QCoapClient::blockwiseReply_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<QByteArray>("replyData");
    QTest::addColumn<QtCoap::SecurityMode>("security");

    QByteArray data;
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 1 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 2 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 3 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 4 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 5 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");

    QTest::newRow("get_large")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_large_secure")
            << QUrl(testServerHost() + "/large")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::SecurityMode::PreSharedKey;
    QTest::newRow("get_large_separate")
            << QUrl(testServerHost() + "/large-separate")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_large_confirmable")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Type::Confirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_large_confirmable_secure")
            << QUrl(testServerHost() + "/large")
            << QCoapMessage::Type::Confirmable
            << data
            << QtCoap::SecurityMode::PreSharedKey;
    QTest::newRow("get_large_separate_confirmable")
            << QUrl(testServerUrl() + "/large-separate")
            << QCoapMessage::Type::Confirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_large_16bits")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("get_large_16bits_confirmable")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Type::Confirmable
            << data
            << QtCoap::SecurityMode::NoSecurity;
}

void tst_QCoapClient::blockwiseReply()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::Type, type);
    QFETCH(QByteArray, replyData);
    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    QCoapRequest request(url);

    if (qstrncmp(QTest::currentDataTag(), "get_large_16bits", 16) == 0)
        client.setBlockSize(16);

    request.setType(type);
    QScopedPointer<QCoapReply> reply(client.get(request));
    QVERIFY(!reply.isNull());
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);
    Helper helper;
    connect(reply.data(), &QCoapReply::error, &helper, &Helper::onError);

    QCOMPARE(spyReplyError.size(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.size(), 1, 30000);
    QCOMPARE(reply->readAll(), replyData);
}

void tst_QCoapClient::blockwiseRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<QByteArray>("requestData");
    QTest::addColumn<QtCoap::ResponseCode>("responseCode");
    QTest::addColumn<QByteArray>("replyData");
    QTest::addColumn<QtCoap::SecurityMode>("security");

    QByteArray data;
    const char alphabet[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    for (int i = 3; i-- > 0; )
        data.append(alphabet);

    QTest::newRow("large_post_empty_reply")
            << QUrl(testServerUrl() + "/query")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::ResponseCode::MethodNotAllowed
            << QByteArray()
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("large_post_large_reply")
            << QUrl(testServerUrl() + "/large-post")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::ResponseCode::Changed
            << data.toUpper()
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("large_post_large_reply_secure")
            << QUrl(testServerHost() + "/large-post")
            << QCoapMessage::Type::NonConfirmable
            << data
            << QtCoap::ResponseCode::Changed
            << data.toUpper()
            << QtCoap::SecurityMode::PreSharedKey;
}

void tst_QCoapClient::blockwiseRequest()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::Type, type);
    QFETCH(QByteArray, requestData);
    QFETCH(QtCoap::ResponseCode, responseCode);
    QFETCH(QByteArray, replyData);
    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    client.setBlockSize(16);

    QCoapRequest request(url);
    request.setType(type);
    request.addOption(QCoapOption::ContentFormat);

    QScopedPointer<QCoapReply> reply(client.post(request, requestData));
    QVERIFY(!reply.isNull());
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.size(), 1, 30000);

    QByteArray dataReply = reply->readAll();
    QCOMPARE(dataReply, replyData);
    QCOMPARE(reply->responseCode(), responseCode);
}

void tst_QCoapClient::discover_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<int>("resourceNumber");
    QTest::addColumn<QtCoap::SecurityMode>("security");

    // Californium test server exposes 31 resources
    QTest::newRow("discover")
            << QUrl(testServerHost())
            << 31
            << QtCoap::SecurityMode::NoSecurity;
    QTest::newRow("discover_secure")
            << QUrl(testServerHost())
            << 31
            << QtCoap::SecurityMode::PreSharedKey;
}

void tst_QCoapClient::discover()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);
    QFETCH(int, resourceNumber);
    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    QScopedPointer<QCoapResourceDiscoveryReply> resourcesReply(client.discover(url)); // /.well-known/core
    QVERIFY(!resourcesReply.isNull());
    QSignalSpy spyReplyFinished(resourcesReply.data(), &QCoapReply::finished);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.size(), 1, 30000);

    const auto discoverUrl = QUrl(url.toString() + "/.well-known/core");
#ifdef QT_BUILD_INTERNAL
    QCOMPARE(resourcesReply->url(),
             QCoapRequestPrivate::adjustedUrl(discoverUrl, client.isSecure()));
#endif
    QCOMPARE(resourcesReply->resources().size(), resourceNumber);
    QCOMPARE(resourcesReply->request().method(), QtCoap::Method::Get);

    //! TODO Test discovery content too
}

void tst_QCoapClient::observe_data()
{
    qWarning("Observe tests may take some time, don't forget to raise Tests timeout in settings.");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::Type>("type");
    QTest::addColumn<QtCoap::SecurityMode>("security");

    QTest::newRow("observe")
            << QUrl(testServerUrl() + "/obs")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_secure")
            << QUrl(testServerHost() + "/obs")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::PreSharedKey;

    QTest::newRow("observe_no_scheme_no_port")
            << QUrl(testServerHost() + "/obs")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_confirmable")
            << QUrl(testServerUrl() + "/obs")
            << QCoapMessage::Type::Confirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_receive")
            << QUrl(testServerUrl() + "/obs-non")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_receive_confirmable")
            << QUrl(testServerUrl() + "/obs-non")
            << QCoapMessage::Type::Confirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_large")
            << QUrl(testServerUrl() + "/obs-large")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_large_secure")
            << QUrl(testServerHost() + "/obs-large")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::PreSharedKey;

    QTest::newRow("observe_large_confirmable")
            << QUrl(testServerUrl() + "/obs-large")
            << QCoapMessage::Type::Confirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_pumping")
            << QUrl(testServerUrl() + "/obs-pumping")
            << QCoapMessage::Type::NonConfirmable
            << QtCoap::SecurityMode::NoSecurity;

    QTest::newRow("observe_pumping_confirmable")
            << QUrl(testServerUrl() + "/obs-pumping")
            << QCoapMessage::Type::Confirmable
            << QtCoap::SecurityMode::NoSecurity;
}

void tst_QCoapClient::observe()
{
    CHECK_FOR_COAP_SERVER;

    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::Type, type);
    QFETCH(QtCoap::SecurityMode, security);

    QCoapClientForSecurityTests client(security);
    if (client.securitySetupMissing())
        QSKIP("Skipping this test, security is not configured properly");

    QCoapRequest request(url);

    request.setType(type);
    QSharedPointer<QCoapReply> reply(client.observe(request),
                                     &QObject::deleteLater);
    QVERIFY(!reply.isNull());
    QSignalSpy spyReplyNotified(reply.data(), &QCoapReply::notified);
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.size(), 3, 30000);
    client.cancelObserve(reply.data());
#ifdef QT_BUILD_INTERNAL
    QCOMPARE(reply->url(),
             QCoapRequestPrivate::adjustedUrl(url, client.isSecure()));
#endif
    QCOMPARE(reply->request().method(), QtCoap::Method::Get);

    QVERIFY2(!spyReplyNotified.wait(7000), "'Notify' signal received after cancelling observe");
    QCOMPARE(spyReplyFinished.size(), 1);

    for (QList<QVariant> receivedSignals : std::as_const(spyReplyNotified)) {
        QRegularExpression regexp(QStringLiteral("..:..:.."));
        QByteArray payload = receivedSignals.at(1).value<QCoapMessage>().payload();
        QString error = QString("Invalid payload for 'notified' signal: %1").arg(QString(payload));
        QVERIFY2(regexp.match(payload).hasMatch(), qPrintable(error));
    }
}

void tst_QCoapClient::confirmableMulticast()
{
    QCoapClient client;
    const auto reply = client.get(QCoapRequest("224.0.1.187", QCoapMessage::Type::Confirmable));
    QVERIFY2(!reply, "Confirmable multicast request didn't fail as expected.");
}

void tst_QCoapClient::multicast()
{
#ifdef QT_BUILD_INTERNAL
    QCoapClientForMulticastTests client;
    QCoapRequest request = QCoapRequest(QUrl("224.0.1.187"));
    request.setToken("abc");
    QCoapReply *reply = client.get(request);
    QVERIFY(reply);

    QHostAddress host0("10.20.30.40");
    QHostAddress host1("10.20.30.41");

    // Simulate sending unicast responses to the multicast request
    emit client.connection()->readyRead("SE\xAD/abc\xC0\xFFReply0", host0);
    emit client.connection()->readyRead("SE\xAD/abc\xC0\xFFReply1", host1);

    QSignalSpy spyMulticastResponse(&client, &QCoapClient::responseToMulticastReceived);
    QTRY_COMPARE(spyMulticastResponse.size(), 2);

    QCoapMessage message0 = qvariant_cast<QCoapMessage>(spyMulticastResponse.at(0).at(1));
    QCOMPARE(message0.payload(), "Reply0");
    QHostAddress sender0 = qvariant_cast<QHostAddress>(spyMulticastResponse.at(0).at(2));
    QCOMPARE(sender0, host0);

    QCoapMessage message1 = qvariant_cast<QCoapMessage>(spyMulticastResponse.at(1).at(1));
    QCOMPARE(message1.payload(), "Reply1");
    QHostAddress sender1 = qvariant_cast<QHostAddress>(spyMulticastResponse.at(1).at(2));
    QCOMPARE(sender1, host1);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapClient::multicast_blockwise()
{
#ifdef QT_BUILD_INTERNAL
    QCoapClientForMulticastTests client;
    QCoapRequest request = QCoapRequest(QUrl("224.0.1.187"));
    request.setToken("abc");
    QCoapReply *reply = client.get(request);
    QVERIFY(reply);

    QHostAddress host0("10.20.30.40");
    QHostAddress host1("10.20.30.41");

    // Simulate blockwise transfer responses coming from two different hosts
    emit client.connection()->readyRead("SE#}abc\xC0\xB1\x1D\xFFReply1", host0);
    emit client.connection()->readyRead("SE#}abc\xC0\xB1\x1D\xFFReply3", host1);
    emit client.connection()->readyRead("SE#~abc\xC0\xB1%\xFFReply2", host0);
    emit client.connection()->readyRead("SE#~abc\xC0\xB1%\xFFReply4", host1);

    QSignalSpy spyMulticastResponse(&client, &QCoapClient::responseToMulticastReceived);
    QTRY_COMPARE(spyMulticastResponse.size(), 2);

    QCoapMessage message0 = qvariant_cast<QCoapMessage>(spyMulticastResponse.at(0).at(1));
    QCOMPARE(message0.payload(), "Reply1Reply2");
    QHostAddress sender0 = qvariant_cast<QHostAddress>(spyMulticastResponse.at(0).at(2));
    QCOMPARE(sender0, host0);

    QCoapMessage message1 = qvariant_cast<QCoapMessage>(spyMulticastResponse.at(1).at(1));
    QCOMPARE(message1.payload(), "Reply3Reply4");
    QHostAddress sender1 = qvariant_cast<QHostAddress>(spyMulticastResponse.at(1).at(2));
    QCOMPARE(sender1, host1);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapClient::setMinimumTokenSize_data()
{
    QTest::addColumn<int>("minTokenSize");
    QTest::addColumn<int>("expectedMinSize");

    QTest::newRow("in_range") << 6 << 6;
    QTest::newRow("out_of_range_small") << 0 << 4;
    QTest::newRow("out_of_range_big") << 9 << 4;
}

void noMessageOutput(QtMsgType, const QMessageLogContext &, const QString &) {}

void tst_QCoapClient::setMinimumTokenSize()
{
#ifdef QT_BUILD_INTERNAL
    // Don't show warning messages for the out of range values
    qInstallMessageHandler(noMessageOutput);

    QFETCH(int, minTokenSize);
    QFETCH(int, expectedMinSize);

    const int maxSize = 8;

    for (int i = 0; i < 20; ++i) {
        QCoapClientForSocketErrorTests client;
        client.setMinimumTokenSize(minTokenSize);

        // With QCoapClientForSocketErrorTests the request will fail, but it doesn't matter,
        // we are interested only in the generated token.
        QSignalSpy spyClientError(&client, &QCoapClient::error);

        QScopedPointer<QCoapReply> reply;
        reply.reset(client.get(QCoapRequest("127.0.0.1")));

        QTRY_COMPARE_WITH_TIMEOUT(spyClientError.size(), 1, 300);
        QVERIFY(reply->request().tokenLength() >= expectedMinSize);
        QVERIFY(reply->request().tokenLength() <= maxSize);
    }
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
