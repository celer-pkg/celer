// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoroutereply_nokia.h"
#include "qgeoroutexmlparser.h"
#include "qgeoerror_messages.h"

#include <qgeorouterequest.h>

#include <QtCore/QCoreApplication>

Q_DECLARE_METATYPE(QList<QGeoRoute>)

QT_BEGIN_NAMESPACE

QGeoRouteReplyNokia::QGeoRouteReplyNokia(const QGeoRouteRequest &request,
                                         const QList<QNetworkReply *> &replies,
                                         QObject *parent)
:   QGeoRouteReply(request, parent), m_parsers(0)
{
    qRegisterMetaType<QList<QGeoRoute> >();

    bool failure = false;
    for (QNetworkReply *reply : replies) {
        if (!reply) {
            failure = true;
            continue;
        }
        connect(reply, &QNetworkReply::finished,
                this, &QGeoRouteReplyNokia::networkFinished);
        connect(reply, &QNetworkReply::errorOccurred,
                this, &QGeoRouteReplyNokia::networkError);
        connect(this, &QGeoRouteReply::aborted, reply, &QNetworkReply::abort);
        connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
    }
    if (failure)
        setError(UnknownError, QStringLiteral("Null reply"));
    else
        connect(this, &QGeoRouteReply::aborted, [this](){ m_parsers = 0; });
}

QGeoRouteReplyNokia::~QGeoRouteReplyNokia()
{
}

void QGeoRouteReplyNokia::networkFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError
        && reply->error() != QNetworkReply::UnknownContentError) {
        return;
    }

    QGeoRouteXmlParser *parser = new QGeoRouteXmlParser(request());
    connect(parser, &QGeoRouteXmlParser::results,
            this, &QGeoRouteReplyNokia::appendResults);
    connect(parser, &QGeoRouteXmlParser::errorOccurred,
            this, &QGeoRouteReplyNokia::parserError);

    ++m_parsers;
    parser->parse(reply->readAll());
}

void QGeoRouteReplyNokia::networkError(QNetworkReply::NetworkError error)
{
    if (error == QNetworkReply::UnknownContentError)
        return;
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(QGeoRouteReply::CommunicationError, reply->errorString());
    if (error != QNetworkReply::OperationCanceledError) // Any error not caused by abort()
        emit aborted(); // aborts all unfinished replies and sets m_parsers to 0
}

void QGeoRouteReplyNokia::appendResults(const QList<QGeoRoute> &routes)
{
    if (!m_parsers)
        return;

    --m_parsers;
    addRoutes(routes);

    if (!m_parsers)
        setFinished(true);
}

void QGeoRouteReplyNokia::parserError(const QString &errorString)
{
    Q_UNUSED(errorString);
    emit aborted();
    setError(QGeoRouteReply::ParseError,
             QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, RESPONSE_NOT_RECOGNIZABLE));
}

QT_END_NAMESPACE
