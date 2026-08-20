// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTS_ABSTRACT_ITEM_REPLICA_P_H
#define QREMOTEOBJECTS_ABSTRACT_ITEM_REPLICA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qremoteobjectabstractitemmodeltypes_p.h"
#include "qremoteobjectabstractitemmodelreplica.h"
#include "qremoteobjectreplica.h"
#include "qremoteobjectpendingcall.h"
#include <list>
#include <unordered_map>
#include <unordered_set>

QT_BEGIN_NAMESPACE

namespace {
    const int DefaultNodesCacheSize = 1000;
}

struct CacheEntry
{
    QHash<int, QVariant> data;
    Qt::ItemFlags flags;

    explicit CacheEntry()
        : flags(Qt::NoItemFlags)
    {}
};

using CachedRowEntry = QList<CacheEntry>;

template <class Key, class Value>
struct LRUCache
{
    typedef std::pair<Key, Value*> Pair;
    std::list<Pair> cachedItems;
    typedef typename std::list<Pair>::iterator CacheIterator;
    std::unordered_map<Key, CacheIterator> cachedItemsMap;
    size_t cacheSize;

    explicit LRUCache()
    {
        bool ok;
        cacheSize = qEnvironmentVariableIntValue("QTRO_NODES_CACHE_SIZE" , &ok);
        if (!ok)
            cacheSize = DefaultNodesCacheSize;
    }

    ~LRUCache()
    {
        clear();
    }

    inline void cleanCache()
    {
        Q_ASSERT(cachedItems.size() == cachedItemsMap.size());

        auto it = cachedItems.rbegin();
        while (cachedItemsMap.size() > cacheSize) {
            // Do not trash elements with children
            // Workaround QTreeView bugs which caches the children indexes for very long time
            while (it->second->hasChildren && it != cachedItems.rend())
                ++it;

            if (it == cachedItems.rend())
                break;

            cachedItemsMap.erase(it->first);
            delete it->second;
            cachedItems.erase((++it).base());
        }
        Q_ASSERT(cachedItems.size() == cachedItemsMap.size());
    }

    void setCacheSize(size_t rootCacheSize)
    {
        cacheSize = rootCacheSize;
        cleanCache();
        cachedItemsMap.reserve(rootCacheSize);
    }

    void changeKeys(Key key, Key delta) {
        std::vector<std::pair<Key, CacheIterator>> changed;
        auto it = cachedItemsMap.begin();
        while (it != cachedItemsMap.end()) {
            if (it->first >= key) {
                changed.emplace_back(it->first + delta, it->second);
                it->second->first += delta;
                it = cachedItemsMap.erase(it);
            } else {
                ++it;
            }
        }
        for (auto pair : changed)
            cachedItemsMap[pair.first] = pair.second;
    }

    void insert(Key key, Value *value)
    {
        changeKeys(key, 1);
        ensure(key, value);
        Q_ASSERT(cachedItems.size() == cachedItemsMap.size());
    }

    void ensure(Key key, Value *value)
    {
        cachedItems.emplace_front(key, value);
        cachedItemsMap[key] = cachedItems.begin();
        cleanCache();
    }

    void remove(Key key)
    {
        auto it = cachedItemsMap.find(key);
        if (it != cachedItemsMap.end()) {
            delete it->second->second;
            cachedItems.erase(it->second);
            cachedItemsMap.erase(it);
        }
        changeKeys(key, -1);
        Q_ASSERT(cachedItems.size() == cachedItemsMap.size());
    }

    Value *get(Key key)
    {
        auto it = cachedItemsMap.find(key);
        if (it == cachedItemsMap.end())
            return nullptr;

        // Move the accessed item to front
        cachedItems.splice(cachedItems.begin(), cachedItems, it->second);
        Q_ASSERT(it->second->first == key);
        return it->second->second;
    }

    Key find(Value *val)
    {
        for (auto it = cachedItemsMap.begin(); it != cachedItemsMap.end(); ++it) {
            if (it->second->second == val)
                return it->first;
        }
        Q_ASSERT_X(false, __FUNCTION__, "Value not found");
        return Key{};
    }

