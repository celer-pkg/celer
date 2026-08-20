// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mytestserver.h"

#include <QCoreApplication>
#include <QTcpServer>
#include <QtTest/QtTest>
#include <QTcpSocket>

const QUrl registryUrl = QUrl(QStringLiteral("tcp://127.0.0.1:65212"));
const QUrl extUrl = QUrl(QStringLiteral("exttcp://127.0.0.1:65213"));
const QUrl extUrl2 = QUrl(QStringLiteral("exttcp://127.0.0.1:65214"));

class tst_Server_Process : public QObject
{
    Q_OBJECT

    struct Device
    {
        Device(QUrl url) : srcNode(url, registryUrl, QRemoteObjectHost::AllowExternalRegistration)
        {
            tcpServer.listen(QHostAddress(url.host()), quint16(url.port()));
            QVERIFY(srcNode.waitForRegistry(3000));
            QObject::connect(&tcpServer, &QTcpServer::newConnection, &tcpServer, [this]() {
                auto conn = this->tcpServer.nextPendingConnection();
                this->srcNode.addHostSideConnection(conn);
            });
        }
        QTcpServer tcpServer;
        QRemoteObjectHost srcNode;
    };

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectRegistryHost registry(registryUrl);

        Device dev1(extUrl);
        MyTestServer myTestServer;
        bool templated = qEnvironmentVariableIsSet("TEMPLATED_REMOTING");
        if (templated)
            QVERIFY(dev1.srcNode.enableRemoting<MyInterfaceSourceAPI>(&myTestServer));
        else
            QVERIFY(dev1.srcNode.enableRemoting(&myTestServer));

        qDebug() << "Waiting for incoming connections";

        QSignalSpy waitForStartedSpy(&myTestServer, &MyTestServer::startedChanged);
        QVERIFY(waitForStartedSpy.isValid());
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), true);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Client connected";

        // BEGIN: Testing

        // make sure continuous changes to enums don't mess up the protocol
        myTestServer.setEnum1(MyTestServer::Second);
        myTestServer.setEnum1(MyTestServer::Third);

        emit myTestServer.advance();

        waitForStartedSpy.clear();
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), false);

        bool next = false;
        connect(&myTestServer, &MyTestServer::nextStep, this, [&next] { next = true; });
        QTRY_VERIFY_WITH_TIMEOUT(next, 5000);

        qDebug() << "Disable remoting";
        QVERIFY(dev1.srcNode.disableRemoting(&myTestServer));

        // Wait before changing the state
        QTest::qWait(200);

        // Change a value while replica is suspect
        myTestServer.setEnum1(MyTestServer::First);

        // Share the object on a different "device", make sure registry updates and connects
        qDebug() << "Enable remoting";
        Device dev2(extUrl2);
        QVERIFY(dev2.srcNode.enableRemoting(&myTestServer));

        // wait for quit
        bool quit = false;
        connect(&myTestServer, &MyTestServer::quitApp, this, [&quit] { quit = true; });
        QTRY_VERIFY_WITH_TIMEOUT(quit, 5000);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Done. Shutting down.";
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
