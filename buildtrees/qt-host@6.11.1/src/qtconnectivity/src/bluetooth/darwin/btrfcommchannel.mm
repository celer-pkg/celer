// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btrfcommchannel_p.h"
#include "qbluetoothaddress.h"
#include "btdelegates_p.h"
#include "btutility_p.h"

#include <QtCore/qtimer.h>

QT_USE_NAMESPACE

namespace {

static constexpr auto channelOpenTimeoutMs = std::chrono::milliseconds{20000};

} // namespace

@implementation DarwinBTRFCOMMChannel
{
    QT_PREPEND_NAMESPACE(DarwinBluetooth)::ChannelDelegate *delegate;
    IOBluetoothDevice *device;
    IOBluetoothRFCOMMChannel *channel;
    bool connected;
    std::unique_ptr<QTimer> channelOpenTimer;
}

- (id)initWithDelegate:(DarwinBluetooth::ChannelDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, Q_FUNC_INFO, "invalid delegate (null)");

    if (self = [super init]) {
        delegate = aDelegate;
        device = nil;
        channel = nil;
        connected = false;
    }

    return self;
}

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::ChannelDelegate) *)aDelegate
      channel:(IOBluetoothRFCOMMChannel *)aChannel
{
    // This type of channel does not require connect, it's created with
    // already open channel.
    Q_ASSERT_X(aDelegate, Q_FUNC_INFO, "invalid delegate (null)");
    Q_ASSERT_X(aChannel, Q_FUNC_INFO, "invalid channel (nil)");

    if (self = [super init]) {
        delegate = aDelegate;
        channel = [aChannel retain];
        [channel setDelegate:self];
        device = [[channel getDevice] retain];
        connected = true;
    }

    return self;
}

- (void)dealloc
{
    if (channel) {
        [channel setDelegate:nil];
        [channel closeChannel];
        [channel release];
    }

    [device release];

    [super dealloc];
}

// A single async connection (you can not reuse this object).
- (IOReturn)connectAsyncToDevice:(const QBluetoothAddress &)address
            withChannelID:(BluetoothRFCOMMChannelID)channelID
{
    if (address.isNull()) {
        qCCritical(QT_BT_DARWIN) << "invalid peer address";
        return kIOReturnNoDevice;
    }

    // Can never be called twice.
    if (connected || device || channel) {
        qCCritical(QT_BT_DARWIN) << "connection is already active";
        return kIOReturnStillOpen;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothDeviceAddress iobtAddress = DarwinBluetooth::iobluetooth_address(address);
    device = [IOBluetoothDevice deviceWithAddress:&iobtAddress];
    if (!device) { // TODO: do I always check this BTW??? Apple's docs say nothing about nil.
        qCCritical(QT_BT_DARWIN) << "failed to create a device";
        return kIOReturnNoDevice;
    }

    const IOReturn status = [device openRFCOMMChannelAsync:&channel
                             withChannelID:channelID delegate:self];
    if (status != kIOReturnSuccess) {
        qCCritical(QT_BT_DARWIN) << "failed to open RFCOMM channel";
        // device is still autoreleased.
        device = nil;
        return status;
    }

    if (!channelOpenTimer) {
        channelOpenTimer.reset(new QTimer);
        QObject::connect(channelOpenTimer.get(), &QTimer::timeout,
                         channelOpenTimer.get(), [self]() {
            qCDebug(QT_BT_DARWIN) << "Could not open the RFCOMM channel within the specified "
                                     "timeout. Assuming that the remote device is not available.";
            [self handleChannelOpenTimeout];
        });
        channelOpenTimer->setSingleShot(true);
    }
    channelOpenTimer->start(channelOpenTimeoutMs);

    [channel retain];// What if we're closed already?
    [device retain];

    return kIOReturnSuccess;
}

- (void)rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel
        data:(void *)dataPointer length:(size_t)dataLength
{
    Q_UNUSED(rfcommChannel);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    // Not sure if it can ever happen and if
    // assert is better.
    if (!dataPointer || !dataLength)
        return;

    delegate->readChannelData(dataPointer, dataLength);
}

- (void)rfcommChannelOpenComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel
        status:(IOReturn)error
{
    Q_UNUSED(rfcommChannel);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    // Timer can be null, if we're running as a server, and the channel is
    // opened as a result of a client connection
    if (channelOpenTimer)
        channelOpenTimer->stop();
    if (error != kIOReturnSuccess) {
        delegate->setChannelError(error);
        delegate->channelClosed();
    } else {
        connected = true;
        delegate->channelOpenComplete();
    }
}

- (void)rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
    Q_UNUSED(rfcommChannel);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    delegate->channelClosed();
    connected = false;
}

- (void)rfcommChannelControlSignalsChanged:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
    Q_UNUSED(rfcommChannel);
}

- (void)rfcommChannelFlowControlChanged:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
    Q_UNUSED(rfcommChannel);
}

- (void)rfcommChannelWriteComplete:(IOBluetoothRFCOMMChannel*)rfcommChannel
        refcon:(void*)refcon status:(IOReturn)error
{
    Q_UNUSED(rfcommChannel);
    Q_UNUSED(refcon);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    if (error != kIOReturnSuccess)
        delegate->setChannelError(error);
    else
        delegate->writeComplete();
}

- (void)rfcommChannelQueueSpaceAvailable:(IOBluetoothRFCOMMChannel*)rfcommChannel
{
    Q_UNUSED(rfcommChannel);
}

- (BluetoothRFCOMMChannelID)getChannelID
{
    if (channel)
        return [channel getChannelID];

    return 0;
}

- (BluetoothDeviceAddress)peerAddress
{
    const BluetoothDeviceAddress *const addr = device ? [device getAddress]
                                                      : nullptr;
    if (addr)
        return *addr;

    return BluetoothDeviceAddress();
}

- (NSString *)peerName
{
    if (device)
        return device.name;

    return nil;
}

- (BluetoothRFCOMMMTU)getMTU
{
    if (channel)
        return [channel getMTU];

    return 0;
}

- (IOReturn) writeSync:(void*)data length:(UInt16)length
{
    Q_ASSERT_X(data, Q_FUNC_INFO, "invalid data (null)");
    Q_ASSERT_X(length, Q_FUNC_INFO, "invalid data size");
    Q_ASSERT_X(connected && channel, Q_FUNC_INFO, "invalid RFCOMM channel");

    return [channel writeSync:data length:length];
}

- (IOReturn) writeAsync:(void*)data length:(UInt16)length
{
    Q_ASSERT_X(data, Q_FUNC_INFO, "invalid data (null)");
    Q_ASSERT_X(length, Q_FUNC_INFO, "invalid data size");
    Q_ASSERT_X(connected && channel, Q_FUNC_INFO, "invalid RFCOMM channel");

    return [channel writeAsync:data length:length refcon:nullptr];
}

- (void)handleChannelOpenTimeout
{
    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    Q_ASSERT_X(channel, Q_FUNC_INFO, "invalid RFCOMM channel");

    delegate->setChannelError(kIOReturnNotOpen);
    [channel closeChannel];
    delegate->channelClosed();
}

@end
