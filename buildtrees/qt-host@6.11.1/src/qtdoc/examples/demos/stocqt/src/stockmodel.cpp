// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "stockmodel.h"
#include <QDateTime>

StockModel::StockModel(const QString &stockId, const QString &name, QObject *parent)
    : QObject(parent)
    , m_stockId(stockId)
    , m_name(name){};

void StockModel::addData(HistoryData data)
{
    m_historyDataList.append(data);
    emit historyDataReady();
}

void StockModel::updateHistory(const QList<HistoryData> &data)
{
    m_historyDataList = data;
    emit historyDataReady();
}

void StockModel::appendQuote(QuoteData data)
{
    // check if already exists
    if (!m_quoteDataList.isEmpty()) {
        const QuoteData latest = m_quoteDataList.at(0);
        if (latest.time == data.time) {
            return;
        }
    }
    m_quoteDataList.insert(0, data);
    emit quoteDataReady();
}

void StockModel::resetQuote()
{
    m_quoteDataList.clear();
    emit quoteDataReady();
}

void StockModel::setDataIsLive(bool live)
{
    m_dataIsLive = live;
}

int StockModel::historyCount() const
{
    return m_historyDataList.count();
}

int StockModel::quoteCount() const
{
    return m_quoteDataList.count();
}

QString StockModel::stockId() const
{
    return m_stockId;
}
QString StockModel::name() const
{
    return m_name;
}

float StockModel::price(int index) const
{
    if (m_quoteDataList.isEmpty())
        return 0;
    return m_quoteDataList.at(index).price;
}

float StockModel::change(int index) const
{
    if (m_quoteDataList.isEmpty())
        return 0;
    return m_quoteDataList.at(index).change;
}
float StockModel::changePercentage(int index) const
{
    if (m_quoteDataList.isEmpty())
        return 0;
    return m_quoteDataList.at(index).changePercentage;
}

int StockModel::openPrice(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).open;
}
int StockModel::closePrice(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).close;
}
int StockModel::highPrice(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).high;
}
int StockModel::lowPrice(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).low;
}
int StockModel::volume(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).volume;
}

int StockModel::avgVolume() const
{
    long long totalVolume = 0;
    for (int i = 0; i < historyCount(); ++i) {
        totalVolume += volume(i);
    }
    const int count = historyCount();
    return (count > 0) ? totalVolume / count : 0;
}

int StockModel::daysFromNow(int index) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    return m_historyDataList.at(index).time.daysTo(QDateTime::currentDateTime());
}

long long StockModel::historyDate(int index, bool milliseconds) const
{
    if (m_historyDataList.isEmpty())
        return 0;
    if (milliseconds)
        return m_historyDataList.at(index).time.toMSecsSinceEpoch();
    else
        return m_historyDataList.at(index).time.toSecsSinceEpoch();
}

long long StockModel::quoteTime(int index, bool milliseconds) const
{
    if (m_quoteDataList.isEmpty() || index >= m_quoteDataList.count())
        return 0;
    if (milliseconds)
        return m_quoteDataList.at(index).time.toMSecsSinceEpoch();
    else
        return m_quoteDataList.at(index).time.toSecsSinceEpoch();
}
int StockModel::indexOf(QDateTime date) const
{
    if (m_historyDataList.empty())
        return -1;
    QDateTime today = date.addMSecs(-QTime::currentTime().msecsSinceStartOfDay());
    bool afterDate = true;
    int index = 0;
    while (afterDate) {
        if (index == m_historyDataList.size() - 1)
            afterDate = false;
        else if (today.secsTo(m_historyDataList.at(index).time) < 0) {
            afterDate = false;
        } else
            index++;
    }
    return index;
}

bool StockModel::dataIsLive() const
{
    return m_dataIsLive;
}
