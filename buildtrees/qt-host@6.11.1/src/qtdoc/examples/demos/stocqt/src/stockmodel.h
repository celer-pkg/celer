// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STOCKMODEL_H
#define STOCKMODEL_H

#include <QDateTime>
#include <QObject>

class QuoteData
{
public:
    float price = 0;
    float change = 0;
    float changePercentage = 0;
    QDateTime time;
};

class HistoryData
{
public:
    float high = 0;
    float low = 0;
    float open = 0;
    float close = 0;
    int volume = 0;
    QDateTime time;
};

class StockModel : public QObject
{
    Q_OBJECT
public:
    StockModel(const QString &stockId, const QString &name, QObject *parent = nullptr);

    void addData(HistoryData data);
    void updateHistory(QList<HistoryData> const &data);
    void appendQuote(QuoteData data);
    void resetQuote();
    void setDataIsLive(bool live);

    Q_INVOKABLE int historyCount() const;
    Q_INVOKABLE int quoteCount() const;
    Q_INVOKABLE QString stockId() const;
    Q_INVOKABLE QString name() const;
    Q_INVOKABLE float price(int index) const;
    Q_INVOKABLE float change(int index) const;
    Q_INVOKABLE float changePercentage(int index) const;
    Q_INVOKABLE int highPrice(int index) const;
    Q_INVOKABLE int lowPrice(int index) const;
    Q_INVOKABLE int openPrice(int index) const;
    Q_INVOKABLE int closePrice(int index) const;
    Q_INVOKABLE int volume(int index) const;
    Q_INVOKABLE int avgVolume() const;
    Q_INVOKABLE int daysFromNow(int index) const;
    Q_INVOKABLE int indexOf(QDateTime date) const;
    Q_INVOKABLE bool dataIsLive() const;
    Q_INVOKABLE long long historyDate(int index, bool milliseconds = true) const;
    Q_INVOKABLE long long quoteTime(int index, bool milliseconds = true) const;

signals:
    void historyDataReady();
    void quoteDataReady();

private:
    QList<HistoryData> m_historyDataList;
    QList<QuoteData> m_quoteDataList;
    QString m_stockId;
    QString m_name;
    bool m_dataIsLive = false;
};

#endif // STOCKMODEL_H
