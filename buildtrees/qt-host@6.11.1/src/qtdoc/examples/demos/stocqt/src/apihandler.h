// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QNetworkAccessManager>
#include "stockmodel.h"

class ApiHandler : public QObject
{
    Q_OBJECT
public:
    ApiHandler();

    void setApiKey(QString apiKey);
    void stockQuote(const QString &symbols, std::function<void(QList<QuoteData>)> onComplete);
    void stockHistory(const QString &symbol, std::function<void(QList<HistoryData>)> onComplete);
    bool useLiveData() const;

private:
    QString m_apiKey;
    bool m_useLiveData = false;
    QNetworkAccessManager m_accessManager;
    QNetworkRequest m_getRequest;
    const QString m_dateFormat = QString("yyyy-MM-dd");

    QList<QuoteData> parseQuote(const QStringList &symbolList, QByteArray *data);
    QList<HistoryData> parseHistory(QByteArray *data);
};

#endif // APIHANDLER_H
