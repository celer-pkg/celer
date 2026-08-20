// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QMetaType>
#include <QProcess>

#include "../../../shared/testutils.h"

class tst_Proxy_MultiProcess: public QObject
{
    Q_OBJECT

public:
    enum ObjectMode { NullPointer, ObjectPointer };
    Q_ENUM(ObjectMode)

private slots:
    void initTestCase()
    {
        QVERIFY(TestUtils::init("tst"));
    }

    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testRun_data()
    {
        QTest::addColumn<bool>("templated");
        QTest::addColumn<ObjectMode>("objectMode");
        QTest::newRow("non-templated, subobject") << false << ObjectPointer;
        QTest::newRow("templated, subobject") << true << ObjectPointer;
        QTest::newRow("non-templated, nullptr") << false << NullPointer;
        QTest::newRow("templated, nullptr") << true << NullPointer;
    }

    void testRun()
    {
#ifdef Q_OS_ANDROID
        QSKIP("QProcess doesn't support running user bundled binaries on Android");
#endif
        QFETCH(bool, templated);
        QFETCH(ObjectMode, objectMode);

        qDebug() << "Starting server process";
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("ObjectMode", QVariant::fromValue(objectMode).toString());
        if (templated) {
            env.insert("TEMPLATED_REMOTING", "true");
        }
        serverProc.setProcessEnvironment(env);
        serverProc.start(TestUtils::findExecutable("proxy_multiprocess_server", "/server"),
                         QStringList());
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);


        qDebug() << "Starting client process";
        QProcess clientProc;
        clientProc.setProcessChannelMode(QProcess::ForwardedChannels);
        clientProc.setProcessEnvironment(env);
        clientProc.start(TestUtils::findExecutable("proxy_multiprocess_client", "/client"),
                         QStringList());
        QVERIFY(clientProc.waitForStarted());

        // wait for client start
        QTest::qWait(200);


        qDebug() << "Starting proxy process";
        QProcess proxyProc;
        proxyProc.setProcessChannelMode(QProcess::ForwardedChannels);
        proxyProc.start(TestUtils::findExecutable("proxy", "/proxy"),
                        QStringList());
        QVERIFY(proxyProc.waitForStarted());

        // wait for proxy start
        QTest::qWait(200);


        QVERIFY(clientProc.waitForFinished());
        QVERIFY(proxyProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(proxyProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Proxy_MultiProcess)

#include "tst_proxy_multiprocess.moc"
