// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPPRODUCTQMLTYPE_H
#define INAPPPRODUCTQMLTYPE_H

#include <QtQml/qqmlparserstatus.h>
#include <QObject>
#include <QtQml/qqml.h>

#include "inappstoreqmltype.h"
#include "../inapp/inappproduct.h"
#include "../inapp/inapptransaction.h"

class InAppTransaction;
class InAppStoreQmlType;
class InAppProductQmlType : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(ProductType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString price READ price NOTIFY priceChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(InAppStoreQmlType *store READ store WRITE setStore NOTIFY storeChanged)
    QML_NAMED_ELEMENT(Product)

public:
    enum Status {
        Uninitialized,
        PendingRegistration,
        Registered,
        Unknown
    };
    Q_ENUM(Status);

    // Must match InAppProduct::ProductType
    enum ProductType {
        Consumable,
        Unlockable
    };
    Q_ENUM(ProductType);

    explicit InAppProductQmlType(QObject *parent = 0);

    Q_INVOKABLE void purchase();
    Q_INVOKABLE void resetStatus();

    void setIdentifier(const QString &identifier);
    QString identifier() const;

    Status status() const;
    QString price() const;
    QString title() const;
    QString description() const;

    void setStore(InAppStoreQmlType *store);
    InAppStoreQmlType *store() const;

    void setType(ProductType type);
    ProductType type() const;

Q_SIGNALS:
    void purchaseSucceeded(InAppTransaction *transaction);
    void purchaseFailed(InAppTransaction *transaction);
    void purchaseRestored(InAppTransaction *transaction);
    void identifierChanged();
    void statusChanged();
    void priceChanged();
    void titleChanged();
    void descriptionChanged();
    void storeChanged();
    void typeChanged();

protected:
    void componentComplete();
    void classBegin() {}

private Q_SLOTS:
    void handleTransaction(InAppTransaction *transaction);
    void handleProductRegistered(InAppProduct *product);
    void handleProductUnknown(InAppProduct::ProductType, const QString &identifier);

private:
    void setProduct(InAppProduct *product);
    void updateProduct();

    QString m_identifier;
    Status m_status;
    InAppProductQmlType::ProductType m_type;
    bool m_componentComplete;

    InAppStoreQmlType *m_store;
    InAppProduct *m_product;
};

#endif // INAPPPRODUCTQMLTYPE_H
