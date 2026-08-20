// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QRemoteObjectNode>
#include <QScopedPointer>
#include "rep_pingpong_replica.h"

#include "../../../shared/testutils.h"

class tst_clientSSL: public QObject
{
    Q_OBJECT
public:
    tst_clientSSL() = default;

private slots:
    void initTestCase()
    {
        QVERIFY(TestUtils::init("tst_client"));
    }
    void testRun()
    {
// TODO: This a limitation of QProcess on Android, QTBUG-88507 is relevant to this issue.
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        serverProc.start(TestUtils::findExecutable("sslTestServer", "/sslTestServer"),
                         QStringList());
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);
        QRemoteObjectNode m_client;
        auto config = QSslConfiguration::defaultConfiguration();
        config.setCaCertificates(QSslCertificate::fromPath(QStringLiteral(":/sslcert/rootCA.pem")));
        QSslConfiguration::setDefaultConfiguration(config);

        QScopedPointer<QSslSocket> socketClient{new QSslSocket};
        socketClient->setLocalCertificate(QStringLiteral(":/sslcert/client.crt"));
        socketClient->setPrivateKey(QStringLiteral(":/sslcert/client.key"));
        socketClient->setPeerVerifyMode(QSslSocket::VerifyPeer);
        socketClient->connectToHostEncrypted(QStringLiteral("127.0.0.1"), 65111);
        QVERIFY(socketClient->waitForEncrypted(-1));

        connect(socketClient.data(), &QSslSocket::errorOccurred,
                socketClient.data(), [](QAbstractSocket::SocketError error){
            QCOMPARE(error, QAbstractSocket::RemoteHostClosedError);
        });
        m_client.addClientSideConnection(socketClient.data());

        QScopedPointer<PingPongReplica> pp{m_client.acquire<PingPongReplica>()};
        QVERIFY(pp->waitForSource());

        QString pongStr;
        connect(pp.data(), &PingPongReplica::pong, this, [&pongStr](const QString &str) {
            pongStr = str;
        });
        pp->ping("yahoo");
        QTRY_COMPARE(pongStr, "Pong yahoo");
        pp->ping("one more");
        QTRY_COMPARE(pongStr, "Pong one more");
        pp->ping("last one");
        QTRY_COMPARE(pongStr, "Pong last one");
        pp->quit();
        QTRY_VERIFY(serverProc.state() != QProcess::Running);
        QCOMPARE(serverProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_clientSSL)

#include "tst_client.moc"
