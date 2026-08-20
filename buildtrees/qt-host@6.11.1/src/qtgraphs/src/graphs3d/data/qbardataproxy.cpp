// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qbar3dseries_p.h"
#include "qbardataproxy_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QBarDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QBarDataProxy class is the data proxy for a 3D bars graph.
 *
 * A bar data proxy handles adding, inserting, changing, and removing rows of
 * data.
 *
 * The data array is a list of vectors (rows) of QBarDataItem instances.
 * Each row can contain a different number of items or even be null.
 *
 * QBarDataProxy takes ownership of all QtGraphs::QBarDataRow objects
 * passed to it, whether directly or in a QtGraphs::QBarDataArray container.
 * If bar data row pointers are used to directly modify data after adding the
 * array to the proxy, the appropriate signal must be emitted to update the
 * graph.
 *
 * QBarDataProxy optionally keeps track of row and column labels, which
 * QCategory3DAxis can utilize to show axis labels.
 *
 * The row and column labels are stored in a separate array from the data in the
 * series rather. Row processing methods are available in the proxy and provide
 * alternative versions that do not affect row labels. This enables
 * the option of having row labels that relate to the position of the data in
 * the array rather than the data itself. Since the series holds the data and
 * row and column labels, it is necessary to create a series associated with the
 * proxy before using these functions for them.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \typealias QBarDataRow
 * \relates QBarDataProxy
 *
 * A list of \l {QBarDataItem} objects.
 */

/*!
 * \typealias QBarDataArray
 * \relates QBarDataProxy
 *
 * A list of pointers to \l {QBarDataRow} objects.
 */

/*!
 * \qmltype BarDataProxy
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QBarDataProxy
 * \inherits AbstractDataProxy
 * \brief The data proxy for a 3D bars graph.
 *
 * This type handles adding, inserting, changing, and removing rows of data with
 * Qt Quick 2.
 *
 * This type cannot be instantiated, but contains properties that are exposed via
 * subtypes.
 *
 * For a more complete description, see QBarDataProxy.
 *
 * \sa ItemModelBarDataProxy, {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty int BarDataProxy::rowCount
 * The number of rows in the array.
 */

/*!
 * \qmlproperty int BarDataProxy::colCount
 * The number of columns in the array.
 */

/*!
 * \qmlproperty Bar3DSeries BarDataProxy::series
 *
 * The series this proxy is attached to.
 */

/*!
    \qmlsignal BarDataProxy::itemChanged(int rowIndex, int columnIndex)

    This signal is emitted when the item at the position specified by \a rowIndex
    and \a columnIndex changes.
    If the item is changed in the array without calling setItem(),
    this signal needs to be emitted to update the graph.
*/

/*!
    \qmlsignal BarDataProxy::rowCountChanged(int count)

    This signal is emitted when rowCount changes to \a count.
*/

/*!
    \qmlsignal BarDataProxy::colCountChanged(int count)

    This signal is emitted when colCount changes to \a count.
*/

/*!
    \qmlsignal BarDataProxy::seriesChanged(Bar3DSeries series)

    This signal is emitted when \l series changes to \a series.
*/

/*!
 * Constructs a bar data proxy with the given \a parent.
 */
QBarDataProxy::QBarDataProxy(QObject *parent)
    : QAbstractDataProxy(*(new QBarDataProxyPrivate()), parent)
{}

/*!
 * \internal
 */
QBarDataProxy::QBarDataProxy(QBarDataProxyPrivate &d, QObject *parent)
    : QAbstractDataProxy(d, parent)
{}

/*!
 * Deletes the bar data proxy.
 */
QBarDataProxy::~QBarDataProxy() {}

/*!
 * \property QBarDataProxy::series
 *
 * \brief The series this proxy is attached to.
 */
QBar3DSeries *QBarDataProxy::series() const
{
    Q_D(const QBarDataProxy);
    if (!d->series())
        qCWarning(lcGraphs3D, "series needs to be created to access data members");
    return static_cast<QBar3DSeries *>(d->series());
}

/*!
 * Clears the existing array and row and column labels.
 */
