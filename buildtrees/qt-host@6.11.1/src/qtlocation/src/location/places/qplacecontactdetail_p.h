// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTACTDETAIL_P_H
#define QPLACECONTACTDETAIL_P_H


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
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QPlaceContactDetailPrivate : public QSharedData
{
public:
    bool operator== (const QPlaceContactDetailPrivate &other) const;

    QString label;
    QString value;
};

QT_END_NAMESPACE

#endif
