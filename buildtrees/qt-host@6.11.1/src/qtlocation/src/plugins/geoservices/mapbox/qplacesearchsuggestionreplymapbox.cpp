// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qplacesearchsuggestionreplymapbox.h"
#include "qplacemanagerenginemapbox.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceSearchRequest>

QT_BEGIN_NAMESPACE

QPlaceSearchSuggestionReplyMapbox::QPlaceSearchSuggestionReplyMapbox(QNetworkReply *reply, QPlaceManagerEngineMapbox *parent)
:   QPlaceSearchSuggestionReply(parent)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }

    connect(reply, &QNetworkReply::finished, this, &QPlaceSearchSuggestionReplyMapbox::onReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &QPlaceSearchSuggestionReplyMapbox::onNetworkError);

    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QPlaceSearchSuggestionReplyMapbox::~QPlaceSearchSuggestionReplyMapbox()
{
}

void QPlaceSearchSuggestionReplyMapbox::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit errorOccurred(errorCode, errorString);

    setFinished(true);
    emit finished();
}

void QPlaceSearchSuggestionReplyMapbox::onReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    const QJsonArray features = document.object().value(QStringLiteral("features")).toArray();

    QStringList suggestions;
    for (const QJsonValueConstRef feature : features) {
        if (feature.isObject())
            suggestions.append(feature.toObject().value(QStringLiteral("text")).toString());
    }

    setSuggestions(suggestions);

    setFinished(true);
    emit finished();
}

void QPlaceSearchSuggestionReplyMapbox::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