void QBarDataProxy::resetArray()
{
    Q_D(QBarDataProxy);
    d->resetArray(QBarDataArray(), QStringList(), QStringList());
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Takes ownership of the array \a newArray. Clears the existing array if the
 * new array differs from it. If the arrays are the same, this function
 * just triggers the arrayReset() signal.
 *
 * Passing a null array deletes the old array and creates a new empty array.
 * Row and column labels are not affected.
 */
void QBarDataProxy::resetArray(QBarDataArray newArray)
{
    Q_D(QBarDataProxy);
    if (!series()) {
        qCWarning(lcGraphs3D, "Series needs to be valid before using resetArray");
        return;
    }

    d->resetArray(std::move(newArray), QStringList(), QStringList());
    emit arrayReset();
    if (rowCount() && colCount()) {
        emit rowCountChanged(rowCount());
        emit colCountChanged(colCount());
    }
}

/*!
 * Takes ownership of the array \a newArray. Clears the existing array if the
 * new array differs from it. If the arrays are the same, this function
 * just triggers the arrayReset() signal.
 *
 * Passing a null array deletes the old array and creates a new empty array.
 *
 * The \a rowLabels and \a columnLabels lists specify the new labels for rows
 * and columns.
 */
void QBarDataProxy::resetArray(QBarDataArray newArray,
                               QStringList rowLabels,
                               QStringList columnLabels)
{
    Q_D(QBarDataProxy);
    if (!series())
        return;

    d->resetArray(std::move(newArray), std::move(rowLabels), std::move(columnLabels));
    emit arrayReset();
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Changes an existing row by replacing the row at the position \a rowIndex
 * with the new row specified by \a row. The new row can be
 * the same as the existing row already stored at \a rowIndex.
 * Existing row labels are not affected.
 */
void QBarDataProxy::setRow(qsizetype rowIndex, QBarDataRow row)
{
    Q_D(QBarDataProxy);
    d->setRow(rowIndex, std::move(row), QString());
    emit rowsChanged(rowIndex, 1);
}

/*!
 * Changes an existing row by replacing the row at the position \a rowIndex
 * with the new row specified by \a row. The new row can be
 * the same as the existing row already stored at \a rowIndex.
 * Changes the row label to \a label.
 */
void QBarDataProxy::setRow(qsizetype rowIndex, QBarDataRow row, QString label)
{
    Q_D(QBarDataProxy);
    d->setRow(rowIndex, std::move(row), std::move(label));
    emit rowsChanged(rowIndex, 1);
}

/*!
 * Changes existing rows by replacing the rows starting at the position
 * \a rowIndex with the new rows specifies by \a rows.
 * Existing row labels are not affected. The rows in the \a rows array can be
 * the same as the existing rows already stored at \a rowIndex.
 */
void QBarDataProxy::setRows(qsizetype rowIndex, QBarDataArray rows)
{
    Q_D(QBarDataProxy);
    d->setRows(rowIndex, std::move(rows), QStringList());
    emit rowsChanged(rowIndex, rows.size());
}

/*!
 * Changes existing rows by replacing the rows starting at the position
 * \a rowIndex with the new rows specifies by \a rows.
 * The row labels are changed to \a labels. The rows in the \a rows array can be
 * the same as the existing rows already stored at \a rowIndex.
 */
void QBarDataProxy::setRows(qsizetype rowIndex, QBarDataArray rows, QStringList labels)
{
    Q_D(QBarDataProxy);
    d->setRows(rowIndex, std::move(rows), std::move(labels));
    emit rowsChanged(rowIndex, rows.size());
}

/*!
 * Changes a single item at the position specified by \a rowIndex and
 * \a columnIndex to the item \a item.
 */
void QBarDataProxy::setItem(qsizetype rowIndex, qsizetype columnIndex, QBarDataItem item)
{
    Q_D(QBarDataProxy);
    d->setItem(rowIndex, columnIndex, std::move(item));
    emit itemChanged(rowIndex, columnIndex);
}

/*!
 * Changes a single item at the position \a position to the item \a item.
 * The x-value of \a position indicates the row and the y-value indicates the
 * column.
 */
void QBarDataProxy::setItem(QPoint position, QBarDataItem item)
{
    setItem(position.x(), position.y(), item);
}

/*!
 * Adds the new row \a row to the end of an array.
 * Existing row labels are not affected.
 *
 * Returns the index of the added row.
 */
qsizetype QBarDataProxy::addRow(QBarDataRow row)
{
    Q_D(QBarDataProxy);
    qsizetype addIndex = d->addRow(std::move(row), QString());
    emit rowsAdded(addIndex, 1);
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
    return addIndex;
}

/*!
 * Adds a the new row \a row with the label \a label to the end of an array.
 *
 * Returns the index of the added row.
 */
qsizetype QBarDataProxy::addRow(QBarDataRow row, QString label)
{
    Q_D(QBarDataProxy);
    qsizetype addIndex = d->addRow(std::move(row), std::move(label));
    emit rowsAdded(addIndex, 1);
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
    return addIndex;
}

/*!
 * Adds the new \a rows to the end of an array.
 * Existing row labels are not affected.
 *
 * Returns the index of the first added row.
 */
qsizetype QBarDataProxy::addRows(QBarDataArray rows)
{
    Q_D(QBarDataProxy);
    qsizetype addIndex = d->addRows(std::move(rows), QStringList());
    emit rowsAdded(addIndex, rows.size());
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
    return addIndex;
}

/*!
 * Adds the new \a rows with \a labels to the end of the array.
 *
 * Returns the index of the first added row.
 */
qsizetype QBarDataProxy::addRows(QBarDataArray rows, QStringList labels)
{
    Q_D(QBarDataProxy);
    qsizetype addIndex = d->addRows(std::move(rows), std::move(labels));
    emit rowsAdded(addIndex, rows.size());
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
    return addIndex;
}

/*!
 * Inserts the new row \a row into \a rowIndex.
 * If \a rowIndex is equal to the array size, the rows are added to the end of
 * the array.
 * The existing row labels are not affected.
 * \note The row labels array will be out of sync with the row array after this
 * call if there were labeled rows beyond the inserted row.
 */
void QBarDataProxy::insertRow(qsizetype rowIndex, QBarDataRow row)
{
    Q_D(QBarDataProxy);
    d->insertRow(rowIndex, std::move(row), QString());
    emit rowsInserted(rowIndex, 1);
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Inserts the new row \a row with the label \a label into \a rowIndex.
 * If \a rowIndex is equal to array size, rows are added to the end of the
 * array.
 */
void QBarDataProxy::insertRow(qsizetype rowIndex, QBarDataRow row, QString label)
{
    Q_D(QBarDataProxy);
    d->insertRow(rowIndex, std::move(row), std::move(label));
    emit rowsInserted(rowIndex, 1);
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Inserts new \a rows into \a rowIndex.
 * If \a rowIndex is equal to the array size, the rows are added to the end of
 * the array. The existing row labels are not affected.
 * \note The row labels array will be out of sync with the row array after this
 * call if there were labeled rows beyond the inserted rows.
 */
void QBarDataProxy::insertRows(qsizetype rowIndex, QBarDataArray rows)
{
    Q_D(QBarDataProxy);
    d->insertRows(rowIndex, std::move(rows), QStringList());
    emit rowsInserted(rowIndex, rows.size());
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Inserts new \a rows with \a labels into \a rowIndex.
 * If \a rowIndex is equal to the array size, the rows are added to the end of
 * the array.
 */
void QBarDataProxy::insertRows(qsizetype rowIndex, QBarDataArray rows, QStringList labels)
{
    Q_D(QBarDataProxy);
    d->insertRows(rowIndex, std::move(rows), std::move(labels));
    emit rowsInserted(rowIndex, rows.size());
    emit rowCountChanged(rowCount());
    emit colCountChanged(colCount());
}

/*!
 * Removes the number of rows specified by \a removeCount starting at the
 * position \a rowIndex. Attempting to remove rows past the end of the
 * array does nothing. If \a removeLabels is \c true, the corresponding row
 * labels are also removed. Otherwise, the row labels are not affected.
 * \note If \a removeLabels is \c false, the row labels array will be out of
 * sync with the row array if there are labeled rows beyond the removed rows.
 */
void QBarDataProxy::removeRows(qsizetype rowIndex, qsizetype removeCount, RemoveLabels removeLabels)
{
    Q_D(QBarDataProxy);
    if (rowIndex < rowCount() && removeCount >= 1) {
        d->removeRows(rowIndex, removeCount, removeLabels == RemoveLabels::No ? false : true);
        emit rowsRemoved(rowIndex, removeCount);
        emit rowCountChanged(rowCount());
        emit colCountChanged(colCount());
    }
}

/*!
 * \property QBarDataProxy::colCount
 *
 * \brief The number of columns in the array.
 */
qsizetype QBarDataProxy::colCount() const
{
    if (this->series() && this->series()->dataArray().size() > 0)
        return this->series()->dataArray().at(0).size();
    else
        return 0;
}

/*!
 * \property QBarDataProxy::rowCount
 *
 * \brief The number of rows in the array.
 */
qsizetype QBarDataProxy::rowCount() const
{
    if (this->series())
        return this->series()->dataArray().size();
    else
        return 0;
}

/*!
 * Returns the reference to the row at the position \a rowIndex. It is
 * guaranteed to be valid only until the next call that modifies data.
 */
const QBarDataRow &QBarDataProxy::rowAt(qsizetype rowIndex) const
{
    const QBarDataArray &dataArray = this->series()->dataArray();
    Q_ASSERT(rowIndex >= 0 && rowIndex < dataArray.size());
    return dataArray[rowIndex];
}

/*!
 * Returns the reference to the item at the position specified by \a rowIndex
 * and \a columnIndex. It is guaranteed to be valid only until the next call
 * that modifies data.
 */
const QBarDataItem &QBarDataProxy::itemAt(qsizetype rowIndex, qsizetype columnIndex) const
{
    const QBarDataArray &dataArray = this->series()->dataArray();
    Q_ASSERT(rowIndex >= 0 && rowIndex < dataArray.size());
    const QBarDataRow &dataRow = dataArray[rowIndex];
    Q_ASSERT(columnIndex >= 0 && columnIndex < dataRow.size());
    return dataRow.at(columnIndex);
}

/*!
 * Returns the reference to the item at the position \a position. The x-value of
 * \a position indicates the row and the y-value indicates the column. The item
 * is guaranteed to be valid only until the next call that modifies data.
 */
const QBarDataItem &QBarDataProxy::itemAt(QPoint position) const
{
    return itemAt(position.x(), position.y());
}

/*!
 * \fn void QBarDataProxy::arrayReset()
 *
 * This signal is emitted when the data array is reset.
 * If the contents of the whole array are changed without calling resetArray(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QBarDataProxy::rowsAdded(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of rows specified by \a count is
 * added, starting at the position \a startIndex.
 * If rows are added to the array without calling addRow() or addRows(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QBarDataProxy::rowsChanged(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of rows specified by \a count is
 * changed, starting at the position \a startIndex.
 * If rows are changed in the array without calling setRow() or setRows(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QBarDataProxy::rowsRemoved(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of rows specified by \a count is
 * removed, starting at the position \a startIndex.
 *
 * The index is the current array size if the rows were removed from the end of
 * the array. If rows are removed from the array without calling removeRows(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QBarDataProxy::rowsInserted(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of rows specified by \a count is
 * inserted at the position \a startIndex.
 *
 * If rows are inserted into the array without calling insertRow() or
 * insertRows(), this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QBarDataProxy::itemChanged(qsizetype rowIndex, qsizetype columnIndex)
 *
 * This signal is emitted when the item at the position specified by \a rowIndex
 * and \a columnIndex changes.
 * If the item is changed in the array without calling setItem(),
 * this signal needs to be emitted to update the graph.
 */

// QBarDataProxyPrivate

QBarDataProxyPrivate::QBarDataProxyPrivate()
    : QAbstractDataProxyPrivate(QAbstractDataProxy::DataType::Bar)
{}

QBarDataProxyPrivate::~QBarDataProxyPrivate() {}

void QBarDataProxyPrivate::resetArray(QBarDataArray &&newArray,
                                      QStringList &&rowLabels,
                                      QStringList &&columnLabels)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    barSeries->setRowLabels(rowLabels);
    barSeries->setColumnLabels(columnLabels);

    if (newArray.data() != barSeries->dataArray().data()) {
        barSeries->clearArray();
        barSeries->setDataArray(newArray);
    }
}

void QBarDataProxyPrivate::setRow(qsizetype rowIndex, QBarDataRow &&row, QString &&label)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0 && rowIndex < barSeries->dataArray().size());

    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(rowIndex, 1, QStringList(label), false);
    if (row.data() != barSeries->dataArray().at(rowIndex).data()) {
        barSeries->clearRow(rowIndex);
        QBarDataArray array = barSeries->dataArray();
        array[rowIndex] = row;
        barSeries->setDataArray(array);
    }
}

void QBarDataProxyPrivate::setRows(qsizetype rowIndex, QBarDataArray &&rows, QStringList &&labels)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0 && (rowIndex + rows.size()) <= barSeries->dataArray().size());

    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(rowIndex, rows.size(), labels, false);
    for (int i = 0; i < rows.size(); i++) {
        if (rows.at(i).data() != barSeries->dataArray().at(rowIndex).data()) {
            barSeries->clearRow(rowIndex);
            QBarDataArray array = barSeries->dataArray();
            array[rowIndex] = rows.at(i);
            barSeries->setDataArray(array);
        }
        rowIndex++;
    }
}

