// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qremoteobjectabstractitemmodelreplica.h"
#include "qremoteobjectabstractitemmodelreplica_p.h"

#include "qremoteobjectnode.h"

#include <QtCore/qdebug.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QtPrivate::ModelIndex, QtPrivate__ModelIndex)
QT_IMPL_METATYPE_EXTERN_TAGGED(QtPrivate::IndexList, QtPrivate__IndexList)
QT_IMPL_METATYPE_EXTERN_TAGGED(QtPrivate::DataEntries, QtPrivate__DataEntries)
QT_IMPL_METATYPE_EXTERN_TAGGED(QtPrivate::MetaAndDataEntries, QtPrivate__MetaAndDataEntries)
QT_IMPL_METATYPE_EXTERN_TAGGED(QtPrivate::IndexValuePair, QtPrivate__IndexValuePair)
QT_IMPL_METATYPE_EXTERN_TAGGED(Qt::Orientation, Qt__Orientation)
QT_IMPL_METATYPE_EXTERN_TAGGED(QItemSelectionModel::SelectionFlags,
                               QItemSelectionModel__SelectionFlags)

inline QDebug operator<<(QDebug stream, const RequestedData &data)
{
    return stream.nospace() << "RequestedData[start=" << data.start << ", end=" << data.end << ", roles=" << data.roles << "]";
}

CacheData::CacheData(QAbstractItemModelReplicaImplementation *model, CacheData *parentItem)
    : replicaModel(model)
    , parent(parentItem)
    , hasChildren(false)
    , columnCount(0)
    , rowCount(0)
{
    if (parent)
        replicaModel->m_activeParents.insert(parent);
}

CacheData::~CacheData() {
    if (parent && !replicaModel->m_activeParents.empty())
        replicaModel->m_activeParents.erase(this);
}

QAbstractItemModelReplicaImplementation::QAbstractItemModelReplicaImplementation()
    : QRemoteObjectReplica()
    , m_selectionModel(nullptr)
    , m_rootItem(this)
{
    QAbstractItemModelReplicaImplementation::registerMetatypes();
    initializeModelConnections();
    connect(this, &QAbstractItemModelReplicaImplementation::availableRolesChanged, this, [this]{
        m_availableRoles.clear();
    });
}

QAbstractItemModelReplicaImplementation::QAbstractItemModelReplicaImplementation(QRemoteObjectNode *node, const QString &name)
    : QRemoteObjectReplica(ConstructWithNode)
    , m_selectionModel(nullptr)
    , m_rootItem(this)
{
    QAbstractItemModelReplicaImplementation::registerMetatypes();
    initializeModelConnections();
    initializeNode(node, name);
    connect(this, &QAbstractItemModelReplicaImplementation::availableRolesChanged, this, [this]{
        m_availableRoles.clear();
    });
}

QAbstractItemModelReplicaImplementation::~QAbstractItemModelReplicaImplementation()
{
    m_rootItem.clear();
    qDeleteAll(m_pendingRequests);
}

void QAbstractItemModelReplicaImplementation::initialize()
{
    QVariantList properties;
    properties << QVariant::fromValue(QList<int>());
    properties << QVariant::fromValue(QIntHash());
    setProperties(std::move(properties));
}

void QAbstractItemModelReplicaImplementation::registerMetatypes()
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

void QAbstractItemModelReplicaImplementation::initializeModelConnections()
{
    connect(this, &QAbstractItemModelReplicaImplementation::dataChanged, this, &QAbstractItemModelReplicaImplementation::onDataChanged);
    connect(this, &QAbstractItemModelReplicaImplementation::rowsInserted, this, &QAbstractItemModelReplicaImplementation::onRowsInserted);
    connect(this, &QAbstractItemModelReplicaImplementation::columnsInserted, this, &QAbstractItemModelReplicaImplementation::onColumnsInserted);
    connect(this, &QAbstractItemModelReplicaImplementation::rowsRemoved, this, &QAbstractItemModelReplicaImplementation::onRowsRemoved);
    connect(this, &QAbstractItemModelReplicaImplementation::rowsMoved, this, &QAbstractItemModelReplicaImplementation::onRowsMoved);
    connect(this, &QAbstractItemModelReplicaImplementation::currentChanged, this, &QAbstractItemModelReplicaImplementation::onCurrentChanged);
    connect(this, &QAbstractItemModelReplicaImplementation::modelReset, this, &QAbstractItemModelReplicaImplementation::onModelReset);
    connect(this, &QAbstractItemModelReplicaImplementation::headerDataChanged, this, &QAbstractItemModelReplicaImplementation::onHeaderDataChanged);
    connect(this, &QAbstractItemModelReplicaImplementation::layoutChanged, this, &QAbstractItemModelReplicaImplementation::onLayoutChanged);

}

inline void removeIndexFromRow(const QModelIndex &index, const QList<int> &roles, CachedRowEntry *entry)
{
    CachedRowEntry &entryRef = *entry;
    if (index.column() < entryRef.size()) {
        CacheEntry &entry = entryRef[index.column()];
        if (roles.isEmpty()) {
            entry.data.clear();
        } else {
            for (int role : roles)
                entry.data.remove(role);
        }
    }
}

void QAbstractItemModelReplicaImplementation::onReplicaCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    QtPrivate::IndexList currentIndex = QtPrivate::toModelIndexList(current, q);
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "current=" << currentIndex;
    replicaSetCurrentIndex(currentIndex, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Current);
}

