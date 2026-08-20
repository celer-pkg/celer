// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QLocalServer>
#include <QRemoteObjectDynamicReplica>
#include <QRemoteObjectHost>
#include <QRemoteObjectNode>
#include <QTest>

class tst_LocalServerOption : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLocalServerOption_data();
    void testLocalServerOption();
};

void tst_LocalServerOption::testLocalServerOption_data()
{
    QTest::addColumn<QLocalServer::SocketOptions>("options");
    QTest::addColumn<bool>("successExpected");

    QTest::newRow("UserAccessOption")
            << QLocalServer::SocketOptions(QLocalServer::UserAccessOption) << true;
    QTest::newRow("GroupAccessOption")
            << QLocalServer::SocketOptions(QLocalServer::GroupAccessOption) << false;
    QTest::newRow("UserAndGroupAccessOption") << QLocalServer::SocketOptions(
            QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption)
                                              << true;
    QTest::newRow("OtherAccessOption")
            << QLocalServer::SocketOptions(QLocalServer::OtherAccessOption) << false;
    QTest::newRow("OtherAndGroupAccessOption") << QLocalServer::SocketOptions(
            QLocalServer::OtherAccessOption | QLocalServer::GroupAccessOption)
                                               << false;
    QTest::newRow("UserAndGroupAndOtherAccessOption") << QLocalServer::SocketOptions(
            QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption
            | QLocalServer::GroupAccessOption) << true;
    QTest::newRow("WorldAccessOption")
            << QLocalServer::SocketOptions(QLocalServer::WorldAccessOption) << true;
}

void tst_LocalServerOption::testLocalServerOption()
{
#ifdef Q_OS_LINUX
    using namespace Qt::Literals;
    QFETCH(QLocalServer::SocketOptions, options);
    QFETCH(bool, successExpected);

    // Server side setup
    std::unique_ptr<QObject> sourceModel = std::make_unique<QObject>();
    QRemoteObjectHost::setLocalServerOptions(options);
    QRemoteObjectHost srcNode(QUrl("local:replica"_L1));
    srcNode.enableRemoting(sourceModel.get(), "RemoteModel"_L1);

    // Client side setup
    QRemoteObjectNode repNode;
    repNode.connectToNode(QUrl("local:replica"_L1));
    std::unique_ptr<QRemoteObjectDynamicReplica> replicaModel(
            repNode.acquireDynamic("RemoteModel"_L1));

    // Verification
    QCOMPARE(replicaModel->waitForSource(1000), successExpected);
    QCOMPARE(repNode.lastError(),
             successExpected ? QRemoteObjectNode::NoError : QRemoteObjectNode::SocketAccessError);
#else
    QSKIP("This test is only useful on Linux");
#endif
}

QTEST_MAIN(tst_LocalServerOption)

#include "tst_localserveroption.moc"
