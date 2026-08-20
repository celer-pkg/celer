// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_subclass_replica.h"
#include "../shared.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

static QMap<int, MyPOD> int_map{{1, initialValue},
                                {16, initialValue}};
static ParentClassReplica::ActivePositions flags1 = ParentClassReplica::Position::position1;
static ParentClassReplica::ActivePositions flags2 = ParentClassReplica::Position::position2
                                                    | ParentClassReplica::Position::position3;
static QMap<ParentClassReplica::ActivePositions, MyPOD> my_map{{flags1, initialValue},
                                                               {flags2, initialValue}};
static QHash<NS2::NamespaceEnum, MyPOD> my_hash{{NS2::NamespaceEnum::Alpha, initialValue},
                                                {NS2::NamespaceEnum::Charlie, initialValue}};

class tst_Client_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_repNode.reset(new QRemoteObjectNode);
        m_repNode->connectToNode(QUrl(QStringLiteral("tcp://127.0.0.1:65213")));
        m_rep.reset(m_repNode->acquire<ParentClassReplica>());
        const auto objectMode = qEnvironmentVariable("ObjectMode");
        qDebug() << "Waiting to connect, mode =" << objectMode;
        QVERIFY(m_rep->waitForSource());
    }

    void testSubClass()
    {
        const auto objectMode = qEnvironmentVariable("ObjectMode");

        qDebug() << "Starting test" << objectMode;
        if (objectMode == QLatin1String("ObjectPointer")) {
            QSignalSpy tracksSpy(m_rep->tracks(), &QAbstractItemModelReplica::initialized);
            QVERIFY(m_rep->subClass() != nullptr);
            QCOMPARE(m_rep->subClass()->myPOD(), initialValue);
            QCOMPARE(m_rep->subClass()->i(), initialI);
            QVERIFY(m_rep->tracks() != nullptr);
            QVERIFY(tracksSpy.size() || tracksSpy.wait());
            QCOMPARE(m_rep->myEnum(), ParentClassReplica::bar);
            QCOMPARE(m_rep->date(), Qt::RFC2822Date);
            QCOMPARE(m_rep->nsEnum(), NS::Bravo);
            QCOMPARE(m_rep->ns2Enum(), NS2::NamespaceEnum::Bravo);
            QCOMPARE(m_rep->variant(), QVariant::fromValue(42.0f));
            QCOMPARE(m_rep->simpleList(), QList<QString>() << "one" << "two");
            QCOMPARE(m_rep->podList(), QList<MyPOD>() << initialValue << initialValue);
            QCOMPARE(m_rep->intMap(), int_map);
            QCOMPARE(m_rep->enumMap(), my_map);
            QCOMPARE(m_rep->podHash(), my_hash);
        } else {
            QVERIFY(m_rep->subClass() == nullptr);
            QVERIFY(m_rep->tracks() == nullptr);
            QCOMPARE(m_rep->myEnum(), ParentClassReplica::foo);
            QCOMPARE(m_rep->date(), Qt::ISODate);
            QCOMPARE(m_rep->nsEnum(), NS::Alpha);
            QCOMPARE(m_rep->ns2Enum(), NS2::NamespaceEnum::Alpha);
            QCOMPARE(m_rep->variant(), QVariant());
        }

        QPoint p(1, 2);
        auto enumReply = m_rep->enumSlot(p, ParentClassReplica::bar);
        QVERIFY(enumReply.waitForFinished());
        QCOMPARE(enumReply.error(), QRemoteObjectPendingCall::NoError);
        QCOMPARE(enumReply.returnValue(), ParentClassReplica::foobar);

        qDebug() << "Verified expected initial states, sending start.";
        QSignalSpy enumSpy(m_rep.data(), &ParentClassReplica::enum2);
        QSignalSpy advanceSpy(m_rep.data(), &ParentClassReplica::advance);
        auto reply = m_rep->start();
        QVERIFY(reply.waitForFinished());
        QVERIFY(reply.error() == QRemoteObjectPendingCall::NoError);
        QCOMPARE(reply.returnValue(), QVariant::fromValue(true));
        QVERIFY(enumSpy.wait());
        QCOMPARE(enumSpy.size(), 1);
        const auto arguments = enumSpy.takeFirst();
        QCOMPARE(arguments.at(0), QVariant::fromValue(ParentClassReplica::foo));
        QCOMPARE(arguments.at(1), QVariant::fromValue(ParentClassReplica::bar));

        QVERIFY(advanceSpy.size() || advanceSpy.wait());
        QVERIFY(m_rep->subClass() != nullptr);
        QCOMPARE(m_rep->subClass()->myPOD(), updatedValue);
        QCOMPARE(m_rep->subClass()->i(), updatedI);
        QVERIFY(m_rep->tracks() != nullptr);
        QCOMPARE(m_rep->myEnum(), ParentClassReplica::foobar);
        QCOMPARE(m_rep->date(), Qt::ISODateWithMs);
        QCOMPARE(m_rep->nsEnum(), NS::Charlie);
        QCOMPARE(m_rep->ns2Enum(), NS2::NamespaceEnum::Charlie);
        QCOMPARE(m_rep->variant(), QVariant::fromValue(podValue));
        qDebug() << "Verified expected final states, cleaning up.";
    }

    void cleanupTestCase()
    {
        auto reply = m_rep->quit();
        // Don't verify the wait result, depending on the timing of the server and proxy
        // closing it may return false.  We just need this process to stay alive long
        // enough for the packets to be sent.
        reply.waitForFinished(5000);
    }

private:
    QScopedPointer<QRemoteObjectNode> m_repNode;
    QScopedPointer<ParentClassReplica> m_rep;
};

QTEST_MAIN(tst_Client_Process)

#include "main.moc"
