// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCore/qabstractitemmodel.h>
#include <QtGraphs/qpiemodelmapper.h>
#include <QtGraphs/qpieseries.h>
#include <private/qpiemodelmapper_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
               "class QPieModelMapper;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperInitPieFromModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperInitPieFromModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperRemoveData_entry, int start, int end);
Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperRemoveData_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperInsertData_entry, int start, int end);
Q_TRACE_POINT(qtgraphs, QGraphs2DPieModelMapperInsertData_exit);

/*!
    \class QPieModelMapper
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QPieModelMapper is a model mapper for pie series.

    Model mappers enable using a data model derived from the QAbstractItemModel
    class as a data source for a graph. A model mapper is used to create
    a connection between a data model and QPieSeries.

    Both model and pie series properties can be used to manipulate the data. The
    model mapper keeps the pie series and the data model in sync.
*/
/*!
    \qmltype PieModelMapper
    \nativetype QPieModelMapper
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D

    \brief Model mapper for pie series.

    Model mappers enable using a data model derived from the QAbstractItemModel
    class as a data source for a graph. A model mapper is used to create
    a connection between a data model and PieSeries.

    Both model and pie series properties can be used to manipulate the data. The
    model mapper keeps the pie series and the data model in sync.

    The following QML example creates a pie series with four slices (assuming
    the model has at least five rows). Each slice gets a label from column 1 and
    a value from column 2.
    \code
        PieModelMapper {
            series: pieSeries
            model: customModel
            labelsSection: 1
            valuesSection: 2
            first: 1
            count: 4
            orientation: Qt.Vertical
        }
    \endcode
*/

/*!
    \property QPieModelMapper::series
    \brief The pie series that is used by the mapper.

    All the data in the series is discarded when it is set to the mapper.
    When a new series is specified, the old series is disconnected (but it preserves its data).
*/
/*!
    \qmlproperty PieSeries PieModelMapper::series
    The pie series that is used by the mapper. If you define the mapper element as a child for a
    PieSeries, leave this property undefined. All the data in the series is discarded when it is set to the mapper.
    When new series is specified the old series is disconnected (but it preserves its data).
*/

/*!
    \property QPieModelMapper::model
    \brief The model that is used by the mapper.
*/
/*!
    \qmlproperty model PieModelMapper::model
    The QAbstractItemModel based model that is used by the mapper. You need to implement the model
    and expose it to QML.

    \note The model has to support adding and removing rows or columns and modifying
    the data in the cells.
*/

/*!
    \property QPieModelMapper::valuesSection
    \brief The column or row of the model that is kept in sync with the values of the pie's slices.

    The default value is -1 (invalid mapping).

    \sa QPieModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype PieModelMapper::valuesSection
    The column or row of the model that is kept in sync with the values of the pie's slices.
    The default value is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QPieModelMapper::labelsSection
    \brief The column or row of the model that is kept in sync with the labels of the pie's slices.

    The default value is -1 (invalid mapping).

    \sa QPieModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype PieModelMapper::labelsSection
    The column or row of the model that is kept in sync with the labels of the pie's slices.
    The default value is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QPieModelMapper::first
    \brief The column or row of the model that contains the first slice value.

    The minimum and default value is 0.

    \sa QPieModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype PieModelMapper::first
    The column or row of the model that contains the first slice value.
    The default value is 0.

    \sa orientation
*/

/*!
    \property QPieModelMapper::count
    \brief The number of columns or rows of the model that are mapped as the data for a pie series.

    The minimum and default value is -1 (number limited by the number of rows in the model).

    \sa QPieModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype PieModelMapper::count
    The number of columns or rows of the model that are mapped as the data for a pie series.
    The default value is -1 (number limited by the number of rows in the model).

    \sa orientation
*/

