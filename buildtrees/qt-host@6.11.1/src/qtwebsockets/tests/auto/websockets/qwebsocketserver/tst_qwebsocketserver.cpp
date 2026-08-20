// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QString>
#include <QtTest>
#include <QNetworkProxy>
#include <QTcpSocket>
#include <QTcpServer>
#include <QtCore/QScopedPointer>
#ifndef QT_NO_SSL
#include <QtNetwork/qsslpresharedkeyauthenticator.h>
#include <QtNetwork/qsslcipher.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslsocket.h>
#endif
#include <QtWebSockets/QWebSocketHandshakeOptions>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketCorsAuthenticator>
#include <QtWebSockets/qwebsocketprotocol.h>

using namespace Qt::StringLiterals;

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::Version)
Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketServer::SslMode)
Q_DECLARE_METATYPE(QWebSocketCorsAuthenticator *)
#ifndef QT_NO_SSL
Q_DECLARE_METATYPE(QSslError)
#endif

// Use this cipher to force PSK key sharing.
static const QString PSK_CIPHER_WITHOUT_AUTH = QStringLiteral("PSK-AES256-CBC-SHA");
static const QByteArray PSK_CLIENT_PRESHAREDKEY = QByteArrayLiteral("\x1a\x2b\x3c\x4d\x5e\x6f");
static const QByteArray PSK_SERVER_IDENTITY_HINT = QByteArrayLiteral("QtTestServerHint");
static const QByteArray PSK_CLIENT_IDENTITY = QByteArrayLiteral("Client_identity");

class PskProvider : public QObject
{
    Q_OBJECT

public:
    bool m_server = false;
    QByteArray m_identity;
    QByteArray m_psk;

public slots:
#if QT_CONFIG(ssl)
    void providePsk(QSslPreSharedKeyAuthenticator *authenticator)
    {
        QVERIFY(authenticator);
        QCOMPARE(authenticator->identityHint(), PSK_SERVER_IDENTITY_HINT);
        if (m_server)
            QCOMPARE(authenticator->maximumIdentityLength(), 0);
        else
            QVERIFY(authenticator->maximumIdentityLength() > 0);

        QVERIFY(authenticator->maximumPreSharedKeyLength() > 0);

        if (!m_identity.isEmpty()) {
            authenticator->setIdentity(m_identity);
            QCOMPARE(authenticator->identity(), m_identity);
        }

        if (!m_psk.isEmpty()) {
            authenticator->setPreSharedKey(m_psk);
            QCOMPARE(authenticator->preSharedKey(), m_psk);
        }
    }
#endif
};

class tst_QWebSocketServer : public QObject
{
    Q_OBJECT

public:
    tst_QWebSocketServer();

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void tst_initialisation();
    void tst_settersAndGetters();
    void tst_listening();
    void tst_connectivity();
    void tst_protocols_data();
    void tst_protocols();
#if QT_CONFIG(ssl)
    void tst_preSharedKey();
#endif
    void tst_maxPendingConnections();
    void tst_serverDestroyedWhileSocketConnected();
    void tst_scheme(); // qtbug-55927
    void tst_handleConnection();
    void tst_handshakeTimeout(); // qtbug-63312, qtbug-57026
    void gentleClose();
    void multipleFrames();

private:
    bool m_shouldSkipUnsupportedIpv6Test;
#ifdef SHOULD_CHECK_SYSCALL_SUPPORT
    bool ipv6GetsockoptionMissing(int level, int optname);
#endif
};

tst_QWebSocketServer::tst_QWebSocketServer() : m_shouldSkipUnsupportedIpv6Test(false)
{
}

void tst_QWebSocketServer::init()
{
    qRegisterMetaType<QWebSocketProtocol::Version>("QWebSocketProtocol::Version");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
    qRegisterMetaType<QWebSocketServer::SslMode>("QWebSocketServer::SslMode");
    qRegisterMetaType<QWebSocketCorsAuthenticator *>("QWebSocketCorsAuthenticator *");
#ifndef QT_NO_SSL
    qRegisterMetaType<QSslError>("QSslError");
    qRegisterMetaType<QSslPreSharedKeyAuthenticator *>();
#endif
}

#ifdef SHOULD_CHECK_SYSCALL_SUPPORT
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

bool tst_QWebSocketServer::ipv6GetsockoptionMissing(int level, int optname)
{
    int testSocket;

    testSocket = socket(PF_INET6, SOCK_STREAM, 0);

    // If we can't test here, assume it's not missing
    if (testSocket == -1)
        return false;

    bool result = false;
    if (getsockopt(testSocket, level, optname, nullptr, 0) == -1) {
        if (errno == EOPNOTSUPP) {
            result = true;
        }
    }

    close(testSocket);
    return result;
}

#endif //SHOULD_CHECK_SYSCALL_SUPPORT

void tst_QWebSocketServer::initTestCase()
{
#ifdef SHOULD_CHECK_SYSCALL_SUPPORT
    // Qemu does not have required support for IPV6 socket options.
    // If this is detected, skip the test
    m_shouldSkipUnsupportedIpv6Test = ipv6GetsockoptionMissing(SOL_IPV6, IPV6_V6ONLY);
#endif
}

