// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOROUTEREPLYESRI_H
#define GEOROUTEREPLYESRI_H

#include <QNetworkReply>
#include <QGeoRouteReply>

QT_BEGIN_NAMESPACE

class GeoRouteReplyEsri : public QGeoRouteReply
{
    Q_OBJECT

public:
    GeoRouteReplyEsri(QNetworkReply *reply, const QGeoRouteRequest &request, QObject *parent = nullptr);
    ~GeoRouteReplyEsri();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // GEOROUTEREPLYESRI_H
