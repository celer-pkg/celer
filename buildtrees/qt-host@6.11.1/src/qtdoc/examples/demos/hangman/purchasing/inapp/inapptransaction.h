// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPTRANSACTION_H
#define INAPPTRANSACTION_H

#include <QtCore/qobject.h>
#include <QtCore/qsharedpointer.h>
#include <QDateTime>
#include <QtQml/qqml.h>

#include "inappproduct.h"

QT_BEGIN_NAMESPACE

class InAppProduct;
class InAppTransactionPrivate;
class InAppTransaction: public QObject
{
    Q_OBJECT
    Q_PROPERTY(TransactionStatus status READ status CONSTANT)
    Q_PROPERTY(InAppProduct* product READ product CONSTANT)
    Q_PROPERTY(QString orderId READ orderId CONSTANT)
    Q_PROPERTY(FailureReason failureReason READ failureReason CONSTANT)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)
    Q_PROPERTY(QDateTime timestamp READ timestamp CONSTANT)
    QML_NAMED_ELEMENT(Transaction)
    QML_UNAVAILABLE

public:
    enum TransactionStatus {
        Unknown,
        PurchaseApproved,
        PurchaseFailed,
        PurchaseRestored
    };
    Q_ENUM(TransactionStatus);

    enum FailureReason {
        NoFailure,
        CanceledByUser,
        ErrorOccurred
    };
    Q_ENUM(FailureReason);

    ~InAppTransaction();

    InAppProduct *product() const;

    virtual QString orderId() const;
    virtual FailureReason failureReason() const;
    virtual QString errorString() const;
    virtual QDateTime timestamp() const;

    Q_INVOKABLE virtual void finalize() = 0;
    Q_INVOKABLE virtual QString platformProperty(const QString &propertyName) const;

    TransactionStatus status() const;

protected:
    explicit InAppTransaction(TransactionStatus status,
                               InAppProduct *product,
                               QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(InAppTransaction)
    QSharedPointer<InAppTransactionPrivate> d;
};

#endif // INAPPTRANSACTION_H
