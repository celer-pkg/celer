// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qabstract3daxis_p.h"
#include "qscatter3dseries_p.h"
#include "qscatterdataproxy_p.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QScatterDataProxy
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QScatterDataProxy class is the data proxy for 3D scatter graphs.
 *
 * A scatter data proxy handles adding, inserting, changing, and removing data
 * items. Since data is stored in series, it is necessary
 * to create a series associated with the proxy before using these functions for
 * the dataset.
 *
 * QScatterDataProxy takes ownership of all
 * QtGraphs::QScatterDataArray and QScatterDataItem objects passed to
 * it.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \typealias QScatterDataArray
 * \relates QScatterDataProxy
 *
 * A list of \l {QScatterDataItem} objects.
 */

/*!
 * \qmltype ScatterDataProxy
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QScatterDataProxy
 * \inherits AbstractDataProxy
 * \brief The data proxy for 3D scatter graphs.
 *
 * This type handles adding, inserting, changing, and removing data items.
 *
 * This type is uncreatable, but contains properties that are exposed via
 * subtypes.
 *
 * \sa ItemModelScatterDataProxy, {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty int ScatterDataProxy::itemCount
 * The number of items in the array.
 */

/*!
 * \qmlproperty Scatter3DSeries ScatterDataProxy::series
 *
 * The series this proxy is attached to.
 */

/*!
    \qmlsignal ScatterDataProxy::itemCountChanged(int count)

    This signal is emitted when itemCount changes to \a count.
*/

/*!
    \qmlsignal ScatterDataProxy::seriesChanged(Scatter3DSeries series)

    This signal is emitted when \l series changes to \a series.
*/

/*!
 * Constructs QScatterDataProxy with the given \a parent.
 */
QScatterDataProxy::QScatterDataProxy(QObject *parent)
    : QAbstractDataProxy(*(new QScatterDataProxyPrivate()), parent)
{}

/*!
 * \internal
 */
QScatterDataProxy::QScatterDataProxy(QScatterDataProxyPrivate &d, QObject *parent)
    : QAbstractDataProxy(d, parent)
{}

/*!
 * Deletes the scatter data proxy.
 */
QScatterDataProxy::~QScatterDataProxy() {}

/*!
 * \property QScatterDataProxy::series
 *
 * \brief The series this proxy is attached to.
 */
