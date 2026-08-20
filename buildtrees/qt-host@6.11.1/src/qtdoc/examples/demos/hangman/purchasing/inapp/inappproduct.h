// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPPRODUCT_H
#define INAPPPRODUCT_H

#include <QObject>
#include <QSharedPointer>

class InAppProductPrivate;
class InAppProduct: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier CONSTANT)
    Q_PROPERTY(ProductType productType READ productType CONSTANT)
    Q_PROPERTY(QString price READ price CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)

public:
    enum ProductType
    {
        Consumable,
        Unlockable
    };
    Q_ENUM(ProductType)

    ~InAppProduct();

    QString identifier() const;
    ProductType productType() const;

    QString price() const;
    QString title() const;
    QString description() const;

    Q_INVOKABLE virtual void purchase() = 0;

protected:
    explicit InAppProduct(const QString &price, const QString &title, const QString &description, ProductType productType, const QString &identifier, QObject *parent = nullptr);

private:
    friend class InAppStore;
    Q_DISABLE_COPY(InAppProduct)

    QSharedPointer<InAppProductPrivate> d;
};

#endif // INAPPPRODUCT_H