/*!
    \property QPieModelMapper::orientation
    \brief Tells the modelmapper how to map data from a model. If
    \c{Qt::Vertical} is used, each of the model's rows defines a pie slice, and the
    model's columns define the label or the value of the pie slice. When the orientation is set to
    \c{Qt::Horizontal}, each of the model's columns defines a pie slice, and the model's
    rows define the label or the value of the pie slice.

    The default value is \c{Qt::Vertical}
*/
/*!
    \qmlproperty  orientation PieModelMapper::orientation
    Tells the modelmapper how to map data from a model. If
    \c{Qt.Vertical} is used, each of the model's rows defines a pie slice, and the
    model's columns define the label or the value of the pie slice. When the orientation is set to
    \c{Qt.Horizontal}, each of the model's columns defines a pie slice, and the model's
    rows define the label or the value of the pie slice.

    The default value is \c{Qt.Vertical}
*/

/*!
    \qmlsignal PieModelMapper::seriesChanged()

    This signal is emitted when the series that the mapper is connected to changes.
*/

/*!
    \qmlsignal PieModelMapper::modelChanged()

    This signal is emitted when the model that the mapper is connected to changes.
*/

/*!
    \qmlsignal PieModelMapper::valuesSectionChanged()

    This signal is emitted when the values section changes.
*/

/*!
    \qmlsignal PieModelMapper::labelsSectionChanged()

    This signal is emitted when the labels section changes.
*/

/*!
    \qmlsignal PieModelMapper::firstChanged()
    This signal is emitted when the first slice changes.
*/

/*!
    \qmlsignal PieModelMapper::countChanged()
    This signal is emitted when the count changes.
*/

/*!
    \qmlsignal PieModelMapper::orientationChanged()
    This signal is emitted when the orientation changes.
*/

QPieModelMapper::~QPieModelMapper() {}

QPieModelMapper::QPieModelMapper(QObject *parent)
    : QObject{*(new QPieModelMapperPrivate), parent} {}

QPieModelMapper::QPieModelMapper(QPieModelMapperPrivate &dd, QObject *parent)
    : QObject(dd, parent) {}

void QPieModelMapper::onSliceLabelChanged() {
    Q_D(QPieModelMapper);
    d->onSliceLabelChanged(qobject_cast<QPieSlice *>(sender()));
}

void QPieModelMapper::onSliceValueChanged() {
    Q_D(QPieModelMapper);
    d->onSliceValueChanged(qobject_cast<QPieSlice *>(sender()));
}

QAbstractItemModel *QPieModelMapper::model() const {
    Q_D(const QPieModelMapper);
    return d->m_model;
}

void QPieModelMapper::setModel(QAbstractItemModel *model) {
    if (model == nullptr)
        return;
    Q_D(QPieModelMapper);
    if (d->m_model) {
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::modelReset,
                                   d,
                                   &QPieModelMapperPrivate::initializePieFromModel);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::dataChanged,
                                   d,
                                   &QPieModelMapperPrivate::onModelUpdated);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsInserted,
                                   d,
                                   &QPieModelMapperPrivate::onModelRowsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsRemoved,
                                   d,
                                   &QPieModelMapperPrivate::onModelRowsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsInserted,
                                   d,
                                   &QPieModelMapperPrivate::onModelColumnsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsRemoved,
                                   d,
                                   &QPieModelMapperPrivate::onModelColumnsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::destroyed,
                                   d,
                                   &QPieModelMapperPrivate::handleModelDestroyed);
    }
    d->m_model = model;
    d->initializePieFromModel();

    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::modelReset,
                            d,
                            &QPieModelMapperPrivate::initializePieFromModel);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::dataChanged,
                            d,
                            &QPieModelMapperPrivate::onModelUpdated);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::rowsInserted,
                            d,
                            &QPieModelMapperPrivate::onModelRowsAdded);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::rowsRemoved,
                            d,
                            &QPieModelMapperPrivate::onModelRowsRemoved);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::columnsInserted,
                            d,
                            &QPieModelMapperPrivate::onModelColumnsAdded);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::columnsRemoved,
                            d,
                            &QPieModelMapperPrivate::onModelColumnsRemoved);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::destroyed,
                            d,
                            &QPieModelMapperPrivate::handleModelDestroyed);
    Q_EMIT modelChanged();
}

QPieSeries *QPieModelMapper::series() const {
    Q_D(const QPieModelMapper);
    return d->m_series;
}