void QAbstractItemModelReplicaImplementation::setModel(QAbstractItemModelReplica *model)
{
    q = model;
    setParent(model);
    m_selectionModel.reset(new QItemSelectionModel(model));
    connect(m_selectionModel.data(), &QItemSelectionModel::currentChanged, this, &QAbstractItemModelReplicaImplementation::onReplicaCurrentChanged);
}

bool QAbstractItemModelReplicaImplementation::clearCache(const QtPrivate::IndexList &start, const QtPrivate::IndexList &end, const QList<int> &roles = QList<int>())
{
    Q_ASSERT(start.size() == end.size());

    bool ok = true;
    const QModelIndex startIndex = toQModelIndex(start, q, &ok);
    if (!ok)
        return false;
    const QModelIndex endIndex = toQModelIndex(end, q, &ok);
    if (!ok)
        return false;
    Q_ASSERT(startIndex.isValid());
    Q_ASSERT(endIndex.isValid());
    Q_ASSERT(startIndex.parent() == endIndex.parent());
    Q_UNUSED(endIndex)
    QModelIndex parentIndex = startIndex.parent();
    auto parentItem = cacheData(parentIndex);

    const int startRow = start.last().row;
    const int lastRow = end.last().row;
    const int startColumn = start.last().column;
    const int lastColumn = end.last().column;
    for (int row = startRow; row <= lastRow; ++row) {
        Q_ASSERT_X(row >= 0 && row < parentItem->rowCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 < %2")).arg(row).arg(parentItem->rowCount)));
        auto item = parentItem->children.get(row);
        if (item) {
            CachedRowEntry *entry = &(item->cachedRowEntry);
            for (int column = startColumn; column <= lastColumn; ++column)
                removeIndexFromRow(q->index(row, column, parentIndex), roles, entry);
        }
    }
    return true;
}

void QAbstractItemModelReplicaImplementation::onDataChanged(const QtPrivate::IndexList &start, const QtPrivate::IndexList &end, const QList<int> &roles)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << start << "end=" << end << "roles=" << roles;

    // we need to clear the cache to make sure the new remote data is fetched if the new data call is happening
    if (clearCache(start, end, roles)) {
        bool ok = true;
        const QModelIndex startIndex = toQModelIndex(start, q, &ok);
        if (!ok)
            return;
        const QModelIndex endIndex = toQModelIndex(end, q, &ok);
        if (!ok)
            return;
        Q_ASSERT(startIndex.parent() == endIndex.parent());
        auto parentItem = cacheData(startIndex.parent());
        int startRow = start.last().row;
        int endRow = end.last().row;
        bool dataChanged = false;
        while (startRow <= endRow) {
            for (;startRow <= endRow; startRow++) {
                if (parentItem->children.exists(startRow))
                    break;
            }

            if (startRow  > endRow)
                break;

            RequestedData data;
            data.roles = roles;
            data.start = start;
            data.start.last().row = startRow;

            while (startRow <= endRow && parentItem->children.exists(startRow))
                ++startRow;

            data.end = end;
            data.end.last().row = startRow -1;

            m_requestedData.append(data);
            dataChanged = true;
        }

        if (dataChanged)
            QMetaObject::invokeMethod(this, "fetchPendingData", Qt::QueuedConnection);
    }
}

void QAbstractItemModelReplicaImplementation::onRowsInserted(const QtPrivate::IndexList &parent, int start, int end)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << start << "end=" << end << "parent=" << parent;

    bool treeFullyLazyLoaded = true;
    const QModelIndex parentIndex = toQModelIndex(parent, q, &treeFullyLazyLoaded, true);
    if (!treeFullyLazyLoaded)
        return;

    auto parentItem = cacheData(parentIndex);
    q->beginInsertRows(parentIndex, start, end);
    parentItem->insertChildren(start, end);
    for (int i = start; i <= end; ++i)
        m_headerData[1].append(CacheEntry());
    q->endInsertRows();
    if (!parentItem->hasChildren && parentItem->columnCount > 0) {
        parentItem->hasChildren = true;
        emit q->dataChanged(parentIndex, parentIndex);
    }
}

void QAbstractItemModelReplicaImplementation::onColumnsInserted(const QtPrivate::IndexList &parent, int start, int end)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << start << "end=" << end << "parent=" << parent;

    bool treeFullyLazyLoaded = true;
    const QModelIndex parentIndex = toQModelIndex(parent, q, &treeFullyLazyLoaded);
    if (!treeFullyLazyLoaded)
        return;

    //Since we need to support QAIM and models that don't emit columnCountChanged
    //check if we have a constant columnCount everywhere if thats the case don't insert
    //more columns
    auto parentItem = cacheData(parentIndex);
    auto parentOfParent = parentItem->parent;
    if (parentOfParent && parentItem != &m_rootItem)
        if (parentOfParent->columnCount == parentItem->columnCount)
            return;
    q->beginInsertColumns(parentIndex, start, end);
    parentItem->columnCount += end - start + 1;
    for (int i = start; i <= end; ++i)
        m_headerData[0].append(CacheEntry());
    q->endInsertColumns();
    if (!parentItem->hasChildren && parentItem->children.size() > 0) {
        parentItem->hasChildren = true;
        emit q->dataChanged(parentIndex, parentIndex);
    }

}