void tst_QWebSocketServer::cleanupTestCase()
{
}

void tst_QWebSocketServer::tst_initialisation()
{
    {
        QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);

        QVERIFY(server.serverName().isEmpty());
        QCOMPARE(server.secureMode(), QWebSocketServer::NonSecureMode);
        QVERIFY(!server.isListening());
        QCOMPARE(server.maxPendingConnections(), 30);
        QCOMPARE(server.serverPort(), quint16(0));
        QCOMPARE(server.serverAddress(), QHostAddress());
        QCOMPARE(server.socketDescriptor(), -1);
        QVERIFY(!server.hasPendingConnections());
        QVERIFY(!server.nextPendingConnection());
        QCOMPARE(server.error(), QWebSocketProtocol::CloseCodeNormal);
        QVERIFY(server.errorString().isEmpty());
    #ifndef QT_NO_NETWORKPROXY
        QCOMPARE(server.proxy().type(), QNetworkProxy::DefaultProxy);
    #endif
    #ifndef QT_NO_SSL
        QCOMPARE(server.sslConfiguration(), QSslConfiguration::defaultConfiguration());
    #endif
        QCOMPARE(server.supportedVersions().size(), 1);
        QCOMPARE(server.supportedVersions().at(0), QWebSocketProtocol::VersionLatest);
        QCOMPARE(server.supportedVersions().at(0), QWebSocketProtocol::Version13);

        server.close();
        //closing a server should not affect any of the parameters
        //certainly if the server was not opened before

        QVERIFY(server.serverName().isEmpty());
        QCOMPARE(server.secureMode(), QWebSocketServer::NonSecureMode);
        QVERIFY(!server.isListening());
        QCOMPARE(server.maxPendingConnections(), 30);
        QCOMPARE(server.serverPort(), quint16(0));
        QCOMPARE(server.serverAddress(), QHostAddress());
        QCOMPARE(server.socketDescriptor(), -1);
        QVERIFY(!server.hasPendingConnections());
        QVERIFY(!server.nextPendingConnection());
        QCOMPARE(server.error(), QWebSocketProtocol::CloseCodeNormal);
        QVERIFY(server.errorString().isEmpty());
    #ifndef QT_NO_NETWORKPROXY
        QCOMPARE(server.proxy().type(), QNetworkProxy::DefaultProxy);
    #endif
    #ifndef QT_NO_SSL
        QCOMPARE(server.sslConfiguration(), QSslConfiguration::defaultConfiguration());
    #endif
        QCOMPARE(server.supportedVersions().size(), 1);
        QCOMPARE(server.supportedVersions().at(0), QWebSocketProtocol::VersionLatest);
        QCOMPARE(server.supportedVersions().at(0), QWebSocketProtocol::Version13);
        QCOMPARE(server.serverUrl(), QUrl());
    }

    {
#ifndef QT_NO_SSL
        QWebSocketServer sslServer(QString(), QWebSocketServer::SecureMode);
        QCOMPARE(sslServer.secureMode(), QWebSocketServer::SecureMode);
#endif
    }
}

void tst_QWebSocketServer::tst_settersAndGetters()
{
    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);

    server.setMaxPendingConnections(23);
    QCOMPARE(server.maxPendingConnections(), 23);
    server.setMaxPendingConnections(INT_MIN);
    QCOMPARE(server.maxPendingConnections(), INT_MIN);
    server.setMaxPendingConnections(INT_MAX);
    QCOMPARE(server.maxPendingConnections(), INT_MAX);

    QVERIFY(!server.setSocketDescriptor(-2));
    QCOMPARE(server.socketDescriptor(), -1);

    server.setServerName(QStringLiteral("Qt WebSocketServer"));
    QCOMPARE(server.serverName(), QStringLiteral("Qt WebSocketServer"));

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy);
    server.setProxy(proxy);
    QCOMPARE(server.proxy(), proxy);
#endif
#ifndef QT_NO_SSL
    //cannot set an ssl configuration on a non secure server
    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    sslConfiguration.setPeerVerifyDepth(sslConfiguration.peerVerifyDepth() + 1);
    server.setSslConfiguration(sslConfiguration);
    QVERIFY(server.sslConfiguration() != sslConfiguration);
    QCOMPARE(server.sslConfiguration(), QSslConfiguration::defaultConfiguration());

    QWebSocketServer sslServer(QString(), QWebSocketServer::SecureMode);
    sslServer.setSslConfiguration(sslConfiguration);
    QCOMPARE(sslServer.sslConfiguration(), sslConfiguration);
    QVERIFY(sslServer.sslConfiguration() != QSslConfiguration::defaultConfiguration());
#endif

    server.setHandshakeTimeout(64);
    QCOMPARE(server.handshakeTimeoutMS(), 64);
#if __has_include(<chrono>)
    auto expected = std::chrono::milliseconds(64);
    QCOMPARE(server.handshakeTimeout(), expected);

    expected = std::chrono::milliseconds(242);
    server.setHandshakeTimeout(expected);
    QCOMPARE(server.handshakeTimeoutMS(), 242);
    QCOMPARE(server.handshakeTimeout(), expected);
