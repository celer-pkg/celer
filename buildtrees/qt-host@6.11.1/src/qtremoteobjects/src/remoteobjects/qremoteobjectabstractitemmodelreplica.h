// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTS_ABSTRACTITEMMODELREPLICA_H
#define QREMOTEOBJECTS_ABSTRACTITEMMODELREPLICA_H

#include <QtRemoteObjects/qtremoteobjectglobal.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qitemselectionmodel.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModelReplicaImplementation;

class Q_REMOTEOBJECTS_EXPORT QAbstractItemModelReplica : public QAbstractItemModel
{
    Q_OBJECT
public:
    ~QAbstractItemModelReplica() override;

    QItemSelectionModel* selectionModel() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    void multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex & index) const  override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QList<int> availableRoles() const;
    QHash<int, QByteArray> roleNames() const override;

    bool isInitialized() const;
    bool hasData(const QModelIndex &index, int role) const;

    size_t rootCacheSize() const;
    void setRootCacheSize(size_t rootCacheSize);

Q_SIGNALS:
    void initialized();

private:
    explicit QAbstractItemModelReplica(QAbstractItemModelReplicaImplementation *rep, QtRemoteObjects::InitialAction action, const QList<int> &rolesHint);
    QScopedPointer<QAbstractItemModelReplicaImplementation> d;
    friend class QAbstractItemModelReplicaImplementation;
    friend class QRemoteObjectNode;
};

QT_END_NAMESPACE

#endif // QREMOTEOBJECTS_ABSTRACTITEMMODELREPLICA_H
