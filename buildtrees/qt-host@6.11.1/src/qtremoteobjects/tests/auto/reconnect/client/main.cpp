// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QtRemoteObjects/qremoteobjectnode.h>

class tst_Client_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        const QString url = qEnvironmentVariable("RO_URL");
        QRemoteObjectNode node;
        node.connectToNode(QUrl(url));
        QRemoteObjectDynamicReplica *ro = node.acquireDynamic("SourceObj");

        QSignalSpy initSpy(ro, &QRemoteObjectDynamicReplica::initialized);
        QVERIFY(initSpy.wait());
        QSignalSpy pongSpy(ro, SIGNAL(pong()));
        QMetaObject::invokeMethod(ro, "ping");
        QVERIFY(pongSpy.wait());
        QMetaObject::invokeMethod(ro, "ping");

        QVERIFY(initSpy.wait());
        QMetaObject::invokeMethod(ro, "ping");
        QVERIFY(pongSpy.wait());
        QMetaObject::invokeMethod(ro, "ping");
        QTest::qWait(100);
        delete ro;
    }
};

QTEST_MAIN(tst_Client_Process)

#include "main.moc"
