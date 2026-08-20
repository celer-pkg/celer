// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#include "qabstractgeotilecache_p.h"

#include "qgeotilespec_p.h"

#include "qgeomappingmanager_p.h"

#include <QDir>
#include <QStandardPaths>
#include <QMetaType>
#include <QPixmap>
#include <QDebug>
#include <QtCore/qtemporaryfile.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QAbstractGeoTileCache::QAbstractGeoTileCache(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QGeoTileSpec>();
    qRegisterMetaType<QList<QGeoTileSpec> >();
    qRegisterMetaType<QSet<QGeoTileSpec> >();
}

QAbstractGeoTileCache::~QAbstractGeoTileCache()
{
}

void QAbstractGeoTileCache::printStats()
{
}

void QAbstractGeoTileCache::handleError(const QGeoTileSpec &, const QString &error)
{
    qWarning() << "tile request error " << error;
}

void QAbstractGeoTileCache::setMaxDiskUsage(int diskUsage)
{
    Q_UNUSED(diskUsage);
}

int QAbstractGeoTileCache::maxDiskUsage() const
{
    return 0;
}

int QAbstractGeoTileCache::diskUsage() const
{
    return 0;
}

void QAbstractGeoTileCache::setMaxMemoryUsage(int memoryUsage)
{
    Q_UNUSED(memoryUsage);
}

int QAbstractGeoTileCache::maxMemoryUsage() const
{
    return 0;
}

int QAbstractGeoTileCache::memoryUsage() const
{
    return 0;
}

QString QAbstractGeoTileCache::baseCacheDirectory()
{
    QString dir;

    // Try the shared cache first and use a specific directory. (e.g. ~/.cache/QtLocation)
    // If this is not supported by the platform, use the application-specific cache
    // location. (e.g. ~/.cache/<app_name>/QtLocation)
    dir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);

    if (!dir.isEmpty()) {
        // The shared cache may not be writable when application isolation is enforced.
        static bool writable = false;
        static bool writableChecked = false;
        if (!writableChecked) {
            writableChecked = true;
            auto mkpath = [] (const QString &dir) {
                const auto rt = QDir::root();
                return rt.mkpath(dir);
            };
            if (mkpath(dir)) {
                QTemporaryFile tmp(QDir(dir).filePath(u"qt_cache_check.XXXXXX"_s));
                writable = tmp.open();
            } else {
                writable = false;
            }
        }
        if (!writable)
            dir = QString();
    }

    if (dir.isEmpty())
        dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    if (!dir.endsWith(QLatin1Char('/')))
        dir += QLatin1Char('/');

    return dir;
}

QString QAbstractGeoTileCache::baseLocationCacheDirectory()
{
    // This scheme allows to have the "tiles" prefix hardcoded here
    // NOTE: changing the Qt version here requires changing it also in QGeoFileTileCache::init,
    // in the code that remove old version tiles !
    return baseCacheDirectory() + QLatin1String("QtLocation/5.8/tiles/");
}

QT_END_NAMESPACE
