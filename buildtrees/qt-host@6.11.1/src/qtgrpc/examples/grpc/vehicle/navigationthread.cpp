// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "navigationthread.h"
#include <navigationservice_client.grpc.qpb.h>

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtGrpc/QGrpcChannelOptions>
#include <QtGrpc/QGrpcHttp2Channel>

using namespace qtgrpc::examples;
using namespace google::protobuf;

NavigationThread::NavigationThread(QObject *parent) : QThread(parent)
{
}

NavigationThread::~NavigationThread() = default;

void NavigationThread::run()
{
    if (!m_client) {
        auto channel = std::shared_ptr<
            QAbstractGrpcChannel>(new QGrpcHttp2Channel(QUrl("http://localhost:50051",
                                                             QUrl::StrictMode)));
        m_client = std::make_shared<qtgrpc::examples::NavigationService::Client>();
        m_client->attachChannel(channel);
    }

    Empty request;
    m_stream = m_client->getNavigationStream(request);

    connect(m_stream.get(), &QGrpcServerStream::messageReceived, this, [this] {
        const auto result = m_stream->read<NavigationMsg>();
        if (!result)
            return;

        emit totalDistanceChanged(result->totalDistance());
        emit traveledDistanceChanged(result->traveledDistance());
        emit directionChanged(result->direction());
        emit streetChanged(result->street());
    });

    connect(
        m_stream.get(), &QGrpcServerStream::finished, this,
        [this](const QGrpcStatus &status) {
            if (!status.isOk()) {
                auto error = QString("Stream error fetching navigation %1 (%2)")
                                 .arg(status.message())
                                 .arg(QVariant::fromValue(status.code()).toString());
                emit connectionError(error);
                qWarning() << error;
                return;
            }

            emit totalDistanceChanged(0);
            emit traveledDistanceChanged(0);
            emit directionChanged(DirectionEnumGadget::DirectionEnum::BACKWARD);
            m_stream.reset();
        },
        Qt::SingleShotConnection);

    QThread::run();

    // Delete the NavigationService::Client object to shut down the connection
    m_client.reset();
}
