// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QMetaType>
#include <QProcess>

#include "../../../shared/testutils.h"

class tst_Integration_MultiProcess: public QObject
{
    Q_OBJECT

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
        QTest::addColumn<bool>("templated");
        QTest::newRow("non-templated enableRemoting") << false;
        QTest::newRow("templated enableRemoting") << true;
    }

    void testRun()
    {
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        QFETCH(bool, templated);

        qDebug() << "Starting server process";
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        if (templated) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert("TEMPLATED_REMOTING", "true");
            serverProc.setProcessEnvironment(env);
        }
        serverProc.start(TestUtils::findExecutable("integration_multiprocess_server", "/server"),
                         QStringList());
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);

        qDebug() << "Starting client process";
        QProcess clientProc;
        clientProc.setProcessChannelMode(QProcess::ForwardedChannels);
        clientProc.start(TestUtils::findExecutable("integration_multiprocess_client", "/client"),
                         QStringList());
        QVERIFY(clientProc.waitForStarted());

        QVERIFY(clientProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Integration_MultiProcess)

#include "tst_integration_multiprocess.moc"
