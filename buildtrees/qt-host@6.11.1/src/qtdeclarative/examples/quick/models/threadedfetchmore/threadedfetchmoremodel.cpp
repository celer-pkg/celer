// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "threadedfetchmoremodel.h"

ThreadedFetchMoreModel::ThreadedFetchMoreModel()
{
    auto worker = new FetchWorker();
    connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);
    worker->moveToThread(&m_workerThread);
    connect(this, &ThreadedFetchMoreModel::fetchDataBlock, worker, &FetchWorker::fetchDataBlock);
    connect(worker, &FetchWorker::dataFetched, this, &ThreadedFetchMoreModel::dataReceived);
    connect(worker, &FetchWorker::noMoreToFetch, this, &ThreadedFetchMoreModel::noMoreToFetch);
    m_workerThread.start();
}

ThreadedFetchMoreModel::~ThreadedFetchMoreModel()
{
    m_workerThread.requestInterruption();
    m_workerThread.quit();
    m_workerThread.wait();
}

int ThreadedFetchMoreModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_dataList.size();
}

QVariant ThreadedFetchMoreModel::data(const QModelIndex &index, int role) const
{
    const auto item = m_dataList.at(index.row());
    switch (static_cast<Role>(role)) {
    case Role::Title:
        return item.title;
    case Role::Subtitle:
        return item.subtitle;
    case Role::Number:
        return item.number;
    case Role::LoadingElement:
        return m_fetchOngoing && index.row() == (rowCount() - 1);
    default:
        break;
    }
    return QVariant{};
}

void ThreadedFetchMoreModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    if (!m_fetchOngoing) {
        addLoadingItem();
        emit fetchDataBlock();
    }
}

bool ThreadedFetchMoreModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_fetchOngoing)
        return false;
    return m_hasUnfetchedItems;
}

void ThreadedFetchMoreModel::dataReceived(const QList<FetchWorker::DataBlock> &items)
{
    removeLoadingItem();
    beginInsertRows(QModelIndex{}, rowCount(), rowCount() + items.size() - 1);
    m_dataList.append(items);
    endInsertRows();
}

void ThreadedFetchMoreModel::noMoreToFetch()
{
    m_hasUnfetchedItems = false;
}

void ThreadedFetchMoreModel::addLoadingItem()
{
    beginInsertRows(QModelIndex{}, rowCount(), rowCount());
    m_fetchOngoing = true;
    m_dataList.append({{}, {}, 0});
    endInsertRows();
}

void ThreadedFetchMoreModel::removeLoadingItem()
{
    beginRemoveRows(QModelIndex{}, rowCount() - 1, rowCount() - 1);
    m_fetchOngoing = false;
    m_dataList.removeAt(rowCount() - 1);
    endRemoveRows();
}

QHash<int, QByteArray> ThreadedFetchMoreModel::roleNames() const
{
    static const QHash<int, QByteArray> names = {{static_cast<int>(Role::Title), "title"},
                                                 {static_cast<int>(Role::Subtitle), "subtitle"},
                                                 {static_cast<int>(Role::Number), "number"},
                                                 {static_cast<int>(Role::LoadingElement),
                                                  "isLoadingElement"}};
    return names;
}
