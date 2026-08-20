// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgeomapreplyosm.h"

#include <QtLocation/private/qgeotilespec_p.h>

QGeoMapReplyOsm::QGeoMapReplyOsm(QNetworkReply *reply,
                                 const QGeoTileSpec &spec,
                                 const QString &imageFormat,
                                 QObject *parent)
:   QGeoTiledMapReply(spec, parent)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &QGeoMapReplyOsm::networkReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoMapReplyOsm::networkReplyError);
    connect(this, &QGeoTiledMapReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    setMapImageFormat(imageFormat);
}

QGeoMapReplyOsm::~QGeoMapReplyOsm()
{
}

void QGeoMapReplyOsm::networkReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) // Already handled in networkReplyError
        return;

    QByteArray a = reply->readAll();

    setMapImageData(a);
    setFinished(true);
}

void QGeoMapReplyOsm::networkReplyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (error == QNetworkReply::OperationCanceledError)
        setFinished(true);
    else
        setError(QGeoTiledMapReply::CommunicationError, reply->errorString());

}
