// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QIOSTAGREADERDELEGATE_P_H
#define QIOSTAGREADERDELEGATE_P_H

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

#include <QString>

#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>
#import <Foundation/Foundation.h>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateImpl;

QT_END_NAMESPACE

API_AVAILABLE(ios(13.0))
@interface QT_MANGLE_NAMESPACE(QIosTagReaderDelegate)
    : NSObject<NFCTagReaderSessionDelegate>

@property QNearFieldManagerPrivateImpl *listener;
@property bool sessionStoppedByApplication;
@property (nonatomic, strong) NSString *message;
@property (nonatomic, strong) NFCTagReaderSession *session;

- (instancetype)initWithListener:(QNearFieldManagerPrivateImpl *)listener;

- (void)startSession;
- (void)stopSession:(QString)message;

- (void)alertMessage:(QString)message;

@end

#endif // QIOSTAGREADERDELEGATE_P_H
