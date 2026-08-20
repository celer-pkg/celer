// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOTILEDMAPREPLYESRI_H
#define GEOTILEDMAPREPLYESRI_H

#include <QNetworkReply>

#include <QtLocation/private/qgeotiledmapreply_p.h>

QT_BEGIN_NAMESPACE

class GeoTiledMapReplyEsri : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    GeoTiledMapReplyEsri(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent = nullptr);
    ~GeoTiledMapReplyEsri();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // GEOTILEDMAPREPLYESRI_H
