// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeocodereply_nokia.h"
#include "qgeocodejsonparser.h"
#include "qgeoerror_messages.h"

#include <QtPositioning/QGeoShape>
#include <QtCore/QCoreApplication>

Q_DECLARE_METATYPE(QList<QGeoLocation>)

QT_BEGIN_NAMESPACE

// manualBoundsRequired will be true if the parser has to manually
// check if a given result lies within the viewport bounds,
// and false if the bounds information was able to be supplied
// to the server in the request (so it should not return any
// out-of-bounds results).
QGeoCodeReplyNokia::QGeoCodeReplyNokia(QNetworkReply *reply, int limit, int offset,
                                       const QGeoShape &viewport, bool manualBoundsRequired,
                                       QObject *parent)
:   QGeoCodeReply(parent), m_parsing(false), m_manualBoundsRequired(manualBoundsRequired)
{
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    qRegisterMetaType<QList<QGeoLocation> >();

    connect(reply, &QNetworkReply::finished,
            this, &QGeoCodeReplyNokia::networkFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QGeoCodeReplyNokia::networkError);
    connect(this, &QGeoCodeReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QGeoCodeReply::aborted, [this](){ m_parsing = false; });
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);


    setLimit(limit);
    setOffset(offset);
    setViewport(viewport);
}

QGeoCodeReplyNokia::~QGeoCodeReplyNokia()
{
}

void QGeoCodeReplyNokia::networkFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QGeoCodeJsonParser *parser = new QGeoCodeJsonParser; // QRunnable, autoDelete = true.
    if (m_manualBoundsRequired)
        parser->setBounds(viewport());
    connect(parser, &QGeoCodeJsonParser::results, this, &QGeoCodeReplyNokia::appendResults);
    connect(parser, &QGeoCodeJsonParser::errorOccurred, this, &QGeoCodeReplyNokia::parseError);

    m_parsing = true;
    parser->parse(reply->readAll());
}

void QGeoCodeReplyNokia::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoCodeReply::CommunicationError, reply->errorString());
}

void QGeoCodeReplyNokia::appendResults(const QList<QGeoLocation> &locations)
{
    if (!m_parsing)
        return;

    m_parsing = false;
    setLocations(locations);
    setFinished(true);
}

void QGeoCodeReplyNokia::parseError(const QString &errorString)
{
    Q_UNUSED(errorString);

    setError(QGeoCodeReply::ParseError,
             QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, RESPONSE_NOT_RECOGNIZABLE));
}

QT_END_NAMESPACE