QScatter3DSeries *QScatterDataProxy::series() const
{
    Q_D(const QScatterDataProxy);
    if (!d->series())
        qCWarning(lcGraphs3D, "%s series needs to be created to access data members",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    return static_cast<QScatter3DSeries *>(d->series());
}

/*!
 * Clears the existing array and triggers the arrayReset() signal.
 */
void QScatterDataProxy::resetArray()
{
    series()->clearArray();

    emit arrayReset();
    emit itemCountChanged(itemCount());
}

/*!
 * Sets the array from \a newArray. If the new array is equal to the
 * existing one, this function simply triggers the arrayReset() signal.
 */
void QScatterDataProxy::resetArray(QScatterDataArray newArray)
{
    Q_D(QScatterDataProxy);
    if (!series())
        return;

    if (!series()->dataArray().isSharedWith(newArray))
        d->resetArray(std::move(newArray));

    emit arrayReset();
    emit itemCountChanged(itemCount());
}

/*!
 * Sets the scale array from \a newArray. If the new array is equal to the
 * existing one, this function simply triggers the scaleArrayReset() signal.
 */
void QScatterDataProxy::resetScaleArray(QList<QVector3D> newArray)
{
    Q_D(QScatterDataProxy);
    if (!series())
        return;

    if (!series()->scaleArray().isSharedWith(newArray))
        d->resetScaleArray(std::move(newArray));

    emit scaleArrayReset();
}

/*!
 * Replaces the item at the position \a index with the item \a item.
 */
void QScatterDataProxy::setItem(qsizetype index, QScatterDataItem item)
{
    Q_D(QScatterDataProxy);
    d->setItem(index, std::move(item));
    emit itemsChanged(index, 1);
}

/*!
 * Replaces the items starting from the position \a index with the items
 * specified by \a items.
 */
void QScatterDataProxy::setItems(qsizetype index, QScatterDataArray items)
{
    Q_D(QScatterDataProxy);
    d->setItems(index, std::move(items));
    emit itemsChanged(index, items.size());
}

/*!
 * Adds the item \a item to the end of the array.
 *
 * Returns the index of the added item.
 */
qsizetype QScatterDataProxy::addItem(QScatterDataItem item)
{
    Q_D(QScatterDataProxy);
    qsizetype addIndex = d->addItem(std::move(item));
    emit itemsAdded(addIndex, 1);
    emit itemCountChanged(itemCount());
    return addIndex;
}

/*!
 * Adds the items specified by \a items to the end of the array.
 *
 * Returns the index of the first added item.
 */
qsizetype QScatterDataProxy::addItems(QScatterDataArray items)
{
    Q_D(QScatterDataProxy);
    qsizetype addIndex = d->addItems(std::move(items));
    emit itemsAdded(addIndex, items.size());
    emit itemCountChanged(itemCount());
    return addIndex;
}

/*!
 * Inserts the item \a item to the position \a index. If the index is equal to
 * the data array size, the item is added to the array.
 */
void QScatterDataProxy::insertItem(qsizetype index, QScatterDataItem item)
{
    Q_D(QScatterDataProxy);
    d->insertItem(index, std::move(item));
    emit itemsInserted(index, 1);
    emit itemCountChanged(itemCount());
}

/*!
 * Inserts the items specified by \a items to the position \a index. If the
 * index is equal to data array size, the items are added to the array.
 */
void QScatterDataProxy::insertItems(qsizetype index, QScatterDataArray items)
{
    Q_D(QScatterDataProxy);
    d->insertItems(index, std::move(items));
    emit itemsInserted(index, items.size());
    emit itemCountChanged(itemCount());
}

/*!
 * Removes the number of items specified by \a removeCount starting at the
 * position \a index. Attempting to remove items past the end of
 * the array does nothing.
 */
void QScatterDataProxy::removeItems(qsizetype index, qsizetype removeCount)
{
    if (index >= series()->dataArray().size())
        return;

    Q_D(QScatterDataProxy);
    d->removeItems(index, removeCount);
    emit itemsRemoved(index, removeCount);
    emit itemCountChanged(itemCount());
}

/*!
 * \property QScatterDataProxy::itemCount
 *
 * \brief The number of items in the array.
 */
qsizetype QScatterDataProxy::itemCount() const
{
    if (series())
        return series()->dataArray().size();
    else
        return 0;
}

/*!
 * Returns the pointer to the item at the index \a index. It is guaranteed to be
 * valid only until the next call that modifies data.
 */
const QScatterDataItem &QScatterDataProxy::itemAt(qsizetype index) const
{
    return series()->dataArray().at(index);
}

/*!
 * Returns the scale data at the index \a index. It is guaranteed to be
 * valid only until the next call that modifies data.
 */
QVector3D QScatterDataProxy::scaleAt(qsizetype index) const
{
    QVector3D ret(1.0f, 1.0f, 1.0f);
    if (series()->scaleArray().isEmpty())
        return ret;
    if (series()->scaleArray().size() <= index) {
        qCWarning(lcGraphs3D, "Scale data size %" PRIdQSIZETYPE " does not match the access index %" PRIdQSIZETYPE
            ". The default scale value will be returned.", series()->scaleArray().size(), index);
        return ret;
    }
    return series()->scaleArray().at(index);
}

/*!
 * \fn void QScatterDataProxy::arrayReset()
 *
 * This signal is emitted when the data array is reset.
 * If the contents of the whole array are changed without calling resetArray(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QScatterDataProxy::itemsAdded(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of items specified by \a count are
 * added, starting at the position \a startIndex.
 * If items are added to the array without calling addItem() or addItems(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QScatterDataProxy::itemsChanged(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of items specified by \a count are
 * changed, starting at the position \a startIndex.
 * If items are changed in the array without calling setItem() or setItems(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QScatterDataProxy::itemsRemoved(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of rows specified by \a count are
 * removed, starting at the position \a startIndex.
 * The index may be larger than the current array size if items are removed from
 * the end. If items are removed from the array without calling removeItems(),
 * this signal needs to be emitted to update the graph.
 */

/*!
 * \fn void QScatterDataProxy::itemsInserted(qsizetype startIndex, qsizetype count)
 *
 * This signal is emitted when the number of items specified by \a count are
 * inserted, starting at the position \a startIndex.
 * If items are inserted into the array without calling insertItem() or
 * insertItems(), this signal needs to be emitted to update the graph.
 */

// QScatterDataProxyPrivate

QScatterDataProxyPrivate::QScatterDataProxyPrivate()
    : QAbstractDataProxyPrivate(QAbstractDataProxy::DataType::Scatter)
{}

QScatterDataProxyPrivate::~QScatterDataProxyPrivate() {}

void QScatterDataProxyPrivate::resetArray(QScatterDataArray &&newArray)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    if (!newArray.isSharedWith(scatterSeries->dataArray()))
        scatterSeries->setDataArray(newArray);
}

void QScatterDataProxyPrivate::resetScaleArray(QList<QVector3D> &&newArray)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    if (!newArray.isSharedWith(scatterSeries->scaleArray()))
        scatterSeries->setScaleArray(newArray);
}

void QScatterDataProxyPrivate::setItem(qsizetype index, QScatterDataItem &&item)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    Q_ASSERT(index >= 0 && index < scatterSeries->dataArray().size());
    QScatterDataArray array = scatterSeries->dataArray();
    array[index] = item;
    scatterSeries->setDataArray(array);
}

