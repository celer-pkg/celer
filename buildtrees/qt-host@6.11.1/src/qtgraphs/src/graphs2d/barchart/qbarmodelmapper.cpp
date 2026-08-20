// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCore/QAbstractItemModel>
#include <QtGraphs/QBarModelMapper>
#include <QtGraphs/QBarSeries>
#include <private/qabstractitemmodel_p.h>
#include <private/qbarmodelmapper_p.h>

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
              "QT_BEGIN_NAMESPACE" \
               "class QBarModelMapper;" \
              "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DBarModelMapperInitBarsFromModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DBarModelMapperInitBarsFromModel_exit);

/*!
    \class QBarModelMapper
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QBarModelMapper class is a model mapper for bar series.

    Model mappers enable using a data model derived from the QAbstractItemModel class
    as a data source for a graph. A model mapper is used to create a connection
    between a data model and QBarSeries.

    Both model and bar series properties can be used to manipulate the data. The model mapper
    keeps the bar series and the data model in sync.

    The model mapper ensures that all the bar sets in the bar series have equal sizes.
    Therefore, adding or removing a value from a bar set causes the same change to be
    made in all the bar sets in the bar series.

*/
/*!
    \qmltype BarModelMapper
    \nativetype QBarModelMapper
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D

    \brief Model mapper for bar series.

    The BarModelMapper type enables using a data model derived from the QAbstractItemModel
    class as a data source for a graph. A model mapper is used to create a connection
    between a data model and QBarSeries. You need to implement
    the data model and expose it to QML.

    Both model and bar series properties can be used to manipulate the data. The model mapper
    keeps the bar series and the data model in sync.

    The model mapper ensures that all the bar sets in the bar series have equal sizes.
    Therefore, adding or removing a value from a bar set causes the same change to be
    made in all the bar sets in the bar series.

    The following QML code snippet creates a bar series with three bar sets (assuming the model
    has at least four columns). Each bar set contains data starting from row 1. The name
    of a bar set is defined by the column header.

    \code
        BarSeries {
            BarModelMapper {
                model: myCustomModel // QAbstractItemModel derived implementation
                firstBarSetColumn: 1
                lastBarSetColumn: 3
                firstRow: 1
                orientation: Qt.Vertical
            }
        }
    \endcode

*/

/*!
    \property QBarModelMapper::series
    \brief The bar series that is used by the mapper.

    All the data in the series is discarded when it is set to the mapper.
    When a new series is specified, the old series is disconnected, but it preserves its data.
*/
/*!
    \qmlproperty BarSeries BarModelMapper::series
    The bar series that is used by the mapper. All the data in the series is discarded when it is
    set to the mapper. When the new series is specified, the old series is disconnected, but it
    preserves its data.
*/

/*!
    \property QBarModelMapper::model
    \brief The data model that is used by the mapper.
*/
/*!
    \qmlproperty model BarModelMapper::model
    The data model that is used by the mapper. You need to implement the model and expose it to QML.

    \note The model has to support adding and removing rows or columns and modifying
    the data in the cells.
*/

/*!
    \property QBarModelMapper::firstBarSetSection
    \brief The section of the model that is used as the data source for the first bar set.

    The default value is -1 (invalid mapping).

    \sa QBarModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype BarModelMapper::firstBarSetSection
    The section of the model that is used as the data source for the first bar set. The default value
    is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QBarModelMapper::lastBarSetSection
    \brief The section of the model that is used as the data source for the last bar set.

    The default value is -1 (invalid mapping).

    \sa QBarModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype BarModelMapper::lastBarSetSection
    The section of the model that is used as the data source for the last bar set. The default
    value is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QBarModelMapper::first
    \brief The row or column of the model that contains the first values of the bar sets in the bar series.

    The minimum and default value is 0.

    \sa QBarModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype BarModelMapper::first
    The row or column of the model that contains the first values of the bar sets in the bar series.
    The default value is 0.

    \sa orientation
*/

