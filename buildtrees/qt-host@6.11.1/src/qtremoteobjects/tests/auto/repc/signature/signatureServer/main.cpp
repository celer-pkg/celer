// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_server_source.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

class MyTestServer : public TestClassSimpleSource
{
    Q_OBJECT
public:
    bool shouldQuit = false;
    // TestClassSimpleSource interface
public slots:
    bool slot1() override {return true;}
    QString slot2() override {return QLatin1String("Hello there");}
    void ping(const QString &message) override
    {
        emit pong(message);
    }

    bool quit() override
    {
        qDebug() << "quit() called";

        return shouldQuit = true;
    }
};

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectHost srcNode(QUrl(QStringLiteral("tcp://127.0.0.1:65214")));
        MyTestServer myTestServer{};
        TestChildClassSimpleSource child;
        myTestServer.setChildProp(&child);

        srcNode.enableRemoting(&myTestServer);

        qDebug() << "Waiting for incoming connections";

        QTRY_VERIFY_WITH_TIMEOUT(myTestServer.shouldQuit, 20000); // wait up to 20s

        qDebug() << "Stopping server";
        QTest::qWait(200); // wait for server to send reply to client invoking quit() function
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