void QBarDataProxyPrivate::setItem(qsizetype rowIndex, qsizetype columnIndex, QBarDataItem &&item)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0 && rowIndex < barSeries->dataArray().size());
    QBarDataArray array = barSeries->dataArray();
    QBarDataRow &row = array[rowIndex];
    Q_ASSERT(columnIndex < row.size());
    row[columnIndex] = item;
    barSeries->setDataArray(array);
}

qsizetype QBarDataProxyPrivate::addRow(QBarDataRow &&row, QString &&label)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    qsizetype currentSize = barSeries->dataArray().size();
    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(currentSize, 1, QStringList(label), false);
    QBarDataArray array = barSeries->dataArray();
    array.append(row);
    barSeries->setDataArray(array);
    return currentSize;
}

qsizetype QBarDataProxyPrivate::addRows(QBarDataArray &&rows, QStringList &&labels)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    QBarDataArray array = barSeries->dataArray();
    qsizetype currentSize = array.size();
    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(currentSize, rows.size(), labels, false);
    for (int i = 0; i < rows.size(); i++)
        array.append(rows.at(i));
    barSeries->setDataArray(array);
    return currentSize;
}

void QBarDataProxyPrivate::insertRow(qsizetype rowIndex, QBarDataRow &&row, QString &&label)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0 && rowIndex <= barSeries->dataArray().size());
    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(rowIndex, 1, QStringList(label), true);
    QBarDataArray array = barSeries->dataArray();
    array.insert(rowIndex, row);
    barSeries->setDataArray(array);
}

