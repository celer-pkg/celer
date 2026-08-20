// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/QBarCategoryAxis>
#include <private/qbarcategoryaxis_p.h>
#include <QtCore/QtMath>

QT_BEGIN_NAMESPACE
/*!
    \class QBarCategoryAxis
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QBarCategoryAxis class adds categories to a graph's axes.

    QBarCategoryAxis can be set up to show an axis line with tick marks, grid lines, and shades.
    Categories are drawn between the ticks.
*/

/*!
    \qmltype BarCategoryAxis
    \nativetype QBarCategoryAxis
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits AbstractAxis

    \brief Adds categories to a graph's axes.

    The BarCategoryAxis type can be set up to show an axis line with tick marks, grid lines, and
    shades. Categories are drawn between the ticks.

    The following QML snippet illustrates how to use BarCategoryAxis:
    \code
        GraphsView {
            anchors.fill: parent
            axisX: BarCategoryAxis {
                categories: ["2023", "2024", "2025"]
                lineVisible: false
            }
            axisY: ValueAxis { }
            BarSeries {
                BarSet {
                    values: [7, 6, 9]
                }
            }
        }
    \endcode
*/

/*!
    \property QBarCategoryAxis::categories
    \brief The categories of an axis.
*/
/*!
    \qmlproperty list BarCategoryAxis::categories
    The categories of an axis.
*/

/*!
    \property QBarCategoryAxis::min
    \brief The minimum value on the axis.

    The minimum value on the axis. The given value should be contained in \c categories.
*/
/*!
    \qmlproperty string BarCategoryAxis::min
    The minimum value on the axis. The given value should be contained in \c categories.
*/

/*!
    \property QBarCategoryAxis::max
    \brief The maximum value on the axis.

    The maximum value on the axis. The given value should be contained in \c categories.
*/
/*!
    \qmlproperty string BarCategoryAxis::max
    The maximum value on the axis. The given value should be contained in \c categories.
*/

/*!
    \property QBarCategoryAxis::count
    \brief The number of categories of an axis.
*/
/*!
    \qmlproperty int BarCategoryAxis::count
    The number of categories of an axis.
*/

/*!
    \enum QBarCategoryAxis::LabelPosition
    \since 6.11
    This enum describes the position of the category labels.
    \value Center Labels are centered to category.
    \value OnValue Labels are positioned to the high end limit of the category.
*/
/*!
    \property QBarCategoryAxis::labelPosition
    \since 6.11
    \brief The position of the category labels. The labels in the beginning and in the end of the
    axes may overlap other axes' labels when positioned on value.
*/
/*!
    \qmlproperty enumeration BarCategoryAxis::labelPosition
    \since 6.11
    \qmlenumeratorsfrom QBarCategoryAxis::LabelPosition
*/

/*!
    \qmlsignal BarCategoryAxis::categoriesChanged()
    This signal is emitted when the categories of the axis change.
*/

/*!
    \qmlsignal BarCategoryAxis::minChanged(string min)
    This signal is emitted when the axis minimum value changes to \a min.
*/

/*!
    \qmlsignal BarCategoryAxis::maxChanged(string max)
    This signal is emitted when the axis maximum value changes to \a max.
*/

/*!
    \qmlsignal BarCategoryAxis::countChanged()
    This signal is emitted when the number of categories of an axis changes.
*/

/*!
    \qmlsignal BarCategoryAxis::categoryRangeChanged(string min, string max)
    This signal is emitted when the range of categories of the axis changes. \a min and \a max are
    the min and max of the new range.
*/

/*!
    Constructs an axis object that is the child of \a parent.
*/
QBarCategoryAxis::QBarCategoryAxis(QObject *parent)
    : QBarCategoryAxis(*(new QBarCategoryAxisPrivate), parent)
{
}

/*!
    Destroys the axis object.
*/
QBarCategoryAxis::~QBarCategoryAxis()
{
}

/*!
    \internal
*/
QBarCategoryAxis::QBarCategoryAxis(QBarCategoryAxisPrivate &dd, QObject *parent)
    : QAbstractAxis(dd, parent)
{
    QObject::connect(this, &QBarCategoryAxis::categoriesChanged, this, &QAbstractAxis::update);
    QObject::connect(this, &QBarCategoryAxis::labelPositionChanged, this, &QAbstractAxis::update);
}

