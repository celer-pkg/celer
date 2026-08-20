// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOFILETILECACHEOSM_H
#define QGEOFILETILECACHEOSM_H

#include "qgeotileproviderosm.h"
#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QHash>
#include <qatomic.h>
#include <QDir>

QT_BEGIN_NAMESPACE

class QGeoFileTileCacheOsm : public QGeoFileTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCacheOsm(const QList<QGeoTileProviderOsm *> &providers,
                         const QString &offlineDirectory = QString(),
                         const QString &directory = QString(), QObject *parent = nullptr);
    ~QGeoFileTileCacheOsm();

    QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) override;

Q_SIGNALS:
    void mapDataUpdated(int mapId);

protected Q_SLOTS:
    void onProviderResolutionFinished(const QGeoTileProviderOsm *provider);
    void onProviderResolutionError(const QGeoTileProviderOsm *provider, QNetworkReply::NetworkError error);

protected:
    void init() override;
    inline QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, int providerId) const;
    QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;
    QSharedPointer<QGeoTileTexture> getFromOfflineStorage(const QGeoTileSpec &spec);
    void dropTiles(int mapId);
    void loadTiles(int mapId);

    void clearObsoleteTiles(const QGeoTileProviderOsm *p);

    QDir m_offlineDirectory;
    bool m_offlineData;
    QList<QGeoTileProviderOsm *> m_providers;
    QList<bool> m_highDpi;
    QList<QDateTime> m_maxMapIdTimestamps;
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHEOSM_H
