// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QRegularExpression>
#include <QString>
#include <QtTest>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketHandshakeOptions>
#include <QtWebSockets/QWebSocketCorsAuthenticator>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/qwebsocketprotocol.h>

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qauthenticator.h>
#include <QtNetwork/qtcpsocket.h>

#if QT_CONFIG(ssl)
#include <QtNetwork/qsslserver.h>
#include <QtNetwork/qsslcertificate.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslsocket.h>
#endif

#include <QtTest/private/qtesthelpers_p.h>

#include <utility>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::Version)

using namespace Qt::StringLiterals;

class EchoServer : public QObject
{
    Q_OBJECT
public:
    explicit EchoServer(QObject *parent = nullptr,
        quint64 maxAllowedIncomingMessageSize = QWebSocket::maxIncomingMessageSize(),
        quint64 maxAllowedIncomingFrameSize = QWebSocket::maxIncomingFrameSize());
    ~EchoServer();

    QHostAddress hostAddress() const { return m_pWebSocketServer->serverAddress(); }
    quint16 port() const { return m_pWebSocketServer->serverPort(); }

Q_SIGNALS:
    void newConnection(QUrl requestUrl);
    void newConnection(QNetworkRequest request);
    void originAuthenticationRequired(QWebSocketCorsAuthenticator* pAuthenticator);

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    quint64 m_maxAllowedIncomingMessageSize;
    quint64 m_maxAllowedIncomingFrameSize;
    QList<QWebSocket *> m_clients;
};

EchoServer::EchoServer(QObject *parent, quint64 maxAllowedIncomingMessageSize, quint64 maxAllowedIncomingFrameSize) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    m_maxAllowedIncomingMessageSize(maxAllowedIncomingMessageSize),
    m_maxAllowedIncomingFrameSize(maxAllowedIncomingFrameSize),
    m_clients()
{
    m_pWebSocketServer->setSupportedSubprotocols({ QStringLiteral("protocol1"),
                                                QStringLiteral("protocol2") });
    if (m_pWebSocketServer->listen(QHostAddress(QStringLiteral("127.0.0.1")))) {
        connect(m_pWebSocketServer, SIGNAL(newConnection()),
                this, SLOT(onNewConnection()));
        connect(m_pWebSocketServer, &QWebSocketServer::originAuthenticationRequired,
                this, &EchoServer::originAuthenticationRequired);
    }
}

EchoServer::~EchoServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void EchoServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    pSocket->setMaxAllowedIncomingFrameSize(m_maxAllowedIncomingFrameSize);
    pSocket->setMaxAllowedIncomingMessageSize(m_maxAllowedIncomingMessageSize);

    Q_EMIT newConnection(pSocket->requestUrl());
    Q_EMIT newConnection(pSocket->request());

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processTextMessage(QString)));
    connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    m_clients << pSocket;
}

void EchoServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendTextMessage(message);
    }
}

void EchoServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}

void EchoServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

class tst_QWebSocket : public QObject
{
    Q_OBJECT

public:
    tst_QWebSocket();

private Q_SLOTS:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void tst_initialisation_data();
    void tst_initialisation();
    void tst_settersAndGetters();
    void tst_invalidOpen_data();
    void tst_invalidOpen();
    void tst_invalidOrigin();
    void tst_sendTextMessage();
    void tst_sendBinaryMessage();
    void tst_ping();
    void tst_errorString();
    void tst_openRequest_data();
    void tst_openRequest();
    void tst_protocolAccessor();
    void protocolsHeaderGeneration_data();
    void protocolsHeaderGeneration();
    void tst_moveToThread();
    void tst_moveToThreadNoWarning();
#ifndef QT_NO_NETWORKPROXY
    void tst_setProxy();
#endif
    void authenticationRequired_data();
    void authenticationRequired();
    void overlongCloseReason();
    void incomingMessageTooLong();
    void incomingFrameTooLong();
    void testingFrameAndMessageSizeApi();
    void hostHeaderFromNetworkRequest();
    void customHeader();
    void splitUtf8Sequence();
};

tst_QWebSocket::tst_QWebSocket()
{
}

void tst_QWebSocket::init()
{
    qRegisterMetaType<QWebSocketProtocol::Version>("QWebSocketProtocol::Version");
}

void tst_QWebSocket::initTestCase()
{
}

void tst_QWebSocket::cleanupTestCase()
{
}

void tst_QWebSocket::tst_initialisation_data()
{
    QTest::addColumn<QString>("origin");
    QTest::addColumn<QString>("expectedOrigin");
    QTest::addColumn<QWebSocketProtocol::Version>("version");
    QTest::addColumn<QWebSocketProtocol::Version>("expectedVersion");

    QTest::newRow("Default origin and version")
            << QString() << QString()
            << QWebSocketProtocol::VersionUnknown << QWebSocketProtocol::VersionLatest;
    QTest::newRow("Specific origin and default version")
            << QStringLiteral("qt-project.org") << QStringLiteral("qt-project.org")
            << QWebSocketProtocol::VersionUnknown << QWebSocketProtocol::VersionLatest;
    QTest::newRow("Specific origin and specific version")
            << QStringLiteral("qt-project.org") << QStringLiteral("qt-project.org")
            << QWebSocketProtocol::Version7 << QWebSocketProtocol::Version7;
}

void tst_QWebSocket::tst_initialisation()
{
    QFETCH(QString, origin);
    QFETCH(QString, expectedOrigin);
    QFETCH(QWebSocketProtocol::Version, version);
    QFETCH(QWebSocketProtocol::Version, expectedVersion);

    QScopedPointer<QWebSocket> socket;

    if (origin.isEmpty() && (version == QWebSocketProtocol::VersionUnknown))
        socket.reset(new QWebSocket);
    else if (!origin.isEmpty() && (version == QWebSocketProtocol::VersionUnknown))
        socket.reset(new QWebSocket(origin));
    else
        socket.reset(new QWebSocket(origin, version));

    QCOMPARE(socket->origin(), expectedOrigin);
    QCOMPARE(socket->version(), expectedVersion);
    QCOMPARE(socket->error(), QAbstractSocket::UnknownSocketError);
    QVERIFY(socket->errorString().isEmpty());
    QVERIFY(!socket->isValid());
    QVERIFY(socket->localAddress().isNull());
    QCOMPARE(socket->localPort(), quint16(0));
    QCOMPARE(socket->pauseMode(), QAbstractSocket::PauseNever);
    QVERIFY(socket->peerAddress().isNull());
    QCOMPARE(socket->peerPort(), quint16(0));
    QVERIFY(socket->peerName().isEmpty());
    QCOMPARE(socket->state(), QAbstractSocket::UnconnectedState);
    QCOMPARE(socket->readBufferSize(), 0);
    QVERIFY(socket->resourceName().isEmpty());
    QVERIFY(!socket->requestUrl().isValid());
    QCOMPARE(socket->closeCode(), QWebSocketProtocol::CloseCodeNormal);
    QVERIFY(socket->closeReason().isEmpty());
    QVERIFY(socket->flush());
    QCOMPARE(socket->sendTextMessage(QStringLiteral("A text message")), 0);
    QCOMPARE(socket->sendBinaryMessage(QByteArrayLiteral("A binary message")), 0);
}

void tst_QWebSocket::tst_settersAndGetters()
{
    QWebSocket socket;

    socket.setPauseMode(QAbstractSocket::PauseNever);
    QCOMPARE(socket.pauseMode(), QAbstractSocket::PauseNever);
    socket.setPauseMode(QAbstractSocket::PauseOnSslErrors);
    QCOMPARE(socket.pauseMode(), QAbstractSocket::PauseOnSslErrors);

    socket.setReadBufferSize(0);
    QCOMPARE(socket.readBufferSize(), 0);
    socket.setReadBufferSize(128);
    QCOMPARE(socket.readBufferSize(), 128);
    socket.setReadBufferSize(-1);
    QCOMPARE(socket.readBufferSize(), -1);
}

