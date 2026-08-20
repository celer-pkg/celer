// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager_ios_p.h"

#include "ios/qiosnfcndefsessiondelegate_p.h"
#include "ios/qiostagreaderdelegate_p.h"
#include "ios/qiosndefnotifier_p.h"

#include "qnearfieldtarget_ios_p.h"

#include <QDateTime>

#include <memory>

#import <CoreNFC/NFCReaderSession.h>
#import <CoreNFC/NFCNDEFReaderSession.h>
#import <CoreNFC/NFCTagReaderSession.h>

QT_BEGIN_NAMESPACE

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    auto notifier = std::make_unique<QNfcNdefNotifier>();

    if (@available(iOS 13, *))
        delegate = [[QT_MANGLE_NAMESPACE(QIosTagReaderDelegate) alloc] initWithListener:this];

    connect(this, &QNearFieldManagerPrivateImpl::tagDiscovered,
            this, &QNearFieldManagerPrivateImpl::onTagDiscovered,
            Qt::QueuedConnection);
    connect(this, &QNearFieldManagerPrivateImpl::didInvalidateWithError,
            this, &QNearFieldManagerPrivateImpl::onDidInvalidateWithError,
            Qt::QueuedConnection);

    ndefDelegate = [[QIosNfcNdefSessionDelegate  alloc] initWithNotifier:notifier.get()];
    if (ndefDelegate) {
        auto watchDog = notifier.release(); // Delegate took the ownership.

        connect(watchDog, &QNfcNdefNotifier::tagDetected,
                this, &QNearFieldManagerPrivateImpl::onTagDiscovered,
                Qt::QueuedConnection);
        connect(watchDog, &QNfcNdefNotifier::invalidateWithError,
                this, &QNearFieldManagerPrivateImpl::onDidInvalidateWithError,
                Qt::QueuedConnection);
    } else {
        qCWarning(QT_IOS_NFC, "Failed to allocate NDEF reading session's delegate");
    }

    sessionTimer.setInterval(2000);
    sessionTimer.setSingleShot(true);
    connect(&sessionTimer, &QTimer::timeout, this, &QNearFieldManagerPrivateImpl::onSessionTimer);
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    if (@available(iOS 13, *))
        [delegate release];

    if (!ndefDelegate)
        return;

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
            [ndefDelegate abort];
        });
    }

    [ndefDelegate release];
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    switch (accessMethod) {
    case QNearFieldTarget::AnyAccess:
    case QNearFieldTarget::NdefAccess:
        return NFCNDEFReaderSession.readingAvailable;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            return NFCTagReaderSession.readingAvailable;
        Q_FALLTHROUGH();
    case QNearFieldTarget::UnknownAccess:
        return false;
    }
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    if (detectionRunning)
        return false;

    activeAccessMethod = QNearFieldTarget::UnknownAccess;

    switch (accessMethod) {
    case QNearFieldTarget::UnknownAccess:
    case QNearFieldTarget::AnyAccess:
        return false;
    case QNearFieldTarget::TagTypeSpecificAccess:
        if (@available(iOS 13, *))
            if (NFCTagReaderSession.readingAvailable) {
                detectionRunning = scheduleSession(accessMethod);
                if (detectionRunning)
                    activeAccessMethod = accessMethod;
                return detectionRunning;
            }
        return false;
    case QNearFieldTarget::NdefAccess:
        if (NFCNDEFReaderSession.readingAvailable) {
            detectionRunning = scheduleSession(accessMethod);
            if (detectionRunning)
                activeAccessMethod = accessMethod;
            return detectionRunning;
        }
        return false;
    }

    return false;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &errorMessage)
{
    if (!detectionRunning)
        return;

    isSessionScheduled = false;

    if (activeAccessMethod == QNearFieldTarget::TagTypeSpecificAccess) {
        stopSession(errorMessage);
    } else if (activeAccessMethod == QNearFieldTarget::NdefAccess) {
        stopNdefSession(errorMessage);
    } else {
        qCWarning(QT_IOS_NFC, "Unknown access method, cannot stop target detection");
        return;
    }

    detectionRunning = false;
    Q_EMIT targetDetectionStopped();
}

