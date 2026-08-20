// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTINGMANAGERENGINEOSM_H
#define QGEOROUTINGMANAGERENGINEOSM_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoRoutingManagerEngine>
#include <QtLocation/private/qgeorouteparser_p.h>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class QGeoRoutingManagerEngineOsm : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    QGeoRoutingManagerEngineOsm(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error,
                                QString *errorString);
    ~QGeoRoutingManagerEngineOsm();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request) override;
    const QGeoRouteParser *routeParser() const;

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QGeoRouteParser *m_routeParser;
    QByteArray m_userAgent;
    QString m_urlPrefix;
};

QT_END_NAMESPACE

#endif // QGEOROUTINGMANAGERENGINEOSM_H

