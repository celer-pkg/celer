// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACERATINGS_P_H
#define QPLACERATINGS_P_H

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
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QPlaceRatingsPrivate : public QSharedData
{
public:
    bool operator==(const QPlaceRatingsPrivate &other) const;

    bool isEmpty() const;

    qreal average = 0;
    qreal maximum = 0;
    int count = 0;
};

QT_END_NAMESPACE

#endif // QPLACERATING_P_H
