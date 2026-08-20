// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "geotiledmapreply_esri.h"

#include <QtLocation/private/qgeotilespec_p.h>

QT_BEGIN_NAMESPACE

static const unsigned char pngSignature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00};
static const unsigned char jpegSignature[] = {0xFF, 0xD8, 0xFF, 0x00};
static const unsigned char gifSignature[] = {0x47, 0x49, 0x46, 0x38, 0x00};

GeoTiledMapReplyEsri::GeoTiledMapReplyEsri(QNetworkReply *reply, const QGeoTileSpec &spec,
                                           QObject *parent) :
    QGeoTiledMapReply(spec, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &GeoTiledMapReplyEsri::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &GeoTiledMapReplyEsri::networkReplyError);
    connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

GeoTiledMapReplyEsri::~GeoTiledMapReplyEsri()
{
}

void GeoTiledMapReplyEsri::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QByteArray const& imageData = reply->readAll();

    bool validFormat = true;
    if (imageData.startsWith(reinterpret_cast<const char*>(pngSignature)))
        setMapImageFormat(QStringLiteral("png"));
    else if (imageData.startsWith(reinterpret_cast<const char*>(jpegSignature)))
        setMapImageFormat(QStringLiteral("jpg"));
    else if (imageData.startsWith(reinterpret_cast<const char*>(gifSignature)))
        setMapImageFormat(QStringLiteral("gif"));
    else
        validFormat = false;

    if (validFormat)
        setMapImageData(imageData);

    setFinished(true);
}

void GeoTiledMapReplyEsri::networkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (error == QNetworkReply::OperationCanceledError)
        setFinished(true);
    else
        setError(QGeoTiledMapReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
