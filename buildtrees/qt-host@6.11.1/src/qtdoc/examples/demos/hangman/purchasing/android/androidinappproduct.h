// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANDROIDINAPPPRODUCT_H
#define ANDROIDINAPPPRODUCT_H

#include "../inapp/inappproduct.h"

class AndroidInAppPurchaseBackend;
class AndroidInAppProduct : public InAppProduct
{
    Q_OBJECT
public:
    explicit AndroidInAppProduct(AndroidInAppPurchaseBackend *backend,
                                  const QString &price,
                                  const QString &title,
                                  const QString &description,
                                  ProductType productType,
                                  const QString &identifier,
                                  QObject *parent = 0);

    QString getProductId();
    void purchase();

private:
    AndroidInAppPurchaseBackend *m_backend;
};

#endif // ANDROIDINAPPPRODUCT_H
