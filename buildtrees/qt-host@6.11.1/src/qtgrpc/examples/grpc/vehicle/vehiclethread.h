// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VEHICLETHREAD_H
#define VEHICLETHREAD_H

#include "vehicleservice_client.grpc.qpb.h"
#include <QtCore/QThread>
#include <memory>

namespace qtgrpc::examples {

class VehicleThread : public QThread
{
    Q_OBJECT

public:
    explicit VehicleThread(QObject *parent = nullptr);
    ~VehicleThread() override;

    void run() override;

signals:
    void speedChanged(int speed);
    void rpmChanged(int rpm);
    void autonomyChanged(int level);

    void connectionError(QString error);

private:
    std::unique_ptr<qtgrpc::examples::VehicleService::Client> m_client;
    std::unique_ptr<QGrpcServerStream> m_streamSpeed;
    std::unique_ptr<QGrpcServerStream> m_streamRpm;
};

}

#endif // VEHICLETHREAD_H
