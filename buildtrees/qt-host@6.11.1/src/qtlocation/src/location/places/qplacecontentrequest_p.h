// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTENTREQUEST_P_H
#define QPLACECONTENTREQUEST_P_H

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
#include <QtCore/QVariant>
#include <QtLocation/QPlaceContent>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QPlaceContentRequestPrivate : public QSharedData
{
public:
    bool operator==(const QPlaceContentRequestPrivate &other) const;

    void clear();

    QPlaceContent::Type contentType = QPlaceContent::NoType;
    QString placeId;
    QVariant contentContext;
    int limit = -1;
};

QT_END_NAMESPACE

#endif
