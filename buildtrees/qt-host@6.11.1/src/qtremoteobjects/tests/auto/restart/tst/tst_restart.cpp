// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QMetaType>
#include <QProcess>

#include "../../../shared/testutils.h"

class tst_Restart: public QObject
{
    Q_OBJECT

public:
    enum RunMode { Baseline, ServerRestartGraceful, ServerRestartFatal };
    enum ObjectMode { NullPointer, ObjectPointer };
    Q_ENUM(RunMode)
    Q_ENUM(ObjectMode)

private slots:
    void initTestCase()
    {
        QVERIFY(TestUtils::init("tst"));
        QLoggingCategory::setFilterRules("qt.remoteobjects.warning=false");
    }

    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testRun_data()
    {
        QTest::addColumn<RunMode>("runMode");
        QTest::addColumn<ObjectMode>("objectMode");
        auto runModeMeta = QMetaEnum::fromType<RunMode>();
        auto objectModeMeta = QMetaEnum::fromType<ObjectMode>();
        for (int i = 0; i < runModeMeta.keyCount(); i++) {
            for (int j = 0; j < objectModeMeta.keyCount(); j++) {
                auto ba = QByteArray(runModeMeta.valueToKey(i));
                ba = ba.append("_").append(objectModeMeta.valueToKey(j));
                QTest::newRow(ba.data()) << static_cast<RunMode>(i) << static_cast<ObjectMode>(j);
            }
        }
    }

    void testRun()
    {
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        QFETCH(RunMode, runMode);
        QFETCH(ObjectMode, objectMode);

        qDebug() << "Starting server process" << runMode;
        bool serverRestart = runMode == ServerRestartFatal || runMode == ServerRestartGraceful;
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("RunMode", QVariant::fromValue(runMode).toString());
        env.insert("ObjectMode", QVariant::fromValue(objectMode).toString());
        serverProc.setProcessEnvironment(env);
        QStringList arguments;
        if (runMode == ServerRestartFatal)
            arguments.append("-nocrashhandler");
        serverProc.start(TestUtils::findExecutable("restart_server", "/server"), arguments);
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);

        qDebug() << "Starting client process";
        QProcess clientProc;
        clientProc.setProcessChannelMode(QProcess::ForwardedChannels);
        clientProc.setProcessEnvironment(env);
        clientProc.start(TestUtils::findExecutable("restart_client", "/client"),
                         QStringList());
        QVERIFY(clientProc.waitForStarted());

        if (serverRestart) {
            env.insert("RunMode", QVariant::fromValue(Baseline).toString()); // Don't include ServerRestart environment variable this time
            qDebug() << "Waiting for server exit";
            QVERIFY(serverProc.waitForFinished());
            if (runMode == ServerRestartFatal)
                QVERIFY(serverProc.exitCode() != 0);
            else
                QCOMPARE(serverProc.exitCode(), 0);
            qDebug() << "Restarting server";
            serverProc.setProcessEnvironment(env);
            serverProc.start(TestUtils::findExecutable("restart_server", "/server"),
                             QStringList());
            QVERIFY(serverProc.waitForStarted());
        }

        QVERIFY(clientProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Restart)

#include "tst_restart.moc"