/*!
    \property QBarModelMapper::count
    \brief The number of rows or columns of the model that are mapped as the data for the bar series.

    The default value is \c{-1} which is also the minimum. The count is
    limited by the number of model's rows/columns.

    \sa QBarModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype BarModelMapper::count
    The number of rows or columns of the model that are mapped as the data for the bar
    series. The default value is \c{-1} which is also the minimum. The count is
    limited by the number of model's rows/columns.

    \sa orientation
*/

/*!
    \property QBarModelMapper::orientation
    \brief Tells the modelmapper how to map data from a model. If
    \c{Qt::Vertical} is used, each of the model's columns defines a bar set, and the
    model's rows define the categories. When the orientation is set to
    \c{Qt::Horizontal}, each of the model's rows defines a bar set, and the model's
    columns define categories.

    The default value is \c{Qt::Vertical}
*/
/*!
    \qmlproperty  orientation BarModelMapper::orientation
    Tells the modelmapper how to map data from a model. If
    \c{Qt.Vertical} is used, each of the model's columns defines a bar set, and the
    model's rows define the categories. When the orientation is set to
    \c{Qt.Horizontal}, each of the model's rows defines a bar set, and the model's
    columns define categories.
*/

/*!
    \qmlsignal BarModelMapper::seriesChanged()

    This signal is emitted when the bar series that the mapper is connected to changes.
*/

/*!
    \qmlsignal BarModelMapper::modelChanged()

    This signal is emitted when the model that the mapper is connected to changes.
*/

/*!
    \qmlsignal BarModelMapper::firstBarSetSectionChanged()
    This signal is emitted when the first bar set section changes.
*/

/*!
    \qmlsignal BarModelMapper::lastBarSetSectionChanged()
    This signal is emitted when the last bar set section changes.
*/

/*!
    \qmlsignal BarModelMapper::firstChanged()
    This signal is emitted when the first row or column changes.
*/

/*!
    \qmlsignal BarModelMapper::countChanged()
    This signal is emitted when the number of rows or columns changes.
*/

/*!
    \qmlsignal BarModelMapper::orientationChanged()
    This signal is emitted when the orientation changes.
*/

QBarModelMapper::QBarModelMapper(QObject *parent)
    : QObject(*(new QBarModelMapperPrivate), parent)
{}

QBarModelMapper::QBarModelMapper(QBarModelMapperPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{}

QBarModelMapper::~QBarModelMapper() {}

QAbstractItemModel *QBarModelMapper::model() const
{
    Q_D(const QBarModelMapper);
    return d->m_model;
}

void QBarModelMapper::setModel(QAbstractItemModel *model)
{
    Q_D(QBarModelMapper);
    if (d->m_model) {
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::modelReset,
                                   d,
                                   &QBarModelMapperPrivate::initializeBarsFromModel);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::dataChanged,
                                   d,
                                   &QBarModelMapperPrivate::modelUpdated);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::headerDataChanged,
                                   d,
                                   &QBarModelMapperPrivate::modelHeaderDataUpdated);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsInserted,
                                   d,
                                   &QBarModelMapperPrivate::modelRowsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsRemoved,
                                   d,
                                   &QBarModelMapperPrivate::modelRowsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsInserted,
                                   d,
                                   &QBarModelMapperPrivate::modelColumnsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsRemoved,
                                   d,
                                   &QBarModelMapperPrivate::modelColumnsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::destroyed,
                                   d,
                                   &QBarModelMapperPrivate::handleModelDestroyed);
    }

    d->m_model = model;

    d->initializeBarsFromModel();

    if (d->m_model) {
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::modelReset,
                                d,
                                &QBarModelMapperPrivate::initializeBarsFromModel);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::dataChanged,
                                d,
                                &QBarModelMapperPrivate::modelUpdated);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::headerDataChanged,
                                d,
                                &QBarModelMapperPrivate::modelHeaderDataUpdated);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::rowsInserted,
                                d,
                                &QBarModelMapperPrivate::modelRowsAdded);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::rowsRemoved,
                                d,
                                &QBarModelMapperPrivate::modelRowsRemoved);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::columnsInserted,
                                d,
                                &QBarModelMapperPrivate::modelColumnsAdded);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::columnsRemoved,
                                d,
                                &QBarModelMapperPrivate::modelColumnsRemoved);
        QObjectPrivate::connect(d->m_model,
                                &QAbstractItemModel::destroyed,
                                d,
                                &QBarModelMapperPrivate::handleModelDestroyed);
    }
    Q_EMIT modelChanged();
}

