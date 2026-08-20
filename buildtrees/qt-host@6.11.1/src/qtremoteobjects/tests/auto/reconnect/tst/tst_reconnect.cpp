// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QProcess>
#include "../../../shared/testutils.h"


class tst_Reconnect: public QObject
{
    Q_OBJECT

private slots:
    void testRun_data()
    {
        QTest::addColumn<QString>("url");
        QTest::addRow("local") << QStringLiteral(LOCAL_SOCKET ":replica");
        QTest::addRow("tcp") << QStringLiteral("tcp://127.0.0.1:65217");
    }

    void testRun()
    {
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        QFETCH(QString, url);

        QVERIFY(TestUtils::init("tst"));

        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("RO_URL", url);
        serverProc.setProcessEnvironment(env);
        serverProc.start(TestUtils::findExecutable("qtro_reconnect_server", "/server"),
                         QStringList());
        QVERIFY(serverProc.waitForStarted());

        QProcess clientProc;
        clientProc.setProcessChannelMode(QProcess::ForwardedChannels);
        clientProc.setProcessEnvironment(env);
        clientProc.start(TestUtils::findExecutable("qtro_reconnect_client", "/client"),
                         QStringList());
        qDebug() << "Started server and client process on:" << url;
        QVERIFY(clientProc.waitForStarted());

        QVERIFY(clientProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Reconnect)

#include "tst_reconnect.moc"
