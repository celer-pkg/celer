// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtHttpServer/qabstracthttpserver.h>

#if defined(QT_WEBSOCKETS_LIB)
#  include <QtWebSockets/qwebsocket.h>
#endif

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtTest/qtesteventloop.h>

#include <QtCore/qregularexpression.h>
#include <QtCore/qurl.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserverresponder.h>
#include <QtHttpServer/qhttpserverwebsocketupgraderesponse.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#if QT_CONFIG(ssl)
#ifdef QT_BUILD_INTERNAL
#include <QtNetwork/private/http2frames_p.h>
#include <QtNetwork/private/http2protocol_p.h>
#endif
#include <QtNetwork/qhttp2configuration.h>
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslserver.h>
#include <QtNetwork/private/qhttp2connection_p.h>
#endif

#include <QtTest/private/qtesthelpers_p.h>

#if QT_CONFIG(ssl)

constexpr char g_privateKey[] = R"(-----BEGIN RSA PRIVATE KEY-----
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
-----END RSA PRIVATE KEY-----)";

constexpr char g_certificate[] = R"(-----BEGIN CERTIFICATE-----
MIIFszCCA5ugAwIBAgIUfpP54qSLfus/pFUIBDizbnrDjE4wDQYJKoZIhvcNAQEL
BQAwaDELMAkGA1UEBhMCRlIxDzANBgNVBAgMBkZyYW5jZTERMA8GA1UEBwwIR3Jl
bm9ibGUxFjAUBgNVBAoMDVF0Q29udHJpYnV0b3IxHTAbBgNVBAMMFHFodHRwc3Nl
cnZlcnRlc3QuY29tMCAXDTIyMDIwNzE0MzE0NVoYDzIyNjgwNzA3MTQzMTQ1WjBo
MQswCQYDVQQGEwJGUjEPMA0GA1UECAwGRnJhbmNlMREwDwYDVQQHDAhHcmVub2Js
ZTEWMBQGA1UECgwNUXRDb250cmlidXRvcjEdMBsGA1UEAwwUcWh0dHBzc2VydmVy
dGVzdC5jb20wggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC92u1m1Wq7
CIbXb693c6NFWJt0rY5vvcM45aBvmsiBD4HZTNw4VrTGyJHJz9tqb0IQSqCflgdK
f3kske83d1uwDy+sWohdOSKwUC4cmqyaxhlsAiIJfy3W2VxSZCwmaNnUgqGTfuWA
YwqRzbw6H8IdtOuEllZ1DKgFs1vwp6pIeR49i7T8Ey07+WuRcmNXoFeArwGA6ol8
6qsYKwG9b6ypnP0PnVkEFVvdgKsFk/bQ1UDnFlSo4XBLeOf72E7VeUlzCIUDdXNT
u5JW3bOPOFfSIfDoY5VuNHghK2of/LShr1H2ZBrSGbNDstxX02KCFfmQzw+yBP2L
peG/aeayOk+t4tiBeheqkNGsvQ9HQJK2za5xF1+EyFjxWI2hqtnXZnYJNo3GIc8b
M1g0k9pAbjSjcsRtDcY/uKBnyCj999JX6RklLmO4AcKxS81QDiC8V9VcvHOgbqp7
AtdIJeioPIn1PuDgvzVbX2GwHFWnyAOpUCIGemD4ZMtoLr1Y6b04xcRE5M6WKfuD
GVDSlk77b8ze4UWj+FeUgf8xvk08qWk0GqAumojC0Yy6C3sPH0mgzBa8QGXMyB7J
xiY1fBIex1DmTEVLm5i5kSuLrZeuT9ZaV6/88MJF1xJSGuNVI2i0VC94YpLALnPY
U4AhmKgDSY/GwGLFJpBjx+GJoLnJ1TDgXQIDAQABo1MwUTAdBgNVHQ4EFgQUK7Un
0JA3DBUVhclrm6pIZsO60U4wHwYDVR0jBBgwFoAUK7Un0JA3DBUVhclrm6pIZsO6
0U4wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAgEAuvSFAgfgurDT
/dbXuJ0O+FuGg4NOTNRil5ji3DnMzReIHpSiKiXu76PHHRFnlosvfAFOHlpYntun
LhbUAxim/iIgWZR33uzvqXMXBORZ0zffjy2SjGCW8ZJYyTmg9c0tc0jEjv7owtlU
m6tUXMOs9U0CzvEKLt0K0dMALaLkXtscuzEWA4PHVvnvTu0Wyjj/8n+DgYzY09kC
YF0lJfcG6bddDgspmYyFpULeGGP7+qwgGh4cVBtY5I4Htr3p7hDo6UGDF6AsMQZF
1CAEgBVRbJgI2GTnptpm9k3EFKwQ81z5O+NnP3ZsuuZ3CEVaPHyQf/POLAIhmZLt
0vS9qoRiS4uMUJDXz2kJFBOFHki073eMvHiKtlpYOlJXMQ4MkHCydjeeuhHcgUCq
ZDWuQMmq/8tMwf4YtvxYtXzAMVW9dM8BgWu2G8/JwPMGUGhLfKkHmc8dlQzGDe/W
K/uVHlJZNF4Y0eXVlq9DUhpvKOjGc8A208wQlsTUgPxljgJ2+4F3D+t0luc3h65m
25iw8eRGuYDoCQLG7u7MI0g8A0H+0h9Xrt8PQql86vmQhmTUhKfedVGOo2t2Bcfn
ignL7f4e1m2jh0oWTLhuP1hnVFN4KAKpVIJXhbEkH59cLCN6ARXiEHCM9rmK5Rgk
NQZlAZc2w1Ha9lqisaWWpt42QVhQM64=
-----END CERTIFICATE-----)";

