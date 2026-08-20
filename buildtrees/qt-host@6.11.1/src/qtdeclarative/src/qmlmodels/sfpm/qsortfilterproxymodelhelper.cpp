// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQmlModels/private/qsortfilterproxymodelhelper_p.h>
#include <QtQmlModels/private/qqmlsortfilterproxymodel_p.h>

QT_BEGIN_NAMESPACE

using IndexMap = QSortFilterProxyModelHelper::IndexMap;


QSortFilterProxyModelHelper::QSortFilterProxyModelHelper()
{

}

QSortFilterProxyModelHelper::~QSortFilterProxyModelHelper()
{
    clearSourceIndexMapping();
}

template<typename F>
void forEachProperty(
        const QMetaObject *metaObject, const QModelIndex &sourceIndex,
        const QQmlSortFilterProxyModel *proxyModel, F &&handler)
{
    for (int i = 0, end = metaObject->propertyCount(); i != end; ++i) {
        const QMetaProperty property = metaObject->property(i);
        const int roleId = proxyModel->itemRoleForName(QString::fromUtf8(property.name()));
        if (roleId < 0)
            continue;

        handler(property, proxyModel->sourceModel()->data(sourceIndex, roleId));
    }
}

void QSortFilterProxyModelHelper::setProperties(
        QVariant *target, const QQmlSortFilterProxyModel *proxyModel,
        const QModelIndex &sourceIndex)
{
    const QMetaType metaType = target->metaType();

    if (metaType.flags() & QMetaType::PointerToQObject) {
        QObject *object = target->value<QObject *>();
        forEachProperty(
                object->metaObject(), sourceIndex, proxyModel,
                [&](const QMetaProperty &property, const QVariant &value) {
                    property.write(object, value);
                });
        return;
    }

    const QMetaObject *metaObject = QQmlMetaType::metaObjectForValueType(metaType);
    if (!metaObject)
        return;

    forEachProperty(
            metaObject, sourceIndex, proxyModel,
            [&](const QMetaProperty &property, const QVariant &value) {
                property.writeOnGadget(target->data(), value);
            });
}

void QSortFilterProxyModelHelper::clearSourceIndexMapping()
{
    qDeleteAll(source_index_mapping);
    source_index_mapping.clear();
}

IndexMap::const_iterator QSortFilterProxyModelHelper::create_mapping(
        const QModelIndex &source_parent) const
{
    IndexMap::const_iterator it = source_index_mapping.constFind(source_parent);
    if (it != source_index_mapping.constEnd()) // was mapped already
        return it;

    auto *model = proxyModel()->sourceModel();
    Q_ASSERT(model != nullptr);

    Mapping *m = new Mapping;

    int source_rows = model->rowCount(source_parent);
    m->source_rows.reserve(source_rows);
    for (int i = 0; i < source_rows; ++i) {
        if (filterAcceptsRowInternal(i, source_parent))
            m->source_rows.append(i);
    }
    int source_cols = model->columnCount(source_parent);
    m->source_columns.reserve(source_cols);
    for (int i = 0; i < source_cols; ++i) {
        if (filterAcceptsColumn(i, source_parent))
            m->source_columns.append(i);
    }

    sort_source_rows(m->source_rows, source_parent);
    m->proxy_rows.resize(source_rows);
    build_source_to_proxy_mapping(m->source_rows, m->proxy_rows);
    m->proxy_columns.resize(source_cols);
    build_source_to_proxy_mapping(m->source_columns, m->proxy_columns);

    m->source_parent = source_parent;

    if (source_parent.isValid()) {
        QModelIndex source_grand_parent = source_parent.parent();
        IndexMap::const_iterator it2 = create_mapping(source_grand_parent);
        Q_ASSERT(it2 != source_index_mapping.constEnd());
        it2.value()->mapped_children.append(source_parent);
    }

    it = IndexMap::const_iterator(source_index_mapping.insert(source_parent, m));
    Q_ASSERT(it != source_index_mapping.constEnd());
    Q_ASSERT(it.value());

    return it;
}

