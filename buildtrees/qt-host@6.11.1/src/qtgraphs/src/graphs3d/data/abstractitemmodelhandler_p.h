// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef ABSTRACTITEMMODELHANDLER_P_H
#define ABSTRACTITEMMODELHANDLER_P_H

#include <QtGraphs/private/qgraphsglobal_p.h>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

class AbstractItemModelHandler : public QObject
{
    Q_OBJECT
public:
    AbstractItemModelHandler(QObject *parent = 0);
    ~AbstractItemModelHandler() override;

    virtual void setItemModel(QAbstractItemModel *itemModel);
    virtual QAbstractItemModel *itemModel() const;

public Q_SLOTS:
    virtual void handleColumnsInserted(const QModelIndex &parent, int start, int end);
    virtual void handleColumnsMoved(const QModelIndex &sourceParent,
                                    int sourceStart,
                                    int sourceEnd,
                                    const QModelIndex &destinationParent,
                                    int destinationColumn);
    virtual void handleColumnsRemoved(const QModelIndex &parent, int start, int end);
    virtual void handleDataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight,
                                   const QList<int> &roles = QList<int>());
    virtual void handleLayoutChanged(
        const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(),
        QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);
    virtual void handleModelReset();
    virtual void handleRowsInserted(const QModelIndex &parent, int start, int end);
    virtual void handleRowsMoved(const QModelIndex &sourceParent,
                                 int sourceStart,
                                 int sourceEnd,
                                 const QModelIndex &destinationParent,
                                 int destinationRow);
    virtual void handleRowsRemoved(const QModelIndex &parent, int start, int end);

    virtual void handleMappingChanged();
    virtual void handlePendingResolve();

Q_SIGNALS:
    void itemModelChanged(const QAbstractItemModel *itemModel);

protected:
    virtual void resolveModel() = 0;

    QPointer<QAbstractItemModel> m_itemModel; // Not owned
    bool resolvePending;
    QTimer m_resolveTimer;
    bool m_fullReset;
    const int noRoleIndex = -1;

private:
    Q_DISABLE_COPY(AbstractItemModelHandler)
};

QT_END_NAMESPACE

#endif
