// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager_pcsc_p.h"
#include "qnearfieldtarget_pcsc_p.h"
#include "pcsc/qpcscmanager_p.h"
#include "pcsc/qpcsccard_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

/*
    Constructs a new near field manager private implementation.

    This object creates a worker thread with an instance of QPcscManager in
    it. All the communication with QPcscManager is done using signal-slot
    mechanism.
*/
QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    m_worker = new QPcscManager;
    m_workerThread = new QThread(this);
    m_workerThread->setObjectName(u"QtNfcThread"_s);
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &QPcscManager::cardInserted, this,
            &QNearFieldManagerPrivateImpl::onCardInserted);
    connect(this, &QNearFieldManagerPrivateImpl::startTargetDetectionRequest, m_worker,
            &QPcscManager::onStartTargetDetectionRequest);
    connect(this, &QNearFieldManagerPrivateImpl::stopTargetDetectionRequest, m_worker,
            &QPcscManager::onStopTargetDetectionRequest);

    m_workerThread->start();
}

/*
    Destroys the near field manager private implementation.
*/
QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    // Destroy the worker. It calls QThread::quit() on the working thread in
    // its destructor.
    QMetaObject::invokeMethod(m_worker, &QObject::deleteLater, Qt::QueuedConnection);
    m_workerThread->wait();
}

bool QNearFieldManagerPrivateImpl::isEnabled() const
{
    return true;
}

bool QNearFieldManagerPrivateImpl::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    switch (accessMethod) {
    case QNearFieldTarget::TagTypeSpecificAccess:
    case QNearFieldTarget::NdefAccess:
        return true;
    default:
        return false;
    }
}

bool QNearFieldManagerPrivateImpl::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    Q_EMIT startTargetDetectionRequest(accessMethod);

    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection(const QString &errorMessage)
{
    Q_UNUSED(errorMessage);

    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    Q_EMIT stopTargetDetectionRequest();
}

/*
    Invoked when the worker has detected a new card.

    The worker will ensure that the card object remains valid until the manager
    emits targetCreatedForCard() signal.
*/
void QNearFieldManagerPrivateImpl::onCardInserted(QPcscCard *card, const QByteArray &uid,
                                                  QNearFieldTarget::AccessMethods accessMethods,
                                                  int maxInputLength)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    auto priv = new QNearFieldTargetPrivateImpl(uid, accessMethods, maxInputLength, this);

    connect(priv, &QNearFieldTargetPrivateImpl::disconnectRequest, card,
            &QPcscCard::onDisconnectRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::destroyed, card, &QPcscCard::onTargetDestroyed);
    connect(priv, &QNearFieldTargetPrivateImpl::sendCommandRequest, card,
            &QPcscCard::onSendCommandRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::readNdefMessagesRequest, card,
            &QPcscCard::onReadNdefMessagesRequest);
    connect(priv, &QNearFieldTargetPrivateImpl::writeNdefMessagesRequest, card,
            &QPcscCard::onWriteNdefMessagesRequest);

    connect(priv, &QNearFieldTargetPrivateImpl::targetLost, this,
            &QNearFieldManagerPrivateImpl::onTargetLost);

    connect(card, &QPcscCard::disconnected, priv, &QNearFieldTargetPrivateImpl::onDisconnected);
    connect(card, &QPcscCard::invalidated, priv, &QNearFieldTargetPrivateImpl::onInvalidated);
    connect(card, &QPcscCard::requestCompleted, priv,
            &QNearFieldTargetPrivateImpl::onRequestCompleted);
    connect(card, &QPcscCard::ndefMessageRead, priv,
            &QNearFieldTargetPrivateImpl::onNdefMessageRead);

    auto target = new QNearFieldTarget(priv, this);

    Q_EMIT targetDetected(target);

    // Let the worker know that the card object can be deleted if it is no
    // longer needed.
    QMetaObject::invokeMethod(card, &QPcscCard::enableAutodelete, Qt::QueuedConnection);
}

void QNearFieldManagerPrivateImpl::onTargetLost(QNearFieldTargetPrivate *target)
{
    Q_EMIT targetLost(target->q_ptr);
}

Q_LOGGING_CATEGORY(QT_NFC_PCSC, "qt.nfc.pcsc")

QT_END_NAMESPACE
