// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "twittertimelinemodel.h"

#include <QtGui>
#include <QtCore>
#include <QtWidgets>

TwitterTimelineModel::TwitterTimelineModel(QObject *parent) : QAbstractTableModel(parent)
{
    connect(&twitter, &Twitter::authenticated, this, &TwitterTimelineModel::authenticated);
    connect(&twitter, &Twitter::authenticated, this, &TwitterTimelineModel::updateTimeline);
}

int TwitterTimelineModel::rowCount(const QModelIndex &parent) const
{
#if defined(QT_DEBUG)
    Q_ASSERT(!parent.isValid());
#else
    Q_UNUSED(parent);
#endif
    return tweets.size();
}

QVariant TwitterTimelineModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    auto it = tweets.begin();
    std::advance(it, index.row());
    switch (index.column())
    {
    case 0:
        return QString::number(it->id);
    case 1:
        return it->createdAt.toString(Qt::ISODateWithMs);
    case 2:
        return it->user;
    case 3:
        return it->text;
    }
    return QVariant();
}

int TwitterTimelineModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant TwitterTimelineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QStringLiteral("Id");
        case 1:
            return QStringLiteral("Created at");
        case 2:
            return QStringLiteral("User");
        case 3:
            return QStringLiteral("Text");
        }
    }
    return section;
}

void TwitterTimelineModel::authenticate(const std::pair<QString, QString> &clientCredentials)
{
    twitter.setClientCredentials(clientCredentials);
    twitter.grant();
}

QAbstractOAuth::Status TwitterTimelineModel::status() const
{
    return twitter.status();
}

void TwitterTimelineModel::updateTimeline()
{
    if (twitter.status() != Twitter::Status::Granted)
        QMessageBox::warning(nullptr, qApp->applicationName(), "Not authenticated");

    QUrl url("https://api.twitter.com/1.1/statuses/home_timeline.json");
    QVariantMap parameters;
    if (tweets.size()) {
        // Tweets are time-ordered, newest first.  Pass the most recent
        // ID we have to request everything more recent than it:
        parameters.insert("since_id", QString::number(tweets.first().id));
        // From https://dev.twitter.com/rest/reference/get/search/tweets:
        // Returns results with an ID greater than (that is, more recent than)
        // the specified ID. There are limits to the number of Tweets which can
        // be accessed through the API. If the limit of Tweets has occurred
        // since the since_id, the since_id will be forced to the oldest ID
        // available.
    }
    QNetworkReply *reply = twitter.get(url, parameters);
    connect(reply, &QNetworkReply::finished, this, &TwitterTimelineModel::parseJson);
}

void TwitterTimelineModel::parseJson()
{
    QJsonParseError parseError;
    auto reply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(reply);
    const auto data = reply->readAll();
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error) {
        qCritical() << "TwitterTimelineModel::parseJson. Error at:" << parseError.offset
                    << parseError.errorString();
        return;
    } else if (document.isObject()) {
        // Error received :(
        const auto object = document.object();
        const auto errorArray = object.value("errors").toArray();
        Q_ASSERT_X(errorArray.size(), "parse", data);
        QStringList errors;
        for (const auto error : errorArray) {
            Q_ASSERT(error.isObject());
            Q_ASSERT(error.toObject().contains("message"));
            errors.append(error.toObject().value("message").toString());
        }
        QMessageBox::warning(nullptr, qApp->applicationName(), errors.join("<br />"));
        return;
    }

    Q_ASSERT_X(document.isArray(), "parse", data);
    const auto array = document.array();
    if (array.size()) {
        beginInsertRows(QModelIndex(), 0, array.size() - 1);
        auto before = tweets.begin();
        for (const auto &value : array) {
            Q_ASSERT(value.isObject());
            const auto object = value.toObject();
            const auto createdAt = QDateTime::fromString(object.value("created_at").toString(),
                                                         "ddd MMM dd HH:mm:ss +0000 yyyy");
            before = tweets.insert(before, Tweet{
                                       object.value("id").toVariant().toULongLong(),
                                       createdAt,
                                       object.value("user").toObject().value("name").toString(),
                                       object.value("text").toString()
                                   });
            std::advance(before, 1);
        }
        endInsertRows();
    }
}
