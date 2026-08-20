// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include <QColor>
#include <QObject>
#include "stockmodel.h"

class FavoritesModel : public QObject
{
    Q_OBJECT
public:
    FavoritesModel();

    void addFavorite(StockModel *stock);
    void removeFavorite(const QString &stockId);
    QList<StockModel *> favorites() const;

    Q_INVOKABLE StockModel *atIndex(int index) const;
    Q_INVOKABLE int count() const;
    Q_INVOKABLE QColor color(int index) const;

signals:
    void onFavoritesChanged(bool full);

private:
    QList<StockModel *> m_favorites;
    const QVector<QColor> m_colors = {
        QColor(255, 0, 0),
        QColor(0, 255, 0),
        QColor(0, 0, 255),
        QColor(255, 255, 0),
        QColor(255, 0, 255),
    };
    QVector<QString> m_ids;
};

#endif // FAVORITESMODEL_H
