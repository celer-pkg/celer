// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IOSINAPPPURCHASEBACKEND_H
#define IOSINAPPPURCHASEBACKEND_H

#include "../inapp/inapppurchasebackend.h"
#include "../inapp/inappproduct.h"
#include "../inapp/inapptransaction.h"

#include <QtCore/QHash>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(InAppPurchaseManager));

QT_BEGIN_NAMESPACE

class IosInAppPurchaseProduct;
class IosInAppPurchaseTransaction;

class IosInAppPurchaseBackend : public InAppPurchaseBackend
{
    Q_OBJECT
public:
    IosInAppPurchaseBackend(QObject *parent = 0);
    ~IosInAppPurchaseBackend();

    void initialize();
    bool isReady() const;
    void queryProduct(InAppProduct::ProductType productType, const QString &identifier);
    void restorePurchases();
    void setPlatformProperty(const QString &propertyName, const QString &value);

    //Called by InAppPurchaseManager
    Q_INVOKABLE void registerProduct(IosInAppPurchaseProduct *product);
    Q_INVOKABLE void registerQueryFailure(const QString &productId);
    Q_INVOKABLE void registerTransaction(IosInAppPurchaseTransaction *transaction);
    InAppProduct::ProductType productTypeForProductId(const QString &productId);
    IosInAppPurchaseProduct *registeredProductForProductId(const QString &productId);

private:
    QT_MANGLE_NAMESPACE(InAppPurchaseManager) *m_iapManager;
    QHash<QString, InAppProduct::ProductType> m_productTypeForPendingId;
    QHash<QString, IosInAppPurchaseProduct*> m_registeredProductForId;

private slots:
    void setParentToBackend(QObject *object);
};

QT_END_NAMESPACE

#endif // IOSINAPPPURCHASEBACKEND_H
