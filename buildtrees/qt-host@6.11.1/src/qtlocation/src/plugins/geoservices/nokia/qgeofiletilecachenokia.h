// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOFILETILECACHENOKIA_H
#define QGEOFILETILECACHENOKIA_H

#include <QtLocation/private/qgeofiletilecache_p.h>

QT_BEGIN_NAMESPACE

class QGeoFileTileCacheNokia : public QGeoFileTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCacheNokia(int ppi, const QString &directory = QString(),
                           QObject *parent = nullptr);
    ~QGeoFileTileCacheNokia();

protected:
    QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format,
                               const QString &directory) const override;
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;

    QString m_ppi;
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHENOKIA_H
