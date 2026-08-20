// Copyright (C) 2020 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QNEARFIELDTARGET_IOS_P_H
#define QNEARFIELDTARGET_IOS_P_H

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

#include "qnearfieldtarget_p.h"
#include "qnearfieldtarget.h"

#include "qndefmessage.h"

#include <QQueue>
#include <QTimer>

#include <memory>
#include <deque>

QT_BEGIN_NAMESPACE

class ResponseProvider : public QObject
{
    Q_OBJECT

    public:
        void provideResponse(QNearFieldTarget::RequestId requestId, QNearFieldTarget::Error error, QByteArray recvBuffer);

    Q_SIGNALS:
        void responseReceived(QNearFieldTarget::RequestId requestId, QNearFieldTarget::Error error, QByteArray recvBuffer);
};

struct NfcDeleter
{
    void operator()(void *tag);
};

struct NdefOperation
{
    enum Type {
        Read,
        Write
    } type = Read;

    QNearFieldTarget::RequestId requestId;
    QNdefMessage message;
};

class QNfcNdefNotifier;

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
    Q_OBJECT

public:
    QNearFieldTargetPrivateImpl(void *tag, QObject *parent = nullptr);
    QNearFieldTargetPrivateImpl(void *sessionDelegate, void *tag, QObject *parent = nullptr);
    ~QNearFieldTargetPrivateImpl() override;
    void invalidate();

    QByteArray uid() const override;
    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    int maxCommandLength() const override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;

    // NdefAccess
    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;


    bool isAvailable() const;

Q_SIGNALS:
    void targetLost(QNearFieldTargetPrivateImpl *target);

private:
    std::unique_ptr<void, NfcDeleter> nfcTag;

    // Owned by the near field manager.
    void *sessionDelegate = nil;
    // Owned by the session delegate.
    QNfcNdefNotifier *notifier = nullptr;
    bool hasNDEFMessage = false;

    bool connected = false;
    bool justConnected = false;
    QTimer targetCheckTimer;
    QNearFieldTarget::RequestId requestInProgress;
    QQueue<std::pair<QNearFieldTarget::RequestId, QByteArray>> queue;
    std::deque<NdefOperation> ndefOperations;

    bool connect();

    bool isNdefTag() const;

private Q_SLOTS:
    void onTargetCheck();
    void onTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
    void onExecuteRequest();
    void onResponseReceived(QNearFieldTarget::RequestId requestId, QNearFieldTarget::Error error, QByteArray recvBuffer);
    // NDEF:
    void messageRead(const QNdefMessage &ndefMessage, QNearFieldTarget::RequestId request);
    void messageWritten(QNearFieldTarget::RequestId request);
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_IOS_P_H
