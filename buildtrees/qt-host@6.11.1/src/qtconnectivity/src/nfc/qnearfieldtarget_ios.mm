// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "ios/qiosnfcndefsessiondelegate_p.h"
#include "ios/qiosndefnotifier_p.h"

#include "qnearfieldtarget_ios_p.h"

#import <CoreNFC/NFCNDEFReaderSession.h>
#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>
#import <CoreNFC/NFCISO7816Tag.h>
#import <CoreNFC/NFCTag.h>

#include <QtCore/qapplicationstatic.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_APPLICATION_STATIC(ResponseProvider, responseProvider)

void ResponseProvider::provideResponse(QNearFieldTarget::RequestId requestId, QNearFieldTarget::Error error, QByteArray recvBuffer) {
    Q_EMIT responseReceived(requestId, error, recvBuffer);
}

void NfcDeleter::operator()(void *obj)
{
    id some = static_cast<id>(obj);

    if ([some conformsToProtocol:@protocol(NFCNDEFTag)])
        [static_cast<id<NFCNDEFTag>>(some) release];
    else if ([some conformsToProtocol:@protocol(NFCTag)])
        [static_cast<id<NFCTag>>(some) release];
    else
        Q_UNREACHABLE();
}

QNearFieldTargetPrivateImpl:: QNearFieldTargetPrivateImpl(void *tag, QObject *parent) :
    QNearFieldTargetPrivate(parent),
    nfcTag(tag)
{
    Q_ASSERT(nfcTag);

    QObject::connect(this, &QNearFieldTargetPrivate::error, this, &QNearFieldTargetPrivateImpl::onTargetError);
    QObject::connect(responseProvider, &ResponseProvider::responseReceived, this, &QNearFieldTargetPrivateImpl::onResponseReceived);
    QObject::connect(&targetCheckTimer, &QTimer::timeout, this, &QNearFieldTargetPrivateImpl::onTargetCheck);
    targetCheckTimer.start(500);
}

QNearFieldTargetPrivateImpl:: QNearFieldTargetPrivateImpl(void *delegate, void *tag, QObject *parent) :
    QNearFieldTargetPrivate(parent),
    nfcTag(tag)
{
    Q_ASSERT(delegate && tag);
    Q_ASSERT([id(tag) conformsToProtocol:@protocol(NFCNDEFTag)]);

    auto qtDelegate = static_cast<QIosNfcNdefSessionDelegate *>(sessionDelegate = delegate);
    notifier = [qtDelegate ndefNotifier];
    Q_ASSERT(notifier);

    // The 'notifier' lives on a (potentially different, unspecified) thread,
    // thus connection is 'queued'.
    QObject::connect(notifier, &QNfcNdefNotifier::tagError, this,
                     &QNearFieldTargetPrivate::error, Qt::QueuedConnection);

    QObject::connect(this, &QNearFieldTargetPrivate::error, this, &QNearFieldTargetPrivateImpl::onTargetError);
    QObject::connect(&targetCheckTimer, &QTimer::timeout, this, &QNearFieldTargetPrivateImpl::onTargetCheck);

    targetCheckTimer.start(500);
}

QNearFieldTargetPrivateImpl::~QNearFieldTargetPrivateImpl()
{
}

void QNearFieldTargetPrivateImpl::invalidate()
{
    queue.clear();
    ndefOperations.clear();

    if (isNdefTag()) {
        Q_ASSERT(notifier);

        QObject::disconnect(notifier, nullptr, this, nullptr);
        notifier = nullptr;
    }

    nfcTag.reset();
    sessionDelegate = nil;

    targetCheckTimer.stop();

    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT targetLost(this);
    }, Qt::QueuedConnection);
}

