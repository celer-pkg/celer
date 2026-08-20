// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef THREADEDFETCHMOREMODEL_H
#define THREADEDFETCHMOREMODEL_H
#include <fetchworker.h>

#include <QAbstractListModel>
#include <QThread>
#include <QtQml/qqmlregistration.h>

class ThreadedFetchMoreModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    ThreadedFetchMoreModel();
    ~ThreadedFetchMoreModel();
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;
    Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void fetchDataBlock();

protected:
    enum class Role { Title = Qt::ItemDataRole::UserRole, Subtitle, Number, LoadingElement };

private slots:
    void dataReceived(const QList<FetchWorker::DataBlock> &items);
    void noMoreToFetch();

private:
    void addLoadingItem();
    void removeLoadingItem();
    QList<FetchWorker::DataBlock> m_dataList;
    QThread m_workerThread;
    bool m_fetchOngoing{false};
    bool m_hasUnfetchedItems{true};
};

#endif // UNTHREADEDFETCHMOREMODEL_H
