// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "twitter.h"

#include <QtGui>
#include <QtCore>
#include <QtNetwork>

Twitter::Twitter(QObject *parent) :
    Twitter(QString(), std::make_pair(QString(), QString()), parent)
{}

Twitter::Twitter(const std::pair<QString, QString> &clientCredentials, QObject *parent) :
    Twitter(QString(), clientCredentials, parent)
{}

Twitter::Twitter(const QString &screenName,
                 const std::pair<QString, QString> &clientCredentials,
                 QObject *parent) :
    QOAuth1(clientCredentials.first, clientCredentials.second, nullptr, parent)
{
    replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    replyHandler->setCallbackPath("callback");
    setReplyHandler(replyHandler);
    setTemporaryCredentialsUrl(QUrl("https://api.twitter.com/oauth/request_token"));
    setAuthorizationUrl(QUrl("https://api.twitter.com/oauth/authenticate"));
    setTokenCredentialsUrl(QUrl("https://api.twitter.com/oauth/access_token"));

    connect(this, &QAbstractOAuth::authorizeWithBrowser, [=](QUrl url) {
        QUrlQuery query(url);

        // Forces the user to enter their credentials to authorize the correct
        // user account
        query.addQueryItem("force_login", "true");

        if (!screenName.isEmpty())
            query.addQueryItem("screen_name", screenName);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    });

    connect(this, &QOAuth1::granted, this, &Twitter::authenticated);

    if (!clientCredentials.first.isEmpty() && !clientCredentials.second.isEmpty())
        grant();
}
