// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VEHICLEMANAGER_H
#define VEHICLEMANAGER_H

#include <QQmlPropertyMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QtTypeTraits>
#include <QtQml/QQmlEngine>
#include <memory>
#include "navigationservice_client.grpc.qpb.h"
#include "navigationservice.qpb.h"
#include "navigationthread.h"
#include "vehiclethread.h"

class VehicleManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum NavigationDirection {
        RIGHT = qToUnderlying(qtgrpc::examples::DirectionEnumGadget::DirectionEnum::RIGHT),
        LEFT = qToUnderlying(qtgrpc::examples::DirectionEnumGadget::DirectionEnum::LEFT),
        STRAIGHT = qToUnderlying(qtgrpc::examples::DirectionEnumGadget::DirectionEnum::STRAIGHT),
        BACKWARD = qToUnderlying(qtgrpc::examples::DirectionEnumGadget::DirectionEnum::BACKWARD)
    };

    Q_ENUM(NavigationDirection);

    explicit VehicleManager(QObject *parent = nullptr);
    ~VehicleManager() override;

    Q_INVOKABLE void restart();

    void addVehicleError(QString error);
    void addNavigationError(QString error);
    void removeErrors();

signals:
    void speedChanged(int speed);
    void rpmChanged(int rpm);
    void autonomyChanged(int level);

    void totalDistanceChanged(int distance);
    void traveledDistanceChanged(int distance);
    void directionChanged(int direction);
    void streetChanged(QString street);

    void vehicleErrorsChanged(QString vehicleErrors);
    void navigationErrorsChanged(QString navigationErrors);

private:
    void clearErrors();
    void startVehicleClient();
    void startNavigationClient();

    std::unique_ptr<qtgrpc::examples::NavigationThread> m_navigationThread;
    std::unique_ptr<qtgrpc::examples::VehicleThread> m_vehicleThread;

    QString m_vehicleErrors;
    QString m_navigationErrors;
};

#endif // VEHICLEMANAGER_H