#endif // QT_CONFIG(ssl)

QT_BEGIN_NAMESPACE

#if QT_CONFIG(ssl)
typedef std::unique_ptr<QSslSocket> QSslSocketPtr;
#endif

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

class tst_QAbstractHttpServer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void request_data();
    void request();
    void checkListenWarns();
    void websocket();
    void verifyWebSocketUpgrades_data();
    void verifyWebSocketUpgrades();
    void verifyWebSocketUpgradesGoesOutOfScope();
    void servers();
    void qtbug82053();
    void http2handshake();
    void http2request();
    void socketDisconnected();
    void keepAliveTimeout();

private:
#if QT_CONFIG(ssl)
    QSslSocketPtr createNewConnection(const QTcpServer *server);
    bool hasServerAlpn = false;
#endif // QT_CONFIG(ssl)
};

void tst_QAbstractHttpServer::initTestCase()
{
#if QT_CONFIG(ssl)
    hasServerAlpn = QSslSocket::supportedFeatures().contains(QSsl::SupportedFeature::ServerSideAlpn);
#endif // QT_CONFIG(ssl)
}

void tst_QAbstractHttpServer::request_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("query");

    QTest::addRow("127.0.0.1") << "127.0.0.1" << "/" << QString();
    QTest::addRow("0.0.0.0") << "0.0.0.0" << "/" << QString();
    QTest::addRow("localhost") << "localhost" << "/" << QString();
    QTest::addRow("localhost with query") << "localhost" << "/" << QString("key=value");
    QTest::addRow("0.0.0.0 path with spaces") << "0.0.0.0" << "/test test" << QString();
    QTest::addRow("0.0.0.0 path with spec spaces") << "0.0.0.0" << "/test%20test" << QString();
    QTest::addRow("127.0.0.1 path with spaces") << "127.0.0.1" << "/test test" << QString();
    QTest::addRow("127.0.0.1 path with spec spaces") << "127.0.0.1" << "/test%20test" << QString();
}

void tst_QAbstractHttpServer::request()
{
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(QString, query);

#if defined(Q_OS_WIN)
    if (host == "0.0.0.0"_L1)
        QSKIP("Windows has problems with 0.0.0.0");
#endif

    struct HttpServer : QAbstractHttpServer
    {
        QUrl url;
        QByteArray body;
        QHttpServerRequest::Method method = QHttpServerRequest::Method::Unknown;
        quint8 padding[4];

        bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder) override
        {
            method = request.method();
            url = request.url();
            body = request.body();
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;
    QTcpServer tcpServer;
    QVERIFY(tcpServer.listen());
    server.bind(&tcpServer);
    QNetworkAccessManager networkAccessManager;
    QUrl url(QStringLiteral("http://%1:%2%3")
             .arg(host)
             .arg(tcpServer.serverPort())
             .arg(path));
    if (!query.isEmpty())
        url.setQuery(query);
    const QNetworkRequest request(url);
    networkAccessManager.get(request);
    QTRY_COMPARE(server.method, QHttpServerRequest::Method::Get);
    QCOMPARE(server.url, url);
    QCOMPARE(server.body, QByteArray());
}

void tst_QAbstractHttpServer::checkListenWarns()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;
    QTcpServer tcpServer;
    QTest::ignoreMessage(QtWarningMsg,
                         QRegularExpression(QStringLiteral("The TCP server .* is not listening.")));
    server.bind(&tcpServer);
}

