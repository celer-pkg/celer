// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qremoteobjectabstractitemmodeladapter_p.h"

#include <QtCore/qitemselectionmodel.h>

inline QList<QModelRoleData> createModelRoleData(const QList<int> &roles)
{
    QList<QModelRoleData> roleData;
    roleData.reserve(roles.size());
    for (int role : roles)
        roleData.emplace_back(role);
    return roleData;
}

// consider evaluating performance difference with item data
inline QVariantList collectData(const QModelIndex &index, const QAbstractItemModel *model,
                                QModelRoleDataSpan roleDataSpan)
{
    model->multiData(index, roleDataSpan);

    QVariantList result;
    result.reserve(roleDataSpan.size());
    for (auto &roleData : roleDataSpan)
        result.push_back(std::move(roleData.data()));

    return result;
}

inline QList<int> filterRoles(const QList<int> &roles, const QList<int> &availableRoles)
{
    if (roles.isEmpty())
        return availableRoles;

    QList<int> neededRoles;
    for (int inRole : roles) {
        for (int availableRole : availableRoles)
            if (inRole == availableRole) {
                neededRoles << inRole;
                continue;
            }
    }
    return neededRoles;
}

QAbstractItemModelSourceAdapter::QAbstractItemModelSourceAdapter(QAbstractItemModel *obj, QItemSelectionModel *sel, const QList<int> &roles)
    : QObject(obj),
      m_model(obj),
      m_availableRoles(roles)
{
    QAbstractItemModelSourceAdapter::registerTypes();
    m_selectionModel = sel;
    connect(m_model, &QAbstractItemModel::dataChanged, this, &QAbstractItemModelSourceAdapter::sourceDataChanged);
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &QAbstractItemModelSourceAdapter::sourceRowsInserted);
    connect(m_model, &QAbstractItemModel::columnsInserted, this, &QAbstractItemModelSourceAdapter::sourceColumnsInserted);
    connect(m_model, &QAbstractItemModel::rowsRemoved, this, &QAbstractItemModelSourceAdapter::sourceRowsRemoved);
    connect(m_model, &QAbstractItemModel::rowsMoved, this, &QAbstractItemModelSourceAdapter::sourceRowsMoved);
    connect(m_model, &QAbstractItemModel::layoutChanged, this, &QAbstractItemModelSourceAdapter::sourceLayoutChanged);
    if (m_selectionModel)
        connect(m_selectionModel, &QItemSelectionModel::currentChanged, this, &QAbstractItemModelSourceAdapter::sourceCurrentChanged);
}

void QAbstractItemModelSourceAdapter::registerTypes()
{
    static bool alreadyRegistered = false;
    if (alreadyRegistered)
        return;

    alreadyRegistered = true;
    qRegisterMetaType<QAbstractItemModel*>();
    qRegisterMetaType<Qt::Orientation>();
    qRegisterMetaType<QList<Qt::Orientation>>();
    qRegisterMetaType<QtPrivate::ModelIndex>();
    qRegisterMetaType<QtPrivate::IndexList>();
    qRegisterMetaType<QtPrivate::DataEntries>();
    qRegisterMetaType<QtPrivate::MetaAndDataEntries>();
    qRegisterMetaType<QItemSelectionModel::SelectionFlags>();
    qRegisterMetaType<QSize>();
    qRegisterMetaType<QIntHash>();
    qRegisterMetaType<QList<int>>();
}

QItemSelectionModel* QAbstractItemModelSourceAdapter::selectionModel() const
{
    return m_selectionModel;
}

QSize QAbstractItemModelSourceAdapter::replicaSizeRequest(QtPrivate::IndexList parentList)
{
    QModelIndex parent = toQModelIndex(parentList, m_model);
    const int rowCount = m_model->rowCount(parent);
    const int columnCount = m_model->columnCount(parent);
    const QSize size(columnCount, rowCount);
    qCDebug(QT_REMOTEOBJECT_MODELS) << "parent" << parentList << "size=" << size;
    return size;
}

void QAbstractItemModelSourceAdapter::replicaSetData(const QtPrivate::IndexList &index, const QVariant &value, int role)
{
    const QModelIndex modelIndex = toQModelIndex(index, m_model);
    Q_ASSERT(modelIndex.isValid());
    const bool result = m_model->setData(modelIndex, value, role);
    Q_ASSERT(result);
    Q_UNUSED(result)
}

