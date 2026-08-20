// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "iosinapppurchasebackend.h"
#include "iosinapppurchaseproduct.h"
#include "iosinapppurchasetransaction.h"

#include <QtCore/QString>

#import <StoreKit/StoreKit.h>

@interface QT_MANGLE_NAMESPACE(InAppPurchaseManager) : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver>
{
    IosInAppPurchaseBackend *backend;
    NSMutableArray<SKPaymentTransaction *> *pendingTransactions;
}

-(void)requestProductData:(NSString *)identifier;
-(void)processPendingTransactions;

@end

@implementation QT_MANGLE_NAMESPACE(InAppPurchaseManager)

-(id)initWithBackend:(IosInAppPurchaseBackend *)iapBackend {
    if (self = [super init]) {
        backend = iapBackend;
        pendingTransactions = [[NSMutableArray<SKPaymentTransaction *> alloc] init];
        [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
        qRegisterMetaType<IosInAppPurchaseProduct*>("IosInAppPurchaseProduct*");
        qRegisterMetaType<IosInAppPurchaseTransaction*>("IosInAppPurchaseTransaction*");
    }
    return self;
}

-(void)dealloc
{
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
    [pendingTransactions release];
    [super dealloc];
}

-(void)requestProductData:(NSString *)identifier
{
    NSSet<NSString *> *productId = [NSSet<NSString *> setWithObject:identifier];
    SKProductsRequest *productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:productId];
    productsRequest.delegate = self;
    [productsRequest start];
}

-(void)processPendingTransactions
{
    NSMutableArray<SKPaymentTransaction *> *registeredTransactions = [NSMutableArray<SKPaymentTransaction *> array];

    for (SKPaymentTransaction *transaction in pendingTransactions) {
        InAppTransaction::TransactionStatus status = [QT_MANGLE_NAMESPACE(InAppPurchaseManager) statusFromTransaction:transaction];

        IosInAppPurchaseProduct *product = backend->registeredProductForProductId(QString::fromNSString(transaction.payment.productIdentifier));

        if (product) {
            //It is possible that the product doesn't exist yet (because of previous restores).
            IosInAppPurchaseTransaction *qtTransaction = new IosInAppPurchaseTransaction(transaction, status, product);
            if (qtTransaction->thread() != backend->thread()) {
                qtTransaction->moveToThread(backend->thread());
                QMetaObject::invokeMethod(backend, "setParentToBackend", Qt::AutoConnection, Q_ARG(QObject*, qtTransaction));
            }
            [registeredTransactions addObject:transaction];
            QMetaObject::invokeMethod(backend, "registerTransaction", Qt::AutoConnection, Q_ARG(IosInAppPurchaseTransaction*, qtTransaction));
        }
    }

    //Remove registeredTransactions from pendingTransactions
    [pendingTransactions removeObjectsInArray:registeredTransactions];
}


//SKProductsRequestDelegate
-(void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    NSArray<SKProduct *> *products = response.products;
    SKProduct *product = [products count] == 1 ? [[products firstObject] retain] : nil;

    if (product == nil) {
        //Invalid product ID
        NSString *invalidId = [response.invalidProductIdentifiers firstObject];
        QMetaObject::invokeMethod(backend, "registerQueryFailure", Qt::AutoConnection, Q_ARG(QString, QString::fromNSString(invalidId)));
    } else {
        //Valid product query
        //Create a IosInAppPurchaseProduct
        IosInAppPurchaseProduct *validProduct = new IosInAppPurchaseProduct(product, backend->productTypeForProductId(QString::fromNSString([product productIdentifier])));
        if (validProduct->thread() != backend->thread()) {
            validProduct->moveToThread(backend->thread());
            QMetaObject::invokeMethod(backend, "setParentToBackend", Qt::AutoConnection, Q_ARG(QObject*, validProduct));
        }
        QMetaObject::invokeMethod(backend, "registerProduct", Qt::AutoConnection, Q_ARG(IosInAppPurchaseProduct*, validProduct));
    }

    [request release];
}

+(InAppTransaction::TransactionStatus)statusFromTransaction:(SKPaymentTransaction *)transaction
{
    InAppTransaction::TransactionStatus status;
    switch (transaction.transactionState) {
        case SKPaymentTransactionStatePurchasing:
            //Ignore the purchasing state as it's not really a transaction
            //And its important that it doesn't need to be finalized as
            //Calling finishTransaction: on a transaction that is
            //in the SKPaymentTransactionStatePurchasing state throws an exception
            status = InAppTransaction::Unknown;
            break;
        case SKPaymentTransactionStatePurchased:
            status = InAppTransaction::PurchaseApproved;
            break;
        case SKPaymentTransactionStateFailed:
            status = InAppTransaction::PurchaseFailed;
            break;
        case SKPaymentTransactionStateRestored:
            status = InAppTransaction::PurchaseRestored;
            break;
        default:
            status = InAppTransaction::Unknown;
            break;
    }
    return status;
}

