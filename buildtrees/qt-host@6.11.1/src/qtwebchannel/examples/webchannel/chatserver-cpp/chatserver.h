// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class ChatServer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList userList READ userList NOTIFY userListChanged)

public:
    explicit ChatServer(QObject *parent = nullptr);
    virtual ~ChatServer();

public:
    //a user logs in with the given username
    Q_INVOKABLE bool login(const QString &userName);

    //the user logs out, will be removed from userlist immediately
    Q_INVOKABLE bool logout(const QString &userName);

    //a user sends a message to all other users
    Q_INVOKABLE bool sendMessage(const QString &user, const QString &msg);

    //response of the keep alive signal from a client.
    // This is used to detect disconnects.
    Q_INVOKABLE void keepAliveResponse(const QString &user);

    QStringList userList() const;

protected slots:
    void sendKeepAlive();
    void checkKeepAliveResponses();

signals:
    void newMessage(QString time, QString user, QString msg);
    void keepAlive();
    void userListChanged();
    void userCountChanged();

private:
    QStringList m_userList;
    QStringList m_stillAliveUsers;
    QTimer *m_keepAliveCheckTimer;
};

#endif // CHATSERVER_H
