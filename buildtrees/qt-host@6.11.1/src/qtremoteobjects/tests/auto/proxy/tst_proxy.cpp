// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "rep_engine_merged.h"
#include "rep_subclass_merged.h"
#include "../shared/model_utilities.h"

#include <QtTest/QtTest>
#include <QRemoteObjectReplica>
#include <QRemoteObjectNode>

#include "../../shared/testutils.h"

const QUrl localHostUrl = QUrl(QLatin1String(LOCAL_SOCKET ":testHost"));
const QUrl tcpHostUrl = QUrl(QLatin1String("tcp://127.0.0.1:9989"));
const QUrl proxyNodeUrl = QUrl(QLatin1String("tcp://127.0.0.1:12123"));
const QUrl remoteNodeUrl = QUrl(QLatin1String("tcp://127.0.0.1:23234"));
const QUrl registryUrl = QUrl(QLatin1String(LOCAL_SOCKET ":testRegistry"));
const QUrl proxyHostUrl = QUrl(QLatin1String(LOCAL_SOCKET ":fromProxy"));

#define SET_NODE_NAME(obj) (obj).setName(QLatin1String(#obj))

class ProxyTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testProxy_data();
    void testProxy();
    void testForwardProxy();
    void testReverseProxy();
    // The following should fail to compile, verifying the SourceAPI templates work
    // for subclasses
    /*
    void testSubclass()
    {
        QRemoteObjectHost host(localHostUrl);

        struct invalidchild {
            MyPOD myPOD() { return MyPOD(12, 13.f, QStringLiteral("Yay!")); }
        };
        struct badparent {
            invalidchild *subClass() { return new invalidchild; }
        } parent;
        host.enableRemoting<ParentClassSourceAPI>(&parent);
    }
    */

    void testTopLevelModel();
};

void ProxyTest::testProxy_data()
{
    QTest::addColumn<bool>("sourceApi");
    QTest::addColumn<bool>("useProxy");
    QTest::addColumn<bool>("dynamic");

    QTest::newRow("dynamicApi, no proxy") << false << false << false;
    QTest::newRow("sourceApi, no proxy") << true << false << false;
    QTest::newRow("dynamicApi, with proxy") << false << true << false;
    QTest::newRow("sourceApi, with proxy") << true << true << false;
    QTest::newRow("dynamicApi, no proxy, dynamicRep") << false << false << true;
    QTest::newRow("sourceApi, no proxy, dynamicRep") << true << false << true;
    QTest::newRow("dynamicApi, with proxy, dynamicRep") << false << true << true;
    QTest::newRow("sourceApi, with proxy, dynamicRep") << true << true << true;
}

