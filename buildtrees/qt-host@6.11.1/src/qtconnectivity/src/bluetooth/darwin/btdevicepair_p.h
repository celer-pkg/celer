// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTDEVICEPAIR_P_H
#define BTDEVICEPAIR_P_H

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

#include "qbluetoothaddress.h"
#include "btdelegates_p.h"
#include "btutility_p.h"

#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

ObjCStrongReference<IOBluetoothDevice> device_with_address(const QBluetoothAddress &address);

} // Namespace DarwinBluetooth.

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(DarwinBTClassicPairing) : NSObject<IOBluetoothDevicePairDelegate>

- (id)initWithTarget:(const QBluetoothAddress &)address
      delegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::PairingDelegate) *)object;

- (void)dealloc;

- (IOReturn)start;
- (bool)isActive;
- (void)stop;

- (const QBluetoothAddress &)targetAddress;
- (IOBluetoothDevicePair *)pairingRequest;
- (IOBluetoothDevice *)targetDevice;

// IOBluetoothDevicePairDelegate:

- (void)devicePairingStarted:(id)sender;
- (void)devicePairingConnecting:(id)sender;
- (void)deviceParingPINCodeRequest:(id)sender;

- (void)devicePairingUserConfirmationRequest:(id)sender
        numericValue:(BluetoothNumericValue)numericValue;

- (void)devicePairingUserPasskeyNotification:(id)sender
        passkey:(BluetoothPasskey)passkey;

- (void)devicePairingFinished:(id)sender
        error:(IOReturn)error;

- (void)deviceSimplePairingComplete:(id)sender
        status:(BluetoothHCIEventStatus)status;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTClassicPairing);

#endif
