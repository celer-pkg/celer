// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LOGINSERVICE_H
#define LOGINSERVICE_H

#include "abstractresource.h"

#include <QtQml/qqml.h>

class BasicLogin: public AbstractResource
{
    Q_OBJECT
    Q_PROPERTY(QString user READ user NOTIFY userChanged)
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY userChanged)
    Q_PROPERTY(QString loginPath MEMBER m_loginPath REQUIRED)
    Q_PROPERTY(QString logoutPath MEMBER m_logoutPath REQUIRED)
    QML_ELEMENT

public:
    explicit BasicLogin(QObject *parent = nullptr);

    QString user() const;
    bool loggedIn() const;
    Q_INVOKABLE void login(const QVariantMap &data);
    Q_INVOKABLE void logout();

signals:
    void userChanged();

private:
    struct User {
        QString email;
        QByteArray token;
        int id;
    };
    QString m_loginPath;
    QString m_logoutPath;
    std::optional<User> m_user;
};

#endif // LOGINSERVICE_H
