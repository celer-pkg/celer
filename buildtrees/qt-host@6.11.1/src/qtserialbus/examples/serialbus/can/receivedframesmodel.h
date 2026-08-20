// Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RECEIVEDFRAMESMODEL_H
#define RECEIVEDFRAMESMODEL_H

#include "common.h"

#include <QAbstractTableModel>
#include <QCanBusFrame>
#include <QQueue>

class ReceivedFramesModel : public QAbstractTableModel
{
public:
    explicit ReceivedFramesModel(QObject *parent = nullptr);

    void appendFrame(const QStringList &slist);
    void appendFrames(const QList<QStringList> &slvector);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void clear();
    void setQueueLimit(int limit = 0); // 0 - unlimited
    int getQueueLimit() { return m_queueLimit; }
    bool needUpdate() const;
    void update();

protected:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    void appendFramesRingBuffer(const QList<QStringList> &slvector);
    void appendFramesUnlimited(const QList<QStringList> &slvector);

private:
    QQueue<QStringList> m_framesQueue;
    QList<QStringList> m_framesAccumulator; // Temporary variable to insert frames data
    int m_queueLimit = 0;
};

#endif // RECEIVEDFRAMESMODEL_H
