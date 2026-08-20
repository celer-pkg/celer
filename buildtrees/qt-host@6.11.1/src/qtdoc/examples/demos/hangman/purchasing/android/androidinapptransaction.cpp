// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "../inapp/inappproduct.h"
#include "androidinapptransaction.h"
#include "androidinapppurchasebackend.h"

QT_BEGIN_NAMESPACE

AndroidInAppTransaction::AndroidInAppTransaction(const QString &signature,
                                                   const QString &data,
                                                   const QString &purchaseToken,
                                                   const QString &orderId,
                                                   TransactionStatus status,
                                                   InAppProduct *product,
                                                   const QDateTime &timestamp,
                                                   FailureReason failureReason,
                                                   const QString &errorString,
                                                   QObject *parent)
    : InAppTransaction(status, product, parent)
    , m_signature(signature)
    , m_data(data)
    , m_purchaseToken(purchaseToken)
    , m_orderId(orderId)
    , m_timestamp(timestamp)
    , m_errorString(errorString)
    , m_failureReason(failureReason)
{
    Q_ASSERT(qobject_cast<AndroidInAppPurchaseBackend *>(parent) != 0);
}

QString AndroidInAppTransaction::orderId() const
{
    return m_orderId;
}

QDateTime AndroidInAppTransaction::timestamp() const
{
    return m_timestamp;
}

QString AndroidInAppTransaction::errorString() const
{
    return m_errorString;
}

InAppTransaction::FailureReason AndroidInAppTransaction::failureReason() const
{
    return m_failureReason;
}

QString AndroidInAppTransaction::platformProperty(const QString &propertyName) const
{
    if (propertyName.compare(QStringLiteral("AndroidSignature"), Qt::CaseInsensitive) == 0)
        return m_signature;
    else if (propertyName.compare(QStringLiteral("AndroidPurchaseData"), Qt::CaseInsensitive) == 0)
        return m_data;
    else
        return InAppTransaction::platformProperty(propertyName);
}

void AndroidInAppTransaction::finalize()
{
    AndroidInAppPurchaseBackend *backend = qobject_cast<AndroidInAppPurchaseBackend *>(parent());
    if (status() == PurchaseApproved || status() == PurchaseRestored) {
        if (product()->productType() == InAppProduct::Consumable){
            backend->consumeTransaction(m_purchaseToken);}
        else if (product()->productType() == InAppProduct::Unlockable){
            backend->registerFinalizedUnlockable(m_purchaseToken);}
        else {
            qWarning("Product type not implemented.");
        }
    }

    deleteLater();
}
QT_END_NAMESPACE
