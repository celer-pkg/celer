// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTDELEGATES_P_H
#define BTDELEGATES_P_H

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

#if defined(Q_OS_MACOS)

#include <IOKit/IOReturn.h>

#include <cstddef>
#include <cstdint>

QT_BEGIN_NAMESPACE

class QBluetoothAddress;

namespace DarwinBluetooth {

class DeviceInquiryDelegate
{
public:
    virtual ~DeviceInquiryDelegate();

    virtual void inquiryFinished() = 0;
    virtual void error(IOReturn error) = 0;
    virtual void classicDeviceFound(void *ioBluetoothDevice) = 0;
};

class PairingDelegate
{
public:
    using BluetoothNumericValue = uint32_t;
    using BluetoothPasskey = BluetoothNumericValue;

    virtual ~PairingDelegate();

    virtual void connecting(void *pair) = 0;
    virtual void requestPIN(void *pair) = 0;
    virtual void requestUserConfirmation(void *pair,
                                         BluetoothNumericValue) = 0;
    virtual void passkeyNotification(void *pair,
                                     BluetoothPasskey passkey) = 0;
    virtual void error(void *pair, IOReturn errorCode) = 0;
    virtual void pairingFinished(void *pair) = 0;
};

class SDPInquiryDelegate {
public:
    virtual ~SDPInquiryDelegate();

    virtual void SDPInquiryFinished(void *ioBluetoothDevice) = 0;
    virtual void SDPInquiryError(void *ioBluetoothDevice, IOReturn errorCode) = 0;
};

// L2CAP and RFCOMM.
class ChannelDelegate
{
public:
    virtual ~ChannelDelegate();

    virtual void setChannelError(IOReturn errorCode) = 0;
    virtual void channelOpenComplete() = 0;
    virtual void channelClosed() = 0;

    virtual void readChannelData(void *data, std::size_t size) = 0;
    virtual void writeComplete() = 0;
};

class ConnectionMonitor {
public:
    virtual ~ConnectionMonitor();

    virtual void deviceConnected(const QBluetoothAddress &address) = 0;
    virtual void deviceDisconnected(const QBluetoothAddress &address) = 0;
};

class SocketListener
{
public:
    virtual ~SocketListener();

    virtual void openNotifyRFCOMM(void *rfcommChannel) = 0;
    virtual void openNotifyL2CAP(void *l2capChannel) = 0;
};


} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif // Q_OS_MACOS

#endif // DARWINBTDELEGATES_P_H
