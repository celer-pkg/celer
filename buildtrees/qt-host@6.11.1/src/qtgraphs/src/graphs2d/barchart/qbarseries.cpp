// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/qabstractseries_p.h"
#include <QtGraphs/qabstractseries.h>
#include <QtGraphs/qbarseries.h>
#include <QtGraphs/qbarset.h>
#include <private/qabstractseries_p.h>
#include <private/qbarseries_p.h>
#include <private/qbarset_p.h>
#include <private/qgraphsview_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QBarSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QBarSeries class presents data in bar graphs.

    This class draws data by default as a series of bars grouped by category,
    with one bar per category from each bar set added to the series.
    It also supports horizontal bars and grouping bars as stacked.

    A bar series needs the GraphsView x-axis to be set to a BarCategoryAxis and
    the y-axis set to ValueAxis.

    \sa QBarSet, QAbstractSeries
*/
/*!
    \qmltype BarSeries
    \nativetype QBarSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits AbstractSeries

    \brief Presents data in bar graphs.

    Draws data by default as a series of bars grouped by category,
    with one bar per category from each bar set added to the series.
    It also supports horizontal bars and grouping bars as stacked.
*/

/*!
    \property QBarSeries::barWidth
    \brief The width of the bars of the series.

    The unit of width is the unit of the x-axis. The minimum width for bars is zero, and negative
    values are treated as zero. Setting the width to zero means that the width of the bar on the
    screen is one pixel regardless of the scale of the x-axis. Bars wider than zero are scaled
    using the x-axis scale.

    By default, the barWidth is 0.5 (bars will take 50% of the available width).
    The valid values range from 0.0 (0%) to 1.0 (100%).

    \note When used with QBarSeries, this value specifies the width of a group of bars instead of
    that of a single bar.
    \sa QBarSeries
*/

/*!
    \enum QBarSeries::BarsType

    This enum value describes the type of the bar series:

    \value Groups Bar sets are grouped by category.
    \value Stacked Bar sets are stacked after each other by category.
    \value StackedPercent Bar sets are stacked after each other by category.
           The segment size corresponds to the percentage of the segment value
           compared with the total value of all segments in the stack.
*/
/*!
    \property QBarSeries::barsType
    \brief The type of the bar series.
*/
/*!
    \qmlproperty enumeration BarSeries::barsType

    The type of the bar series:

    \value BarSeries.BarsType.Groups
        Bar sets are grouped by category. This is the default value.
    \value BarSeries.BarsType.Stacked
        Bar sets are stacked after each other by category.
    \value BarSeries.BarsType.StackedPercent
        Bar sets are stacked after each other by category. The segment size corresponds
        to the percentage of the segment value compared with the total value of all
        segments in the stack.
*/

/*!
    \qmlproperty real BarSeries::barWidth
    The unit of width is the unit of the x-axis. The minimum width for bars is zero, and negative
    values are treated as zero. Setting the width to zero means that the width of the bar on the
    screen is one pixel regardless of the scale of the x-axis. Bars wider than zero are scaled
    using the x-axis scale.

    By default, the barWidth is 0.5 (bars will take 50% of the available width).
    The valid values range from 0.0 (0%) to 1.0 (100%).

    \note When used with the BarSeries type, this value specifies the width of a group of bars
    instead of that of a single bar.
*/

/*!
    \property QBarSeries::count
    \brief The number of bar sets in a bar series.
*/
/*!
    \qmlproperty int BarSeries::count
    The number of bar sets in a bar series.
*/

/*!
    \property QBarSeries::labelsVisible
    \brief The visibility of the labels in a bar series.
    The default label visibility is \c false.

    \note The default label only displays the value for the set.
    You can create custom labels with a \l barDelegate.
    \sa barDelegate
*/
/*!
    \qmlproperty bool BarSeries::labelsVisible
    The visibility of the labels in a bar series.
    The default label visibility is \c false.

    \note The default label only displays the value for the set.
    You can create custom labels with a \l barDelegate.

    \sa barDelegate
*/

/*!
    \property QBarSeries::labelsFormat
    \brief The format used for showing labels in a bar series.

    QBarSeries supports the following format tag:
    \table
    \row
    \li @value      \li The value of the bar
    \endtable

    For example, the following usage of the format tags would produce labels that show the value
    followed by the unit (u):
    \code
    series->setLabelsFormat("@value u");
    \endcode

    By default, the labels show the value of the bar. For the percent bar series, \e % is added
    after the value. The labels are shown on the plot area, if the bars are close to each other,
    the labels may overlap.

    \sa labelsVisible, labelsPosition, labelsPrecision
*/
/*!
    \qmlproperty string BarSeries::labelsFormat
    The format used for showing labels in a bar series.

    \sa QBarSeries::labelsFormat, labelsVisible, labelsPosition
*/

/*!
    \enum QBarSeries::LabelsPosition

    This enum value describes the position of the data value labels:

    \value Center Label is located in the center of the bar.
    \value InsideEnd Label is located inside the bar at the top.
    \value InsideBase Label is located inside the bar at the bottom.
    \value OutsideEnd Label is located outside the bar at the top.
*/

/*!
    \property QBarSeries::labelsPosition
    \brief The position of value labels.

    \sa labelsVisible, labelsFormat
*/
/*!
    \qmlproperty enumeration BarSeries::labelsPosition

    The position of the data value labels:

    \value  BarSeries.LabelsPosition.Center
        Label is located in the center of the bar.
    \value  BarSeries.LabelsPosition.InsideEnd
        Label is located inside the bar at the top.
    \value  BarSeries.LabelsPosition.InsideBase
        Label is located inside the bar at the bottom.
    \value  BarSeries.LabelsPosition.OutsideEnd
        Label is located outside the bar at the top.

    \sa labelsVisible, labelsFormat
*/

