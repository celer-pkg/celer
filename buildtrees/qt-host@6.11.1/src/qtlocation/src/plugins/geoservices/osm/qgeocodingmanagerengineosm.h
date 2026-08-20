// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODINGMANAGERENGINEOSM_H
#define QGEOCODINGMANAGERENGINEOSM_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoCodingManagerEngine>
#include <QtLocation/QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;

class QGeoCodingManagerEngineOsm : public QGeoCodingManagerEngine
{
    Q_OBJECT

public:
    QGeoCodingManagerEngineOsm(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    ~QGeoCodingManagerEngineOsm();

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
    QString m_urlPrefix;
    bool m_debugQuery = false;
    bool m_includeExtraData = false;
};

QT_END_NAMESPACE

#endif // QGEOCODINGMANAGERENGINEOSM_H
