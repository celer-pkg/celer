// Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPROPOSEDSEARCHRESULT_P_H
#define QPROPOSEDSEARCHRESULT_P_H

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

QT_BEGIN_NAMESPACE

class QPlaceProposedSearchResultPrivate : public QPlaceSearchResultPrivate
{
public:
    bool compare(const QPlaceSearchResultPrivate *other) const override;

    Q_DEFINE_SEARCHRESULT_PRIVATE_HELPER(QPlaceProposedSearchResult, QPlaceSearchResult::ProposedSearchResult)

    QPlaceSearchRequest searchRequest;
};

QT_END_NAMESPACE

#endif // QPROPOSEDSEARCHRESULT_P_H