void tst_QWebSocket::tst_invalidOpen_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("expectedUrl");
    QTest::addColumn<QString>("expectedPeerName");
    QTest::addColumn<QString>("expectedResourceName");
    QTest::addColumn<QAbstractSocket::SocketState>("stateAfterOpenCall");
    QTest::addColumn<int>("disconnectedCount");
    QTest::addColumn<int>("stateChangedCount");

    QTest::newRow("Illegal local address")
            << QStringLiteral("ws://127.0.0.1:1/") << QStringLiteral("ws://127.0.0.1:1/")
            << QStringLiteral("127.0.0.1")
            << QStringLiteral("/") << QAbstractSocket::ConnectingState
            << 1
            << 2;  //going from connecting to disconnected
    QTest::newRow("URL containing new line in the hostname")
            << QStringLiteral("ws://myhacky\r\nserver/") << QString()
            << QString()
            << QString() << QAbstractSocket::UnconnectedState
            << 0 << 0;
    QTest::newRow("URL containing new line in the resource name")
            << QStringLiteral("ws://127.0.0.1:1/tricky\r\npath") << QString()
            << QString()
            << QString()
            << QAbstractSocket::UnconnectedState
            << 0 << 0;
}

void tst_QWebSocket::tst_invalidOpen()
{
    QFETCH(QString, url);
    QFETCH(QString, expectedUrl);
    QFETCH(QString, expectedPeerName);
    QFETCH(QString, expectedResourceName);
    QFETCH(QAbstractSocket::SocketState, stateAfterOpenCall);
    QFETCH(int, disconnectedCount);
    QFETCH(int, stateChangedCount);
#if defined(__QNX__) && __QNX__ >= 800
    if (url == QStringLiteral("ws://127.0.0.1:1/"))
        QSKIP("QNX 8.0 rejects loopback connects to non-listening ports "
              "synchronously, bypassing the ConnectingState transient the "
              "test expects to observe.");
#endif
    QWebSocket socket;
    QSignalSpy errorSpy(&socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QSignalSpy aboutToCloseSpy(&socket, SIGNAL(aboutToClose()));
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy stateChangedSpy(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
    QSignalSpy readChannelFinishedSpy(&socket, SIGNAL(readChannelFinished()));
    QSignalSpy textFrameReceivedSpy(&socket, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy binaryFrameReceivedSpy(&socket, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy textMessageReceivedSpy(&socket, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageReceivedSpy(&socket, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy pongSpy(&socket, SIGNAL(pong(quint64,QByteArray)));
    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    socket.open(QUrl(url));

    QVERIFY(socket.origin().isEmpty());
    QCOMPARE(socket.version(), QWebSocketProtocol::VersionLatest);
    //at this point the socket is in a connecting state
    //so, there should no error at this point
    QCOMPARE(socket.error(), QAbstractSocket::UnknownSocketError);
    QVERIFY(!socket.errorString().isEmpty());
    QVERIFY(!socket.isValid());
    QVERIFY(socket.localAddress().isNull());
    QCOMPARE(socket.localPort(), quint16(0));
    QCOMPARE(socket.pauseMode(), QAbstractSocket::PauseNever);
    QVERIFY(socket.peerAddress().isNull());
    QCOMPARE(socket.peerPort(), quint16(0));
    QCOMPARE(socket.peerName(), expectedPeerName);
    QCOMPARE(socket.state(), stateAfterOpenCall);
    QCOMPARE(socket.readBufferSize(), 0);
    QCOMPARE(socket.resourceName(), expectedResourceName);
    QCOMPARE(socket.requestUrl().toString(), expectedUrl);
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeNormal);
    QVERIFY(socket.closeReason().isEmpty());
    QCOMPARE(socket.sendTextMessage(QStringLiteral("A text message")), 0);
    QCOMPARE(socket.sendBinaryMessage(QByteArrayLiteral("A text message")), 0);

    if (errorSpy.size() == 0)
        QVERIFY(errorSpy.wait());
    QCOMPARE(errorSpy.size(), 1);
    QList<QVariant> arguments = errorSpy.takeFirst();
    QAbstractSocket::SocketError socketError =
            qvariant_cast<QAbstractSocket::SocketError>(arguments.at(0));
    QCOMPARE(socketError, QAbstractSocket::ConnectionRefusedError);
    QCOMPARE(aboutToCloseSpy.size(), 0);
    QCOMPARE(connectedSpy.size(), 0);
    QCOMPARE(disconnectedSpy.size(), disconnectedCount);
    QCOMPARE(stateChangedSpy.size(), stateChangedCount);
    if (stateChangedCount == 2) {
        arguments = stateChangedSpy.takeFirst();
        QAbstractSocket::SocketState socketState =
                qvariant_cast<QAbstractSocket::SocketState>(arguments.at(0));
        arguments = stateChangedSpy.takeFirst();
        socketState = qvariant_cast<QAbstractSocket::SocketState>(arguments.at(0));
        QCOMPARE(socketState, QAbstractSocket::UnconnectedState);
    }
    QCOMPARE(readChannelFinishedSpy.size(), 0);
    QCOMPARE(textFrameReceivedSpy.size(), 0);
    QCOMPARE(binaryFrameReceivedSpy.size(), 0);
    QCOMPARE(textMessageReceivedSpy.size(), 0);
    QCOMPARE(binaryMessageReceivedSpy.size(), 0);
    QCOMPARE(pongSpy.size(), 0);
    QCOMPARE(bytesWrittenSpy.size(), 0);
}

void tst_QWebSocket::tst_invalidOrigin()
{
#if defined(__QNX__) && __QNX__ >= 800
    QSKIP("QNX 8.0 rejects loopback connects to non-listening ports "
          "synchronously, bypassing the ConnectingState transient the "
          "test expects to observe.");
#endif
    QWebSocket socket(QStringLiteral("My server\r\nin the wild."));

    QSignalSpy errorSpy(&socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QSignalSpy aboutToCloseSpy(&socket, SIGNAL(aboutToClose()));
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy stateChangedSpy(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
    QSignalSpy readChannelFinishedSpy(&socket, SIGNAL(readChannelFinished()));
    QSignalSpy textFrameReceivedSpy(&socket, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy binaryFrameReceivedSpy(&socket, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy textMessageReceivedSpy(&socket, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageReceivedSpy(&socket, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy pongSpy(&socket, SIGNAL(pong(quint64,QByteArray)));
    QSignalSpy bytesWrittenSpy(&socket, SIGNAL(bytesWritten(qint64)));

    socket.open(QUrl(QStringLiteral("ws://127.0.0.1:1/")));

    //at this point the socket is in a connecting state
    //so, there should no error at this point
    QCOMPARE(socket.error(), QAbstractSocket::UnknownSocketError);
    QVERIFY(!socket.errorString().isEmpty());
    QVERIFY(!socket.isValid());
    QVERIFY(socket.localAddress().isNull());
    QCOMPARE(socket.localPort(), quint16(0));
    QCOMPARE(socket.pauseMode(), QAbstractSocket::PauseNever);
    QVERIFY(socket.peerAddress().isNull());
    QCOMPARE(socket.peerPort(), quint16(0));
    QCOMPARE(socket.peerName(), QStringLiteral("127.0.0.1"));
    QCOMPARE(socket.state(), QAbstractSocket::ConnectingState);
    QCOMPARE(socket.readBufferSize(), 0);
    QCOMPARE(socket.resourceName(), QStringLiteral("/"));
    QCOMPARE(socket.requestUrl(), QUrl(QStringLiteral("ws://127.0.0.1:1/")));
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeNormal);

    QVERIFY(errorSpy.wait());

    QCOMPARE(errorSpy.size(), 1);
    QList<QVariant> arguments = errorSpy.takeFirst();
    QAbstractSocket::SocketError socketError =
            qvariant_cast<QAbstractSocket::SocketError>(arguments.at(0));
    QCOMPARE(socketError, QAbstractSocket::ConnectionRefusedError);
    QCOMPARE(aboutToCloseSpy.size(), 0);
    QCOMPARE(connectedSpy.size(), 0);
    QCOMPARE(disconnectedSpy.size(), 1);
    QCOMPARE(stateChangedSpy.size(), 2);   //connectingstate, unconnectedstate
    arguments = stateChangedSpy.takeFirst();
    QAbstractSocket::SocketState socketState =
            qvariant_cast<QAbstractSocket::SocketState>(arguments.at(0));
    arguments = stateChangedSpy.takeFirst();
    socketState = qvariant_cast<QAbstractSocket::SocketState>(arguments.at(0));
    QCOMPARE(socketState, QAbstractSocket::UnconnectedState);
    QCOMPARE(readChannelFinishedSpy.size(), 0);
    QCOMPARE(textFrameReceivedSpy.size(), 0);
    QCOMPARE(binaryFrameReceivedSpy.size(), 0);
    QCOMPARE(textMessageReceivedSpy.size(), 0);
    QCOMPARE(binaryMessageReceivedSpy.size(), 0);
    QCOMPARE(pongSpy.size(), 0);
    QCOMPARE(bytesWrittenSpy.size(), 0);
}

void tst_QWebSocket::tst_sendTextMessage()
{
    EchoServer echoServer;

    QWebSocket socket;

    //should return 0 because socket is not open yet
    QCOMPARE(socket.sendTextMessage(QStringLiteral("1234")), 0);

    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy serverConnectedSpy(&echoServer, SIGNAL(newConnection(QUrl)));
    QSignalSpy textMessageReceived(&socket, SIGNAL(textMessageReceived(QString)));
    QSignalSpy textFrameReceived(&socket, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy binaryMessageReceived(&socket, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy binaryFrameReceived(&socket, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy socketError(&socket, SIGNAL(error(QAbstractSocket::SocketError)));

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    url.setPath("/segment/with spaces");
    QUrlQuery query;
    query.addQueryItem("queryitem", "with encoded characters");
    url.setQuery(query);

    socket.open(url);

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socketError.size(), 0);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QList<QVariant> arguments = serverConnectedSpy.takeFirst();
    QUrl urlConnected = arguments.at(0).toUrl();
    QCOMPARE(urlConnected, url);

    QCOMPARE(socket.bytesToWrite(), 0);
    socket.sendTextMessage(QStringLiteral("Hello world!"));
    QVERIFY(socket.bytesToWrite() > 12); // 12 + a few extra bytes for header

    QVERIFY(textMessageReceived.wait(500));
    QCOMPARE(socket.bytesToWrite(), 0);

    QCOMPARE(textMessageReceived.size(), 1);
    QCOMPARE(binaryMessageReceived.size(), 0);
    QCOMPARE(binaryFrameReceived.size(), 0);
    arguments = textMessageReceived.takeFirst();
    QString messageReceived = arguments.at(0).toString();
    QCOMPARE(messageReceived, QStringLiteral("Hello world!"));

    QCOMPARE(textFrameReceived.size(), 1);
    arguments = textFrameReceived.takeFirst();
    QString frameReceived = arguments.at(0).toString();
    bool isLastFrame = arguments.at(1).toBool();
    QCOMPARE(frameReceived, QStringLiteral("Hello world!"));
    QVERIFY(isLastFrame);

    socket.close();
    socketConnectedSpy.clear();
    textMessageReceived.clear();
    textFrameReceived.clear();

    // QTBUG-74464 QWebsocket doesn't receive text (binary) message with size > 32 kb
    socket.open(url);

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socketError.size(), 0);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    arguments = serverConnectedSpy.takeFirst();
    urlConnected = arguments.at(0).toUrl();
    QCOMPARE(urlConnected, url);
    QCOMPARE(socket.bytesToWrite(), 0);

    // transmit a long text message with 1 MB
    QString longString(0x100000, 'a');
    socket.sendTextMessage(longString);
    QVERIFY(socket.bytesToWrite() > longString.size());
    QVERIFY(textMessageReceived.wait());
    QCOMPARE(socket.bytesToWrite(), 0);

    QCOMPARE(textMessageReceived.size(), 1);
    QCOMPARE(binaryMessageReceived.size(), 0);
    QCOMPARE(binaryFrameReceived.size(), 0);
    arguments = textMessageReceived.takeFirst();
    messageReceived = arguments.at(0).toString();
    QCOMPARE(messageReceived.size(), longString.size());
    QCOMPARE(messageReceived, longString);

    arguments = textFrameReceived.takeLast();
    isLastFrame = arguments.at(1).toBool();
    QVERIFY(isLastFrame);

    socket.close();
    socketConnectedSpy.clear();
    textMessageReceived.clear();
    textFrameReceived.clear();

    //QTBUG-36762: QWebSocket emits multiplied signals when socket was reopened
    socket.open(QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                     QStringLiteral(":") + QString::number(echoServer.port())));

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);

    socket.sendTextMessage(QStringLiteral("Hello world!"));

    QVERIFY(textMessageReceived.wait(500));
    QCOMPARE(textMessageReceived.size(), 1);
    QCOMPARE(binaryMessageReceived.size(), 0);
    QCOMPARE(binaryFrameReceived.size(), 0);
    arguments = textMessageReceived.takeFirst();
    messageReceived = arguments.at(0).toString();
    QCOMPARE(messageReceived, QStringLiteral("Hello world!"));

    QCOMPARE(textFrameReceived.size(), 1);
    arguments = textFrameReceived.takeFirst();
    frameReceived = arguments.at(0).toString();
    isLastFrame = arguments.at(1).toBool();
    QCOMPARE(frameReceived, QStringLiteral("Hello world!"));
    QVERIFY(isLastFrame);

    QString reason = QStringLiteral("going away");
    socket.close(QWebSocketProtocol::CloseCodeGoingAway, reason);
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeGoingAway);
    QCOMPARE(socket.closeReason(), reason);
}

void tst_QWebSocket::tst_sendBinaryMessage()
{
    EchoServer echoServer;

    QWebSocket socket;

    //should return 0 because socket is not open yet
    QCOMPARE(socket.sendBinaryMessage(QByteArrayLiteral("1234")), 0);

    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy textMessageReceived(&socket, SIGNAL(textMessageReceived(QString)));
    QSignalSpy textFrameReceived(&socket, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy binaryMessageReceived(&socket, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy binaryFrameReceived(&socket, SIGNAL(binaryFrameReceived(QByteArray,bool)));

    socket.open(QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                     QStringLiteral(":") + QString::number(echoServer.port())));

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);

    QCOMPARE(socket.bytesToWrite(), 0);
    socket.sendBinaryMessage(QByteArrayLiteral("Hello world!"));
    QVERIFY(socket.bytesToWrite() > 12); // 12 + a few extra bytes for header

    QVERIFY(binaryMessageReceived.wait(500));
    QCOMPARE(socket.bytesToWrite(), 0);

    QCOMPARE(textMessageReceived.size(), 0);
    QCOMPARE(textFrameReceived.size(), 0);
    QCOMPARE(binaryMessageReceived.size(), 1);
    QList<QVariant> arguments = binaryMessageReceived.takeFirst();
    QByteArray messageReceived = arguments.at(0).toByteArray();
    QCOMPARE(messageReceived, QByteArrayLiteral("Hello world!"));

    QCOMPARE(binaryFrameReceived.size(), 1);
    arguments = binaryFrameReceived.takeFirst();
    QByteArray frameReceived = arguments.at(0).toByteArray();
    bool isLastFrame = arguments.at(1).toBool();
    QCOMPARE(frameReceived, QByteArrayLiteral("Hello world!"));
    QVERIFY(isLastFrame);

    socket.close();

    //QTBUG-36762: QWebSocket emits multiple signals when socket is reopened
    socketConnectedSpy.clear();
    binaryMessageReceived.clear();
    binaryFrameReceived.clear();

    socket.open(QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                     QStringLiteral(":") + QString::number(echoServer.port())));

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);

    socket.sendBinaryMessage(QByteArrayLiteral("Hello world!"));

    QVERIFY(binaryMessageReceived.wait(500));
    QCOMPARE(textMessageReceived.size(), 0);
    QCOMPARE(textFrameReceived.size(), 0);
    QCOMPARE(binaryMessageReceived.size(), 1);
    arguments = binaryMessageReceived.takeFirst();
    messageReceived = arguments.at(0).toByteArray();
    QCOMPARE(messageReceived, QByteArrayLiteral("Hello world!"));

    QCOMPARE(binaryFrameReceived.size(), 1);
    arguments = binaryFrameReceived.takeFirst();
    frameReceived = arguments.at(0).toByteArray();
    isLastFrame = arguments.at(1).toBool();
    QCOMPARE(frameReceived, QByteArrayLiteral("Hello world!"));
    QVERIFY(isLastFrame);
}

void tst_QWebSocket::tst_ping()
{
    EchoServer echoServer;

    QWebSocket socket;

    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy serverConnectedSpy(&echoServer, QOverload<QUrl>::of(&EchoServer::newConnection));
    QSignalSpy pongReceived(&socket, &QWebSocket::pong);
    QSignalSpy textMessageReceived(&socket, &QWebSocket::textMessageReceived);
    QSignalSpy textFrameReceived(&socket, &QWebSocket::textFrameReceived);
    QSignalSpy binaryMessageReceived(&socket, &QWebSocket::binaryMessageReceived);
    QSignalSpy binaryFrameReceived(&socket, &QWebSocket::binaryFrameReceived);
    QSignalSpy socketError(&socket, &QWebSocket::errorOccurred);

    socket.ping(QByteArrayLiteral("Hello ping!")); // ping as no return value
    QVERIFY(!pongReceived.wait(500));
    QCOMPARE(pongReceived.size(), 0); // But we should have no pong result

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    url.setPath("/segment/with spaces");
    QUrlQuery query;
    query.addQueryItem("queryitem", "with encoded characters");
    url.setQuery(query);

    socket.open(url);

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QCOMPARE(socketError.size(), 0);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QList<QVariant> arguments = serverConnectedSpy.takeFirst();
    QUrl urlConnected = arguments.at(0).toUrl();
    QCOMPARE(urlConnected, url);

    QCOMPARE(socket.bytesToWrite(), 0);
    socket.ping(QByteArrayLiteral("Hello ping!"));
    QVERIFY(socket.bytesToWrite() > 11); // 11 + a few extra bytes for header

    QVERIFY(pongReceived.wait(500));
    QCOMPARE(socket.bytesToWrite(), 0);

    QCOMPARE(pongReceived.size(), 1);
    QCOMPARE(textMessageReceived.size(), 0);
    QCOMPARE(textFrameReceived.size(), 0);
    QCOMPARE(binaryMessageReceived.size(), 0);
    QCOMPARE(binaryFrameReceived.size(), 0);
    arguments = pongReceived.takeFirst();
    QString messageReceived = arguments.at(1).toString();
    bool elapsedIsInt;
    quint64 elapsed = arguments.at(0).toUInt(&elapsedIsInt);
    QCOMPARE(elapsedIsInt, true);
    QCOMPARE_LT(elapsed, 1000);
    QCOMPARE(messageReceived, QByteArrayLiteral("Hello ping!"));
    QByteArray longByteArray(125, 'a');
    longByteArray.append(125, 'b');
    socket.ping(longByteArray); // ping method should send only 125 first bytes
    QVERIFY(socket.bytesToWrite() > 125); // 125 + a few extra bytes for header

    QVERIFY(pongReceived.wait(500));
    QCOMPARE(socket.bytesToWrite(), 0);

    QCOMPARE(pongReceived.size(), 1);
    QCOMPARE(textMessageReceived.size(), 0);
    QCOMPARE(textFrameReceived.size(), 0);
    QCOMPARE(binaryMessageReceived.size(), 0);
    QCOMPARE(binaryFrameReceived.size(), 0);
    arguments = pongReceived.takeFirst();
    messageReceived = arguments.at(1).toString();
    elapsed = arguments.at(0).toUInt(&elapsedIsInt);
    QCOMPARE(elapsedIsInt, true);
    QCOMPARE_LT(elapsed, 1000);
    QCOMPARE(messageReceived, longByteArray.left(125)); // max length is of 125 for ping
    QCOMPARE(messageReceived[124], 'a'); // and we should still be in 'a' part

    socket.close();
    socketConnectedSpy.clear();
    textMessageReceived.clear();
    textFrameReceived.clear();
    pongReceived.clear();
}

void tst_QWebSocket::tst_errorString()
{
    //Check for QTBUG-37228: QWebSocket returns "Unknown Error" for known errors
    QWebSocket socket;

    //check that the default error string is empty
    QVERIFY(socket.errorString().isEmpty());

    QSignalSpy errorSpy(&socket, SIGNAL(error(QAbstractSocket::SocketError)));

    socket.open(QUrl(QStringLiteral("ws://someserver.on.mars:9999")));

    QTRY_COMPARE_WITH_TIMEOUT(errorSpy.size(), 1, 10000);
    QList<QVariant> arguments = errorSpy.takeFirst();
    QAbstractSocket::SocketError socketError =
            qvariant_cast<QAbstractSocket::SocketError>(arguments.at(0));
    QCOMPARE(socketError, QAbstractSocket::HostNotFoundError);
    QCOMPARE(socket.errorString(), QStringLiteral("Host not found"));

    // Check that handshake status code is parsed. The error is triggered by
    // refusing the origin authentication
    EchoServer echoServer;
    errorSpy.clear();
    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy serverConnectedSpy(&echoServer, SIGNAL(newConnection(QUrl)));
    connect(&echoServer, &EchoServer::originAuthenticationRequired,
            &socket, [](QWebSocketCorsAuthenticator* pAuthenticator){
        pAuthenticator->setAllowed(false);
    });

    socket.open(QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                     QStringLiteral(":") + QString::number(echoServer.port())));
    QTRY_VERIFY(errorSpy.size() > 0);
    QCOMPARE(serverConnectedSpy.size(), 0);
    QCOMPARE(socketConnectedSpy.size(), 0);
    QCOMPARE(socket.errorString(),
             QStringLiteral("QWebSocketPrivate::processHandshake: Unhandled http status code: 403"
                            " (Access Forbidden)."));
}

void tst_QWebSocket::tst_openRequest_data()
{
    QTest::addColumn<QStringList>("subprotocols");
    QTest::addColumn<QString>("subprotocolHeader");
    QTest::addColumn<QRegularExpression>("warningExpression");

    QTest::addRow("no subprotocols") << QStringList{} << QString{} << QRegularExpression{};
    QTest::addRow("single subprotocol") << QStringList{"foobar"} << QStringLiteral("foobar")
                                        << QRegularExpression{};
    QTest::addRow("multiple subprotocols") << QStringList{"foo", "bar"}
                                           << QStringLiteral("foo, bar")
                                           << QRegularExpression{};
    QTest::addRow("subprotocol with whitespace")
            << QStringList{"chat", "foo\r\nbar with space"}
            << QStringLiteral("chat")
            << QRegularExpression{".*invalid.*bar with space"};

    QTest::addRow("subprotocol with invalid chars")
            << QStringList{"chat", "foo{}"}
            << QStringLiteral("chat")
            << QRegularExpression{".*invalid.*foo"};
}

void tst_QWebSocket::tst_openRequest()
{
    QFETCH(QStringList, subprotocols);
    QFETCH(QString, subprotocolHeader);
    QFETCH(QRegularExpression, warningExpression);

    EchoServer echoServer;

    QWebSocket socket;

    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy serverRequestSpy(&echoServer, SIGNAL(newConnection(QNetworkRequest)));

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QLatin1Char(':') + QString::number(echoServer.port()));
    url.setPath(QLatin1String("/"));
    QUrlQuery query;
    query.addQueryItem("queryitem", "with encoded characters");
    url.setQuery(query);
    QNetworkRequest req(url);
    req.setRawHeader("X-Custom-Header", "A custom header");
    QWebSocketHandshakeOptions options;
    options.setSubprotocols(subprotocols);

    if (!warningExpression.pattern().isEmpty())
        QTest::ignoreMessage(QtWarningMsg, warningExpression);

    socket.open(req, options);

    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QTRY_COMPARE(serverRequestSpy.size(), 1);
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QList<QVariant> arguments = serverRequestSpy.takeFirst();
    QNetworkRequest requestConnected = arguments.at(0).value<QNetworkRequest>();
    QCOMPARE(requestConnected.url(), req.url());
    QCOMPARE(requestConnected.rawHeader("X-Custom-Header"), req.rawHeader("X-Custom-Header"));

    if (subprotocols.isEmpty())
        QVERIFY(!requestConnected.hasRawHeader("Sec-WebSocket-Protocol"));
    else
        QCOMPARE(requestConnected.rawHeader("Sec-WebSocket-Protocol"), subprotocolHeader);


    socket.close();
}

void tst_QWebSocket::tst_protocolAccessor()
{
    EchoServer echoServer;

    QWebSocket socket;

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QLatin1Char(':') + QString::number(echoServer.port()));

    QWebSocketHandshakeOptions options;
    options.setSubprotocols({ "foo", "protocol2" });

    socket.open(url, options);

    QTRY_COMPARE(socket.state(), QAbstractSocket::ConnectedState);

    QCOMPARE(socket.subprotocol(), "protocol2");

    socket.close();
}

void tst_QWebSocket::protocolsHeaderGeneration_data()
{
    QTest::addColumn<QStringList>("subprotocols");
    QTest::addColumn<int>("numInvalidEntries");

    using QSL = QStringList;
    QTest::addRow("all-invalid") << QSL{ "hello?", "------,,,,------" } << 2;
    QTest::addRow("one-valid") << QSL{ "hello?", "ImValid" } << 1;
    QTest::addRow("all-valid") << QSL{ "hello", "ImValid" } << 0;
}

// We test that the Sec-WebSocket-Protocol header is generated normally in presence
// of one or more invalid entries. That is, it should not be included at all
// if there are no valid entries, and there should be no separators with only
// one valid entry.
void tst_QWebSocket::protocolsHeaderGeneration()
{
    QFETCH(const QStringList, subprotocols);
    QFETCH(const int, numInvalidEntries);
    const bool containsValidEntry = numInvalidEntries != subprotocols.size();

    QTcpServer tcpServer;
    QVERIFY(tcpServer.listen());

    QWebSocket socket;

    QUrl url = QUrl("ws://127.0.0.1:%1"_L1.arg(QString::number(tcpServer.serverPort())));

    QWebSocketHandshakeOptions options;
    options.setSubprotocols(subprotocols);

    QCOMPARE(options.subprotocols().size(), subprotocols.size());
    for (int i = 0; i < numInvalidEntries; ++i) {
        QTest::ignoreMessage(QtMsgType::QtWarningMsg,
                QRegularExpression("Ignoring invalid WebSocket subprotocol name \".*\""));
    }
    socket.open(url, options);

    QTRY_VERIFY(tcpServer.hasPendingConnections());
    QTcpSocket *serverSocket = tcpServer.nextPendingConnection();
    QVERIFY(serverSocket);

    bool hasSeenHeader = false;
    while (serverSocket->state() == QAbstractSocket::ConnectedState) {
        if (!serverSocket->canReadLine()) {
            QTRY_VERIFY2(serverSocket->canReadLine(),
                    "Reached end-of-data without seeing end-of-header!");
        }
        const QByteArray fullLine = serverSocket->readLine();
        QByteArrayView line = fullLine;
        if (line == "\r\n") // End-of-Header
            break;
        QByteArrayView headerPrefix = "Sec-WebSocket-Protocol:";
        if (line.size() < headerPrefix.size())
            continue;
        if (line.first(headerPrefix.size()).compare(headerPrefix, Qt::CaseInsensitive) != 0)
            continue;
        hasSeenHeader = true;
        QByteArrayView protocols = line.sliced(headerPrefix.size()).trimmed();
        QVERIFY(!protocols.empty());
        QCOMPARE(protocols.count(','), subprotocols.size() - numInvalidEntries - 1);
        // Keep going in case we encounter the header again
    }
    QCOMPARE(hasSeenHeader, containsValidEntry);
    serverSocket->disconnectFromHost();
}

class WebSocket : public QWebSocket
{
    Q_OBJECT

public:
    explicit WebSocket()
    {
        connect(this, SIGNAL(triggerClose()), SLOT(onClose()), Qt::QueuedConnection);
        connect(this, SIGNAL(triggerOpen(QUrl)), SLOT(onOpen(QUrl)), Qt::QueuedConnection);
        connect(this, SIGNAL(triggerSendTextMessage(QString)), SLOT(onSendTextMessage(QString)), Qt::QueuedConnection);
        connect(this, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)), Qt::QueuedConnection);
    }

    void asyncClose() { triggerClose(); }
    void asyncOpen(const QUrl &url) { triggerOpen(url); }
    void asyncSendTextMessage(const QString &msg) { triggerSendTextMessage(msg); }

    QString receivedMessage;

Q_SIGNALS:
    void triggerClose();
    void triggerOpen(const QUrl &);
    void triggerSendTextMessage(const QString &);
    void done();

private Q_SLOTS:
    void onClose() { close(); }
    void onOpen(const QUrl &url) { open(url); }
    void onSendTextMessage(const QString &msg) { sendTextMessage(msg); }
    void onTextMessageReceived(const QString &msg) { receivedMessage = msg; done(); }
};

struct Warned
{
    static QtMessageHandler origHandler;
    static bool warned;
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& str)
    {
        if (type == QtWarningMsg) {
            warned = true;
        }
        if (origHandler)
            origHandler(type, context, str);
    }
};
QtMessageHandler Warned::origHandler = nullptr;
bool Warned::warned = false;