void tst_QAbstractHttpServer::websocket()
{
#if !defined(QT_WEBSOCKETS_LIB)
    QSKIP("This test requires WebSocket support");
#else
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;
    server.addWebSocketUpgradeVerifier( // Accept all websocket connections
            &server, [](const QHttpServerRequest &request) {
                Q_UNUSED(request);
                return QHttpServerWebSocketUpgradeResponse::accept();
            });
    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);
    auto makeWebSocket = [this, &tcpServer]() mutable {
        auto s = std::make_unique<QWebSocket>(QString::fromUtf8(""),
                                              QWebSocketProtocol::VersionLatest, this);
        const QUrl url(QString::fromLatin1("ws://localhost:%1").arg(tcpServer.serverPort()));
        s->open(url);
        return s;
    };

    // We have to send two requests to make sure that swapping between
    // QTcpSocket and QWebSockets works correctly
    auto clientSocket1 = makeWebSocket();
    auto clientSocket2 = makeWebSocket();
    QVERIFY(clientSocket1);
    QVERIFY(clientSocket2);

    QSignalSpy newConnectionSpy(&server, &HttpServer::newWebSocketConnection);
    QTRY_COMPARE(newConnectionSpy.size(), 2);

    std::unique_ptr<QWebSocket> serverSocket1(server.nextPendingWebSocketConnection());
    QVERIFY(serverSocket1);
    std::unique_ptr<QWebSocket> serverSocket2(server.nextPendingWebSocketConnection());
    QVERIFY(serverSocket2);

    QSignalSpy serverSocket1Spy(serverSocket1.get(), &QWebSocket::textMessageReceived);
    QSignalSpy serverSocket2Spy(serverSocket2.get(), &QWebSocket::textMessageReceived);
    QSignalSpy clientSocket1Spy(clientSocket1.get(), &QWebSocket::textMessageReceived);
    QSignalSpy clientSocket2Spy(clientSocket2.get(), &QWebSocket::textMessageReceived);

    // Check that all WebSockets are ready for read/write
    QTRY_VERIFY(serverSocket1->isValid());
    QTRY_VERIFY(serverSocket2->isValid());
    QTRY_VERIFY(clientSocket1->isValid());
    QTRY_VERIFY(clientSocket2->isValid());

    serverSocket1->sendTextMessage("Server to Client");
    serverSocket2->sendTextMessage("Server to Client");
    clientSocket1->sendTextMessage("Client to Server");
    clientSocket2->sendTextMessage("Client to Server");

    QTRY_COMPARE(clientSocket1Spy.size(), 1);
    QList<QVariant> clientSocket1Incoming = clientSocket1Spy.takeFirst();
    QCOMPARE(clientSocket1Incoming.at(0).typeId(), QMetaType::QString);
    QCOMPARE(clientSocket1Incoming.at(0).toString(), "Server to Client");

    QTRY_COMPARE(clientSocket2Spy.size(), 1);
    QList<QVariant> clientSocket2Incoming = clientSocket2Spy.takeFirst();
    QCOMPARE(clientSocket2Incoming.at(0).typeId(), QMetaType::QString);
    QCOMPARE(clientSocket2Incoming.at(0).toString(), "Server to Client");

    QTRY_COMPARE(serverSocket1Spy.size(), 1);
    QList<QVariant> serverSocket1Incoming = serverSocket1Spy.takeFirst();
    QCOMPARE(serverSocket1Incoming.at(0).typeId(), QMetaType::QString);
    QCOMPARE(serverSocket1Incoming.at(0).toString(), "Client to Server");

    QTRY_COMPARE(serverSocket2Spy.size(), 1);
    QList<QVariant> serverSocket2Incoming = serverSocket2Spy.takeFirst();
    QCOMPARE(serverSocket2Incoming.at(0).typeId(), QMetaType::QString);
    QCOMPARE(serverSocket2Incoming.at(0).toString(), "Client to Server");
#endif // defined(QT_WEBSOCKETS_LIB)
}