void QAbstractItemModelReplicaImplementation::onRowsRemoved(const QtPrivate::IndexList &parent, int start, int end)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << start << "end=" << end << "parent=" << parent;

    bool treeFullyLazyLoaded = true;
    const QModelIndex parentIndex = toQModelIndex(parent, q, &treeFullyLazyLoaded);
    if (!treeFullyLazyLoaded)
        return;

    auto parentItem = cacheData(parentIndex);
    q->beginRemoveRows(parentIndex, start, end);
    if (parentItem)
        parentItem->removeChildren(start, end);
    m_headerData[1].erase(m_headerData[1].begin() + start, m_headerData[1].begin() + end + 1);
    q->endRemoveRows();
}

void QAbstractItemModelReplicaImplementation::onRowsMoved(QtPrivate::IndexList srcParent, int srcRow, int count, QtPrivate::IndexList destParent, int destRow)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO;

    const QModelIndex sourceParent = toQModelIndex(srcParent, q);
    const QModelIndex destinationParent = toQModelIndex(destParent, q);
    Q_ASSERT(!sourceParent.isValid());
    Q_ASSERT(!destinationParent.isValid());
    q->beginMoveRows(sourceParent, srcRow, count, destinationParent, destRow);
//TODO misses parents...
    QtPrivate::IndexList start, end;
    start << QtPrivate::ModelIndex(srcRow, 0);
    end << QtPrivate::ModelIndex(srcRow + count, q->columnCount(sourceParent)-1);
    clearCache(start, end);
    QtPrivate::IndexList start2, end2;
    start2 << QtPrivate::ModelIndex(destRow, 0);
    end2 << QtPrivate::ModelIndex(destRow + count, q->columnCount(destinationParent)-1);
    clearCache(start2, end2);
    q->endMoveRows();
}

void QAbstractItemModelReplicaImplementation::onCurrentChanged(QtPrivate::IndexList current, QtPrivate::IndexList previous)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "current=" << current << "previous=" << previous;
    Q_UNUSED(previous)
    Q_ASSERT(m_selectionModel);
    bool ok;
    // If we have several tree models sharing a selection model, we
    // can't guarantee that all Replicas have the selected cell
    // available.
    const QModelIndex currentIndex = toQModelIndex(current, q, &ok);
    // Ignore selection if we can't find the desired cell.
    if (ok)
        m_selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Current);
}

void QAbstractItemModelReplicaImplementation::handleInitDone(QRemoteObjectPendingCallWatcher *watcher)
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO;

    handleModelResetDone(watcher);
    m_initDone = true;
    emit q->initialized();
}

void QAbstractItemModelReplicaImplementation::handleModelResetDone(QRemoteObjectPendingCallWatcher *watcher)
{
    QSize size;
    if (m_initialAction == QtRemoteObjects::FetchRootSize)
        size = watcher->returnValue().toSize();
    else {
        Q_ASSERT(watcher->returnValue().canConvert<QtPrivate::MetaAndDataEntries>());
        size = watcher->returnValue().value<QtPrivate::MetaAndDataEntries>().size;
    }

    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "size=" << size;

    q->beginResetModel();
    m_rootItem.clear();
    if (size.height() > 0) {
        m_rootItem.rowCount = size.height();
        m_rootItem.hasChildren = true;
    }

    m_rootItem.columnCount = size.width();
    m_headerData[0].resize(size.width());
    m_headerData[1].resize(size.height());
    {
        QList<CacheEntry> &headerEntries = m_headerData[0];
        for (int i = 0; i < size.width(); ++i )
            headerEntries[i].data.clear();
    }
    {
        QList<CacheEntry> &headerEntries = m_headerData[1];
        for (int i = 0; i < size.height(); ++i )
            headerEntries[i].data.clear();
    }
    if (m_initialAction == QtRemoteObjects::PrefetchData) {
        auto entries = watcher->returnValue().value<QtPrivate::MetaAndDataEntries>();
        for (int i = 0; i < entries.data.size(); ++i)
            fillCache(entries.data[i], entries.roles);
    }
    q->endResetModel();
    m_pendingRequests.removeAll(watcher);
    delete watcher;
}

void QAbstractItemModelReplicaImplementation::handleSizeDone(QRemoteObjectPendingCallWatcher *watcher)
{
    SizeWatcher *sizeWatcher = static_cast<SizeWatcher*>(watcher);
    const QSize size = sizeWatcher->returnValue().toSize();
    auto parentItem = cacheData(sizeWatcher->parentList);
    const QModelIndex parent = toQModelIndex(sizeWatcher->parentList, q);

    if (size.width() != parentItem->columnCount) {
        const int columnCount = std::max(0, parentItem->columnCount);
        Q_ASSERT_X(size.width() >= parentItem->columnCount, __FUNCTION__, "The column count should only shrink in columnsRemoved!!");
        parentItem->columnCount = size.width();
        if (size.width() > columnCount) {
            Q_ASSERT(size.width() > 0);
            q->beginInsertColumns(parent, columnCount, size.width() - 1);
            q->endInsertColumns();
        } else {
            Q_ASSERT_X(size.width() == columnCount, __FUNCTION__, qPrintable(QString(QLatin1String("%1 != %2")).arg(size.width()).arg(columnCount)));
        }
    }

    Q_ASSERT_X(size.height() >= parentItem->rowCount, __FUNCTION__, "The new size and the current size should match!!");
    if (!parentItem->rowCount) {
        if (size.height() > 0) {
            q->beginInsertRows(parent, 0, size.height() - 1);
            parentItem->rowCount = size.height();
            q->endInsertRows();
        }
    } else {
        Q_ASSERT_X(parentItem->rowCount == size.height(), __FUNCTION__, qPrintable(QString(QLatin1String("%1 != %2")).arg(parentItem->rowCount).arg(size.height())));
    }
    m_pendingRequests.removeAll(watcher);
    delete watcher;
}

