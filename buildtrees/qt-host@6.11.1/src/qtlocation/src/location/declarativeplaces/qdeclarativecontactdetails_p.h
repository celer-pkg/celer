// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVECONTACTDETAIL_P_H
#define QDECLARATIVECONTACTDETAIL_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtQml/QQmlPropertyMap>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QDeclarativeContactDetails : public QQmlPropertyMap
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContactDetails)
    QML_ADDED_IN_VERSION(5, 0)
    QML_UNCREATABLE("ContactDetails instances cannot be instantiated. "
                    "Only Place types have ContactDetails and they cannot "
                    "be re-assigned (but can be modified).")

public:
    explicit QDeclarativeContactDetails(QObject *parent = nullptr);
    QVariant updateValue(const QString &key, const QVariant &input) override;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeContactDetails)

#endif
