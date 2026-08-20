// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ARROWSMODEL_H
#define ARROWSMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

class ArrowsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    struct Arrow {
        float startX = 0;
        float startY = 0;
        float endX = 0;
        float endY = 0;
        int startNodeId = 0;
        int endNodeId = 1;
        bool operator==(const Arrow& rhs) const noexcept
        {
           return this->startNodeId == rhs.startNodeId;
        }
    };

    enum NodesModelRoles {
        Type = Qt::UserRole + 1,
        StartX,
        StartY,
        EndX,
        EndY,
        StartNodeId,
        EndNodeId
    };

    explicit ArrowsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

signals:
    void rowCountChanged();

private:
    friend class NodeView;
    friend class EffectManager;
    QList<Arrow> m_arrowsList;

};

#endif // ARROWSMODEL_H
