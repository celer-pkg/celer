// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "inapppurchasebackend.h"

InAppPurchaseBackend::InAppPurchaseBackend(QObject *parent)
    : QObject(parent)
    , m_store(0)
{
}

void InAppPurchaseBackend::initialize()
{
    emit ready();
}

bool InAppPurchaseBackend::isReady() const
{
    return true;
}

void InAppPurchaseBackend::queryProducts(const QList<Product> &products)
{
    for (const Product &product : products)
        queryProduct(product.productType, product.identifier);
}

void InAppPurchaseBackend::queryProduct(InAppProduct::ProductType productType,
                                                    const QString &identifier)
{
    qWarning("InAppPurchaseBackend not implemented on this platform!");
    Q_UNUSED(productType);
    Q_UNUSED(identifier);
}

void InAppPurchaseBackend::restorePurchases()
{
    qWarning("InAppPurchaseBackend not implemented on this platform!");
}

void InAppPurchaseBackend::setPlatformProperty(const QString &propertyName, const QString &value)
{
    Q_UNUSED(propertyName);
    Q_UNUSED(value);
}