void QPieModelMapper::setSeries(QPieSeries *series) {
    Q_D(QPieModelMapper);
    if (d->m_series) {
        QObjectPrivate::disconnect(d->m_series,
                                   &QPieSeries::added,
                                   d,
                                   &QPieModelMapperPrivate::onSlicesAdded);
        QObjectPrivate::disconnect(d->m_series,
                                   &QPieSeries::removed,
                                   d,
                                   &QPieModelMapperPrivate::onSlicesRemoved);
        QObjectPrivate::disconnect(d->m_series,
                                   &QPieSeries::destroyed,
                                   d,
                                   &QPieModelMapperPrivate::handleSeriesDestroyed);
    }

    if (series == 0)
        return;

    d->m_series = series;
    d->initializePieFromModel();
    // connect the signals from the series
    QObjectPrivate::connect(d->m_series,
                            &QPieSeries::added,
                            d,
                            &QPieModelMapperPrivate::onSlicesAdded);
    QObjectPrivate::connect(d->m_series,
                            &QPieSeries::removed,
                            d,
                            &QPieModelMapperPrivate::onSlicesRemoved);
    QObjectPrivate::connect(d->m_series,
                            &QPieSeries::destroyed,
                            d,
                            &QPieModelMapperPrivate::handleSeriesDestroyed);
    Q_EMIT seriesChanged();
}

qsizetype QPieModelMapper::first() const
{
    Q_D(const QPieModelMapper);
    return d->m_first;
}

void QPieModelMapper::setFirst(qsizetype first)
{
    Q_D(QPieModelMapper);
    d->m_first = qMax(first, 0);
    d->initializePieFromModel();
    Q_EMIT firstChanged();
}

qsizetype QPieModelMapper::count() const
{
    Q_D(const QPieModelMapper);
    return d->m_count;
}

void QPieModelMapper::setCount(qsizetype count)
{
    Q_D(QPieModelMapper);
    d->m_count = qMax(count, -1);
    d->initializePieFromModel();
    Q_EMIT countChanged();
}

Qt::Orientation QPieModelMapper::orientation() const {
    Q_D(const QPieModelMapper);
    return d->m_orientation;
}

void QPieModelMapper::setOrientation(Qt::Orientation orientation) {
    Q_D(QPieModelMapper);
    d->m_orientation = orientation;
    d->initializePieFromModel();
    Q_EMIT orientationChanged();
}

qsizetype QPieModelMapper::valuesSection() const
{
    Q_D(const QPieModelMapper);
    return d->m_valuesSection;
}

void QPieModelMapper::setValuesSection(qsizetype valuesSection)
{
    Q_D(QPieModelMapper);
    d->m_valuesSection = qMax(-1, valuesSection);
    d->initializePieFromModel();
    Q_EMIT valuesSectionChanged();
}

qsizetype QPieModelMapper::labelsSection() const
{
    Q_D(const QPieModelMapper);
    return d->m_labelsSection;
}

