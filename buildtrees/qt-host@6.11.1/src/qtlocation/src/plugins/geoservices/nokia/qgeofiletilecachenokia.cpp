// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeofiletilecachenokia.h"
#include <QtLocation/private/qgeotilespec_p.h>
#include <QDir>

QT_BEGIN_NAMESPACE

QGeoFileTileCacheNokia::QGeoFileTileCacheNokia(int ppi, const QString &directory, QObject *parent)
    :QGeoFileTileCache(directory, parent)
{
    m_ppi = QString::number(ppi) + QLatin1String("p");
}

QGeoFileTileCacheNokia::~QGeoFileTileCacheNokia()
{

}

QString QGeoFileTileCacheNokia::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const
{
    QString filename = spec.plugin();
    filename += QLatin1String("-");
    filename += QString::number(spec.mapId());
    filename += QLatin1String("-");
    filename += QString::number(spec.zoom());
    filename += QLatin1String("-");
    filename += QString::number(spec.x());
    filename += QLatin1String("-");
    filename += QString::number(spec.y());

    //Append version if real version number to ensure backwards compatibility and eviction of old tiles
    if (spec.version() != -1) {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String("-");
    filename += m_ppi;

    filename += QLatin1String(".");
    filename += format;

    QDir dir = QDir(directory);

    return dir.filePath(filename);
}

QGeoTileSpec QGeoFileTileCacheNokia::filenameToTileSpec(const QString &filename) const
{
    QGeoTileSpec emptySpec;

    const QStringList parts = filename.split('.');

    if (parts.length() != 2)
        return emptySpec;

    const QString name = parts.at(0);
    const QStringList fields = name.split('-');

    const qsizetype length = fields.length();
    if (length != 6 && length != 7)
        return emptySpec;
    else if (fields.last() != m_ppi)
        return QGeoTileSpec();

    QList<int> numbers;

    bool ok = false;
    for (qsizetype i = 1; i < length-1; ++i) { // skipping -<ppi>
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    //File name without version, append default
    if (numbers.length() < 5)
        numbers.append(-1);

    return QGeoTileSpec(fields.at(0),
                    numbers.at(0),
                    numbers.at(1),
                    numbers.at(2),
                    numbers.at(3),
                    numbers.at(4));
}

QT_END_NAMESPACE
