// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTLEDEVICEINQUIRY_P_H
#define BTLEDEVICEINQUIRY_P_H

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

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdeviceinfo.h"
#include "btgcdtimer_p.h"
#include "btutility_p.h"

#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

#include <CoreBluetooth/CoreBluetooth.h>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;

namespace DarwinBluetooth
{

class LECBManagerNotifier;

} // namespace DarwinBluetooth

QT_END_NAMESPACE

using QT_PREPEND_NAMESPACE(DarwinBluetooth)::LECBManagerNotifier;
using QT_PREPEND_NAMESPACE(DarwinBluetooth)::ObjCScopedPointer;

enum LEInquiryState
{
    InquiryStarting,
    InquiryActive,
    InquiryFinished,
    InquiryCancelled,
    ErrorPoweredOff,
    ErrorLENotSupported,
    ErrorNotAuthorized
};

@interface QT_MANGLE_NAMESPACE(DarwinBTLEDeviceInquiry) : NSObject<CBCentralManagerDelegate, QT_MANGLE_NAMESPACE(GCDTimerDelegate)>
- (id)initWithNotifier:(LECBManagerNotifier *)aNotifier;
- (void)dealloc;

// IMPORTANT: both 'startWithTimeout' and 'stop' MUST be executed on the "Qt's
// LE queue".
- (void)startWithTimeout:(int)timeout;
- (void)stop;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTLEDeviceInquiry);

#endif