QSortFilterProxyModelHelper::IndexMap::const_iterator QSortFilterProxyModelHelper::create_mapping_recursive(
        const QModelIndex &source_parent) const
{
    if (source_parent.isValid()) {
        const QModelIndex source_grand_parent = source_parent.parent();
        IndexMap::const_iterator it = source_index_mapping.constFind(source_grand_parent);
        IndexMap::const_iterator end = source_index_mapping.constEnd();
        if (it == end) {
            it = create_mapping_recursive(source_grand_parent);
            end = source_index_mapping.constEnd();
            if (it == end)
                return end;
        }
        Mapping *gm = it.value();
        if (gm->proxy_rows.at(source_parent.row()) == -1 ||
            gm->proxy_columns.at(source_parent.column()) == -1) {
            // Can't do, parent is filtered
            return end;
        }
    }
    return create_mapping(source_parent);
}

/*!
    \internal

    updates the mapping of the children when inserting or removing items
*/
void QSortFilterProxyModelHelper::updateChildrenMapping(const QModelIndex &source_parent, Mapping *parent_mapping,
                                Direction direction, int start, int end, int delta_item_count, bool remove)
{
    // see if any mapped children should be (re)moved
    QList<std::pair<QModelIndex, Mapping *>> moved_source_index_mappings;
    auto it2 = parent_mapping->mapped_children.begin();
    for ( ; it2 != parent_mapping->mapped_children.end();) {
        const QModelIndex source_child_index = *it2;
        const int pos = (direction == Direction::Rows)
                ? source_child_index.row()
                : source_child_index.column();
        if (pos < start) {
            // not affected
            ++it2;
        } else if (remove && pos <= end) {
            // in the removed interval
            it2 = parent_mapping->mapped_children.erase(it2);
            remove_from_mapping(source_child_index);
        } else {
            // below the removed items -- recompute the index
            QModelIndex new_index;
            auto model = proxyModel()->sourceModel();
            const int newpos = remove ? pos - delta_item_count : pos + delta_item_count;
            if (direction == Direction::Rows) {
                new_index = model->index(newpos,
                                         source_child_index.column(),
                                         source_parent);
            } else {
                new_index = model->index(source_child_index.row(),
                                         newpos,
                                         source_parent);
            }
            *it2 = new_index;
            ++it2;
            // update mapping
            Mapping *cm = source_index_mapping.take(source_child_index);
            Q_ASSERT(cm);
            // we do not reinsert right away, because the new index might be identical with another, old index
            moved_source_index_mappings.emplace_back(new_index, cm);
        }
    }

    // reinsert moved, mapped indexes
    for (auto &pair : std::as_const(moved_source_index_mappings)) {
        pair.second->source_parent = pair.first;
        source_index_mapping.insert(pair.first, pair.second);
    }
}

QModelIndex QSortFilterProxyModelHelper::source_to_proxy(const QModelIndex &source_index) const
{
    if (!source_index.isValid())
        return QModelIndex(); // for now; we may want to be able to set a root index later
    if (source_index.model() != proxyModel()->sourceModel()) {
        qWarning("QSortFilterProxyModel: index from wrong model passed to mapFromSource");
        Q_ASSERT(!"QSortFilterProxyModel: index from wrong model passed to mapFromSource");
        return QModelIndex();
    }
    QModelIndex source_parent = source_index.parent();
    IndexMap::const_iterator it = create_mapping(source_parent);
    Mapping *m = it.value();
    if ((source_index.row() >= m->proxy_rows.size()) || (source_index.column() >= m->proxy_columns.size()))
        return QModelIndex();
    int proxy_row = m->proxy_rows.at(source_index.row());
    int proxy_column = m->proxy_columns.at(source_index.column());
    if (proxy_row == -1 || proxy_column == -1)
        return QModelIndex();
    return createIndex(proxy_row, proxy_column, it);
}

QModelIndex QSortFilterProxyModelHelper::proxy_to_source(const QModelIndex &proxy_index) const
{
    if (!proxy_index.isValid())
        return QModelIndex(); // for now; we may want to be able to set a root index later
    if (proxy_index.model() != proxyModel()) {
        qWarning("QSortFilterProxyModel: index from wrong model passed to mapToSource");
        Q_ASSERT(!"QSortFilterProxyModel: index from wrong model passed to mapToSource");
        return QModelIndex();
    }
    IndexMap::const_iterator it = index_to_iterator(proxy_index);
    Mapping *m = it.value();
    if ((proxy_index.row() >= m->source_rows.size()) || (proxy_index.column() >= m->source_columns.size()))
        return QModelIndex();
    int source_row = m->source_rows.at(proxy_index.row());
    int source_col = m->source_columns.at(proxy_index.column());
    auto *model = proxyModel()->sourceModel();
    return model->index(source_row, source_col, it.key());
}

