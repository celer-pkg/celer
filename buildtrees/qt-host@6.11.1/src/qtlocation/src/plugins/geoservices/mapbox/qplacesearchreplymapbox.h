// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACESEARCHREPLYMAPBOX_H
#define QPLACESEARCHREPLYMAPBOX_H

#include <QtLocation/QPlaceSearchReply>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineMapbox;
class QPlaceResult;

class QPlaceSearchReplyMapbox : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyMapbox(const QPlaceSearchRequest &request, QNetworkReply *reply,
                          QPlaceManagerEngineMapbox *parent);
    ~QPlaceSearchReplyMapbox();

public slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);

private slots:
    void onReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYMAPBOX_H