void ProxyTest::testProxy()
{
    QFETCH(bool, sourceApi);
    QFETCH(bool, useProxy);
    QFETCH(bool, dynamic);

    //Setup Local Registry
    QRemoteObjectRegistryHost registry(registryUrl);
    SET_NODE_NAME(registry);
    //Setup Local Host
    QRemoteObjectHost host(localHostUrl);
    SET_NODE_NAME(host);
    host.setRegistryUrl(registryUrl);
    EngineSimpleSource engine;
    engine.setRpm(1234);
    engine.setType(EngineSimpleSource::Gas);
    if (sourceApi)
        host.enableRemoting<EngineSourceAPI>(&engine);
    else
        host.enableRemoting(&engine);

    QRemoteObjectHost proxyNode;
    SET_NODE_NAME(proxyNode);
    if (useProxy) {
        proxyNode.setHostUrl(tcpHostUrl);
        proxyNode.proxy(registryUrl);
    }

    //Setup Local Replica
    QRemoteObjectNode client;
    SET_NODE_NAME(client);
    if (useProxy)
        client.connectToNode(tcpHostUrl);
    else
        client.setRegistryUrl(registryUrl);

    QScopedPointer<QRemoteObjectReplica> replica;
    if (!dynamic) {
        //QLoggingCategory::setFilterRules("qt.remoteobjects*=true");
        replica.reset(client.acquire<EngineReplica>());
        QVERIFY(replica->waitForSource(1000));

        EngineReplica *rep = qobject_cast<EngineReplica *>(replica.data());

        //Compare Replica to Source
        QCOMPARE(rep->rpm(), engine.rpm());
        QCOMPARE(EngineReplica::EngineType(rep->type()), EngineReplica::Gas);

        //Change Replica and make sure change propagates to source
        QSignalSpy sourceSpy(&engine, &EngineSimpleSource::rpmChanged);
        QSignalSpy replicaSpy(rep, &EngineReplica::rpmChanged);
        rep->pushRpm(42);
        sourceSpy.wait();
        QCOMPARE(sourceSpy.size(), 1);
        QCOMPARE(engine.rpm(), 42);

        // ... and the change makes it back to the replica
        replicaSpy.wait();
        QCOMPARE(replicaSpy.size(), 1);
        QCOMPARE(rep->rpm(), 42);
    } else {
        replica.reset(client.acquireDynamic(QStringLiteral("Engine")));
        QVERIFY(replica->waitForSource(1000));

        //Compare Replica to Source
        const QMetaObject *metaObject = replica->metaObject();
        const int rpmIndex = metaObject->indexOfProperty("rpm");
        Q_ASSERT(rpmIndex != -1);
        const QMetaProperty rpmMeta =  metaObject->property(rpmIndex);
        QCOMPARE(rpmMeta.read(replica.data()).value<int>(), engine.rpm());
        const int typeIndex = metaObject->indexOfProperty("type");
        Q_ASSERT(typeIndex != -1);
        const QMetaProperty typeMeta =  metaObject->property(typeIndex);
        QCOMPARE(typeMeta.read(replica.data()).value<EngineReplica::EngineType>(), EngineReplica::Gas);

        //Change Replica and make sure change propagates to source
        QSignalSpy sourceSpy(&engine, &EngineSimpleSource::rpmChanged);
        QSignalSpy replicaSpy(replica.data(), QByteArray(QByteArrayLiteral("2")+rpmMeta.notifySignal().methodSignature().constData()));

        const int rpmPushIndex = metaObject->indexOfMethod("pushRpm(int)");
        Q_ASSERT(rpmPushIndex != -1);
        QMetaMethod pushMethod = metaObject->method(rpmPushIndex);
        Q_ASSERT(pushMethod.isValid());
        QVERIFY(pushMethod.invoke(replica.data(), Q_ARG(int, 42)));

        sourceSpy.wait();
        QCOMPARE(sourceSpy.size(), 1);
        QCOMPARE(engine.rpm(), 42);

        // ... and the change makes it back to the replica
        replicaSpy.wait();
        QCOMPARE(replicaSpy.size(), 1);
        QCOMPARE(rpmMeta.read(replica.data()).value<int>(), engine.rpm());
    }

    // Make sure disabling the Source cascades the state change
    bool res = host.disableRemoting(&engine);
    Q_ASSERT(res);
    QSignalSpy stateSpy(replica.data(), &QRemoteObjectReplica::stateChanged);
    stateSpy.wait();
    QCOMPARE(stateSpy.size(), 1);
    QCOMPARE(replica->state(), QRemoteObjectReplica::Suspect);

    // Now test subclass Source
    ParentClassSimpleSource parent;
    SubClassSimpleSource subclass;
    const MyPOD initialValue(42, 3.14f, QStringLiteral("SubClass"));
    subclass.setMyPOD(initialValue);
    QStringListModel model;
    model.setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    parent.setSubClass(&subclass);
    parent.setTracks(&model);
    QCOMPARE(subclass.myPOD(), initialValue);
    if (sourceApi)
        host.enableRemoting<ParentClassSourceAPI>(&parent);
    else
        host.enableRemoting(&parent);
    if (!dynamic) {
        replica.reset(client.acquire<ParentClassReplica>());
        ParentClassReplica *rep = qobject_cast<ParentClassReplica *>(replica.data());
        QSignalSpy tracksSpy(rep->tracks(), &QAbstractItemModelReplica::initialized);
        QVERIFY(replica->waitForSource(1000));
        //QLoggingCategory::setFilterRules("qt.remoteobjects*=false");

        //Compare Replica to Source
        QVERIFY(rep->subClass() != nullptr);
        QCOMPARE(rep->subClass()->myPOD(), parent.subClass()->myPOD());
        QVERIFY(rep->tracks() != nullptr);
        QVERIFY(tracksSpy.size() || tracksSpy.wait());
        // Rep file only uses display role, but proxy doesn't forward that yet
        if (!useProxy)
            QCOMPARE(rep->tracks()->availableRoles(), QList<int> { Qt::DisplayRole });
        else {
            const auto &availableRolesVec = rep->tracks()->availableRoles();
            QSet<int> availableRoles;
            for (int r : availableRolesVec)
                availableRoles.insert(r);
            const auto &rolesHash = model.roleNames();
            QSet<int> roles;
            for (auto it = rolesHash.cbegin(), end = rolesHash.cend(); it != end; ++it)
                roles.insert(it.key());
            QCOMPARE(availableRoles, roles);
        }
        QList<QModelIndex> pending;
        QTRY_COMPARE(rep->tracks()->rowCount(), model.rowCount());
        for (int i = 0; i < rep->tracks()->rowCount(); i++)
        {
            // We haven't received any data yet
            const auto index = rep->tracks()->index(i, 0);
            QCOMPARE(rep->tracks()->data(index), QVariant());
            pending.append(index);
        }
        if (useProxy) { // A first batch of updates will be the empty proxy values
            WaitForDataChanged w(rep->tracks(), pending);
            QVERIFY(w.wait());
        }
        WaitForDataChanged w(rep->tracks(), pending);
        QVERIFY(w.wait());
        for (int i = 0; i < rep->tracks()->rowCount(); i++)
        {
            QTRY_COMPARE(rep->tracks()->data(rep->tracks()->index(i, 0)), model.data(model.index(i), Qt::DisplayRole));
        }

        //Change SubClass and make sure change propagates
        SubClassSimpleSource updatedSubclass;
        const MyPOD updatedValue(-1, 123.456f, QStringLiteral("Updated"));
        updatedSubclass.setMyPOD(updatedValue);
        QSignalSpy replicaSpy(rep, &ParentClassReplica::subClassChanged);
        parent.setSubClass(&updatedSubclass);
        replicaSpy.wait();
        QCOMPARE(replicaSpy.size(), 1);
        QCOMPARE(rep->subClass()->myPOD(), parent.subClass()->myPOD());
        QCOMPARE(rep->subClass()->myPOD(), updatedValue);
    } else {
        replica.reset(client.acquireDynamic(QStringLiteral("ParentClass")));
        QVERIFY(replica->waitForSource(1000));

        const QMetaObject *metaObject = replica->metaObject();
        // Verify subClass pointer
        const int subclassIndex = metaObject->indexOfProperty("subClass");
        QVERIFY(subclassIndex != -1);
        const QMetaProperty subclassMeta =  metaObject->property(subclassIndex);
        QObject *subclassQObjectPtr = subclassMeta.read(replica.data()).value<QObject *>();
        QVERIFY(subclassQObjectPtr != nullptr);
        QRemoteObjectDynamicReplica *subclassReplica = qobject_cast<QRemoteObjectDynamicReplica *>(subclassQObjectPtr);
        QVERIFY(subclassReplica != nullptr);
        // Verify tracks pointer
        const int tracksIndex = metaObject->indexOfProperty("tracks");
        QVERIFY(tracksIndex != -1);
        const QMetaProperty tracksMeta =  metaObject->property(tracksIndex);
        QObject *tracksQObjectPtr = tracksMeta.read(replica.data()).value<QObject *>();
        QVERIFY(tracksQObjectPtr != nullptr);
        QAbstractItemModelReplica *tracksReplica = qobject_cast<QAbstractItemModelReplica *>(tracksQObjectPtr);
        QVERIFY(tracksReplica != nullptr);

        // Verify subClass data
        const int podIndex = subclassReplica->metaObject()->indexOfProperty("myPOD");
        QVERIFY(podIndex != -1);
        const QMetaProperty podMeta = subclassReplica->metaObject()->property(podIndex);
        MyPOD pod = podMeta.read(subclassReplica).value<MyPOD>();
        QCOMPARE(pod, parent.subClass()->myPOD());

        // Verify tracks data
        // Rep file only uses display role, but proxy doesn't forward that yet
        if (!useProxy)
            QCOMPARE(tracksReplica->availableRoles(), QList<int> { Qt::DisplayRole });
        else {
            const auto &availableRolesVec = tracksReplica->availableRoles();
            QSet<int> availableRoles;
            for (int r : availableRolesVec)
                availableRoles.insert(r);
            const auto &rolesHash = model.roleNames();
            QSet<int> roles;
            for (auto it = rolesHash.cbegin(), end = rolesHash.cend(); it != end; ++it)
                roles.insert(it.key());
            QCOMPARE(availableRoles, roles);
        }
        QTRY_COMPARE(tracksReplica->isInitialized(), true);
        QSignalSpy dataSpy(tracksReplica, &QAbstractItemModelReplica::dataChanged);
        QList<QModelIndex> pending;
        QTRY_COMPARE(tracksReplica->rowCount(), model.rowCount());
        for (int i = 0; i < tracksReplica->rowCount(); i++)
        {
            // We haven't received any data yet
            const auto index = tracksReplica->index(i, 0);
            QCOMPARE(tracksReplica->data(index), QVariant());
            pending.append(index);
        }
        if (useProxy) { // A first batch of updates will be the empty proxy values
            WaitForDataChanged w(tracksReplica, pending);
            QVERIFY(w.wait());
        }
        WaitForDataChanged w(tracksReplica, pending);
        QVERIFY(w.wait());
        for (int i = 0; i < tracksReplica->rowCount(); i++)
        {
            QCOMPARE(tracksReplica->data(tracksReplica->index(i, 0)), model.data(model.index(i), Qt::DisplayRole));
        }

        //Change SubClass and make sure change propagates
        SubClassSimpleSource updatedSubclass;
        const MyPOD updatedValue(-1, 123.456f, QStringLiteral("Updated"));
        updatedSubclass.setMyPOD(updatedValue);
        QSignalSpy replicaSpy(replica.data(), QByteArray(QByteArrayLiteral("2")+subclassMeta.notifySignal().methodSignature().constData()));
        parent.setSubClass(&updatedSubclass);
        replicaSpy.wait();
        QCOMPARE(replicaSpy.size(), 1);
        subclassQObjectPtr = subclassMeta.read(replica.data()).value<QObject *>();
        QVERIFY(subclassQObjectPtr != nullptr);
        subclassReplica = qobject_cast<QRemoteObjectDynamicReplica *>(subclassQObjectPtr);
        QVERIFY(subclassReplica != nullptr);

        pod = podMeta.read(subclassReplica).value<MyPOD>();
        QCOMPARE(pod, parent.subClass()->myPOD());
    }
    replica.reset();
}