#endif
}

void tst_QWebSocketServer::tst_listening()
{
    //These listening tests are not too extensive, as the implementation of QWebSocketServer
    //relies on QTcpServer

    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);

    QSignalSpy serverAcceptErrorSpy(&server, SIGNAL(acceptError(QAbstractSocket::SocketError)));
    QSignalSpy serverConnectionSpy(&server, SIGNAL(newConnection()));
    QSignalSpy serverErrorSpy(&server,
                              SIGNAL(serverError(QWebSocketProtocol::CloseCode)));
    QSignalSpy corsAuthenticationSpy(&server,
                              SIGNAL(originAuthenticationRequired(QWebSocketCorsAuthenticator*)));
    QSignalSpy serverClosedSpy(&server, SIGNAL(closed()));
#ifndef QT_NO_SSL
    QSignalSpy peerVerifyErrorSpy(&server, SIGNAL(peerVerifyError(QSslError)));
    QSignalSpy sslErrorsSpy(&server, SIGNAL(sslErrors(QList<QSslError>)));
    QSignalSpy sslErrorsOccurredSpy(&server, SIGNAL(sslErrorsOccurred(QSslSocket*, QList<QSslError>)));
#endif

    QVERIFY(server.listen());   //listen on all network interface, choose an appropriate port
    QVERIFY(server.isListening());
    QCOMPARE(serverClosedSpy.size(), 0);
    server.close();
    QTRY_COMPARE(serverClosedSpy.size(), 1);
    QVERIFY(!server.isListening());
    QCOMPARE(serverErrorSpy.size(), 0);

    QVERIFY(!server.listen(QHostAddress(QStringLiteral("1.2.3.4")), 0));
    QCOMPARE(server.error(), QWebSocketProtocol::CloseCodeAbnormalDisconnection);
    QCOMPARE(server.errorString().toLatin1().constData(), "The address is not available");
    QVERIFY(!server.isListening());

    QCOMPARE(serverAcceptErrorSpy.size(), 0);
    QCOMPARE(serverConnectionSpy.size(), 0);
    QCOMPARE(corsAuthenticationSpy.size(), 0);
#ifndef QT_NO_SSL
    QCOMPARE(peerVerifyErrorSpy.size(), 0);
    QCOMPARE(sslErrorsSpy.size(), 0);
    QCOMPARE(sslErrorsOccurredSpy.size(), 0);
#endif
    QCOMPARE(serverErrorSpy.size(), 1);
    QCOMPARE(serverClosedSpy.size(), 1);
}

void tst_QWebSocketServer::tst_connectivity()
{
    if (m_shouldSkipUnsupportedIpv6Test)
        QSKIP("Syscalls needed for ipv6 sockoptions missing functionality");

    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);
    QSignalSpy serverConnectionSpy(&server, SIGNAL(newConnection()));
    QSignalSpy serverErrorSpy(&server,
                              SIGNAL(serverError(QWebSocketProtocol::CloseCode)));
    QSignalSpy corsAuthenticationSpy(&server,
                              SIGNAL(originAuthenticationRequired(QWebSocketCorsAuthenticator*)));
    QSignalSpy serverClosedSpy(&server, SIGNAL(closed()));
#ifndef QT_NO_SSL
    QSignalSpy peerVerifyErrorSpy(&server, SIGNAL(peerVerifyError(QSslError)));
    QSignalSpy sslErrorsSpy(&server, SIGNAL(sslErrors(QList<QSslError>)));
    QSignalSpy sslErrorsOccurredSpy(&server, SIGNAL(sslErrorsOccurred(QSslSocket*, QList<QSslError>)));
#endif
    QWebSocket socket;
    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));

    QVERIFY(server.listen());
    QCOMPARE(server.serverAddress(), QHostAddress(QHostAddress::Any));
    QCOMPARE(server.serverUrl(), QUrl(QStringLiteral("ws://") + QHostAddress(QHostAddress::LocalHost).toString() +
                                 QStringLiteral(":").append(QString::number(server.serverPort()))));

    socket.open(server.serverUrl());

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(serverConnectionSpy.size(), 1);
    QCOMPARE(corsAuthenticationSpy.size(), 1);

    QCOMPARE(serverClosedSpy.size(), 0);

    server.close();

    QTRY_COMPARE(serverClosedSpy.size(), 1);
#ifndef QT_NO_SSL
    QCOMPARE(peerVerifyErrorSpy.size(), 0);
    QCOMPARE(sslErrorsSpy.size(), 0);
    QCOMPARE(sslErrorsOccurredSpy.size(), 0);
#endif
    QCOMPARE(serverErrorSpy.size(), 0);
}

