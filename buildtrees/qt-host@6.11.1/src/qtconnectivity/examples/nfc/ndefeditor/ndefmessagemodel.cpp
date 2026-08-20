// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "ndefmessagemodel.h"

#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>

NdefMessageModel::NdefMessageModel(QObject *parent) : QAbstractListModel(parent) { }

int NdefMessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_message.size();
}

static NdefMessageModel::RecordType getRecordType(const QNdefRecord &record)
{
    if (record.isRecordType<QNdefNfcTextRecord>()) {
        return NdefMessageModel::TextRecord;
    } else if (record.isRecordType<QNdefNfcUriRecord>()) {
        return NdefMessageModel::UriRecord;
    }
    return NdefMessageModel::OtherRecord;
}

static QString getText(const QNdefRecord &record)
{
    if (record.isRecordType<QNdefNfcTextRecord>()) {
        QNdefNfcTextRecord r(record);
        return r.text();
    } else if (record.isRecordType<QNdefNfcUriRecord>()) {
        QNdefNfcUriRecord r(record);
        return r.uri().toString();
    }
    return record.payload().toHex(':');
}

QVariant NdefMessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() < 0 || index.row() >= m_message.size())
        return {};

    const auto &record = m_message.at(index.row());

    switch (role) {
    case RecordTypeRole:
        return getRecordType(record);
    case RecordTextRole:
        return getText(record);
    default:
        return {};
    }
}

QHash<int, QByteArray> NdefMessageModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[RecordTypeRole] = "recordType";
    names[RecordTextRole] = "recordText";
    return names;
}

bool NdefMessageModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    if (row < 0 || count <= 0)
        return false;
    if (row >= m_message.size() || (m_message.size() - row) < count)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    m_message.remove(row, count);
    endRemoveRows();
    Q_EMIT messageChanged();

    return true;
}

QNdefMessage NdefMessageModel::message() const
{
    return m_message;
}

void NdefMessageModel::setMessage(const QNdefMessage &newMessage)
{
    if (m_message == newMessage)
        return;

    beginResetModel();
    m_message = newMessage;
    endResetModel();

    Q_EMIT messageChanged();
}

void NdefMessageModel::clearMessage()
{
    if (m_message.isEmpty())
        return;
    removeRows(0, m_message.size(), {});
}

void NdefMessageModel::addTextRecord(const QString &text)
{
    QNdefNfcTextRecord record;
    record.setText(text);

    const auto newRow = m_message.size();
    beginInsertRows({}, newRow, newRow);
    m_message.append(std::move(record));
    endInsertRows();
    Q_EMIT messageChanged();
}

void NdefMessageModel::addUriRecord(const QString &uri)
{
    QNdefNfcUriRecord record;
    record.setUri(QUrl(uri));

    const auto newRow = m_message.size();
    beginInsertRows({}, newRow, newRow);
    m_message.append(std::move(record));
    endInsertRows();
    Q_EMIT messageChanged();
}

void NdefMessageModel::setTextData(int row, const QString &text)
{
    if (row < 0 || row >= m_message.size())
        return;

    const auto &record = m_message.at(row);

    if (record.isRecordType<QNdefNfcTextRecord>()) {
        QNdefNfcTextRecord r(record);
        r.setText(text);
        m_message[row] = r;
    } else if (record.isRecordType<QNdefNfcUriRecord>()) {
        QNdefNfcUriRecord r(record);
        r.setUri(text);
        m_message[row] = r;
    } else {
        return;
    }

    const auto idx = index(row);
    Q_EMIT dataChanged(idx, idx, { RecordTextRole });
    Q_EMIT messageChanged();
}