void ProxyTest::testForwardProxy()
{
    // Setup Local Registry
    QRemoteObjectRegistryHost registry(registryUrl);
    SET_NODE_NAME(registry);

    // Setup Local Host
    QRemoteObjectHost host(localHostUrl, registryUrl);
    SET_NODE_NAME(host);

    // Setup Proxy
    QRemoteObjectRegistryHost proxyNode(proxyNodeUrl);
    SET_NODE_NAME(proxyNode);
    proxyNode.proxy(registryUrl, proxyHostUrl);
    // Include the reverseProxy to make sure we don't try to send back
    // proxied objects.
    proxyNode.reverseProxy();

    // Setup Source
    EngineSimpleSource engine;
    engine.setRpm(1234);

    // Setup Remote Node
    QRemoteObjectHost remoteNode(remoteNodeUrl);
    SET_NODE_NAME(remoteNode);
    remoteNode.connectToNode(proxyNodeUrl);

    // Add source
    host.enableRemoting(&engine);

    // Setup Replica
    const QScopedPointer<EngineReplica> replica(remoteNode.acquire<EngineReplica>());
    QTRY_VERIFY(replica->waitForSource(300));

    // Compare Replica to Source
    QCOMPARE(replica->rpm(), engine.rpm());
}

void ProxyTest::testReverseProxy()
{
    // Setup Local Registry
    QRemoteObjectRegistryHost registry(registryUrl);
    SET_NODE_NAME(registry);

    // Setup Local Host
    QRemoteObjectHost host(localHostUrl, registryUrl);
    SET_NODE_NAME(host);

    // Setup Proxy
    QRemoteObjectRegistryHost proxyNode(proxyNodeUrl);
    SET_NODE_NAME(proxyNode);
    proxyNode.proxy(registryUrl, proxyHostUrl);
    proxyNode.reverseProxy();

    // Setup Source
    EngineSimpleSource engine;
    engine.setRpm(1234);

    // Setup Remote Node
    QRemoteObjectHost remoteNode(remoteNodeUrl, proxyNodeUrl);
    SET_NODE_NAME(remoteNode);

    // Add source
    remoteNode.enableRemoting(&engine);

    // Setup Replica
    const QScopedPointer<EngineReplica> replica(host.acquire<EngineReplica>());
    QTRY_VERIFY(replica->waitForSource(300));

    //Compare Replica to Source
    QCOMPARE(replica->rpm(), engine.rpm());
}

void ProxyTest::testTopLevelModel()
{
    QRemoteObjectRegistryHost registry(registryUrl);

    //Setup Local Host
    QRemoteObjectHost host(localHostUrl);
    SET_NODE_NAME(host);
    host.setRegistryUrl(registryUrl);

    QStringListModel model;
    model.setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    host.enableRemoting(&model, "trackList", QList<int> { Qt::DisplayRole });

    QRemoteObjectHost proxyNode;
    SET_NODE_NAME(proxyNode);
    proxyNode.setHostUrl(tcpHostUrl);
    proxyNode.proxy(registryUrl);

    //Setup Local Replica
    QRemoteObjectNode client;
    SET_NODE_NAME(client);
    client.connectToNode(tcpHostUrl);
    QAbstractItemModelReplica *replica = client.acquireModel("trackList");
    QSignalSpy tracksSpy(replica, &QAbstractItemModelReplica::initialized);
    QVERIFY(tracksSpy.wait());
    QTRY_COMPARE(replica->rowCount(), model.rowCount());
}

QTEST_MAIN(ProxyTest)

#include "tst_proxy.moc"
