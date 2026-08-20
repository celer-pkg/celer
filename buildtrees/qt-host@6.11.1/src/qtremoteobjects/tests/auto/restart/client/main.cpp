// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_subclass_replica.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>

class tst_Client_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QLoggingCategory::setFilterRules("qt.remoteobjects.warning=true");
        m_repNode.reset(new QRemoteObjectNode);
        m_repNode->connectToNode(QUrl(QStringLiteral("tcp://127.0.0.1:65217")));
        m_rep.reset(m_repNode->acquire<ParentClassReplica>());
        connect(m_rep.data(), &QRemoteObjectReplica::stateChanged,
                this, [this](QRemoteObjectReplica::State state,
                             QRemoteObjectReplica::State previousState) {
            qDebug() << "**** stateChanged" << state << previousState;
            if (state == QRemoteObjectReplica::Suspect) {
                qWarning() << "Replica suspect";
                this->serverRestarted = true;
            } else if (state == QRemoteObjectReplica::Valid) {
                if (this->serverRestarted) {
                    qWarning() << "Replica valid again";
                    auto reply = m_rep->start();
                }
            }
        });
        QVERIFY(m_rep->waitForSource());
    }

    void testRun()
    {
        const auto objectMode = qEnvironmentVariable("ObjectMode");

        qWarning() << "From client";
        const MyPOD initialValue(42, 3.14f, QStringLiteral("SubClass"));
        if (objectMode == QLatin1String("ObjectPointer")) {
            QSignalSpy tracksSpy(m_rep->tracks(), &QAbstractItemModelReplica::initialized);
            QVERIFY(m_rep->subClass() != nullptr);
            QCOMPARE(m_rep->subClass()->myPOD(), initialValue);
            QVERIFY(m_rep->tracks() != nullptr);
            QVERIFY(tracksSpy.size() || tracksSpy.wait());
        } else {
            QVERIFY(m_rep->subClass() == nullptr);
            QVERIFY(m_rep->tracks() == nullptr);
        }
        auto reply = m_rep->start();
        QVERIFY(reply.waitForFinished());

        QSignalSpy advanceSpy(m_rep.data(), SIGNAL(advance()));
        QVERIFY(advanceSpy.wait(15000));
        if (objectMode == QLatin1String("ObjectPointer")) {
            QVERIFY(m_rep->subClass() != nullptr);
            QCOMPARE(m_rep->subClass()->myPOD(), initialValue);
            QVERIFY(m_rep->tracks() != nullptr);
        } else {
            QVERIFY(m_rep->subClass() == nullptr);
            QVERIFY(m_rep->tracks() == nullptr);
        }
    }

    void cleanupTestCase()
    {
        auto reply = m_rep->quit();
        QVERIFY(reply.waitForFinished());
    }

private:
    QScopedPointer<QRemoteObjectNode> m_repNode;
    QScopedPointer<ParentClassReplica> m_rep;
    bool serverRestarted = false;
};

QTEST_MAIN(tst_Client_Process)

#include "main.moc"