bool QNearFieldManagerPrivateImpl::scheduleSession(QNearFieldTarget::AccessMethod accessMethod)
{
    if (sessionTimer.isActive()) {
        isSessionScheduled = true;
        return true;
    }
    isSessionScheduled = false;

    if (accessMethod == QNearFieldTarget::TagTypeSpecificAccess) {
        startSession();
        return true;
    } else if (accessMethod == QNearFieldTarget::NdefAccess) {
        return startNdefSession();
    }

    return false;
}

void QNearFieldManagerPrivateImpl::startSession()
{
    if (@available(iOS 13, *)) {
        [delegate startSession];
    }
}

bool QNearFieldManagerPrivateImpl::startNdefSession()
{
    if (!ndefDelegate)
        return false;

    if (auto queue = qt_Nfc_Queue()) {
        __block bool startSessionSucceded = false;
        dispatch_sync(queue, ^{ startSessionSucceded = [ndefDelegate startSession]; });
        return startSessionSucceded;
    }

    return false;
}

void QNearFieldManagerPrivateImpl::stopSession(const QString &error)
{
    Q_ASSERT(activeAccessMethod == QNearFieldTarget::TagTypeSpecificAccess);

    clearTargets();

    if (@available(iOS 13, *)) {
        [delegate stopSession:error];
    }
}

void QNearFieldManagerPrivateImpl::stopNdefSession(const QString &error)
{
    Q_ASSERT(activeAccessMethod == QNearFieldTarget::NdefAccess);

    clearTargets();

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
            [ndefDelegate stopSession:error];
        });
    }
}

void QNearFieldManagerPrivateImpl::clearTargets()
{
    auto i = detectedTargets.begin();
    while (i != detectedTargets.end()) {
        if (*i)
            (*i)->invalidate();
        else
            qCWarning(QT_IOS_NFC, "Stale private near field target found");
        i = detectedTargets.erase(i);
    }
}


void QNearFieldManagerPrivateImpl::setUserInformation(const QString &message)
{
    if (activeAccessMethod != QNearFieldTarget::NdefAccess)
        [delegate alertMessage:message];

    if (detectionRunning) {
        // Too late!
        qCWarning(QT_IOS_NFC,
                  "User information must be set prior before the target detection started");
        return;
    }

    if (auto queue = qt_Nfc_Queue()) {
        dispatch_sync(queue, ^{
           [ndefDelegate setAlertMessage:message];
        });
    }
}

void QNearFieldManagerPrivateImpl::onTagDiscovered(void *tag)
{
    QNearFieldTargetPrivateImpl *target = nullptr;
    if (activeAccessMethod == QNearFieldTarget::NdefAccess) {
        [id(tag) retain];
        target = new QNearFieldTargetPrivateImpl(ndefDelegate, tag);
    } else {
        target = new QNearFieldTargetPrivateImpl(tag);
    }

    detectedTargets += target;

    connect(target, &QNearFieldTargetPrivateImpl::targetLost,
            this, &QNearFieldManagerPrivateImpl::onTargetLost);
    Q_EMIT targetDetected(new QNearFieldTarget(target, this));
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivateImpl *target)
{
    detectedTargets.removeOne(target);
    Q_EMIT targetLost(target->q_ptr);

    if (detectionRunning && detectedTargets.isEmpty())
        onDidInvalidateWithError(true);
}

void QNearFieldManagerPrivateImpl::onDidInvalidateWithError(bool doRestart)
{
    clearTargets();
    sessionTimer.start();

    if (detectionRunning && doRestart && scheduleSession(activeAccessMethod))
        return;

    detectionRunning = false;
    Q_EMIT targetDetectionStopped();
}

void QNearFieldManagerPrivateImpl::onSessionTimer()
{
    if (isSessionScheduled && !scheduleSession(activeAccessMethod)) {
        detectionRunning = false;
        Q_EMIT targetDetectionStopped();
    }
}

QT_END_NAMESPACE
