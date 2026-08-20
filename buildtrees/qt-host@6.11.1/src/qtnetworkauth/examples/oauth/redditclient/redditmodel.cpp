// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "redditmodel.h"

#include <QJsonArray>

#include <QDesktopServices>

#include <QNetworkAccessManager>
#include <QRestAccessManager>
#include <QRestReply>

#include <QOAuthHttpServerReplyHandler>

using namespace Qt::StringLiterals;

static constexpr auto hotUrl = "https://oauth.reddit.com/hot"_L1;
static constexpr auto authorizationUrl = "https://www.reddit.com/api/v1/authorize"_L1;
static constexpr auto accessTokenUrl = "https://www.reddit.com/api/v1/access_token"_L1;

RedditModel::RedditModel(QObject *parent) : QAbstractTableModel(parent) {}

RedditModel::RedditModel(const QString &clientId, QObject *parent) :
    QAbstractTableModel(parent)
{
    QNetworkAccessManager *qnam = new QNetworkAccessManager(this);
    network = new QRestAccessManager(qnam, qnam);

    redditApi.setBaseUrl(QUrl(hotUrl));

    auto replyHandler = new QOAuthHttpServerReplyHandler(QHostAddress::Any, 1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl(authorizationUrl));
    oauth2.setTokenUrl(QUrl(accessTokenUrl));
    oauth2.setRequestedScopeTokens({"identity", "read"});
    oauth2.setClientIdentifier(clientId);

    QObject::connect(&oauth2, &QAbstractOAuth::granted, this, [this] {
        redditApi.setBearerToken(oauth2.token().toLatin1());
        updateHotThreads();
    });
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
            &QDesktopServices::openUrl);
    oauth2.grant();
}

int RedditModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return threads.size();
}

int RedditModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return threads.size() ? 1 : 0;
}

QVariant RedditModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto childrenObject = threads.at(index.row());
        Q_ASSERT(childrenObject.value("data"_L1).isObject());
        const auto dataObject = childrenObject.value("data"_L1).toObject();
        return dataObject.value("title"_L1).toString();
    }
    return QVariant();
}

void RedditModel::updateHotThreads()
{
    network->get(redditApi.createRequest(), this, [this](QRestReply &reply) {
        if (!reply.isSuccess()) {
            emit error(reply.errorString());
            return;
        }
        const auto document = reply.readJson();
        Q_ASSERT(document && document->isObject());
        const auto rootObject = document->object();
        Q_ASSERT(rootObject.value("kind"_L1).toString() == "Listing"_L1);
        const auto dataValue = rootObject.value("data"_L1);
        Q_ASSERT(dataValue.isObject());
        const auto dataObject = dataValue.toObject();
        const auto childrenValue = dataObject.value("children"_L1);
        Q_ASSERT(childrenValue.isArray());
        const auto childrenArray = childrenValue.toArray();

        if (childrenArray.isEmpty())
            return;

        beginInsertRows(QModelIndex(), threads.size(), childrenArray.size() + threads.size() - 1);
        for (const auto childValue : std::as_const(childrenArray)) {
            Q_ASSERT(childValue.isObject());
            threads.append(childValue.toObject());
        }
        endInsertRows();
    });
}
