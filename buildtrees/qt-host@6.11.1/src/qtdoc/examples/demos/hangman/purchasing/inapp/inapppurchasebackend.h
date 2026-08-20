// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPPURCHASEBACKEND_H
#define INAPPPURCHASEBACKEND_H

#include <QObject>

#include "inappproduct.h"

QT_BEGIN_NAMESPACE

class InAppProduct;
class InAppTransaction;
class InAppStore;
class InAppPurchaseBackend : public QObject
{
    Q_OBJECT
public:
    struct Product
    {
        Product(InAppProduct::ProductType type, const QString &id)
            : productType(type), identifier(id)
        {
        }

        InAppProduct::ProductType productType;
        QString identifier;
    };

    explicit InAppPurchaseBackend(QObject *parent = 0);

    virtual void initialize();
    virtual bool isReady() const;

    virtual void queryProducts(const QList<Product> &products);
    virtual void queryProduct(InAppProduct::ProductType productType, const QString &identifier);
    virtual void restorePurchases();

    virtual void setPlatformProperty(const QString &propertyName, const QString &value);

    void setStore(InAppStore *store) { m_store = store; }
    InAppStore *store() const { return m_store; }

Q_SIGNALS:
    void ready();
    void transactionReady(InAppTransaction *transaction);
    void productQueryFailed(InAppProduct::ProductType productType, const QString &identifier);
    void productQueryDone(InAppProduct *product);

private:
    InAppStore *m_store;
};

#endif // INAPPPURCHASEBACKEND_H
