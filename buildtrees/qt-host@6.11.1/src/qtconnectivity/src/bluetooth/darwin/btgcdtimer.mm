// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "btgcdtimer_p.h"
#include "btutility_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qdebug.h>

#include <algorithm>

QT_USE_NAMESPACE
using namespace DarwinBluetooth;

@implementation DarwinBTGCDTimer {
@private
    qint64 timeoutMS;
    qint64 timeoutStepMS;

    // Optional:
    id objectUnderWatch;
    OperationTimeout timeoutType;

    QElapsedTimer timer;
    id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)> timeoutHandler;

    bool cancelled;
}

- (instancetype)initWithDelegate:(id<QT_MANGLE_NAMESPACE(GCDTimerDelegate)>)delegate
{
    if (self = [super init]) {
        timeoutHandler = delegate;
        timeoutMS = 0;
        timeoutStepMS = 0;
        objectUnderWatch = nil;
        timeoutType  = OperationTimeout::none;
        cancelled = false;
    }
    return self;
}

- (void)watchAfter:(id)object withTimeoutType:(OperationTimeout)type
{
    objectUnderWatch = object;
    timeoutType = type;
}

- (void)startWithTimeout:(qint64)ms step:(qint64)stepMS
{
    Q_ASSERT(!timeoutMS && !timeoutStepMS);
    Q_ASSERT(!cancelled);

    if (!timeoutHandler) {
        // Nobody to report timeout to, no need to start any task then.
        return;
    }

    if (ms <= 0 || stepMS <= 0) {
        qCWarning(QT_BT_DARWIN, "Invalid timeout/step parameters");
        return;
    }

    timeoutMS = ms;
    timeoutStepMS = stepMS;
    timer.start();

    [self handleTimeout];
}

- (void)handleTimeout
{
    if (cancelled)
        return;

    const qint64 elapsed = timer.elapsed();
    if (elapsed >= timeoutMS) {
        [timeoutHandler timeout:self];
    } else {
        // Re-schedule:
        dispatch_queue_t leQueue(qt_LE_queue());
        Q_ASSERT(leQueue);
        const qint64 timeChunkMS = std::min(timeoutMS - elapsed, timeoutStepMS);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                     int64_t(timeChunkMS / 1000. * NSEC_PER_SEC)),
                                     leQueue,
                                     ^{
                                           [self handleTimeout];
                                      });
    }
}

- (void)cancelTimer
{
    cancelled = true;
    timeoutHandler = nil;
    objectUnderWatch = nil;
    timeoutType = OperationTimeout::none;
}

- (id)objectUnderWatch
{
    return objectUnderWatch;
}

- (OperationTimeout)timeoutType
{
    return timeoutType;
}

@end