/*!
    \property QBarSeries::labelsMargin
    \brief The margin of the value labels in pixels.

    This margin from side is used when \l labelsPosition is set to something else
    than \c LabelsPosition.Center. The default value is \c 0.
*/
/*!
    \qmlproperty real BarSeries::labelsMargin
    The margin of the value labels in pixels.

    This margin from side is used when \l labelsPosition is set to something else
    than \c LabelsPosition.Center. The default value is \c 0.
*/

/*!
    \property QBarSeries::labelsAngle
    \brief The angle of the value labels in degrees.
*/
/*!
    \qmlproperty real BarSeries::labelsAngle
    The angle of the value labels in degrees.
*/

/*!
    \property QBarSeries::labelsPrecision
    \brief The maximum amount of significant digits shown in value labels.

    Default value is 6.
*/
/*!
    \qmlproperty real BarSeries::labelsPrecision
    The maximum amount of significant digits shown in value labels.

    Default value is 6.
*/

/*!
    \property QBarSeries::barDelegate
    \brief A custom QML component used for visualizing each of the bars.
    Instance of this component is created for each of the bar.
    When this is not defined, a default rectangle visualization for bars is used.

    The dynamic properties available for this component are:

    \table
    \header
        \li Type
        \li Name
        \li Description
    \row
        \li QColor
        \li barColor
        \li The fill color of the bar. This value comes either from the \l QGraphsTheme
        or from \l{QBarSet::color} if the \l QBarSet overrides the color.
    \row
        \li QColor
        \li barBorderColor
        \li The border color of the bar. This value comes either from the \l QGraphsTheme
        or from \l{QBarSet::borderColor} if the \l QBarSet overrides the color.
    \row
        \li qreal
        \li barBorderWidth
        \li The width of the bar border. This value comes either from the \l QGraphsTheme
        or from \l{QBarSet::borderWidth} if the \l QBarSet overrides the width.
    \row
        \li qreal
        \li barValue
        \li The value of the bar. This value comes from the \l{QBarSet::values}.
    \row
        \li QString
        \li barLabel
        \li The label of the bar. This value comes from the \l{QBarSet::label}.
    \row
        \li bool
        \li barSelected
        \li This value is true when the bar is selected, meaning that the bar index
        is in \l{QBarSet::selectedBars}.
    \row
        \li int
        \li barIndex
        \li Index of the bar, from 0 to the amount of bars - 1. [since 6.9]
    \endtable

    To use any of these, add property with the defined name into your custom component.
    For example \c{"property color barColor"} and \c{"property real barValue"}.
*/
/*!
    \qmlproperty Component BarSeries::barDelegate
    A custom QML component used for visualizing each of the bars.
    Instance of this component is created for each of the bar.
    When this is not defined, a default rectangle visualization for bars is used.

    The dynamic properties available for this component are:

    \table
    \header
        \li Type
        \li Name
        \li Description
    \row
        \li color
        \li barColor
        \li The fill color of the bar. This value comes either from the \l GraphsTheme
        or from \l{BarSet::color} if the \l BarSet overrides the color.
    \row
        \li color
        \li barBorderColor
        \li The border color of the bar. This value comes either from the \l GraphsTheme
        or from \l{BarSet::borderColor} if the \l BarSet overrides the color.
    \row
        \li real
        \li barBorderWidth
        \li The width of the bar border. This value comes either from the \l GraphsTheme
        or from \l{BarSet::borderWidth} if the \l BarSet overrides the width.
    \row
        \li real
        \li barValue
        \li The value of the bar. This value comes from the \l{BarSet::values}.
    \row
        \li string
        \li barLabel
        \li The label of the bar. This value comes from the \l{BarSet::label}.
    \row
        \li bool
        \li barSelected
        \li This value is true when the bar is selected, meaning that the bar index
        is in \l{BarSet::selectedBars}.
    \row
        \li int
        \li barIndex
        \li Index of the bar, from 0 to the amount of bars - 1. [since 6.9]
    \endtable

    To use any of these, add property with the defined name into your custom component.
    For example \c{"property color barColor"} and \c{"property real barValue"}.
*/

/*!
    \property QBarSeries::barSets
    \brief A list of sets added to the series.
 */
/*!
    \qmlproperty list<BarSet> BarSeries::barSets
    A list of sets added to the series.
*/

/*!
    \fn void QBarSeries::updatedBars()
    This signal is emitted when bars are updated.
*/

/*!
    \fn void QBarSeries::labelsVisibleChanged(bool visible)
    This signal is emitted when the labels' visibility changes to \a visible.
    \sa labelsVisible(), setLabelsVisible()
*/

/*!
    \fn void QBarSeries::barsetsAdded(const QList<QBarSet *> &sets)
    This signal is emitted when the bar sets specified by \a sets are added to the series.
    \sa append(), insert()
*/
/*!
    \qmlsignal BarSeries::barsetsAdded()
    This signal is emitted when bar sets are added to the series.
*/

/*!
    \fn void QBarSeries::barsetsRemoved(const QList<QBarSet *> &sets)
    This signal is emitted when the bar sets specified by \a sets are removed from the series.
    \sa remove()
*/
/*!
    \qmlsignal BarSeries::barsetsRemoved()
    This signal is emitted when bar sets are removed from the series.
*/

