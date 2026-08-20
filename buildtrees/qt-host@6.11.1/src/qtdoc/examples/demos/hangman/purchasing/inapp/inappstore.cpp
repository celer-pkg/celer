// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "inappstore.h"
#include "inapppurchasebackend.h"
#include "inapptransaction.h"

#ifdef Q_OS_ANDROID
#include "../android/androidinapppurchasebackend.h"
#endif

#ifdef Q_OS_IOS
#include "../ios/iosinapppurchasebackend.h"
#endif

class IAPRegisterMetaTypes
{
public:
    IAPRegisterMetaTypes()
    {
        qRegisterMetaType<InAppProduct::ProductType>("InAppProduct::ProductType");
    }
} _registerIAPMetaTypes;

InAppStore::InAppStore(QObject *parent)
    : QObject(parent)
{
    d = QSharedPointer<InAppStorePrivate>(new InAppStorePrivate);
    setupBackend();
}

InAppStore::~InAppStore()
{
}

void InAppStore::setupBackend()
{
#if defined Q_OS_ANDROID
    d->backend = new AndroidInAppPurchaseBackend;
#elif defined Q_OS_IOS
    d->backend = new IosInAppPurchaseBackend;
#else
    d->backend = new InAppPurchaseBackend;
#endif
    d->backend->setStore(this);

    connect(d->backend, &InAppPurchaseBackend::ready,
            this, &InAppStore::registerPendingProducts);
    connect(d->backend, &InAppPurchaseBackend::transactionReady,
            this, &InAppStore::transactionReady);
    connect(d->backend, &InAppPurchaseBackend::productQueryFailed,
            this, &InAppStore::productUnknown);
    connect(d->backend, &InAppPurchaseBackend::productQueryDone,
            this, static_cast<void (InAppStore::*)(InAppProduct *)>(&InAppStore::registerProduct));
}

void InAppStore::registerProduct(InAppProduct *product)
{
    d->registeredProducts[product->identifier()] = product;
    emit productRegistered(product);
}

void InAppStore::registerPendingProducts()
{
    QList<InAppPurchaseBackend::Product> products;
    products.reserve(d->pendingProducts.size());

    QHash<QString, InAppProduct::ProductType>::const_iterator it;
    for (it = d->pendingProducts.constBegin(); it != d->pendingProducts.constEnd(); ++it)
        products.append(InAppPurchaseBackend::Product(it.value(), it.key()));
    d->pendingProducts.clear();

    d->backend->queryProducts(products);
    if (d->pendingRestorePurchases)
        restorePurchases();
}

void InAppStore::restorePurchases()
{
    if (d->backend->isReady()) {
        d->pendingRestorePurchases = false;
        d->backend->restorePurchases();
    } else {
        d->pendingRestorePurchases = true;
    }
}

void InAppStore::setPlatformProperty(const QString &propertyName, const QString &value)
{
    d->backend->setPlatformProperty(propertyName, value);
}

void InAppStore::registerProduct(InAppProduct::ProductType productType, const QString &identifier)
{
    if (!d->backend->isReady()) {
        d->pendingProducts[identifier] = productType;
        if (!d->hasCalledInitialize) {
            d->hasCalledInitialize = true;
            d->backend->initialize();
        }
    } else {
        d->backend->queryProduct(productType, identifier);
    }
}

InAppProduct *InAppStore::registeredProduct(const QString &identifier) const
{
    return d->registeredProducts.value(identifier);
}
