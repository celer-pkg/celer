// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldtarget_pcsc_p.h"
#include "qndefmessage.h"
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

/*
    Construct QNearFieldTargetPrivateImpl instance.

    This object communicates with a QPcscCard object that lives inside the
    worker thread via signal-slot mechanism.
*/
QNearFieldTargetPrivateImpl::QNearFieldTargetPrivateImpl(
        const QByteArray &uid, QNearFieldTarget::AccessMethods accessMethods, int maxInputLength,
        QObject *parent)
    : QNearFieldTargetPrivate(parent),
      m_uid(uid),
      m_accessMethods(accessMethods),
      m_maxInputLength(maxInputLength)
{
    qCDebug(QT_NFC_PCSC) << "New card with UID" << m_uid.toHex(':');
}

QNearFieldTargetPrivateImpl::~QNearFieldTargetPrivateImpl()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
}

QByteArray QNearFieldTargetPrivateImpl::uid() const
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    return m_uid;
}

QNearFieldTarget::Type QNearFieldTargetPrivateImpl::type() const
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    // Currently NDEF access is only supported for Type 4 tags
    if (m_accessMethods & QNearFieldTarget::NdefAccess)
        return QNearFieldTarget::NfcTagType4;

    return QNearFieldTarget::ProprietaryTag;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivateImpl::accessMethods() const
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    return m_accessMethods;
}

bool QNearFieldTargetPrivateImpl::disconnect()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_connected || !m_isValid)
        return false;

    Q_EMIT disconnectRequest();
    return true;
}

int QNearFieldTargetPrivateImpl::maxCommandLength() const
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    return m_maxInputLength;
}

void QNearFieldTargetPrivateImpl::onRequestCompleted(const QNearFieldTarget::RequestId &request,
                                                     QNearFieldTarget::Error reason,
                                                     const QVariant &result)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (reason == QNearFieldTarget::NoError)
        setResponseForRequest(request, result);
    else
        reportError(reason, request);
}

void QNearFieldTargetPrivateImpl::onNdefMessageRead(const QNdefMessage &message)
{
    Q_EMIT ndefMessageRead(message);
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::sendCommand(const QByteArray &command)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return QNearFieldTarget::RequestId(nullptr);

    m_connected = true;

    QNearFieldTarget::RequestId reqId(new QNearFieldTarget::RequestIdPrivate);
    Q_EMIT sendCommandRequest(reqId, command);

    return reqId;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivateImpl::readNdefMessages()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return QNearFieldTarget::RequestId(nullptr);

    m_connected = true;

    QNearFieldTarget::RequestId reqId(new QNearFieldTarget::RequestIdPrivate);
    Q_EMIT readNdefMessagesRequest(reqId);

    return reqId;
}

QNearFieldTarget::RequestId
QNearFieldTargetPrivateImpl::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return QNearFieldTarget::RequestId(nullptr);

    m_connected = true;

    QNearFieldTarget::RequestId reqId(new QNearFieldTarget::RequestIdPrivate);
    Q_EMIT writeNdefMessagesRequest(reqId, messages);

    return reqId;
}

void QNearFieldTargetPrivateImpl::onDisconnected()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_connected)
        return;

    m_connected = false;

    Q_EMIT disconnected();
}

void QNearFieldTargetPrivateImpl::onInvalidated()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return;

    m_isValid = false;

    Q_EMIT targetLost(this);
}

QT_END_NAMESPACE
