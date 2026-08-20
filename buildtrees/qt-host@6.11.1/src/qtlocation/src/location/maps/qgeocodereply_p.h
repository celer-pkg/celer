// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODEREPLY_P_H
#define QGEOCODEREPLY_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include "qgeocodereply.h"

#include "qgeoshape.h"

#include <QList>

QT_BEGIN_NAMESPACE

class QGeoLocation;

class Q_LOCATION_EXPORT QGeoCodeReplyPrivate
{
public:
    QGeoCodeReplyPrivate();
    QGeoCodeReplyPrivate(QGeoCodeReply::Error error, const QString &errorString);

    static const QGeoCodeReplyPrivate *get(const QGeoCodeReply &reply);
    static QGeoCodeReplyPrivate *get(QGeoCodeReply &reply);

    QGeoCodeReply::Error error = QGeoCodeReply::NoError;
    QString errorString;
    bool isFinished = false;

    QGeoShape viewport;
    QList<QGeoLocation> locations;

    qsizetype limit = -1;
    qsizetype offset = 0;
private:
    Q_DISABLE_COPY(QGeoCodeReplyPrivate)
};

QT_END_NAMESPACE

#endif
