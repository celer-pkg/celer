// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEICON_P_H
#define QPLACEICON_P_H

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

#include <QMap>
#include <QUrl>
#include <QSharedData>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QPlaceManager;
class QPlaceIconPrivate: public QSharedData
{
public:
    bool operator == (const QPlaceIconPrivate &other) const;

    QPlaceManager *manager = nullptr;
    QVariantMap parameters;
};

QT_END_NAMESPACE

#endif