/*!
    \fn void QBarSeries::setValueChanged(qsizetype index, QBarSet *barset)
    This signal is emitted when a barset's value is changed. \a index is the index of
    the barset in the series. The \a barset is a pointer to the changed set.
*/
/*!
    \fn void QBarSeries::setValueAdded(qsizetype index, qsizetype count, QBarSet *barset)
    This signal is emitted when a barset's value is changed. \a index is the index of
    the barset in the series. The number of the added values is indicated \a count.
    The \a barset is a pointer to the changed set.
*/
/*!
    \fn void QBarSeries::setValueRemoved(qsizetype index, qsizetype count, QBarSet *barset)
    This signal is emitted when a barset's value is changed. \a index is the index of
    the barset in the series. The number of the removed values is indicated \a count.
    The \a barset is a pointer to the changed set.
*/

/*!
    \qmlmethod BarSet BarSeries::at(int index)
    Returns the bar set at \a index. Returns null if the index is not valid.
*/

/*!
    \qmlmethod BarSet BarSeries::insert(int index, BarSet barset)
    Inserts a bar set specified by \a barset to a series at the position specified by \a index.
    If the set is \c null or already belongs to the series, the set is not
    appended. Returns \c true if inserting succeeds.

    \sa append()
*/

/*!
    \qmlmethod bool BarSeries::append(BarSet barset)

    Adds a set of bars specified by \a barset to the bar series and takes
    ownership of it. If the set is null or it already belongs to the series,
    it will not be appended.

    Returns \c true if appending succeeded.

    \sa insert()
*/

/*!
    \qmlmethod bool BarSeries::remove(BarSet barset)
    Removes the bar set specified by \a barset from the series. Returns \c true if successful,
    \c false otherwise.
*/

/*!
    \qmlmethod void BarSeries::clear()
    Removes all bar sets from the series.
*/

/*!
    \qmlmethod bool BarSeries::replace(int index, BarSet barset)
    Replaces the bar set at the position specified by \a index from the series and replaces it
    with \a barset. Returns \c true if successful, \c false otherwise.
*/

/*!
    \qmlmethod Barset BarSeries::at(int index)
    Returns the bar set specified by \a index from the series. Returns \c null otherwise.
*/

/*!
    \qmlmethod int BarSeries::find(BarSet barset)
    Returns the index of the bar set specified by \a barset from the series. Returns \c -1 if
    not found.
*/

/*!
    \qmlmethod void BarSeries::removeMultiple(int index, int count)
    Removes a range of bar sets as specified by the \a index and \a count. The call
    traverses over all sets even if removal of one fails.
*/

/*!
    \qmlmethod bool BarSeries::remove(int index)
    Removes the bar set specified by \a index from the series. Returns \c true if the
    removal was successful, \c false otherwise.
*/

/*!
    \qmlmethod bool BarSeries::replace(BarSet oldSet, BarSet newSet)
    Replaces the bar set specified by \a oldSet with newSet. Returns \c true if the
    removal was successful, \c false otherwise. \a oldSet is destroyed if this 
    is successful.
*/

/*!
    \qmlmethod bool BarSeries::replace(list<BarSet> sets)
    Completely replaces all current bar set with \a sets. The size does not need
    to match. Returns false if any of the bar set in \a sets are invalid.
*/

/*!
    \qmlsignal BarSeries::clicked(int index, BarSet barset)
    This signal is emitted when the user clicks or taps the bar specified by \a index
    in the bar set specified by \a barset.
*/

/*!
    \qmlsignal BarSeries::doubleClicked(int index, BarSet barset)
    This signal is emitted when the user double-clicks or double-taps the bar specified
    by \a index in the bar set specified by \a barset.
    This signal always occurs after \l clicked.
*/

/*!
    \qmlsignal BarSeries::pressed(int index, BarSet barset)
    This signal is emitted when the user clicks or taps the bar specified by \a index
    in the bar set specified by \a barset and holds down the mouse button or gesture.
*/

/*!
    \qmlsignal BarSeries::released(int index, BarSet barset);
    This signal is emitted when the user releases a previously pressed mouse button
    or gesture on the bar specified by \a index in the bar set specified by \a barset.
*/

/*!
    \internal
*/

/*!
    Constructs an empty bar series that is a QObject and a child of \a parent.
*/
QBarSeries::QBarSeries(QObject *parent)
    : QAbstractSeries(*(new QBarSeriesPrivate()), parent)
{}

QBarSeries::~QBarSeries() {}

QBarSeries::QBarSeries(QBarSeriesPrivate &dd, QObject *parent)
    : QAbstractSeries(dd, parent)
{}

/*!
    Returns the bar series.
*/
QAbstractSeries::SeriesType QBarSeries::type() const
{
    return QAbstractSeries::SeriesType::Bar;
}

/*!
    \property QBarSeries::seriesColors
    \brief The list of base colors to be used for all the objects in the series.

    If there are more series than colors, the color list wraps and starts again
    with the first color in the list. If this is not set (default), colors
    from the \l{QGraphsTheme::seriesColors} will be used.
*/
/*!
    \qmlproperty list<color> BarSeries::seriesColors
    The list of base colors to be used for all the objects in the series.

    If there are more series than colors, the color list wraps and starts again
    with the first color in the list. If this is not set (default), colors
    from the \l{GraphsTheme::seriesColors} will be used.
*/
QList<QColor> QBarSeries::seriesColors() const
{
    Q_D(const QBarSeries);
    return d->m_seriesColors;
}

void QBarSeries::setSeriesColors(const QList<QColor> &newSeriesColors)
{
    Q_D(QBarSeries);
    for (const QColor &color : newSeriesColors) {
        if (!color.isValid()) {
            qCWarning(lcProperties2D, "QBarSeries::setSeriesColors. Tried to use invalid color value.");
            break;
        }
    }

    if (d->m_seriesColors == newSeriesColors) {
        qCDebug(lcProperties2D) << "QBarSeries::setSeriesColors. Set value of" << newSeriesColors << "is the same as it already was.";
        return;
    }

    d->m_seriesColors = newSeriesColors;
    emit seriesColorsChanged();
    emit update();
}

