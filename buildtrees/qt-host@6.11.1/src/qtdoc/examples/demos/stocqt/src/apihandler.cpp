// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "apihandler.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

ApiHandler::ApiHandler() {}


void ApiHandler::stockQuote(const QString &symbols, std::function<void(QList<QuoteData>)> onComplete)
{
    const QStringList &symbolList = symbols.split(u',');

        QString filePath = QStringLiteral(":/data/quotes.json");
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "failed to read";
        }
        QByteArray jsonData = file.readAll();
        file.close();
        QList<QuoteData> dataList = parseQuote(symbolList, &jsonData);
        onComplete(dataList);

}

void ApiHandler::stockHistory(const QString &symbol,
                              std::function<void(QList<HistoryData>)> onComplete)
{

        QString filePath = QString(":/data/" + symbol + ".json");
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "failed to read";
        }
        QByteArray jsonData = file.readAll();
        file.close();
        QList<HistoryData> dataList = parseHistory(&jsonData);
        onComplete(dataList);

}

QList<QuoteData> ApiHandler::parseQuote(const QStringList &symbolList, QByteArray *data)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(*data, &error);
    QList<QuoteData> dataList;

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Error when parsing json: " << error.errorString();
        return dataList;
    }
    QJsonArray array = doc.array();
    for (int i = 0; i < symbolList.size(); ++i) {
        for (int j = 0; j < array.size(); ++j) {
            QJsonValue obj = array.at(j).toObject();
            if (symbolList.at(i) == obj["symbol"].toString()) {
                QuoteData data;
                data.price = obj["price"].toDouble();
                data.change = obj["change"].toDouble();
                data.changePercentage = obj["changesPercentage"].toDouble();
                data.time = QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInt());
                dataList.append(data);
            }
        }
    }
    return dataList;
}
QList<HistoryData> ApiHandler::parseHistory(QByteArray *data)
{
    QList<HistoryData> dataList;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(*data, &error);

    QJsonArray array = doc["historical"].toArray();

    for (int i = 0; i < array.size(); ++i) {
        QJsonValue obj = array.at(i);
        HistoryData data;
        data.high = obj["high"].toDouble();
        data.low = obj["low"].toDouble();
        data.open = obj["open"].toDouble();
        data.close = obj["close"].toDouble();
        data.volume = obj["volume"].toInt();
        data.time = QDateTime::fromString(obj["date"].toString(), m_dateFormat);
        dataList.append(data);
    }
    return dataList;
}

bool ApiHandler::useLiveData() const
{
    return m_useLiveData;
}

void ApiHandler::setApiKey(QString apiKey)
{
    m_apiKey = apiKey;
}
