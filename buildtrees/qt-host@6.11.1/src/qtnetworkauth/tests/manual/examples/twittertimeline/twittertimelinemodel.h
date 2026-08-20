// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TWITTERTIMELINEMODEL_H
#define TWITTERTIMELINEMODEL_H

#include "twitter.h"

#include <QtCore>
#include <QtNetwork>

class TwitterTimelineModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TwitterTimelineModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void authenticate(const std::pair<QString, QString> &clientCredentials);
    QAbstractOAuth::Status status() const;

public slots:
    void updateTimeline();

signals:
    void authenticated();

private:
    Q_DISABLE_COPY(TwitterTimelineModel)

    void parseJson();

    struct Tweet {
        quint64 id;
        QDateTime createdAt;
        QString user;
        QString text;
    };

    QList<Tweet> tweets;
    Twitter twitter;
};

#endif // TWITTERTIMELINEMODEL_H
