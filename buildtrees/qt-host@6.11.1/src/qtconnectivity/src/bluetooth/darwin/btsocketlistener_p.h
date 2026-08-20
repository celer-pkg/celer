// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTSOCKETLISTENER_P_H
#define BTSOCKETLISTENER_P_H

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

namespace DarwinBluetooth {

class SocketListener;

}

QT_END_NAMESPACE

// A single DarwinBTSocketListener can be started only once with
// RFCOMM or L2CAP protocol. It must be deleted to stop listening.

@interface QT_MANGLE_NAMESPACE(DarwinBTSocketListener) : NSObject

- (id)initWithListener:(QT_PREPEND_NAMESPACE(DarwinBluetooth::SocketListener) *)aDelegate;
- (void)dealloc;

- (bool)listenRFCOMMConnectionsWithChannelID:(BluetoothRFCOMMChannelID)channelID;
- (bool)listenL2CAPConnectionsWithPSM:(BluetoothL2CAPPSM)psm;

- (void)rfcommOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothRFCOMMChannel *)newChannel;

- (void)l2capOpenNotification:(IOBluetoothUserNotification *)notification
        channel:(IOBluetoothL2CAPChannel *)newChannel;

- (quint16)port;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTSocketListener);

#endif
