// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANDROIDINAPPPURCHASEBACKEND_H
#define ANDROIDINAPPPURCHASEBACKEND_H

#include <QMutex>
#include <QSet>
#include <QDateTime>
#include <QJniObject>
#include <QJniEnvironment>

#include "../inapp/inapppurchasebackend.h"
#include "../inapp/inappproduct.h"
#include "../inapp/inapptransaction.h"

QT_BEGIN_NAMESPACE
class AndroidInAppProduct;
class AndroidInAppPurchaseBackend : public InAppPurchaseBackend
{
    Q_OBJECT
public:
    explicit AndroidInAppPurchaseBackend(QObject *parent = 0);

    void initialize();
    bool isReady() const;

    void queryProducts(const QList<Product> &products);
    void queryProduct(InAppProduct::ProductType productType, const QString &identifier);
    void restorePurchases();

    void setPlatformProperty(const QString &propertyName, const QString &value);

    void purchaseProduct(AndroidInAppProduct *product);

    void consumeTransaction(const QString &purchaseToken);
    void registerFinalizedUnlockable(const QString &identifier);

    // Callbacks from Java
    Q_INVOKABLE void registerProduct(const QString &productId,
                                     const QString &price,
                                     const QString &title,
                                     const QString &description);
    Q_INVOKABLE void registerPurchased(const QString &identifier,
                                       const QString &signature,
                                       const QString &data,
                                       const QString &purchaseToken,
                                       const QString &orderId,
                                       const QDateTime &timestamp);
    Q_INVOKABLE void purchaseSucceeded(int requestCode,
                                       const QString &signature,
                                       const QString &data,
                                       const QString &purchaseToken,
                                       const QString &orderId,
                                       const QDateTime &timestamp);
    Q_INVOKABLE void purchaseFailed(int requestCode,
                                    int failureReason,
                                    const QString &errorString);
    Q_INVOKABLE void registerReady();

private:
    void checkFinalizationStatus(InAppProduct *product,
                                 InAppTransaction::TransactionStatus status = InAppTransaction::PurchaseApproved);
    bool transactionFinalizedForProduct(InAppProduct *product);
    void purchaseFailed(InAppProduct *product,
                        int failureReason,
                        const QString &errorString);

    struct PurchaseInfo
    {
        PurchaseInfo(const QString &signature_, const QString &data_, const QString &purchaseToken_, const QString &orderId_, const QDateTime &timestamp_)
            : signature(signature_)
            , data(data_)
            , purchaseToken(purchaseToken_)
            , orderId(orderId_)
            , timestamp(timestamp_)
        {
        }

        QString signature;
        QString data;
        QString purchaseToken;
        QString orderId;
        QDateTime timestamp;
    };

    mutable QRecursiveMutex m_mutex;
    bool m_isReady;
    QList<QString> purchasedUnlockebles;
    QJniObject m_javaObject;
    QScopedPointer<AndroidInAppPurchaseBackend> d;
    QHash<QString, InAppProduct::ProductType> m_productTypeForPendingId;
    QHash<QString, PurchaseInfo> m_infoForPurchase;
    QHash<int, InAppProduct *> m_activePurchaseRequests;
};
QT_END_NAMESPACE

#endif // ANDROIDINAPPPURCHASEBACKEND_H