void tst_QAbstractHttpServer::verifyWebSocketUpgrades_data()
{
#if !defined(QT_WEBSOCKETS_LIB)
    QSKIP("This test requires WebSocket support");
#endif
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("useSslPort");
    QTest::addColumn<bool>("shouldSucceed");
    QTest::addColumn<bool>("missingHandlerExpected");

    QTest::addRow("allowed") << "ws://localhost:%1/allowed" << false << true << false;
    QTest::addRow("denied") << "ws://localhost:%1/denied" << false << false << false;
    QTest::addRow("no match") << "ws://localhost:%1/nomatch" << false << false << true;
#if QT_CONFIG(ssl)
    if (QSslSocket::supportsSsl()) {
        QTest::addRow("sslonly-without") << "ws://localhost:%1/ssl" << false << false << false;
        QTest::addRow("sslonly-with") << "wss://localhost:%1/ssl" << true << true << false;
    }
#endif
}

void tst_QAbstractHttpServer::verifyWebSocketUpgrades()
{
#if defined(QT_WEBSOCKETS_LIB)
    QFETCH(const QString, url);
    QFETCH(const bool, useSslPort);
    QFETCH(const bool, shouldSucceed);
    QFETCH(const bool, missingHandlerExpected);
    int port = 0;

    struct HttpServer : QAbstractHttpServer
    {
        HttpServer(bool missingExpected) : missingHandlerExpected(missingExpected) { }

        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }

        bool missingHandlerExpected = false;
        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(missingHandlerExpected);
        }
    } server(missingHandlerExpected);

    server.addWebSocketUpgradeVerifier(&server, [](const QHttpServerRequest &request) {
        if (request.url().path() == "/allowed"_L1)
            return QHttpServerWebSocketUpgradeResponse::accept();
        else
            return QHttpServerWebSocketUpgradeResponse::passToNext();
    });
    server.addWebSocketUpgradeVerifier(&server, [](const QHttpServerRequest &request) {
        // Explicitly deny
        if (request.url().path() == "/denied"_L1)
            return QHttpServerWebSocketUpgradeResponse::deny();
        else
            return QHttpServerWebSocketUpgradeResponse::passToNext();
    });
#if QT_CONFIG(ssl)
    server.addWebSocketUpgradeVerifier(&server, [](const QHttpServerRequest &request) {
        if (request.url().path() == "/ssl"_L1) {
            // The QSslConfiguration of a request is null if connection is not using SSL
            if (request.sslConfiguration().isNull())
                return QHttpServerWebSocketUpgradeResponse::deny();
            else
                return QHttpServerWebSocketUpgradeResponse::accept();
        }
        return QHttpServerWebSocketUpgradeResponse::passToNext();
    });
#endif // QT_CONFIG(ssl)
    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);
#if QT_CONFIG(ssl)
    if (QTestPrivate::isSecureTransportBlockingTest()) {
        // We were built with SDK below 15, but a file-based keychains are not working anymore on macOS 15...
        QSKIP("This test will block in keychain access");
    }

    QSslServer sslServer;
    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    sslConfiguration.setLocalCertificate(QSslCertificate(QByteArray(g_certificate)));
    sslConfiguration.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
    sslServer.setSslConfiguration(sslConfiguration);
    sslServer.listen();
    server.bind(&sslServer);
    if (useSslPort) {
        port = sslServer.serverPort();
    } else {
        port = tcpServer.serverPort();
    }
#else
    QVERIFY(!useSslPort);
    port = tcpServer.serverPort();
#endif

    auto makeWebSocket = [&, this]() mutable {
        auto s = std::make_unique<QWebSocket>(QString::fromUtf8(""),
                                              QWebSocketProtocol::VersionLatest, this);
        const QUrl qurl(url.arg(port));
#if QT_CONFIG(ssl)
        if (useSslPort) {
            const QList<QSslError> expectedSslErrors = {
                QSslError(QSslError::SelfSignedCertificate, QSslCertificate(g_certificate)),
                // Non-OpenSSL backends are not able to report a specific error code
                // for self-signed certificates.
                QSslError(QSslError::CertificateUntrusted, QSslCertificate(g_certificate)),
                QSslError(QSslError::HostNameMismatch, QSslCertificate(g_certificate)),
            };
            s->ignoreSslErrors(expectedSslErrors);
        }
#endif // QT_CONFIG(ssl)
        s->open(qurl);
        return s;
    };

    auto s1 = makeWebSocket();
    auto s2 = makeWebSocket();

    QSignalSpy newConnectionSpy(&server, &HttpServer::newWebSocketConnection);
    if (shouldSucceed) {
        QTRY_COMPARE(newConnectionSpy.size(), 2);
        server.nextPendingWebSocketConnection();
        server.nextPendingWebSocketConnection();
    } else {
        QTest::qWait(useSslPort ? 2s : 1s);
        QCOMPARE(newConnectionSpy.size(), 0);
    }
