// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "roles.h"
#include "sortfiltermodel.h"

using namespace Qt::StringLiterals;

SortFilterModel::SortFilterModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
}

void SortFilterModel::updateFilterString(const QString &str)
{
    beginFilterChange();
    m_filterString = str;
    endFilterChange();
}

void SortFilterModel::updateShowInView(bool show)
{
    beginFilterChange();
    m_showInView = show;
    endFilterChange();
}

void SortFilterModel::updateShowInUse(bool show)
{
    beginFilterChange();
    m_showInUse = show;
    endFilterChange();
}

void SortFilterModel::updateSelectedSystems(int id, bool show)
{
    beginFilterChange();
    if (show)
        m_selectedSystems.insert(id);
    else
        m_selectedSystems.remove(id);
    endFilterChange();
}

void SortFilterModel::updateSortRoles(int role, bool use)
{
    if (use)
        m_sortRoles.insert(role);
    else
        m_sortRoles.remove(role);
    invalidate();
}

bool SortFilterModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    const auto idx = sourceModel()->index(row, 0);
    bool result = true;
    if (!m_filterString.isEmpty()) {
        const QString name = idx.data(Roles::VisibleNameRole).toString();
        result = name.contains(m_filterString, Qt::CaseInsensitive);
    }
    if (result) {
        const int systemId = idx.data(Roles::SystemIdRole).toInt();
        result = m_selectedSystems.contains(systemId);
    }
    if (result) {
        const bool used = idx.data(Roles::InUseRole).toBool();
        if (used)
            result = m_showInUse;
        else
            result = m_showInView;
    }
    return result;
}

bool SortFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Sort 'By Identifier' can actually be called a sort by satellite system
    // name + sort by id.
    if (m_sortRoles.contains(Roles::InUseRole)) {
        const auto leftVal = left.data(Roles::InUseRole).toBool();
        const auto rightVal = right.data(Roles::InUseRole).toBool();
        if (leftVal != rightVal)
            return leftVal < rightVal;
    }
    if (m_sortRoles.contains(Roles::SystemRole)) {
        const auto leftVal = left.data(Roles::SystemRole).toString();
        const auto rightVal = right.data(Roles::SystemRole).toString();
        if (leftVal != rightVal)
            return leftVal < rightVal;
    }
    const auto leftId = left.data(Roles::IdRole).toInt();
    const auto rightId = right.data(Roles::IdRole).toInt();
    return leftId < rightId;
}
