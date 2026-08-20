// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CHATMESSAGEMODEL_H
#define CHATMESSAGEMODEL_H

#include "chatmessages.qpb.h"

#include <QtQmlIntegration/qqmlintegration.h>

#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtCore/QStringView>

class ChatMessageModel : public QAbstractListModel
{
    Q_OBJECT
    QML_INTERFACE

public:
    explicit ChatMessageModel(QObject *parent = nullptr);
    ~ChatMessageModel() override;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void appendMessage(const chat::ChatMessage &message);

private:
    QList<chat::ChatMessage> m_chatMessages;
    Q_DISABLE_COPY_MOVE(ChatMessageModel)
};

#endif // CHATMESSAGEMODEL_H