#endif // defined(QT_WEBSOCKETS_LIB)
}

void tst_QAbstractHttpServer::verifyWebSocketUpgradesGoesOutOfScope()
{
#if defined(QT_WEBSOCKETS_LIB)
    const QString url("ws://localhost:%1/allowed");
    int port = 0;

    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }
        bool missingHandlerExpected = false;
        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(missingHandlerExpected);
        }

    } server;

    auto makeWebSocket = [&, this]() mutable {
        auto s = std::make_unique<QWebSocket>(QString::fromUtf8(""),
                                              QWebSocketProtocol::VersionLatest, this);
        const QUrl qurl(url.arg(port));

        s->open(qurl);
        return s;
    };

    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);
    port = tcpServer.serverPort();

    { // Verifier is in scope
        struct WebSocketVerifier : public QObject
        {
            QHttpServerWebSocketUpgradeResponse verify(const QHttpServerRequest &request)
            {
                if (request.url().path() == "/allowed"_L1)
                    return QHttpServerWebSocketUpgradeResponse::accept();
                else
                    return QHttpServerWebSocketUpgradeResponse::passToNext();
            }
        } verifier;

        server.addWebSocketUpgradeVerifier(&verifier, &WebSocketVerifier::verify);

        auto s1 = makeWebSocket();
        auto s2 = makeWebSocket();

        QSignalSpy newConnectionSpy(&server, &HttpServer::newWebSocketConnection);
        QTRY_COMPARE(newConnectionSpy.size(), 2); // Success
        server.nextPendingWebSocketConnection();
        server.nextPendingWebSocketConnection();
    }

    { // Verifier is out of scope
        server.missingHandlerExpected = true;

        auto s1 = makeWebSocket();
        auto s2 = makeWebSocket();

        QSignalSpy newConnectionSpy(&server, &HttpServer::newWebSocketConnection);
        QTest::qWait(2s);
        QCOMPARE(newConnectionSpy.size(), 0); // Failure
    }
#endif // defined(QT_WEBSOCKETS_LIB)
}

void tst_QAbstractHttpServer::servers()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;
    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);
    QTcpServer tcpServer2;
    tcpServer2.listen();
    server.bind(&tcpServer2);
    QTRY_COMPARE(server.servers().size(), 2);
    QTRY_COMPARE(server.serverPorts().size(), 2);
    QTRY_COMPARE(server.servers().first(), &tcpServer);
    QTRY_COMPARE(server.serverPorts().first(), tcpServer.serverPort());
    QTRY_COMPARE(server.servers().last(), &tcpServer2);
    QTRY_COMPARE(server.serverPorts().last(), tcpServer2.serverPort());
}

void tst_QAbstractHttpServer::qtbug82053()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool wasConnectRequest{false};
        bool handleRequest(const QHttpServerRequest &req, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            wasConnectRequest = (req.method() == QHttpServerRequest::Method::Connect);
            return false;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override { }
    } server;
    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, tcpServer.serverPort());
    client.waitForConnected();
    client.write("CONNECT / HTTP/1.1\n\n");
    client.waitForBytesWritten();
    QTest::qWait(0);
    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
    QTRY_VERIFY(server.wasConnectRequest);
}

#if QT_CONFIG(ssl)
QSslSocketPtr tst_QAbstractHttpServer::createNewConnection(const QTcpServer * server)
{
    QSslSocketPtr socketPtr = std::make_unique<QSslSocket>();

    QSslConfiguration clientConfig = socketPtr->sslConfiguration();
    clientConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
    socketPtr->setSslConfiguration(clientConfig);
    socketPtr->connectToHostEncrypted(server->serverAddress().toString(),
                                      server->serverPort());
    socketPtr->waitForConnected();

    // Expected errors
    connect(socketPtr.get(), &QSslSocket::sslErrors, this , [socket = socketPtr.get()]() {
        socket->ignoreSslErrors();
    });

    QTestEventLoop loop;
    int waitFor = 2;
    connect(socketPtr.get(), &QSslSocket::encrypted, &loop, [&loop, &waitFor]() {
        if (!--waitFor)
            loop.exitLoop();
    });
    connect(server, &QTcpServer::pendingConnectionAvailable, &loop, [&loop, &waitFor]() {
        if (!--waitFor)
            loop.exitLoop();
    });
    loop.enterLoop(5);

    return socketPtr;
}
#endif // QT_CONFIG(ssl)

