// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOCODINGMANAGERENGINEESRI_H
#define GEOCODINGMANAGERENGINEESRI_H

#include <QGeoServiceProvider>
#include <QGeoCodingManagerEngine>
#include <QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class GeoCodingManagerEngineEsri : public QGeoCodingManagerEngine
{
    Q_OBJECT

public:
    GeoCodingManagerEngineEsri(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    virtual ~GeoCodingManagerEngineEsri();

    QGeoCodeReply *geocode(const QGeoAddress &address, const QGeoShape &bounds) override;
    QGeoCodeReply *geocode(const QString &address, int limit, int offset,
                           const QGeoShape &bounds) override;
    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) override;

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoCodeReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
};

QT_END_NAMESPACE

#endif // GEOCODINGMANAGERENGINEESRI_H
