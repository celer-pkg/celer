// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOROUTINGMANAGER_NOKIA_H
#define QGEOROUTINGMANAGER_NOKIA_H

#include "qgeoserviceproviderplugin_nokia.h"

#include <qgeoserviceprovider.h>
#include <qgeoroutingmanagerengine.h>

QT_BEGIN_NAMESPACE

class QGeoNetworkAccessManager;
class QGeoUriProvider;

class QGeoRoutingManagerEngineNokia : public QGeoRoutingManagerEngine
{
    Q_OBJECT
public:
    QGeoRoutingManagerEngineNokia(QGeoNetworkAccessManager *networkInterface,
                                  const QVariantMap &parameters,
                                  QGeoServiceProvider::Error *error,
                                  QString *errorString);
    ~QGeoRoutingManagerEngineNokia();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request) override;
    QGeoRouteReply *updateRoute(const QGeoRoute &route, const QGeoCoordinate &position) override;

private Q_SLOTS:
    void routeFinished();
    void routeError(QGeoRouteReply::Error error, const QString &errorString);

private:
    QStringList calculateRouteRequestString(const QGeoRouteRequest &request) const;
    QStringList updateRouteRequestString(const QGeoRoute &route, const QGeoCoordinate &position);
    QString routeRequestString(const QGeoRouteRequest &request) const;
    bool checkEngineSupport(const QGeoRouteRequest &request,
                            QGeoRouteRequest::TravelModes travelModes) const;
    QString modesRequestString(const QGeoRouteRequest &request,
                               QGeoRouteRequest::TravelModes travelModes,
                               const QString &optimization) const;
    static QString trimDouble(double degree, int decimalDigits = 10);

    QGeoNetworkAccessManager *m_networkManager;
    QGeoUriProvider *m_uriProvider;
    QString m_apiKey;
};

QT_END_NAMESPACE

#endif
