// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeofiletilecachemapbox.h"
#include <QtLocation/private/qgeotilespec_p.h>
#include <QDir>

QT_BEGIN_NAMESPACE

QGeoFileTileCacheMapbox::QGeoFileTileCacheMapbox(const QList<QGeoMapType> &mapTypes,
                                                 int scaleFactor, const QString &directory,
                                                 QObject *parent)
    :QGeoFileTileCache(directory, parent), m_mapTypes(mapTypes)
{
    m_scaleFactor = qBound(1, scaleFactor, 2);
    for (qsizetype i = 0; i < mapTypes.size(); i++)
        m_mapNameToId.insert(mapTypes[i].name(), i + 1);
}

QGeoFileTileCacheMapbox::~QGeoFileTileCacheMapbox()
{

}

QString QGeoFileTileCacheMapbox::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format,
                                                    const QString &directory) const
{
    QString filename = spec.plugin();
    filename += QLatin1String("-");
    filename += m_mapTypes[spec.mapId()-1].name();
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

    filename += QLatin1String("-@");
    filename += QString::number(m_scaleFactor);
    filename += QLatin1Char('x');

    filename += QLatin1String(".");
    filename += format;

    QDir dir = QDir(directory);

    return dir.filePath(filename);
}

QGeoTileSpec QGeoFileTileCacheMapbox::filenameToTileSpec(const QString &filename) const
{
    QStringList parts = filename.split('.');

    if (parts.length() != 3) // 3 because the map name has always a dot in it.
        return QGeoTileSpec();

    const QString name = parts.at(0) + QChar('.') + parts.at(1);
    const QStringList fields = name.split('-');

    const qsizetype length = fields.length();
    if (length != 6 && length != 7)
        return QGeoTileSpec();
    const qsizetype scaleIdx = fields.last().indexOf("@");
    if (scaleIdx < 0 || fields.last().size() <= (scaleIdx + 2))
        return QGeoTileSpec();
    const int scaleFactor = fields.last()[scaleIdx + 1].digitValue();
    if (scaleFactor != m_scaleFactor)
        return QGeoTileSpec();

    QList<int> numbers;

    bool ok = false;
    for (qsizetype i = 2; i < length - 1; ++i) { // skipping -@_X
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return QGeoTileSpec();
        numbers.append(value);
    }

    //File name without version, append default
    if (numbers.length() < 4)
        numbers.append(-1);

    return QGeoTileSpec(fields.at(0),
                    m_mapNameToId[fields.at(1)],
                    numbers.at(0),
                    numbers.at(1),
                    numbers.at(2),
                    numbers.at(3));
}

QT_END_NAMESPACE
