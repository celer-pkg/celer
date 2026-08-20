// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "logmodel.h"

#include <QtCore/QDateTime>

using namespace Qt::Literals::StringLiterals;

LogModel::LogModel(QObject *parent) : QAbstractListModel(parent)
{
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid))
        return {};

    const auto &e = m_entries.at(index.row());
    switch (role) {
    case LevelRole:
        return QVariant::fromValue(e.level);
    case TimestampRole:
        return e.timestamp;
    case OperationIdRole:
        return e.operationId;
    case ServiceRole:
        return e.descriptor.service;
    case MethodRole:
        return e.descriptor.method;
    case RpcTypeRole:
        return QVariant::fromValue(e.descriptor.type);
    case MessageRole:
        return e.message;
    default:
        return {};
    }
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    return {
        { LevelRole,       "level"       },
        { TimestampRole,   "timestamp"   },
        { OperationIdRole, "operationId" },
        { ServiceRole,     "service"     },
        { MethodRole,      "method"      },
        { RpcTypeRole,     "rpcType"     },
        { MessageRole,     "message"     },
    };
}

void LogModel::add(Level level, quint64 operationId, QtGrpc::RpcDescriptor descriptor,
                   QString message)
{
    const static QString logFormat("dd/MM/yyyy, hh:mm:ss.zz");

    LogEntry entry{
        level,
        QDateTime::currentDateTime().toString(logFormat),
        operationId,
        descriptor,
        std::move(message),
    };

    beginInsertRows({ }, 0, 0);
    m_entries.prepend(std::move(entry));
    endInsertRows();
}
