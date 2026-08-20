// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mytestserver.h"
#include "rep_subclass_source.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        const auto runMode = qEnvironmentVariable("RunMode");
        const auto objectMode = qEnvironmentVariable("ObjectMode");

        QRemoteObjectHost srcNode(QUrl(QStringLiteral("tcp://127.0.0.1:65217")));
        MyTestServer myTestServer;
        SubClassSimpleSource subclass;
        QStringListModel model;
        const MyPOD initialValue(42, 3.14f, QStringLiteral("SubClass"));
        if (objectMode == QLatin1String("ObjectPointer")) {
            subclass.setMyPOD(initialValue);
            model.setStringList(QStringList() << "Track1" << "Track2" << "Track3");
            myTestServer.setSubClass(&subclass);
            myTestServer.setTracks(&model);
        }
        srcNode.enableRemoting(&myTestServer);

        qDebug() << "Waiting for incoming connections";

        QSignalSpy waitForStartedSpy(&myTestServer, &MyTestServer::startedChanged);
        QVERIFY(waitForStartedSpy.isValid());
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), true);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Client connected";
        if (runMode != QLatin1String("Baseline")) {
            qDebug() << "Server quitting" << runMode;
            if (runMode == QLatin1String("ServerRestartFatal"))
                qFatal("Fatal");
            QCoreApplication::exit();
            return;
        }
        emit myTestServer.advance();

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
