// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEDMAPREPLY_P_H
#define QGEOTILEDMAPREPLY_P_H

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

#include "qgeotiledmapreply_p.h"
#include "qgeotilespec_p.h"

QT_BEGIN_NAMESPACE

class QGeoTiledMapReplyPrivate
{
public:
    QGeoTiledMapReplyPrivate(const QGeoTileSpec &spec);
    QGeoTiledMapReplyPrivate(QGeoTiledMapReply::Error error, const QString &errorString);

    QGeoTiledMapReply::Error error = QGeoTiledMapReply::NoError;
    QString errorString;
    bool isFinished = false;
    bool isCached = false;

    QGeoTileSpec spec;
    QByteArray mapImageData;
    QString mapImageFormat;
};

QT_END_NAMESPACE

#endif
