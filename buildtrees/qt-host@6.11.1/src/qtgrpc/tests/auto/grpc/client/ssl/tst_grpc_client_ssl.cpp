// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGrpc/qtgrpcglobal.h>

#include <QSignalSpy>
#include <QTest>
#include <QtNetwork/QSslConfiguration>

#include <grpcclienttestbase.h>
#include <message_latency_defs.h>

#include "testservice_client.grpc.qpb.h"

using namespace qtgrpc::tests;

class QtGrpcSslClientTest : public GrpcClientTestBase
{
    Q_OBJECT
public:
    QtGrpcSslClientTest()
        : GrpcClientTestBase(Channels(GrpcClientTestBase::Channel::SslNoCredentials))
    {
    }
private Q_SLOTS:
    void incorrectSecureCredentialsTest();
};

void QtGrpcSslClientTest::incorrectSecureCredentialsTest()
{
    SimpleStringMessage req;
    req.setTestFieldString("Hello Qt!");

    auto reply = client()->testMethod(req);
    QSignalSpy finishedSpy(reply.get(), &QGrpcCallReply::finished);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(finishedSpy.count(), 1, MessageLatencyWithThreshold);
    auto status = finishedSpy.first().first().value<QGrpcStatus>();
    QVERIFY(!status.isOk());
}

QTEST_MAIN(QtGrpcSslClientTest)
#include "tst_grpc_client_ssl.moc"
