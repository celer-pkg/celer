// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTRFCOMMCHANNEL_P_H
#define BTRFCOMMCHANNEL_P_H

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

#include <QtCore/private/qglobal_p.h>
#include <QtCore/private/qcore_mac_p.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

namespace DarwinBluetooth {

class ChannelDelegate;

}

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTRFCOMMChannel) : NSObject<IOBluetoothRFCOMMChannelDelegate>

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::ChannelDelegate *)aDelegate;
- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::ChannelDelegate *)aDelegate
      channel:(IOBluetoothRFCOMMChannel *)aChannel;

- (void)dealloc;

// A single async connection (can connect only once).
- (IOReturn)connectAsyncToDevice:(const QT_PREPEND_NAMESPACE(QBluetoothAddress) &)address
            withChannelID:(BluetoothRFCOMMChannelID)channelID;

- (void)rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel
        data:(void *)dataPointer length:(size_t)dataLength;
- (void)rfcommChannelOpenComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel
        status:(IOReturn)error;
- (void)rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel;
- (void)rfcommChannelControlSignalsChanged:(IOBluetoothRFCOMMChannel*)rfcommChannel;
- (void)rfcommChannelFlowControlChanged:(IOBluetoothRFCOMMChannel*)rfcommChannel;
- (void)rfcommChannelWriteComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel
        refcon:(void*)refcon status:(IOReturn)error;
- (void)rfcommChannelQueueSpaceAvailable:(IOBluetoothRFCOMMChannel*)rfcommChannel;

//
- (BluetoothRFCOMMChannelID)getChannelID;
- (BluetoothDeviceAddress)peerAddress;
- (NSString *)peerName;

- (BluetoothRFCOMMMTU)getMTU;

- (IOReturn) writeSync:(void*)data length:(UInt16)length;
- (IOReturn) writeAsync:(void*)data length:(UInt16)length;

- (void)handleChannelOpenTimeout;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTRFCOMMChannel);

#endif
