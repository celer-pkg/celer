// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef USERSTATUSMODEL_H
#define USERSTATUSMODEL_H

#include "chatmessages.qpb.h"

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>

class UserStatusModel : public QAbstractListModel
{
    Q_OBJECT
    QML_INTERFACE

public:
    enum UserRole {
        Username = Qt::UserRole,
        Status,
    };

    explicit UserStatusModel(QObject *parent = nullptr);
    ~UserStatusModel() override;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::UserRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    bool updateUserStatus(const chat::ChatMessage &message);

signals:
    void rowCountChanged();

private:
    QList<std::pair<QString, chat::UserStatus::Type>> mActiveUsers;
    Q_DISABLE_COPY_MOVE(UserStatusModel)
};

#endif // USERSTATUSMODEL_H
