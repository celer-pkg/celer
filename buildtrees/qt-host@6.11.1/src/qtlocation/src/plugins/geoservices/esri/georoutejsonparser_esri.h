// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOROUTEJSONPARSERESRI_H
#define GEOROUTEJSONPARSERESRI_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QGeoRoute>
#include <QMap>

QT_BEGIN_NAMESPACE

class GeoRouteJsonParserEsri
{
public:
    GeoRouteJsonParserEsri(const QJsonDocument &document);

    QList<QGeoRoute> routes() const;
    bool isValid() const;
    QString errorString() const;

private:
    void parseDirections();
    void parseDirection(const QJsonObject &direction);
    void parseRoutes();
    void parseRoute(const QJsonObject &route);

    QString m_error;
    QMap<int, QGeoRoute> m_routes;
    QJsonObject m_json;
};

QT_END_NAMESPACE

#endif // GEOROUTEJSONPARSERESRI_H
