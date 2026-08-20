// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtQmlModels/private/qqmlsorterbase_p.h>
#include <QtQmlModels/private/qqmlsortfilterproxymodel_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Sorter
    \inherits QtObject
    \inqmlmodule QtQml.Models
    \since 6.10
    \preliminary
    \brief Abstract base type providing functionality common to sorters.

    Sorter provides a set of common properties for all the sorters that they
    inherit from.
*/

QQmlSorterBase::QQmlSorterBase(QQmlSorterBasePrivate *privObj, QObject *parent)
    : QObject (*privObj, parent)
{
}

/*!
    \qmlproperty bool Sorter::enabled

    This property enables the \l SortFilterProxyModel to consider this sorter
    while sorting the model data.

    The default value is \c true.
*/
bool QQmlSorterBase::enabled() const
{
    Q_D(const QQmlSorterBase);
    return d->m_enabled;

}

void QQmlSorterBase::setEnabled(const bool enabled)
{
    Q_D(QQmlSorterBase);
    if (d->m_enabled == enabled)
        return;
    d->m_enabled = enabled;
    invalidate(true);
    emit enabledChanged();
}

/*!
    \qmlproperty Qt::SortOrder Sorter::sortOrder

    This property holds the order used by \l SortFilterProxyModel when sorting
    the model data.

    The default value is \c Qt::AscendingOrder.
*/
Qt::SortOrder QQmlSorterBase::sortOrder() const
{
    Q_D(const QQmlSorterBase);
    return d->m_sortOrder;
}

void QQmlSorterBase::setSortOrder(const Qt::SortOrder sortOrder)
{
    Q_D(QQmlSorterBase);
    if (d->m_sortOrder == sortOrder)
        return;
    d->m_sortOrder = sortOrder;
    invalidate();
    emit sortOrderChanged();
}

/*!
    \qmlproperty int Sorter::priority

    This property holds the priority that is given to this sorter compared to
    other sorters. The lesser value results in a higher priority and the higher
    value results in a lower priority.

    The default value is \c -1.
*/
int QQmlSorterBase::priority() const
{
    Q_D(const QQmlSorterBase);
    return d->m_sorterPriority;
}

void QQmlSorterBase::setPriority(const int priority)
{
    Q_D(QQmlSorterBase);
    if (d->m_sorterPriority == priority)
        return;
    d->m_sorterPriority = priority;
    invalidate(true);
    emit priorityChanged();
}

/*!
    \qmlproperty int Sorter::column

    This property holds the column that this sorter is applied to.

    The default value is \c 0.
*/
int QQmlSorterBase::column() const
{
    Q_D(const QQmlSorterBase);
    return d->m_sortColumn;
}

void QQmlSorterBase::setColumn(const int column)
{
    Q_D(QQmlSorterBase);
    if (d->m_sortColumn == column)
        return;
    d->m_sortColumn = column;
    invalidate();
    emit columnChanged();
}

/*!
    \internal
*/
void QQmlSorterBase::invalidate(bool updateCache)
{
    // Update the cached filters and invalidate the model
    if (updateCache)
        emit invalidateCache(this);
    emit invalidateModel();
}

QT_END_NAMESPACE

#include "moc_qqmlsorterbase_p.cpp"