QBarSeries *QBarModelMapper::series() const
{
    Q_D(const QBarModelMapper);
    return d->m_series;
}

void QBarModelMapper::setSeries(QBarSeries *series)
{
    Q_D(QBarModelMapper);
    if (d->m_series) {
        QObjectPrivate::disconnect(d->m_series,
                                   &QBarSeries::barsetsAdded,
                                   d,
                                   &QBarModelMapperPrivate::barSetsAdded);
        QObjectPrivate::disconnect(d->m_series,
                                   &QBarSeries::barsetsRemoved,
                                   d,
                                   &QBarModelMapperPrivate::barSetsRemoved);
        QObjectPrivate::disconnect(d->m_series,
                                   &QBarSeries::destroyed,
                                   d,
                                   &QBarModelMapperPrivate::handleSeriesDestroyed);
    }

    d->m_series = series;

    d->initializeBarsFromModel();
    if (d->m_series) {
        QObjectPrivate::connect(d->m_series,
                                &QBarSeries::barsetsAdded,
                                d,
                                &QBarModelMapperPrivate::barSetsAdded);
        QObjectPrivate::connect(d->m_series,
                                &QBarSeries::barsetsRemoved,
                                d,
                                &QBarModelMapperPrivate::barSetsRemoved);
        QObjectPrivate::connect(d->m_series,
                                &QBarSeries::destroyed,
                                d,
                                &QBarModelMapperPrivate::handleSeriesDestroyed);
    }
    Q_EMIT seriesChanged();
}

qsizetype QBarModelMapper::first() const
{
    Q_D(const QBarModelMapper);
    return d->m_first;
}

void QBarModelMapper::setFirst(qsizetype newFirst)
{
    Q_D(QBarModelMapper);
    d->m_first = qMax(newFirst, 0);
    d->initializeBarsFromModel();
    Q_EMIT firstChanged();
}

Qt::Orientation QBarModelMapper::orientation() const
{
    Q_D(const QBarModelMapper);
    return d->m_orientation;
}

void QBarModelMapper::setOrientation(Qt::Orientation orientation)
{
    Q_D(QBarModelMapper);
    d->m_orientation = orientation;
    d->initializeBarsFromModel();
    Q_EMIT orientationChanged();
}

qsizetype QBarModelMapper::count() const
{
    Q_D(const QBarModelMapper);
    return d->m_count;
}

void QBarModelMapper::setCount(qsizetype newCount)
{
    Q_D(QBarModelMapper);
    d->m_count = qMax(newCount, -1);
    d->initializeBarsFromModel();
    Q_EMIT countChanged();
}

qsizetype QBarModelMapper::lastBarSetSection() const
{
    Q_D(const QBarModelMapper);
    return d->m_lastBarSetSection;
}

void QBarModelMapper::setLastBarSetSection(qsizetype newLastBarSetSection)
{
    Q_D(QBarModelMapper);
    d->m_lastBarSetSection = qMax(-1, newLastBarSetSection);
    d->initializeBarsFromModel();
    Q_EMIT lastBarSetSectionChanged();
}