void tst_QWebSocket::tst_moveToThread()
{
    Warned::origHandler = qInstallMessageHandler(&Warned::messageHandler);

    EchoServer echoServer;

    QThread* thread = new QThread(this);
    thread->start();

    WebSocket* socket = new WebSocket;
    socket->moveToThread(thread);

    const QString textMessage = QStringLiteral("Hello world!");
    QSignalSpy socketConnectedSpy(socket, SIGNAL(connected()));
    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    url.setPath("/segment/with spaces");
    QUrlQuery query;
    query.addQueryItem("queryitem", "with encoded characters");
    url.setQuery(query);

    socket->asyncOpen(url);
    if (socketConnectedSpy.size() == 0)
        QVERIFY(socketConnectedSpy.wait(500));

    socket->asyncSendTextMessage(textMessage);

    QTimer timer;
    timer.setInterval(1000);
    timer.start();
    QEventLoop loop;
    connect(socket, SIGNAL(done()), &loop, SLOT(quit()));
    connect(socket, SIGNAL(done()), &timer, SLOT(stop()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();

    socket->asyncClose();

    QTRY_COMPARE_WITH_TIMEOUT(loop.isRunning(), false, 200);
    QCOMPARE(socket->receivedMessage, textMessage);

    socket->deleteLater();
    thread->quit();
    thread->wait();
}

void tst_QWebSocket::tst_moveToThreadNoWarning()
{
    // check for warnings in tst_moveToThread()
    // couldn't done there because warnings are processed after the test run
    QCOMPARE(Warned::warned, false);
}


#ifndef QT_NO_NETWORKPROXY
void tst_QWebSocket::tst_setProxy()
{
    // check if property assignment works as expected.
    QWebSocket socket;
    QCOMPARE(socket.proxy(), QNetworkProxy(QNetworkProxy::DefaultProxy));

    QNetworkProxy proxy;
    proxy.setPort(123);
    socket.setProxy(proxy);
    QCOMPARE(socket.proxy(), proxy);

    proxy.setPort(321);
    QCOMPARE(socket.proxy().port(), quint16(123));
    socket.setProxy(proxy);
    QCOMPARE(socket.proxy(), proxy);
}
#endif // QT_NO_NETWORKPROXY

class AuthServer : public QTcpServer
{
    Q_OBJECT
public:
    AuthServer()
    {
        connect(this, &QTcpServer::pendingConnectionAvailable, this, &AuthServer::handleConnection);
    }

    void incomingConnection(qintptr sockfd) override
    {
        if (withEncryption) {
#if QT_CONFIG(ssl)
            auto *sslSocket = new QSslSocket(this);
            connect(sslSocket, &QSslSocket::encrypted, this,
                [this, sslSocket]() {
                    addPendingConnection(sslSocket);
                });
            sslSocket->setSslConfiguration(configuration);
            sslSocket->setSocketDescriptor(sockfd);
            sslSocket->startServerEncryption();
#else
            QFAIL("withEncryption should not be 'true' if we don't have TLS");
#endif
        } else {
            QTcpSocket *socket = new QTcpSocket(this);
            socket->setSocketDescriptor(sockfd);
            addPendingConnection(socket);
        }
    }

    void handleConnection()
    {
        QTcpSocket *serverSocket = nextPendingConnection();
        connect(serverSocket, &QTcpSocket::readyRead, this, &AuthServer::handleReadyRead);
    }

    void handleReadyRead()
    {
        auto *serverSocket = qobject_cast<QTcpSocket *>(sender());
        incomingData.append(serverSocket->readAll());
        if (finished) {
            qWarning() << "Unexpected trailing data..." << incomingData;
            return;
        }
        if (!incomingData.contains("\r\n\r\n")) {
            qDebug("Not all of the data arrived at once, waiting for more...");
            return;
        }
        // Move incomingData into local variable and reset it since we received it all:
        const QByteArray fullHeader = std::exchange(incomingData, {});

        QLatin1StringView authView = getHeaderValue("Authorization"_L1, fullHeader);
        if (authView.isEmpty())
            return writeAuthRequired(serverSocket);
        qsizetype sep = authView.indexOf(' ');
        if (sep == -1)
            return writeAuthRequired(serverSocket);
        QLatin1StringView authenticateMethod = authView.first(sep);
        QLatin1StringView authenticateAttempt = authView.sliced(sep + 1);
        if (authenticateMethod != "Basic" || authenticateAttempt != expectedBasicPayload())
            return writeAuthRequired(serverSocket);

        QLatin1StringView keyView = getHeaderValue("Sec-WebSocket-Key"_L1, fullHeader);
        QVERIFY(!keyView.isEmpty());

        const QByteArray accept =
                QByteArrayView(keyView) % QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        auto generatedKey = QCryptographicHash::hash(accept, QCryptographicHash::Sha1).toBase64();
        serverSocket->write("HTTP/1.1 101 Switching Protocols\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Accept: " % generatedKey % "\r\n"
                            "\r\n");
        finished = true;
    }

    void writeAuthRequired(QTcpSocket *socket) const
    {
        QByteArray payload = "HTTP/1.1 401 UNAUTHORIZED\r\n"
            "WWW-Authenticate: Basic realm=shadow\r\n";
        if (withConnectionClose)
            payload.append("Connection: Close\r\n");
        else if (withContentLength)
            payload.append("Content-Length: " % QByteArray::number(body.size()) % "\r\n");
        payload.append("\r\n");

        if (withBody)
            payload.append(body);

        socket->write(payload);
        if (withConnectionClose)
            socket->close();
    }

    static QLatin1StringView getHeaderValue(const QLatin1StringView keyHeader,
                                            const QByteArrayView fullHeader)
    {
        const auto fullHeaderView = QLatin1StringView(fullHeader);
        const qsizetype headerStart = fullHeaderView.indexOf(keyHeader, 0, Qt::CaseInsensitive);
        if (headerStart == -1)
            return {};
        qsizetype valueStart = headerStart + keyHeader.size();
        Q_ASSERT(fullHeaderView.size() > valueStart);
        Q_ASSERT(fullHeaderView[valueStart] == ':');
        ++valueStart;
        const qsizetype valueEnd = fullHeaderView.indexOf(QLatin1StringView("\r\n"), valueStart);
        if (valueEnd == -1)
            return {};
        return fullHeaderView.sliced(valueStart, valueEnd - valueStart).trimmed();
    }

    static QByteArray expectedBasicPayload()
    {
        return QByteArray(user % ':' % password).toBase64();
    }

    static constexpr QByteArrayView user = "user";
    static constexpr QByteArrayView password = "password";
    static constexpr QUtf8StringView body = "Authorization required";

    bool withBody = false;
    bool withContentLength = true;
    bool withConnectionClose = false;
    bool withEncryption = false;
#if QT_CONFIG(ssl)
    QSslConfiguration configuration;
#endif

private:
    QByteArray incomingData;
    bool finished = false;
};

struct ServerScenario {
    QByteArrayView label;
    bool withContentLength = false;
    bool withBody = false;
    bool withConnectionClose = false;
    bool withEncryption = false;
};
struct Credentials { QString username, password; };
struct ClientScenario {
    QByteArrayView label;
    Credentials urlCredentials;
    QVector<Credentials> callbackCredentials;
    bool expectSuccess = true;
};

void tst_QWebSocket::authenticationRequired_data()
{
    const QString correctUser = QString::fromUtf8(AuthServer::user.toByteArray());
    const QString correctPassword = QString::fromUtf8(AuthServer::password.toByteArray());

    QTest::addColumn<ServerScenario>("serverScenario");
    QTest::addColumn<ClientScenario>("clientScenario");

    // Need to test multiple server scenarios:
    // 1. Normal server (connection: keep-alive, Content-Length)
    // 2. Older server (connection: close, Content-Length)
    // 3. Even older server (connection: close, no Content-Length)
    // 4. Strange server (connection: close, no Content-Length, no body)
    // 5. Quiet server (connection: keep-alive, no Content-Length, no body)
    ServerScenario serverScenarios[] = {
        { "normal-server", true, true, false, false },
        { "connection-close", true, true, true, false },
        { "connection-close-no-content-length", false, true, true, false },
        { "connection-close-no-content-length-no-body", false, false, true, false },
        { "keep-alive-no-content-length-no-body", false, false, false, false },
    };

    // And some client scenarios
    // 1. User/pass supplied in url
    // 2. User/pass supplied in callback
    // 3. _Wrong_ user/pass supplied in URL, correct in callback
    // 4. _Wrong_ user/pass supplied in URL, _wrong_ supplied in callback
    // 5. No user/pass supplied in URL, nothing supplied in callback
    // 5. No user/pass supplied in URL, wrong, then correct, supplied in callback
    ClientScenario clientScenarios[]{
        { "url-ok", {correctUser, correctPassword}, {} },
        { "callback-ok", {}, { {correctUser, correctPassword } } },
        { "url-wrong-callback-ok", {u"admin"_s, u"admin"_s}, { {correctUser, correctPassword} } },
        { "url-wrong-callback-wrong", {u"admin"_s, u"admin"_s}, { {u"test"_s, u"test"_s} }, false },
        { "no-creds", {{}, {}}, {}, false },
        { "url-wrong-callback-2-ok", {u"admin"_s, u"admin"_s}, { {u"test"_s, u"test"_s}, {correctUser , correctPassword} } },
    };

    for (auto &server : serverScenarios) {
        for (auto &client : clientScenarios) {
            QTest::addRow("Server:%s,Client:%s", server.label.data(), client.label.data())
                    << server << client;
        }
    }
#if QT_CONFIG(ssl)
    if (!QSslSocket::supportsSsl()) {
        qDebug("Skipping the SslServer part of this test because proper TLS is not supported.");
        return;
    }
    if (QTestPrivate::isSecureTransportBlockingTest()) {
        qDebug("SecureTransport is blocking in keychain access.");
        return;
    }

    // And double that, but now with TLS
    for (auto &server : serverScenarios) {
        server.withEncryption = true;
        for (auto &client : clientScenarios) {
            QTest::addRow("SslServer:%s,Client:%s", server.label.data(), client.label.data())
                    << server << client;
        }
    }
#endif
}

void tst_QWebSocket::authenticationRequired()
{
    QFETCH(const ServerScenario, serverScenario);
    QFETCH(const ClientScenario, clientScenario);

    int credentialIndex = 0;
    auto handleAuthenticationRequired = [&clientScenario,
                                         &credentialIndex](QAuthenticator *authenticator) {
        if (credentialIndex == clientScenario.callbackCredentials.size()) {
            if (clientScenario.expectSuccess)
                QFAIL("Ran out of credentials to try, but failed to authorize!");
            if (clientScenario.callbackCredentials.isEmpty())
                return;
            // If we don't expect to succeed, retry the last returned credentials.
            // QAuthenticator should notice there is no change in user/pass and
            // ignore it, leading to authentication failure.
            --credentialIndex;
        }
        // Verify that realm parsing works:
        QCOMPARE_EQ(authenticator->realm(), u"shadow"_s);

        Credentials credentials = clientScenario.callbackCredentials[credentialIndex++];
        authenticator->setUser(credentials.username);
        authenticator->setPassword(credentials.password);
    };

    AuthServer server;
    server.withBody = serverScenario.withBody;
    server.withContentLength = serverScenario.withContentLength;
    server.withConnectionClose = serverScenario.withConnectionClose;
    server.withEncryption = serverScenario.withEncryption;
#if QT_CONFIG(ssl)
    if (serverScenario.withEncryption) {
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        QList<QSslCertificate> certificates = QSslCertificate::fromPath(u":/localhost.cert"_s);
        QVERIFY(!certificates.isEmpty());
        config.setLocalCertificateChain(certificates);
        QFile keyFile(u":/localhost.key"_s);
        QVERIFY(keyFile.open(QIODevice::ReadOnly));
        config.setPrivateKey(QSslKey(keyFile.readAll(), QSsl::Rsa));
        server.configuration = config;
    }
#endif

    QVERIFY(server.listen());
    QUrl url = QUrl(u"ws://127.0.0.1"_s);
    if (serverScenario.withEncryption)
        url.setScheme(u"wss"_s);
    url.setPort(server.serverPort());
    url.setUserName(clientScenario.urlCredentials.username);
    url.setPassword(clientScenario.urlCredentials.password);

    QWebSocket socket;
    QSignalSpy connectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy errorSpy(&socket, &QWebSocket::errorOccurred);
    QSignalSpy stateChangedSpy(&socket, &QWebSocket::stateChanged);
    connect(&socket, &QWebSocket::authenticationRequired, &socket, handleAuthenticationRequired);
#if QT_CONFIG(ssl)
    if (serverScenario.withEncryption) {
        auto config = socket.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        socket.setSslConfiguration(config);
        QObject::connect(&socket, &QWebSocket::sslErrors, &socket,
                qOverload<>(&QWebSocket::ignoreSslErrors));
    }
#endif
    socket.open(url);

    if (clientScenario.expectSuccess) {
        // Wait for connected!
        QTRY_COMPARE_EQ(connectedSpy.size(), 1);
        QCOMPARE_EQ(errorSpy.size(), 0);
        // connecting->connected
        const int ExpectedStateChanges = 2;
        QTRY_COMPARE_EQ(stateChangedSpy.size(), ExpectedStateChanges);
        auto firstState = stateChangedSpy.at(0).front().value<QAbstractSocket::SocketState>();
        QCOMPARE_EQ(firstState, QAbstractSocket::ConnectingState);
        auto secondState = stateChangedSpy.at(1).front().value<QAbstractSocket::SocketState>();
        QCOMPARE_EQ(secondState, QAbstractSocket::ConnectedState);
    } else {
        // Wait for error!
        QTRY_COMPARE_EQ(errorSpy.size(), 1);
        QCOMPARE_EQ(connectedSpy.size(), 0);
        // connecting->unconnected
        const int ExpectedStateChanges = 2;
        QTRY_COMPARE_EQ(stateChangedSpy.size(), ExpectedStateChanges);
        auto firstState = stateChangedSpy.at(0).front().value<QAbstractSocket::SocketState>();
        QCOMPARE_EQ(firstState, QAbstractSocket::ConnectingState);
        auto secondState = stateChangedSpy.at(1).front().value<QAbstractSocket::SocketState>();
        QCOMPARE_EQ(secondState, QAbstractSocket::UnconnectedState);
    }
}

void tst_QWebSocket::overlongCloseReason()
{
    EchoServer echoServer;

    QWebSocket socket;

    //should return 0 because socket is not open yet
    QCOMPARE(socket.sendTextMessage(QStringLiteral("1234")), 0);

    QSignalSpy socketConnectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy socketDisconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy serverConnectedSpy(&echoServer, SIGNAL(newConnection(QUrl)));

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    socket.open(url);
    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QTRY_COMPARE(serverConnectedSpy.size(), 1);

    const QString reason(200, QChar::fromLatin1('a'));
    socket.close(QWebSocketProtocol::CloseCodeGoingAway, reason);
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeGoingAway);
    // Max length of a control frame is 125, but 2 bytes are used for the close code:
    QCOMPARE(socket.closeReason().size(), 123);
    QCOMPARE(socket.closeReason(), reason.left(123));
    QTRY_COMPARE(socketDisconnectedSpy.size(), 1);
}

void tst_QWebSocket::incomingMessageTooLong()
{
//QTBUG-70693
    quint64 maxAllowedIncomingMessageSize = 1024;
    quint64 maxAllowedIncomingFrameSize = QWebSocket::maxIncomingFrameSize();

    EchoServer echoServer(nullptr, maxAllowedIncomingMessageSize, maxAllowedIncomingFrameSize);

    QWebSocket socket;

    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy serverConnectedSpy(&echoServer, QOverload<QUrl>::of(&EchoServer::newConnection));
    QSignalSpy socketDisconnectedSpy(&socket, &QWebSocket::disconnected);

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    socket.open(url);
    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QTRY_COMPARE(serverConnectedSpy.size(), 1);

    QString payload(maxAllowedIncomingMessageSize+1, 'a');
    QCOMPARE(socket.sendTextMessage(payload), payload.size());

    QTRY_COMPARE(socketDisconnectedSpy.size(), 1);
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeTooMuchData);
}