void QPieModelMapper::setLabelsSection(qsizetype labelsSection)
{
    Q_D(QPieModelMapper);
    d->m_labelsSection = qMax(-1, labelsSection);
    d->initializePieFromModel();
    Q_EMIT labelsSectionChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QPieModelMapperPrivate::QPieModelMapperPrivate() {}

QPieModelMapperPrivate::~QPieModelMapperPrivate() {}

void QPieModelMapperPrivate::blockModelSignals(bool block) {
    m_modelSignalsBlock = block;
}

void QPieModelMapperPrivate::blockSeriesSignals(bool block) {
    m_seriesSignalsBlock = block;
}

QPieSlice *QPieModelMapperPrivate::pieSlice(QModelIndex index) const {
    if (!index.isValid())
        return 0; // index is invalid

    if (m_orientation == Qt::Vertical
        && (index.column() == m_valuesSection || index.column() == m_labelsSection)) {
        if (index.row() >= m_first && (m_count == -1 || index.row() < m_first + m_count)) {
            if (m_model->index(index.row(), m_valuesSection).isValid()
                && m_model->index(index.row(), m_labelsSection).isValid())
                return m_series->slices().at(index.row() - m_first);
            else
                return 0;
        }
    } else if (m_orientation == Qt::Horizontal
               && (index.row() == m_valuesSection || index.row() == m_labelsSection)) {
        if (index.column() >= m_first && (m_count == -1 || index.column() < m_first + m_count)) {
            if (m_model->index(m_valuesSection, index.column()).isValid()
                && m_model->index(m_labelsSection, index.column()).isValid())
                return m_series->slices().at(index.column() - m_first);
            else
                return 0;
        }
    }
    return 0; // This part of model has not been mapped to any slice
}

QModelIndex QPieModelMapperPrivate::valueModelIndex(qsizetype sliceIndex) {
    if (m_count != -1 && sliceIndex >= m_count)
        return QModelIndex(); // invalid

    if (m_orientation == Qt::Vertical)
        return m_model->index(int(sliceIndex) + m_first, m_valuesSection);
    else
        return m_model->index(m_valuesSection, int(sliceIndex) + m_first);
}

QModelIndex QPieModelMapperPrivate::labelModelIndex(qsizetype sliceIndex) {
    if (m_count != -1 && sliceIndex >= m_count)
        return QModelIndex(); // invalid

    if (m_orientation == Qt::Vertical)
        return m_model->index(int(sliceIndex) + m_first, m_labelsSection);
    else
        return m_model->index(m_labelsSection, int(sliceIndex) + m_first);
}

bool QPieModelMapperPrivate::isLabelIndex(QModelIndex index) const {
    if (m_orientation == Qt::Vertical && index.column() == m_labelsSection)
        return true;
    else if (m_orientation == Qt::Horizontal && index.row() == m_labelsSection)
        return true;

    return false;
}

bool QPieModelMapperPrivate::isValueIndex(QModelIndex index) const {
    if (m_orientation == Qt::Vertical && index.column() == m_valuesSection)
        return true;
    else if (m_orientation == Qt::Horizontal && index.row() == m_valuesSection)
        return true;

    return false;
}

void QPieModelMapperPrivate::onSlicesAdded(const QList<QPieSlice *> &slices) {
    if (m_seriesSignalsBlock)
        return;

    if (slices.size() == 0)
        return;

    int firstIndex = int(m_series->slices().indexOf(slices.at(0)));
    if (firstIndex == -1)
        return;

    if (m_count != -1)
        m_count += slices.size();
    Q_Q(QPieModelMapper);
    for (int i = firstIndex; i < firstIndex + slices.size(); i++) {
        m_slices.insert(i, slices.at(i - firstIndex));
        QObject::connect(slices.at(i - firstIndex),
                         &QPieSlice::labelChanged,
                         q,
                         &QPieModelMapper::onSliceLabelChanged);
        QObject::connect(slices.at(i - firstIndex),
                         &QPieSlice::valueChanged,
                         q,
                         &QPieModelMapper::onSliceValueChanged);
    }

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->insertRows(firstIndex + m_first, int(slices.size()));
    else
        m_model->insertColumns(firstIndex + m_first, int(slices.size()));

    for (int i = firstIndex; i < firstIndex + slices.size(); i++) {
        m_model->setData(valueModelIndex(i), slices.at(i - firstIndex)->value());
        m_model->setData(labelModelIndex(i), slices.at(i - firstIndex)->label());
    }
    blockModelSignals(false);
}

void QPieModelMapperPrivate::onSlicesRemoved(const QList<QPieSlice *> &slices) {
    if (m_seriesSignalsBlock)
        return;

    if (slices.size() == 0)
        return;

    int firstIndex = int(m_slices.indexOf(slices.at(0)));
    if (firstIndex == -1)
        return;

    if (m_count != -1)
        m_count -= slices.size();

    for (int i = firstIndex + int(slices.size()) - 1; i >= firstIndex; i--)
        m_slices.removeAt(i);

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->removeRows(firstIndex + m_first, int(slices.size()));
    else
        m_model->removeColumns(firstIndex + m_first, int(slices.size()));
    blockModelSignals(false);
}

void QPieModelMapperPrivate::onSliceLabelChanged(const QPieSlice *slice) {
    if (m_seriesSignalsBlock)
        return;

    blockModelSignals();
    m_model->setData(labelModelIndex(m_series->slices().indexOf(slice)), slice->label());
    blockModelSignals(false);
}

void QPieModelMapperPrivate::onSliceValueChanged(const QPieSlice *slice) {
    if (m_seriesSignalsBlock)
        return;

    blockModelSignals();
    m_model->setData(valueModelIndex(m_series->slices().indexOf(slice)), slice->value());
    blockModelSignals(false);
}

void QPieModelMapperPrivate::handleSeriesDestroyed() { m_series = nullptr; }

void QPieModelMapperPrivate::onModelUpdated(QModelIndex topLeft, QModelIndex bottomRight)
{
    if (m_model == nullptr || m_series == nullptr)
        return;

    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    QModelIndex index;
    QPieSlice *slice;
    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        for (int column = topLeft.column(); column <= bottomRight.column(); column++) {
            index = topLeft.sibling(row, column);
            slice = pieSlice(index);
            if (slice) {
                if (isValueIndex(index))
                    slice->setValue(m_model->data(index, Qt::DisplayRole).toReal());
                if (isLabelIndex(index))
                    slice->setLabel(m_model->data(index, Qt::DisplayRole).toString());
            }
        }
    }
    blockSeriesSignals(false);
}

