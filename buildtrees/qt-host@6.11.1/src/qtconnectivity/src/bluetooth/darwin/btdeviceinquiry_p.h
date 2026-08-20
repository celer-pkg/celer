// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTDEVICEINQUIRY_P_H
#define BTDEVICEINQUIRY_P_H

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

#include "btdelegates_p.h"

#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>
#include <IOKit/IOReturn.h>

#include <IOBluetooth/IOBluetooth.h>

@interface QT_MANGLE_NAMESPACE(DarwinBTClassicDeviceInquiry) : NSObject<IOBluetoothDeviceInquiryDelegate>

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::DeviceInquiryDelegate) *)delegate;
- (void)dealloc;

- (bool)isActive;
- (IOReturn)start;
- (IOReturn)stop;

//Obj-C delegate:
- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender
        error:(IOReturn)error aborted:(BOOL)aborted;

- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender
        device:(IOBluetoothDevice *)device;

- (void)deviceInquiryStarted:(IOBluetoothDeviceInquiry *)sender;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTClassicDeviceInquiry);

#endif
