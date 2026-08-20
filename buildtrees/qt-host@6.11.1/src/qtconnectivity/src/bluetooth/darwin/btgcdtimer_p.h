// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTGCDTIMER_P_H
#define BTGCDTIMER_P_H

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

#include "btutility_p.h"

#include <QtCore/private/qcore_mac_p.h>

#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

enum class OperationTimeout
{
    none,
    serviceDiscovery,
    includedServicesDiscovery,
    characteristicsDiscovery,
    characteristicRead,
    descriptorsDiscovery,
    descriptorRead,
    characteristicWrite
};

} // namespace DarwinBluetooth

QT_END_NAMESPACE

@protocol QT_MANGLE_NAMESPACE(GCDTimerDelegate)
@required
- (void)timeout:(id)sender;
@end

@interface QT_MANGLE_NAMESPACE(DarwinBTGCDTimer) : NSObject
- (instancetype)initWithDelegate:(id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)>)delegate;
- (void)watchAfter:(id)object withTimeoutType:(QT_PREPEND_NAMESPACE(DarwinBluetooth)::OperationTimeout)type;
- (void)startWithTimeout:(qint64)ms step:(qint64)stepMS;
- (void)handleTimeout;
- (void)cancelTimer;
- (id)objectUnderWatch;
- (QT_PREPEND_NAMESPACE(DarwinBluetooth)::OperationTimeout)timeoutType;
@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(DarwinBTGCDTimer);

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

using GCDTimer = ObjCStrongReference<DarwinBTGCDTimer>;

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif // BTGCDTIMER_P_H

