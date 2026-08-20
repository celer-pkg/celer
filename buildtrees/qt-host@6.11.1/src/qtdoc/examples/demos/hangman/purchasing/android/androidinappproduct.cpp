// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "androidinappproduct.h"
#include "androidinapppurchasebackend.h"

QT_BEGIN_NAMESPACE

AndroidInAppProduct::AndroidInAppProduct(AndroidInAppPurchaseBackend *backend,
                                           const QString &price,
                                           const QString &title,
                                           const QString &description,
                                           ProductType productType,
                                           const QString &identifier,
                                           QObject *parent)
    : InAppProduct(price, title, description, productType, identifier, parent)
    , m_backend(backend)
{
}

void AndroidInAppProduct::purchase()
{
    m_backend->purchaseProduct(this);
}
