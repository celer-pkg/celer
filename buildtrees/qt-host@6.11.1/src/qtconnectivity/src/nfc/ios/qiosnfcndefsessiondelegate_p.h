// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QIOSNFCNDEFSESSIONDELEFATE_P_H
#define QIOSNFCNDEFSESSIONDELEFATE_P_H

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

#include <QtCore/qglobal.h>

#include <QtCore/private/qcore_mac_p.h>

#include "qnearfieldmanager.h"
#include "qnearfieldtarget.h"

#include <Foundation/Foundation.h>
#include <CoreNFC/CoreNFC.h>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivateImpl;
class QNfcNdefNotifier;
class QNearFieldTarget;
class QNdefMessage;
class QString;

dispatch_queue_t qt_Nfc_Queue();

QT_END_NAMESPACE

QT_USE_NAMESPACE

@interface QT_MANGLE_NAMESPACE(QIosNfcNdefSessionDelegate) : NSObject<NFCNDEFReaderSessionDelegate>

@property (strong, nonatomic) NFCNDEFReaderSession *session;
@property (strong, nonatomic) id<NFCNDEFTag> ndefTag;


-(instancetype)initWithNotifier:(QNfcNdefNotifier *)aNotifier;
-(void)dealloc;

-(QNfcNdefNotifier *)ndefNotifier;

-(void)setAlertMessage:(const QString &)message;

// Those methods to be called on the session.sessionQueue:
-(bool)startSession;
-(void)stopSession:(const QString &)message;
-(void)abort;

// Delegate's methods, implementing the protocol NFCNDEFReaderSessionDelegate.
// Those methods not to be called by the Qt.

// "Gets called when a session becomes invalid.  At this point the client is expected to
// discard the returned session object."
-(void)readerSession:(NFCNDEFReaderSession *)session
       didInvalidateWithError:(NSError *)error;

// "Gets called when the reader detects NFC tag(s) with NDEF messages in the polling sequence.
// Polling is automatically restarted once the detected tag is removed from the reader's read
// range.  This method is only get call if the optional -readerSession:didDetectTags: method
// is not implemented."
-(void)readerSession:(NFCNDEFReaderSession *)session
       didDetectNDEFs:(NSArray<NFCNDEFMessage *> *)messages;

// "Gets called when the reader detects NDEF tag(s) in the RF field.  Presence of this method
// overrides -readerSession:didDetectNDEFs: and enables read-write capability for the session."
-(void)readerSession:(NFCNDEFReaderSession *)session
       didDetectTags:(NSArray<__kindof id<NFCNDEFTag>> *)tags;

// "Gets called when the NFC reader session has become active. RF is enabled and reader is
// scanning for tags."
-(void)readerSessionDidBecomeActive:(NFCNDEFReaderSession *)session;

@end

QT_NAMESPACE_ALIAS_OBJC_CLASS(QIosNfcNdefSessionDelegate);

#endif // QIOSNFCNDEFSESSIONDELEFATE_P_H