QByteArray QNearFieldTargetPrivateImpl::uid() const
{
    if (!nfcTag || isNdefTag()) // NFCNDEFTag does not have this information ...
        return {};

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        id<NFCISO7816Tag> iso7816Tag = tag.asNFCISO7816Tag;
        if (iso7816Tag)
            return QByteArray::fromNSData(iso7816Tag.identifier);
    }

    return {};
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::type() const
{
    if (!nfcTag || isNdefTag()) // No information provided by NFCNDEFTag.
        return QNearFieldTarget::ProprietaryTag;

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        id<NFCISO7816Tag> iso7816Tag = tag.asNFCISO7816Tag;

        if (tag.type != NFCTagTypeISO7816Compatible || iso7816Tag == nil)
             return QNearFieldTarget::ProprietaryTag;

        if (iso7816Tag.historicalBytes != nil && iso7816Tag.applicationData == nil)
            return QNearFieldTarget::NfcTagType4A;

        if (iso7816Tag.historicalBytes == nil && iso7816Tag.applicationData != nil)
            return QNearFieldTarget::NfcTagType4B;

        return QNearFieldTarget::NfcTagType4;
    }

    return QNearFieldTarget::ProprietaryTag;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivateImpl::accessMethods() const
{
    if (isNdefTag())
        return QNearFieldTarget::NdefAccess;

    if (@available(iOS 13, *)) {
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        if (tag && [tag conformsToProtocol:@protocol(NFCISO7816Tag)])
            return QNearFieldTarget::TagTypeSpecificAccess;
    }

    return QNearFieldTarget::UnknownAccess;
}

