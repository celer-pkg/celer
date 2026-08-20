// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TWITTERTIMELINE_TWITTER_H
#define TWITTERTIMELINE_TWITTER_H

#include <QtCore>
#include <QtNetwork>
#include <QtNetworkAuth>

class Twitter : public QOAuth1
{
    Q_OBJECT

public:
    Twitter(QObject *parent = nullptr);
    Twitter(const std::pair<QString, QString> &clientCredentials, QObject *parent = nullptr);
    Twitter(const QString &screenName,
            const std::pair<QString, QString> &clientCredentials,
            QObject *parent = nullptr);

signals:
    void authenticated();

private:
    Q_DISABLE_COPY(Twitter)

    QOAuthHttpServerReplyHandler *replyHandler = nullptr;
};

#endif // TWITTERTIMELINE_TWITTER_H
