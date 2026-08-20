// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qiooperation_p.h"
#include "qiooperation_p_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/private/qrandomaccessasyncfile_p_p.h>

QT_BEGIN_NAMESPACE

QIOOperationPrivate::QIOOperationPrivate(QtPrivate::QIOOperationDataStorage *storage)
    : dataStorage(storage)
{
    Q_ASSERT(storage);
}

QIOOperationPrivate::~QIOOperationPrivate()
{
}

void QIOOperationPrivate::appendBytesProcessed(qint64 num)
{
    processed += num;
}

void QIOOperationPrivate::operationComplete(QIOOperation::Error err)
{
    Q_Q(QIOOperation);

    error = err;
    state = State::Finished;
    if (err != QIOOperation::Error::None)
        Q_EMIT q->errorOccurred(err);
    Q_EMIT q->finished();
}

void QIOOperationPrivate::setError(QIOOperation::Error err)
{
    Q_Q(QIOOperation);
    error = err;
    if (err != QIOOperation::Error::None) {
        state = State::Finished;
        Q_EMIT q->errorOccurred(err);
        Q_EMIT q->finished();
    }
}

QIOOperation::~QIOOperation()
{
    ensureCompleteOrCanceled();
}

QIOOperation::Type QIOOperation::type() const
{
    Q_D(const QIOOperation);
    return d->type;
}

QIOOperation::Error QIOOperation::error() const
{
    Q_D(const QIOOperation);
    return d->error;
}

bool QIOOperation::isFinished() const
{
    Q_D(const QIOOperation);
    return d->state == QIOOperationPrivate::State::Finished;
}

QIOOperation::QIOOperation(QIOOperationPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    if (auto file = qobject_cast<QRandomAccessAsyncFile*>(parent))
        d_func()->file = file;
}

void QIOOperation::ensureCompleteOrCanceled()
{
    // Block until the operation is either complete or canceled.
    // Otherwise the write/read might use removed data
    Q_D(QIOOperation);
    if (d->state != QIOOperationPrivate::State::Finished) {
        if (d->file) {
            auto *filePriv = QRandomAccessAsyncFilePrivate::get(d->file);
            filePriv->cancelAndWait(this);
        }
    }
}

QIOReadWriteOperationBase::~QIOReadWriteOperationBase()
        = default;

qint64 QIOReadWriteOperationBase::offset() const
{
    Q_D(const QIOOperation);
    return d->offset;
}

qint64 QIOReadWriteOperationBase::numBytesProcessed() const
{
    if (!isFinished())
        return -1;
    Q_D(const QIOOperation);
    return d->processed;
}

QIOReadWriteOperationBase::QIOReadWriteOperationBase(QIOOperationPrivate &dd, QObject *parent)
    : QIOOperation(dd, parent)
{
}

QIOReadOperation::~QIOReadOperation() = default;

QByteArray QIOReadOperation::data() const
{
    if (!isFinished())
        return {};
    Q_D(const QIOOperation);
    return d->dataStorage->getValue<QByteArray>();
}

QIOReadOperation::QIOReadOperation(QIOOperationPrivate &dd, QObject *parent)
    : QIOReadWriteOperationBase(dd, parent)
{
    Q_ASSERT(dd.type == QIOOperation::Type::Read);
    Q_ASSERT(dd.dataStorage->containsByteArray());
}

QIOWriteOperation::~QIOWriteOperation() = default;

QByteArray QIOWriteOperation::data() const
{
    if (!isFinished())
        return {};
    Q_D(const QIOOperation);
    return d->dataStorage->getValue<QByteArray>();
}

QIOWriteOperation::QIOWriteOperation(QIOOperationPrivate &dd, QObject *parent)
    : QIOReadWriteOperationBase(dd, parent)
{
    Q_ASSERT(dd.type == QIOOperation::Type::Write);
    Q_ASSERT(dd.dataStorage->containsByteArray());
}

QIOVectoredReadOperation::~QIOVectoredReadOperation() = default;

QSpan<const QSpan<std::byte>> QIOVectoredReadOperation::data() const
{
    if (!isFinished())
        return {};
    Q_D(const QIOOperation);
    return d->dataStorage->getValue<QSpan<const QSpan<std::byte>>>();
}

QIOVectoredReadOperation::QIOVectoredReadOperation(QIOOperationPrivate &dd, QObject *parent)
    : QIOReadWriteOperationBase(dd, parent)
{
    Q_ASSERT(dd.type == QIOOperation::Type::Read);
    Q_ASSERT(dd.dataStorage->containsReadSpans());
}

QIOVectoredWriteOperation::~QIOVectoredWriteOperation() = default;

QSpan<const QSpan<const std::byte>> QIOVectoredWriteOperation::data() const
{
    if (!isFinished())
        return {};
    Q_D(const QIOOperation);
    return d->dataStorage->getValue<QSpan<const QSpan<const std::byte>>>();
}

QIOVectoredWriteOperation::QIOVectoredWriteOperation(QIOOperationPrivate &dd, QObject *parent)
    : QIOReadWriteOperationBase(dd, parent)
{
    Q_ASSERT(dd.type == QIOOperation::Type::Write);
    Q_ASSERT(dd.dataStorage->containsWriteSpans());
}

QT_END_NAMESPACE
