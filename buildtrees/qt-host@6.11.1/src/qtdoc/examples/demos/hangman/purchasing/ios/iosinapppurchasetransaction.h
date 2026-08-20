// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IosINAPPPURCHASETRANSACTION_H
#define IosINAPPPURCHASETRANSACTION_H

#include "../inapp/inapptransaction.h"
#include <QtCore/QString>

#import <StoreKit/StoreKit.h>
#import <Foundation/Foundation.h>
#import <StoreKit/StoreKitDefines.h>

@class SKPaymentTransaction;

QT_BEGIN_NAMESPACE

class IosInAppPurchaseBackend;

class IosInAppPurchaseTransaction : public InAppTransaction
{
    Q_OBJECT
public:
    IosInAppPurchaseTransaction(SKPaymentTransaction *transaction,
                                 const TransactionStatus status,
                                 InAppProduct *product,
                                 IosInAppPurchaseBackend *backend = 0);

    void finalize();
    QString orderId() const;
    FailureReason failureReason() const;
    QString errorString() const;
    QDateTime timestamp() const;

private:
    SKPaymentTransaction *m_nativeTransaction;
    QString m_errorString;
    FailureReason m_failureReason;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(IosInAppPurchaseTransaction*)

#endif // IOSINAPPTRANSACTION_P_H
