// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "iosinapppurchaseproduct.h"
#include "iosinapppurchasebackend.h"

QT_BEGIN_NAMESPACE


static NSString *localizedPrice(SKProduct *product)
{
    NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
    [numberFormatter setLocale:product.priceLocale];
    NSString *formattedString = [numberFormatter stringFromNumber:product.price];
    [numberFormatter release];
    return formattedString;
}

IosInAppPurchaseProduct::IosInAppPurchaseProduct(SKProduct *product,
                                                   ProductType productType,
                                                   IosInAppPurchaseBackend *backend)
    : InAppProduct(QString::fromNSString(localizedPrice(product)),
                    QString::fromNSString([product localizedTitle]),
                    QString::fromNSString([product localizedDescription]),
                    productType,
                    QString::fromNSString([product productIdentifier]),
                    backend),
                    m_nativeProduct(product)
{
}

void IosInAppPurchaseProduct::purchase()
{
    SKPayment *payment = [SKPayment paymentWithProduct:m_nativeProduct];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}

QT_END_NAMESPACE

#include "moc_iosinapppurchaseproduct.cpp"
