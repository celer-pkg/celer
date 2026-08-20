// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_MyInterface_replica.h"

#include <QCoreApplication>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtTest/QtTest>
#include <QTcpSocket>

const QUrl registryUrl = QUrl(QStringLiteral("tcp://127.0.0.1:65212"));

class tst_Client_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_repNode.setRegistryUrl(registryUrl);
        QRemoteObjectNode::RemoteObjectSchemaHandler setupTcp = [this](QUrl url) {
            QTcpSocket *socket = new QTcpSocket(&this->m_repNode);
            connect(socket, &QTcpSocket::connected,
                    this, [socket, this]() {
                this->m_repNode.addClientSideConnection(socket);
            });
            connect(socket, &QTcpSocket::errorOccurred,
                    socket, [socket](QAbstractSocket::SocketError error) {
                qDebug() << "SocketError" << error;
                delete socket;
            });
            socket->connectToHost(url.host(), quint16(url.port()));
        };
        m_repNode.registerExternalSchema(QStringLiteral("exttcp"), setupTcp);
        QVERIFY(m_repNode.waitForRegistry(3000));
        m_rep.reset(m_repNode.acquire<MyInterfaceReplica>());
    }

    void testRun()
    {

        QVERIFY(m_rep->waitForSource());
        auto reply = m_rep->start();
        QVERIFY(reply.waitForFinished());

        // BEGIN: Testing
        QSignalSpy advanceSpy(m_rep.data(), &MyInterfaceReplica::advance);

        QSignalSpy spy(m_rep.data(), &MyInterfaceReplica::enum1Changed);
        QVERIFY(advanceSpy.wait());

        QCOMPARE(spy.size(), 2);
        // END: Testing

        reply = m_rep->stop();
        QVERIFY(reply.waitForFinished());
    }

    void testEnumDetails()
    {
        QHash<QByteArray, int> kvs = {{"First", 0}, {"Second", 1}, {"Third", 2}};
        QScopedPointer<QRemoteObjectDynamicReplica> rep(m_repNode.acquireDynamic("MyInterface"));
        QVERIFY(rep->waitForSource());

        auto mo = rep->metaObject();
        int enumIdx = mo->indexOfEnumerator("Enum1");
        QVERIFY(enumIdx != -1);
        auto enumerator = mo->enumerator(enumIdx);
        QCOMPARE(enumerator.name(), "Enum1");
        QCOMPARE(enumerator.keyCount(), 3);
        for (int i = 0; i < 3; ++i) {
            auto key = enumerator.key(i);
            auto val = enumerator.value(i);
            auto it = kvs.find(key);
            QVERIFY(it != kvs.end());
            QCOMPARE(*it, val);
            kvs.erase(it);
        }

        int propIdx = mo->indexOfProperty("enum1");
        QVERIFY(propIdx != -1);
        auto property = mo->property(propIdx);
        property.write(rep.data(), 1);
        QTRY_COMPARE(property.read(rep.data()).toInt(), 1);
    }

    void testMethodSignalParamDetails()
    {
        QScopedPointer<QRemoteObjectDynamicReplica> rep(m_repNode.acquireDynamic("MyInterface"));
        QVERIFY(rep->waitForSource());

        auto mo = rep->metaObject();
        int signalIdx = mo->indexOfSignal("testEnumParamsInSignals(MyInterfaceReplica::Enum1,bool,QString)");
        QVERIFY(signalIdx != -1);
        auto simm = mo->method(signalIdx);
        {
            QCOMPARE(simm.parameterCount(), 3);
            auto paramNames = simm.parameterNames();
            QCOMPARE(paramNames.size(), 3);
            QCOMPARE(paramNames.at(0), QByteArrayLiteral("enumSignalParam"));
            QCOMPARE(paramNames.at(1), QByteArrayLiteral("signalParam2"));
            QCOMPARE(paramNames.at(2), QByteArrayLiteral("__repc_variable_1"));
            QCOMPARE(simm.parameterMetaType(0), QMetaType::fromType<MyInterfaceReplica::Enum1>());
            QCOMPARE(simm.parameterMetaType(1), QMetaType::fromType<bool>());
            QCOMPARE(simm.parameterMetaType(2), QMetaType::fromType<QString>());
        }

        int slotIdx = mo->indexOfSlot("testEnumParamsInSlots(MyInterfaceReplica::Enum1,bool,int)");
        QVERIFY(slotIdx != -1);
        auto slmm = mo->method(slotIdx);
        {
            QCOMPARE(slmm .parameterCount(), 3);
            auto paramNames = slmm .parameterNames();
            QCOMPARE(paramNames.size(), 3);
            QCOMPARE(paramNames.at(0), QByteArrayLiteral("enumSlotParam"));
            QCOMPARE(paramNames.at(1), QByteArrayLiteral("slotParam2"));
            QCOMPARE(paramNames.at(2), QByteArrayLiteral("__repc_variable_1"));
        }

        MyInterfaceReplica::Enum1 enumVal = {};
        mo->invokeMethod(rep.data(), "testEnumParamsInSlots",
                                    Q_ARG(MyInterfaceReplica::Enum1, enumVal),
                                    Q_ARG(bool, true), Q_ARG(int, 1234));

        int enumIdx = mo->indexOfProperty("enum1");
        QVERIFY(enumIdx != -1);
        QTRY_COMPARE(mo->property(enumIdx).read(rep.data()).toInt(), 0);

        int startedIdx = mo->indexOfProperty("started");
        QVERIFY(startedIdx != -1);
        QTRY_COMPARE(mo->property(startedIdx).read(rep.data()).toBool(), true);
    }

    void testMethodSignal()
    {
        QScopedPointer<MyInterfaceReplica> rep(new MyInterfaceReplica());
        rep->setNode(&m_repNode);
        QVERIFY(rep->waitForSource());

        rep->testEnumParamsInSlots(MyInterfaceReplica::Second, false, 74);

        connect(rep.data(), &MyInterfaceReplica::testEnumParamsInSignals, this,
                [](MyInterfaceReplica::Enum1 enumSignalParam) { QCOMPARE(enumSignalParam, MyInterfaceReplica::Second); });

        QTRY_COMPARE(rep->enum1(), MyInterfaceReplica::Second);
        QTRY_COMPARE(rep->started(), false);
    }

    void testDisconnect()
    {
        auto reply = m_rep->next();
        QSignalSpy stateSpy(m_rep.data(), &MyInterfaceReplica::stateChanged);
        QVERIFY(reply.waitForFinished());

        QTRY_COMPARE(stateSpy.size(), 1);
        QCOMPARE(m_rep->state(), QRemoteObjectReplica::Suspect);

        QTRY_COMPARE(stateSpy.size(), 2);
        QCOMPARE(m_rep->state(), QRemoteObjectReplica::Valid);
        // Make sure we updated to the correct enum1 value
        QCOMPARE(m_rep->enum1(), MyInterfaceReplica::First);
    }

    void cleanupTestCase()
    {
        auto reply = m_rep->quit();
        QVERIFY(reply.waitForFinished());
    }

private:
    QRemoteObjectNode m_repNode;
    QScopedPointer<MyInterfaceReplica> m_rep;
};

QTEST_MAIN(tst_Client_Process)

#include "main.moc"
