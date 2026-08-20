// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOTILEREQUESTMANAGER_P_H
#define QGEOTILEREQUESTMANAGER_P_H

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

#include <QtCore/QSharedPointer>
#include <QtLocation/private/qlocationglobal_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMap;
class QGeoTiledMappingManagerEngine;
class QGeoTileSpec;
struct QGeoTileTexture;

class QGeoTileRequestManagerPrivate;

class Q_LOCATION_EXPORT QGeoTileRequestManager
{
public:
    explicit QGeoTileRequestManager(QGeoTiledMap *map, QGeoTiledMappingManagerEngine *engine);
    ~QGeoTileRequestManager();

    QMap<QGeoTileSpec, QSharedPointer<QGeoTileTexture> > requestTiles(const QSet<QGeoTileSpec> &tiles);

    void tileError(const QGeoTileSpec &tile, const QString &errorString);
    void tileFetched(const QGeoTileSpec &spec);
    QSharedPointer<QGeoTileTexture> tileTexture(const QGeoTileSpec &spec);

private:
    std::unique_ptr<QGeoTileRequestManagerPrivate> d_ptr;
    Q_DISABLE_COPY(QGeoTileRequestManager)
};

QT_END_NAMESPACE

#endif // QGEOTILEREQUESTMANAGER_P_H