/*!
    \property QBarSeries::borderColors
    \brief The list of border colors to be used for all the objects in the series.

    If there are more series than colors, the color list wraps and starts again
    with the first color in the list. If this is not set (default), colors
    from the \l{QGraphsTheme::borderColors} will be used.
*/
/*!
    \qmlproperty list<color> BarSeries::borderColors
    The list of border colors to be used for all the objects in the series.

    If there are more series than colors, the color list wraps and starts again
    with the first color in the list. If this is not set (default), colors
    from the \l{GraphsTheme::borderColors} will be used.
*/
QList<QColor> QBarSeries::borderColors() const
{
    Q_D(const QBarSeries);
    return d->m_borderColors;
}

void QBarSeries::setBorderColors(const QList<QColor> &newBorderColors)
{
    Q_D(QBarSeries);
    for (const QColor &color : newBorderColors) {
        if (!color.isValid()) {
            qCWarning(lcProperties2D, "QBarSeries::setBorderColors. Tried to use invalid color value.");
            break;
        }
    }

    if (d->m_borderColors == newBorderColors) {
        qCDebug(lcProperties2D) << "QBarSeries::setBorderColors. Set value of:" << newBorderColors << "is the same than it already was.";
        return;
    }

    d->m_borderColors = newBorderColors;
    emit borderColorsChanged();
    emit update();
}

void QBarSeries::setBarsType(QBarSeries::BarsType type)
{
    Q_D(QBarSeries);
    if (d->m_barsType != type) {
        d->m_barsType = type;
        emit barsTypeChanged(type);
        emit update();
    } else {
        qCDebug(lcProperties2D) << "QBarSeries::setBarsType. Set value of:" << type << "is the same than it already was.";
    }
}

QBarSeries::BarsType QBarSeries::barsType() const
{
    Q_D(const QBarSeries);
    return d->m_barsType;
}

/*!
    Sets the width of the bars of the series to \a width.
*/
void QBarSeries::setBarWidth(qreal width)
{
    Q_D(QBarSeries);
    if (d->barWidth() != width) {
        d->setBarWidth(width);
        emit barWidthChanged();
    } else {
        qCDebug(lcProperties2D, "QBarSeries::setBarWidth. Set value of: %f is the same than it already was.",
                width);
    }
}

/*!
    Returns the width of the bars of the series.
    \sa setBarWidth()
*/
qreal QBarSeries::barWidth() const
{
    Q_D(const QBarSeries);
    return d->barWidth();
}

/*!
    Adds a set of bars specified by \a set to the bar series and takes ownership of it. If the set
    is null or it already belongs to the series, it will not be appended.
    Returns \c true if appending succeeded.
*/
bool QBarSeries::append(QBarSet *set)
{
    Q_D(QBarSeries);
    bool success = d->append(set);
    if (success) {
        QList<QBarSet *> sets;
        sets.append(set);
        set->setParent(this);
        QObject::connect(set, &QBarSet::update, this, &QBarSeries::update);
        emit barsetsAdded(sets);
        emit barSetsChanged();
        emit countChanged();
        emit update();
    }
    return success;
}

/*!
    Removes the bar set specified by \a set from the series and permanently deletes it if
    the removal succeeds. Returns \c true if the set was removed.
*/
bool QBarSeries::remove(QBarSet *set)
{
    Q_D(QBarSeries);
    bool success = d->remove(set);
    if (success) {
        QList<QBarSet *> sets;
        sets.append(set);
        set->setParent(0);
        QObject::disconnect(set, &QBarSet::update, this, &QBarSeries::update);
        emit barsetsRemoved(sets);
        emit barSetsChanged();
        emit countChanged();
        emit update();
        delete set;
        set = 0;
    }
    return success;
}

/*!
    Takes a single \a set from the series. Does not delete the bar set object.
    \note The series remains the barset's parent object. You must set the
    parent object to take full ownership.

    Returns \c true if the take operation succeeds.
*/
bool QBarSeries::take(QBarSet *set)
{
    Q_D(QBarSeries);
    bool success = d->remove(set);
    if (success) {
        QList<QBarSet *> sets;
        sets.append(set);
        QObject::disconnect(set, &QBarSet::update, this, &QBarSeries::update);
        emit barsetsRemoved(sets);
        emit barSetsChanged();
        emit countChanged();
        emit update();
    }
    return success;
}

/*!
    Adds a list of bar sets specified by \a sets to a bar series and takes ownership of the sets.
    Returns \c true if all sets were appended successfully. If any of the sets is null or was
    previously appended to the series, nothing is appended and this function returns \c false.
    If any of the sets appears in the list more than once, nothing is appended and this function
    returns \c false.
*/
bool QBarSeries::append(const QList<QBarSet *> &sets)
{
    Q_D(QBarSeries);
    if (!d->append(sets))
        return false;

    for (auto *set : sets) {
        set->setParent(this);
        QObject::connect(set, &QBarSet::update, this, &QBarSeries::update);
    }

    emit barsetsAdded(sets);
    emit barSetsChanged();
    emit countChanged();
    emit update();
    return true;
}