void tst_QWebSocketServer::tst_protocols_data()
{
    QTest::addColumn<QStringList>("clientProtocols");
    QTest::addColumn<QStringList>("headerProtocols");
    QTest::addColumn<QString>("expectedProtocol");

    QTest::addRow("none") << QStringList{} << QStringList{} << QString{};
    QTest::addRow("same order as server")
            << QStringList{ "chat", "superchat" } << QStringList{} << QStringLiteral("chat");
    QTest::addRow("different order from server")
            << QStringList{ "superchat", "chat" } << QStringList{} << QStringLiteral("superchat");
    QTest::addRow("unsupported protocol") << QStringList{ "foo" } << QStringList{} << QString{};
    QTest::addRow("mixed supported/unsupported protocol")
            << QStringList{ "foo", "chat" } << QStringList{} << QStringLiteral("chat");

    QTest::addRow("same order as server, in header")
            << QStringList{} << QStringList{ "chat", "superchat" } << QStringLiteral("chat");
    QTest::addRow("specified in options and in header")
            << QStringList{ "superchat" } << QStringList{ "chat" } << QStringLiteral("superchat");
}

void tst_QWebSocketServer::tst_protocols()
{
    QFETCH(QStringList, clientProtocols);
    QFETCH(QStringList, headerProtocols);
    QFETCH(QString, expectedProtocol);

    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);
    server.setSupportedSubprotocols({ QStringLiteral("chat"), QStringLiteral("superchat") });

    QSignalSpy newConnectionSpy(&server, &QWebSocketServer::newConnection);

    QVERIFY(server.listen());

    QWebSocketHandshakeOptions opt;
    QNetworkRequest request(server.serverUrl());
    if (!headerProtocols.isEmpty()) {
        QString protocols = headerProtocols.join(u',');
        request.setRawHeader("Sec-WebSocket-Protocol", protocols.toUtf8());
    }
    opt.setSubprotocols(clientProtocols);

    QWebSocket client;
    client.open(request, opt);

    QTRY_COMPARE(client.state(), QAbstractSocket::ConnectedState);
    QTRY_COMPARE(newConnectionSpy.size(), 1);

    auto *serverSocket = server.nextPendingConnection();
    QVERIFY(serverSocket);

    QCOMPARE(client.subprotocol(), expectedProtocol);
    QCOMPARE(serverSocket->subprotocol(), expectedProtocol);
    QCOMPARE(serverSocket->handshakeOptions().subprotocols(), clientProtocols + headerProtocols);
}

#if QT_CONFIG(ssl)
void tst_QWebSocketServer::tst_preSharedKey()
{
    if (m_shouldSkipUnsupportedIpv6Test)
        QSKIP("Syscalls needed for ipv6 sockoptions missing functionality");

    if (QSslSocket::activeBackend() != u"openssl")
        QSKIP("OpenSSL required");

    QWebSocketServer server(QString(), QWebSocketServer::SecureMode);

    bool cipherFound = false;
    const QList<QSslCipher> supportedCiphers = QSslConfiguration::supportedCiphers();
    for (const QSslCipher &cipher : supportedCiphers) {
        if (cipher.name() == PSK_CIPHER_WITHOUT_AUTH) {
            cipherFound = true;
            break;
        }
    }

    if (!cipherFound)
        QSKIP("SSL implementation does not support the necessary cipher");

    QSslCipher cipher(PSK_CIPHER_WITHOUT_AUTH);
    QList<QSslCipher> list;
    list << cipher;

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2); // With TLS 1.3 there are some issues with PSK, force 1.2
    config.setCiphers(list);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setPreSharedKeyIdentityHint(PSK_SERVER_IDENTITY_HINT);
    server.setSslConfiguration(config);

    PskProvider providerServer;
    providerServer.m_server = true;
    providerServer.m_identity = PSK_CLIENT_IDENTITY;
    providerServer.m_psk = PSK_CLIENT_PRESHAREDKEY;
    connect(&server, &QWebSocketServer::preSharedKeyAuthenticationRequired, &providerServer, &PskProvider::providePsk);

    QSignalSpy serverPskRequiredSpy(&server, &QWebSocketServer::preSharedKeyAuthenticationRequired);
    QSignalSpy serverConnectionSpy(&server, &QWebSocketServer::newConnection);
    QSignalSpy serverErrorSpy(&server,
                              SIGNAL(serverError(QWebSocketProtocol::CloseCode)));
    QSignalSpy serverClosedSpy(&server, &QWebSocketServer::closed);
    QSignalSpy sslErrorsSpy(&server, SIGNAL(sslErrors(QList<QSslError>)));
    QSignalSpy sslErrorsOccurredSpy(&server, SIGNAL(sslErrorsOccurred(QSslSocket*, QList<QSslError>)));

    QWebSocket socket;
    QSslConfiguration socketConfig = QSslConfiguration::defaultConfiguration();
    if (QSslSocket::sslLibraryVersionNumber() >= 0x10101000L)
        socketConfig.setProtocol(QSsl::TlsV1_2); // With TLS 1.3 there are some issues with PSK, force 1.2
    socketConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    socketConfig.setCiphers(list);
    socket.setSslConfiguration(socketConfig);

    PskProvider providerClient;
    providerClient.m_identity = PSK_CLIENT_IDENTITY;
    providerClient.m_psk = PSK_CLIENT_PRESHAREDKEY;
    connect(&socket, &QWebSocket::preSharedKeyAuthenticationRequired, &providerClient, &PskProvider::providePsk);
    QSignalSpy socketPskRequiredSpy(&socket, &QWebSocket::preSharedKeyAuthenticationRequired);
    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);

    QVERIFY(server.listen());
    QCOMPARE(server.serverAddress(), QHostAddress(QHostAddress::Any));
    QCOMPARE(server.serverUrl(), QUrl(QString::asprintf("wss://%ls:%d",
                                 qUtf16Printable(QHostAddress(QHostAddress::LocalHost).toString()), server.serverPort())));

    socket.open(server.serverUrl());

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(serverConnectionSpy.size(), 1);
    QCOMPARE(serverPskRequiredSpy.size(), 1);
    QCOMPARE(socketPskRequiredSpy.size(), 1);

    QCOMPARE(serverClosedSpy.size(), 0);

    server.close();

    QTRY_COMPARE(serverClosedSpy.size(), 1);
    QCOMPARE(sslErrorsSpy.size(), 0);
    QCOMPARE(sslErrorsOccurredSpy.size(), 0);
    QCOMPARE(serverErrorSpy.size(), 0);
}
#endif