void QAbstractItemModelReplicaImplementation::init()
{
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << this->node()->objectName();
    QRemoteObjectPendingCallWatcher *watcher = doModelReset();
    connect(watcher, &QRemoteObjectPendingCallWatcher::finished, this, &QAbstractItemModelReplicaImplementation::handleInitDone);
}

QRemoteObjectPendingCallWatcher* QAbstractItemModelReplicaImplementation::doModelReset()
{
    qDeleteAll(m_pendingRequests);
    m_pendingRequests.clear();
    QtPrivate::IndexList parentList;
    QRemoteObjectPendingCallWatcher *watcher;
    if (m_initialAction == QtRemoteObjects::FetchRootSize) {
        auto call = replicaSizeRequest(parentList);
        watcher = new SizeWatcher(parentList, call);
    } else {
        auto call = replicaCacheRequest(m_rootItem.children.cacheSize, m_initialFetchRolesHint);
        watcher = new QRemoteObjectPendingCallWatcher(call);
    }
    m_pendingRequests.push_back(watcher);
    return watcher;
}

inline void fillCacheEntry(CacheEntry *entry, const QtPrivate::IndexValuePair &pair, const QList<int> &roles)
{
    Q_ASSERT(entry);

    const QVariantList &data = pair.data;
    Q_ASSERT(roles.size() == data.size());

    entry->flags = pair.flags;

    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "data.size=" << data.size();
    for (int i = 0; i < data.size(); ++i) {
        const int role = roles[i];
        const QVariant dataVal = data[i];
        qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "role=" << role << "data=" << dataVal;
        entry->data[role] = dataVal;
    }
}

inline void fillRow(CacheData *item, const QtPrivate::IndexValuePair &pair, const QAbstractItemModel *model, const QList<int> &roles)
{
    CachedRowEntry &rowRef = item->cachedRowEntry;
    const QModelIndex index = toQModelIndex(pair.index, model);
    Q_ASSERT(index.isValid());
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "row=" << index.row() << "column=" << index.column();
    if (index.column() == 0)
        item->hasChildren = pair.hasChildren;
    bool existed = false;
    for (int i = 0; i < rowRef.size(); ++i) {
        if (i == index.column()) {
            fillCacheEntry(&rowRef[i], pair, roles);
            existed = true;
        }
    }
    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "existed=" << existed;
    if (!existed) {
        CacheEntry entries;
        fillCacheEntry(&entries, pair, roles);
        rowRef.append(entries);
    }
}

int collectEntriesForRow(QtPrivate::DataEntries* filteredEntries, int row, const QtPrivate::DataEntries &entries, int startIndex)
{
    Q_ASSERT(filteredEntries);
    const int size = int(entries.data.size());
    for (int i = startIndex; i < size; ++i)
    {
        const QtPrivate::IndexValuePair &pair = entries.data[i];
        if (pair.index.last().row == row)
            filteredEntries->data << pair;
        else
            return i;
    }
    return size;
}

void QAbstractItemModelReplicaImplementation::fillCache(const QtPrivate::IndexValuePair &pair, const QList<int> &roles)
{
    if (auto item = createCacheData(pair.index)) {
        fillRow(item, pair, q, roles);
        item->rowCount = pair.size.height();
        item->columnCount = pair.size.width();
    }
    for (const auto &it : pair.children)
        fillCache(it, roles);
}