/*!
    \qmlmethod void BarCategoryAxis::append(list categories)
    Appends \a categories to an axis. The maximum value on the axis will be changed
    to match the last category in \a categories. If no categories were previously defined,
    the minimum value on the axis will also be changed to match the first category in
    \a categories.

    A category has to be a valid QString and it cannot be duplicated. Duplicated
    categories will not be appended.
*/
/*!
    Appends \a categories to an axis. The maximum value on the axis will be changed
    to match the last category in \a categories. If no categories were previously defined,
    the minimum value on the axis will also be changed to match the first category in
    \a categories.

    A category has to be a valid QString and it cannot be duplicated. Duplicated
    categories will not be appended.
*/
void QBarCategoryAxis::append(const QStringList &categories)
{
    if (categories.isEmpty())
        return;

    Q_D(QBarCategoryAxis);

    qsizetype count = d->m_categories.size();

    for (const auto &category : categories) {
        if (!d->m_categories.contains(category) && !category.isNull()) {
            d->m_categories.append(category);
        }
    }

    if (d->m_categories.size() == count)
        return;

    if (count == 0)
        setRange(d->m_categories.first(), d->m_categories.last());
    else
        setRange(d->m_minCategory, d->m_categories.last());

    emit categoriesChanged();
    emit countChanged();
}

/*!
    \qmlmethod void BarCategoryAxis::append(string category)
    Appends \a category to an axis. The maximum value on the axis will be changed
    to match the last \a category. If no categories were previously defined, the minimum
    value on the axis will also be changed to match \a category.

    A category has to be a valid QString and it cannot be duplicated. Duplicated
    categories will not be appended.
*/
/*!
    Appends \a category to an axis. The maximum value on the axis will be changed
    to match the last \a category. If no categories were previously defined, the minimum
    value on the axis will also be changed to match \a category.

    A category has to be a valid QString and it cannot be duplicated. Duplicated
    categories will not be appended.
*/
void QBarCategoryAxis::append(const QString &category)
{
    Q_D(QBarCategoryAxis);

    qsizetype count = d->m_categories.size();

    if (!d->m_categories.contains(category) && !category.isNull())
        d->m_categories.append(category);

    if (d->m_categories.size() == count)
        return;

    if (count == 0)
        setRange(d->m_categories.last(), d->m_categories.last());
    else
        setRange(d->m_minCategory, d->m_categories.last());

    emit categoriesChanged();
    emit countChanged();
}

/*!
    \qmlmethod void BarCategoryAxis::remove(string category)
    Removes \a category from the axis. Removing a category that currently sets the
    maximum or minimum value on the axis will affect the axis range.
*/
/*!
    Removes \a category from the axis. Removing a category that currently sets the
    maximum or minimum value on the axis will affect the axis range.
*/
void QBarCategoryAxis::remove(const QString &category)
{
    Q_D(QBarCategoryAxis);

    if (d->m_categories.contains(category)) {
        d->m_categories.removeAt(d->m_categories.indexOf(category));
        if (!d->m_categories.isEmpty()) {
            if (d->m_minCategory == category) {
                setRange(d->m_categories.first(), d->m_maxCategory);
            } else if (d->m_maxCategory == category) {
                setRange(d->m_minCategory, d->m_categories.last());
            }
        } else {
            setRange(QString(), QString());
        }
        emit categoriesChanged();
        emit countChanged();
    }
}

/*!
    \qmlmethod void BarCategoryAxis::remove(int index)
    Removes a category at \a index from the axis. Removing a category that currently sets the
    maximum or minimum value on the axis will affect the axis range.
*/
/*!
    Removes a category at \a index from the axis. Removing a category that currently sets the
    maximum or minimum value on the axis will affect the axis range.
*/
void QBarCategoryAxis::remove(qsizetype index)
{
    Q_D(QBarCategoryAxis);
    if (index < 0 || index >= d->m_categories.size())
        return;
    remove(d->m_categories.at(index));
}

/*!
    \qmlmethod void BarCategoryAxis::insert(int index, string category)
    Inserts \a category to the axis at \a index. \a category has to be a valid QString
    and it cannot be duplicated. If \a category is prepended or appended to other
    categories, the minimum and maximum values on the axis are updated accordingly.
*/
/*!
    Inserts \a category to the axis at \a index. \a category has to be a valid QString
    and it cannot be duplicated. If \a category is prepended or appended to other
    categories, the minimum and maximum values on the axis are updated accordingly.
*/
void QBarCategoryAxis::insert(qsizetype index, const QString &category)
{
    Q_D(QBarCategoryAxis);

    qsizetype count = d->m_categories.size();

    if (!d->m_categories.contains(category) && !category.isNull())
        d->m_categories.insert(index, category);

    if (d->m_categories.size() == count)
        return;

    if (count == 0) {
        setRange(d->m_categories.first(), d->m_categories.first());
    } else if (index == 0) {
        setRange(d->m_categories.first(), d->m_maxCategory);
    } else if (index == count) {
        setRange(d->m_minCategory, d->m_categories.last());
    }

    emit categoriesChanged();
    emit countChanged();
}

