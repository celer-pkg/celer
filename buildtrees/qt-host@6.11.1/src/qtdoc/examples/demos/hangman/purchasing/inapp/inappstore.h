// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPSTORE_H
#define INAPPSTORE_H

#include <QObject>
#include <QMutex>
#include <QHash>

#include "inappproduct.h"
#include "inapppurchasebackend.h"

class InAppProduct;
class InAppTransaction;
class InAppPurchaseBackend;
class InAppStorePrivate
{
public:
    InAppStorePrivate()
        : backend(0)
        , hasCalledInitialize(false)
        , pendingRestorePurchases(false)
    {
    }

    ~InAppStorePrivate()
    {
        delete backend;
    }

    QHash<QString, InAppProduct::ProductType> pendingProducts;
    QHash<QString, InAppProduct *> registeredProducts;
    InAppPurchaseBackend *backend;
    bool hasCalledInitialize;
    bool pendingRestorePurchases;
};
class InAppStore: public QObject
{
    Q_OBJECT

public:
    explicit InAppStore(QObject *parent = nullptr);
    ~InAppStore();

    Q_INVOKABLE void restorePurchases();
    Q_INVOKABLE void registerProduct(InAppProduct::ProductType productType, const QString &identifier);
    Q_INVOKABLE InAppProduct *registeredProduct(const QString &identifier) const;
    Q_INVOKABLE void setPlatformProperty(const QString &propertyName, const QString &value);

signals:
    void productRegistered(InAppProduct *product);
    void productUnknown(InAppProduct::ProductType productType, const QString &identifier);
    void transactionReady(InAppTransaction *transaction);

private Q_SLOTS:
    void registerPendingProducts();
    void registerProduct(InAppProduct *);

private:
    void setupBackend();

    Q_DISABLE_COPY(InAppStore)
    QSharedPointer<InAppStorePrivate> d;
};

#endif // INAPPSTORE_H