void QPieModelMapperPrivate::onModelRowsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        insertData(start, end);
    } else if (start <= m_valuesSection
               || start <= m_labelsSection) { // if the changes affect the map - reinitialize the pie
        initializePieFromModel();
    }
    blockSeriesSignals(false);
}

void QPieModelMapperPrivate::onModelRowsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        removeData(start, end);
    } else if (start <= m_valuesSection || start <= m_labelsSection) {
        // if the changes affect the map - reinitialize the pie
        initializePieFromModel();
    }
    blockSeriesSignals(false);
}

void QPieModelMapperPrivate::onModelColumnsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        insertData(start, end);
    } else if (start <= m_valuesSection || start <= m_labelsSection) {
        // if the changes affect the map - reinitialize the pie
        initializePieFromModel();
    }
    blockSeriesSignals(false);
}

void QPieModelMapperPrivate::onModelColumnsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        removeData(start, end);
    } else if (start <= m_valuesSection || start <= m_labelsSection) {
        // if the changes affect the map - reinitialize the pie
        initializePieFromModel();
    }
    blockSeriesSignals(false);
}

void QPieModelMapperPrivate::handleModelDestroyed() { m_model = nullptr; }

void QPieModelMapperPrivate::insertData(qsizetype start, qsizetype end) {
    if (m_model == nullptr || m_series == nullptr)
        return;

    if (m_count != -1 && start >= m_first + m_count) {
        return;
    } else {
        Q_TRACE_SCOPE(QGraphs2DPieModelMapperInsertData, start, end);
        qsizetype addedCount = end - start + 1;
        if (m_count != -1 && addedCount > m_count)
            addedCount = m_count;
        qsizetype first = qMax(start, m_first);
        qsizetype last = qMin(first + addedCount - 1,
                              m_orientation == Qt::Vertical ? m_model->rowCount() - 1
                                                            : m_model->columnCount() - 1);
        Q_Q(QPieModelMapper);

        for (qsizetype i = first; i <= last; i++) {
            QModelIndex valueIndex = valueModelIndex(i - m_first);
            QModelIndex labelIndex = labelModelIndex(i - m_first);
            if (valueIndex.isValid() && labelIndex.isValid()) {
                QPieSlice *slice = new QPieSlice;
                slice->setValue(m_model->data(valueIndex, Qt::DisplayRole).toDouble());
                slice->setLabel(m_model->data(labelIndex, Qt::DisplayRole).toString());
                QObject::connect(slice,
                                 &QPieSlice::labelChanged,
                                 q,
                                 &QPieModelMapper::onSliceLabelChanged);
                QObject::connect(slice,
                                 &QPieSlice::valueChanged,
                                 q,
                                 &QPieModelMapper::onSliceValueChanged);
                m_series->insert(i - m_first, slice);
                m_slices.insert(i - m_first, slice);
            }
        }

        // remove excess of slices (abouve m_count)
        if (m_count != -1 && m_series->slices().size() > m_count) {
            for (qsizetype i = m_series->slices().size() - 1; i >= m_count; i--) {
                m_series->remove(m_series->slices().at(i));
                m_slices.removeAt(i);
            }
        }
    }
}

