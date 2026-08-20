// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STOCKLISTMODEL_H
#define STOCKLISTMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include "stockmodel.h"

class StockListData
{
public:
    QString stockId = "id";
    QString name = "name";
    bool favorite = false;
    StockModel *model;

    StockListData(const QString &stockId, const QString &name, QObject *parent = nullptr)
        : stockId(stockId)
        , name(name)
    {
        model = new StockModel(stockId, name, parent);
    };
};

class StockListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        idRole = 0,
        nameRole,
        favoriteRole,
        dateRole,
        priceRole,
        changeRole,
        changePercentageRole,
        filterRole
    };
    explicit StockListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateDetails(const QList<QuoteData> &data);
    void addFavorite(const QString &stockId);
    void removeFavorite(const QString &stockId);
    StockModel *stockModel(int index) const;
    void resetQuotes();

private:
    QList<StockListData> m_data;
    QList<QString> m_favorites;
};

#endif // STOCKLISTMODEL_H
