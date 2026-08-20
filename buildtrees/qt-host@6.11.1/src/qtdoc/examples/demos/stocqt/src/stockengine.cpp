// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "stockengine.h"

StockEngine::StockEngine()
{
    m_proxyModel.setParent(this);
    m_proxyModel.setSourceModel(&m_stockListModel);
    m_proxyModel.setFilterRole(StockListModel::filterRole);
    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_stockModel = m_stockListModel.stockModel(0);
    m_liveDataTimer.setInterval(1000 * 60); // update every minute
    connect(&m_liveDataTimer, &QTimer::timeout, this, &StockEngine::updateStockListModel);
}

StockListModel *StockEngine::stockListModel()
{
    return &m_stockListModel;
}
FavoritesModel *StockEngine::favoritesModel()
{
    return &m_favoritesModel;
}

StockModel *StockEngine::stockModel()
{
    return m_stockModel;
}

QSortFilterProxyModel *StockEngine::filterModel()
{
    return &m_proxyModel;
}

void StockEngine::testApiKey()
{
    emit onApiKeyTested(false);
}

void StockEngine::updateStockListModel()
{
    QString symbols;
    for (int i = 0; i < m_stockListModel.rowCount(); ++i) {
        auto qIndex = m_stockListModel.index(i);
        symbols.append(m_stockListModel.data(qIndex, StockListModel::idRole).value<QString>() + ",");
    }
    symbols.removeLast(); // remove last comma
    m_apiHandler.stockQuote(symbols, [this](QList<QuoteData> data) {
        m_stockListModel.updateDetails(data);
    });
}

void StockEngine::updateStockView(const QString &stockId)
{
    disconnect(m_stockModel);
    int index = 0;
    for (int i = 0; i < m_stockListModel.rowCount(); i++) {
        auto qIndex = m_stockListModel.index(i);
        if (m_stockListModel.data(qIndex, StockListModel::idRole).value<QString>() == stockId)
            index = i;
    }
    StockModel *stock;
    stock = m_stockListModel.stockModel(index);
    m_stockModel = stock;
    connect(m_stockModel, &StockModel::historyDataReady, this, &StockEngine::onStockModelChanged);
    connect(m_stockModel, &StockModel::quoteDataReady, this, &StockEngine::onStockModelChanged);
    
    if (stock->historyCount() == 0 || stock->dataIsLive() != m_apiHandler.useLiveData())
        updateStockModelHistory(stock->stockId());
    else
        emit onStockModelChanged();
}

void StockEngine::addFavorite(const QString &stockId)
{
    for (int i = 0; i < m_stockListModel.rowCount(); i++) {
        auto index = m_stockListModel.index(i);
        if (m_stockListModel.data(index, StockListModel::idRole).value<QString>() == stockId) {
            StockModel *stock = m_stockListModel.stockModel(i);
            if (m_favoritesModel.count() < 5) {
                m_stockListModel.addFavorite(stockId);
                m_favoritesModel.addFavorite(stock);
                if (stock->historyCount() == 0) {
                    m_apiHandler.stockHistory(stockId, [stock](QList<HistoryData> dataList) {
                        stock->updateHistory(dataList);
                    });
                }
                emit onFavoritesChanged(m_favoritesModel.count() == 5);
            } else {
                qDebug() << "Favorites are full";
            }
        }
    }
}

void StockEngine::removeFavorite(const QString &stockId)
{
    m_stockListModel.removeFavorite(stockId);
    m_favoritesModel.removeFavorite(stockId);
    emit onFavoritesChanged(false);
}

void StockEngine::updateFavorites()
{
    auto favorites = m_favoritesModel.favorites();
    for (int i = 0; i < favorites.size(); ++i) {
        StockModel *stock = favorites.at(i);
        if (stock->historyCount() == 0) {
            m_apiHandler.stockHistory(stock->stockId(), [stock](const QList<HistoryData> &dataList) {
                stock->updateHistory(dataList);
            });
        }
    }
}

void StockEngine::updateStockModelHistory(const QString &stockId)
{
    m_apiHandler.stockHistory(stockId, [this](QList<HistoryData> dataList) {
        m_stockModel->updateHistory(dataList);
    });
}

void StockEngine::updateStockModelQuote()
{
    m_apiHandler.stockQuote(currentStockId(), [this](QList<QuoteData> data) {
        m_stockModel->appendQuote(data.at(0));
    });
}

QString StockEngine::currentStockId() const
{
    return m_stockModel->stockId();
}

QString StockEngine::currentName() const
{
    return m_stockModel->name();
}

bool StockEngine::isFavorite(const QString &stockId) const
{
    for (int i = 0; i < m_favoritesModel.count(); ++i) {
        if (m_favoritesModel.atIndex(i)->stockId() == stockId)
            return true;
    }
    return false;
}

bool StockEngine::useLiveData() const
{
    return m_apiHandler.useLiveData();
}