void QAbstractItemModelReplicaImplementation::requestedData(QRemoteObjectPendingCallWatcher *qobject)
{
    RowWatcher *watcher = static_cast<RowWatcher *>(qobject);
    Q_ASSERT(watcher);
    Q_ASSERT(watcher->start.size() == watcher->end.size());

    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "start=" << watcher->start << "end=" << watcher->end;

    QtPrivate::IndexList parentList = watcher->start;
    Q_ASSERT(!parentList.isEmpty());
    parentList.pop_back();
    auto parentItem = cacheData(parentList);
    QtPrivate::DataEntries entries = watcher->returnValue().value<QtPrivate::DataEntries>();

    const int rowCount = parentItem->rowCount;
    const int columnCount = parentItem->columnCount;

    if (rowCount < 1 || columnCount < 1)
        return;

    const int startRow =  std::min(watcher->start.last().row, rowCount - 1);
    const int endRow = std::min(watcher->end.last().row, rowCount - 1);
    const int startColumn = std::min(watcher->start.last().column, columnCount - 1);
    const int endColumn = std::min(watcher->end.last().column, columnCount - 1);
    Q_ASSERT_X(startRow >= 0 && startRow < parentItem->rowCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 < %2")).arg(startRow).arg(parentItem->rowCount)));
    Q_ASSERT_X(endRow >= 0 && endRow < parentItem->rowCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 < %2")).arg(endRow).arg(parentItem->rowCount)));

    for (int i = 0; i < entries.data.size(); ++i) {
        QtPrivate::IndexValuePair pair = entries.data[i];
        if (auto item = createCacheData(pair.index))
            fillRow(item, pair, q, watcher->roles);
    }

    const QModelIndex parentIndex = toQModelIndex(parentList, q);
    const QModelIndex startIndex = q->index(startRow, startColumn, parentIndex);
    const QModelIndex endIndex = q->index(endRow, endColumn, parentIndex);
    Q_ASSERT(startIndex.isValid());
    Q_ASSERT(endIndex.isValid());
    emit q->dataChanged(startIndex, endIndex, watcher->roles);
    m_pendingRequests.removeAll(watcher);
    delete watcher;
}

void QAbstractItemModelReplicaImplementation::fetchPendingData()
{
    if (m_requestedData.isEmpty())
        return;

    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "m_requestedData.size=" << m_requestedData.size();

    std::vector<RequestedData> finalRequests;
    RequestedData curData;
    for (const RequestedData &data : std::exchange(m_requestedData, {})) {
        qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "REQUESTED start=" << data.start << "end=" << data.end << "roles=" << data.roles;

        Q_ASSERT(!data.start.isEmpty());
        Q_ASSERT(!data.end.isEmpty());
        Q_ASSERT(data.start.size() == data.end.size());
        if (curData.start.isEmpty() || curData.start.last().row == -1 || curData.start.last().column == -1)
            curData = data;
        if (curData.start.size() != data.start.size()) {
            finalRequests.push_back(curData);
            curData = data;
        } else {
            if (data.start.size() > 1) {
                for (int i = 0; i < data.start.size() - 1; ++i) {
                    if (curData.start[i].row != data.start[i].row ||
                        curData.start[i].column != data.start[i].column) {
                        finalRequests.push_back(curData);
                        curData = data;
                    }
                }
            }

            const QtPrivate::ModelIndex curIndStart = curData.start.last();
            const QtPrivate::ModelIndex curIndEnd = curData.end.last();
            const QtPrivate::ModelIndex dataIndStart = data.start.last();
            const QtPrivate::ModelIndex dataIndEnd = data.end.last();
            const QtPrivate::ModelIndex resStart(std::min(curIndStart.row, dataIndStart.row), std::min(curIndStart.column, dataIndStart.column));
            const QtPrivate::ModelIndex resEnd(std::max(curIndEnd.row, dataIndEnd.row), std::max(curIndEnd.column, dataIndEnd.column));
            QList<int> roles = curData.roles;
            if (!curData.roles.isEmpty()) {
                for (int role : data.roles) {
                    if (!curData.roles.contains(role))
                        roles.append(role);
                }
            }
            QRect firstRect( QPoint(curIndStart.row, curIndStart.column), QPoint(curIndEnd.row, curIndEnd.column));
            QRect secondRect( QPoint(dataIndStart.row, dataIndStart.column), QPoint(dataIndEnd.row, dataIndEnd.column));

            const bool borders = (qAbs(curIndStart.row - dataIndStart.row) == 1) ||
                                 (qAbs(curIndStart.column - dataIndStart.column) == 1) ||
                                 (qAbs(curIndEnd.row - dataIndEnd.row) == 1) ||
                                 (qAbs(curIndEnd.column - dataIndEnd.column) == 1);

            if ((resEnd.row - resStart.row < 100) && (firstRect.intersects(secondRect) || borders)) {
                QtPrivate::IndexList start = curData.start;
                start.pop_back();
                start.push_back(resStart);
                QtPrivate::IndexList end = curData.end;
                end.pop_back();
                end.push_back(resEnd);
                curData.start = start;
                curData.end = end;
                curData.roles = roles;
                Q_ASSERT(!start.isEmpty());
                Q_ASSERT(!end.isEmpty());
            } else {
                finalRequests.push_back(curData);
                curData = data;
            }
        }
    }
    finalRequests.push_back(curData);
    //qCDebug(QT_REMOTEOBJECT_MODELS) << "Final requests" << finalRequests;
    int rows = 0;
                                                                        // There is no point to eat more than can chew
    for (auto it = finalRequests.rbegin(); it != finalRequests.rend() && size_t(rows) < m_rootItem.children.cacheSize; ++it) {
        qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO << "FINAL start=" << it->start << "end=" << it->end << "roles=" << it->roles;

        QRemoteObjectPendingReply<QtPrivate::DataEntries> reply = replicaRowRequest(it->start, it->end, it->roles);
        RowWatcher *watcher = new RowWatcher(it->start, it->end, it->roles, reply);
        rows += 1 + it->end.first().row - it->start.first().row;
        m_pendingRequests.push_back(watcher);
        connect(watcher, &RowWatcher::finished, this, &QAbstractItemModelReplicaImplementation::requestedData);
    }
}

void QAbstractItemModelReplicaImplementation::onModelReset()
{
    if (!m_initDone)
        return;

    qCDebug(QT_REMOTEOBJECT_MODELS) << Q_FUNC_INFO;
    QRemoteObjectPendingCallWatcher *watcher = doModelReset();
    connect(watcher, &QRemoteObjectPendingCallWatcher::finished, this, &QAbstractItemModelReplicaImplementation::handleModelResetDone);
}

void QAbstractItemModelReplicaImplementation::onHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    // TODO clean cache
    const int index = orientation == Qt::Horizontal ? 0 : 1;
    QList<CacheEntry> &entries = m_headerData[index];
    for (int i = first; i <= last && i < entries.size(); ++i )
        entries[i].data.clear();
    emit q->headerDataChanged(orientation, first, last);
}

void QAbstractItemModelReplicaImplementation::fetchPendingHeaderData()
{
    QList<int> roles;
    QList<int> sections;
    QList<Qt::Orientation> orientations;
    for (const RequestedHeaderData &data : std::as_const(m_requestedHeaderData)) {
        roles.push_back(data.role);
        sections.push_back(data.section);
        orientations.push_back(data.orientation);
    }
    QRemoteObjectPendingReply<QVariantList> reply = replicaHeaderRequest(orientations, sections, roles);
    HeaderWatcher *watcher = new HeaderWatcher(orientations, sections, roles, reply);
    connect(watcher, &HeaderWatcher::finished, this, &QAbstractItemModelReplicaImplementation::requestedHeaderData);
    m_requestedHeaderData.clear();
    m_pendingRequests.push_back(watcher);
}

void QAbstractItemModelReplicaImplementation::onLayoutChanged(const QtPrivate::IndexList &parents,
                                                              QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> indexes;
    for (const QtPrivate::ModelIndex &parent : std::as_const(parents)) {
        const QModelIndex parentIndex = toQModelIndex(QtPrivate::IndexList{parent}, q);
        indexes << QPersistentModelIndex(parentIndex);
    }
    QRemoteObjectPendingCallWatcher *watcher;
    auto call = replicaCacheRequest(m_rootItem.children.cacheSize, m_initialFetchRolesHint);
    watcher = new QRemoteObjectPendingCallWatcher(call);
    m_pendingRequests.push_back(watcher);
    connect(watcher, &QRemoteObjectPendingCallWatcher::finished, this, [this, watcher, indexes, hint]() {
        Q_ASSERT(watcher->returnValue().canConvert<QtPrivate::MetaAndDataEntries>());
        const QSize size = watcher->returnValue().value<QtPrivate::MetaAndDataEntries>().size;

        q->layoutAboutToBeChanged(indexes, hint);
        m_rootItem.clear();
        if (size.height() > 0) {
            m_rootItem.rowCount = size.height();
            m_rootItem.hasChildren = true;
        }

        m_rootItem.columnCount = size.width();
        if (m_initialAction == QtRemoteObjects::PrefetchData) {
            auto entries = watcher->returnValue().value<QtPrivate::MetaAndDataEntries>();
            for (int i = 0; i < entries.data.size(); ++i)
                fillCache(entries.data[i], entries.roles);
        }
        m_pendingRequests.removeAll(watcher);
        watcher->deleteLater();
        emit q->layoutChanged(indexes, hint);
    });
}

static inline QList<QPair<int, int>> listRanges(const QList<int> &list)
{
    QList<QPair<int, int>> result;
    if (!list.isEmpty()) {
        QPair<int, int> currentElem = qMakePair(list.first(), list.first());
        const auto end = list.constEnd();
        for (auto it = list.constBegin() + 1; it != end; ++it) {
            if (currentElem.first == *it + 1)
                currentElem.first = *it;
            else if ( currentElem.second == *it -1)
                currentElem.second = *it;
            else if (currentElem.first <= *it && currentElem.second >= *it)
                continue;
            else {
                result.push_back(currentElem);
                currentElem.first = *it;
                currentElem.second = *it;
            }
        }
        result.push_back(currentElem);
    }
    return result;
}

void QAbstractItemModelReplicaImplementation::requestedHeaderData(QRemoteObjectPendingCallWatcher *qobject)
{
    HeaderWatcher *watcher = static_cast<HeaderWatcher *>(qobject);
    Q_ASSERT(watcher);

    QVariantList data = watcher->returnValue().value<QVariantList>();
    Q_ASSERT(watcher->orientations.size() == data.size());
    Q_ASSERT(watcher->sections.size() == data.size());
    Q_ASSERT(watcher->roles.size() == data.size());
    QList<int> horizontalSections;
    QList<int> verticalSections;

    for (int i = 0; i < data.size(); ++i) {
        if (watcher->orientations[i] == Qt::Horizontal)
            horizontalSections.append(watcher->sections[i]);
        else
            verticalSections.append(watcher->sections[i]);
        const int index = watcher->orientations[i] == Qt::Horizontal ? 0 : 1;
        const int role = watcher->roles[i];
        QHash<int, QVariant> &dat = m_headerData[index][watcher->sections[i]].data;
        dat[role] = data[i];
    }
    QList<QPair<int, int>> horRanges = listRanges(horizontalSections);
    QList<QPair<int, int>> verRanges = listRanges(verticalSections);

    for (int i = 0; i < horRanges.size(); ++i)
        emit q->headerDataChanged(Qt::Horizontal, horRanges[i].first, horRanges[i].second);
    for (int i = 0; i < verRanges.size(); ++i)
        emit q->headerDataChanged(Qt::Vertical, verRanges[i].first, verRanges[i].second);
    m_pendingRequests.removeAll(watcher);
    delete watcher;
}

/*!
    \class QAbstractItemModelReplica
    \inmodule QtRemoteObjects
    \brief The QAbstractItemModelReplica class serves as a convenience class for
    Replicas of Sources based on QAbstractItemModel.

    QAbstractItemModelReplica makes replicating QAbstractItemModels more
    efficient by employing caching and pre-fetching.

    \sa QAbstractItemModel
*/

/*!
    \internal
*/
QAbstractItemModelReplica::QAbstractItemModelReplica(QAbstractItemModelReplicaImplementation *rep, QtRemoteObjects::InitialAction action, const QList<int> &rolesHint)
    : QAbstractItemModel()
    , d(rep)
{
    d->m_initialAction = action;
    d->m_initialFetchRolesHint = rolesHint;

    rep->setModel(this);
    connect(rep, &QAbstractItemModelReplicaImplementation::initialized, d.data(), &QAbstractItemModelReplicaImplementation::init);
}

/*!
    Destroys the instance of QAbstractItemModelReplica.
*/
QAbstractItemModelReplica::~QAbstractItemModelReplica()
{
}

static QVariant findData(const CachedRowEntry &row, const QModelIndex &index, int role, bool *cached = nullptr)
{
    if (index.column() < row.size()) {
        const CacheEntry &entry = row[index.column()];
        QHash<int, QVariant>::ConstIterator it = entry.data.constFind(role);
        if (it != entry.data.constEnd()) {
            if (cached)
                *cached = true;
            return it.value();
        }
    }
    if (cached)
        *cached = false;
    return QVariant();
}

/*!
    Returns a pointer to the QItemSelectionModel for the current
    QAbstractItemModelReplica.
*/
QItemSelectionModel* QAbstractItemModelReplica::selectionModel() const
{
    return d->m_selectionModel.data();
}

/*!
    \reimp
*/
bool QAbstractItemModelReplica::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::UserRole - 1) {
        auto parent = d->cacheData(index);
        if (!parent)
            return false;
        bool ok = true;
        auto row = value.toInt(&ok);
        if (ok)
            parent->ensureChildren(row, row);
        return ok;
    }
    if (!index.isValid())
        return false;
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return false;
    if (index.column() < 0 || index.column() >= columnCount(index.parent()))
        return false;

    const QList<int> &availRoles = availableRoles();
    const auto res = std::find(availRoles.begin(), availRoles.end(), role);
    if (res == availRoles.end()) {
        qCWarning(QT_REMOTEOBJECT_MODELS) << "Tried to setData for index" << index << "on a not supported role" << role;
        return false;
    }
    // sendInvocationRequest to change server side data;
    d->replicaSetData(QtPrivate::toModelIndexList(index, this), value, role);
    return true;
}

