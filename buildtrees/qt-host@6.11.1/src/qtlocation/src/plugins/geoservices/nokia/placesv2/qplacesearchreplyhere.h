// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACESEARCHREPLYHERE_H
#define QPLACESEARCHREPLYHERE_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QPlaceSearchReply>

QT_BEGIN_NAMESPACE

class QPlaceManagerEngineNokiaV2;
class QPlaceResult;
class QPlaceProposedSearchResult;

class QPlaceSearchReplyHere : public QPlaceSearchReply
{
    Q_OBJECT

public:
    explicit QPlaceSearchReplyHere(const QPlaceSearchRequest &request,
                                   QNetworkReply *reply,
                                   QPlaceManagerEngineNokiaV2 *parent);
    ~QPlaceSearchReplyHere();

private slots:
    void setError(QPlaceReply::Error error_, const QString &errorString);
    void replyFinished();
    void replyError(QNetworkReply::NetworkError error);

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;
    QPlaceProposedSearchResult parseSearchResult(const QJsonObject &item) const;

    QPlaceManagerEngineNokiaV2 *m_engine;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYHERE_H