int QNearFieldTargetPrivateImpl::maxCommandLength() const
{
    if (accessMethods() & QNearFieldTarget::TagTypeSpecificAccess)
        return 0xFEFF;

    // TODO: check if 'capacity' of NFCNDEFTag can be used?
    return 0;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::sendCommand(const QByteArray &command)
{
    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());

    if (!(accessMethods() & QNearFieldTarget::TagTypeSpecificAccess)) {
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    queue.enqueue(std::pair(requestId, command));

    if (!connect()) {
        reportError(QNearFieldTarget::TargetOutOfRangeError, requestId);
        return requestId;
    }

    onExecuteRequest();
    return requestId;
}

bool QNearFieldTargetPrivateImpl::hasNdefMessage()
{
    return hasNDEFMessage;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::readNdefMessages()
{
    hasNDEFMessage = false;

    QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate);

    if (!(accessMethods() & QNearFieldTarget::NdefAccess) || !isNdefTag()) {
        qCWarning(QT_IOS_NFC, "Target does not allow to read NDEF messages, "
                              "was not detected as NDEF tag by the reader session?");
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    NdefOperation op;
    op.type = NdefOperation::Read;
    op.requestId = requestId;

    ndefOperations.push_back(op);
    onExecuteRequest();

    return requestId;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    auto requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate);

    if (!(accessMethods() & QNearFieldTarget::NdefAccess) || !isNdefTag()) {
        qCWarning(QT_IOS_NFC, "Target does not allow to write NDEF messages, "
                              "was not detected as NDEF tag by the reader session?");
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    if (messages.size() != 1) {
        // The native framework does not allow to write 'messages', only _one_ message
        // at a time. Not to multiply the complexity of having 'ndefOperations' queue
        // with some queue inside the delegate's code (plus some unpredictable errors
        // handling) - require a single message as a single request.
        qCWarning(QT_IOS_NFC, "Only one NDEF message per request ID can be written");
        reportError(QNearFieldTarget::UnsupportedError, requestId);
        return requestId;
    }

    NdefOperation op;
    op.type = NdefOperation::Write;
    op.requestId = requestId;
    op.message = messages.first();

    ndefOperations.push_back(op);
    onExecuteRequest();

    return requestId;
}

bool QNearFieldTargetPrivateImpl::isAvailable() const
{
    if (requestInProgress.isValid())
        return true;

    const auto tagIsAvailable = [this](auto tag) {
        return tag && (!connected || tag.available);
    };

    if (isNdefTag())
        return tagIsAvailable(static_cast<id<NFCNDEFTag>>(nfcTag.get()));

    if (@available(iOS 13, *))
        return tagIsAvailable(static_cast<id<NFCTag>>(nfcTag.get()));

    return false;
}

bool QNearFieldTargetPrivateImpl::connect()
{
    if (connected || requestInProgress.isValid())
        return true;

    if (isNdefTag())
        return connected = true;

    if (!isAvailable() || queue.isEmpty())
        return false;

    if (@available(iOS 13, *)) {
        requestInProgress = queue.head().first;
        id<NFCTag> tag = static_cast<id<NFCTag>>(nfcTag.get());
        NFCTagReaderSession* session = tag.session;
        [session connectToTag: tag completionHandler: ^(NSError* error){
            const int errorCode = error == nil ? -1 : error.code;
            QMetaObject::invokeMethod(this, [this, errorCode] {
                requestInProgress = QNearFieldTarget::RequestId();
                if (errorCode == -1) {
                    connected = true;
                    justConnected = true;
                    onExecuteRequest();
                } else {
                    const auto requestId = queue.dequeue().first;
                    reportError(
                        errorCode == NFCReaderError::NFCReaderErrorSecurityViolation
                            ? QNearFieldTarget::UnsupportedTargetError
                            : QNearFieldTarget::ConnectionError,
                        requestId);
                    invalidate();
                }
            });
        }];
        return true;
    }

    return false;
}

bool QNearFieldTargetPrivateImpl::isNdefTag() const
{
    const id tag = static_cast<id>(nfcTag.get());
    if ([tag conformsToProtocol:@protocol(NFCMiFareTag)])
        return false;
    if ([tag conformsToProtocol:@protocol(NFCFeliCaTag)])
        return false;
    if ([tag conformsToProtocol:@protocol(NFCISO15693Tag)])
        return false;
    if ([tag conformsToProtocol:@protocol(NFCISO7816Tag)])
        return false;
    return [tag conformsToProtocol:@protocol(NFCNDEFTag)];
}

void QNearFieldTargetPrivateImpl::onTargetCheck()
{
    if (!isAvailable())
        invalidate();
}

void QNearFieldTargetPrivateImpl::onTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)
{
    Q_UNUSED(id);

    if (error == QNearFieldTarget::TimeoutError)
        invalidate();
}

namespace {

QNdefMessage ndefToQtNdefMessage(NFCNDEFMessage *nativeMessage)
{
    if (!nativeMessage)
        return {};

    QList<QNdefRecord> ndefRecords;
    for (NFCNDEFPayload *ndefRecord in nativeMessage.records) {
        QNdefRecord qtNdefRecord;
        if (ndefRecord.typeNameFormat != NFCTypeNameFormatUnchanged) // Does not match anything in Qt.
            qtNdefRecord.setTypeNameFormat(QNdefRecord::TypeNameFormat(ndefRecord.typeNameFormat));
        if (ndefRecord.identifier)
            qtNdefRecord.setId(QByteArray::fromNSData(ndefRecord.identifier));
        if (ndefRecord.type)
            qtNdefRecord.setType(QByteArray::fromNSData(ndefRecord.type));
        if (ndefRecord.payload)
            qtNdefRecord.setPayload(QByteArray::fromNSData(ndefRecord.payload));
        ndefRecords.push_back(qtNdefRecord);
    }

    return QNdefMessage{ndefRecords};
}

} // Unnamed namespace.

void QNearFieldTargetPrivateImpl::onExecuteRequest()
{
    if (!nfcTag || requestInProgress.isValid())
        return;

    if (isNdefTag()) {
        if (ndefOperations.empty())
            return;

        auto *ndefDelegate = static_cast<QIosNfcNdefSessionDelegate *>(sessionDelegate);
        Q_ASSERT(ndefDelegate);

        Q_ASSERT(qt_Nfc_Queue()); // This is where callbacks get called.

        const auto op = ndefOperations.front();
        ndefOperations.pop_front();
        requestInProgress = op.requestId;
        auto requestId = requestInProgress; // Copy so we capture by value in the block.

        id<NFCNDEFTag> ndefTag = static_cast<id<NFCNDEFTag>>(nfcTag.get());

        std::unique_ptr<QNfcNdefNotifier> guard(new QNfcNdefNotifier);
        auto *cbNotifier = guard.get();

        QObject::connect(cbNotifier, &QNfcNdefNotifier::tagError, this,
                         &QNearFieldTargetPrivate::error, Qt::QueuedConnection);

        if (op.type == NdefOperation::Read) {
            QObject::connect(cbNotifier, &QNfcNdefNotifier::ndefMessageRead,
                             this, &QNearFieldTargetPrivateImpl::messageRead,
                             Qt::QueuedConnection);

            // We call it here, but the callback will be executed on an unspecified thread.
            [ndefTag readNDEFWithCompletionHandler:^(NFCNDEFMessage * _Nullable msg, NSError * _Nullable err) {
                const std::unique_ptr<QNfcNdefNotifier> notifierGuard(cbNotifier);
                if (err) {
                    NSLog(@"Reading NDEF messaged ended with error: %@", err);
                    emit cbNotifier->tagError(QNearFieldTarget::NdefReadError, requestId);
                    return;
                }

                const QNdefMessage ndefMessage(ndefToQtNdefMessage(msg));
                emit cbNotifier->ndefMessageRead(ndefMessage, requestId);
            }];
        } else {
            QObject::connect(cbNotifier, &QNfcNdefNotifier::ndefMessageWritten,
                             this, &QNearFieldTargetPrivateImpl::messageWritten,
                             Qt::QueuedConnection);

            NSData *ndefData = op.message.toByteArray().toNSData(); // autoreleased.
            Q_ASSERT(ndefData);

            NFCNDEFMessage *ndefMessage = [NFCNDEFMessage ndefMessageWithData:ndefData]; // autoreleased.
            Q_ASSERT(ndefMessage);

            [ndefTag writeNDEF:ndefMessage completionHandler:^(NSError *err) {
                const std::unique_ptr<QNfcNdefNotifier> notifierGuard(cbNotifier);
                if (err) {
                    NSLog(@"Writing NDEF messaged ended with error: %@", err);
                    emit cbNotifier->tagError(QNearFieldTarget::NdefWriteError, requestId);
                    return;
                }

                emit cbNotifier->ndefMessageWritten(requestId);
            }];
        }
        guard.release(); // Owned by the completion handler now.
        return;
    }

    if (@available(iOS 13, *)) {
        if (queue.isEmpty())
            return;
        const auto request = queue.dequeue();
        requestInProgress = request.first;
        const auto tag = static_cast<id<NFCISO7816Tag>>(nfcTag.get());
        auto *apdu = [[[NFCISO7816APDU alloc] initWithData: request.second.toNSData()] autorelease];
        [tag sendCommandAPDU: apdu completionHandler: ^(NSData* responseData, uint8_t sw1, uint8_t sw2, NSError* error){
            QByteArray recvBuffer = QByteArray::fromNSData(responseData);
            recvBuffer += static_cast<char>(sw1);
            recvBuffer += static_cast<char>(sw2);
            auto errorToReport = QNearFieldTarget::NoError;
            if (error != nil)
            {
                switch (error.code) {
                    case NFCReaderError::NFCReaderTransceiveErrorSessionInvalidated:
                    case NFCReaderError::NFCReaderTransceiveErrorTagNotConnected:
                        if (justConnected) {
                            errorToReport = QNearFieldTarget::UnsupportedTargetError;
                            justConnected = false;
                            break;
                        }
                        Q_FALLTHROUGH();
                    default:
                        errorToReport = QNearFieldTarget::CommandError;
                }
            }
            responseProvider->provideResponse(request.first, errorToReport, recvBuffer);
        }];
    }
}

void  QNearFieldTargetPrivateImpl::onResponseReceived(QNearFieldTarget::RequestId requestId, QNearFieldTarget::Error error, QByteArray recvBuffer)
{
    if (requestInProgress != requestId)
        return;

    requestInProgress = QNearFieldTarget::RequestId();
    if (error == QNearFieldTarget::NoError) {
        setResponseForRequest(requestId, recvBuffer, true);
        onExecuteRequest();
    } else {
        reportError(error, requestId);
        invalidate();
    }
}

void QNearFieldTargetPrivateImpl::messageRead(const QNdefMessage &message, QNearFieldTarget::RequestId requestId)
{
    hasNDEFMessage = message.size() != 0;

    setResponseForRequest(requestId, message.toByteArray(), true);
    requestInProgress = {}; // Invalidating, so we can execute the next one.
    onExecuteRequest();

    Q_EMIT ndefMessageRead(message);
}

void QNearFieldTargetPrivateImpl::messageWritten(QNearFieldTarget::RequestId requestId)
{
    requestInProgress = {}; // Invalidating, so we can execute the next one.
    onExecuteRequest();

    Q_EMIT requestCompleted(requestId);
}

QT_END_NAMESPACE