/*!
    Returns the \a role data for the item at \a index if available in cache.
    A default-constructed QVariant is returned if the index is invalid, the role
    is not one of the available roles, the \l {Replica} is uninitialized or
    the data was not available.
    If the data was not available in cache it will be requested from the
    \l {Source}.

    \sa QAbstractItemModel::data(), hasData(), setData(), isInitialized()
*/
QVariant QAbstractItemModelReplica::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QModelRoleData roleData(role);
    multiData(index, roleData);
    return roleData.data();
}

/*!
  \reimp
*/
void QAbstractItemModelReplica::multiData(const QModelIndex &index,
                                          QModelRoleDataSpan roleDataSpan) const
{
    Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));

    if (!d->isInitialized()) {
        qCDebug(QT_REMOTEOBJECT_MODELS)<<"Data not initialized yet";

        for (auto &roleData : roleDataSpan)
            roleData.clearData();
        return;
    }

    QList<int> rolesToFetch;
    const auto roles = availableRoles();
    if (auto item = d->cacheData(index); item) {
        // If the index is found in cache, try to find the data for each role
        for (auto &roleData : roleDataSpan) {
            const auto role = roleData.role();
            if (roles.contains(role)) {
                bool cached = false;
                QVariant result = findData(item->cachedRowEntry, index, role, &cached);
                if (cached) {
                    roleData.setData(std::move(result));
                } else {
                    roleData.clearData();
                    rolesToFetch.push_back(role);
                }
            } else {
                roleData.clearData();
            }
        }
    } else {
        // If the index is not found in cache, schedule all roles for fetching
        for (auto &roleData : roleDataSpan) {
            const auto role = roleData.role();
            if (roles.contains(role))
                rolesToFetch.push_back(role);
            roleData.clearData();
        }
    }

    if (rolesToFetch.empty())
        return;

    auto parentItem = d->cacheData(index.parent());
    Q_ASSERT(parentItem);
    Q_ASSERT(index.row() < parentItem->rowCount);
    const int row = index.row();
    QtPrivate::IndexList parentList = QtPrivate::toModelIndexList(index.parent(), this);
    QtPrivate::IndexList start = QtPrivate::IndexList() << parentList << QtPrivate::ModelIndex(row, 0);
    QtPrivate::IndexList end = QtPrivate::IndexList() << parentList << QtPrivate::ModelIndex(row, std::max(0, parentItem->columnCount - 1));
    Q_ASSERT(toQModelIndex(start, this).isValid());

    RequestedData data;
    data.start = start;
    data.end = end;
    data.roles = rolesToFetch;
    d->m_requestedData.push_back(data);
    qCDebug(QT_REMOTEOBJECT_MODELS) << "FETCH PENDING DATA" << start << end << rolesToFetch;
    QMetaObject::invokeMethod(d.data(), "fetchPendingData", Qt::QueuedConnection);
}

