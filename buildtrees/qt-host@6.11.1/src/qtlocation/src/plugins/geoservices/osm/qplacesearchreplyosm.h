// Copyright (C) 2016 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHREPLYOSM_H
#define QPLACESEARCHREPLYOSM_H

#include <QtLocation/QPlaceSearchReply>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineOsm;
class QPlaceResult;

class QPlaceSearchReplyOsm : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyOsm(const QPlaceSearchRequest &request, QNetworkReply *reply,
                          QPlaceManagerEngineOsm *parent);
    ~QPlaceSearchReplyOsm();

    QString requestUrl;

private slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
    void replyFinished();
    void networkError(QNetworkReply::NetworkError error);

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYOSM_H