    bool exists(Value *val)
    {
        for (const auto &pair : cachedItems)
            if (pair.second == val)
                return true;

        return false;
    }

    bool exists(Key key)
    {
        return cachedItemsMap.find(key) != cachedItemsMap.end();
    }

    size_t size()
    {
        return cachedItemsMap.size();
    }

    void clear()
    {
        for (const auto &pair : cachedItems)
            delete pair.second;
        cachedItems.clear();
        cachedItemsMap.clear();
    }
};

class QAbstractItemModelReplicaImplementation;
struct CacheData
{
    QAbstractItemModelReplicaImplementation *replicaModel;
    CacheData *parent;
    CachedRowEntry cachedRowEntry;

    bool hasChildren;
    LRUCache<int, CacheData> children;
    int columnCount;
    int rowCount;

    explicit CacheData(QAbstractItemModelReplicaImplementation *model, CacheData *parentItem = nullptr);

    ~CacheData();

    void ensureChildren(int start, int end)
    {
        for (int i = start; i <= end; ++i)
            if (!children.exists(i))
                children.ensure(i, new CacheData(replicaModel, this));
    }

    void insertChildren(int start, int end) {
        Q_ASSERT_X(start >= 0 && start <= end, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 <= %2")).arg(start).arg(end)));
        for (int i = start; i <= end; ++i) {
            auto cacheData = new CacheData(replicaModel, this);
            cacheData->columnCount = columnCount;
            children.insert(i, cacheData);
            ++rowCount;
        }
        if (rowCount)
            hasChildren = true;
    }
    void removeChildren(int start, int end) {
        Q_ASSERT_X(start >= 0 && start <= end && end < rowCount, __FUNCTION__, qPrintable(QString(QLatin1String("0 <= %1 <= %2 < %3")).arg(start).arg(end).arg(rowCount)));
        for (int i = end; i >= start; --i) {
            children.remove(i);
            --rowCount;
        }
        hasChildren = rowCount;
    }
    void clear() {
        cachedRowEntry.clear();
        children.clear();
        hasChildren = false;
        columnCount = 0;
        rowCount = 0;
    }
};

struct RequestedData
{
    QtPrivate::IndexList start;
    QtPrivate::IndexList end;
    QList<int> roles;
};

struct RequestedHeaderData
{
    int role;
    int section;
    Qt::Orientation orientation;
};

class SizeWatcher : public QRemoteObjectPendingCallWatcher
{
    Q_OBJECT
public:
    SizeWatcher(QtPrivate::IndexList _parentList, const QRemoteObjectPendingReply<QSize> &reply)
        : QRemoteObjectPendingCallWatcher(reply),
          parentList(_parentList) {}
    QtPrivate::IndexList parentList;
};

class RowWatcher : public QRemoteObjectPendingCallWatcher
{
    Q_OBJECT
public:
    RowWatcher(QtPrivate::IndexList _start, QtPrivate::IndexList _end, QList<int> _roles, const QRemoteObjectPendingReply<QtPrivate::DataEntries> &reply)
        : QRemoteObjectPendingCallWatcher(reply),
          start(_start),
          end(_end),
          roles(_roles) {}
    QtPrivate::IndexList start, end;
    QList<int> roles;
};

class HeaderWatcher : public QRemoteObjectPendingCallWatcher
{
    Q_OBJECT
public:
    HeaderWatcher(QList<Qt::Orientation> _orientations, QList<int> _sections, QList<int> _roles, const QRemoteObjectPendingReply<QVariantList> &reply)
        : QRemoteObjectPendingCallWatcher(reply),
          orientations(_orientations),
          sections(_sections),
          roles(_roles) {}
    QList<Qt::Orientation> orientations;
    QList<int> sections, roles;
};

class QAbstractItemModelReplicaImplementation : public QRemoteObjectReplica
{
    Q_OBJECT
    //TODO Use an input name for the model on the Replica side
    Q_CLASSINFO(QCLASSINFO_REMOTEOBJECT_TYPE, "ServerModelAdapter")
    Q_PROPERTY(QList<int> availableRoles READ availableRoles NOTIFY availableRolesChanged)
    Q_PROPERTY(QIntHash roleNames READ roleNames)
public:
    QAbstractItemModelReplicaImplementation();
    QAbstractItemModelReplicaImplementation(QRemoteObjectNode *node, const QString &name);
    ~QAbstractItemModelReplicaImplementation() override;
    void initialize() override;
    static void registerMetatypes();