void QSortFilterProxyModelHelper::build_source_to_proxy_mapping(
        QList<int> &proxy_to_source, QList<int> &source_to_proxy, int start) const
{
    if (start == 0)
        source_to_proxy.fill(-1);
    const int proxy_count = proxy_to_source.size();
    for (int i = start; i < proxy_count; ++i)
        source_to_proxy[proxy_to_source.at(i)] = i;
}

bool QSortFilterProxyModelHelper::can_create_mapping(const QModelIndex &source_parent) const
{
    if (source_parent.isValid()) {
        QModelIndex source_grand_parent = source_parent.parent();
        IndexMap::const_iterator it = source_index_mapping.constFind(source_grand_parent);
        if (it == source_index_mapping.constEnd()) {
            // Don't care, since we don't have mapping for the grand parent
            return false;
        }
        Mapping *gm = it.value();
        if (gm->proxy_rows.at(source_parent.row()) == -1 ||
            gm->proxy_columns.at(source_parent.column()) == -1) {
            // Don't care, since parent is filtered
            return false;
        }
    }
    return true;
}

void QSortFilterProxyModelHelper::remove_from_mapping(const QModelIndex &source_parent)
{
    if (Mapping *m = source_index_mapping.take(source_parent)) {
        for (const QModelIndex &mappedIdx : std::as_const(m->mapped_children))
            remove_from_mapping(mappedIdx);
        delete m;
    }
}

void QSortFilterProxyModelHelper::proxy_item_range(const QList<int> &source_to_proxy,
        const QList<int> &source_items, int &proxy_low, int &proxy_high) const
{
    proxy_low = INT_MAX;
    proxy_high = INT_MIN;
    for (int i = 0; i < source_items.size(); ++i) {
        int proxy_item = source_to_proxy.at(source_items.at(i));
        Q_ASSERT(proxy_item != -1);
        if (proxy_item < proxy_low)
            proxy_low = proxy_item;
        if (proxy_item > proxy_high)
            proxy_high = proxy_item;
    }
}


QModelIndexPairList QSortFilterProxyModelHelper::store_persistent_indexes() const
{
    QModelIndexPairList source_indexes;
    auto *proxyPriv = QAbstractProxyModelPrivate::get(proxyModel());
    source_indexes.reserve(proxyPriv->persistent.indexes.size());
    for (const QPersistentModelIndexData *data : std::as_const(proxyPriv->persistent.indexes)) {
        const QModelIndex &proxy_index = data->index;
        const QModelIndex source_index = proxyModel()->mapToSource(proxy_index);
        source_indexes.emplace_back(proxy_index, source_index);
    }
    return source_indexes;
}

void QSortFilterProxyModelHelper::update_persistent_indexes(const QModelIndexPairList &source_indexes)
{
    QModelIndexList from, to;
    const int numSourceIndexes = source_indexes.size();
    from.reserve(numSourceIndexes);
    to.reserve(numSourceIndexes);
    for (const auto &indexPair : source_indexes) {
        const QPersistentModelIndex &source_index = indexPair.second;
        const QModelIndex &old_proxy_index = indexPair.first;
        create_mapping(source_index.parent());
        const QModelIndex proxy_index = proxyModel()->mapFromSource(source_index);
        from << old_proxy_index;
        to << proxy_index;
    }
    changePersistentIndexList(from, to);
}