//SKPaymentTransactionObserver
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray<SKPaymentTransaction *> *)transactions
{
    Q_UNUSED(queue);
    for (SKPaymentTransaction *transaction in transactions) {
        //Create IosInAppPurchaseTransaction
        InAppTransaction::TransactionStatus status = [QT_MANGLE_NAMESPACE(InAppPurchaseManager) statusFromTransaction:transaction];

        if (status == InAppTransaction::Unknown)
            continue;

        IosInAppPurchaseProduct *product = backend->registeredProductForProductId(QString::fromNSString(transaction.payment.productIdentifier));

        if (product) {
            //It is possible that the product doesn't exist yet (because of previous restores).
            IosInAppPurchaseTransaction *qtTransaction = new IosInAppPurchaseTransaction(transaction, status, product);
            if (qtTransaction->thread() != backend->thread()) {
                qtTransaction->moveToThread(backend->thread());
                QMetaObject::invokeMethod(backend, "setParentToBackend", Qt::AutoConnection, Q_ARG(QObject*, qtTransaction));
            }
            QMetaObject::invokeMethod(backend, "registerTransaction", Qt::AutoConnection, Q_ARG(IosInAppPurchaseTransaction*, qtTransaction));
        } else {
            //Add the transaction to the pending transactions list
            [pendingTransactions addObject:transaction];
        }
    }
}

@end


QT_BEGIN_NAMESPACE

IosInAppPurchaseBackend::IosInAppPurchaseBackend(QObject *parent)
    : InAppPurchaseBackend(parent)
    , m_iapManager(0)
{
}

IosInAppPurchaseBackend::~IosInAppPurchaseBackend()
{
    [m_iapManager release];
}

void IosInAppPurchaseBackend::initialize()
{
    m_iapManager = [[QT_MANGLE_NAMESPACE(InAppPurchaseManager) alloc] initWithBackend:this];
    emit InAppPurchaseBackend::ready();
}

bool IosInAppPurchaseBackend::isReady() const
{
    if (m_iapManager)
        return true;
    return false;
}

void IosInAppPurchaseBackend::queryProduct(InAppProduct::ProductType productType, const QString &identifier)
{
    Q_UNUSED(productType)

    if (m_productTypeForPendingId.contains(identifier)) {
        qWarning("Product query already pending for %s", qPrintable(identifier));
        return;
    }

    m_productTypeForPendingId[identifier] = productType;

    [m_iapManager requestProductData:(identifier.toNSString())];
}

void IosInAppPurchaseBackend::restorePurchases()
{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

void IosInAppPurchaseBackend::setPlatformProperty(const QString &propertyName, const QString &value)
{
    Q_UNUSED(propertyName);
    Q_UNUSED(value);
}

void IosInAppPurchaseBackend::registerProduct(IosInAppPurchaseProduct *product)
{
    QHash<QString, InAppProduct::ProductType>::iterator it = m_productTypeForPendingId.find(product->identifier());
    Q_ASSERT(it != m_productTypeForPendingId.end());

    m_registeredProductForId[product->identifier()] = product;
    emit productQueryDone(product);
    m_productTypeForPendingId.erase(it);
    [m_iapManager processPendingTransactions];
}

void IosInAppPurchaseBackend::registerQueryFailure(const QString &productId)
{
    QHash<QString, InAppProduct::ProductType>::iterator it = m_productTypeForPendingId.find(productId);
    Q_ASSERT(it != m_productTypeForPendingId.end());

    emit InAppPurchaseBackend::productQueryFailed(it.value(), it.key());
    m_productTypeForPendingId.erase(it);
}

void IosInAppPurchaseBackend::registerTransaction(IosInAppPurchaseTransaction *transaction)
{
    emit InAppPurchaseBackend::transactionReady(transaction);
}

InAppProduct::ProductType IosInAppPurchaseBackend::productTypeForProductId(const QString &productId)
{
    return m_productTypeForPendingId[productId];
}

IosInAppPurchaseProduct *IosInAppPurchaseBackend::registeredProductForProductId(const QString &productId)
{
    return m_registeredProductForId[productId];
}

void IosInAppPurchaseBackend::setParentToBackend(QObject *object)
{
    object->setParent(this);
}

QT_END_NAMESPACE

#include "moc_iosinapppurchasebackend.cpp"
