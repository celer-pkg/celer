// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:authorization-protocol

#include "qoauthoobreplyhandler.h"
#include "qoauthoobreplyhandler_p.h"

#include <QtCore/qurlquery.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qloggingcategory.h>

#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qrestreply.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QOAuthOobReplyHandler::QOAuthOobReplyHandler(QObject *parent)
    : QAbstractOAuthReplyHandler(parent)
{}

/*! \internal */
QOAuthOobReplyHandler::QOAuthOobReplyHandler(QOAuthOobReplyHandlerPrivate &d, QObject *parent)
    : QAbstractOAuthReplyHandler(d, parent)
{}

QOAuthOobReplyHandler::~QOAuthOobReplyHandler()
    = default; // must be empty until Qt 7 (was inline until Qt 6.8)

QString QOAuthOobReplyHandler::callback() const
{
    return QStringLiteral("oob");
}

void QOAuthOobReplyHandler::networkReplyFinished(QNetworkReply *reply)
{
    QRestReply restReply(reply);

    if (restReply.hasError()) {
        emit tokenRequestErrorOccurred(QAbstractOAuth::Error::NetworkError, reply->errorString());
        return;
    }
    if (!restReply.isHttpStatusSuccess()) {
        emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError, reply->errorString());
        return;
    }
    if (reply->header(QNetworkRequest::ContentTypeHeader).isNull()) {
        emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError,
                                       u"Empty Content-type header"_s);
        return;
    }
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).isNull() ?
                QStringLiteral("text/html") :
                reply->header(QNetworkRequest::ContentTypeHeader).toString();
    const QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError, u"No data received"_s);
        return;
    }

    Q_EMIT replyDataReceived(data);

    QVariantMap ret;

    if (contentType.startsWith(QStringLiteral("text/html")) ||
            contentType.startsWith(QStringLiteral("application/x-www-form-urlencoded"))) {
        ret = parseResponse(data);
    } else if (contentType.startsWith(QStringLiteral("application/json"))
               || contentType.startsWith(QStringLiteral("text/javascript"))) {
        const QJsonDocument document = QJsonDocument::fromJson(data);
        if (!document.isObject()) {
            emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError,
                          u"Received data is not a JSON object: %1"_s.arg(QString::fromUtf8(data)));
            return;
        }
        const QJsonObject object = document.object();
        if (object.isEmpty()) {
            emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError,
                                           u"Received an empty JSON object"_s);
            return;
        }
        ret = object.toVariantMap();
    } else {
        emit tokenRequestErrorOccurred(QAbstractOAuth::Error::ServerError,
                               u"Unknown Content-type %1"_s.arg(contentType));
        return;
    }

    Q_EMIT tokensReceived(ret);
}

QVariantMap QOAuthOobReplyHandler::parseResponse(const QByteArray &response)
{
    QVariantMap ret;
    QUrlQuery query(QString::fromUtf8(response));
    const auto queryItems = query.queryItems(QUrl::FullyDecoded);
    for (auto it = queryItems.begin(), end = queryItems.end(); it != end; ++it)
        ret.insert(it->first, it->second);
    return ret;
}

QT_END_NAMESPACE

#include "moc_qoauthoobreplyhandler.cpp"