/*!
    \internal

    Given source-to-proxy mapping \a source_to_proxy and the set of
    source items \a source_items (which are part of that mapping),
    determines the corresponding proxy item intervals that should
    be removed from the proxy model.

    The result is a vector of pairs, where each pair represents a
    (start, end) tuple, sorted in ascending order.
*/
QList<std::pair<int, int>> QSortFilterProxyModelHelper::proxy_intervals_for_source_items(
        const QList<int> &source_to_proxy, const QList<int> &source_items) const
{
    QList<std::pair<int, int>> proxy_intervals;
    if (source_items.isEmpty())
        return proxy_intervals;

    int source_items_index = 0;
    while (source_items_index < source_items.size()) {
        int first_proxy_item = source_to_proxy.at(source_items.at(source_items_index));
        Q_ASSERT(first_proxy_item != -1);
        int last_proxy_item = first_proxy_item;
        ++source_items_index;
        // Find end of interval
        while ((source_items_index < source_items.size())
               && (source_to_proxy.at(source_items.at(source_items_index)) == last_proxy_item + 1)) {
            ++last_proxy_item;
            ++source_items_index;
        }
        // Add interval to result
        proxy_intervals.emplace_back(first_proxy_item, last_proxy_item);
    }
    std::stable_sort(proxy_intervals.begin(), proxy_intervals.end());
    // Consolidate adjacent intervals
    for (int i = proxy_intervals.size()-1; i > 0; --i) {
        std::pair<int, int> &interval = proxy_intervals[i];
        std::pair<int, int> &preceeding_interval = proxy_intervals[i - 1];
        if (interval.first == preceeding_interval.second + 1) {
            preceeding_interval.second = interval.second;
            interval.first = interval.second = -1;
        }
    }
    proxy_intervals.removeIf([](std::pair<int, int> interval) { return interval.first < 0; });
    return proxy_intervals;
}

/*!
    \internal

    Given source-to-proxy mapping \a source_to_proxy and proxy-to-source mapping
    \a proxy_to_source, removes items from \a proxy_start to \a proxy_end
    (inclusive) from this proxy model.
*/
void QSortFilterProxyModelHelper::remove_proxy_interval(
        QList<int> &source_to_proxy, QList<int> &proxy_to_source, int proxy_start, int proxy_end,
        const QModelIndex &proxy_parent, Direction direction, bool emit_signal)
{
    if (emit_signal) {
        if (direction == Direction::Rows)
            beginRemoveRows(proxy_parent, proxy_start, proxy_end);
        else
            beginRemoveColumns(proxy_parent, proxy_start, proxy_end);
    }
    // Remove items from proxy-to-source mapping
    for (int i = proxy_start; i <= proxy_end; ++i)
        source_to_proxy[proxy_to_source.at(i)] = -1;
    proxy_to_source.remove(proxy_start, proxy_end - proxy_start + 1);
    build_source_to_proxy_mapping(proxy_to_source, source_to_proxy, proxy_start);
    if (emit_signal) {
        if (direction == Direction::Rows)
            endRemoveRows();
        else
            endRemoveColumns();
    }
}

