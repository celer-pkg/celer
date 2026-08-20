// Copyright (C) 2016 Vlad Seryakov <vseryakov@gmail.com>
// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOROUTINGMANAGERENGINEMAPBOX_H
#define QGEOROUTINGMANAGERENGINEMAPBOX_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoRoutingManagerEngine>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;
class QGeoRouteParser;

class QGeoRoutingManagerEngineMapbox : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    QGeoRoutingManagerEngineMapbox(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error,
                                QString *errorString);
    ~QGeoRoutingManagerEngineMapbox();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request) override;
    const QGeoRouteParser *routeParser() const;

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_accessToken;
    bool m_useMapboxText = false;
    QGeoRouteParser *m_routeParser = nullptr;
};

QT_END_NAMESPACE

#endif // QGEOROUTINGMANAGERENGINEOSM_H

