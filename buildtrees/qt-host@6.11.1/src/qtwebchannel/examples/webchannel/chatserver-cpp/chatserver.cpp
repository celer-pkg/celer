// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatserver.h"

#include <QDebug>
#include <QTime>
#include <QTimer>

ChatServer::ChatServer(QObject *parent)
    : QObject(parent)
{
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &ChatServer::sendKeepAlive);
    t->start(10000);

    m_keepAliveCheckTimer = new QTimer(this);
    m_keepAliveCheckTimer->setSingleShot(true);
    m_keepAliveCheckTimer->setInterval(2000);
    connect(m_keepAliveCheckTimer, &QTimer::timeout, this, &ChatServer::checkKeepAliveResponses);
}

ChatServer::~ChatServer()
{}


bool ChatServer::login(const QString &userName)
{
    //stop keepAliveCheck, when a new user logged in
    if (m_keepAliveCheckTimer->isActive()) {
        m_keepAliveCheckTimer->stop();
        m_stillAliveUsers.clear();
    }

    if (m_userList.contains(userName)) {
        return false;
    }

    qDebug() << "User logged in:" << userName;
    m_userList.append(userName);
    m_userList.sort();
    emit userListChanged();
    emit userCountChanged();
    return true;
}

bool ChatServer::logout(const QString &userName)
{
    if (!m_userList.contains(userName)) {
        return false;
    } else {
        m_userList.removeAt(m_userList.indexOf(userName));
        emit userListChanged();
        emit userCountChanged();
        return true;
    }
}

bool ChatServer::sendMessage(const QString &user, const QString &msg)
{
    if (m_userList.contains(user)) {
        emit newMessage(QTime::currentTime().toString("HH:mm:ss"), user, msg);
        return true;
    } else {
        return false;
    }
}

void ChatServer::sendKeepAlive()
{
    emit keepAlive();
    m_keepAliveCheckTimer->start();
}

void ChatServer::checkKeepAliveResponses()
{
    qDebug() << "Keep Alive Check" << m_stillAliveUsers;
    m_userList = m_stillAliveUsers;
    m_stillAliveUsers.clear();
    m_userList.sort();
    emit userListChanged();
}

void ChatServer::keepAliveResponse(const QString &user)
{
    m_stillAliveUsers.append(user);
}


QStringList ChatServer::userList() const
{
    return m_userList;
}