/*!
    Inserts a bar set specified by \a set to a series at the position specified by \a index
    and takes ownership of the set. If the set is null or already belongs to the series, it will
    not be appended. Returns \c true if inserting succeeds.
*/
bool QBarSeries::insert(qsizetype index, QBarSet *set)
{
    Q_D(QBarSeries);
    bool success = d->insert(index, set);
    if (success) {
        QList<QBarSet *> sets;
        sets.append(set);
        QObject::connect(set, &QBarSet::update, this, &QBarSeries::update);
        emit barsetsAdded(sets);
        emit barSetsChanged();
        emit countChanged();
        emit update();
    }
    return success;
}

/*!
    Removes all bar sets from the series and permanently deletes them.
*/
void QBarSeries::clear()
{
    Q_D(QBarSeries);
    const QList<QBarSet *> sets = barSets();
    bool success = d->remove(sets);
    if (success) {
        emit barsetsRemoved(sets);
        emit barSetsChanged();
        emit countChanged();
        for (QBarSet *set : sets) {
            QObject::disconnect(set, &QBarSet::update, this, &QBarSeries::update);
            set->deleteLater();
        }
        emit update();
    }
}

/*!
    Replaces the BarSet which is present at \a index with \a set.
    The original BarSet will be permanently deleted.
*/
void QBarSeries::replace(qsizetype index, QBarSet *set)
{
    Q_D(QBarSeries);

    if (d->m_barSets.size() <= index)
        return;
    if (!set)
        return;
    if (index < 0)
        index = 0;

    remove(d->m_barSets[index]);
    d->insert(index, set);

    QList<QBarSet *> sets;
    sets.append(set);
    QObject::connect(set, &QBarSet::update, this, &QBarSeries::update);
    emit barsetsReplaced(sets);
}

/*!
    Retrieves the BarSet specified at the location \a index.
    Returns null if no BarSet was found.
*/
QBarSet *QBarSeries::at(qsizetype index)
{
    Q_D(QBarSeries);

    if (d->m_barSets.size() <= index)
        return nullptr;
    if (index < 0)
        return nullptr;

    return d->m_barSets[index];
}

/*!
    Returns the index of the first BarSet found as defined by \a set. Returns -1 if no BarSet was found.
*/
qsizetype QBarSeries::find(QBarSet *set) const
{
    Q_D(const QBarSeries);

    for (qsizetype i = 0; i < d->m_barSets.size(); ++i) {
        if (set == d->m_barSets[i])
            return i;
    }

    return -1;
}

/*!
    Removes multiple BarSets from the series starting from \a index to a number of \a count.
    The BarSets will be permanently deleted.
*/
void QBarSeries::removeMultiple(qsizetype index, qsizetype count)
{
    Q_D(QBarSeries);

    if (index < 0 || count < 1 || index + count > d->m_barSets.size())
        return;

    for (qsizetype i = index; i < index + count; ++i)
        remove(d->m_barSets[index]);
}

/*!
    Removes the BarSet at the location \a index. The BarSet will be permanently deleted.
*/
bool QBarSeries::remove(qsizetype index)
{
    Q_D(QBarSeries);

    if (index >= d->m_barSets.size())
        return false;
    if (index < 0)
        return false;

    return remove(d->m_barSets[index]);
}

/*!
    Replaces the BarSet specified by \a oldValue with the one in \a newValue. The BarSet
    in \a oldValue will be permanently deleted if found inside the series. Returns \c true if
    the replace is successful.
*/
bool QBarSeries::replace(QBarSet *oldValue, QBarSet *newValue)
{
    Q_D(QBarSeries);

    if (!oldValue || !newValue)
        return false;
    if (oldValue == newValue)
        return false;

    for (qsizetype i = 0; i < d->m_barSets.size(); ++i) {
        if (d->m_barSets[i] == oldValue) {
            remove(d->m_barSets[i]);
            d->insert(i, newValue);

            QList<QBarSet *> sets;
            sets.append(newValue);
            QObject::connect(newValue, &QBarSet::update, this, &QBarSeries::update);
            emit barsetsReplaced(sets);
            return true;
        }
    }

    return false;
}

/*!
    Replaces the entire BarSet list inside of this BarSeries with the BarSets specified by \a sets.
    All of the original BarSets will be permanently deleted. Returns \c true if all BarSets are replaced.
*/
bool QBarSeries::replace(const QList<QBarSet *> &sets)
{
    Q_D(QBarSeries);

    for (const auto set : sets) {
        if (!set)
            return false;
    }

    for (const auto set : std::as_const(d->m_barSets)) {
        remove(set);
    }

    for (const auto set : sets) {
        QObject::connect(set, &QBarSet::update, this, &QBarSeries::update);
    }

    d->append(sets);
    emit barsetsReplaced(sets);

    return true;
}

/*!
    Returns the number of bar sets in a bar series.
*/
qsizetype QBarSeries::count() const
{
    Q_D(const QBarSeries);
    return d->m_barSets.size();
}

/*!
    Returns a list of bar sets in a bar series. Keeps the ownership of the bar sets.
 */
QList<QBarSet *> QBarSeries::barSets() const
{
    Q_D(const QBarSeries);
    return d->m_barSets;
}

/*!
    Sets the visibility of labels in a bar series to \a visible.
*/
void QBarSeries::setLabelsVisible(bool visible)
{
    Q_D(QBarSeries);
    if (d->m_labelsVisible != visible) {
        d->setLabelsVisible(visible);
        emit labelsVisibleChanged(visible);
        emit update();
    } else {
        qCDebug(lcProperties2D) << "QBarSeries::setLabelsVisible. Label visibility is already set to:" << visible;
    }
}

/*!
    Returns the visibility of labels.
*/
bool QBarSeries::labelsVisible() const
{
    Q_D(const QBarSeries);
    return d->m_labelsVisible;
}

