// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "grpcclienttestbase.h"

#include <QtGrpc/QGrpcChannelOptions>
#include <QtNetwork/qtnetwork-config.h>
#include <message_latency_defs.h>

using namespace Qt::Literals::StringLiterals;

void GrpcClientTestBase::initTestCase_data()
{
    QTest::addColumn<GrpcClientTestBase::Channels>("type");
    QTest::addColumn<std::shared_ptr<QAbstractGrpcChannel>>("channel");

    if (m_channels.testFlag(Channel::Qt)) {
        QTest::newRow("Http2Client")
            << QFlags{ Channel::Qt }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("http://localhost:50051",
                                                                    QUrl::StrictMode)));
#if !defined(Q_OS_WINDOWS) && QT_CONFIG(localserver)
        QTest::newRow("Http2ClientUnix")
            << QFlags{ Channel::Qt }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("unix:///tmp/qtgrpc_test.sock",
                                                                    QUrl::StrictMode)));
#endif
    }

#ifdef TEST_GRPC_SERVER_SUPPORTS_JSON
    if (m_channels.testFlag(Channel::Json)) {
        QHash<QByteArray, QByteArray> md{
            {"content-type"_ba, "application/grpc+json"}
        };
        QTest::newRow("Http2ClientJson")
            << QFlags{ Channel::Qt }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("http://localhost:50051",
                                                                    QUrl::StrictMode),
                                                               QGrpcChannelOptions{}
                                                                   .setMetadata(md)));

        QTest::newRow("Http2ClientJsonUnix")
            << QFlags{ Channel::Qt }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("unix:///tmp/qtgrpc_test.sock",
                                                                    QUrl::StrictMode),
                                                               QGrpcChannelOptions{}
                                                                   .setMetadata(md)));
    }
#endif

#if QT_CONFIG(ssl)
    if (m_channels.testFlag(Channel::Ssl)) {
        QFile caCerificateFile(":/assets/cert.pem");
        QVERIFY2(caCerificateFile.open(QFile::ReadOnly), "Unable to open ssl ca certificate file");
        QSslConfiguration sslConfig;
        QSslCertificate caCert(caCerificateFile.readAll());
        sslConfig.setProtocol(QSsl::TlsV1_2);
        sslConfig.addCaCertificate(caCert);
        sslConfig.setAllowedNextProtocols({ QByteArray("h2") });
        QTest::newRow("Http2ClientSSL")
            << QFlags{ Channel::Qt, Channel::Ssl }
            << std::shared_ptr<QAbstractGrpcChannel>(
                   new QGrpcHttp2Channel(QUrl("https://localhost:50052", QUrl::StrictMode),
                                         QGrpcChannelOptions{}.setSslConfiguration(sslConfig)));
    }

    if (m_channels.testFlag(Channel::SslNoCredentials)) {
        QSslConfiguration sslConfig;
        sslConfig.setProtocol(QSsl::TlsV1_2);
        sslConfig.setAllowedNextProtocols({ QByteArray("h2") });
        QGrpcChannelOptions channelOptions;
        channelOptions.setSslConfiguration(sslConfig);

        QTest::newRow("Http2ClientSSLNoCredentials")
            << QFlags{ Channel::Qt, Channel::SslNoCredentials }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("https://localhost:50052",
                                                                    QUrl::StrictMode),
                                                               channelOptions));
    }
#endif

    if (m_channels.testFlag(Channel::WithChannelDeadline)) {
        constexpr auto
            channelTimeout = std::chrono::milliseconds(static_cast<int64_t>(MessageLatency * 0.25));
        QTest::newRow("Http2ClientDeadline")
            << QFlags{ Channel::Qt, Channel::WithChannelDeadline }
            << std::shared_ptr<
                   QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("http://localhost:50051",
                                                                    QUrl::StrictMode),
                                                               QGrpcChannelOptions{}
                                                                   .setDeadlineTimeout(channelTimeout)));
    }
}

void GrpcClientTestBase::init()
{
    QFETCH_GLOBAL(GrpcClientTestBase::Channels, type);
    m_channelType = type;
    QFETCH_GLOBAL(std::shared_ptr<QAbstractGrpcChannel>, channel);
    m_client = std::make_shared<qtgrpc::tests::TestService::Client>();
    m_client->attachChannel(std::move(channel));

    if (m_serverProccess.state() != QProcess::ProcessState::Running) {
        qInfo() << "Restarting server";
        m_serverProccess.restart();
        QVERIFY2(m_serverProccess.state() == QProcess::ProcessState::Running,
                 "Precondition failed - Server cannot be started.");
    }
}

std::shared_ptr<qtgrpc::tests::TestService::Client> GrpcClientTestBase::client()
{
    return m_client;
}

GrpcClientTestBase::Channels GrpcClientTestBase::channelType()
{
    return m_channelType;
}