void tst_QWebSocketServer::tst_maxPendingConnections()
{
    if (m_shouldSkipUnsupportedIpv6Test)
        QSKIP("Syscalls needed for ipv6 sockoptions missing functionality");

    //tests if maximum connections are respected
    //also checks if there are no side-effects like signals that are unexpectedly thrown
    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);
    server.setMaxPendingConnections(2);
    QSignalSpy serverConnectionSpy(&server, SIGNAL(newConnection()));
    QSignalSpy serverErrorSpy(&server,
                              SIGNAL(serverError(QWebSocketProtocol::CloseCode)));
    QSignalSpy corsAuthenticationSpy(&server,
                              SIGNAL(originAuthenticationRequired(QWebSocketCorsAuthenticator*)));
    QSignalSpy serverClosedSpy(&server, SIGNAL(closed()));
#ifndef QT_NO_SSL
    QSignalSpy peerVerifyErrorSpy(&server, SIGNAL(peerVerifyError(QSslError)));
    QSignalSpy sslErrorsSpy(&server, SIGNAL(sslErrors(QList<QSslError>)));
    QSignalSpy sslErrorsOccurredSpy(&server, SIGNAL(sslErrorsOccurred(QSslSocket*, QList<QSslError>)));
#endif
    QSignalSpy serverAcceptErrorSpy(&server, SIGNAL(acceptError(QAbstractSocket::SocketError)));

    QWebSocket socket1;
    QWebSocket socket2;
    QWebSocket socket3;

    QSignalSpy socket1ConnectedSpy(&socket1, SIGNAL(connected()));
    QSignalSpy socket2ConnectedSpy(&socket2, SIGNAL(connected()));
    QSignalSpy socket3ConnectedSpy(&socket3, SIGNAL(connected()));

    QVERIFY(server.listen());

    socket1.open(server.serverUrl());

    QTRY_COMPARE(socket1ConnectedSpy.size(), 1);
    QCOMPARE(socket1.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(serverConnectionSpy.size(), 1);
    QCOMPARE(corsAuthenticationSpy.size(), 1);
    socket2.open(server.serverUrl());
    QTRY_COMPARE(socket2ConnectedSpy.size(), 1);
    QCOMPARE(socket2.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(serverConnectionSpy.size(), 2);
    QCOMPARE(corsAuthenticationSpy.size(), 2);
    socket3.open(server.serverUrl());
    QVERIFY(!socket3ConnectedSpy.wait(250));
    QCOMPARE(socket3ConnectedSpy.size(), 0);
    QCOMPARE(socket3.state(), QAbstractSocket::UnconnectedState);
    QCOMPARE(serverConnectionSpy.size(), 2);
    QCOMPARE(corsAuthenticationSpy.size(), 2);

    QVERIFY(server.hasPendingConnections());
    QWebSocket *pSocket = server.nextPendingConnection();
    QVERIFY(pSocket);
    delete pSocket;
    QVERIFY(server.hasPendingConnections());
    pSocket = server.nextPendingConnection();
    QVERIFY(pSocket);
    delete pSocket;
    QVERIFY(!server.hasPendingConnections());
    QVERIFY(!server.nextPendingConnection());

//will resolve in another commit
#ifndef Q_OS_WIN
    QCOMPARE(serverErrorSpy.size(), 1);
    QCOMPARE(serverErrorSpy.at(0).at(0).value<QWebSocketProtocol::CloseCode>(),
             QWebSocketProtocol::CloseCodeAbnormalDisconnection);
#endif
    QCOMPARE(serverClosedSpy.size(), 0);

    server.close();

    QTRY_COMPARE(serverClosedSpy.size(), 1);
#ifndef QT_NO_SSL
    QCOMPARE(peerVerifyErrorSpy.size(), 0);
    QCOMPARE(sslErrorsSpy.size(), 0);
    QCOMPARE(sslErrorsOccurredSpy.size(), 0);
#endif
    QCOMPARE(serverAcceptErrorSpy.size(), 0);
}

void tst_QWebSocketServer::tst_serverDestroyedWhileSocketConnected()
{
    if (m_shouldSkipUnsupportedIpv6Test)
        QSKIP("Syscalls needed for ipv6 sockoptions missing functionality");

    QWebSocketServer * server = new QWebSocketServer(QString(), QWebSocketServer::NonSecureMode);
    QSignalSpy serverConnectionSpy(server, SIGNAL(newConnection()));
    QSignalSpy corsAuthenticationSpy(server,
                              SIGNAL(originAuthenticationRequired(QWebSocketCorsAuthenticator*)));
    QSignalSpy serverClosedSpy(server, SIGNAL(closed()));

    QWebSocket socket;
    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy socketDisconnectedSpy(&socket, SIGNAL(disconnected()));

    QVERIFY(server->listen());
    QCOMPARE(server->serverAddress(), QHostAddress(QHostAddress::Any));
    QCOMPARE(server->serverUrl(), QUrl(QStringLiteral("ws://") + QHostAddress(QHostAddress::LocalHost).toString() +
                                  QStringLiteral(":").append(QString::number(server->serverPort()))));

    socket.open(server->serverUrl());

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(serverConnectionSpy.size(), 1);
    QCOMPARE(corsAuthenticationSpy.size(), 1);

    QCOMPARE(serverClosedSpy.size(), 0);

    delete server;

    QTRY_COMPARE(socketDisconnectedSpy.size(), 1);
}

#ifndef QT_NO_SSL
static void setupSecureServer(QWebSocketServer *secureServer)
{
    QSslConfiguration sslConfiguration;
    QFile certFile(QStringLiteral(":/localhost.cert"));
    QFile keyFile(QStringLiteral(":/localhost.key"));
    QVERIFY(certFile.open(QIODevice::ReadOnly));
    QVERIFY(keyFile.open(QIODevice::ReadOnly));
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    secureServer->setSslConfiguration(sslConfiguration);
}
#endif

void tst_QWebSocketServer::tst_scheme()
{
    if (m_shouldSkipUnsupportedIpv6Test)
        QSKIP("Syscalls needed for ipv6 sockoptions missing functionality");

    QWebSocketServer plainServer(QString(), QWebSocketServer::NonSecureMode);
    QSignalSpy plainServerConnectionSpy(&plainServer, SIGNAL(newConnection()));

    QVERIFY(plainServer.listen());

    QWebSocket plainSocket;
    plainSocket.open(plainServer.serverUrl());

    QTRY_COMPARE(plainServerConnectionSpy.size(), 1);
    QScopedPointer<QWebSocket> plainServerSocket(plainServer.nextPendingConnection());
    QVERIFY(!plainServerSocket.isNull());
    QCOMPARE(plainServerSocket->requestUrl().scheme(), QStringLiteral("ws"));
    plainServer.close();

#ifndef QT_NO_SSL
    QWebSocketServer secureServer(QString(), QWebSocketServer::SecureMode);
    setupSecureServer(&secureServer);
    if (QTest::currentTestFailed())
        return;
    QSignalSpy secureServerConnectionSpy(&secureServer, SIGNAL(newConnection()));

    QVERIFY(secureServer.listen());

    QSslCipher sessionCipher;
    QWebSocket secureSocket;
    connect(&secureSocket, &QWebSocket::sslErrors,
            &secureSocket, [&] {
                secureSocket.ignoreSslErrors();
                sessionCipher = secureSocket.sslConfiguration().sessionCipher();
            });
    secureSocket.open(secureServer.serverUrl());

    QTRY_COMPARE(secureServerConnectionSpy.size(), 1);
    QScopedPointer<QWebSocket> secureServerSocket(secureServer.nextPendingConnection());
    QVERIFY(!secureServerSocket.isNull());
    QCOMPARE(secureServerSocket->requestUrl().scheme(), QStringLiteral("wss"));
    secureServer.close();
    QVERIFY(!sessionCipher.isNull());

    QWebSocketServer selfSignedServer(QString(), QWebSocketServer::SecureMode);
    QSslConfiguration sslConfiguration;
    QFile certFile(QStringLiteral(":/selfsigned.cert"));
    QFile keyFile(QStringLiteral(":/selfsigned.key"));
    QVERIFY(certFile.open(QIODevice::ReadOnly));
    QVERIFY(keyFile.open(QIODevice::ReadOnly));
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyPeer);
    selfSignedServer.setSslConfiguration(sslConfiguration);
    QSignalSpy selfSignedServerConnectionSpy(&selfSignedServer, SIGNAL(newConnection()));
    QSignalSpy selfSignedServerSslErrorsOccurredSpy(
        &selfSignedServer, SIGNAL(sslErrorsOccurred(QSslSocket*, QList<QSslError>)));

    QVERIFY(selfSignedServer.listen());

    QWebSocket selfSignedSocket;
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    selfSignedSocket.setSslConfiguration(sslConfiguration);
    QObject::connect(&selfSignedSocket, &QWebSocket::sslErrors,
                     &selfSignedSocket, qOverload<>(&QWebSocket::ignoreSslErrors));
    selfSignedSocket.open(selfSignedServer.serverUrl());

    QTRY_COMPARE(selfSignedServerSslErrorsOccurredSpy.size(), 1);
#endif
}