/*!
    \qmlmethod void BarCategoryAxis::replace(string oldCategory, string newCategory)
    Replaces \a oldCategory with \a newCategory. If \a oldCategory does not exist on the axis,
    nothing is done. \a newCategory has to be a valid QString and it cannot be duplicated. If
    the minimum or maximum category is replaced, the minimum and maximum values on the axis are
    updated accordingly.
*/
/*!
    Replaces \a oldCategory with \a newCategory. If \a oldCategory does not exist on the axis,
    nothing is done. \a newCategory has to be a valid QString and it cannot be duplicated. If
    the minimum or maximum category is replaced, the minimum and maximum values on the axis are
    updated accordingly.
*/
void QBarCategoryAxis::replace(const QString &oldCategory, const QString &newCategory)
{
    Q_D(QBarCategoryAxis);

    qsizetype pos = d->m_categories.indexOf(oldCategory);

    if (pos != -1 && !d->m_categories.contains(newCategory) && !newCategory.isNull()) {
        d->m_categories.replace(pos, newCategory);
        if (d->m_minCategory == oldCategory)
            setRange(newCategory, d->m_maxCategory);
        else if (d->m_maxCategory == oldCategory)
            setRange(d->m_minCategory, newCategory);

        emit categoriesChanged();
        emit countChanged();
    }
}

/*!
    \qmlmethod void BarCategoryAxis::clear()
    Removes all categories. Sets the maximum and minimum values of the axis range to QString::null.
*/
/*!
    Removes all categories. Sets the maximum and minimum values of the axis range to QString::null.
*/
void QBarCategoryAxis::clear()
{
    Q_D(QBarCategoryAxis);
    d->m_categories.clear();
    setRange(QString(), QString());
    emit categoriesChanged();
    emit countChanged();
}

/*!
    Sets \a categories and discards the old ones. The axis range is adjusted to match the
    first and last category in \a categories.

    A category has to be a valid QString and it cannot be duplicated.
*/
void QBarCategoryAxis::setCategories(const QStringList &categories)
{
    Q_D(QBarCategoryAxis);
    d->m_categories.clear();
    d->m_minCategory = QString();
    d->m_maxCategory = QString();
    d->m_min = 0;
    d->m_max = 0;
    d->m_count = 0;
    append(categories);
}

/*!
    Returns categories.
*/
QStringList QBarCategoryAxis::categories()
{
    Q_D(QBarCategoryAxis);
    return d->m_categories;
}

/*!
    Returns the number of categories.
 */
qsizetype QBarCategoryAxis::count() const
{
    Q_D(const QBarCategoryAxis);
    return d->m_categories.size();
}

void QBarCategoryAxis::setLabelPosition(LabelPosition position)
{
    Q_D(QBarCategoryAxis);
    if (d->m_labelPosition != position) {
        d->m_labelPosition = position;
        emit labelPositionChanged(position);
    }
}

QBarCategoryAxis::LabelPosition QBarCategoryAxis::labelPosition() const
{
    Q_D(const QBarCategoryAxis);
    return d->m_labelPosition;
}

/*!
    \qmlmethod string BarCategoryAxis::at(int index)
    Returns the category at \a index.
*/
/*!
    Returns the category at \a index.
*/
QString QBarCategoryAxis::at(qsizetype index) const
{
    Q_D(const QBarCategoryAxis);
    if (index < 0 || index >= d->m_categories.size())
        return QString();
    return d->m_categories.at(index);
}

/*!
    Sets the minimum category to \a min.
*/
void QBarCategoryAxis::setMin(const QString &min)
{
    Q_D(QBarCategoryAxis);
    d->setRange(min, d->m_maxCategory);
}

/*!
    Returns the minimum category.
*/
QString QBarCategoryAxis::min() const
{
    Q_D(const QBarCategoryAxis);
    return d->m_minCategory;
}

/*!
    Sets the maximum category to \a max.
*/
void QBarCategoryAxis::setMax(const QString &max)
{
    Q_D(QBarCategoryAxis);
    d->setRange(d->m_minCategory, max);
}

