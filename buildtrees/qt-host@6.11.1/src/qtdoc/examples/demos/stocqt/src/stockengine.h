// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STOCKENGINE_H
#define STOCKENGINE_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QVariant>
#include "apihandler.h"
#include "favoritesmodel.h"
#include "stocklistmodel.h"
#include "stockmodel.h"
#include <qqmlregistration.h>

class StockEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    StockEngine();

    Q_PROPERTY(StockListModel *stockListModel READ stockListModel NOTIFY onStockListModelChanged)
    Q_PROPERTY(FavoritesModel *favoritesModel READ favoritesModel NOTIFY onFavoritesModelChanged)
    Q_PROPERTY(StockModel *stockModel READ stockModel NOTIFY onStockModelChanged)
    Q_PROPERTY(QSortFilterProxyModel *filterModel READ filterModel NOTIFY filterChanged FINAL)

    StockListModel *stockListModel();
    FavoritesModel *favoritesModel();
    StockModel *stockModel();
    QString timeFrame();
    QSortFilterProxyModel *filterModel();

    Q_INVOKABLE QString currentStockId() const;
    Q_INVOKABLE QString currentName() const;
    Q_INVOKABLE bool isFavorite(const QString &stockId) const;
    Q_INVOKABLE bool useLiveData() const;

public slots:
    void testApiKey();
    void updateStockListModel();
    void updateStockView(const QString &stockId);
    void addFavorite(const QString &stockId);
    void removeFavorite(const QString &stockId);
    void updateFavorites();
    void updateStockModelHistory(const QString &stockId);
    void updateStockModelQuote();

signals:
    void onStockListModelChanged();
    void onStockModelChanged();
    void stockDataReady();
    void onFavoritesModelChanged();
    void onFavoritesChanged(bool full);
    void onDataTypeChanged(bool live);
    void onApiKeyTested(bool keyValid);
    void onTimeFrameChanged();
    void onLiveDataReady(float liveData);
    void filterChanged();

private:
    StockListModel m_stockListModel;
    FavoritesModel m_favoritesModel;
    StockModel *m_stockModel;
    ApiHandler m_apiHandler;
    QSortFilterProxyModel m_proxyModel;
    QTimer m_liveDataTimer;
};

#endif // STOCKENGINE_H
