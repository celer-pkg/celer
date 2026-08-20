// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QtGrpc/QtGrpcNamespace>

#include <QtQmlIntegration/QtQmlIntegration>

#include <QtCore/QAbstractListModel>
#include <QtCore/QString>

class LogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Role {
        LevelRole = Qt::UserRole + 1,
        TimestampRole,
        OperationIdRole,
        ServiceRole,
        MethodRole,
        RpcTypeRole,
        MessageRole,
    };
    Q_ENUM(Role)
    enum class Level { Debug, Info, Warning, Error };
    Q_ENUM(Level)

    explicit LogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = { }) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void add(Level level, quint64 operationId, QtGrpc::RpcDescriptor descriptor, QString message);

private:
    struct LogEntry
    {
        Level level;
        QString timestamp;
        quint64 operationId;
        QtGrpc::RpcDescriptor descriptor;
        QString message;
    };
    QList<LogEntry> m_entries;
};

#endif // LOGMODEL_H
