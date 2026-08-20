// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODINGMANAGERENGINEMAPBOX_H
#define QGEOCODINGMANAGERENGINEMAPBOX_H

#include <QtCore/QUrlQuery>
#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoCodingManagerEngine>
#include <QtLocation/QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class QGeoCodingManagerEngineMapbox : public QGeoCodingManagerEngine
{
    Q_OBJECT

public:
    QGeoCodingManagerEngineMapbox(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    ~QGeoCodingManagerEngineMapbox();

    QGeoCodeReply *geocode(const QGeoAddress &address, const QGeoShape &bounds) override;
    QGeoCodeReply *geocode(const QString &address, int limit, int offset,
                           const QGeoShape &bounds) override;
    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) override;

private slots:
    void onReplyFinished();
    void onReplyError(QGeoCodeReply::Error errorCode, const QString &errorString);

private:
    QGeoCodeReply *doSearch(const QString &, QUrlQuery &, const QGeoShape &bounds);

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_accessToken;
    QString m_urlPrefix;
    bool m_isEnterprise;
};

QT_END_NAMESPACE

#endif // QGEOCODINGMANAGERENGINEMAPBOX_H
