// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHREQUEST_P_H
#define QPLACESEARCHREQUEST_P_H

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

#include "qplacesearchrequest.h"
#include "qgeocoordinate.h"
#include "qgeoshape.h"

#include <QtCore/QSharedData>
#include <QtCore/QList>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtCore/QVariant>
#include <QtLocation/QGeoRoute>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QPlaceSearchRequestPrivate : public QSharedData
{
public:
    bool operator==(const QPlaceSearchRequestPrivate &other) const;

    void clear();
    static const QPlaceSearchRequestPrivate *get(const QPlaceSearchRequest &request);
    static QPlaceSearchRequestPrivate *get(QPlaceSearchRequest &request);

    QString searchTerm;
    QList<QPlaceCategory> categories;
    QGeoShape searchArea;
    QString recommendationId;
    QLocation::VisibilityScope visibilityScope = QLocation::UnspecifiedVisibility;
    QPlaceSearchRequest::RelevanceHint relevanceHint = QPlaceSearchRequest::UnspecifiedHint;
    QGeoRoute routeSearchArea;
    int limit = -1;
    QVariant searchContext;
    bool related = false;
    int page = 0;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREQUEST_P_H
