// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOFILETILECACHEMAPBOX_H
#define QGEOFILETILECACHEMAPBOX_H

#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QMap>

QT_BEGIN_NAMESPACE

class QGeoFileTileCacheMapbox : public QGeoFileTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCacheMapbox(const QList<QGeoMapType> &mapTypes, int scaleFactor, const QString &directory = QString(), QObject *parent = nullptr);
    ~QGeoFileTileCacheMapbox();

protected:
    QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const override;
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;

    QList<QGeoMapType> m_mapTypes;
    QMap<QString, int> m_mapNameToId;
    int m_scaleFactor;
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHEMAPBOX_H
