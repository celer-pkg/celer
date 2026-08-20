// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESUPPLIER_P_H
#define QPLACESUPPLIER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QSharedData>
#include <QUrl>

#include "qplaceicon.h"

QT_BEGIN_NAMESPACE

class QPlaceSupplierPrivate : public QSharedData
{
public:
    bool operator==(const QPlaceSupplierPrivate &other) const;

    bool isEmpty() const;

    QString name;
    QString supplierId;
    QUrl url;
    QPlaceIcon icon;
};

QT_END_NAMESPACE

#endif // QPLACESUPPLIER_P_H
