// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYCONTROLLERPRIVATEANDROID_P_H
#define QLOWENERGYCONTROLLERPRIVATEANDROID_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <qglobal.h>
#include <QtCore/QQueue>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include "qlowenergycontroller.h"
#include "qlowenergycontrollerbase_p.h"

#include <QtCore/QJniObject>
#include "android/lowenergynotificationhub_p.h"

#include <functional>

QT_BEGIN_NAMESPACE

class QLowEnergyServiceData;
class QTimer;

class LowEnergyNotificationHub;

extern void registerQLowEnergyControllerMetaType();

class QLowEnergyControllerPrivateAndroid final : public QLowEnergyControllerPrivate
{
    Q_OBJECT
public:
    QLowEnergyControllerPrivateAndroid();
    ~QLowEnergyControllerPrivateAndroid() override;

    void init() override;

    void connectToDevice() override;
    void disconnectFromDevice() override;

    void discoverServices() override;
    void discoverServiceDetails(const QBluetoothUuid &service,
                                QLowEnergyService::DiscoveryMode mode) override;

    void startAdvertising(const QLowEnergyAdvertisingParameters &params,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData) override;
    void stopAdvertising() override;

    void requestConnectionUpdate(const QLowEnergyConnectionParameters &params) override;

    // read data
    void readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                            const QLowEnergyHandle charHandle) override;
    void readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                        const QLowEnergyHandle charHandle,
                        const QLowEnergyHandle descriptorHandle) override;

    // write data
    void writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                             const QLowEnergyHandle charHandle,
                             const QByteArray &newValue, QLowEnergyService::WriteMode mode) override;
    void writeDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                         const QLowEnergyHandle charHandle,
                         const QLowEnergyHandle descriptorHandle,
                         const QByteArray &newValue) override;

    void addToGenericAttributeList(const QLowEnergyServiceData &service,
                                   QLowEnergyHandle startHandle) override;

    int mtu() const override;
    void readRssi() override;

private:

    LowEnergyNotificationHub *hub;

private slots:
    void connectionUpdated(QLowEnergyController::ControllerState newState,
                           QLowEnergyController::Error errorCode);
    void mtuChanged(int mtu);
    void remoteRssiRead(int rssi, bool success);
    void servicesDiscovered(QLowEnergyController::Error errorCode,
                            const QString &foundServices);
    void serviceDetailsDiscoveryFinished(const QString& serviceUuid,
                                         int startHandle, int endHandle);
    void characteristicRead(const QBluetoothUuid &serviceUuid, int handle,
                            const QBluetoothUuid &charUuid, int properties,
                            const QByteArray& data);
    void descriptorRead(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
                        int handle, const QBluetoothUuid &descUuid, const QByteArray &data);
    void characteristicWritten(int charHandle, const QByteArray &data,
                               QLowEnergyService::ServiceError errorCode);
    void descriptorWritten(int descHandle, const QByteArray &data,
                           QLowEnergyService::ServiceError errorCode);
    void serverDescriptorWritten(const QJniObject &jniDesc, const QByteArray &newValue);
    void characteristicChanged(int charHandle, const QByteArray &data);
    void serverCharacteristicChanged(const QJniObject &jniChar, const QByteArray &newValue);
    void serviceError(int attributeHandle, QLowEnergyService::ServiceError errorCode);
    void advertisementError(int errorCode);

private:
    void peripheralConnectionUpdated(QLowEnergyController::ControllerState newState,
                           QLowEnergyController::Error errorCode);
    void centralConnectionUpdated(QLowEnergyController::ControllerState newState,
                           QLowEnergyController::Error errorCode);
};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLERPRIVATEANDROID_P_H
