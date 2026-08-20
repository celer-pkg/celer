// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODINGMANAGER_NOKIA_H
#define QGEOCODINGMANAGER_NOKIA_H

#include "qgeoserviceproviderplugin_nokia.h"

#include <qgeoserviceprovider.h>
#include <qgeocodingmanagerengine.h>

#include <QLocale>

QT_BEGIN_NAMESPACE

class QGeoNetworkAccessManager;
class QGeoUriProvider;

class QGeoCodingManagerEngineNokia : public QGeoCodingManagerEngine
{
    Q_OBJECT
public:
    QGeoCodingManagerEngineNokia(QGeoNetworkAccessManager *networkManager,
                                 const QVariantMap &parameters,
                                 QGeoServiceProvider::Error *error,
                                 QString *errorString);
    ~QGeoCodingManagerEngineNokia();

    QGeoCodeReply *geocode(const QGeoAddress &address,
                             const QGeoShape &bounds) override;
    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) override;

    QGeoCodeReply *geocode(const QString &searchString,
                            int limit,
                            int offset,
                            const QGeoShape &bounds) override;

private Q_SLOTS:
    void placesFinished();
    void placesError(QGeoCodeReply::Error error, const QString &errorString);

private:
    static QString trimDouble(double degree, int decimalDigits = 10);
    QGeoCodeReply *geocode(QString requestString, const QGeoShape &bounds, bool manualBoundsRequired = true, int limit = -1, int offset = 0);
    QString languageToMarc(QLocale::Language language);
    QString getAuthenticationString() const;

    QGeoNetworkAccessManager *m_networkManager;
    QGeoUriProvider *m_uriProvider;
    QGeoUriProvider *m_reverseGeocodingUriProvider;
    QString m_apiKey;
};

QT_END_NAMESPACE

#endif