/*!
    \reimp
*/
QModelIndex QAbstractItemModelReplica::parent(const QModelIndex &index) const
{
    if (!index.isValid() || !index.internalPointer())
        return QModelIndex();
    auto parent = static_cast<CacheData*>(index.internalPointer());
    Q_ASSERT(parent);
    if (parent == &d->m_rootItem)
        return QModelIndex();
    if (d->m_activeParents.find(parent) == d->m_activeParents.end() || d->m_activeParents.find(parent->parent) == d->m_activeParents.end())
        return QModelIndex();
    int row = parent->parent->children.find(parent);
    Q_ASSERT(row >= 0);
    return createIndex(row, 0, parent->parent);
}

/*!
    \reimp
*/
QModelIndex QAbstractItemModelReplica::index(int row, int column, const QModelIndex &parent) const
{
    auto parentItem = d->cacheData(parent);
    if (!parentItem)
        return QModelIndex();

    Q_ASSERT_X((parent.isValid() && parentItem && parentItem != &d->m_rootItem) || (!parent.isValid() && parentItem == &d->m_rootItem), __FUNCTION__, qPrintable(QString(QLatin1String("isValid=%1 equals=%2")).arg(parent.isValid()).arg(parentItem == &d->m_rootItem)));

    // hmpf, following works around a Q_ASSERT-bug in QAbstractItemView::setModel which does just call
    // d->model->index(0,0) without checking the range before-hand what triggers our assert in case the
    // model is empty when view::setModel is called :-/ So, work around with the following;
    if (row < 0 || row >= parentItem->rowCount)
        return QModelIndex();

    if (column < 0 || column >= parentItem->columnCount)
        return QModelIndex();

    if (parentItem != &d->m_rootItem)
        parentItem->ensureChildren(row, row);
    return createIndex(row, column, reinterpret_cast<void*>(parentItem));
}

