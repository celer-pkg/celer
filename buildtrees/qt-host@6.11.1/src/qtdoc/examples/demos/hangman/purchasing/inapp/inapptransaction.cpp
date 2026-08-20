// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "inapptransaction.h"

class InAppTransactionPrivate
{
public:
    InAppTransactionPrivate(InAppTransaction::TransactionStatus s,
                             InAppProduct *p)
        : status(s)
        , product(p)
    {
    }

    InAppTransaction::TransactionStatus status;
    InAppProduct *product;
};

InAppTransaction::InAppTransaction(TransactionStatus status,
                                     InAppProduct *product,
                                     QObject *parent)
    : QObject(parent)
{
    d = QSharedPointer<InAppTransactionPrivate>(new InAppTransactionPrivate(status, product));
}

InAppTransaction::~InAppTransaction()
{
}

InAppProduct *InAppTransaction::product() const
{
    return d->product;
}

InAppTransaction::TransactionStatus InAppTransaction::status() const
{
    return d->status;
}

InAppTransaction::FailureReason InAppTransaction::failureReason() const
{
    return NoFailure;
}

QString InAppTransaction::errorString() const
{
    return QString();
}

QDateTime InAppTransaction::timestamp() const
{
    return QDateTime();
}

QString InAppTransaction::orderId() const
{
    return QString();
}

QString InAppTransaction::platformProperty(const QString &propertyName) const
{
    Q_UNUSED(propertyName);
    return QString();
}
