// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECATEGORY_P_H
#define QPLACECATEGORY_P_H

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

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtLocation/QLocation>

#include "qplaceicon.h"
#include "private/qglobal_p.h"

QT_BEGIN_NAMESPACE

class QPlaceCategoryPrivate : public QSharedData
{
public:
    bool operator==(const QPlaceCategoryPrivate &other) const;
    bool isEmpty() const;

    QString categoryId;
    QString name;
    QLocation::Visibility visibility = QLocation::UnspecifiedVisibility;
    QPlaceIcon icon;
};

QT_END_NAMESPACE

#endif // QPLACECATEGORY_P_H
