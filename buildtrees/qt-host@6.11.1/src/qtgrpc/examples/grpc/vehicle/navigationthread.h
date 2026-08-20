// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef NAVIGATIONTHREAD_H
#define NAVIGATIONTHREAD_H

#include <QtCore/QString>
#include <QtCore/QThread>
#include <memory>
#include <navigationservice_client.grpc.qpb.h>

namespace qtgrpc::examples {

class NavigationThread : public QThread
{
    Q_OBJECT

public:
    explicit NavigationThread(QObject *parent = nullptr);
    ~NavigationThread() override;

    void run() override;

signals:
    void totalDistanceChanged(int distance);
    void traveledDistanceChanged(int distance);
    void directionChanged(qtgrpc::examples::DirectionEnumGadget::DirectionEnum direction);
    void streetChanged(QString street);

    void connectionError(QString error);

private:
    std::unique_ptr<QGrpcServerStream> m_stream;
    std::shared_ptr<qtgrpc::examples::NavigationService::Client> m_client;
};

}

#endif // NAVIGATIONTHREAD_H