QtPrivate::DataEntries QAbstractItemModelSourceAdapter::replicaRowRequest(QtPrivate::IndexList start, QtPrivate::IndexList end, QList<int> roles)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << "Requested rows" << "start=" << start << "end=" << end << "roles=" << roles;

    Q_ASSERT(start.size() == end.size());
    Q_ASSERT(!start.isEmpty());

    if (roles.isEmpty())
        roles << m_availableRoles;

    QtPrivate::IndexList parentList = start;
    Q_ASSERT(!parentList.isEmpty());
    parentList.pop_back();
    QModelIndex parent = toQModelIndex(parentList, m_model);

    const int startRow = start.last().row;
    const int startColumn = start.last().column;
    const int rowCount = m_model->rowCount(parent);
    const int columnCount = m_model->columnCount(parent);

    QtPrivate::DataEntries entries;
    if (rowCount <= 0)
        return entries;
    const int endRow = std::min(end.last().row, rowCount - 1);
    const int endColumn = std::min(end.last().column, columnCount - 1);
    Q_ASSERT_X(endRow >= 0 && endRow < rowCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 < %2")).arg(endRow).arg(rowCount)));
    Q_ASSERT_X(endColumn >= 0 && endColumn < columnCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 < %2")).arg(endColumn).arg(columnCount)));

    auto roleData = createModelRoleData(roles);
    for (int row = startRow; row <= endRow; ++row) {
        for (int column = startColumn; column <= endColumn; ++column) {
            const QModelIndex current = m_model->index(row, column, parent);
            Q_ASSERT(current.isValid());
            const QtPrivate::IndexList currentList = QtPrivate::toModelIndexList(current, m_model);
            const QVariantList data = collectData(current, m_model, roleData);
            const bool hasChildren = m_model->hasChildren(current);
            const Qt::ItemFlags flags = m_model->flags(current);
            qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "current=" << currentList << "data=" << data;
            entries.data << QtPrivate::IndexValuePair(currentList, data, hasChildren, flags);
        }
    }
    return entries;
}

QtPrivate::MetaAndDataEntries QAbstractItemModelSourceAdapter::replicaCacheRequest(size_t size, const QList<int> &roles)
{
    QtPrivate::MetaAndDataEntries res;
    res.roles = roles.isEmpty() ? m_availableRoles : roles;
    res.data = fetchTree(QModelIndex {}, size, res.roles);
    const int rowCount = m_model->rowCount(QModelIndex{});
    const int columnCount = m_model->columnCount(QModelIndex{});
    res.size = QSize{columnCount, rowCount};
    return res;
}

QVariantList QAbstractItemModelSourceAdapter::replicaHeaderRequest(QList<Qt::Orientation> orientations, QList<int> sections, QList<int> roles)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "orientations=" << orientations << "sections=" << sections << "roles=" << roles;
    QVariantList data;
    Q_ASSERT(roles.size() == sections.size());
    Q_ASSERT(roles.size() == orientations.size());
    for (int i = 0; i < roles.size(); ++i) {
        data << m_model->headerData(sections[i], orientations[i], roles[i]);
    }
    return data;
}

void QAbstractItemModelSourceAdapter::replicaSetCurrentIndex(QtPrivate::IndexList index, QItemSelectionModel::SelectionFlags command)
{
    if (m_selectionModel)
        m_selectionModel->setCurrentIndex(toQModelIndex(index, m_model), command);
}

void QAbstractItemModelSourceAdapter::sourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QList<int> & roles) const
{
    QList<int> neededRoles = filterRoles(roles, availableRoles());
    if (neededRoles.isEmpty()) {
        qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "Needed roles is empty!";
        return;
    }
    Q_ASSERT(topLeft.isValid());
    Q_ASSERT(bottomRight.isValid());
    QtPrivate::IndexList start = QtPrivate::toModelIndexList(topLeft, m_model);
    QtPrivate::IndexList end = QtPrivate::toModelIndexList(bottomRight, m_model);
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << start << "end=" << end << "neededRoles=" << neededRoles;
    emit dataChanged(start, end, neededRoles);
}

void QAbstractItemModelSourceAdapter::sourceRowsInserted(const QModelIndex & parent, int start, int end)
{
    QtPrivate::IndexList parentList = QtPrivate::toModelIndexList(parent, m_model);
    emit rowsInserted(parentList, start, end);
}

void QAbstractItemModelSourceAdapter::sourceColumnsInserted(const QModelIndex & parent, int start, int end)
{
    QtPrivate::IndexList parentList = QtPrivate::toModelIndexList(parent, m_model);
    emit columnsInserted(parentList, start, end);
}

void QAbstractItemModelSourceAdapter::sourceRowsRemoved(const QModelIndex & parent, int start, int end)
{
    QtPrivate::IndexList parentList = QtPrivate::toModelIndexList(parent, m_model);
    emit rowsRemoved(parentList, start, end);
}

void QAbstractItemModelSourceAdapter::sourceRowsMoved(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild) const
{
    emit rowsMoved(QtPrivate::toModelIndexList(sourceParent, m_model), sourceRow, count, QtPrivate::toModelIndexList(destinationParent, m_model), destinationChild);
}

void QAbstractItemModelSourceAdapter::sourceCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    QtPrivate::IndexList currentIndex = QtPrivate::toModelIndexList(current, m_model);
    QtPrivate::IndexList previousIndex = QtPrivate::toModelIndexList(previous, m_model);
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "current=" << currentIndex << "previous=" << previousIndex;
    emit currentChanged(currentIndex, previousIndex);
}

void QAbstractItemModelSourceAdapter::sourceLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QtPrivate::IndexList indexes;
    for (const QPersistentModelIndex &idx : parents)
        indexes << QtPrivate::toModelIndexList((QModelIndex)idx, m_model);
    emit layoutChanged(indexes, hint);
}

QList<QtPrivate::IndexValuePair> QAbstractItemModelSourceAdapter::fetchTree(const QModelIndex &parent, size_t &size, const QList<int> &roles)
{
    QList<QtPrivate::IndexValuePair> entries;
    const int rowCount = m_model->rowCount(parent);
    const int columnCount = m_model->columnCount(parent);
    if (!columnCount || !rowCount)
        return entries;
    entries.reserve(std::min(rowCount * columnCount, int(size)));
    auto roleData = createModelRoleData(roles);
    for (int row = 0; row < rowCount && size > 0; ++row)
        for (int column = 0; column < columnCount && size > 0; ++column) {
            const auto index = m_model->index(row, column, parent);
            const QtPrivate::IndexList currentList = QtPrivate::toModelIndexList(index, m_model);
            const QVariantList data = collectData(index, m_model, roleData);
            const bool hasChildren = m_model->hasChildren(index);
            const Qt::ItemFlags flags = m_model->flags(index);
            int rc = m_model->rowCount(index);
            int cc = m_model->columnCount(index);
            QtPrivate::IndexValuePair rowData(currentList, data, hasChildren, flags, QSize{cc, rc});
            --size;
            if (hasChildren)
                rowData.children = fetchTree(index, size, roles);
            entries.push_back(rowData);
        }
    return entries;
}
