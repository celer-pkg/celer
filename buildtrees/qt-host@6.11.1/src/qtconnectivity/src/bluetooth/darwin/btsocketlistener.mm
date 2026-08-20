// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btsocketlistener_p.h"
#include "btdelegates_p.h"
#include "btutility_p.h"

#include <QtCore/qdebug.h>

QT_USE_NAMESPACE

@implementation DarwinBTSocketListener
{
    IOBluetoothUserNotification *connectionNotification;
    QT_PREPEND_NAMESPACE(DarwinBluetooth::SocketListener) *delegate;
    quint16 port;
}

- (id)initWithListener:(DarwinBluetooth::SocketListener *)aDelegate
{
    Q_ASSERT_X(aDelegate, Q_FUNC_INFO, "invalid delegate (null)");
    if (self = [super init]) {
        connectionNotification = nil;
        delegate = aDelegate;
        port = 0;
    }

    return self;
}

- (void)dealloc
{
    [connectionNotification unregister];
    [connectionNotification release];

    [super dealloc];
}

- (bool)listenRFCOMMConnectionsWithChannelID:(BluetoothRFCOMMChannelID)channelID
{
    Q_ASSERT_X(!connectionNotification, Q_FUNC_INFO, "already listening");

    connectionNotification = [IOBluetoothRFCOMMChannel  registerForChannelOpenNotifications:self
                                                        selector:@selector(rfcommOpenNotification:channel:)
                                                        withChannelID:channelID
                                                        direction:kIOBluetoothUserNotificationChannelDirectionIncoming];
    connectionNotification = [connectionNotification retain];
    if (connectionNotification)
        port = channelID;

    return connectionNotification;
}

- (bool)listenL2CAPConnectionsWithPSM:(BluetoothL2CAPPSM)psm
{
    Q_ASSERT_X(!connectionNotification, Q_FUNC_INFO, "already listening");

    connectionNotification = [IOBluetoothL2CAPChannel registerForChannelOpenNotifications:self
                                                      selector:@selector(l2capOpenNotification:channel:)
                                                      withPSM:psm
                                                      direction:kIOBluetoothUserNotificationChannelDirectionIncoming];
    connectionNotification = [connectionNotification retain];
    if (connectionNotification)
        port = psm;

    return connectionNotification;
}

- (void)rfcommOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothRFCOMMChannel *)newChannel
{
    Q_UNUSED(notification);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    delegate->openNotifyRFCOMM(newChannel);
}

- (void)l2capOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothL2CAPChannel *)newChannel
{
    Q_UNUSED(notification);

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");
    delegate->openNotifyL2CAP(newChannel);
}

- (quint16)port
{
    return port;
}

@end
