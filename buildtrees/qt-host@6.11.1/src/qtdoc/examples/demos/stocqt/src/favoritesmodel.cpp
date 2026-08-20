// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "favoritesmodel.h"

FavoritesModel::FavoritesModel() {}

void FavoritesModel::addFavorite(StockModel *stock)
{
    m_favorites.append(stock);
    emit onFavoritesChanged(m_favorites.size() >= 5);
}

void FavoritesModel::removeFavorite(const QString &stockId)
{
    for (int i = 0; i < m_favorites.size(); ++i) {
        if (m_favorites.at(i)->stockId() == stockId) {
            m_favorites.removeAt(i);
            emit onFavoritesChanged(false);
        }
    }
}

QList<StockModel *> FavoritesModel::favorites() const
{
    return m_favorites;
}

int FavoritesModel::count() const
{
    return m_favorites.size();
}

QColor FavoritesModel::color(int index) const
{
    return m_colors.at(index);
}

StockModel *FavoritesModel::atIndex(int index) const
{
    if (index < 0 || index > m_favorites.count()) {
        qDebug() << "invalid index: " << index;
        return 0;
    } else if (m_favorites.empty()) {
        qDebug() << "favorites empty";
        return 0;
    }
    return m_favorites.at(index);
}
