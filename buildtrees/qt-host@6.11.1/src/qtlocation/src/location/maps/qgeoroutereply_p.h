// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEREPLY_P_H
#define QGEOROUTEREPLY_P_H

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

#include "qgeorouterequest.h"
#include "qgeoroutereply.h"

#include <QList>

QT_BEGIN_NAMESPACE

class QGeoRoute;

class QGeoRouteReplyPrivate
{
public:
    explicit QGeoRouteReplyPrivate(const QGeoRouteRequest &request);
    QGeoRouteReplyPrivate(QGeoRouteReply::Error error, QString errorString);

    QGeoRouteReply::Error error = QGeoRouteReply::NoError;
    QString errorString;
    bool isFinished = false;

    QGeoRouteRequest request;
    QList<QGeoRoute> routes;

private:
    Q_DISABLE_COPY(QGeoRouteReplyPrivate)
};

QT_END_NAMESPACE

#endif
