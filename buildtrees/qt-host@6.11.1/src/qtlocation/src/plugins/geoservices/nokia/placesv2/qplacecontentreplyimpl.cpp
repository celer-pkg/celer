// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "jsonparserhelpers.h"
#include "qplacecontentreplyimpl.h"
#include "qplacecontentrequest.h"
#include "../qplacemanagerengine_nokiav2.h"
#include "../qgeoerror_messages.h"


#include <QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

QT_BEGIN_NAMESPACE

QPlaceContentReplyImpl::QPlaceContentReplyImpl(const QPlaceContentRequest &request,
                                               QNetworkReply *reply,
                                               QPlaceManagerEngineNokiaV2 *engine)
    :   QPlaceContentReply(engine), m_engine(engine)
{
    Q_ASSERT(engine);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }
    setRequest(request);

    connect(reply, &QNetworkReply::finished,
            this, &QPlaceContentReplyImpl::replyFinished);
    connect(reply, &QNetworkReply::errorOccurred,
            this, &QPlaceContentReplyImpl::replyError);
    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QPlaceContentReplyImpl::~QPlaceContentReplyImpl()
{
}

void QPlaceContentReplyImpl::setError(QPlaceReply::Error error_, const QString &errorString)
{
    QPlaceContentReply::setError(error_, errorString);
    emit errorOccurred(error_, errorString);
    setFinished(true);
    emit finished();
}

void QPlaceContentReplyImpl::replyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, PARSE_ERROR));
        return;
    }

    QJsonObject object = document.object();

    QPlaceContent::Collection collection;
    int totalCount;
    QPlaceContentRequest previous;
    QPlaceContentRequest next;

    parseCollection(request().contentType(), object, &collection, &totalCount,
                    &previous, &next, m_engine);

    setTotalCount(totalCount);
    setContent(collection);
    setPreviousPageRequest(previous);
    setNextPageRequest(next);

    setFinished(true);
    emit finished();
}

void QPlaceContentReplyImpl::replyError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (error == QNetworkReply::OperationCanceledError)
        setError(QPlaceReply::CancelError, QStringLiteral("Request cancelled"));
    else
        setError(QPlaceReply::CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