qsizetype QBarModelMapper::firstBarSetSection() const
{
    Q_D(const QBarModelMapper);
    return d->m_firstBarSetSection;
}

void QBarModelMapper::setFirstBarSetSection(qsizetype newFirstBarSetSection)
{
    Q_D(QBarModelMapper);
    d->m_firstBarSetSection = qMax(-1, newFirstBarSetSection);
    d->initializeBarsFromModel();
    Q_EMIT firstBarSetSectionChanged();
}

void QBarModelMapper::onValuesAdded(qsizetype index, qsizetype count)
{
    Q_D(QBarModelMapper);

    if (d->m_seriesSignalsBlock)
        return;
    d->handleValuesAdded(qobject_cast<QBarSet *>(sender()), index, count);
}

void QBarModelMapper::onBarLabelChanged()
{
    Q_D(QBarModelMapper);

    if (d->m_seriesSignalsBlock)
        return;
    d->handleBarLabelChanged(qobject_cast<QBarSet *>(sender()));
}

void QBarModelMapper::onBarValueChanged(qsizetype index)
{
    Q_D(QBarModelMapper);

    if (d->m_seriesSignalsBlock)
        return;
    d->handleBarValueChanged(qobject_cast<QBarSet *>(sender()), index);
}

/////////////////////////////////////////////////////////////////////////////////////
QBarModelMapperPrivate::QBarModelMapperPrivate() {}

QBarModelMapperPrivate::~QBarModelMapperPrivate() {}

QModelIndex QBarModelMapperPrivate::barModelIndex(qsizetype barSection, qsizetype posInBar)
{
    if (m_count != -1 && posInBar >= m_count)
        return QModelIndex(); // invalid

    if (barSection < m_firstBarSetSection || barSection > m_lastBarSetSection)
        return QModelIndex(); // invalid

    if (m_orientation == Qt::Vertical)
        return m_model->index(int(posInBar) + m_first, int(barSection));
    else
        return m_model->index(int(barSection), int(posInBar) + m_first);
}

void QBarModelMapperPrivate::blockSeriesSignals(const bool block)
{
    m_seriesSignalsBlock = block;
}

void QBarModelMapperPrivate::blockModelSignals(const bool block)
{
    m_modelSignalsBlock = block;
}

QBarSet *QBarModelMapperPrivate::barSet(QModelIndex index)
{
    if (!index.isValid())
        return 0;

    if (m_orientation == Qt::Vertical && index.column() >= m_firstBarSetSection
        && index.column() <= m_lastBarSetSection) {
        if (index.row() >= m_first && (m_count == -1 || index.row() < m_first + m_count))
            return m_series->barSets().at(index.column() - m_firstBarSetSection);

    } else if (m_orientation == Qt::Horizontal && index.row() >= m_firstBarSetSection
               && index.row() <= m_lastBarSetSection) {
        if (index.column() >= m_first && (m_count == -1 || index.column() < m_first + m_count))
            return m_series->barSets().at(index.row() - m_firstBarSetSection);
    }
    return 0; // This part of model has not been mapped to any slice
}

