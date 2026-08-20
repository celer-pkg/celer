// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2016 Javier S. Pedro <maemo@javispedro.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QLOWENERGYCONTROLLER_DARWIN_P_H
#define QLOWENERGYCONTROLLER_DARWIN_P_H

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

#include "qlowenergyserviceprivate_p.h"
#include "qlowenergycontrollerbase_p.h"
#include "qlowenergycontroller.h"
#include "darwin/btnotifier_p.h"
#include "qbluetoothaddress.h"
#include "darwin/btraii_p.h"
#include "qbluetoothuuid.h"

#include <QtCore/qsharedpointer.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QByteArray;

class QLowEnergyControllerPrivateDarwin : public QLowEnergyControllerPrivate
{
    friend class QLowEnergyController;
    friend class QLowEnergyService;

    Q_OBJECT
public:
    QLowEnergyControllerPrivateDarwin();
    ~QLowEnergyControllerPrivateDarwin();

    void init() override;
    bool lazyInit();
    void connectToDevice() override;
    void disconnectFromDevice() override;
    void discoverServices() override;
    void discoverServiceDetails(const QBluetoothUuid &serviceUuid,
                                QLowEnergyService::DiscoveryMode mode) override;

    void readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                            const QLowEnergyHandle charHandle) override;
    void readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                        const QLowEnergyHandle charHandle,
                        const QLowEnergyHandle descriptorHandle) override;

    void writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                             const QLowEnergyHandle charHandle, const QByteArray &newValue,
                             QLowEnergyService::WriteMode mode) override;
    void writeDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                         const QLowEnergyHandle charHandle,
                         const QLowEnergyHandle descriptorHandle,
                         const QByteArray &newValue) override;


    void requestConnectionUpdate(const QLowEnergyConnectionParameters &params) override;
    void addToGenericAttributeList(const QLowEnergyServiceData &service,
                                   QLowEnergyHandle startHandle) override;

    int mtu() const override;
    void readRssi() override;

    void startAdvertising(const QLowEnergyAdvertisingParameters &params,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData) override;
    void stopAdvertising()override;
    QLowEnergyService *addServiceHelper(const QLowEnergyServiceData &service) override;

    // Valid - a central or peripheral instance was allocated, and this may also
    // mean a proper usage description was provided/found:
    bool isValid() const;

private Q_SLOTS:
    void _q_connected();
    void _q_disconnected();

    void _q_mtuChanged(int newValue);
    void _q_serviceDiscoveryFinished();
    void _q_serviceDetailsDiscoveryFinished(QSharedPointer<QLowEnergyServicePrivate> service);
    void _q_servicesWereModified();

    void _q_characteristicRead(QLowEnergyHandle charHandle, const QByteArray &value);
    void _q_characteristicWritten(QLowEnergyHandle charHandle, const QByteArray &value);
    void _q_characteristicUpdated(QLowEnergyHandle charHandle, const QByteArray &value);
    void _q_descriptorRead(QLowEnergyHandle descHandle, const QByteArray &value);
    void _q_descriptorWritten(QLowEnergyHandle charHandle, const QByteArray &value);
    void _q_notificationEnabled(QLowEnergyHandle charHandle, bool enabled);

    void _q_LEnotSupported();
    void _q_CBManagerError(QLowEnergyController::Error error);
    void _q_CBManagerError(const QBluetoothUuid &serviceUuid, QLowEnergyController::Error error);
    void _q_CBManagerError(const QBluetoothUuid &serviceUuid, QLowEnergyService::ServiceError error);

private:
    void setNotifyValue(QSharedPointer<QLowEnergyServicePrivate> service,
                        QLowEnergyHandle charHandle, const QByteArray &newValue);

    quint16 updateValueOfCharacteristic(QLowEnergyHandle charHandle,
                                        const QByteArray &value,
                                        bool appendValue);

    quint16 updateValueOfDescriptor(QLowEnergyHandle charHandle,
                                    QLowEnergyHandle descHandle,
                                    const QByteArray &value,
                                    bool appendValue);

    bool connectSlots(DarwinBluetooth::LECBManagerNotifier *notifier);

    DarwinBluetooth::ScopedPointer centralManager;
    DarwinBluetooth::ScopedPointer peripheralManager;

    using ServiceMap = QMap<QBluetoothUuid, QSharedPointer<QLowEnergyServicePrivate>>;
};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLER_DARWIN_P_H
