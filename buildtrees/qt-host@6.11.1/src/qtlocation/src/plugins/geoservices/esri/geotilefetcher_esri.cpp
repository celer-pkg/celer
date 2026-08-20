// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geotilefetcher_esri.h"
#include "geotiledmappingmanagerengine_esri.h"
#include "geotiledmapreply_esri.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QtLocation/private/qgeotilespec_p.h>

QT_BEGIN_NAMESPACE

GeoTileFetcherEsri::GeoTileFetcherEsri(QGeoTiledMappingManagerEngine *parent) :
    QGeoTileFetcher(parent), m_networkManager(new QNetworkAccessManager(this)),
    m_userAgent(QByteArrayLiteral("Qt Location based application"))
{
}

QGeoTiledMapReply *GeoTileFetcherEsri::getTileImage(const QGeoTileSpec &spec)
{
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent());

    GeoTiledMappingManagerEngineEsri *engine = qobject_cast<GeoTiledMappingManagerEngineEsri *>(
          parent());

    GeoMapSource *mapSource = engine->mapSource(spec.mapId());

    if (!mapSource)
        qWarning("Unknown mapId %d\n", spec.mapId());
    else
        request.setUrl(mapSource->url().arg(spec.zoom()).arg(spec.x()).arg(spec.y()));

    QNetworkReply *reply = m_networkManager->get(request);

    return new GeoTiledMapReplyEsri(reply, spec);
}

QT_END_NAMESPACE
