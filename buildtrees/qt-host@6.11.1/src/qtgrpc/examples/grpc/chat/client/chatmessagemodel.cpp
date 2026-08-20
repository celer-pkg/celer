// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatmessagemodel.h"
#include "chatmessages.qpb.h"

ChatMessageModel::ChatMessageModel(QObject *parent) : QAbstractListModel(parent)
{
}

ChatMessageModel::~ChatMessageModel() = default;

int ChatMessageModel::rowCount(const QModelIndex & /*parent*/) const
{
    return int(m_chatMessages.count());
}

QVariant ChatMessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() > m_chatMessages.size())
        return {};
    const auto msg = m_chatMessages[index.row()];
    switch (static_cast<Qt::ItemDataRole>(role)) {
    case Qt::DisplayRole:
        return msg;
    case Qt::WhatsThisRole:
        switch (msg.contentField()) {
        case chat::ChatMessage::ContentFields::Text:
            return "text";
        case chat::ChatMessage::ContentFields::File:
            if (msg.file().hasContinuation())
                return "continuation";
            else if (msg.file().type() == chat::FileMessage::Type::IMAGE)
                return "image";
            else
                return "file";
        case chat::ChatMessage::ContentFields::UserStatus:
            return "userStatus";
        default:
            return {};
        }
    default:
        return {};
    }
}

void ChatMessageModel::appendMessage(const chat::ChatMessage &message)
{
    if (message.hasUserStatus()) {
        const auto &st = message.userStatus();
        if (st.fetched() || st.type() == chat::UserStatus::Type::NONE)
            return;
    }

    if (message.hasFile() && message.file().hasContinuation()) {
        const auto &msgCont = message.file().continuation();
        if (msgCont.index() > 0) {
            for (int i = 0; i < m_chatMessages.length(); ++i) {
                auto &idx = m_chatMessages[i];
                if (idx.hasFile() && idx.file().hasContinuation()) {
                    const auto &idxCont = idx.file().continuation();
                    if (idxCont.uuid() == msgCont.uuid() && idxCont.index() != msgCont.index()) {
                        idx = message; // Update the continuation message
                        if (msgCont.index() == msgCont.count() - 1) { // Last continuation received
                            auto copy = idx;
                            beginRemoveRows(QModelIndex(), i, i);
                            m_chatMessages.remove(i); // Remove the Continuation message
                            endRemoveRows();
                            copy.file().clearContinuation(); // Clear the Continuation
                            appendMessage(copy); // And retrigger this function with a file message
                            return;
                        }
                        emit dataChanged(index(i), index(i));
                        return;
                    }
                }
            }
            qWarning("Continuation not found in chat messages");
            return;
        }
    }

    beginInsertRows(QModelIndex(), int(m_chatMessages.size()), int(m_chatMessages.size()));
    m_chatMessages.append(message);
    endInsertRows();
}
