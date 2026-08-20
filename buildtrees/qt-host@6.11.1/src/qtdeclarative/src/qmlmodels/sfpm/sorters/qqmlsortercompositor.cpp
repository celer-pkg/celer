// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQmlModels/private/qqmlsortercompositor_p.h>
#include <QtQmlModels/private/qqmlsortfilterproxymodel_p_p.h>
#include <QtQmlModels/private/qqmlsortfilterproxymodel_p.h>

QT_BEGIN_NAMESPACE

QQmlSorterCompositor::QQmlSorterCompositor(QObject *parent)
    : QQmlSorterBase(new QQmlSorterCompositorPrivate, parent)
{
    Q_D(QQmlSorterCompositor);
    d->init();
    // Connect the model reset with the update in the filter
    // The cache need to be updated once the model is reset with the
    // source model data.
    connect(d->m_sfpmModel, &QQmlSortFilterProxyModel::modelReset,
            this, &QQmlSorterCompositor::updateSorters);
    connect(d->m_sfpmModel, &QQmlSortFilterProxyModel::primarySorterChanged,
            this, &QQmlSorterCompositor::updateEffectiveSorters);
}

QQmlSorterCompositor::~QQmlSorterCompositor()
{

}

void QQmlSorterCompositorPrivate::init()
{
    Q_Q(QQmlSorterCompositor);
    m_sfpmModel = qobject_cast<QQmlSortFilterProxyModel *>(q->parent());
}

void QQmlSorterCompositor::append(QQmlListProperty<QQmlSorterBase> *sorterComp, QQmlSorterBase* sorter)
{
    auto *sorterCompositor = reinterpret_cast<QQmlSorterCompositor *>(sorterComp->object);
    sorterCompositor->append(sorter);
}

qsizetype QQmlSorterCompositor::count(QQmlListProperty<QQmlSorterBase> *sorterComp)
{
    auto *sorterCompositor = reinterpret_cast<QQmlSorterCompositor *> (sorterComp->object);
    return sorterCompositor->count();
}

QQmlSorterBase *QQmlSorterCompositor::at(QQmlListProperty<QQmlSorterBase> *sorterComp, qsizetype index)
{
    auto *sorterCompositor = reinterpret_cast<QQmlSorterCompositor *> (sorterComp->object);
    return sorterCompositor->at(index);
}

void QQmlSorterCompositor::clear(QQmlListProperty<QQmlSorterBase> *sorterComp)
{
    auto *sorterCompositor = reinterpret_cast<QQmlSorterCompositor *> (sorterComp->object);
    sorterCompositor->clear();
}

void QQmlSorterCompositor::append(QQmlSorterBase *sorter)
{
    if (!sorter)
        return;
    Q_D(QQmlSorterCompositor);
    d->m_sorters.append(sorter);
    // Update sorter cache depending on the priority
    updateCache();
    // Connect the sorter to the corresponding slot to invalidate the model
    // and the sorter cache
    QObject::connect(sorter, &QQmlSorterBase::invalidateModel,
                         d->m_sfpmModel, &QQmlSortFilterProxyModel::invalidate);
    // This is needed as its required to update cache when there is any
    // change in the filter itself (for instance, a change in the priority of
    // the filter)
    QObject::connect(sorter, &QQmlSorterBase::invalidateCache,
                     this, &QQmlSorterCompositor::updateCache);
    // Reset the primary sort column when any sort order or column
    // changed
    QObject::connect(sorter, &QQmlSorterBase::sortOrderChanged,
                     this, [d] { d->resetPrimarySorter(); });
    QObject::connect(sorter, &QQmlSorterBase::columnChanged,
                     this, [d] { d->resetPrimarySorter(); });
    // Since we added new filter to the list, emit the filter changed signal
    // for the filters thats been appended to the list
    emit d->m_sfpmModel->sortersChanged();
}

qsizetype QQmlSorterCompositor::count()
{
    Q_D(QQmlSorterCompositor);
    return d->m_sorters.count();
}

QQmlSorterBase* QQmlSorterCompositor::at(qsizetype index)
{
    Q_D(QQmlSorterCompositor);
    return d->m_sorters.at(index);
}

void QQmlSorterCompositor::clear()
{
    Q_D(QQmlSorterCompositor);
    d->m_effectiveSorters.clear();
    d->m_sorters.clear();
    // Emit the filter changed signal as we cleared the filter list
    emit d->m_sfpmModel->sortersChanged();
}

QList<QQmlSorterBase *> QQmlSorterCompositor::sorters()
{
    Q_D(QQmlSorterCompositor);
    return d->m_sorters;
}

