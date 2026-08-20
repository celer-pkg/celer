// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QString>
#include <QtTest>
#include "rep_model_merged.h"

class ModelreplicaTest : public QObject
{
    Q_OBJECT

public:
    ModelreplicaTest() = default;

private Q_SLOTS:
    void basicFunctions();
    void basicFunctions_data();
    void nullModel();
    void nestedSortFilterProxyModel_data() { basicFunctions_data(); }
    void nestedSortFilterProxyModel();
    void sortFilterProxyModel_data();
    void sortFilterProxyModel();
};

void ModelreplicaTest::basicFunctions_data()
{
    QTest::addColumn<bool>("templated");
    QTest::newRow("non-templated enableRemoting") << false;
    QTest::newRow("templated enableRemoting") << true;
}

void ModelreplicaTest::basicFunctions()
{
    QFETCH(bool, templated);

    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5550"));
    auto model = new QStringListModel();
    model->setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    MediaSimpleSource source;
    source.setTracks(model);
    if (templated)
        host.enableRemoting<MediaSourceAPI>(&source);
    else
        host.enableRemoting(&source);

    auto model2 = new QStringListModel();
    model2->setStringList(QStringList() << "Track4" << "Track5");
    OtherMediaSimpleSource source2;
    source2.setTracks(model2);
    host.enableRemoting(&source2);

    QRemoteObjectNode client(QUrl("tcp://localhost:5550"));
    const QScopedPointer<MediaReplica> replica(client.acquire<MediaReplica>());
    QSignalSpy tracksSpy(replica->tracks(), &QAbstractItemModelReplica::initialized);
    QVERIFY(replica->waitForSource(300));
    QVERIFY(tracksSpy.wait());
    // Rep file only uses display role
    QCOMPARE(QList<int>{Qt::DisplayRole}, replica->tracks()->availableRoles());

    QCOMPARE(model->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
    {
        // We haven't received any data yet
        QCOMPARE(QVariant(), replica->tracks()->data(replica->tracks()->index(i, 0)));
    }

    // Wait for data to be fetch and confirm
    QTest::qWait(100);
    QCOMPARE(model->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
    {
        QTRY_COMPARE(model->data(model->index(i), Qt::DisplayRole), replica->tracks()->data(replica->tracks()->index(i, 0)));
    }

    // ensure the tracks objects are distinct
    const QScopedPointer<OtherMediaReplica> otherReplica(client.acquire<OtherMediaReplica>());
    QSignalSpy otherTracksSpy(otherReplica->tracks(), &QAbstractItemModelReplica::initialized);
    QVERIFY(otherReplica->waitForSource(300));
    QVERIFY(otherTracksSpy.wait());
    QCOMPARE(otherReplica->tracks()->availableRoles().size(), 2);

}

void ModelreplicaTest::nullModel()
{
    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5550"));
    MediaSimpleSource source;
    host.enableRemoting(&source);

    QRemoteObjectNode client(QUrl("tcp://localhost:5550"));
    const QScopedPointer<MediaReplica> replica(client.acquire<MediaReplica>());
    QVERIFY(replica->waitForSource(300));

    auto model = new QStringListModel(this);
    model->setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    source.setTracks(model);

    QTRY_VERIFY(replica->tracks());
    QTRY_COMPARE(replica->tracks()->rowCount(), 3);
    QTRY_COMPARE(replica->tracks()->data(replica->tracks()->index(0, 0)), "Track1");

    model = new QStringListModel(this);
    model->setStringList(QStringList() << "New Track1" << "New Track2" << "New Track3"  << "New Track4");
    source.setTracks(model);
    QTRY_COMPARE(replica->tracks()->rowCount(), 4);
    QTRY_COMPARE(replica->tracks()->data(replica->tracks()->index(3, 0)), "New Track4");
}

void ModelreplicaTest::nestedSortFilterProxyModel()
{
    QFETCH(bool, templated);

    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5550"));
    auto model = new QStringListModel(this);
    model->setStringList(QStringList() << "CCC" << "AAA" << "BBB");
    auto proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    MediaSimpleSource source;
    source.setTracks(proxyModel);
    if (templated)
        host.enableRemoting<MediaSourceAPI>(&source);
    else
        host.enableRemoting(&source);

    QRemoteObjectNode client(QUrl("tcp://localhost:5550"));
    const QScopedPointer<MediaReplica> replica(client.acquire<MediaReplica>());
    QSignalSpy tracksSpy(replica->tracks(), &QAbstractItemModelReplica::initialized);
    QVERIFY(replica->waitForSource(300));
    QVERIFY(tracksSpy.wait());
    // Rep file only uses display role
    QCOMPARE(QVector<int>{Qt::DisplayRole}, replica->tracks()->availableRoles());

    QCOMPARE(proxyModel->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++) {
        // We haven't received any data yet
        QCOMPARE(QVariant(), replica->tracks()->data(replica->tracks()->index(i, 0)));
    }

    // Wait for data to be fetch and confirm
    QCOMPARE(proxyModel->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
        QTRY_COMPARE(proxyModel->data(proxyModel->index(i, 0), Qt::DisplayRole), replica->tracks()->data(replica->tracks()->index(i, 0)));

    proxyModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(proxyModel->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
        QTRY_COMPARE(proxyModel->data(proxyModel->index(i, 0), Qt::DisplayRole), replica->tracks()->data(replica->tracks()->index(i, 0)));
}

void ModelreplicaTest::sortFilterProxyModel_data()
{
    QTest::addColumn<bool>("prefetch");

    QTest::newRow("size only") << false;
    QTest::newRow("prefetch") << true;
}

void ModelreplicaTest::sortFilterProxyModel()
{
    QFETCH(bool, prefetch);

    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5550"));
    auto model = new QStringListModel(this);
    model->setStringList(QStringList() << "CCC" << "AAA" << "BBB");
    auto proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    QList<int> roles = { Qt::DisplayRole };
    host.enableRemoting(proxyModel, "test", roles);

    auto fetchMode = prefetch ? QtRemoteObjects::PrefetchData : QtRemoteObjects::FetchRootSize;
    QRemoteObjectNode client(QUrl("tcp://localhost:5550"));
    QScopedPointer<QAbstractItemModelReplica> replica(client.acquireModel("test", fetchMode));
    QSignalSpy initSpy(replica.get(), &QAbstractItemModelReplica::initialized);
    QVERIFY(initSpy.wait());

    QCOMPARE(roles, replica->availableRoles());

    // Wait for data to be fetch and confirm
    QCOMPARE(proxyModel->rowCount(), replica->rowCount());
    for (int i = 0; i < replica->rowCount(); i++)
        QTRY_COMPARE(proxyModel->data(proxyModel->index(i, 0), Qt::DisplayRole),
                     replica->data(replica->index(i, 0)));

    proxyModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(proxyModel->rowCount(), replica->rowCount());

    for (int i = 0; i < replica->rowCount(); i++)
        QTRY_COMPARE(proxyModel->data(proxyModel->index(i, 0), Qt::DisplayRole),
                     replica->data(replica->index(i, 0)));
}

QTEST_MAIN(ModelreplicaTest)

#include "tst_modelreplicatest.moc"