    inline const QList<int> &availableRoles() const
    {
        if (m_availableRoles.isEmpty())
            m_availableRoles = propAsVariant(0).value<QList<int>>();
        return m_availableRoles;
    }

    QHash<int, QByteArray> roleNames() const
    {
       QIntHash roles = propAsVariant(1).value<QIntHash>();
       return roles;
    }

    void setModel(QAbstractItemModelReplica *model);
    bool clearCache(const QtPrivate::IndexList &start, const QtPrivate::IndexList &end, const QList<int> &roles);

Q_SIGNALS:
    void availableRolesChanged();
    void dataChanged(QtPrivate::IndexList topLeft, QtPrivate::IndexList bottomRight, QList<int> roles);
    void rowsInserted(QtPrivate::IndexList parent, int first, int last);
    void rowsRemoved(QtPrivate::IndexList parent, int first, int last);
    void rowsMoved(QtPrivate::IndexList parent, int start, int end, QtPrivate::IndexList destination, int row);
    void currentChanged(QtPrivate::IndexList current, QtPrivate::IndexList previous);
    void modelReset();
    void headerDataChanged(Qt::Orientation,int,int);
    void columnsInserted(QtPrivate::IndexList parent, int first, int last);
    void layoutChanged(QtPrivate::IndexList parents, QAbstractItemModel::LayoutChangeHint hint);
public Q_SLOTS:
    QRemoteObjectPendingReply<QSize> replicaSizeRequest(QtPrivate::IndexList parentList)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaSizeRequest(QtPrivate::IndexList)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(parentList);
        return QRemoteObjectPendingReply<QSize>(sendWithReply(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args));
    }
    QRemoteObjectPendingReply<QtPrivate::DataEntries> replicaRowRequest(QtPrivate::IndexList start, QtPrivate::IndexList end, QList<int> roles)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaRowRequest(QtPrivate::IndexList,QtPrivate::IndexList,QList<int>)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(start) << QVariant::fromValue(end) << QVariant::fromValue(roles);
        return QRemoteObjectPendingReply<QtPrivate::DataEntries>(sendWithReply(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args));
    }
    QRemoteObjectPendingReply<QVariantList> replicaHeaderRequest(QList<Qt::Orientation> orientations, QList<int> sections, QList<int> roles)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaHeaderRequest(QList<Qt::Orientation>,QList<int>,QList<int>)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(orientations) << QVariant::fromValue(sections) << QVariant::fromValue(roles);
        return QRemoteObjectPendingReply<QVariantList>(sendWithReply(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args));
    }
    void replicaSetCurrentIndex(QtPrivate::IndexList index, QItemSelectionModel::SelectionFlags command)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaSetCurrentIndex(QtPrivate::IndexList,QItemSelectionModel::SelectionFlags)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(index) << QVariant::fromValue(command);
        send(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args);
    }
    void replicaSetData(QtPrivate::IndexList index, const QVariant &value, int role)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaSetData(QtPrivate::IndexList,QVariant,int)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(index) << QVariant::fromValue(value) << QVariant::fromValue(role);
        send(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args);
    }
    QRemoteObjectPendingReply<QtPrivate::MetaAndDataEntries> replicaCacheRequest(size_t size, QList<int> roles)
    {
        static int __repc_index = QAbstractItemModelReplicaImplementation::staticMetaObject.indexOfSlot("replicaCacheRequest(size_t,QList<int>)");
        QVariantList __repc_args;
        __repc_args << QVariant::fromValue(size) << QVariant::fromValue(roles);
        return QRemoteObjectPendingReply<QtPrivate::MetaAndDataEntries>(sendWithReply(QMetaObject::InvokeMetaMethod, __repc_index, __repc_args));
    }
    void onHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void onDataChanged(const QtPrivate::IndexList &start, const QtPrivate::IndexList &end, const QList<int> &roles);
    void onRowsInserted(const QtPrivate::IndexList &parent, int start, int end);
    void onRowsRemoved(const QtPrivate::IndexList &parent, int start, int end);
    void onColumnsInserted(const QtPrivate::IndexList &parent, int start, int end);
    void onRowsMoved(QtPrivate::IndexList srcParent, int srcRow, int count, QtPrivate::IndexList destParent, int destRow);
    void onCurrentChanged(QtPrivate::IndexList current, QtPrivate::IndexList previous);
    void onModelReset();
    void requestedData(QRemoteObjectPendingCallWatcher *);
    void requestedHeaderData(QRemoteObjectPendingCallWatcher *);
    void init();
    void fetchPendingData();
    void fetchPendingHeaderData();
    void handleInitDone(QRemoteObjectPendingCallWatcher *watcher);
    void handleModelResetDone(QRemoteObjectPendingCallWatcher *watcher);
    void handleSizeDone(QRemoteObjectPendingCallWatcher *watcher);
    void onReplicaCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void fillCache(const QtPrivate::IndexValuePair &pair,const QList<int> &roles);
    void onLayoutChanged(const QtPrivate::IndexList &parents, QAbstractItemModel::LayoutChangeHint hint);
