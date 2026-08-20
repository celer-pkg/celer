// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BLUETOOTHMANAGEMENT_P_H
#define BLUETOOTHMANAGEMENT_P_H

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

#include <QtCore/qdatetime.h>
#include <QtCore/qmutex.h>
#include <QtCore/qobject.h>

#include <QtBluetooth/qbluetoothaddress.h>

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE qsizetype(16384)
#endif
#include "../qprivatelinearbuffer_p.h"

QT_BEGIN_NAMESPACE

class QSocketNotifier;

class BluetoothManagement : public QObject
{
    Q_OBJECT

public:

    enum class EventCode {
        CommandCompleteEvent = 0x0001,
        CommandStatusEvent = 0x0002,
        ControllerErrorEvent = 0x0003,
        IndexAddedEvent = 0x0004,
        IndexRemovedEvent = 0x0005,
        NewSettingsEvent = 0x0006,
        ClassOfDeviceChangedEvent = 0x0007,
        LocalNameChangedEvent = 0x0008,
        NewLinkKeyEvent = 0x0009,
        NewLongTermKeyEvent = 0x000A,
        DeviceConnectedEvent = 0x000B,
        DeviceDisconnectedEvent = 0x000C,
        ConnectFailedEvent = 0x000D,
        PINCodeRequestEvent = 0x000E,
        UserConfirmationRequestEvent = 0x000F,
        UserPasskeyRequestEvent = 0x0010,
        AuthenticationFailedEvent = 0x0011,
        DeviceFoundEvent = 0x0012,
        DiscoveringEvent = 0x0013,
        DeviceBlockedEvent = 0x0014,
        DeviceUnblockedEvent = 0x00150,
        DeviceUnpairedEvent = 0x0016,
        PasskeyNotifyEvent = 0x0017,
        NewIdentityResolvingKeyEvent = 0x0018,
        NewSignatureResolvingKeyEvent = 0x0019,
        DeviceAddedEvent = 0x001a,
        DeviceRemovedEvent = 0x001b,
        NewConnectionParameterEvent = 0x001c,
        UnconfiguredIndexAddedEvent = 0x001d,
        UnconfiguredIndexRemovedEvent = 0x001e,
        NewConfigurationOptionsEvent = 0x001f,
        ExtendedIndexAddedEvent = 0x0020,
        ExtendedIndexRemovedEvent = 0x0021,
        LocalOutOfBandExtendedDataUpdatedEvent = 0x0022,
        AdvertisingAddedEvent = 0x0023,
        AdvertisingRemovedEvent = 0x0024,
        ExtendedControllerInformationChangedEvent = 0x0025,
        PHYConfigurationChangedEvent = 0x0026,
        ExperimentalFeatureChangedEvent = 0x0027,
        DefaultSystemConfigurationChangedEvent = 0x0028,
        DefaultRuntimeConfigurationChangedEvent = 0x0029,
        DeviceFlagsChangedEvent = 0x002a,
        AdvertisementMonitorAddedEvent = 0x002b,
        AdvertisementMonitorRemovedEvent = 0x002c,
        ControllerSuspendEvent = 0x002d,
        ControllerResumeEvent = 0x002e
    };
    Q_ENUM(EventCode)

    explicit BluetoothManagement(QObject *parent = nullptr);
    static BluetoothManagement *instance();

    bool isAddressRandom(const QBluetoothAddress &address) const;
    bool isMonitoringEnabled() const;

private slots:
    void _q_readNotifier();
    void processRandomAddressFlagInformation(const QBluetoothAddress &address);
    void cleanupOldAddressFlags();

private:
    void readyRead();

    int fd = -1;
    QSocketNotifier* notifier;
    QPrivateLinearBuffer buffer;
    QHash<QBluetoothAddress, QDateTime> privateFlagAddresses;
    mutable QMutex accessLock;
};


QT_END_NAMESPACE

#endif // BLUETOOTHMANAGEMENT_P_H
