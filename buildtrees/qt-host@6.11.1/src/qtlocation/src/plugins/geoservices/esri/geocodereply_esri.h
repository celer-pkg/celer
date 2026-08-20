// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOCODEREPLYESRI_H
#define GEOCODEREPLYESRI_H

#include <QNetworkReply>
#include <QGeoCodeReply>

QT_BEGIN_NAMESPACE

class QGeoLocation;

class GeoCodeReplyEsri : public QGeoCodeReply
{
    Q_OBJECT
    Q_MOC_INCLUDE(<QGeoLocation>)

public:
    enum OperationType
    {
        Geocode,
        ReverseGeocode
    };

public:
    GeoCodeReplyEsri(QNetworkReply *reply, OperationType operationType, QObject *parent = nullptr);
    ~GeoCodeReplyEsri();

    inline OperationType operationType() const;

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

    QGeoLocation parseAddress(const QJsonObject &object);
    QGeoLocation parseCandidate(const QJsonObject &candidate);

private:
    OperationType m_operationType;
};

inline GeoCodeReplyEsri::OperationType GeoCodeReplyEsri::operationType() const
{
    return m_operationType;
}

QT_END_NAMESPACE

#endif // GEOCODEREPLYESRI_H
