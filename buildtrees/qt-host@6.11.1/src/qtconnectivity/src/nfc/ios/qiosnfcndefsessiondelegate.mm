// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qiosnfcndefsessiondelegate_p.h"
#include "qiosndefnotifier_p.h"

#include "qndefmessage.h"

#include <QtCore/qscopeguard.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>

#include <memory>

QT_BEGIN_NAMESPACE

dispatch_queue_t qt_Nfc_Queue()
{
    static dispatch_queue_t nfcQueue = []{
        auto queue = dispatch_queue_create("qt-NFC-queue", DISPATCH_QUEUE_SERIAL);
        if (!queue)
            qCWarning(QT_IOS_NFC, "Failed to create the QtNfc's dispatch queue");
        return queue;
    }();
    static const auto queueGuard = qScopeGuard([]{
        if (nfcQueue)
            dispatch_release(nfcQueue);
    });
    return nfcQueue;
}

QT_END_NAMESPACE

QT_USE_NAMESPACE

@implementation QIosNfcNdefSessionDelegate
{
    std::unique_ptr<QNfcNdefNotifier> notifier;
    QString alertMessage;
    NFCNDEFStatus tagStatus;
    NSUInteger capacity;
    QNearFieldTarget::RequestId requestId;
}

-(instancetype)initWithNotifier:(QNfcNdefNotifier *)aNotifier
{
    Q_ASSERT(aNotifier);

    if (self = [super init]) {
        auto queue = qt_Nfc_Queue();
        if (!queue)
            return self;

        tagStatus = NFCNDEFStatusNotSupported;
        capacity = 0;
        notifier.reset(aNotifier);
    }

    return self;
}

-(void)dealloc
{
    [self abort];
    [super dealloc];
}

-(QNfcNdefNotifier *)ndefNotifier
{
    return notifier.get();
}

-(void)abort
{
    notifier.reset(nullptr);
    [self reset];
}

-(bool)startSession
{
    if (self.session)
        return true;

    auto queue = qt_Nfc_Queue();
    Q_ASSERT(queue);
    self.session = [[NFCNDEFReaderSession alloc] initWithDelegate:self queue:queue invalidateAfterFirstRead:NO];
    if (alertMessage.size())
        self.session.alertMessage = alertMessage.toNSString();

    if (!self.session)
        return false;

    qCDebug(QT_IOS_NFC, "Starting NFC NDEF reader session");
    [self.session beginSession];
    return true;
}

-(void)reset
{
    self.session = nil; // Strong property, releases.
    self.ndefTag = nil; // Strong property, releases.
    requestId = {};
    tagStatus = NFCNDEFStatusNotSupported;
    capacity = 0;
}

-(void)stopSession:(const QString &)message
{
    if (!self.session)
        return;

    if (self.ndefTag && notifier.get())
        emit notifier->tagLost(self.ndefTag);

    if (message.size())
        [self.session invalidateSessionWithErrorMessage:message.toNSString()];
    else
        [self.session invalidateSession];

    [self reset];
}

-(void)setAlertMessage:(const QString &)message
{
    alertMessage = message;
}

-(void)readerSession:(NFCNDEFReaderSession *)session
       didInvalidateWithError:(NSError *)error
{
    if (session != self.session) // If we stopped the session, this maybe the case.
        return;

    if (!notifier.get()) // Aborted.
        return;

    NSLog(@"session did invalidate with error %@", error);

    if (error.code != NFCReaderSessionInvalidationErrorUserCanceled && error.code != NFCReaderErrorUnsupportedFeature) {
        if (self.ndefTag)
            emit notifier->tagError(QNearFieldTarget::TimeoutError, {});

        emit notifier->invalidateWithError(true);
        [self reset];
    }

    // Native errors:
    //
    // NFCReaderErrorRadioDisabled
    // NFCReaderErrorUnsupportedFeature
    // NFCReaderErrorSecurityViolation
    // NFCReaderErrorInvalidParameter
    // NFCReaderErrorParameterOutOfBound
    // NFCReaderErrorInvalidParameterLength
    // NFCReaderTransceiveErrorTagConnectionLost
    // NFCReaderTransceiveErrorRetryExceeded
    // NFCReaderTransceiveErrorSessionInvalidated
    // NFCReaderTransceiveErrorTagNotConnected
    // NFCReaderTransceiveErrorPacketTooLong
    // NFCReaderSessionInvalidationErrorUserCanceled
    // NFCReaderSessionInvalidationErrorSessionTimeout
    // NFCReaderSessionInvalidationErrorSessionTerminatedUnexpectedly
    // NFCReaderSessionInvalidationErrorSystemIsBusy
    // NFCReaderSessionInvalidationErrorFirstNDEFTagRead
    // NFCTagCommandConfigurationErrorInvalidParameters
    // NFCNdefReaderSessionErrorTagNotWritable
    // NFCNdefReaderSessionErrorTagUpdateFailure
    // NFCNdefReaderSessionErrorTagSizeTooSmall
    //NFCNdefReaderSessionErrorZeroLengthMessage

    // And these are what Qt has ...
    /*
    enum Error {
        NoError,
        UnknownError,
        UnsupportedError,
        TargetOutOfRangeError,
        NoResponseError,
        ChecksumMismatchError,
        InvalidParametersError,
        ConnectionError,
        NdefReadError,
        NdefWriteError,
        CommandError,
        TimeoutError
    };
    */
    // TODO: try to map those native errors to Qt ones ...
}