void tst_QWebSocket::incomingFrameTooLong()
{
//QTBUG-70693
    quint64 maxAllowedIncomingMessageSize = QWebSocket::maxIncomingMessageSize();
    quint64 maxAllowedIncomingFrameSize = 1024;

    EchoServer echoServer(nullptr, maxAllowedIncomingMessageSize, maxAllowedIncomingFrameSize);

    QWebSocket socket;
    socket.setOutgoingFrameSize(maxAllowedIncomingFrameSize+1);

    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy serverConnectedSpy(&echoServer, QOverload<QUrl>::of(&EchoServer::newConnection));
    QSignalSpy socketDisconnectedSpy(&socket, &QWebSocket::disconnected);

    QUrl url = QUrl(QStringLiteral("ws://") + echoServer.hostAddress().toString() +
                    QStringLiteral(":") + QString::number(echoServer.port()));
    socket.open(url);
    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QTRY_COMPARE(serverConnectedSpy.size(), 1);

    QString payload(maxAllowedIncomingFrameSize+1, 'a');
    QCOMPARE(socket.sendTextMessage(payload), payload.size());

    QTRY_COMPARE(socketDisconnectedSpy.size(), 1);
    QCOMPARE(socket.closeCode(), QWebSocketProtocol::CloseCodeTooMuchData);
}