/*!
    \reimp
*/
bool QAbstractItemModelReplica::hasChildren(const QModelIndex &parent) const
{
    auto parentItem = d->cacheData(parent);
    if (parent.isValid() && parent.column() != 0)
        return false;
    else
        return parentItem ? parentItem->hasChildren : false;
}

/*!
    \reimp
*/
int QAbstractItemModelReplica::rowCount(const QModelIndex &parent) const
{
    auto parentItem = d->cacheData(parent);
    const bool canHaveChildren = parentItem && parentItem->hasChildren && !parentItem->rowCount && parent.column() == 0;
    if (canHaveChildren) {
        QtPrivate::IndexList parentList = QtPrivate::toModelIndexList(parent, this);
        QRemoteObjectPendingReply<QSize> reply = d->replicaSizeRequest(parentList);
        SizeWatcher *watcher = new SizeWatcher(parentList, reply);
        connect(watcher, &SizeWatcher::finished, d.data(), &QAbstractItemModelReplicaImplementation::handleSizeDone);
    } else if (parent.column() > 0) {
        return 0;
    }

    return parentItem ? parentItem->rowCount : 0;
}

/*!
    \reimp
*/
int QAbstractItemModelReplica::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;
    auto parentItem = d->cacheData(parent);
    if (!parentItem)
        return 0;
    while (parentItem->columnCount < 0 && parentItem->parent)
        parentItem = parentItem->parent;
    return std::max(0, parentItem->columnCount);
}

/*!
    Returns the data for the given \a role and \a section in the header with the
    specified \a orientation.

    If the data is not available it will be requested from the \l {Source}.

    \sa QAbstractItemModel::headerData()
*/
QVariant QAbstractItemModelReplica::headerData(int section, Qt::Orientation orientation, int role) const
{
    const int index = orientation == Qt::Horizontal ? 0 : 1;
    const QList<CacheEntry> elem = d->m_headerData[index];
    if (section >= elem.size())
        return QVariant();

    const QHash<int, QVariant> &dat = elem.at(section).data;
    QHash<int, QVariant>::ConstIterator it = dat.constFind(role);
    if (it != dat.constEnd())
        return it.value();

    RequestedHeaderData data;
    data.role = role;
    data.section = section;
    data.orientation = orientation;
    d->m_requestedHeaderData.push_back(data);
    QMetaObject::invokeMethod(d.data(), "fetchPendingHeaderData", Qt::QueuedConnection);
    return QVariant();
}

/*!
    \reimp
*/
Qt::ItemFlags QAbstractItemModelReplica::flags(const QModelIndex &index) const
{
    CacheEntry *entry = d->cacheEntry(index);
    return entry ? entry->flags : Qt::NoItemFlags;
}

/*!
    \fn void QAbstractItemModelReplica::initialized()

    The initialized signal is emitted the first time we receive data
    from the \l {Source}.

    \sa isInitialized()
*/

/*!
    Returns \c true if this replica has been initialized with data from the
    \l {Source} object. Returns \c false otherwise.

    \sa initialized()
*/
bool QAbstractItemModelReplica::isInitialized() const
{
    return d->isInitialized();
}

/*!
    Returns \c true if there exists \a role data for the item at \a index.
    Returns \c false in any other case.
*/
bool QAbstractItemModelReplica::hasData(const QModelIndex &index, int role) const
{
    if (!d->isInitialized() || !index.isValid())
        return false;
    auto item = d->cacheData(index);
    if (!item)
        return false;
    bool cached = false;
    const CachedRowEntry &entry = item->cachedRowEntry;
    QVariant result = findData(entry, index, role, &cached);
    Q_UNUSED(result)
    return cached;
}

/*!
    Returns the current size of the internal cache.
    By default this is set to the value of the \c QTRO_NODES_CACHE_SIZE
    environment variable, or a default of \c 1000 if it is invalid or doesn't
    exist.

    \sa setRootCacheSize()
*/
size_t QAbstractItemModelReplica::rootCacheSize() const
{
    return d->m_rootItem.children.cacheSize;
}

/*!
    Sets the size of the internal cache to \a rootCacheSize.

    \sa rootCacheSize()
*/
void QAbstractItemModelReplica::setRootCacheSize(size_t rootCacheSize)
{
    d->m_rootItem.children.setCacheSize(rootCacheSize);
}

/*!
    Returns a list of available roles.

    \sa QAbstractItemModel
*/
QList<int> QAbstractItemModelReplica::availableRoles() const
{
    return d->availableRoles();
}

/*!
    \reimp
*/
QHash<int, QByteArray> QAbstractItemModelReplica::roleNames() const
{
    return d->roleNames();
}

QT_END_NAMESPACE
