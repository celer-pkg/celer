// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PLACESEARCHREPLYESRI_H
#define PLACESEARCHREPLYESRI_H

#include <QtLocation/QPlaceSearchReply>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class PlaceManagerEngineEsri;
class QNetworkReply;
class QPlaceResult;

class PlaceSearchReplyEsri : public QPlaceSearchReply
{
    Q_OBJECT

public:
    PlaceSearchReplyEsri(const QPlaceSearchRequest &request, QNetworkReply *reply,
                         const QHash<QString, QString> &candidateFields,
                         const QHash<QString, QString> &countries, PlaceManagerEngineEsri *parent);
    ~PlaceSearchReplyEsri();

    QString requestUrl;

private slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
    void replyFinished();
    void networkError(QNetworkReply::NetworkError error);

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;

    const QHash<QString, QString> &m_candidateFields;
    const QHash<QString, QString> &m_countries;
};

QT_END_NAMESPACE

#endif // PLACESEARCHREPLYESRI_H