/*!
    \internal

    Handles source model items insertion (columnsInserted(), rowsInserted()).
    Determines
    1) which of the inserted items to also insert into proxy model (filtering),
    2) where to insert the items into the proxy model (sorting), then inserts
    those items.
    The items are inserted into the proxy model in intervals (based on
    sorted order), so that the proper rows/columnsInserted(start, end)
    signals will be generated.
*/
void QSortFilterProxyModelHelper::source_items_inserted(
        const QModelIndex &source_parent, int start, int end, Direction direction)
{
    if ((start < 0) || (end < 0))
        return;
    QSortFilterProxyModelHelper::IndexMap::const_iterator it = source_index_mapping.constFind(source_parent);
    if (it == source_index_mapping.constEnd()) {
        if (!can_create_mapping(source_parent))
            return;
        it = create_mapping(source_parent);
        Mapping *m = it.value();
        QModelIndex proxy_parent = proxyModel()->mapFromSource(source_parent);
        if (m->source_rows.size() > 0) {
            beginInsertRows(proxy_parent, 0, m->source_rows.size() - 1);
            endInsertRows();
        }
        if (m->source_columns.size() > 0) {
            beginInsertColumns(proxy_parent, 0, m->source_columns.size() - 1);
            endInsertColumns();
        }
        return;
    }

    Mapping *m = it.value();
    QList<int> &source_to_proxy = (direction == Direction::Rows) ? m->proxy_rows : m->proxy_columns;
    QList<int> &proxy_to_source = (direction == Direction::Rows) ? m->source_rows : m->source_columns;

    int delta_item_count = end - start + 1;
    int old_item_count = source_to_proxy.size();

    updateChildrenMapping(source_parent, m, direction, start, end, delta_item_count, false);

    // Expand source-to-proxy mapping to account for new items
    if (start < 0 || start > source_to_proxy.size()) {
        qWarning("QSortFilterProxyModel: invalid inserted rows reported by source model");
        remove_from_mapping(source_parent);
        return;
    }
    source_to_proxy.insert(start, delta_item_count, -1);

    if (start < old_item_count) {
        // Adjust existing "stale" indexes in proxy-to-source mapping
        int proxy_count = proxy_to_source.size();
        for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
            int source_item = proxy_to_source.at(proxy_item);
            if (source_item >= start)
                proxy_to_source.replace(proxy_item, source_item + delta_item_count);
        }
        build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);
    }

    // Figure out which items to add to mapping based on filter
    QList<int> source_items;
    for (int i = start; i <= end; ++i) {
        if ((direction == Direction::Rows)
                    ? filterAcceptsRowInternal(i, source_parent)
                    : filterAcceptsColumn(i, source_parent)) {
            source_items.append(i);
        }
    }

    auto model = proxyModel()->sourceModel();
    if (model->rowCount(source_parent) == delta_item_count) {
        // Items were inserted where there were none before.
        // If it was new rows make sure to create mappings for columns so that a
        // valid mapping can be retrieved later and vice-versa.
        QList<int> &orthogonal_proxy_to_source = (direction == Direction::Columns) ? m->source_rows : m->source_columns;
        QList<int> &orthogonal_source_to_proxy = (direction == Direction::Columns) ? m->proxy_rows : m->proxy_columns;
        if (orthogonal_source_to_proxy.isEmpty()) {
            const int ortho_end = (direction == Direction::Columns) ? model->rowCount(source_parent) :
                    model->columnCount(source_parent);
            orthogonal_source_to_proxy.resize(ortho_end);
            for (int ortho_item = 0; ortho_item < ortho_end; ++ortho_item) {
                if ((direction == Direction::Columns) ? filterAcceptsRowInternal(ortho_item, source_parent)
                                                    : filterAcceptsColumn(ortho_item, source_parent)) {
                    orthogonal_proxy_to_source.append(ortho_item);
                }
            }
            if (direction == Direction::Columns) {
                // We're reacting to columnsInserted, but we've just inserted new rows. Sort them.
                sort_source_rows(orthogonal_proxy_to_source, source_parent);
            }
            build_source_to_proxy_mapping(orthogonal_proxy_to_source, orthogonal_source_to_proxy);
        }
    }

    // Sort and insert the items
    if (direction == Direction::Rows) // Only sort rows
        sort_source_rows(source_items, source_parent);
    insert_source_items(source_to_proxy, proxy_to_source, source_items, source_parent, direction);
}

/*!
    \internal

    Handles source model items removal (columnsAboutToBeRemoved(), rowsAboutToBeRemoved()).
*/
void QSortFilterProxyModelHelper::source_items_about_to_be_removed(
        const QModelIndex &source_parent, int start, int end, Direction direction)
{
    if ((start < 0) || (end < 0))
        return;
    QSortFilterProxyModelHelper::IndexMap::const_iterator it = source_index_mapping.constFind(source_parent);
    if (it == source_index_mapping.constEnd()) {
        // Don't care, since we don't have mapping for this index
        return;
    }

    Mapping *m = it.value();
    QList<int> &source_to_proxy = (direction == Direction::Rows) ? m->proxy_rows : m->proxy_columns;
    QList<int> &proxy_to_source = (direction == Direction::Rows) ? m->source_rows : m->source_columns;

    // figure out which items to remove
    QList<int> source_items_to_remove;
    int proxy_count = proxy_to_source.size();
    for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
        int source_item = proxy_to_source.at(proxy_item);
        if ((source_item >= start) && (source_item <= end))
            source_items_to_remove.append(source_item);
    }

    remove_source_items(source_to_proxy, proxy_to_source, source_items_to_remove,
                        source_parent, direction);
}

