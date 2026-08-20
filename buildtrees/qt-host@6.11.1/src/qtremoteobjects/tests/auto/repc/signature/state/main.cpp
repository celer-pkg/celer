// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_mismatch_replica.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

class tst_State_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMismatch()
    {
        QRemoteObjectNode repNode;
        repNode.connectToNode(QUrl{QStringLiteral("tcp://127.0.0.1:65214")});
        QSharedPointer<TestClassReplica> rep{repNode.acquire<TestClassReplica>()};
        QSignalSpy stateChangedSpy(rep.data(), &QRemoteObjectReplica::stateChanged);
        QTRY_COMPARE(rep->state(), QRemoteObjectReplica::SignatureMismatch);
        QCOMPARE(stateChangedSpy.size(), 1);
        auto args = stateChangedSpy.takeFirst();
        QCOMPARE(args.size(), 2);
        QCOMPARE(args.at(0).toInt(), int(QRemoteObjectReplica::SignatureMismatch));
        QCOMPARE(args.at(1).toInt(), int(QRemoteObjectReplica::Default));
    }

    void testDynamic()
    {
        QRemoteObjectNode repNode;
        repNode.connectToNode(QUrl{QStringLiteral("tcp://127.0.0.1:65214")});
        QSharedPointer<QRemoteObjectDynamicReplica> rep{repNode.acquireDynamic("TestClass")};
        QSignalSpy stateChangedSpy(rep.data(), &QRemoteObjectReplica::stateChanged);
        QTRY_COMPARE(rep->state(), QRemoteObjectReplica::Valid);
        QCOMPARE(stateChangedSpy.size(), 1);
        auto args = stateChangedSpy.takeFirst();
        QCOMPARE(args.size(), 2);
        QCOMPARE(args.at(0).toInt(), int(QRemoteObjectReplica::Valid));
        QCOMPARE(args.at(1).toInt(), int(QRemoteObjectReplica::Uninitialized));
    }
};

QTEST_MAIN(tst_State_Process)

#include "main.moc"
