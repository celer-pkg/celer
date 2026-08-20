// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geomapsource.h"

#include <QUrl>

QT_BEGIN_NAMESPACE

static const QString kArcGISTileScheme(QStringLiteral("/tile/${z}/${y}/${x}"));

struct MapStyleData
{
    QString name;
    QGeoMapType::MapStyle style;
};

static const MapStyleData mapStyles[] =
{
    { QStringLiteral("StreetMap"),              QGeoMapType::StreetMap },
    { QStringLiteral("SatelliteMapDay"),        QGeoMapType::SatelliteMapDay },
    { QStringLiteral("SatelliteMapNight"),      QGeoMapType::SatelliteMapNight },
    { QStringLiteral("TerrainMap"),             QGeoMapType::TerrainMap },
    { QStringLiteral("HybridMap"),              QGeoMapType::HybridMap },
    { QStringLiteral("TransitMap"),             QGeoMapType::TransitMap },
    { QStringLiteral("GrayStreetMap"),          QGeoMapType::GrayStreetMap },
    { QStringLiteral("PedestrianMap"),          QGeoMapType::PedestrianMap },
    { QStringLiteral("CarNavigationMap"),       QGeoMapType::CarNavigationMap },
    { QStringLiteral("CustomMap"),              QGeoMapType::CustomMap }
};

GeoMapSource::GeoMapSource(QGeoMapType::MapStyle style, const QString &name,
                           const QString &description, bool mobile, bool night, int mapId,
                           const QString &url, const QString &copyright, const QGeoCameraCapabilities &cameraCapabilities) :
    QGeoMapType(style, name, description, mobile, night, mapId, "esri", cameraCapabilities),
    m_url(url), m_copyright(copyright)
{
}

QString GeoMapSource::toFormat(const QString &url)
{
    QString format = url;

    if (!format.contains(QLatin1String("${")))
        format += kArcGISTileScheme;

    format.replace(QLatin1String("${z}"), QLatin1String("%1"));
    format.replace(QLatin1String("${x}"), QLatin1String("%2"));
    format.replace(QLatin1String("${y}"), QLatin1String("%3"));
    format.replace(QLatin1String("${token}"), QLatin1String("%4"));

    return format;
}

QGeoMapType::MapStyle GeoMapSource::mapStyle(const QString &styleString)
{
    for (const MapStyleData &mapStyle : mapStyles) {
        if (styleString.compare(mapStyle.name, Qt::CaseInsensitive) == 0)
            return mapStyle.style;
    }

    QGeoMapType::MapStyle style = static_cast<QGeoMapType::MapStyle>(styleString.toInt());
    if (style <= QGeoMapType::NoMap)
        style = QGeoMapType::CustomMap;

    return style;
}

QT_END_NAMESPACE