void QBarModelMapperPrivate::insertData(qsizetype start, qsizetype end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    // Currently bar graph needs to be fully recalculated when change is made.
    // Re-initialize
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::removeData(qsizetype start, qsizetype end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    // Currently bar graph needs to be fully recalculated when change is made.
    // Re-initialize
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::initializeBarsFromModel()
{
    if (m_model == nullptr || m_series == nullptr)
        return;
    Q_Q(QBarModelMapper);
    blockSeriesSignals();
    m_series->clear();
    m_barSets.clear();

    Q_TRACE_SCOPE(QGraphs2DBarModelMapperInitBarsFromModel);

    // create the initial bar sets
    for (int i = m_firstBarSetSection; i <= m_lastBarSetSection; i++) {
        int posInBar = 0;
        QModelIndex barIndex = barModelIndex(i, posInBar);
        // check if there is such model index
        if (barIndex.isValid()) {
            QBarSet *barSet = new QBarSet(
                m_model->headerData(i, m_orientation == Qt::Vertical ? Qt::Horizontal : Qt::Vertical)
                    .toString());
            while (barIndex.isValid()) {
                barSet->append(m_model->data(barIndex, Qt::DisplayRole).toDouble());
                posInBar++;
                barIndex = barModelIndex(i, posInBar);
            }
            QObjectPrivate::connect(barSet,
                                    &QBarSet::valuesRemoved,
                                    this,
                                    &QBarModelMapperPrivate::valuesRemoved);

            QObject::connect(barSet, &QBarSet::valuesAdded, q, &QBarModelMapper::onValuesAdded);
            QObject::connect(barSet, &QBarSet::valueChanged, q, &QBarModelMapper::onBarValueChanged);
            QObject::connect(barSet, &QBarSet::labelChanged, q, &QBarModelMapper::onBarLabelChanged);
            m_series->append(barSet);
            m_barSets.append(barSet);
        } else {
            break;
        }
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelUpdated(QModelIndex topLeft, QModelIndex bottomRight)
{
    if (m_model == nullptr || m_series == nullptr)
        return;

    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    QModelIndex index;
    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        for (int column = topLeft.column(); column <= bottomRight.column(); column++) {
            index = topLeft.sibling(row, column);
            QBarSet *bar = barSet(index);
            if (bar) {
                if (m_orientation == Qt::Vertical)
                    bar->replace(row - m_first, m_model->data(index).toReal());
                else
                    bar->replace(column - m_first, m_model->data(index).toReal());
            }
        }
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelHeaderDataUpdated(Qt::Orientation orientation, qsizetype first, qsizetype last)
{
    if (m_model == nullptr || m_series == nullptr)
        return;

    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (orientation != m_orientation) {
        for (int section = first; section <= last; section++) {
            if (section >= m_firstBarSetSection && section <= m_lastBarSetSection) {
                QBarSet *bar = m_series->barSets().at(section - m_firstBarSetSection);
                if (bar)
                    bar->setLabel(m_model->headerData(section, orientation).toString());
            }
        }
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelRowsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        insertData(start, end);
    } else if (start <= m_firstBarSetSection || start <= m_lastBarSetSection) {
        // if the changes affect the map - reinitialize
        initializeBarsFromModel();
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelRowsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        removeData(start, end);
    } else if (start <= m_firstBarSetSection || start <= m_lastBarSetSection) {
        // if the changes affect the map - reinitialize
        initializeBarsFromModel();
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelColumnsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        insertData(start, end);
    } else if (start <= m_firstBarSetSection || start <= m_lastBarSetSection) {
        // if the changes affect the map - reinitialize
        initializeBarsFromModel();
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::modelColumnsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        removeData(start, end);
    } else if (start <= m_firstBarSetSection || start <= m_lastBarSetSection) {
        // if the changes affect the map - reinitialize
        initializeBarsFromModel();
    }
    blockSeriesSignals(false);
}

void QBarModelMapperPrivate::handleModelDestroyed()
{
    m_model = nullptr;
}

void QBarModelMapperPrivate::barSetsAdded(const QList<QBarSet *> &sets)
{
    if (m_seriesSignalsBlock)
        return;

    if (sets.size() == 0)
        return;

    int firstIndex = int(m_series->barSets().indexOf(sets.at(0)));
    if (firstIndex == -1)
        return;

    qsizetype maxCount = 0;
    for (int i = 0; i < sets.size(); i++) {
        if (sets.at(i)->count() > m_count)
            maxCount = sets.at(i)->count();
    }

    if (m_count != -1 && m_count < maxCount)
        m_count = maxCount;

    m_lastBarSetSection += sets.size();

    blockModelSignals();
    int modelCapacity = m_orientation == Qt::Vertical ? m_model->rowCount() - m_first
                                                      : m_model->columnCount() - m_first;
    if (maxCount > modelCapacity) {
        if (m_orientation == Qt::Vertical)
            m_model->insertRows(m_model->rowCount(), int(maxCount) - modelCapacity);
        else
            m_model->insertColumns(m_model->columnCount(), int(maxCount) - modelCapacity);
    }

    if (m_orientation == Qt::Vertical)
        m_model->insertColumns(firstIndex + m_firstBarSetSection, int(sets.size()));
    else
        m_model->insertRows(firstIndex + m_firstBarSetSection, int(sets.size()));

    for (int i = firstIndex + m_firstBarSetSection;
         i < firstIndex + m_firstBarSetSection + sets.size();
         i++) {
        m_model->setHeaderData(i,
                               m_orientation == Qt::Vertical ? Qt::Horizontal : Qt::Vertical,
                               sets.at(i - firstIndex - m_firstBarSetSection)->label());
        for (int j = 0; j < sets.at(i - firstIndex - m_firstBarSetSection)->count(); j++) {
            m_model->setData(barModelIndex(i, j),
                             sets.at(i - firstIndex - m_firstBarSetSection)->at(j));
        }
    }
    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::barSetsRemoved(const QList<QBarSet *> &sets)
{
    if (m_seriesSignalsBlock)
        return;

    if (sets.size() == 0)
        return;

    int firstIndex = int(m_barSets.indexOf(sets.at(0)));
    if (firstIndex == -1)
        return;

    m_lastBarSetSection -= sets.size();

    for (qsizetype i = firstIndex + sets.size() - 1; i >= firstIndex; i--)
        m_barSets.removeAt(i);

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->removeColumns(firstIndex + m_firstBarSetSection, int(sets.size()));
    else
        m_model->removeRows(firstIndex + m_firstBarSetSection, int(sets.size()));
    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::handleValuesAdded(QBarSet *set, qsizetype index, qsizetype count)
{
    if (m_seriesSignalsBlock)
        return;

    if (m_count != -1)
        m_count += count;
    Q_ASSERT(set);
    qsizetype barSetIndex = m_barSets.indexOf(set);

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->insertRows(int(index) + m_first, int(count));
    else
        m_model->insertColumns(int(index) + m_first, int(count));

    for (qsizetype j = index; j < index + count; j++) {
        m_model->setData(barModelIndex(barSetIndex + m_firstBarSetSection, int(j)),
                         m_barSets.at(barSetIndex)->at(j));
    }

    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::valuesRemoved(qsizetype index, qsizetype count)
{
    if (m_seriesSignalsBlock)
        return;

    if (m_count != -1)
        m_count -= count;

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->removeRows(int(index) + m_first, int(count));
    else
        m_model->removeColumns(int(index) + m_first, int(count));

    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::handleBarLabelChanged(QBarSet *set)
{
    if (m_seriesSignalsBlock)
        return;

    Q_ASSERT(set);
    int barSetIndex = int(m_barSets.indexOf(set));

    blockModelSignals();
    m_model->setHeaderData(barSetIndex + m_firstBarSetSection,
                           m_orientation == Qt::Vertical ? Qt::Horizontal : Qt::Vertical,
                           m_barSets.at(barSetIndex)->label());
    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::handleBarValueChanged(QBarSet *set, qsizetype index)
{
    if (m_seriesSignalsBlock)
        return;

    Q_ASSERT(set);
    int barSetIndex = int(m_barSets.indexOf(set));

    blockModelSignals();
    m_model->setData(barModelIndex(barSetIndex + m_firstBarSetSection, index),
                     m_barSets.at(barSetIndex)->at(index));
    blockModelSignals(false);
    initializeBarsFromModel();
}

void QBarModelMapperPrivate::handleSeriesDestroyed()
{
    m_series = nullptr;
}

QT_END_NAMESPACE

#include "moc_qbarmodelmapper.cpp"
