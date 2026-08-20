// Copyright (C) 2016 Lorn Potter
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "iosmotionmanager.h"

#import <CoreMotion/CoreMotion.h>

static CMMotionManager *sharedManager = nil;

@implementation QIOSMotionManager

+ (CMMotionManager *)sharedManager
{
    static dispatch_once_t staticToken;
    dispatch_once(&staticToken, ^{
        sharedManager = [[CMMotionManager alloc] init];
        sharedManager.showsDeviceMovementDisplay = YES;
    });
    return sharedManager;
}

@end
