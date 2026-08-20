// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTPERIPHERALMANAGER_P_H
#define BTPERIPHERALMANAGER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of internal files. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "btutility_p.h"

#include "qlowenergyserviceprivate_p.h"
#include "qbluetooth.h"

#include <QtCore/qsharedpointer.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qglobal.h>
#include <QtCore/qpair.h>
#include <QtCore/qmap.h>

#include <QtCore/private/qcore_mac_p.h>

#include <vector>
#include <deque>
#include <map>

#include <Foundation/Foundation.h>

#include <CoreBluetooth/CoreBluetooth.h>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingParameters;
class QLowEnergyAdvertisingData;
class QLowEnergyServiceData;

namespace DarwinBluetooth
{

class LECBManagerNotifier;

} // namespace DarwinBluetooth

using namespace DarwinBluetooth;


template<class Type>
using GenericLEMap = QMap<QLowEnergyHandle, Type>;

enum class PeripheralState
{
    idle,
    waitingForPowerOn,
    advertising,
    connected
};

struct UpdateRequest
{
    UpdateRequest() = default;
    UpdateRequest(QLowEnergyHandle handle, const ObjCStrongReference<NSData> &val)
        : charHandle(handle),
          value(val)
    {
    }

    QLowEnergyHandle charHandle = {};
    ObjCStrongReference<NSData> value;
};

QT_END_NAMESPACE

// Exposing names in a header is ugly, but constant QT_PREPEND_NAMESPACE is even worse ...
// After all, this header is to be included only in its own and controller's *.mm files.

QT_USE_NAMESPACE

using ValueRange = QPair<NSUInteger, NSUInteger>;

@interface QT_MANGLE_NAMESPACE(DarwinBTPeripheralManager) : NSObject<CBPeripheralManagerDelegate>

- (id)initWith:(LECBManagerNotifier *)notifier;
- (void)dealloc;

- (QSharedPointer<QLowEnergyServicePrivate>)addService:(const QLowEnergyServiceData &)data;
- (void) setParameters:(const QLowEnergyAdvertisingParameters &)parameters
         data:(const QLowEnergyAdvertisingData &)data
         scanResponse:(const QLowEnergyAdvertisingData &)scanResponse;

// To be executed on the Qt's special BTLE dispatch queue.
- (void)startAdvertising;
- (void)stopAdvertising;
- (void)detach;

- (void)write:(const QByteArray &)value
        charHandle:(QLowEnergyHandle)charHandle;


// CBPeripheralManagerDelegate's callbacks (BTLE queue).
- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral;
- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral
        error:(NSError *)error;
- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didAddService:(CBService *)service error:(NSError *)error;
- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central
        didSubscribeToCharacteristic:(CBCharacteristic *)characteristic;
- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central
        didUnsubscribeFromCharacteristic:(CBCharacteristic *)characteristic;
- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didReceiveReadRequest:(CBATTRequest *)request;
- (void)peripheralManager:(CBPeripheralManager *)peripheral
        didReceiveWriteRequests:(NSArray *)requests;
- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager *)peripheral;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTPeripheralManager);

#endif
