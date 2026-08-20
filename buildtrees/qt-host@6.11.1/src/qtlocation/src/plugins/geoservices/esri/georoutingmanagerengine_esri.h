// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOROUTINGMANAGERENGINEESRI_H
#define GEOROUTINGMANAGERENGINEESRI_H

#include <QGeoServiceProvider>
#include <QGeoRoutingManagerEngine>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class GeoRoutingManagerEngineEsri : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    GeoRoutingManagerEngineEsri(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                                QString *errorString);
    virtual ~GeoRoutingManagerEngineEsri();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request) override;

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QString preferedDirectionLangage();
    QString preferedDirectionsLengthUnits();

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_token;
};

QT_END_NAMESPACE

#endif // GEOROUTINGMANAGERENGINEESRI_H
