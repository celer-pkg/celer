// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "vehiclethread.h"
#include <vehicleservice_client.grpc.qpb.h>

#include <QtCore/QUrl>
#include <QtGrpc/QGrpcChannelOptions>
#include <QtGrpc/QGrpcHttp2Channel>

#include <QDebug>

using namespace qtgrpc::examples;
using namespace google::protobuf;

VehicleThread::VehicleThread(QObject *parent) : QThread(parent)
{
}

VehicleThread::~VehicleThread() = default;

void VehicleThread::run()
{
    if (!m_client) {
        m_client = std::make_unique<qtgrpc::examples::VehicleService::Client>();
        m_client
            ->attachChannel(std::make_shared<QGrpcHttp2Channel>(QUrl("http://localhost:50051")));
    }

    //! [Speed stream]
    Empty speedRequest;
    m_streamSpeed = m_client->getSpeedStream(speedRequest);

    connect(m_streamSpeed.get(), &QGrpcServerStream::messageReceived, this, [this]() {
        if (const auto speedResponse = m_streamSpeed->read<SpeedMsg>()) {
            emit speedChanged(speedResponse->speed());
        }
    });

    connect(
        m_streamSpeed.get(), &QGrpcServerStream::finished, this,
        [this](const QGrpcStatus &status) {
            if (!status.isOk()) {
                auto error = QString("Stream error fetching speed %1 (%2)")
                                 .arg(status.message())
                                 .arg(QVariant::fromValue(status.code()).toString());
                emit connectionError(error);
                qWarning() << error;
                return;
            }
        },
        Qt::SingleShotConnection);
    //! [Speed stream]

    Empty rpmRequest;
    m_streamRpm = m_client->getRpmStream(rpmRequest);
    connect(m_streamRpm.get(), &QGrpcServerStream::messageReceived, this, [this]() {
        if (const auto rpmResponse = m_streamRpm->read<RpmMsg>()) {
            emit rpmChanged(rpmResponse->rpm());
        }
    });

    connect(
        m_streamRpm.get(), &QGrpcServerStream::finished, this,
        [this](const QGrpcStatus &status) {
            if (!status.isOk()) {
                auto error = QString("Stream error fetching RPM %1 (%2)")
                                 .arg(status.message())
                                 .arg(QVariant::fromValue(status.code()).toString());
                emit connectionError(error);
                qWarning() << error;
                return;
            }
        },
        Qt::SingleShotConnection);

    //! [Autonomy call]
    Empty autonomyRequest;
    std::unique_ptr<QGrpcCallReply> autonomyReply = m_client->getAutonomy(autonomyRequest);
    const auto *autonomyReplyPtr = autonomyReply.get();
    connect(
        autonomyReplyPtr, &QGrpcCallReply::finished, this,
        [this, autonomyReply = std::move(autonomyReply)](const QGrpcStatus &status) {
            if (!status.isOk()) {
                auto error = QString("Call error fetching autonomy %1 (%2)")
                                 .arg(status.message())
                                 .arg(QVariant::fromValue(status.code()).toString());
                emit connectionError(error);
                qWarning() << error;
                return;
            }

            if (const auto autonomyMsg = autonomyReply->read<AutonomyMsg>()) {
                emit autonomyChanged(autonomyMsg->autonomy());
            }
        },
        Qt::SingleShotConnection);
    //! [Autonomy call]

    QThread::run();

    // Delete the VehicleService::Client object to shut down the connection
    m_client.reset();
}