void QBarSeries::setLabelsFormat(const QString &format)
{
    Q_D(QBarSeries);
    if (d->m_labelsFormat != format) {
        d->m_labelsFormat = format;
        d->setLabelsDirty(true);
        emit labelsFormatChanged(format);
        emit update();
    } else {
        qCDebug(lcProperties2D) << "QBarSeries::setLabelsFormat. Format is already set to value:"
                                << format;
    }
}

QString QBarSeries::labelsFormat() const
{
    Q_D(const QBarSeries);
    return d->m_labelsFormat;
}

void QBarSeries::setLabelsMargin(qreal margin)
{
    Q_D(QBarSeries);
    if (d->m_labelsMargin != margin) {
        d->m_labelsMargin = margin;
        d->setLabelsDirty(true);
        emit labelsMarginChanged(margin);
        emit update();
    } else {
        qCDebug(lcProperties2D, "QBarSeries::setLabelsMargin. Margin is already set to value: %f",
                margin);
    }
}

qreal QBarSeries::labelsMargin() const
{
    Q_D(const QBarSeries);
    return d->m_labelsMargin;
}

void QBarSeries::setLabelsAngle(qreal angle)
{
    Q_D(QBarSeries);
    if (d->m_labelsAngle != angle) {
        d->m_labelsAngle = angle;
        d->setLabelsDirty(true);
        emit labelsAngleChanged(angle);
        emit update();
    } else {
        qCDebug(lcProperties2D, "QBarSeries::setLabelsAngle. Label angle is already set to value: %f",
                angle);
    }
}

qreal QBarSeries::labelsAngle() const
{
    Q_D(const QBarSeries);
    return d->m_labelsAngle;
}

void QBarSeries::setLabelsPosition(QBarSeries::LabelsPosition position)
{
    Q_D(QBarSeries);
    if (d->m_labelsPosition != position) {
        d->m_labelsPosition = position;
        emit labelsPositionChanged(position);
        emit update();
    } else {
        qCDebug(lcProperties2D) << "QBarSeries::setLabelsPosition. Position is already set to:"
                                << position;
    }
}

QBarSeries::LabelsPosition QBarSeries::labelsPosition() const
{
    Q_D(const QBarSeries);
    return d->m_labelsPosition;
}

void QBarSeries::setLabelsPrecision(int precision)
{
    Q_D(QBarSeries);
    if (d->m_labelsPrecision != precision) {
        d->m_labelsPrecision = precision;
        d->setLabelsDirty(true);
        emit labelsPrecisionChanged(precision);
        emit update();
    } else {
        qCDebug(lcProperties2D, "QBarSeries::setLabelsPrecision. Precision is already set to: %d",
                precision);
    }
}

int QBarSeries::labelsPrecision() const
{
    Q_D(const QBarSeries);
    return d->m_labelsPrecision;
}

QQmlComponent *QBarSeries::barDelegate() const
{
    Q_D(const QBarSeries);
    return d->m_barDelegate;
}

void QBarSeries::setBarDelegate(QQmlComponent *newBarDelegate)
{
    Q_D(QBarSeries);
    if (d->m_barDelegate == newBarDelegate) {
        qCDebug(lcProperties2D) << "QBarSeries::setBarDelegate. BarDelegate is already set to:"
                                << newBarDelegate;
        return;
    }

    d->m_barDelegate = newBarDelegate;
    d->m_barDelegateDirty = true;
    emit barDelegateChanged();
    emit update();
}

// Select all the elements in the series
void QBarSeries::selectAll()
{
    Q_D(QBarSeries);
    for (auto s : std::as_const(d->m_barSets)) {
        s->selectAllBars();
    }
}

// Deselect all the elements in the series
void QBarSeries::deselectAll()
{
    Q_D(QBarSeries);
    for (auto s : std::as_const(d->m_barSets)) {
        s->deselectAllBars();
    }
}

void QBarSeries::componentComplete()
{
    for (auto *child : children()) {
        if (auto bs = qobject_cast<QBarSet *>(child)) {
            append(bs);
            qCDebug(lcSeries2D) << "append barset" << bs << "to barseries. barset values:" << bs->values();
        }
    }

    qCDebug(lcEvents2D) << "QBarSeries::componentComplete.";

    QAbstractSeries::componentComplete();
}

void QBarSeries::handleSetValueChange(qsizetype index)
{
    QBarSet *set = qobject_cast<QBarSet *>(sender());
    if (set)
        emit setValueChanged(index, set);
    emit update();
}

void QBarSeries::handleSetValueAdd(qsizetype index, qsizetype count)
{
    QBarSet *set = qobject_cast<QBarSet *>(sender());
    if (set)
        emit setValueAdded(index, count, set);
    emit update();
}

void QBarSeries::handleSetValueRemove(qsizetype index, qsizetype count)
{
    QBarSet *set = qobject_cast<QBarSet *>(sender());
    if (set)
        emit setValueRemoved(index, count, set);
    emit update();
}

bool QBarSeries::barDelegateDirty() const
{
    Q_D(const QBarSeries);
    return d->m_barDelegateDirty;
}

void QBarSeries::setBarDelegateDirty(bool dirty)
{
    Q_D(QBarSeries);
    d->m_barDelegateDirty = dirty;
}

QBarSeriesPrivate::QBarSeriesPrivate()
    : QAbstractSeriesPrivate(QAbstractSeries::SeriesType::Bar)
    , m_barWidth(0.5) // Default value is 50% of category width
    , m_labelsVisible(false)
    , m_visible(true)
    , m_blockBarUpdate(false)
    , m_labelsFormat()
    , m_labelsMargin(0)
    , m_labelsAngle(0)
    , m_labelsPrecision(6)
    , m_labelsDirty(true)
    , m_barDelegateDirty(false)
{
}

