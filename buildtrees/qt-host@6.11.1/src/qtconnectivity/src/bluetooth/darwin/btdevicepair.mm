// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btdevicepair_p.h"
#include "btutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

ObjCStrongReference<IOBluetoothDevice> device_with_address(const QBluetoothAddress &address)
{
    if (address.isNull())
        return {};

    const BluetoothDeviceAddress &iobtAddress = iobluetooth_address(address);
    ObjCStrongReference<IOBluetoothDevice> res([IOBluetoothDevice deviceWithAddress:&iobtAddress], RetainPolicy::doInitialRetain);
    return res;
}

} // namespace DarwinBluetooth

QT_END_NAMESPACE

QT_USE_NAMESPACE

@implementation DarwinBTClassicPairing
{
    QT_PREPEND_NAMESPACE(QBluetoothAddress) m_targetAddress;

    bool m_active;
    IOBluetoothDevicePair *m_pairing; // The real pairing request
    QT_PREPEND_NAMESPACE(DarwinBluetooth)::PairingDelegate *m_object;
}

- (id)initWithTarget:(const QBluetoothAddress &)address
      delegate:(DarwinBluetooth::PairingDelegate *)object
{
    if (self = [super init]) {
        Q_ASSERT_X(!address.isNull(), Q_FUNC_INFO, "invalid target address");
        Q_ASSERT_X(object, Q_FUNC_INFO, "invalid delegate (null)");

        m_targetAddress = address;
        m_object = object;
        m_active = false;
    }

    return self;
}

- (void)dealloc
{
    [m_pairing stop];
    [m_pairing release];
    [super dealloc];
}

- (IOReturn) start
{
    if (m_active)
        return kIOReturnBusy;

    Q_ASSERT_X(!m_targetAddress.isNull(), Q_FUNC_INFO, "invalid target address");

    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothDeviceAddress &iobtAddress = DarwinBluetooth::iobluetooth_address(m_targetAddress);
    // Device is autoreleased.
    IOBluetoothDevice *const device = [IOBluetoothDevice deviceWithAddress:&iobtAddress];
    if (!device) {
        qCCritical(QT_BT_DARWIN) << "failed to create a device to pair with";
        return kIOReturnError;
    }

    m_pairing = [[IOBluetoothDevicePair pairWithDevice:device] retain];
    if (!m_pairing) {
        qCCritical(QT_BT_DARWIN) << "failed to create a device pair";
        return kIOReturnError;
    }

    [m_pairing setDelegate:self];
    const IOReturn result = [m_pairing start];
    if (result != kIOReturnSuccess) {
        [m_pairing release];
        m_pairing = nil;
    } else
        m_active = true;

    return result;
}

- (bool)isActive
{
    return m_active;
}

- (void)stop
{
    // stop: stops pairing, removes the delegate
    // and disconnects if device was connected.
    if (m_pairing)
        [m_pairing stop];
}

- (const QBluetoothAddress &)targetAddress
{
    return m_targetAddress;
}

- (IOBluetoothDevicePair *)pairingRequest
{
    return [[m_pairing retain] autorelease];
}

- (IOBluetoothDevice *)targetDevice
{
    return [m_pairing device];//It's retained/autoreleased by pair.
}

// IOBluetoothDevicePairDelegate:

- (void)devicePairingStarted:(id)sender
{
    Q_UNUSED(sender);
}

- (void)devicePairingConnecting:(id)sender
{
    Q_UNUSED(sender);
}

- (void)deviceParingPINCodeRequest:(id)sender
{
    Q_UNUSED(sender);
}

- (void)devicePairingUserConfirmationRequest:(id)sender
        numericValue:(BluetoothNumericValue)numericValue
{
    if (sender != m_pairing) // Can never happen.
        return;

    Q_ASSERT_X(m_object, Q_FUNC_INFO, "invalid delegate (null)");

    m_object->requestUserConfirmation(self, numericValue);
}

- (void)devicePairingUserPasskeyNotification:(id)sender
        passkey:(BluetoothPasskey)passkey
{
    Q_UNUSED(sender);
    Q_UNUSED(passkey);
}

- (void)devicePairingFinished:(id)sender error:(IOReturn)error
{
    Q_ASSERT_X(m_object, Q_FUNC_INFO, "invalid delegate (null)");

    if (sender != m_pairing) // Can never happen though.
        return;

    m_active = false;
    if (error != kIOReturnSuccess)
        m_object->error(self, error);
    else
        m_object->pairingFinished(self);
}

- (void)deviceSimplePairingComplete:(id)sender
        status:(BluetoothHCIEventStatus)status
{
    Q_UNUSED(sender);
    Q_UNUSED(status);
}

@end
