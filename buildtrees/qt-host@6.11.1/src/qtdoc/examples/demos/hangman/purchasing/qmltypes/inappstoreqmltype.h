// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef INAPPSTOREQMLTYPE_H
#define INAPPSTOREQMLTYPE_H

#include <QList>
#include <QObject>
#include <QQmlListProperty>
#include <QtQml/qqml.h>

#include "inappproductqmltype.h"
#include "../inapp/inappstore.h"

class InAppStore;
class InAppProductQmlType;
class InAppStoreQmlType : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Store)

public:
    explicit InAppStoreQmlType(QObject *parent = 0);

    InAppStore *store() const;

    Q_INVOKABLE void restorePurchases();

private:
    InAppStore *m_store;
    QList<InAppProductQmlType *> m_products;
};



#endif // INAPPSTOREQMLTYPE_H