void tst_QAbstractHttpServer::http2handshake()
{
#if QT_CONFIG(ssl)
    if (!hasServerAlpn)
        QSKIP("Server-side ALPN is unsupported, skipping test");

    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &) override
        { return false; }
        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override { }
    } server;

    auto sslserver = std::make_unique<QSslServer>();
    QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
    serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
    serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
    serverConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
    sslserver->setSslConfiguration(serverConfig);
    QVERIFY2(sslserver->listen(QHostAddress::LocalHost), "HTTPS server listen failed");
    QVERIFY2(server.bind(sslserver.get()), "HTTPS server bind failed");
    sslserver.release();

    const auto serverPtr = server.servers().constFirst();
    QSslSocketPtr client1 = createNewConnection(serverPtr);
    // Check client socket
    QVERIFY(client1->isEncrypted());
    QCOMPARE(client1->state(), QAbstractSocket::ConnectedState);

    // HTTP/2 Client Preface
    client1->write("PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n");
    client1->waitForBytesWritten();

    QTestEventLoop loop;
    connect(client1.get(), &QAbstractSocket::readyRead, &loop, &QTestEventLoop::exitLoop);
    loop.enterLoop(5);

    // HTTP/2 Server Preface
    QVERIFY(client1->bytesAvailable() != 0);
#ifdef QT_BUILD_INTERNAL
    auto serverPrefaceResult = client1->readAll();
    auto serverPreface = Http2::configurationToSettingsFrame(QHttp2Configuration()).buffer;
    auto serverPrefaceExpected = QByteArrayView(reinterpret_cast<const char *>(&serverPreface[0]),
                                 serverPreface.size());
    QCOMPARE(serverPrefaceResult, serverPrefaceExpected);

    // Check settings send in server's preface
    QHttp2Configuration h2config;
    constexpr quint32 MaxFrameSize = 16394;
    constexpr bool ServerPushEnabled = true;
    constexpr quint32 StreamReceiveWindowSize = 50000;
    h2config.setMaxFrameSize(MaxFrameSize);
    h2config.setServerPushEnabled(ServerPushEnabled);
    h2config.setStreamReceiveWindowSize(StreamReceiveWindowSize);
    server.setHttp2Configuration(h2config);

    QSslSocketPtr client2 = createNewConnection(serverPtr);
    QVERIFY(client2->isEncrypted());
    QCOMPARE(client2->state(), QAbstractSocket::ConnectedState);

    // HTTP/2 Client Preface
    client2->write("PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n");
    client2->waitForBytesWritten();

    connect(client2.get(), &QAbstractSocket::readyRead, &loop, &QTestEventLoop::exitLoop);
    loop.enterLoop(5);

    // HTTP/2 Server Preface
    QVERIFY(client2->bytesAvailable() != 0);
    serverPrefaceResult = client2->readAll();
    serverPreface = Http2::configurationToSettingsFrame(h2config).buffer;
    serverPrefaceExpected = QByteArrayView(reinterpret_cast<const char *>(&serverPreface[0]),
                                                serverPreface.size());
    QCOMPARE(serverPrefaceResult, serverPrefaceExpected);
#endif // QT_BUILD_INTERNAL

#else
    QSKIP("TLS/SSL is not available, skipping test");
#endif // QT_CONFIG(ssl)
}