void tst_QWebSocket::testingFrameAndMessageSizeApi()
{
//requested by André Hartmann, QTBUG-70693
    QWebSocket socket;

    const quint64 outgoingFrameSize = 5;
    socket.setOutgoingFrameSize(outgoingFrameSize);
    QTRY_COMPARE(outgoingFrameSize, socket.outgoingFrameSize());

    const quint64 maxAllowedIncomingFrameSize = 9;
    socket.setMaxAllowedIncomingFrameSize(maxAllowedIncomingFrameSize);
    QTRY_COMPARE(maxAllowedIncomingFrameSize, socket.maxAllowedIncomingFrameSize());

    const quint64 maxAllowedIncomingMessageSize = 889;
    socket.setMaxAllowedIncomingMessageSize(maxAllowedIncomingMessageSize);
    QTRY_COMPARE(maxAllowedIncomingMessageSize, socket.maxAllowedIncomingMessageSize());
}

void tst_QWebSocket::hostHeaderFromNetworkRequest()
{
    QTcpServer server;
    QSignalSpy serverSpy(&server, &QTcpServer::newConnection);

    QVERIFY(server.listen(QHostAddress(u"127.0.0.1"_s)));

    QUrl url = QUrl(u"ws://127.0.0.1"_s);
    url.setPort(server.serverPort());
    QNetworkRequest request(url);
    request.setRawHeader("Host", "localhost");

    QWebSocket socket;
    socket.open(request);

    QVERIFY(serverSpy.wait());
    std::unique_ptr<QTcpSocket> serverSocket;
    serverSocket.reset(server.nextPendingConnection());
    QSignalSpy serverSocketSpy(serverSocket.get(), &QIODevice::readyRead);
    QByteArray data;
    while (!data.contains("\r\n\r\n")) {
        QVERIFY(serverSocketSpy.wait());
        data.append(serverSocket->readAll());
    }

    const auto headerEndIndex = data.indexOf("\r\n\r\n");
    QCOMPARE_NE(headerEndIndex, -1);
    const auto headerView = QLatin1StringView(QByteArrayView(data).first(headerEndIndex + 2));
    QCOMPARE(headerView.count("Host: "_L1, Qt::CaseInsensitive), 1);
    QVERIFY(headerView.contains("\r\nHost: localhost\r\n"_L1, Qt::CaseInsensitive));

    const QLatin1StringView keyView =
            AuthServer::getHeaderValue("Sec-WebSocket-Key"_L1, QByteArrayView(data));
    QVERIFY(!keyView.isEmpty());
    const QByteArray accept =
            QByteArrayView(keyView) % "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"_ba;
    serverSocket->write(
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: "
            % QCryptographicHash::hash(accept, QCryptographicHash::Sha1).toBase64()
            % "\r\n\r\n");

    QSignalSpy connectedSpy(&socket, &QWebSocket::connected);
    QVERIFY(connectedSpy.wait());
}

