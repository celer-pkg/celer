// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mytestserver.h"
#include "rep_PodInterface_source.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectHost srcNode(QUrl(QStringLiteral("tcp://127.0.0.1:65213")));
        MyTestServer myTestServer;
        bool templated = qEnvironmentVariableIsSet("TEMPLATED_REMOTING");
        if (templated)
            srcNode.enableRemoting<MyInterfaceSourceAPI>(&myTestServer);
        else
            srcNode.enableRemoting(&myTestServer);

        PodInterfaceSimpleSource myPodSource;
        myPodSource.setMyPod(MyPOD(1, 5.0, "test"));
        if (templated)
            srcNode.enableRemoting<PodInterfaceSourceAPI>(&myPodSource);
        else
            srcNode.enableRemoting(&myPodSource);

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

        // END: Testing

        waitForStartedSpy.clear();
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), false);

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
