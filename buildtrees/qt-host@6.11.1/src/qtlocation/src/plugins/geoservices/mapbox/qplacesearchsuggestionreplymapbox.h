// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACESEARCHSUGGESTIONREPLYMAPBOX_H
#define QPLACESEARCHSUGGESTIONREPLYMAPBOX_H

#include <QtLocation/QPlaceSearchSuggestionReply>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineMapbox;
class QPlaceResult;

class QPlaceSearchSuggestionReplyMapbox : public QPlaceSearchSuggestionReply
{
    Q_OBJECT

public:
    QPlaceSearchSuggestionReplyMapbox(QNetworkReply *reply, QPlaceManagerEngineMapbox *parent);
    ~QPlaceSearchSuggestionReplyMapbox();

public slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);

private slots:
    void onReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QPLACESEARCHSUGGESTIONREPLYMAPBOX_H