public:
    QScopedPointer<QItemSelectionModel> m_selectionModel;
    QList<CacheEntry> m_headerData[2];

    CacheData m_rootItem;
    inline CacheData* cacheData(const QModelIndex &index) const {
        if (!index.isValid())
            return const_cast<CacheData*>(&m_rootItem);
        if (index.internalPointer()) {
            auto parent = static_cast<CacheData*>(index.internalPointer());
            if (m_activeParents.find(parent) != m_activeParents.end())
                return parent->children.get(index.row());
        }
        return nullptr;
    }
    inline CacheData* cacheData(const QtPrivate::IndexList &index) const {
        return cacheData(toQModelIndex(index, q));
    }
    inline CacheData* createCacheData(const QtPrivate::IndexList &index) const {
        bool ok = false;
        auto modelIndex = toQModelIndex(index, q, &ok);
        if (!ok)
            return nullptr;
        cacheData(modelIndex.parent())->ensureChildren(modelIndex.row() , modelIndex.row());
        return cacheData(modelIndex);
    }
    inline CacheEntry* cacheEntry(const QModelIndex &index) const {
        auto data = cacheData(index);
        if (!data || index.column() < 0 || index.column() >= data->cachedRowEntry.size())
            return nullptr;
        CacheEntry &entry = data->cachedRowEntry[index.column()];
        return &entry;
    }
    inline CacheEntry* cacheEntry(const QtPrivate::IndexList &index) const {
        return cacheEntry(toQModelIndex(index, q));
    }

    QRemoteObjectPendingCallWatcher *doModelReset();
    void initializeModelConnections();

    bool m_initDone = false;
    QList<RequestedData> m_requestedData;
    QList<RequestedHeaderData> m_requestedHeaderData;
    QList<QRemoteObjectPendingCallWatcher*> m_pendingRequests;
    QAbstractItemModelReplica *q;
    mutable QList<int> m_availableRoles;
    std::unordered_set<CacheData*> m_activeParents;
    QtRemoteObjects::InitialAction m_initialAction;
    QList<int> m_initialFetchRolesHint;
};

QT_END_NAMESPACE

#endif // QREMOTEOBJECTS_ABSTRACT_ITEM_REPLICA_P_H
