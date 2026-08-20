// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTCENTRALMANAGER_P_H
#define BTCENTRALMANAGER_P_H

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

#include "qlowenergycontroller.h"
#include "qbluetoothuuid.h"
#include "btgcdtimer_p.h"
#include "btutility_p.h"

#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qglobal.h>
#include <QtCore/qqueue.h>
#include <QtCore/qhash.h>

#include <Foundation/Foundation.h>

#include <CoreBluetooth/CoreBluetooth.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

class LECBManagerNotifier;

enum CentralManagerState
{
    // QLowEnergyController already has some of these states,
    // but it's not enough and we need more special states here.
    CentralManagerIdle,
    // Required by CBCentralManager to avoid problems with dangled pointers.
    CentralManagerUpdating,
    CentralManagerConnecting,
    CentralManagerDiscovering,
    CentralManagerDisconnecting
};

// In Qt we work with handles and UUIDs. Core Bluetooth
// has NSArrays (and nested NSArrays inside servces/characteristics).
// To simplify a navigation, I need a simple way to map from a handle
// to a Core Bluetooth object. These are weak pointers,
// will probably require '__weak' with ARC.
typedef QHash<QLowEnergyHandle, CBService *> ServiceHash;
typedef QHash<QLowEnergyHandle, CBCharacteristic *> CharHash;
typedef QHash<QLowEnergyHandle, CBDescriptor *> DescHash;

// Descriptor/charactesirstic read/write requests
// - we have to serialize 'concurrent' requests.
struct LERequest {
    enum RequestType {
        CharRead,
        CharWrite,
        DescRead,
        DescWrite,
        ClientConfiguration,
        Invalid
    };

    LERequest() : type(Invalid),
                  withResponse(false),
                  handle(0)
    {}

    RequestType type;
    bool withResponse;
    QLowEnergyHandle handle;
    QByteArray value;
};

typedef QQueue<LERequest> RequestQueue;

// Core Bluetooth's write confirmation does not provide
// the updated value (characteristic or descriptor).
// But the Qt's Bluetooth API ('write with response')
// expects this updated value as a response, so we have
// to cache this write value and report it back.
// 'NSObject *' will require '__weak' with ARC.
typedef QHash<NSObject *, QByteArray> ValueHash;

} // namespace DarwinBluetooth

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTCentralManager) : NSObject<CBCentralManagerDelegate,
                                                                  CBPeripheralDelegate,
                                                                  QT_MANGLE_NAMESPACE(GCDTimerDelegate)>
- (id)initWith:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::LECBManagerNotifier *)notifier;
- (void)dealloc;

- (CBPeripheral *)peripheral;

// IMPORTANT: _all_ these methods are to be executed on qt_LE_queue,
// when passing parameters - C++ objects _must_ be copied (see the controller's code).
- (void)connectToDevice:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)aDeviceUuid;

- (void)disconnectFromDevice;

- (void)discoverServices;
- (void)discoverServiceDetails:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid
        readValues:(bool)read;

- (int)mtu;
- (void)readRssi;

- (void)setNotifyValue:(const QT_PREPEND_NAMESPACE(QByteArray) &)value
        forCharacteristic:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))charHandle
        onService:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid;

- (void)readCharacteristic:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))charHandle
        onService:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid;

- (void)write:(const QT_PREPEND_NAMESPACE(QByteArray) &)value
        charHandle:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))charHandle
        onService:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid
        withResponse:(bool)writeWithResponse;

- (void)readDescriptor:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))descHandle
        onService:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid;

- (void)write:(const QT_PREPEND_NAMESPACE(QByteArray) &)value
        descHandle:(QT_PREPEND_NAMESPACE(QLowEnergyHandle))descHandle
        onService:(const QT_PREPEND_NAMESPACE(QBluetoothUuid) &)serviceUuid;

- (void)detach;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTCentralManager);

#endif
