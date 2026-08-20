// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef GRPCCLIENTTESTBASE_H
#define GRPCCLIENTTESTBASE_H

#include <QObject>
#include <QTest>
#include <QUrl>

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/QGrpcHttp2Channel>

#include <server_proc_runner.h>
#include <testservice_client.grpc.qpb.h>

class GrpcClientTestBase : public QObject
{
    Q_OBJECT
protected:
    enum Channel {
        NoChannels = 0x0,
        Qt = 0x1,
        Ssl = 0x2,
        Json = 0x4,
        SslNoCredentials = 0x8,
        WithChannelDeadline = 0x10,
    };
    Q_DECLARE_FLAGS(Channels, Channel)

    GrpcClientTestBase(Channels channels) : m_channels(channels) { }

    std::shared_ptr<qtgrpc::tests::TestService::Client> client();
    GrpcClientTestBase::Channels channelType();

public Q_SLOTS:
    void initTestCase_data();
    void init();

private:
    ServerProcRunner m_serverProccess{ QFINDTESTDATA(TEST_GRPC_SERVER_PATH) };
    Channels m_channelType;
    Channels m_channels;
    std::shared_ptr<qtgrpc::tests::TestService::Client> m_client;
};

#endif // GRPCCLIENTTESTBASE_H
