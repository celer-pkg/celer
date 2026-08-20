// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEATTRIBUTE_P_H
#define QPLACEATTRIBUTE_P_H

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

#include <QSharedData>
#include <QString>
#include <QVariant>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QPlaceAttributePrivate : public QSharedData
{
public:
    bool operator== (const QPlaceAttributePrivate &other) const;

    bool isEmpty() const;

    QString label;
    QString text;
};

QT_END_NAMESPACE

#endif

