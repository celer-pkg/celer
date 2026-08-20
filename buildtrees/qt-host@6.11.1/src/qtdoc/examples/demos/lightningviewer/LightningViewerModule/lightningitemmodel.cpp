// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "lightningitemmodel.h"
#include "laststrikeinfo.h"

#include <QGeoPositionInfo>

namespace {
constexpr int HISTORY_SIZE = 1000;
enum class Role : int {
    Begin = 0,
    Latitude = Begin,
    Longitude,
    Timestamp,
    End
};
constexpr auto getRoleName(Role role) {
    switch (role) {
    case Role::Latitude:
        return "latitude";
    case Role::Longitude:
        return "longitude";
    case Role::Timestamp:
        return "timestamp";
    default:
        break;
    }
    return "";
}
}

LightningItemModel::LightningItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void LightningItemModel::insertData(const LightningItemData &data)
{
    if (m_data.size() == HISTORY_SIZE) {
        beginRemoveRows(QModelIndex{}, 0, 0);
        m_data.pop_front();
        endRemoveRows();
    }

    const int insertRow = m_data.size();
    beginInsertRows(QModelIndex{}, insertRow, insertRow);
    m_data.push_back(data);
    endInsertRows();
}

void LightningItemModel::getNearestStrikeInfo(const QGeoCoordinate &coordinate,
                                              LastStrikeInfo *nearestStrikeInfo) const
{
    if (nearestStrikeInfo == nullptr)
        return;

    LastStrikeInfo strikeInfo;

    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it)
    {
        const LightningItemData &lightningData = *it;

        strikeInfo.distance = lightningData.getDistanceTo(coordinate);
        strikeInfo.timestamp = lightningData.timestamp;

        if (strikeInfo < *nearestStrikeInfo)
            *nearestStrikeInfo = strikeInfo;
    }
}

void LightningItemModel::getLatestStrikeInfo(const QGeoCoordinate &searchCenter,
                                             double searchRadius,
                                             LastStrikeInfo *latestStrikeInfo) const
{
    if (latestStrikeInfo == nullptr)
        return;

    for (int index = m_data.size() - 1; index >= 0; --index)
    {
        const LightningItemData &lightningData = m_data[index];

        const double distance = lightningData.getDistanceTo(searchCenter);
        if (distance > searchRadius)
            continue;

        latestStrikeInfo->distance = distance;
        latestStrikeInfo->timestamp = lightningData.timestamp;
        latestStrikeInfo->direction = lightningData.getDirectionFrom(searchCenter);
        break;
    }
}

int LightningItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

QVariant LightningItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant{};

    // child not supported
    if (index.parent().isValid())
        return QVariant{};

    const int row = index.row();

    // out of range row
    if ((row < 0) or (m_data.size() <= row))
        return QVariant{};

    const LightningItemData &data = m_data[row];

    switch (static_cast<Role>(role)) {
    case Role::Latitude:
        return data.latitude;
    case Role::Longitude:
        return data.longitude;
    case Role::Timestamp:
        return data.timestamp.count();
    default:
        break;
    }

    // invalid role
    return QVariant{};
}

QHash<int, QByteArray> LightningItemModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    for (int role = static_cast<int>(Role::Begin); role < static_cast<int>(Role::End); ++role)
        roleNames.emplace(role, getRoleName(static_cast<Role>(role)));
    return roleNames;
}
