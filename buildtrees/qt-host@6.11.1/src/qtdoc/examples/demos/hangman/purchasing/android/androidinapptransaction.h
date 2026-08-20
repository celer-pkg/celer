// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANDROIDINAPPTRANSACTION_H
#define ANDROIDINAPPTRANSACTION_H


#include "../inapp/inapptransaction.h"

class AndroidInAppTransaction : public InAppTransaction
{
    Q_OBJECT
public:
    explicit AndroidInAppTransaction(const QString &signature,
                                      const QString &data,
                                      const QString &purchaseToken,
                                      const QString &orderId,
                                      TransactionStatus status,
                                      InAppProduct *product,
                                      const QDateTime &timestamp,
                                      FailureReason failureReason,
                                      const QString &errorString,
                                      QObject *parent = 0);

    void finalize();

    QString orderId() const;
    QString errorString() const;
    FailureReason failureReason() const;
    QDateTime timestamp() const;
    QString platformProperty(const QString &propertyName) const;

private:
    QString m_signature;
    QString m_data;
    QString m_purchaseToken;
    QString m_orderId;
    QDateTime m_timestamp;
    QString m_errorString;
    FailureReason m_failureReason;
};

#endif // ANDROIDINAPPTRANSACTION_H
