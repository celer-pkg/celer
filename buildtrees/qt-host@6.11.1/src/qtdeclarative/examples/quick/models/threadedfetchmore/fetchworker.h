// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FETCHWORKER_H
#define FETCHWORKER_H

#include <QObject>

class FetchWorker : public QObject
{
    Q_OBJECT
public:
    struct DataBlock
    {
        QString title;
        QString subtitle;
        int number;
    };

    explicit FetchWorker(QObject *parent = nullptr);

signals:
    void dataFetched(const QList<FetchWorker::DataBlock> &items);
    void noMoreToFetch();

public slots:
    void fetchDataBlock();

private:
    static QList<FetchWorker::DataBlock> slowDataConstruction(int fromIndex);
    int m_existingItemsCount{0};
};

#endif // FETCHWORKER_H
