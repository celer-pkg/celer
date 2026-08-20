// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "inappstoreqmltype.h"
#include "../inapp/inappstore.h"

InAppStoreQmlType::InAppStoreQmlType(QObject *parent)
    : QObject(parent)
    , m_store(new InAppStore(this))
{
}

InAppStore *InAppStoreQmlType::store() const
{
    return m_store;
}

void InAppStoreQmlType::restorePurchases()
{
    m_store->restorePurchases();
}