qsizetype QBarSeriesPrivate::categoryCount() const
{
    // No categories defined. return count of longest set.
    qsizetype count = 0;
    for (qsizetype i = 0; i < m_barSets.size(); i++) {
        if (m_barSets.at(i)->count() > count)
            count = m_barSets.at(i)->count();
    }

    return count;
}

void QBarSeriesPrivate::setBarWidth(qreal width)
{
    Q_Q(QBarSeries);
    width = std::clamp<qreal>(width, 0.0, 1.0);
    if (!qFuzzyCompare(width + 1, m_barWidth + 1)) {
        m_barWidth = width;
        emit q->update();
    }
}

qreal QBarSeriesPrivate::barWidth() const
{
    return m_barWidth;
}

QBarSet *QBarSeriesPrivate::barsetAt(qsizetype index)
{
    return m_barSets.at(index);
}

void QBarSeriesPrivate::setVisible(bool visible)
{
    Q_Q(QBarSeries);
    m_visible = visible;
    emit q->visibleChanged();
}

void QBarSeriesPrivate::setLabelsVisible(bool visible)
{
    m_labelsVisible = visible;
}

qreal QBarSeriesPrivate::min()
{
    if (m_barSets.size() <= 0)
        return 0;

    qreal min = INT_MAX;

    for (int i = 0; i < m_barSets.size(); i++) {
        qsizetype categoryCount = m_barSets.at(i)->count();
        for (qsizetype j = 0; j < categoryCount; j++) {
            qreal temp = m_barSets.at(i)->at(j);
            if (temp < min)
                min = temp;
        }
    }
    return min;
}

qreal QBarSeriesPrivate::max()
{
    if (m_barSets.size() <= 0)
        return 0;

    qreal max = INT_MIN;

    for (int i = 0; i < m_barSets.size(); i++) {
        qsizetype categoryCount = m_barSets.at(i)->count();
        for (qsizetype j = 0; j < categoryCount; j++) {
            qreal temp = m_barSets.at(i)->at(j);
            if (temp > max)
                max = temp;
        }
    }

    return max;
}

qreal QBarSeriesPrivate::valueAt(int set, int category)
{
    if ((set < 0) || (set >= m_barSets.size()))
        return 0; // No set, no value.
    else if ((category < 0) || (category >= m_barSets.at(set)->count()))
        return 0; // No category, no value.

    return m_barSets.at(set)->at(category);
}

qreal QBarSeriesPrivate::percentageAt(int set, int category)
{
    if ((set < 0) || (set >= m_barSets.size()))
        return 0; // No set, no value.
    else if ((category < 0) || (category >= m_barSets.at(set)->count()))
        return 0; // No category, no value.

    qreal value = m_barSets.at(set)->at(category);
    qreal sum = categorySum(category);
    if (qFuzzyIsNull(sum))
        return 0;

    return value / sum;
}

qreal QBarSeriesPrivate::categorySum(qsizetype category)
{
    qreal sum(0);
    qsizetype count = m_barSets.size(); // Count sets
    for (qsizetype set = 0; set < count; set++) {
        if (category < m_barSets.at(set)->count())
            sum += m_barSets.at(set)->at(category);
    }
    return sum;
}

qreal QBarSeriesPrivate::absoluteCategorySum(int category)
{
    qreal sum(0);
    qsizetype count = m_barSets.size(); // Count sets
    for (qsizetype set = 0; set < count; set++) {
        if (category < m_barSets.at(set)->count())
            sum += qAbs(m_barSets.at(set)->at(category));
    }
    return sum;
}

qreal QBarSeriesPrivate::maxCategorySum()
{
    qreal max = INT_MIN;
    qsizetype count = categoryCount();
    for (qsizetype i = 0; i < count; i++) {
        qreal sum = categorySum(i);
        if (sum > max)
            max = sum;
    }
    return max;
}

qreal QBarSeriesPrivate::minX()
{
    if (m_barSets.size() <= 0)
        return 0;

    qreal min = INT_MAX;

    for (int i = 0; i < m_barSets.size(); i++) {
        qsizetype categoryCount = m_barSets.at(i)->count();
        for (qsizetype j = 0; j < categoryCount; j++) {
            qreal temp = m_barSets.at(i)->d_func()->m_values.at(j).x();
            if (temp < min)
                min = temp;
        }
    }
    return min;
}

qreal QBarSeriesPrivate::maxX()
{
    if (m_barSets.size() <= 0)
        return 0;

    qreal max = INT_MIN;

    for (int i = 0; i < m_barSets.size(); i++) {
        qsizetype categoryCount = m_barSets.at(i)->count();
        for (qsizetype j = 0; j < categoryCount; j++) {
            qreal temp = m_barSets.at(i)->d_func()->m_values.at(j).x();
            if (temp > max)
                max = temp;
        }
    }

    return max;
}

qreal QBarSeriesPrivate::categoryTop(qsizetype category)
{
    // Returns top (sum of all positive values) of category.
    // Returns 0, if all values are negative
    qreal top(0);
    qsizetype count = m_barSets.size();
    for (qsizetype set = 0; set < count; set++) {
        if (category < m_barSets.at(set)->count()) {
            qreal temp = m_barSets.at(set)->at(category);
            if (temp > 0) {
                top += temp;
            }
        }
    }
    return top;
}

