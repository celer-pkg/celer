// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/qcoreevent.h>

#include "inappproductqmltype.h"
#include "inappstoreqmltype.h"
#include "../inapp/inapptransaction.h"
#include "../inapp/inappstore.h"

InAppProductQmlType::InAppProductQmlType(QObject *parent)
    : QObject(parent)
    , m_status(Uninitialized)
    , m_type(InAppProductQmlType::ProductType(-1))
    , m_componentComplete(false)
    , m_store(0)
    , m_product(0)
{
}

void InAppProductQmlType::setStore(InAppStoreQmlType *store)
{
    if (m_store == store)
        return;

    if (m_store != 0)
        m_store->store()->disconnect(this);

    m_store = store;
    connect(m_store->store(), &InAppStore::productRegistered,
            this, &InAppProductQmlType::handleProductRegistered);
    connect(m_store->store(), &InAppStore::productUnknown,
            this, &InAppProductQmlType::handleProductUnknown);
    connect(m_store->store(), &InAppStore::transactionReady,
                     this, &InAppProductQmlType::handleTransaction);

    updateProduct();

    emit storeChanged();
}

InAppStoreQmlType *InAppProductQmlType::store() const
{
    return m_store;
}

void InAppProductQmlType::componentComplete()
{
    if (!m_componentComplete) {
        m_componentComplete = true;
        updateProduct();
    }
}

void InAppProductQmlType::setIdentifier(const QString &identifier)
{
    if (m_identifier == identifier)
        return;

    if (m_status != Uninitialized) {
        qWarning("A product's identifier cannot be changed once the product has been initialized.");
        return;
    }

    m_identifier = identifier;
    if (m_componentComplete)
        updateProduct();
    emit identifierChanged();
}

void InAppProductQmlType::updateProduct()
{
    if (m_store == 0)
        return;

    Status oldStatus = m_status;
    InAppProduct *product = 0;
    if (m_identifier.isEmpty() || m_type == InAppProductQmlType::ProductType(-1)) {
        m_status = Uninitialized;
    } else {
        product = m_store->store()->registeredProduct(m_identifier);
        if (product != 0 && product == m_product)
            return;

        if (product == 0) {
            m_status = PendingRegistration;
            m_store->store()->registerProduct(InAppProduct::ProductType(m_type), m_identifier);
        } else if (product->productType() != InAppProduct::ProductType(m_type)) {
            qWarning("Product registered multiple times with different product types.");
            product = 0;
            m_status = Uninitialized;
        } else {
            m_status = Registered;
        }
    }

    setProduct(product);
    if (oldStatus != m_status)
        emit statusChanged();
}

void InAppProductQmlType::resetStatus()
{
    updateProduct();
}

QString InAppProductQmlType::identifier() const
{
    return m_identifier;
}

void InAppProductQmlType::setType(InAppProductQmlType::ProductType type)
{
    if (m_type == type)
        return;

    if (m_status != Uninitialized) {
        qWarning("A product's type cannot be changed once the product has been initialized.");
        return;
    }

    m_type = type;
    if (m_componentComplete)
        updateProduct();

    emit typeChanged();
}

InAppProductQmlType::ProductType InAppProductQmlType::type() const
{
    return m_type;
}

InAppProductQmlType::Status InAppProductQmlType::status() const
{
    return m_status;
}

QString InAppProductQmlType::price() const
{
    return m_product != 0 ? m_product->price() : QString();
}

QString InAppProductQmlType::title() const
{
    return m_product != 0 ? m_product->title() : QString();
}

QString InAppProductQmlType::description() const
{
    return m_product != 0 ? m_product->description() : QString();
}

void InAppProductQmlType::setProduct(InAppProduct *product)
{
    if (m_product == product)
        return;

    QString oldPrice = price();
    QString oldTitle = title();
    QString oldDescription = description();
    m_product = product;
    if (price() != oldPrice)
        emit priceChanged();
    if (title() != oldTitle)
        emit titleChanged();
    if (description() != oldDescription)
        emit descriptionChanged();
}

void InAppProductQmlType::handleProductRegistered(InAppProduct *product)
{
    if (product->identifier() == m_identifier) {
        Q_ASSERT(product->productType() == InAppProduct::ProductType(m_type));
        setProduct(product);
        if (m_status != Registered) {
            m_status = Registered;
            emit statusChanged();
        }
    }
}

void InAppProductQmlType::handleProductUnknown(InAppProduct::ProductType, const QString &identifier)
{
    if (identifier == m_identifier) {
        setProduct(0);
        if (m_status != Unknown) {
            m_status = Unknown;
            emit statusChanged();
        }
    }
}

void InAppProductQmlType::handleTransaction(InAppTransaction *transaction)
{
    if (transaction->product()->identifier() != m_identifier)
        return;

    if (transaction->status() == InAppTransaction::PurchaseApproved)
        emit purchaseSucceeded(transaction);
    else if (transaction->status() == InAppTransaction::PurchaseRestored)
        emit purchaseRestored(transaction);
    else{
        emit purchaseFailed(transaction);}
}

void InAppProductQmlType::purchase()
{
    if (m_product != 0 && m_status == Registered)
        m_product->purchase();
    else
        qWarning("Attempted to purchase unregistered product");
}
