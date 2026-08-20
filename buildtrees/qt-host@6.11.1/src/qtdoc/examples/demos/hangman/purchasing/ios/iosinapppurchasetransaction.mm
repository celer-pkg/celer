// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "iosinapppurchasetransaction.h"
#include "iosinapppurchasebackend.h"

#import <StoreKit/StoreKit.h>

QT_BEGIN_NAMESPACE

IosInAppPurchaseTransaction::IosInAppPurchaseTransaction(SKPaymentTransaction *transaction,
                                           const TransactionStatus status,
                                           InAppProduct *product,
                                           IosInAppPurchaseBackend *backend)
    : InAppTransaction(status, product, backend)
    , m_nativeTransaction(transaction)
    , m_failureReason(NoFailure)
{
    if (status == PurchaseFailed) {
        m_failureReason = ErrorOccurred;
        switch (m_nativeTransaction.error.code) {
        case SKErrorClientInvalid:
            m_errorString = QStringLiteral("Client Invalid");
            break;
        case SKErrorPaymentCancelled:
            m_errorString = QStringLiteral("Payment Cancelled");
            m_failureReason = CanceledByUser;
            break;
        case SKErrorPaymentInvalid:
            m_errorString = QStringLiteral("Payment Invalid");
            break;
        case SKErrorPaymentNotAllowed:
            m_errorString = QStringLiteral("Payment Not Allowed");
            break;
#if defined(Q_OS_IOS) || defined(Q_OS_TVOS)
        case SKErrorStoreProductNotAvailable:
            m_errorString = QStringLiteral("Store Product Not Available");
            break;
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(90300) || QT_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(90200)
        case SKErrorCloudServicePermissionDenied:
            m_errorString = QStringLiteral("Cloud Service Permission Denied");
            break;
        case SKErrorCloudServiceNetworkConnectionFailed:
            m_errorString = QStringLiteral("Cloud Service Network Connection Failed");
            break;
#endif
                                               // rdar://35589806
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(100300) // || QT_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(100200)
        case SKErrorCloudServiceRevoked:
            m_errorString = QStringLiteral("Cloud Service Revoked");
            break;
#endif
#endif
        case SKErrorUnknown:
        default:
            m_errorString = QString::fromNSString([m_nativeTransaction.error localizedDescription]);
        }
    }
}

void IosInAppPurchaseTransaction::finalize()
{
    [[SKPaymentQueue defaultQueue] finishTransaction:m_nativeTransaction];
}

QString IosInAppPurchaseTransaction::orderId() const
{
    return QString::fromNSString(m_nativeTransaction.transactionIdentifier);
}

InAppTransaction::FailureReason IosInAppPurchaseTransaction::failureReason() const
{
    return m_failureReason;
}

QString IosInAppPurchaseTransaction::errorString() const
{
    return m_errorString;
}

QDateTime IosInAppPurchaseTransaction::timestamp() const
{
    return QDateTime::fromNSDate(m_nativeTransaction.transactionDate);
}

QT_END_NAMESPACE

#include "moc_iosinapppurchasetransaction.cpp"
