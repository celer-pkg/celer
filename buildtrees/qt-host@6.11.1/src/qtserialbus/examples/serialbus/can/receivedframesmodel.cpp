// Copyright (C) 2021 Evgeny Shtanov <shtanov_evgenii@mail.ru>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "receivedframesmodel.h"

#include <QSize>

#include <iterator>

constexpr int ColumnAlignment[] = {
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignRight | Qt::AlignVCenter,
    Qt::AlignLeft | Qt::AlignVCenter
};

ReceivedFramesModel::ReceivedFramesModel(QObject *parent) : QAbstractTableModel(parent)
{
}

bool ReceivedFramesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    QList<QStringList>::iterator i_start = m_framesQueue.begin() + row;
    QList<QStringList>::iterator i_end = i_start + count;
    m_framesQueue.erase(i_start, i_end);

    endRemoveRows();

    return true;
}

QVariant ReceivedFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
        switch (section) {
        case Number:
            return tr("#");
        case Timestamp:
            return tr("Timestamp");
        case Flags:
            return tr("Flags");
        case CanID:
            return tr("Frame ID");
        case DLC:
            return tr("DLC");
        case Data:
            return tr("Payload");
        }
    }

    if ((role == Qt::SizeHintRole) && (orientation == Qt::Horizontal)) {
        switch (section) {
        case Number:
            return QSize(80, 25);
        case Timestamp:
            return QSize(130, 25);
        case Flags:
            return QSize(50, 25);
        case CanID:
            return QSize(75, 25);
        case DLC:
            return QSize(30, 25);
        case Data:
            return QSize(200, 25);
        }
    }

    return {};
}

QVariant ReceivedFramesModel::data(const QModelIndex &index, int role) const
{
    if (m_framesQueue.empty())
        return {};

    const int row = index.row();
    const int column = index.column();

    switch (role) {
    case Qt::TextAlignmentRole:
        return ColumnAlignment[index.column()];
    case Qt::DisplayRole:
        return m_framesQueue.at(row).at(column);
    case ClipboardTextRole:
        if (index.column() == DLC)
            return QString("[%1]").arg(m_framesQueue.at(row).at(column));
        else
            return m_framesQueue.at(row).at(column);
    default:
        return {};
    }
}

int ReceivedFramesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_framesQueue.size();
}

int ReceivedFramesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : Count;
}

void ReceivedFramesModel::appendFrames(const QList<QStringList> &slvector)
{
    m_framesAccumulator.append(slvector);
}

bool ReceivedFramesModel::needUpdate() const
{
    return !m_framesAccumulator.empty();
}

void ReceivedFramesModel::update()
{
    if (m_framesAccumulator.empty())
        return;

    if (m_queueLimit)
        appendFramesRingBuffer(m_framesAccumulator);
    else
        appendFramesUnlimited(m_framesAccumulator);

    m_framesAccumulator.clear();
}

void ReceivedFramesModel::appendFramesRingBuffer(const QList<QStringList> &slvector)
{
    if (m_queueLimit <= (rowCount() + slvector.size())) {
        if (slvector.size() < m_queueLimit)
            removeRows(0, rowCount() + slvector.size() - m_queueLimit + 1);
        else
            clear();
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount() + slvector.size() - 1);

    if (slvector.size() < m_queueLimit)
        m_framesQueue.append(slvector);
    else
        m_framesQueue.append(slvector.mid(slvector.size() - m_queueLimit));

    endInsertRows();
}

void ReceivedFramesModel::appendFrame(const QStringList &slist)
{
    appendFrames({slist});
}

void ReceivedFramesModel::appendFramesUnlimited(const QList<QStringList> &slvector)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + slvector.size() - 1);

    m_framesQueue.append(slvector);

    endInsertRows();
}

void ReceivedFramesModel::clear()
{
    if (m_framesQueue.count()) {
        beginResetModel();

        m_framesQueue.clear();

        endResetModel();
    }
}

void ReceivedFramesModel::setQueueLimit(int limit)
{
    m_queueLimit = limit;

    if (limit && m_framesQueue.size() > limit)
        removeRows(0, m_framesQueue.size() - limit);
}
