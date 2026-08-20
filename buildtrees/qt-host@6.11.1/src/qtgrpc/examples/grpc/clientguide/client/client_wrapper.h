// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include "clientguide_client.grpc.qpb.h"

#include <QtCore/qcoreapplication.h>

// In this example, the wrapper is used only to track active operations and
// automatically exit the client once they have all finished. In a real-world
// application, shutdown would typically be managed by the application itself.
class ClientGuideServiceWrapper : public QObject
{
public:
    ~ClientGuideServiceWrapper() override = default;

    bool attachChannel(std::shared_ptr<QAbstractGrpcChannel> channel)
    {
        return client.attachChannel(std::move(channel));
    }

    std::unique_ptr<QGrpcCallReply> UnaryCall(const client::guide::Request &arg,
                                              const QGrpcCallOptions &options = { })
    {
        auto call = client.UnaryCall(arg, options);
        trackOperation(call.get());
        return call;
    }
    std::unique_ptr<QGrpcServerStream> ServerStreaming(const client::guide::Request &arg,
                                                       const QGrpcCallOptions &options = { })
    {
        auto stream = client.ServerStreaming(arg, options);
        trackOperation(stream.get());
        return stream;
    }
    std::unique_ptr<QGrpcClientStream> ClientStreaming(const client::guide::Request &arg,
                                                       const QGrpcCallOptions &options = { })
    {
        auto stream = client.ClientStreaming(arg, options);
        trackOperation(stream.get());
        return stream;
    }
    std::unique_ptr<QGrpcBidiStream> BidirectionalStreaming(const client::guide::Request &arg,
                                                            const QGrpcCallOptions &options = { })
    {
        auto stream = client.BidirectionalStreaming(arg, options);
        trackOperation(stream.get());
        return stream;
    }

private:
    void trackOperation(const QGrpcOperation *op)
    {
        ++activeRPCs;
        killTimer(timerId);
        timerId = 0;
        connect(op, &QGrpcOperation::finished, this, [this] {
            if (--activeRPCs <= 0)
                timerId = startTimer(std::chrono::seconds(1));
        });
    }
    void timerEvent(QTimerEvent *event) override
    {
        Q_ASSERT(event->timerId() == timerId);
        qDebug("All operations completed! Automatically shutting down...");
        QCoreApplication::quit();
    }

private:
    client::guide::ClientGuideService::Client client;
    int timerId = 0;
    int activeRPCs = 0;
};
