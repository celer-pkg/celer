// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IOSINAPPPURCHASEPRODUCT_H
#define IOSINAPPPURCHASEPRODUCT_H

#include "../inapp/inappproduct.h"

#import <StoreKit/StoreKit.h>

@class SKProduct;

QT_BEGIN_NAMESPACE

class IosInAppPurchaseBackend;

class IosInAppPurchaseProduct : public InAppProduct
{
    Q_OBJECT
public:
    explicit IosInAppPurchaseProduct(SKProduct *product,
                                      ProductType productType,
                                      IosInAppPurchaseBackend *backend = 0);
    void purchase();

private:
    SKProduct *m_nativeProduct;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(IosInAppPurchaseProduct*)

#endif // IOSINAPPPURCHASEPRODUCT_H
