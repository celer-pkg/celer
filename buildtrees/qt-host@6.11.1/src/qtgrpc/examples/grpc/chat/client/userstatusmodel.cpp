// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "userstatusmodel.h"
#include "chatmessages.qpb.h"

UserStatusModel::UserStatusModel(QObject *parent) : QAbstractListModel(parent)
{
}

UserStatusModel::~UserStatusModel() = default;

int UserStatusModel::rowCount(const QModelIndex & /*parent*/) const
{
    return int(mActiveUsers.count());
}

QVariant UserStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() > mActiveUsers.size())
        return {};

    const auto &i = mActiveUsers[index.row()];
    switch (static_cast<UserRole>(role)) {
    case Username:
        return i.first;
    case Status:
        return QVariant::fromValue(i.second);
    }

    return {};
}

QHash<int, QByteArray> UserStatusModel::roleNames() const
{
    return {
        { qToUnderlying(UserRole::Username), "username" },
        { qToUnderlying(UserRole::Status),   "status"   },
    };
}

bool UserStatusModel::updateUserStatus(const chat::ChatMessage &message)
{
    if (!message.hasUserStatus())
        return false;

    const auto &st = message.userStatus();
    for (int i = 0; i < mActiveUsers.length(); ++i) {
        if (mActiveUsers.at(i).first == message.username()) {
            if (st.type() == chat::UserStatus::Type::LOGOUT) {
                beginRemoveRows({}, i, i);
                mActiveUsers.remove(i);
                endRemoveRows();
                emit rowCountChanged();
                return false;
            }
            if (mActiveUsers.at(i).second == st.type())
                return false;
            mActiveUsers[i].second = st.type();
            emit dataChanged(index(i), index(i));
            return false;
        }
    }

    if (st.type() == chat::UserStatus::Type::NONE || st.type() == chat::UserStatus::Type::LOGOUT)
        return false;

    beginInsertRows({}, int(mActiveUsers.size()), int(mActiveUsers.size()));
    mActiveUsers.push_back({ message.username(), st.type() });
    endInsertRows();
    emit rowCountChanged();
    return true;
}