void QBarDataProxyPrivate::insertRows(qsizetype rowIndex, QBarDataArray &&rows, QStringList &&labels)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0 && rowIndex <= barSeries->dataArray().size());
    QBarDataArray array = barSeries->dataArray();

    QBar3DSeriesPrivate::get(barSeries)->fixRowLabels(rowIndex, rows.size(), labels, true);
    for (int i = 0; i < rows.size(); i++)
        array.insert(rowIndex++, rows.at(i));
    barSeries->setDataArray(array);
}

void QBarDataProxyPrivate::removeRows(qsizetype rowIndex, qsizetype removeCount, bool removeLabels)
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    Q_ASSERT(rowIndex >= 0);
    qsizetype maxRemoveCount = barSeries->dataArray().size() - rowIndex;
    removeCount = qMin(removeCount, maxRemoveCount);
    bool labelsChanged = false;
    QBarDataArray array = barSeries->dataArray();
    for (int i = 0; i < removeCount; i++) {
        barSeries->clearRow(rowIndex);
        array.removeAt(rowIndex);
        if (removeLabels && barSeries->rowLabels().size() > rowIndex) {
            auto rowLabels = barSeries->rowLabels();
            rowLabels.removeAt(rowIndex);
            barSeries->setRowLabels(rowLabels);
            labelsChanged = true;
        }
    }
    barSeries->setDataArray(array);
    if (labelsChanged)
        emit barSeries->rowLabelsChanged();
}