void QScatterDataProxyPrivate::setItems(qsizetype index, QScatterDataArray &&items)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    Q_ASSERT(index >= 0 && (index + items.size()) <= scatterSeries->dataArray().size());
    QScatterDataArray array = scatterSeries->dataArray();
    for (int i = 0; i < items.size(); i++)
        array[index++] = items[i];
    scatterSeries->setDataArray(array);
}

qsizetype QScatterDataProxyPrivate::addItem(QScatterDataItem &&item)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    qsizetype currentSize = scatterSeries->dataArray().size();
    QScatterDataArray array = scatterSeries->dataArray();
    array.append(item);
    scatterSeries->setDataArray(array);
    return currentSize;
}

qsizetype QScatterDataProxyPrivate::addItems(QScatterDataArray &&items)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    qsizetype currentSize = 0;
    if (scatterSeries) {
        currentSize = scatterSeries->dataArray().size();
        QScatterDataArray array = scatterSeries->dataArray();
        array += items;
        scatterSeries->setDataArray(array);
    }
    return currentSize;
}

void QScatterDataProxyPrivate::insertItem(qsizetype index, QScatterDataItem &&item)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    Q_ASSERT(index >= 0 && index <= scatterSeries->dataArray().size());
    QScatterDataArray array = scatterSeries->dataArray();
    array.insert(index, item);
    scatterSeries->setDataArray(array);
}

void QScatterDataProxyPrivate::insertItems(qsizetype index, QScatterDataArray &&items)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    Q_ASSERT(index >= 0 && index <= scatterSeries->dataArray().size());
    QScatterDataArray array = scatterSeries->dataArray();
    for (int i = 0; i < items.size(); i++)
        array.insert(index++, items.at(i));
    scatterSeries->setDataArray(array);
}

void QScatterDataProxyPrivate::removeItems(qsizetype index, qsizetype removeCount)
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    Q_ASSERT(index >= 0);
    qsizetype maxRemoveCount = scatterSeries->dataArray().size() - index;
    removeCount = qMin(removeCount, maxRemoveCount);
    QScatterDataArray array = scatterSeries->dataArray();
    array.remove(index, removeCount);
    scatterSeries->setDataArray(array);
}

void QScatterDataProxyPrivate::limitValues(QVector3D &minValues,
                                           QVector3D &maxValues,
                                           QAbstract3DAxis *axisX,
                                           QAbstract3DAxis *axisY,
                                           QAbstract3DAxis *axisZ) const
{
    auto *scatterSeries = static_cast<QScatter3DSeries *>(series());
    if (scatterSeries->dataArray().isEmpty())
        return;

    QVector3D firstPos = scatterSeries->dataArray().at(0).position();

    float minX = firstPos.x();
    float maxX = minX;
    float minY = firstPos.y();
    float maxY = minY;
    float minZ = firstPos.z();
    float maxZ = minZ;

    if (scatterSeries->dataArray().size() > 1) {
        for (int i = 1; i < scatterSeries->dataArray().size(); i++) {
            QVector3D pos = scatterSeries->dataArray().at(i).position();

            float value = pos.x();
            if (qIsNaN(value) || qIsInf(value))
                continue;
            if (isValidValue(minX, value, axisX))
                minX = value;
            if (maxX < value)
                maxX = value;

            value = pos.y();
            if (qIsNaN(value) || qIsInf(value))
                continue;
            if (isValidValue(minY, value, axisY))
                minY = value;
            if (maxY < value)
                maxY = value;

            value = pos.z();
            if (qIsNaN(value) || qIsInf(value))
                continue;
            if (isValidValue(minZ, value, axisZ))
                minZ = value;
            if (maxZ < value)
                maxZ = value;
        }
    }

    minValues.setX(minX);
    minValues.setY(minY);
    minValues.setZ(minZ);

    maxValues.setX(maxX);
    maxValues.setY(maxY);
    maxValues.setZ(maxZ);
}

bool QScatterDataProxyPrivate::isValidValue(float axisValue,
                                            float value,
                                            QAbstract3DAxis *axis) const
{
    return (axisValue > value
            && (value > 0.0f || (value == 0.0f && axis->d_func()->allowZero())
                || (value < 0.0f && axis->d_func()->allowNegatives())));
}

void QScatterDataProxyPrivate::setSeries(QAbstract3DSeries *series)
{
    Q_Q(QScatterDataProxy);
    QAbstractDataProxyPrivate::setSeries(series);
    QScatter3DSeries *scatterSeries = static_cast<QScatter3DSeries *>(series);
    emit q->seriesChanged(scatterSeries);
}

QT_END_NAMESPACE

#include "moc_qscatterdataproxy.cpp"