/*!
    \internal

    Handles source model items removal (columnsRemoved(), rowsRemoved()).
*/
void QSortFilterProxyModelHelper::source_items_removed(
        const QModelIndex &source_parent, int start, int end, Direction direction)
{
    if ((start < 0) || (end < 0))
        return;
    QSortFilterProxyModelHelper::IndexMap::const_iterator it = source_index_mapping.constFind(source_parent);
    if (it == source_index_mapping.constEnd()) {
        // Don't care, since we don't have mapping for this index
        return;
    }

    Mapping *m = it.value();
    QList<int> &source_to_proxy = (direction == Direction::Rows) ? m->proxy_rows : m->proxy_columns;
    QList<int> &proxy_to_source = (direction == Direction::Rows) ? m->source_rows : m->source_columns;

    if (end >= source_to_proxy.size())
        end = source_to_proxy.size() - 1;

    // Shrink the source-to-proxy mapping to reflect the new item count
    int delta_item_count = end - start + 1;
    source_to_proxy.remove(start, delta_item_count);

    int proxy_count = proxy_to_source.size();
    if (proxy_count > source_to_proxy.size()) {
        // mapping is in an inconsistent state -- redo the whole mapping
        beginResetModel();
        remove_from_mapping(source_parent);
        endResetModel();
        return;
    }

    // Adjust "stale" indexes in proxy-to-source mapping
    for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
        int source_item = proxy_to_source.at(proxy_item);
        if (source_item >= start) {
            Q_ASSERT(source_item - delta_item_count >= 0);
            proxy_to_source.replace(proxy_item, source_item - delta_item_count);
        }
    }

    build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);
    updateChildrenMapping(source_parent, m, direction, start, end, delta_item_count, true);
}


/*!
    \internal

    Given source-to-proxy mapping \a source_to_proxy and proxy-to-source mapping
    \a proxy_to_source, inserts the given \a source_items into this proxy model.
    The source items are inserted in intervals (based on some sorted order), so
    that the proper rows/columnsInserted(start, end) signals will be generated.
*/
void QSortFilterProxyModelHelper::insert_source_items(
        QList<int> &source_to_proxy, QList<int> &proxy_to_source,
        const QList<int> &source_items, const QModelIndex &source_parent,
        Direction direction, bool emit_signal)
{
    QModelIndex proxy_parent = proxyModel()->mapFromSource(source_parent);
    if (!proxy_parent.isValid() && source_parent.isValid())
        return; // nothing to do (source_parent is not mapped)

    const auto proxy_intervals = proxy_intervals_for_source_items_to_add(
            proxy_to_source, source_items, source_parent, direction);

    const auto end = proxy_intervals.rend();
    for (auto it = proxy_intervals.rbegin(); it != end; ++it) {
        const std::pair<int, QList<int>> &interval = *it;
        const int proxy_start = interval.first;
        const QList<int> &source_items = interval.second;
        const int proxy_end = proxy_start + source_items.size() - 1;

        if (emit_signal) {
            if (direction == Direction::Rows)
                beginInsertRows(proxy_parent, proxy_start, proxy_end);
            else
                beginInsertColumns(proxy_parent, proxy_start, proxy_end);
        }

        // TODO: use the range QList::insert() overload once it is implemented (QTBUG-58633).
        proxy_to_source.insert(proxy_start, source_items.size(), 0);
        std::copy(source_items.cbegin(), source_items.cend(), proxy_to_source.begin() + proxy_start);
        build_source_to_proxy_mapping(proxy_to_source, source_to_proxy, proxy_start);

        if (emit_signal) {
            if (direction == Direction::Rows)
                endInsertRows();
            else
                endInsertColumns();
        }
    }
}

/*!
    \internal

    Given source-to-proxy mapping \a src_to_proxy and proxy-to-source mapping
    \a proxy_to_source, removes \a source_items from this proxy model.
    The corresponding proxy items are removed in intervals, so that the proper
    rows/columnsRemoved(start, end) signals will be generated.
*/
void QSortFilterProxyModelHelper::remove_source_items(QList<int> &source_to_proxy,
        QList<int> &proxy_to_source, const QList<int> &source_items,
        const QModelIndex &source_parent, Direction direction,
        bool emit_signal)
{
    QModelIndex proxy_parent = proxyModel()->mapFromSource(source_parent);
    if (!proxy_parent.isValid() && source_parent.isValid()) {
        proxy_to_source.clear();
        return; // nothing to do (already removed)
    }
    const auto proxy_intervals = proxy_intervals_for_source_items(
                        source_to_proxy, source_items);
    const auto end = proxy_intervals.rend();
    for (auto it = proxy_intervals.rbegin(); it != end; ++it) {
        const std::pair<int, int> &interval = *it;
        const int proxy_start = interval.first;
        const int proxy_end = interval.second;
        remove_proxy_interval(source_to_proxy, proxy_to_source, proxy_start, proxy_end,
                              proxy_parent, direction, emit_signal);
    }
}

