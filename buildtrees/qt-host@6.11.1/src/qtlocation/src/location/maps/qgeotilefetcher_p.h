// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEFETCHER_H
#define QGEOTILEFETCHER_H

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

#include <QObject>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qabstractgeotilecache_p.h>

QT_BEGIN_NAMESPACE

class QGeoMapRequestOptions;
class QGeoTileFetcherPrivate;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMapReply;
class QGeoTileSpec;

class Q_LOCATION_EXPORT QGeoTileFetcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoTileFetcher)

public:
    QGeoTileFetcher(QGeoMappingManagerEngine *parent);
    virtual ~QGeoTileFetcher();

public Q_SLOTS:
    void updateTileRequests(const QSet<QGeoTileSpec> &tilesAdded, const QSet<QGeoTileSpec> &tilesRemoved);

private Q_SLOTS:
    void cancelTileRequests(const QSet<QGeoTileSpec> &tiles);
    void requestNextTile();
    void finished();

Q_SIGNALS:
    void tileFinished(const QGeoTileSpec &spec, const QByteArray &bytes, const QString &format);
    void tileError(const QGeoTileSpec &spec, const QString &errorString);

protected:
    QGeoTileFetcher(QGeoTileFetcherPrivate &dd, QGeoMappingManagerEngine *parent);

    void timerEvent(QTimerEvent *event) override;
    QAbstractGeoTileCache::CacheAreas cacheHint() const;
    virtual bool initialized() const;
    virtual bool fetchingEnabled() const;

private:

    virtual QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec) = 0;
    virtual void handleReply(QGeoTiledMapReply *reply, const QGeoTileSpec &spec);

    Q_DISABLE_COPY(QGeoTileFetcher)
    friend class QGeoTiledMappingManagerEngine;
};

QT_END_NAMESPACE

#endif