QQmlListProperty<QQmlSorterBase> QQmlSorterCompositor::sortersListProperty()
{
    Q_D(QQmlSorterCompositor);
    return QQmlListProperty<QQmlSorterBase>(reinterpret_cast<QObject*>(this), &d->m_sorters,
                                                  QQmlSorterCompositor::append,
                                                  QQmlSorterCompositor::count,
                                                  QQmlSorterCompositor::at,
                                                  QQmlSorterCompositor::clear);
}

void QQmlSorterCompositor::updateEffectiveSorters()
{
    Q_D(QQmlSorterCompositor);

    if (!d->m_primarySorter || !d->m_primarySorter->enabled()) {
        updateCache();
        return;
    }

    QList<QQmlSorterBase *> sorters;
    sorters.append(d->m_primarySorter);
    std::copy_if(d->m_effectiveSorters.constBegin(), d->m_effectiveSorters.constEnd(),
                 std::back_inserter(sorters), [d](QQmlSorterBase *sorter){
                     // Consider only the filters that are enabled and exclude the primary
                     // sorter as its already added to the list
                     return (sorter != d->m_primarySorter);
                 });
    d->m_effectiveSorters = sorters;
}

void QQmlSorterCompositor::updateSorters()
{
    Q_D(QQmlSorterCompositor);
    // Update sorters that has dependency with the model data to determine
    // whether it needs to be included or not
    for (auto &sorter: d->m_sorters)
        sorter->update(d->m_sfpmModel);
    updateCache();
}

void QQmlSorterCompositor::updateCache()
{
    Q_D(QQmlSorterCompositor);
    // Clear the existing cache
    d->m_effectiveSorters.clear();
    if (d->m_sfpmModel && d->m_sfpmModel->sourceModel()) {
        // Sort the filter according to their priority
        QList<QQmlSorterBase *> sorters = d->m_sorters;
        std::stable_sort(sorters.begin(), sorters.end(),
                [](QQmlSorterBase *sorterLeft, QQmlSorterBase *sorterRight) {
                        return sorterLeft->priority() < sorterRight->priority();
                });
        // Cache only the filters that are need to be evaluated (in order)
        std::copy_if(sorters.begin(), sorters.end(), std::back_inserter(d->m_effectiveSorters),
                [](QQmlSorterBase *sorter) { return sorter->enabled(); });
        // If there is no primary sorter set by the user explicitly, reset the
        // primary sorter according to the sorters in the lists
        d->resetPrimarySorter();
    }
}

bool QQmlSorterCompositor::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight, const QQmlSortFilterProxyModel *proxyModel) const
{
    Q_D(const QQmlSorterCompositor);
    for (const auto &sorter : d->m_effectiveSorters) {
        const int sortSection = sorter->column();
        if ((sortSection > -1) && (sortSection < proxyModel->sourceModel()->columnCount())) {
            const auto *sourceModel = proxyModel->sourceModel();
            const QPartialOrdering result = sorter->compare(sourceModel->index(sourceLeft.row(), sortSection),
                                                            sourceModel->index(sourceRight.row(), sortSection),
                                                            proxyModel);
            if (result == QPartialOrdering::Less || result == QPartialOrdering::Greater) {
                auto *priv = static_cast<const QQmlSortFilterProxyModelPrivate *>(QQmlSortFilterProxyModelPrivate::get(proxyModel));
                return (priv->m_sortOrder == sorter->sortOrder()) ? result < 0 : result > 0;
            }
        }
    }
    // Verify the index order when the ordering is either equal or unordered
    return sourceLeft.row() < sourceRight.row();
}

void QQmlSorterCompositorPrivate::setPrimarySorter(QQmlSorterBase *sorter)
{
    if (sorter == nullptr ||
        (std::find(m_sorters.constBegin(), m_sorters.constEnd(), sorter) != m_sorters.constEnd())) {
        m_primarySorter = sorter;
        if (m_primarySorter && m_primarySorter->enabled()) {
            m_sfpmModel->setPrimarySortOrder(m_primarySorter->sortOrder());
            m_sfpmModel->setPrimarySortColumn(m_primarySorter->column());
            return;
        }
    }
    resetPrimarySorter();
}

void QQmlSorterCompositorPrivate::resetPrimarySorter()
{
    if (!m_primarySorter) {
        if (!m_effectiveSorters.isEmpty()) {
            // Set the primary sort column and its order to the proxy model
            const auto *sorter = m_effectiveSorters.at(0);
            m_sfpmModel->setPrimarySortOrder(sorter->sortOrder());
            m_sfpmModel->setPrimarySortColumn(sorter->column());
        } else {
            // By default reset the  sort order to ascending order and the
            // column to 0
            m_sfpmModel->setPrimarySortOrder(Qt::AscendingOrder);
            m_sfpmModel->setPrimarySortColumn(0);
        }
    }
}

QT_END_NAMESPACE

#include "moc_qqmlsortercompositor_p.cpp"
