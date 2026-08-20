// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QString>
#include <QtTest>
#include "rep_class_merged.h"

#include "../../shared/testutils.h"

class SubClassReplicaTest : public QObject
{
    Q_OBJECT

public:
    SubClassReplicaTest();

private Q_SLOTS:
    void basicFunctions();
    void basicFunctions_data();
};

SubClassReplicaTest::SubClassReplicaTest()
{
}

void SubClassReplicaTest::basicFunctions_data()
{
    QTest::addColumn<bool>("templated");
    QTest::addColumn<bool>("nullobject");
    QTest::newRow("non-templated pointer") << false << false;
    QTest::newRow("templated pointer") << true << false;
    QTest::newRow("non-templated nullptr") << false << true;
    QTest::newRow("templated nullptr") << true << true;
}

void SubClassReplicaTest::basicFunctions()
{
    QFETCH(bool, templated);
    QFETCH(bool, nullobject);

    QRemoteObjectRegistryHost host(QUrl(LOCAL_SOCKET ":test"));
    SubClassSimpleSource subclass1, subclass2;
    ParentClassSimpleSource parent;
    parent.setSub1(&subclass1);

    SubClassSimpleSource subclass3;
    subclass3.setValue(4);
    OtherParentClassSimpleSource otherParent;
    otherParent.setSub1(&subclass3);
    host.enableRemoting(&otherParent, "OtherParent1");

    SubClassSimpleSource subclass4;
    subclass4.setValue(15);
    OtherParentClassSimpleSource otherParent2;
    otherParent2.setSub1(&subclass4);
    host.enableRemoting(&otherParent2, "OtherParent2");

    if (nullobject)
        parent.setSub2(nullptr);
    else
        parent.setSub2(&subclass2);
    if (templated)
        host.enableRemoting<ParentClassSourceAPI>(&parent);
    else
        host.enableRemoting(&parent);

    QRemoteObjectNode client(QUrl(LOCAL_SOCKET ":test"));
    const QScopedPointer<ParentClassReplica> replica(client.acquire<ParentClassReplica>());
    QVERIFY(replica->waitForSource(1000));

    auto sub1 = replica->sub1();
    QSignalSpy spy(sub1, &SubClassReplica::valueChanged);
    subclass1.setValue(10);
    QVERIFY(spy.wait());
    QCOMPARE(subclass1.value(), sub1->value());
    if (nullobject) {
        QCOMPARE(replica->sub2(), nullptr);
        QCOMPARE(parent.sub2(), nullptr);
    } else
        QCOMPARE(subclass2.value(), replica->sub2()->value());

    const QScopedPointer<OtherParentClassReplica> otherReplica(client.acquire<OtherParentClassReplica>("OtherParent1"));
    QVERIFY(otherReplica->waitForSource(1000));

    const QScopedPointer<OtherParentClassReplica> otherReplica2(client.acquire<OtherParentClassReplica>("OtherParent2"));
    QVERIFY(otherReplica2->waitForSource(1000));

    QCOMPARE(otherReplica->sub1()->value(), 4);
    QCOMPARE(otherReplica2->sub1()->value(), 15);
    otherReplica->sub1()->pushValue(19);
    otherReplica2->sub1()->pushValue(24);
    QTRY_COMPARE(subclass4.value(), 24);
    QTRY_COMPARE(subclass3.value(), 19);
}

QTEST_MAIN(SubClassReplicaTest)

#include "tst_subclassreplicatest.moc"