QPair<float, float> QBarDataProxyPrivate::limitValues(qsizetype startRow,
                                                      qsizetype endRow,
                                                      qsizetype startColumn,
                                                      qsizetype endColumn) const
{
    auto *barSeries = static_cast<QBar3DSeries *>(series());
    QPair<float, float> limits = qMakePair(0.0f, 0.0f);
    endRow = qMin(endRow, barSeries->dataArray().size() - 1);
    for (qsizetype i = startRow; i <= endRow; i++) {
        QBarDataRow row = barSeries->dataArray().at(i);
        qsizetype lastColumn = qMin(endColumn, row.size() - 1);
        for (qsizetype j = startColumn; j <= lastColumn; j++) {
            const QBarDataItem &item = row.at(j);
            float itemValue = item.value();
            if (limits.second < itemValue)
                limits.second = itemValue;
            if (limits.first > itemValue)
                limits.first = itemValue;
        }
    }
    return limits;
}

void QBarDataProxyPrivate::setSeries(QAbstract3DSeries *series)
{
    Q_Q(QBarDataProxy);
    QAbstractDataProxyPrivate::setSeries(series);
    QBar3DSeries *barSeries = static_cast<QBar3DSeries *>(series);
    emit q->seriesChanged(barSeries);
}

QT_END_NAMESPACE

#include "moc_qbardataproxy.cpp"
