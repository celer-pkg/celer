// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_mismatch_replica.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

class tst_Mismatch_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectNode repNode;
        repNode.connectToNode(QUrl(QStringLiteral("tcp://127.0.0.1:65214")));
        QTest::ignoreMessage(QtWarningMsg, " Signature mismatch for TestClassReplica \"TestClass\"");
        QSharedPointer<TestClassReplica> rep(repNode.acquire<TestClassReplica>());
        QTRY_COMPARE(rep->state(), QRemoteObjectReplica::SignatureMismatch);
    }
};

QTEST_MAIN(tst_Mismatch_Process)

#include "mismatch.moc"
