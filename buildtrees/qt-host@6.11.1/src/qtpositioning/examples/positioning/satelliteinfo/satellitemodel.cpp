// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "roles.h"
#include "satellitemodel.h"

using namespace Qt::StringLiterals;

static QString systemString(QGeoSatelliteInfo::SatelliteSystem system)
{
    switch (system) {
    case QGeoSatelliteInfo::Undefined:
        return u"Undefined"_s;
    case QGeoSatelliteInfo::GPS:
        return u"GPS"_s;
    case QGeoSatelliteInfo::GLONASS:
        return u"GLONASS"_s;
    case QGeoSatelliteInfo::GALILEO:
        return u"GALILEO"_s;
    case QGeoSatelliteInfo::BEIDOU:
        return u"BEIDOU"_s;
    case QGeoSatelliteInfo::QZSS:
        return u"QZSS"_s;
    case QGeoSatelliteInfo::Multiple:
        return u"Multiple"_s;
    case QGeoSatelliteInfo::CustomType:
        return u"CustomType"_s;
    }
    return u"Undefined"_s;
}

SatelliteModel::SatelliteModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

int SatelliteModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_satellites.size());
}

QVariant SatelliteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    const QGeoSatelliteInfo &info = m_satellites.at(index.row());
    switch (role) {
    case Roles::IdRole:
        return info.satelliteIdentifier();
    case Roles::RssiRole:
        return info.signalStrength();
    case Roles::AzimuthRole:
        return info.attribute(QGeoSatelliteInfo::Azimuth);
    case Roles::ElevationRole:
        return info.attribute(QGeoSatelliteInfo::Elevation);
    case Roles::SystemRole:
        return systemString(info.satelliteSystem());
    case Roles::SystemIdRole:
        return info.satelliteSystem();
    case Roles::InUseRole:
        return m_inUseIds.contains(getUid(info));
    case Roles::VisibleNameRole:
        return u"%1-%2"_s.arg(systemString(info.satelliteSystem()),
                              QString::number(info.satelliteIdentifier()));
    }

    return QVariant();
}

//! [0]
QHash<int, QByteArray> SatelliteModel::roleNames() const
{
    return {
        {Roles::IdRole, "id"},
        {Roles::RssiRole, "rssi"},
        {Roles::AzimuthRole, "azimuth"},
        {Roles::ElevationRole, "elevation"},
        {Roles::SystemRole, "system"},
        {Roles::SystemIdRole, "systemId"},
        {Roles::InUseRole, "inUse"},
        {Roles::VisibleNameRole, "name"}
    };
}
//! [0]

void SatelliteModel::updateSatellitesInView(const QList<QGeoSatelliteInfo> &inView)
{
    const bool emitSizeChanged = inView.size() != m_satellites.size();

    QSet<UniqueId> idsInUpdate;
    for (const QGeoSatelliteInfo &info : inView)
        idsInUpdate.insert(getUid(info));

    // 1. Get the ids of all satellites to be removed.
    const auto toBeRemoved = m_allIds - idsInUpdate;
    // 2. Remove them and call {begin,end}RemoveRows() for each of them
    qsizetype idx = 0;
    while (idx < m_satellites.size()) {
        const auto &sat = m_satellites.at(idx);
        if (toBeRemoved.contains(getUid(sat))) {
            beginRemoveRows(QModelIndex(), idx, idx);
            m_satellites.removeAt(idx);
            endRemoveRows();
        } else {
            ++idx;
        }
    }
    // 3. Get ids of all new elements.
    const auto toBeAdded = idsInUpdate - m_allIds;
    // 4. Sort the input items, so that we always have the same order of
    //    elements during comparison
    auto inViewCopy = inView;
    std::sort(inViewCopy.begin(), inViewCopy.end(),
              [](const auto &lhs, const auto &rhs) {
                  if (lhs.satelliteIdentifier() == rhs.satelliteIdentifier())
                      return lhs.satelliteSystem() < rhs.satelliteSystem();
                  return lhs.satelliteIdentifier() < rhs.satelliteIdentifier();
              });
    // 5. Iterate through the list:
    //    * if the id of the satellite is new, use {begin,end}InsertRows()
    //    * otherwise use dataChanged()
    for (idx = 0; idx < inViewCopy.size(); ++idx) {
        const auto &sat = inViewCopy.at(idx);
        if (toBeAdded.contains(getUid(sat))) {
            beginInsertRows(QModelIndex(), idx, idx);
            m_satellites.insert(idx, inViewCopy.at(idx));
            endInsertRows();
        } else {
            m_satellites[idx] = inViewCopy.at(idx);
            emit dataChanged(index(idx), index(idx),
                             {Roles::RssiRole, Roles::AzimuthRole, Roles::ElevationRole});
        }
    }
    m_allIds = idsInUpdate;
    if (emitSizeChanged)
        emit sizeChanged();
}

void SatelliteModel::updateSatellitesInUse(const QList<QGeoSatelliteInfo> &inUse)
{
    m_inUseIds.clear();
    for (const QGeoSatelliteInfo &info : inUse)
        m_inUseIds.insert(getUid(info));
    emit dataChanged(index(0), index(m_satellites.size() - 1), {Roles::InUseRole});
}

SatelliteModel::UniqueId SatelliteModel::getUid(const QGeoSatelliteInfo &info)
{
    return std::make_pair(static_cast<int>(info.satelliteSystem()), info.satelliteIdentifier());
}
