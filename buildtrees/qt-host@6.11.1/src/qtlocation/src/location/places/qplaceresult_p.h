// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACERESULT_P_H
#define QPLACERESULT_P_H

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

#include "qplacesearchresult_p.h"
#include "qplace.h"

QT_BEGIN_NAMESPACE

class QPlaceResultPrivate : public QPlaceSearchResultPrivate
{
public:
    bool compare(const QPlaceSearchResultPrivate *other) const override;

    Q_DEFINE_SEARCHRESULT_PRIVATE_HELPER(QPlaceResult, QPlaceSearchResult::PlaceResult)

    qreal distance = Q_QNAN;
    QPlace place;
    bool sponsored = false;
};

QT_END_NAMESPACE

#endif
