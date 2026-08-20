// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_server_replica.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

class tst_Match_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectNode repNode;
        repNode.connectToNode(QUrl(QStringLiteral("tcp://127.0.0.1:65214")));
        QSharedPointer<TestClassReplica> rep(repNode.acquire<TestClassReplica>());
        QSignalSpy stateChangedSpy(rep.data(), &QRemoteObjectReplica::stateChanged);
        QVERIFY(rep->waitForSource());
        QCOMPARE(rep->state(), QRemoteObjectReplica::Valid);

        QVERIFY(rep->quit().waitForFinished());
        QTRY_COMPARE(rep->state(), QRemoteObjectReplica::Suspect);

        QCOMPARE(stateChangedSpy.size(), 2);

        // Test Default to Valid transition
        auto args = stateChangedSpy.takeFirst();
        QCOMPARE(args.size(), 2);
        QCOMPARE(args.at(0).toInt(), int(QRemoteObjectReplica::Valid));
        QCOMPARE(args.at(1).toInt(), int(QRemoteObjectReplica::Default));

        // Test Valid to Suspect transition
        args = stateChangedSpy.takeFirst();
        QCOMPARE(args.size(), 2);
        QCOMPARE(args.at(0).toInt(), int(QRemoteObjectReplica::Suspect));
        QCOMPARE(args.at(1).toInt(), int(QRemoteObjectReplica::Valid));
    }
};

QTEST_MAIN(tst_Match_Process)

#include "main.moc"