void tst_QWebSocketServer::tst_handleConnection()
{
    QWebSocketServer wsServer(QString(), QWebSocketServer::NonSecureMode);
    QSignalSpy wsServerConnectionSpy(&wsServer, &QWebSocketServer::newConnection);

    QTcpServer tcpServer;
    connect(&tcpServer, &QTcpServer::newConnection,
            &wsServer, [&tcpServer, &wsServer]() {
        wsServer.handleConnection(tcpServer.nextPendingConnection());
    });
    QVERIFY(tcpServer.listen());

    QWebSocket webSocket;
    QSignalSpy wsConnectedSpy(&webSocket, &QWebSocket::connected);
    webSocket.open(QUrl{u"ws://localhost:%1"_s.arg(tcpServer.serverPort())});
    QTRY_COMPARE(wsConnectedSpy.size(), 1);

    QTRY_COMPARE(wsServerConnectionSpy.size(), 1);

    QScopedPointer<QWebSocket> webServerSocket(wsServer.nextPendingConnection());
    QVERIFY(!webServerSocket.isNull());

    QSignalSpy wsMessageReceivedSpy(webServerSocket.data(), &QWebSocket::textMessageReceived);
    webSocket.sendTextMessage("dummy");

    QTRY_COMPARE(wsMessageReceivedSpy.size(), 1);
    QList<QVariant> arguments = wsMessageReceivedSpy.takeFirst();
    QCOMPARE(arguments.first().toString(), QString("dummy"));

    QSignalSpy clientMessageReceivedSpy(&webSocket, &QWebSocket::textMessageReceived);
    webServerSocket->sendTextMessage("hello");
    QVERIFY(webServerSocket->bytesToWrite() > 5); // 5 + a few extra bytes for header
    QTRY_COMPARE(webServerSocket->bytesToWrite(), 0);
    QTRY_COMPARE(clientMessageReceivedSpy.size(), 1);
    arguments = clientMessageReceivedSpy.takeFirst();
    QCOMPARE(arguments.first().toString(), QString("hello"));
}