qreal QBarSeriesPrivate::categoryBottom(qsizetype category)
{
    // Returns bottom (sum of all negative values) of category
    // Returns 0, if all values are positive
    qreal bottom(0);
    qsizetype count = m_barSets.size();
    for (qsizetype set = 0; set < count; set++) {
        if (category < m_barSets.at(set)->count()) {
            qreal temp = m_barSets.at(set)->at(category);
            if (temp < 0) {
                bottom += temp;
            }
        }
    }
    return bottom;
}
qreal QBarSeriesPrivate::top()
{
    // Returns top of all categories
    qreal top(0);
    qsizetype count = categoryCount();
    for (qsizetype i = 0; i < count; i++) {
        qreal temp = categoryTop(i);
        if (temp > top)
            top = temp;
    }
    return top;
}

qreal QBarSeriesPrivate::bottom()
{
    // Returns bottom of all categories
    qreal bottom(0);
    qsizetype count = categoryCount();
    for (qsizetype i = 0; i < count; i++) {
        qreal temp = categoryBottom(i);
        if (temp < bottom)
            bottom = temp;
    }
    return bottom;
}

bool QBarSeriesPrivate::blockBarUpdate()
{
    return m_blockBarUpdate;
}

bool QBarSeriesPrivate::append(QBarSet *set)
{
    if ((m_barSets.contains(set)) || (set == 0))
        return false; // Fail if set is already in list or set is null.

    m_barSets.append(set);
    Q_Q(QBarSeries);
    QObject::connect(set, &QBarSet::updatedBars, q, &QBarSeries::updatedBars);
    QObject::connect(set, &QBarSet::valueChanged, q, &QBarSeries::handleSetValueChange);
    QObject::connect(set, &QBarSet::valueAdded, q, &QBarSeries::handleSetValueAdd);
    QObject::connect(set, &QBarSet::valueRemoved, q, &QBarSeries::handleSetValueRemove);
    QObject::connect(set, &QBarSet::selectedBarsChanged, q, &QBarSeries::updatedBars);

    return true;
}

bool QBarSeriesPrivate::remove(QBarSet *set)
{
    if (!m_barSets.contains(set))
        return false; // Fail if set is not in list

    m_barSets.removeOne(set);
    Q_Q(QBarSeries);
    QObject::disconnect(set, &QBarSet::updatedBars, q, &QBarSeries::updatedBars);
    QObject::disconnect(set, &QBarSet::valueChanged, q, &QBarSeries::handleSetValueChange);
    QObject::disconnect(set, &QBarSet::valueAdded, q, &QBarSeries::handleSetValueAdd);
    QObject::disconnect(set, &QBarSet::valueRemoved, q, &QBarSeries::handleSetValueRemove);
    QObject::disconnect(set, &QBarSet::selectedBarsChanged, q, &QBarSeries::updatedBars);

    return true;
}

bool QBarSeriesPrivate::append(const QList<QBarSet *> &sets)
{
    for (auto *set : sets) {
        if ((set == 0) || (m_barSets.contains(set)))
            return false; // Fail if any of the sets is null or is already appended.
        if (sets.count(set) != 1)
            return false; // Also fail if same set is more than once in given list.
    }

    Q_Q(QBarSeries);
    for (auto *set : sets) {
        m_barSets.append(set);
        QObject::connect(set, &QBarSet::updatedBars, q, &QBarSeries::updatedBars);
        QObject::connect(set, &QBarSet::valueChanged, q, &QBarSeries::handleSetValueChange);
        QObject::connect(set, &QBarSet::valueAdded, q, &QBarSeries::handleSetValueAdd);
        QObject::connect(set, &QBarSet::valueRemoved, q, &QBarSeries::handleSetValueRemove);
        QObject::connect(set, &QBarSet::selectedBarsChanged, q, &QBarSeries::updatedBars);
    }

    return true;
}

bool QBarSeriesPrivate::remove(const QList<QBarSet *> &sets)
{
    if (sets.size() == 0)
        return false;

    for (QBarSet *set : sets) {
        if ((set == 0) || (!m_barSets.contains(set)))
            return false; // Fail if any of the sets is null or is not in series
        if (sets.count(set) != 1)
            return false; // Also fail if same set is more than once in given list.
    }

    Q_Q(QBarSeries);
    for (QBarSet *set : sets) {
        m_barSets.removeOne(set);
        QObject::disconnect(set, &QBarSet::updatedBars, q, &QBarSeries::updatedBars);
        QObject::disconnect(set,
                            &QBarSet::valueChanged,
                            q,
                            &QBarSeries::handleSetValueChange);
        QObject::disconnect(set, &QBarSet::valueAdded, q, &QBarSeries::handleSetValueAdd);
        QObject::disconnect(set,
                            &QBarSet::valueRemoved,
                            q,
                            &QBarSeries::handleSetValueRemove);
        QObject::disconnect(set, &QBarSet::selectedBarsChanged, q, &QBarSeries::updatedBars);
    }

    return true;
}

bool QBarSeriesPrivate::insert(qsizetype index, QBarSet *set)
{
    if ((m_barSets.contains(set)) || (set == 0))
        return false; // Fail if set is already in list or set is null.

    m_barSets.insert(index, set);
    Q_Q(QBarSeries);
    QObject::connect(set, &QBarSet::updatedBars, q, &QBarSeries::updatedBars);
    QObject::connect(set, &QBarSet::valueChanged, q, &QBarSeries::handleSetValueChange);
    QObject::connect(set, &QBarSet::valueAdded, q, &QBarSeries::handleSetValueAdd);
    QObject::connect(set, &QBarSet::valueRemoved, q, &QBarSeries::handleSetValueRemove);
    QObject::disconnect(set, &QBarSet::selectedBarsChanged, q, &QBarSeries::updatedBars);

    return true;
}

QT_END_NAMESPACE

#include "moc_qbarseries.cpp"