QSet<int> QSortFilterProxyModelHelper::handle_filter_changed(
        QList<int> &source_to_proxy, QList<int> &proxy_to_source,
        const QModelIndex &source_parent, Direction direction)
{
    // Figure out which mapped items to remove
    QList<int> source_items_remove;
    for (int i = 0; i < proxy_to_source.size(); ++i) {
        const int source_item = proxy_to_source.at(i);
        if ((direction == Direction::Rows)
                    ? !filterAcceptsRowInternal(source_item, source_parent)
                    : !filterAcceptsColumn(source_item, source_parent)) {
            // This source item does not satisfy the filter, so it must be removed
            source_items_remove.append(source_item);
        }
    }
    // Figure out which non-mapped items to insert
    QList<int> source_items_insert;
    int source_count = source_to_proxy.size();
    for (int source_item = 0; source_item < source_count; ++source_item) {
        if (source_to_proxy.at(source_item) == -1) {
            if ((direction == Direction::Rows)
                        ? filterAcceptsRowInternal(source_item, source_parent)
                        : filterAcceptsColumn(source_item, source_parent)) {
                // This source item satisfies the filter, so it must be added
                source_items_insert.append(source_item);
            }
        }
    }
    if (!source_items_remove.isEmpty() || !source_items_insert.isEmpty()) {
        // Do item removal and insertion
        remove_source_items(source_to_proxy, proxy_to_source,
                            source_items_remove, source_parent, direction);
        if (direction == Direction::Rows)
            sort_source_rows(source_items_insert, source_parent);
        insert_source_items(source_to_proxy, proxy_to_source,
                            source_items_insert, source_parent, direction);
    }
    return qListToSet(source_items_remove);
}


void QSortFilterProxyModelHelper::filter_changed(Direction dir, const QModelIndex &source_parent)
{
    QSortFilterProxyModelHelper::IndexMap::const_iterator it = source_index_mapping.constFind(source_parent);
    if (it == source_index_mapping.constEnd())
        return;
    Mapping *m = it.value();
    const QSet<int> rows_removed = (dir & Direction::Rows) ? handle_filter_changed(m->proxy_rows, m->source_rows, source_parent, Direction::Rows) : QSet<int>();
    const QSet<int> columns_removed = (dir & Direction::Columns) ? handle_filter_changed(m->proxy_columns, m->source_columns, source_parent, Direction::Columns) : QSet<int>();

    // We need to iterate over a copy of m->mapped_children because otherwise it may be changed by
    // other code, invalidating the iterator it2.
    // The m->mapped_children vector can be appended to with indexes which are no longer filtered
    // out (in create_mapping) when this function recurses for child indexes.
    const QList<QModelIndex> mappedChildren = m->mapped_children;
    QList<int> indexesToRemove;
    for (int i = 0; i < mappedChildren.size(); ++i) {
        const QModelIndex &source_child_index = mappedChildren.at(i);
        if (rows_removed.contains(source_child_index.row()) || columns_removed.contains(source_child_index.column())) {
            indexesToRemove.push_back(i);
            remove_from_mapping(source_child_index);
        } else {
            filter_changed(dir, source_child_index);
        }
    }
    QList<int>::const_iterator removeIt = indexesToRemove.constEnd();
    const QList<int>::const_iterator removeBegin = indexesToRemove.constBegin();

    // We can't just remove these items from mappedChildren while iterating above and then
    // do something like m->mapped_children = mappedChildren, because mapped_children might
    // be appended to in create_mapping, and we would lose those new items.
    // Because they are always appended in create_mapping, we can still remove them by
    // position here.
    while (removeIt != removeBegin) {
        --removeIt;
        m->mapped_children.remove(*removeIt);
    }
}

/*!
    \internal

    Sorts the existing mappings.
*/
void QSortFilterProxyModelHelper::sort()
{
    auto *pModel = const_cast<QAbstractProxyModel *>(proxyModel());
    emit pModel->layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
    QModelIndexPairList source_indexes = store_persistent_indexes();
    for (auto [key, value]: source_index_mapping.asKeyValueRange()) {
        const QModelIndex &source_parent = key;
        Mapping *m = value;
        sort_source_rows(m->source_rows, source_parent);
        build_source_to_proxy_mapping(m->source_rows, m->proxy_rows);
    }
    update_persistent_indexes(source_indexes);
    emit pModel->layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}


QT_END_NAMESPACE