struct SocketSpy {
    QTcpSocket *socket;
    QSignalSpy *disconnectSpy;
    ~SocketSpy() {
        delete socket;
        delete disconnectSpy;
    }
};

static void openManyConnections(QList<SocketSpy *> *sockets, quint16 port, int numConnections)
{
    for (int i = 0; i < numConnections; i++) {
        QTcpSocket *c = new QTcpSocket;
        QSignalSpy *spy = new QSignalSpy(c, &QTcpSocket::disconnected);

        c->connectToHost("127.0.0.1", port);

        sockets->append(new SocketSpy{c, spy});
    }
}

// Sum the counts together, for better output on failure (e.g. "FAIL: Actual: 49, Expected: 50")
static int sumSocketSpyCount(const QList<SocketSpy *> &sockets)
{
    return std::accumulate(sockets.cbegin(), sockets.cend(), 0, [](int c, SocketSpy *s) {
        return c + s->disconnectSpy->size();
    });
}

void tst_QWebSocketServer::tst_handshakeTimeout()
{
    { // No Timeout
        QWebSocketServer plainServer(QString(), QWebSocketServer::NonSecureMode);
        plainServer.setHandshakeTimeout(-1);
        QSignalSpy plainServerConnectionSpy(&plainServer, SIGNAL(newConnection()));

        QVERIFY(plainServer.listen());

        QWebSocket socket;
        socket.open(plainServer.serverUrl());

        QTRY_COMPARE(plainServerConnectionSpy.size(), 1);
        QScopedPointer<QWebSocket> plainServerSocket(plainServer.nextPendingConnection());
        QVERIFY(!plainServerSocket.isNull());

        plainServer.close();
    }

    { // Unencrypted
        QWebSocketServer plainServer(QString(), QWebSocketServer::NonSecureMode);
        plainServer.setHandshakeTimeout(500);
        QSignalSpy plainServerConnectionSpy(&plainServer, SIGNAL(newConnection()));

        QVERIFY(plainServer.listen());

        /* QTcpServer has a default of 30 pending connections. The test checks
         * whether, when that list is full, the connections are dropped after
         * a timeout and later pending connections are processed. */
        const int numConnections = 50;
        QList<SocketSpy *> sockets;
        auto cleaner = qScopeGuard([&sockets]() { qDeleteAll(sockets); });
        openManyConnections(&sockets, plainServer.serverPort(), numConnections);

        QCoreApplication::processEvents();

        /* We have 50 plain TCP connections open, that are not proper websockets. */
        QCOMPARE(plainServerConnectionSpy.size(), 0);

        QWebSocket socket;
        socket.open(plainServer.serverUrl());

        /* Check that a real websocket will be processed after some non-websocket
         * TCP connections timeout. */
        QTRY_COMPARE(plainServerConnectionSpy.size(), 1);
        QScopedPointer<QWebSocket> plainServerSocket(plainServer.nextPendingConnection());
        QVERIFY(!plainServerSocket.isNull());

        /* Check that all non websocket connections eventually timeout. */
        QTRY_COMPARE(sumSocketSpyCount(sockets), numConnections);

        plainServer.close();
    }

#if QT_CONFIG(ssl)
    { // Encrypted
        QWebSocketServer secureServer(QString(), QWebSocketServer::SecureMode);
        setupSecureServer(&secureServer);
        if (QTest::currentTestFailed())
            return;
        secureServer.setHandshakeTimeout(2000);

        QSignalSpy secureServerConnectionSpy(&secureServer, SIGNAL(newConnection()));

        QVERIFY(secureServer.listen());

        const int numConnections = 50;
        QList<SocketSpy *> sockets;
        auto cleaner = qScopeGuard([&sockets]() { qDeleteAll(sockets); });
        openManyConnections(&sockets, secureServer.serverPort(), numConnections);

        QCoreApplication::processEvents();
        QCOMPARE(secureServerConnectionSpy.size(), 0);

        QWebSocket secureSocket;
        connect(&secureSocket, &QWebSocket::errorOccurred,
                this, [](QAbstractSocket::SocketError error) {
                    // This shouldn't print but it's useful for debugging when/if it does.
                    qDebug() << "Error occurred in the client:" << error;
                });
        QSslConfiguration config = secureSocket.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        secureSocket.setSslConfiguration(config);

        secureSocket.open(secureServer.serverUrl());

        QTRY_COMPARE(secureServerConnectionSpy.size(), 1);
        QScopedPointer<QWebSocket> serverSocket(secureServer.nextPendingConnection());
        QVERIFY(!serverSocket.isNull());

        QTRY_COMPARE(sumSocketSpyCount(sockets), numConnections);

        secureServer.close();
    }
#endif

    { // Ensure properly handshaked connections are not timed out
        QWebSocketServer plainServer(QString(), QWebSocketServer::NonSecureMode);
        plainServer.setHandshakeTimeout(250);
        QSignalSpy plainServerConnectionSpy(&plainServer, SIGNAL(newConnection()));

        QVERIFY(plainServer.listen());

        QWebSocket socket;
        QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
        QSignalSpy socketDisconnectedSpy(&socket, &QWebSocket::disconnected);
        socket.open(plainServer.serverUrl());

        QTRY_COMPARE(plainServerConnectionSpy.size(), 1);
        QTRY_COMPARE(socketConnectedSpy.size(), 1);

        QEventLoop loop;
        QTimer::singleShot(500, &loop, &QEventLoop::quit);
        loop.exec();

        QCOMPARE(socketDisconnectedSpy.size(), 0);
    }
}