void tst_QWebSocket::customHeader()
{
    QTcpServer server;
    QSignalSpy serverSpy(&server, &QTcpServer::newConnection);

    server.listen();
    QUrl url = QUrl(QStringLiteral("ws://127.0.0.1"));
    url.setPort(server.serverPort());

    QNetworkRequest request(url);
    request.setRawHeader("CustomHeader", "Example");
    QWebSocket socket;
    socket.open(request);

    // Custom websocket server below (needed because a QWebSocketServer on
    // localhost doesn't show the issue):
    QVERIFY(serverSpy.wait());
    QTcpSocket *serverSocket = server.nextPendingConnection();
    QSignalSpy serverSocketSpy(serverSocket, &QTcpSocket::readyRead);
    QByteArray data;
    while (!data.contains("\r\n\r\n")) {
        QVERIFY(serverSocketSpy.wait());
        data.append(serverSocket->readAll());
    }
    QVERIFY(QLatin1StringView(data).contains("CustomHeader: Example"_L1, Qt::CaseInsensitive));
    const auto view = QLatin1String(data);
    const auto keyHeader = QLatin1String("Sec-WebSocket-Key:");
    const qsizetype keyStart = view.indexOf(keyHeader, 0, Qt::CaseInsensitive) + keyHeader.size();
    QVERIFY(keyStart != -1);
    const qsizetype keyEnd = view.indexOf(QLatin1String("\r\n"), keyStart);
    QVERIFY(keyEnd != -1);
    const QLatin1String keyView = view.sliced(keyStart, keyEnd - keyStart).trimmed();
    const QByteArray accept =
            QByteArrayView(keyView) % QByteArrayLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    serverSocket->write(
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: "
            % QCryptographicHash::hash(accept, QCryptographicHash::Sha1).toBase64()
        ); // trailing \r\n\r\n intentionally left off to make the client wait for it
    serverSocket->flush();
    // This would freeze prior to the fix for QTBUG-102111, because the client would loop forever.
    // We use qWait to give the OS some time to move the bytes over to the client and push the event
    // to our eventloop.
    QTest::qWait(100);
    serverSocket->write("\r\n\r\n");

    // And check the client properly connects:
    QSignalSpy connectedSpy(&socket, &QWebSocket::connected);
    QVERIFY(connectedSpy.wait());
}