void QPieModelMapperPrivate::removeData(qsizetype start, qsizetype end) {
    if (m_model == 0 || m_series == 0)
        return;

    Q_TRACE_SCOPE(QGraphs2DPieModelMapperRemoveData, start, end);
    qsizetype removedCount = end - start + 1;
    if (m_count != -1 && start >= m_first + m_count) {
        return;
    } else {
        qsizetype toRemove = qMin(m_series->slices().size(),
                                  removedCount); // first find how many items can actually be removed
        qsizetype first = qMax(start,
                               m_first); // get the index of the first item that will be removed.
        qsizetype last = qMin(first + toRemove - 1,
                              m_series->slices().size() + m_first
                                      - 1); // get the index of the last item that will be removed.
        for (qsizetype i = last; i >= first; i--) {
            m_series->remove(m_series->slices().at(i - m_first));
            m_slices.removeAt(i - m_first);
        }

        if (m_count != -1) {
            qsizetype itemsAvailable; // check how many are available to be added
            if (m_orientation == Qt::Vertical)
                itemsAvailable = m_model->rowCount() - m_first - m_series->slices().size();
            else
                itemsAvailable = m_model->columnCount() - m_first - m_series->slices().size();
            qsizetype toBeAdded = qMin(itemsAvailable,
                                       m_count
                                               - m_series->slices().size()); // add not more items than there
                                                                             // is space left to be filled.
            qsizetype currentSize = m_series->slices().size();
            if (toBeAdded > 0) {
                for (qsizetype i = m_series->slices().size(); i < currentSize + toBeAdded; i++) {
                    QModelIndex valueIndex = valueModelIndex(i - m_first);
                    QModelIndex labelIndex = labelModelIndex(i - m_first);
                    if (valueIndex.isValid() && labelIndex.isValid()) {
                        QPieSlice *slice = new QPieSlice;
                        slice->setValue(m_model->data(valueIndex, Qt::DisplayRole).toDouble());
                        slice->setLabel(m_model->data(labelIndex, Qt::DisplayRole).toString());
                        m_series->insert(i, slice);
                        m_slices.insert(i, slice);
                    }
                }
            }
        }
    }
}

void QPieModelMapperPrivate::initializePieFromModel() {
    if (m_model == nullptr || m_series == nullptr)
        return;

    Q_TRACE_SCOPE(QGraphs2DPieModelMapperInitPieFromModel);
    blockSeriesSignals();
    // clear current content
    m_series->clear();
    m_slices.clear();

    // create the initial slices set
    int slicePos = 0;
    QModelIndex valueIndex = valueModelIndex(slicePos);
    QModelIndex labelIndex = labelModelIndex(slicePos);
    Q_Q(QPieModelMapper);
    while (valueIndex.isValid() && labelIndex.isValid()) {
        QPieSlice *slice = new QPieSlice;
        slice->setLabel(m_model->data(labelIndex, Qt::DisplayRole).toString());
        slice->setValue(m_model->data(valueIndex, Qt::DisplayRole).toDouble());
        QObject::connect(slice, &QPieSlice::labelChanged, q, &QPieModelMapper::onSliceLabelChanged);
        QObject::connect(slice, &QPieSlice::valueChanged, q, &QPieModelMapper::onSliceValueChanged);
        m_series->append(slice);
        m_slices.append(slice);
        slicePos++;
        valueIndex = valueModelIndex(slicePos);
        labelIndex = labelModelIndex(slicePos);
    }
    blockSeriesSignals(false);
}

QT_END_NAMESPACE

#include "moc_qpiemodelmapper.cpp"
