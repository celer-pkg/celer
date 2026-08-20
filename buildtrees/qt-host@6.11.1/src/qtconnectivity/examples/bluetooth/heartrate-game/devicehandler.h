// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"

#include <QLowEnergyController>
#include <QLowEnergyService>

#include <QDateTime>
#include <QList>
#include <QTimer>

#include <QQmlEngine>

class DeviceInfo;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool measuring READ measuring NOTIFY measuringChanged)
    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(int hr READ hr NOTIFY statsChanged)
    Q_PROPERTY(int maxHR READ maxHR NOTIFY statsChanged)
    Q_PROPERTY(int minHR READ minHR NOTIFY statsChanged)
    Q_PROPERTY(float average READ average NOTIFY statsChanged)
    Q_PROPERTY(int time READ time NOTIFY statsChanged)
    Q_PROPERTY(float calories READ calories NOTIFY statsChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)

    QML_ELEMENT

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = nullptr);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    bool measuring() const;
    bool alive() const;

    // Statistics
    int hr() const;
    int time() const;
    float average() const;
    int maxHR() const;
    int minHR() const;
    float calories() const;

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateHeartRateValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                                  const QByteArray &value);

    void updateDemoHR();

private:
    void addMeasurement(int value);

    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyDescriptor m_notificationDesc;
    DeviceInfo *m_currentDevice = nullptr;

    bool m_foundHeartRateService = false;
    bool m_measuring = false;
    int m_currentValue = 0, m_min = 0, m_max = 0, m_sum = 0;
    float m_avg = 0, m_calories = 0;

    // Statistics
    QDateTime m_start;
    QDateTime m_stop;

    QList<int> m_measurements;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    QTimer m_demoTimer;
};

#endif // DEVICEHANDLER_H