void tst_QAbstractHttpServer::http2request()
{
#if QT_CONFIG(ssl)
    if (!hasServerAlpn)
        QSKIP("Server-side ALPN is unsupported, skipping test");

    struct HttpServer : QAbstractHttpServer
    {
        QUrl url;
        QHttpHeaders headers;
        QByteArray body = {"hello client"};
        bool receivedRequest = false;

        bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder) override
        {
            receivedRequest = true;
            url = request.url();

            headers.append("name1", "value1");
            headers.append("name2", "value2");
            responder.write(body, headers, QHttpServerResponder::StatusCode::Ok);

            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;

    auto sslserver = std::make_unique<QSslServer>();
    QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
    serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
    serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
    serverConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
    sslserver->setSslConfiguration(serverConfig);
    QVERIFY2(sslserver->listen(QHostAddress::LocalHost), "HTTPS server listen failed");
    quint16 port = sslserver->serverPort();
    QVERIFY2(server.bind(sslserver.get()), "HTTPS server bind failed");
    sslserver.release();

    const auto serverPtr = server.servers().constFirst();
    QNetworkAccessManager manager;
    const QList<QSslError> expectedSslErrors = {
        QSslError(QSslError::SelfSignedCertificate, QSslCertificate(g_certificate)),
        QSslError(QSslError::CertificateUntrusted, QSslCertificate(g_certificate)),
        QSslError(QSslError::HostNameMismatch, QSslCertificate(g_certificate)),
    };
    connect(&manager, &QNetworkAccessManager::sslErrors,
            this, [expectedSslErrors](QNetworkReply *reply, const QList<QSslError> &errors) {
                for (const auto &error: errors) {
                    if (!expectedSslErrors.contains(error))
                        qCritical() << "Got unexpected ssl error:" << error << error.certificate();
                }
                reply->ignoreSslErrors(expectedSslErrors);
            });

    QUrl url;
    url.setScheme("https");
    url.setHost(serverPtr->serverAddress().toString());
    url.setPort(port);
    url.setPath("/foo");
    QNetworkRequest req(url);
    QNetworkReply *reply = manager.get(req);
    QTRY_VERIFY(reply->isFinished());

    const QVariant http2Used = reply->attribute(QNetworkRequest::Http2WasUsedAttribute);
    QVERIFY(http2Used.isValid());
    QVERIFY(http2Used.toBool());

    QCOMPARE(server.receivedRequest, true);
    QCOMPARE(server.url, url);

    QCOMPARE(reply->readAll(), server.body);
    QHttpHeaders expectedHeaders = server.headers;
    expectedHeaders.append(QHttpHeaders::WellKnownHeader::ContentLength,
                           QByteArray::number(server.body.size()));
    QCOMPARE(reply->headers().toListOfPairs(), expectedHeaders.toListOfPairs());
#else
    QSKIP("TLS/SSL is not available, skipping test");
#endif // QT_CONFIG(ssl)
}

void tst_QAbstractHttpServer::socketDisconnected()
{
#if QT_CONFIG(ssl)
    if (!hasServerAlpn)
        QSKIP("Server-side ALPN is unsupported, skipping test");

    struct HttpServer : QAbstractHttpServer
    {
        bool handlingStarted = false;
        bool handlingFinished = false;

        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            handlingStarted = true;

            QTestEventLoop loop;
            loop.enterLoopMSecs(500);

            responder.write(QHttpServerResponder::StatusCode::Ok);

            handlingFinished = true;
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;

    auto sslserver = std::make_unique<QSslServer>();
    QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
    serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
    serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
    serverConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
    sslserver->setSslConfiguration(serverConfig);
    QVERIFY2(sslserver->listen(QHostAddress::LocalHost), "HTTPS server listen failed");
    QVERIFY2(server.bind(sslserver.get()), "HTTPS server bind failed");
    sslserver.release();

    const auto serverPtr = server.servers().constFirst();
    QSslSocketPtr socket = createNewConnection(serverPtr);
    QVERIFY(socket->isEncrypted());
    QCOMPARE(socket->state(), QAbstractSocket::ConnectedState);

    QHttp2Connection *connection = QHttp2Connection::createDirectConnection(socket.get(), {});

    QSignalSpy settingsFrameReceivedSpy{ connection, &QHttp2Connection::settingsFrameReceived };
    connect(socket.get(), &QIODevice::readyRead, connection, &QHttp2Connection::handleReadyRead);
    connection->handleReadyRead();

    auto stream = connection->createStream().unwrap();
    QVERIFY(stream);

    QVERIFY(settingsFrameReceivedSpy.wait());

    // disconnect while request is being processed on server
    QTimer timer;
    connect(&timer, &QTimer::timeout, this, [&server, &socket, &timer] {
        if (server.handlingStarted) {
            socket->disconnectFromHost();
            timer.stop();
        }
    });
    timer.start(100);

    HPack::HttpHeader headers = HPack::HttpHeader{
       { ":authority", "example.com" },
       { ":method", "GET" },
       { ":path", "/" },
       { ":scheme", "https" },
    };
    stream->sendHEADERS(headers, true);

    QTRY_VERIFY(server.handlingFinished);

#else
    QSKIP("TLS/SSL is not available, skipping test");
#endif // QT_CONFIG(ssl)
}

void tst_QAbstractHttpServer::keepAliveTimeout()
{
    struct HttpServer : QAbstractHttpServer
    {
        bool handleRequest(const QHttpServerRequest &, QHttpServerResponder &responder) override
        {
            auto _responder = std::move(responder);
            return true;
        }

        void missingHandler(const QHttpServerRequest &, QHttpServerResponder &) override
        {
            Q_ASSERT(false);
        }
    } server;

    QTcpServer tcpServer;
    tcpServer.listen();
    server.bind(&tcpServer);

    QHttpServerConfiguration config;
    config.setKeepAliveTimeout(2s);
    server.setConfiguration(config);

    // Test HTTP/1 connection

    QTcpSocket client;
    client.connectToHost(QHostAddress::LocalHost, tcpServer.serverPort());
    QVERIFY(client.waitForConnected());

    client.write("GET / HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
    client.waitForBytesWritten();

    // sending another request before timeout
    client.write("GET / HTTP/1.1\r\nConnection: keep-alive\r\n\r\n");
    client.waitForBytesWritten();
    QCOMPARE(client.state(), QAbstractSocket::ConnectedState);

    // wait past the timeout
    QTRY_COMPARE_WITH_TIMEOUT(client.state(), QAbstractSocket::UnconnectedState, 6000);

#if QT_CONFIG(ssl)
    if (!hasServerAlpn)
        return;

    // Test HTTP/2 connection

    auto sslserver = std::make_unique<QSslServer>();
    QSslConfiguration serverConfig = QSslConfiguration::defaultConfiguration();
    serverConfig.setLocalCertificate(QSslCertificate(g_certificate));
    serverConfig.setPrivateKey(QSslKey(g_privateKey, QSsl::Rsa));
    serverConfig.setAllowedNextProtocols({ QSslConfiguration::ALPNProtocolHTTP2 });
    sslserver->setSslConfiguration(serverConfig);
    QVERIFY2(sslserver->listen(QHostAddress::LocalHost), "HTTPS server listen failed");
    QVERIFY2(server.bind(sslserver.get()), "HTTPS server bind failed");
    sslserver.release();

    const auto serverPtr = server.servers().constLast();

    // create two HTTP/2 connections
    QSslSocketPtr socket1 = createNewConnection(serverPtr);
    QVERIFY(socket1->isEncrypted());
    QCOMPARE(socket1->state(), QAbstractSocket::ConnectedState);

    QSslSocketPtr socket2 = createNewConnection(serverPtr);
    QVERIFY(socket2->isEncrypted());
    QCOMPARE(socket2->state(), QAbstractSocket::ConnectedState);

    QHttp2Connection *connection1 = QHttp2Connection::createDirectConnection(socket1.get(), {});
    QVERIFY(connection1);
    QHttp2Connection *connection2 = QHttp2Connection::createDirectConnection(socket2.get(), {});
    QVERIFY(connection2);

    // wait for HTTP/2 handshake
    QSignalSpy settingsFrameReceivedSpy1{ connection1, &QHttp2Connection::settingsFrameReceived };
    connect(socket1.get(), &QIODevice::readyRead, connection1, &QHttp2Connection::handleReadyRead);
    connection1->handleReadyRead();

    QSignalSpy settingsFrameReceivedSpy2{ connection2, &QHttp2Connection::settingsFrameReceived };
    connect(socket2.get(), &QIODevice::readyRead, connection2, &QHttp2Connection::handleReadyRead);
    connection2->handleReadyRead();

    auto stream1 = connection1->createStream().unwrap();
    QVERIFY(stream1);
    QVERIFY(settingsFrameReceivedSpy1.wait());

    auto stream2 = connection2->createStream().unwrap();
    QVERIFY(stream2);
    QVERIFY(settingsFrameReceivedSpy2.wait());

    // send a request on socket1
    HPack::HttpHeader headers = {
                                  { ":authority", "example.com" },
                                  { ":method", "GET" },
                                  { ":path", "/" },
                                  { ":scheme", "https" },
                                };
    stream1->sendHEADERS(headers, true);

    // the idle socket should be closed after timeout
    QTRY_VERIFY_WITH_TIMEOUT(socket2->state() == QAbstractSocket::UnconnectedState, 6000);
    // the active connection should remain open
    QVERIFY(socket1->state() == QAbstractSocket::ConnectedState);
#endif // QT_CONFIG(ssl)
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QAbstractHttpServer)

#include "tst_qabstracthttpserver.moc"