void tst_QWebSocketServer::gentleClose()
{
    QWebSocketServer server("gentleClose", QWebSocketServer::NonSecureMode);
    QSignalSpy serverConnectionSpy(&server, &QWebSocketServer::newConnection);
    QVERIFY(server.listen());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, &QWebSocket::connected);
    QSignalSpy clientDisconnectedSpy(&client, &QWebSocket::disconnected);
    QSignalSpy clientErrorSpy(&client, &QWebSocket::errorOccurred);
    client.open(server.serverUrl());

    QVERIFY(serverConnectionSpy.wait());
    QVERIFY(connectedSpy.wait());
    QWebSocket *serverSocket = server.nextPendingConnection();
    QSignalSpy disconnectedSpy(serverSocket, &QWebSocket::disconnected);
    QSignalSpy serverErrorSpy(serverSocket, &QWebSocket::errorOccurred);

    client.close(QWebSocketProtocol::CloseCodeNormal);

    QVERIFY(disconnectedSpy.wait());
    QVERIFY(clientDisconnectedSpy.wait());

    QVERIFY(client.closeCode() != QWebSocketProtocol::CloseCodeGoingAway);
    QVERIFY(serverSocket->closeCode() != QWebSocketProtocol::CloseCodeGoingAway);
    QVERIFY(client.error() != QAbstractSocket::RemoteHostClosedError);
    QVERIFY(serverSocket->error() != QAbstractSocket::RemoteHostClosedError);
    QCOMPARE(clientErrorSpy.count(), 0);
    QCOMPARE(serverErrorSpy.count(), 0);
}

void tst_QWebSocketServer::multipleFrames()
{
    QWebSocketServer server(QString(), QWebSocketServer::NonSecureMode);
    QSignalSpy serverConnectionSpy(&server, &QWebSocketServer::newConnection);
    QVERIFY(server.listen());

    QWebSocket socket;
    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy messageReceivedSpy(&socket, &QWebSocket::binaryMessageReceived);
    socket.open(server.serverUrl());

    QVERIFY(serverConnectionSpy.wait());
    QVERIFY(socketConnectedSpy.wait());

    auto serverSocket = std::unique_ptr<QWebSocket>(server.nextPendingConnection());
    QVERIFY(serverSocket);
    for (int i = 0; i < 10; i++)
        serverSocket->sendBinaryMessage(QByteArray("abc"));
    if (serverSocket->bytesToWrite())
        QVERIFY(serverSocket->flush());

    QVERIFY(messageReceivedSpy.wait());
    // Since there's no guarantee the operating system will fit all 10 websocket frames into 1 tcp
    // frame, let's just assume it will do at least 2. EXCEPT_FAIL any which doesn't merge any.
    QVERIFY2(messageReceivedSpy.size() > 1, "Received only 1 message in the TCP frame!");
}

QTEST_MAIN(tst_QWebSocketServer)

#include "tst_qwebsocketserver.moc"