-(void)readerSession:(NFCNDEFReaderSession *)session
       didDetectNDEFs:(NSArray<NFCNDEFMessage *> *)messages
{
    Q_UNUSED(session);
    Q_UNUSED(messages);
    // It's intentionally a noop and should never be called, because
    // we implement the other method, giving us access to a tag.
    Q_UNREACHABLE();
}

-(void)restartPolling
{
    if (!self.session)
        return;

    auto queue = qt_Nfc_Queue();
    Q_ASSERT(queue);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                 int64_t(100./1000. * NSEC_PER_SEC)),//100 ms
                                 queue,
                                 ^{
                                       [self.session restartPolling];
                                  });
}

-(void)tag:(id<NFCNDEFTag>)tag didUpdateNDEFStatus:(NFCNDEFStatus)status
       capacity:(NSUInteger)aCapacity error:(NSError *)error
{
    if (!notifier.get()) // Aborted.
        return;

    if (tag != self.ndefTag)
        return;

    if (error) {
        NSLog(@"Querying NDEF tag's status failed: %@, restarting polling ...", error);
        self.ndefTag = nil;
        return [self restartPolling];
    }

    tagStatus = status;
    capacity = aCapacity;

    if (status == NFCNDEFStatusNotSupported) {
        qCDebug(QT_IOS_NFC, "The discovered tag does not support NDEF.");
        return [self restartPolling];
    }

    if (status == NFCNDEFStatusReadWrite)
        qCDebug(QT_IOS_NFC, "NDEF read/write capable tag found");

    if (status == NFCNDEFStatusReadOnly)
        qCDebug(QT_IOS_NFC, "The discovered tag is read only");

    qCInfo(QT_IOS_NFC) << "The max message size for the tag is:" << capacity;

    [self.session connectToTag:self.ndefTag completionHandler:^(NSError * _Nullable error) {
        if (!error) {
            if (notifier.get())
                emit notifier->tagDetected(self.ndefTag);
        } else {
            NSLog(@"Failed to connect to NDEF-capable tag, error: %@", error);
            [self restartPolling];
        }
    }];
}


-(void)readerSession:(NFCNDEFReaderSession *)session
       didDetectTags:(NSArray<__kindof id<NFCNDEFTag>> *)tags
{
    if (!notifier.get())
        return; // Aborted by Qt.

    if (session != self.session) // We stopped _that_ session.
        return;

    if (tags.count != 1) {
        qCWarning(QT_IOS_NFC, "Unexpected number of NDEF tags, restarting ...");
        [self restartPolling];
        return;
    }

    NSLog(@"detected a tag! %@", tags[0]);

    id<NFCNDEFTag> tag = tags[0];
    self.ndefTag = tag; // Strong reference, retains.
    tagStatus = NFCNDEFStatusNotSupported;
    capacity = 0;

    [self.ndefTag queryNDEFStatusWithCompletionHandler:
     ^(NFCNDEFStatus status, NSUInteger aCapacity, NSError * _Nullable error) {
        [self tag:tag didUpdateNDEFStatus:status capacity:aCapacity error:error];
    }];
}

-(void)readerSessionDidBecomeActive:(NFCNDEFReaderSession *)session
{
    if (session != self.session)
        return [session invalidateSession];

    qCInfo(QT_IOS_NFC, "session is active now");
}

@end

