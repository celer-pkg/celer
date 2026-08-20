// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "fetchworker.h"

#include <QThread>

static constexpr int s_fetchBlockSize{50};
static constexpr int s_maximumListSize{s_fetchBlockSize * 10};
static constexpr int s_sleepIterations{20};
static constexpr std::chrono::milliseconds s_sleepInterval{100};

FetchWorker::FetchWorker(QObject *parent)
    : QObject{parent}
{}

void FetchWorker::fetchDataBlock()
{
    if (m_existingItemsCount < s_maximumListSize) {
        QList<DataBlock> itemsToSend = slowDataConstruction(m_existingItemsCount);
        m_existingItemsCount += itemsToSend.size();
        emit dataFetched(itemsToSend);
    }
    if (m_existingItemsCount >= s_maximumListSize)
        emit noMoreToFetch();
}

QList<FetchWorker::DataBlock> FetchWorker::slowDataConstruction(int fromIndex)
{
    // Block for two seconds to mimic slow data source, while allowing interruption in case of application close
    for (int iterations = s_sleepIterations;
         iterations > 0 && !QThread::currentThread()->isInterruptionRequested();
         --iterations) {
        QThread::sleep(s_sleepInterval);
    }
    QList<DataBlock> returnValues;
    returnValues.reserve(s_fetchBlockSize);
    int number = fromIndex;
    for (int blocks = 0; blocks < s_fetchBlockSize; ++blocks) {
        ++number;
        returnValues.append({QStringLiteral("Contact %1 name").arg(number),
                             QStringLiteral("Contact %1 telephone").arg(number),
                             number});
    }
    return returnValues;
}
