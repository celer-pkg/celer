// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTL2CAPCHANNEL_P_H
#define BTL2CAPCHANNEL_P_H

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

#include <QtCore/private/qcore_mac_p.h>
#include <QtCore/private/qglobal_p.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

#include <cstddef>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

namespace DarwinBluetooth {

class ChannelDelegate;

}

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTL2CAPChannel) : NSObject<IOBluetoothL2CAPChannelDelegate>

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::ChannelDelegate *)aDelegate;
- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::ChannelDelegate *)aDelegate
      channel:(IOBluetoothL2CAPChannel *)aChannel;

- (void)dealloc;

// Async. connection (connect can be called only once).
- (IOReturn)connectAsyncToDevice:(const QT_PREPEND_NAMESPACE(QBluetoothAddress) &)address
            withPSM:(BluetoothL2CAPChannelID)psm;

// IOBluetoothL2CAPChannelDelegate:
- (void)l2capChannelData:(IOBluetoothL2CAPChannel*)l2capChannel
        data:(void *)dataPointer length:(size_t)dataLength;
- (void)l2capChannelOpenComplete:(IOBluetoothL2CAPChannel*)
        l2capChannel status:(IOReturn)error;
- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel*)l2capChannel;
- (void)l2capChannelReconfigured:(IOBluetoothL2CAPChannel*)l2capChannel;
- (void)l2capChannelWriteComplete:(IOBluetoothL2CAPChannel*)l2capChannel
        refcon:(void*)refcon status:(IOReturn)error;
- (void)l2capChannelQueueSpaceAvailable:(IOBluetoothL2CAPChannel*)l2capChannel;

//
- (BluetoothL2CAPPSM)getPSM;
- (BluetoothDeviceAddress)peerAddress;
- (NSString *)peerName;
- (bool)isConnected;

// Writes the given data synchronously over the target L2CAP channel to the remote
// device.
// The length of the data may not exceed the L2CAP channel's outgoing MTU.
// This method will block until the data has been successfully sent to the
// hardware for transmission (or an error occurs).
- (IOReturn) writeSync:(void*)data length:(UInt16)length;

// The length of the data may not exceed the L2CAP channel's outgoing MTU.
// When the data has been successfully passed to the hardware to be transmitted,
// the delegate method -l2capChannelWriteComplete:refcon:status: will be called.
// Returns kIOReturnSuccess if the data was buffered successfully.
- (IOReturn) writeAsync:(void*)data length:(UInt16)length;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTL2CAPChannel);

#endif