/*!
    Returns the maximum category.
*/
QString QBarCategoryAxis::max() const
{
    Q_D(const QBarCategoryAxis);
    return d->m_maxCategory;
}

/*!
    Sets the axis range from \a minCategory to \a maxCategory.
*/
void QBarCategoryAxis::setRange(const QString &minCategory, const QString &maxCategory)
{
    Q_D(QBarCategoryAxis);
    d->setRange(minCategory,maxCategory);
}

/*!
    Returns the type of the axis.
*/
QAbstractAxis::AxisType QBarCategoryAxis::type() const
{
    return QAbstractAxis::AxisType::BarCategory;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QBarCategoryAxisPrivate::QBarCategoryAxisPrivate()
    : m_min(0.0)
    , m_max(0.0)
    , m_count(0)
    , m_labelPosition(QBarCategoryAxis::LabelPosition::Center)
{

}

QBarCategoryAxisPrivate::~QBarCategoryAxisPrivate()
{

}

void QBarCategoryAxisPrivate::setMin(const QVariant &min)
{
    setRange(min, m_maxCategory);
}

void QBarCategoryAxisPrivate::setMax(const QVariant &max)
{
    setRange(m_minCategory, max);
}

void QBarCategoryAxisPrivate::setRange(const QVariant &min, const QVariant &max)
{
    QString value1 = min.toString();
    QString value2 = max.toString();
    setRange(value1, value2);
}

void QBarCategoryAxisPrivate::setRange(qreal min, qreal max)
{
    Q_Q(QBarCategoryAxis);

    bool categoryChanged = false;
    bool changed = false;

    if (min > max) {
        qCWarning(lcAxis2D, "QBarCategoryAxis::setRange. Tried to use invalid values, min > max");
        return;
    }

    if (!qFuzzyIsNull(m_min - min)) {
        m_min = min;
        changed = true;

        int imin = m_min + 0.5;
        if (imin >= 0 && imin < m_categories.size()) {
            QString minCategory = m_categories.at(imin);
            if (m_minCategory != minCategory && !minCategory.isEmpty()) {
                m_minCategory = minCategory;
                categoryChanged = true;
                emit q->minChanged(minCategory);
            }
        }

    }

    if (!qFuzzyIsNull(m_max - max)) {
        m_max = max;
        changed = true;

        int imax = m_max - 0.5;
        if (imax >= 0 && imax < m_categories.size()) {
            QString maxCategory = m_categories.at(imax);
            if (m_maxCategory != maxCategory && !maxCategory.isEmpty()) {
                m_maxCategory = maxCategory;
                categoryChanged = true;
                emit q->maxChanged(maxCategory);
            }
        }
    }

    if (categoryChanged){
        emit q->categoryRangeChanged(m_minCategory, m_maxCategory);
    }

    if (changed) {
        emit q->rangeChanged(m_min, m_max);
    }
}

void  QBarCategoryAxisPrivate::setRange(const QString &minCategory, const QString &maxCategory)
{
    Q_Q(QBarCategoryAxis);
    bool changed = false;

    //special case in case or clearing all categories
    if (minCategory.isNull() && maxCategory.isNull()) {
        m_minCategory = minCategory;
        m_maxCategory = maxCategory;
        m_min = 0;
        m_max = 0;
        m_count = 0;
        emit q->minChanged(minCategory);
        emit q->maxChanged(maxCategory);
        emit q->categoryRangeChanged(m_minCategory, m_maxCategory);
        emit q->rangeChanged(m_min, m_max);
        return;
    }

    if (m_categories.indexOf(maxCategory) < m_categories.indexOf(minCategory))
        return;

    if (!minCategory.isNull() && (m_minCategory != minCategory || m_minCategory.isNull())
            && m_categories.contains(minCategory)) {
        m_minCategory = minCategory;
        m_min = m_categories.indexOf(m_minCategory) - 0.5;
        changed = true;
        emit q->minChanged(minCategory);
    }

    if (!maxCategory.isNull() && (m_maxCategory != maxCategory || m_maxCategory.isNull())
            && m_categories.contains(maxCategory)) {
        m_maxCategory = maxCategory;
        m_max = m_categories.indexOf(m_maxCategory) + 0.5;
        changed = true;
        emit q->maxChanged(maxCategory);
    }

    if (changed) {
        m_count = m_max - m_min;
        emit q->categoryRangeChanged(m_minCategory, m_maxCategory);
        emit q->rangeChanged(m_min, m_max);
    }
}

QT_END_NAMESPACE

#include "moc_qbarcategoryaxis.cpp"
