// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "inappproduct.h"

QT_BEGIN_NAMESPACE

class InAppProductPrivate
{
public:
    InAppProductPrivate(const QString &price, const QString &title, const QString &description, InAppProduct::ProductType type, const QString &id)
        : localPrice(price)
        , localTitle(title)
        , localDescription(description)
        , productType(type)
        , identifier(id)
    {
    }

    QString localPrice;
    QString localTitle;
    QString localDescription;
    InAppProduct::ProductType productType;
    QString identifier;
};

InAppProduct::InAppProduct(const QString &price, const QString &title, const QString &description, ProductType productType, const QString &identifier, QObject *parent)
    : QObject(parent)
{
    d = QSharedPointer<InAppProductPrivate>(new InAppProductPrivate(price, title, description, productType, identifier));
}

InAppProduct::~InAppProduct()
{
}

QString InAppProduct::price() const
{
    return d->localPrice;
}

QString InAppProduct::title() const
{
    return d->localTitle;
}

QString InAppProduct::description() const
{
    return d->localDescription;
}

QString InAppProduct::identifier() const
{
    return d->identifier;
}

InAppProduct::ProductType InAppProduct::productType() const
{
    return d->productType;
}