void tst_QWebSocket::splitUtf8Sequence()
{
    EchoServer echoServer;
    QWebSocket socket;
    socket.setOutgoingFrameSize(10ull);
    // prepare payload, just a bunch of bytes and a sequence at the end that will be split across
    // two frames
    QString payload;
    constexpr qsizetype Overhead = 2; // overhead for this message
    payload.reserve(qsizetype(socket.outgoingFrameSize() - Overhead) + 2);
    payload.assign(qsizetype(socket.outgoingFrameSize() - Overhead) - 1, u'a');
    payload += u"🙂";
    QSignalSpy socketConnectedSpy(&socket, &QWebSocket::connected);
    QSignalSpy serverConnectedSpy(&echoServer, qOverload<QUrl>(&EchoServer::newConnection));
    QSignalSpy messageReceivedSpy(&socket, &QWebSocket::textMessageReceived);
    QSignalSpy errorOccurredSpy(&socket, &QWebSocket::errorOccurred);
    const QUrl url = QUrl(u"ws://127.0.0.1:"_s + QString::number(echoServer.port()));
    socket.open(url);
    QTRY_COMPARE(socketConnectedSpy.size(), 1);
    QTRY_COMPARE(serverConnectedSpy.size(), 1);
    qint64 bytesWritten = socket.sendTextMessage(payload);
    QCOMPARE(bytesWritten, payload.size() + Overhead);
    QTRY_COMPARE(messageReceivedSpy.size(), 1);
    QTRY_COMPARE(errorOccurredSpy.size(), 0);
    QCOMPARE(messageReceivedSpy.at(0).at(0).toString(), payload);
}

QTEST_MAIN(tst_QWebSocket)

#include "tst_qwebsocket.moc"
