// Copyright (C) 2017 Governikus GmbH & Co. K
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldtarget_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE


QNearFieldTargetPrivate::QNearFieldTargetPrivate(QObject *parent)
:   QObject(parent)
,   q_ptr(nullptr)
{
}

QByteArray QNearFieldTargetPrivate::uid() const
{
    return QByteArray();
}

QNearFieldTarget::Type QNearFieldTargetPrivate::type() const
{
    return QNearFieldTarget::Type::ProprietaryTag;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivate::accessMethods() const
{
    return QNearFieldTarget::AccessMethod::UnknownAccess;
}

bool QNearFieldTargetPrivate::disconnect()
{
    return false;
}

// NdefAccess
bool QNearFieldTargetPrivate::hasNdefMessage()
{
    return false;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivate::readNdefMessages()
{
    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

QNearFieldTarget::RequestId
QNearFieldTargetPrivate::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    Q_UNUSED(messages);

    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

// TagTypeSpecificAccess
int QNearFieldTargetPrivate::maxCommandLength() const
{
    return 0;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivate::sendCommand(const QByteArray &command)
{
    Q_UNUSED(command);

    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

bool QNearFieldTargetPrivate::waitForRequestCompleted(const QNearFieldTarget::RequestId &id,
                                                      int msecs)
{
    QElapsedTimer timer;
    timer.start();

    const QPointer<QNearFieldTargetPrivate> weakThis = this;

    do {
        if (!weakThis)
            return false;

        if (m_decodedResponses.contains(id))
            return true;
        else
            QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 1);
    } while (timer.elapsed() <= msecs);

    reportError(QNearFieldTarget::TimeoutError, id);

    return false;
}

QVariant QNearFieldTargetPrivate::requestResponse(const QNearFieldTarget::RequestId &id) const
{
    return m_decodedResponses.value(id);
}

void QNearFieldTargetPrivate::setResponseForRequest(const QNearFieldTarget::RequestId &id,
                                                    const QVariant &response,
                                                    bool emitRequestCompleted)
{
    for (auto i = m_decodedResponses.begin(), end = m_decodedResponses.end(); i != end; /* erasing */) {
        // no more external references
        if (i.key().refCount() == 1)
            i = m_decodedResponses.erase(i);
        else
            ++i;
    }

    m_decodedResponses.insert(id, response);

    if (emitRequestCompleted)
        Q_EMIT requestCompleted(id);
}

void QNearFieldTargetPrivate::reportError(QNearFieldTarget::Error error,
                                          const QNearFieldTarget::RequestId &id)
{
    setResponseForRequest(id, QVariant(), false);
    QMetaObject::invokeMethod(this, [this, error, id]() {
        Q_EMIT this->error(error, id);
    }, Qt::QueuedConnection);
}

QT_END_NAMESPACE

#include "moc_qnearfieldtarget_p.cpp"
