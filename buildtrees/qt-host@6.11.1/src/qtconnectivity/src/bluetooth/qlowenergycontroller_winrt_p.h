// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYCONTROLLERPRIVATEWINRT_P_H
#define QLOWENERGYCONTROLLERPRIVATEWINRT_P_H

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
#include <QtCore/QList>
#include <QtCore/QQueue>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include "qlowenergycontroller.h"
#include "qlowenergycontrollerbase_p.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

namespace winrt
{
    using AsyncStatus = Windows::Foundation::AsyncStatus;
    using IInspectable = Windows::Foundation::IInspectable;
}

#include <functional>

QT_BEGIN_NAMESPACE

class QLowEnergyServiceData;
class QTimer;

extern void registerQLowEnergyControllerMetaType();

class QLowEnergyControllerPrivateWinRT final : public QLowEnergyControllerPrivate
{
    Q_OBJECT
public:
    QLowEnergyControllerPrivateWinRT();
    ~QLowEnergyControllerPrivateWinRT() override;

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

signals:
    void characteristicChanged(quint16 charHandle, const QByteArray &data);
    void abortConnection();

private slots:
    void handleCharacteristicChanged(quint16 charHandle, const QByteArray &data);
    void handleServiceHandlerError(const QString &error);

private:
    void handleConnectionError(const char *logMessage);

    BluetoothLEDevice mDevice = nullptr;
    GattSession mGattSession = nullptr;
    winrt::event_token mStatusChangedToken{ 0 };
    winrt::event_token mMtuChangedToken{ 0 };
    struct ValueChangedEntry {
        ValueChangedEntry() {}
        ValueChangedEntry(GattCharacteristic c,
                          winrt::event_token t)
            : characteristic(c)
            , token(t)
        {
        }

        GattCharacteristic characteristic = nullptr;
        winrt::event_token token = { 0 };
    };
    QList<ValueChangedEntry> mValueChangedTokens;

    QMap<QBluetoothUuid, GattDeviceService> m_openedServices;
    QSet<QBluetoothUuid> m_requestDetailsServiceUuids;

    using NativeServiceCallback = std::function<void(GattDeviceService)>;
    bool getNativeService(const QBluetoothUuid &serviceUuid, NativeServiceCallback callback);

    using NativeCharacteristicCallback = std::function<void(GattCharacteristic)>;
    bool getNativeCharacteristic(const QBluetoothUuid &serviceUuid,
        const QBluetoothUuid &charUuid,
        NativeCharacteristicCallback callback);

    void registerForValueChanges(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid);
    void unregisterFromValueChanges();
    void onValueChange(GattCharacteristic characteristic, GattValueChangedEventArgs const &args);
    void onMtuChange(GattSession session, winrt::IInspectable args);
    bool registerForMtuChanges();
    void unregisterFromMtuChanges();

    bool registerForStatusChanges();
    void unregisterFromStatusChanges();
    void onStatusChange(BluetoothLEDevice dev, winrt::IInspectable args);

    void obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer,
        GattDeviceService nativeService);
    void onServiceDiscoveryFinished(winrt::Windows::Foundation::IAsyncOperation<GattDeviceServicesResult> const &op,
                                    winrt::AsyncStatus status);

    void readCharacteristicHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                                  const QLowEnergyHandle charHandle,
                                  GattCharacteristic characteristic);
    void readDescriptorHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                              const QLowEnergyHandle charHandle,
                              const QLowEnergyHandle descriptorHandle,
                              GattCharacteristic characteristic);
    void writeCharacteristicHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                                   const QLowEnergyHandle charHandle, const QByteArray &newValue,
                                   bool writeWithResponse,
                                   GattCharacteristic characteristic);
    void writeDescriptorHelper(const QSharedPointer<QLowEnergyServicePrivate> service,
                               const QLowEnergyHandle charHandle,
                               const QLowEnergyHandle descriptorHandle,
                               const QByteArray &newValue,
                               GattCharacteristic characteristic);
    void discoverServiceDetailsHelper(const QBluetoothUuid &service,
                                      QLowEnergyService::DiscoveryMode mode,
                                      GattDeviceService deviceService);

    void clearAllServices();
    void closeAndRemoveService(const QBluetoothUuid &uuid);
};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLERPRIVATEWINRT_P_H
