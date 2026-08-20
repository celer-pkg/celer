// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtCore/QAbstractItemModel>
#include <QtGraphs/QXYModelMapper>
#include <QtGraphs/QXYSeries>
#include "qxymodelmapper_p.h"
#include "qxyseries_p.h"

#include <qtgraphs_tracepoints_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcModelMapper2D, "qt.graphs2d.modelmapper")

Q_TRACE_PREFIX(qtgraphs,
            "QT_BEGIN_NAMESPACE" \
            "class QXYModelMapper;" \
            "QT_END_NAMESPACE"
          )

Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeInitXYFromModel_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeInitXYFromModel_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeRemoveData_entry, int start, int end);
Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeRemoveData_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeInsertData_entry, int start, int end);
Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMappeInsertData_exit);

Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMapperOnModelUpdated_entry);
Q_TRACE_POINT(qtgraphs, QGraphs2DXYModelMapperOnModelUpdated_exit);

/*!
    \class QXYModelMapper
    \inmodule QtGraphs
    \ingroup graphs_2D

    \brief The QXYModelMapper class is a model mapper for line,
    spline, and scatter series.

    Model mappers enable using a data model derived from the QAbstractItemModel
    class as a data source for a graph. A model mapper is used to
    create a connection between a line, spline, or scatter series.
    A \e TableModel is a natural choice
    for the model.

    Both model and series properties can be used to manipulate the data. The
    model mapper keeps the series and the data model in sync.

    \sa QXYSeries
*/
/*!
    \qmltype XYModelMapper
    \nativetype QXYModelMapper
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D

    \brief A model mapper for XYSeries.

    Model mappers enable using a data model derived from the QAbstractItemModel
    class as a data source for a graph. A model mapper is used to
    create a connection between a line, spline, or scatter series.
    A \e TableModel is a natural choice
    for the model.

    Both model and series properties can be used to manipulate the data. The
    model mapper keeps the series and the data model in sync.

   \sa XYSeries
*/

/*!
    \property QXYModelMapper::series
    \brief The series that is used by the mapper.

    All the data in the series is discarded when it is set to the mapper.
    When a new series is specified, the old series is disconnected (but it
    preserves its data).
*/
/*!
    \qmlproperty XYSeries XYModelMapper::series
    The series that is used by the mapper. All the data in the series is
    discarded when it is set to the mapper. When a new series is specified, the
    old series is disconnected (but it preserves its data).
*/

/*!
    \property QXYModelMapper::model
    \brief The model that is used by the mapper.
*/
/*!
    \qmlproperty SomeModel XYModelMapper::model
    The data model that is used by the mapper. You need to implement the model
    and expose it to QML.

    \note The model has to support adding and removing rows or columns and
    modifying the data in the cells.
*/

/*!
    \property QXYModelMapper::xSection
    \brief The section of the model that contains the x-coordinates of data
    points.

    The default value is -1 (invalid mapping).

    \sa QXYModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype XYModelMapper::xSection
    the section of the model that contains the x-coordinates of data points.
    The default value is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QXYModelMapper::ySection
    \brief the section of the model that contains the y-coordinates of data
    points.

    The default value is -1 (invalid mapping).

    \sa QXYModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype XYModelMapper::ySection
    the section of the model that contains the y-coordinates of data points.
    The default value is -1 (invalid mapping).

    \sa orientation
*/

/*!
    \property QXYModelMapper::first
    \brief The row of the model that contains the data for the first point
    of the series.

    The minimum and default value is 0.

    \sa QXYModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype XYModelMapper::first
    The row of the model that contains the data for the first point of the series.
    The default value is 0.

    \sa orientation
*/

/*!
    \property QXYModelMapper::count
    \brief The number of rows of the model that are mapped as the data for series.

    The minimum and default value is -1 (the number is limited by the number of
    rows in the model).

    \sa QXYModelMapper::orientation
*/
/*!
    \qmlproperty qsizetype XYModelMapper::count
    The number of rows of the model that are mapped as the data for series. The default value is
    -1 (the number is limited by the number of rows in the model).

    \sa orientation
*/

/*!
    \property QXYModelMapper::orientation
    \brief Tells the modelmapper how to map data from a model. If
    \c{Qt::Vertical} is used, each of the model's columns defines a bar set, and the
    model's rows define the categories. When the orientation is set to
    \c{Qt::Horizontal}, each of the model's rows defines a bar set, and the model's
    columns define categories.

    The default value is \c{Qt::Vertical}
*/
/*!
    \qmlproperty orientation XYModelMapper::orientation
    Tells the modelmapper how to map data from a model. If
    \c{Qt.Vertical} is used, the model has \e X and \e Y columns, and the
    model's rows define the data points. When the orientation is set to
    \c{Qt.Horizontal}, the model has \e X and \e Y rows, and the model's
    columns define the data points.
*/

/*!
    \qmlsignal XYModelMapper::seriesChanged()

    This signal is emitted when the series that the mapper is connected to changes.
*/

/*!
    \qmlsignal XYModelMapper::modelChanged()

    This signal is emitted when the model that the mapper is connected to changes.
*/

/*!
    \qmlsignal XYModelMapper::xSectionChanged()

    This signal is emitted when the section that contains the x-coordinates of
    data points changes.
*/

/*!
    \qmlsignal XYModelMapper::ySectionChanged()

    This signal is emitted when the section that contains the y-coordinates of
    data points changes.
*/

/*!
    \qmlsignal XYModelMapper::firstChanged()
    This signal is emitted when the first row changes.
*/

/*!
    \qmlsignal XYModelMapper::countChanged()
    This signal is emitted when the number of rows changes.
*/

/*!
    \qmlsignal BarModelMapper::orientationChanged()
    This signal is emitted when the orientation changes.
*/

QXYModelMapper::~QXYModelMapper() {}

QXYModelMapper::QXYModelMapper(QObject *parent)
    : QObject{*(new QXYModelMapperPrivate), parent}
{}

QXYModelMapper::QXYModelMapper(QXYModelMapperPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{}

QAbstractItemModel *QXYModelMapper::model() const
{
    Q_D(const QXYModelMapper);
    return d->m_model;
}

void QXYModelMapper::setModel(QAbstractItemModel *model)
{
    if (model == 0)
        return;

    Q_D(QXYModelMapper);
    if (d->m_model) {
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::dataChanged,
                                   d,
                                   &QXYModelMapperPrivate::onModelUpdated);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsInserted,
                                   d,
                                   &QXYModelMapperPrivate::onModelRowsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::rowsRemoved,
                                   d,
                                   &QXYModelMapperPrivate::onModelRowsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsInserted,
                                   d,
                                   &QXYModelMapperPrivate::onModelColumnsAdded);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::columnsRemoved,
                                   d,
                                   &QXYModelMapperPrivate::onModelColumnsRemoved);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::modelReset,
                                   d,
                                   &QXYModelMapperPrivate::initializeXYFromModel);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::layoutChanged,
                                   d,
                                   &QXYModelMapperPrivate::initializeXYFromModel);
        QObjectPrivate::disconnect(d->m_model,
                                   &QAbstractItemModel::destroyed,
                                   d,
                                   &QXYModelMapperPrivate::handleModelDestroyed);
    }

    d->m_model = model;
    d->initializeXYFromModel();
    //    connect signals from the model
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::dataChanged,
                            d,
                            &QXYModelMapperPrivate::onModelUpdated);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::rowsInserted,
                            d,
                            &QXYModelMapperPrivate::onModelRowsAdded);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::rowsRemoved,
                            d,
                            &QXYModelMapperPrivate::onModelRowsRemoved);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::columnsInserted,
                            d,
                            &QXYModelMapperPrivate::onModelColumnsAdded);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::columnsRemoved,
                            d,
                            &QXYModelMapperPrivate::onModelColumnsRemoved);

    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::modelReset,
                            d,
                            &QXYModelMapperPrivate::initializeXYFromModel);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::layoutChanged,
                            d,
                            &QXYModelMapperPrivate::initializeXYFromModel);
    QObjectPrivate::connect(d->m_model,
                            &QAbstractItemModel::destroyed,
                            d,
                            &QXYModelMapperPrivate::handleModelDestroyed);
    Q_EMIT modelChanged();
}

QXYSeries *QXYModelMapper::series() const
{
    Q_D(const QXYModelMapper);
    return d->m_series;
}

void QXYModelMapper::setSeries(QXYSeries *series)
{
    Q_D(QXYModelMapper);
    if (d->m_series) {
        QObjectPrivate::disconnect(d->m_series,
                                   &QXYSeries::pointAdded,
                                   d,
                                   &QXYModelMapperPrivate::onPointAdded);
        QObjectPrivate::disconnect(d->m_series,
                                   &QXYSeries::pointRemoved,
                                   d,
                                   &QXYModelMapperPrivate::onPointRemoved);
        QObjectPrivate::disconnect(d->m_series,
                                   &QXYSeries::pointReplaced,
                                   d,
                                   &QXYModelMapperPrivate::onPointReplaced);
        QObjectPrivate::disconnect(d->m_series,
                                   &QXYSeries::destroyed,
                                   d,
                                   &QXYModelMapperPrivate::handleSeriesDestroyed);
        QObjectPrivate::disconnect(d->m_series,
                                   &QXYSeries::pointsRemoved,
                                   d,
                                   &QXYModelMapperPrivate::onPointsRemoved);
    }

    if (series == 0)
        return;

    d->m_series = series;
    d->initializeXYFromModel();
    // connect the signals from the series
    QObjectPrivate::connect(d->m_series,
                            &QXYSeries::pointAdded,
                            d,
                            &QXYModelMapperPrivate::onPointAdded);
    QObjectPrivate::connect(d->m_series,
                            &QXYSeries::pointRemoved,
                            d,
                            &QXYModelMapperPrivate::onPointRemoved);
    QObjectPrivate::connect(d->m_series,
                            &QXYSeries::pointReplaced,
                            d,
                            &QXYModelMapperPrivate::onPointReplaced);
    QObjectPrivate::connect(d->m_series,
                            &QXYSeries::destroyed,
                            d,
                            &QXYModelMapperPrivate::handleSeriesDestroyed);
    QObjectPrivate::connect(d->m_series,
                            &QXYSeries::pointsRemoved,
                            d,
                            &QXYModelMapperPrivate::onPointsRemoved);
    Q_EMIT seriesChanged();
}

qsizetype QXYModelMapper::first() const
{
    Q_D(const QXYModelMapper);
    return d->m_first;
}

void QXYModelMapper::setFirst(qsizetype first)
{
    Q_D(QXYModelMapper);
    d->m_first = qMax(first, 0);
    d->initializeXYFromModel();
    Q_EMIT firstChanged();
}

qsizetype QXYModelMapper::count() const
{
    Q_D(const QXYModelMapper);
    return d->m_count;
}

void QXYModelMapper::setCount(qsizetype count)
{
    Q_D(QXYModelMapper);
    d->m_count = qMax(count, -1);
    d->initializeXYFromModel();
    Q_EMIT countChanged();
}

Qt::Orientation QXYModelMapper::orientation() const
{
    Q_D(const QXYModelMapper);
    return d->m_orientation;
}

void QXYModelMapper::setOrientation(Qt::Orientation orientation)
{
    Q_D(QXYModelMapper);
    d->m_orientation = orientation;
    d->initializeXYFromModel();
    Q_EMIT orientationChanged();
}

qsizetype QXYModelMapper::xSection() const
{
    Q_D(const QXYModelMapper);
    return d->m_xSection;
}

void QXYModelMapper::setXSection(qsizetype xSection)
{
    Q_D(QXYModelMapper);
    d->m_xSection = qMax(-1, xSection);
    d->initializeXYFromModel();
    Q_EMIT xSectionChanged();
}

qsizetype QXYModelMapper::ySection() const
{
    Q_D(const QXYModelMapper);
    return d->m_ySection;
}

void QXYModelMapper::setYSection(qsizetype ySection)
{
    Q_D(QXYModelMapper);
    d->m_ySection = qMax(-1, ySection);
    d->initializeXYFromModel();
    Q_EMIT ySectionChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QXYModelMapperPrivate::QXYModelMapperPrivate() {}

QXYModelMapperPrivate::~QXYModelMapperPrivate() {}

void QXYModelMapperPrivate::blockModelSignals(bool block)
{
    m_modelSignalsBlock = block;
}

void QXYModelMapperPrivate::blockSeriesSignals(bool block)
{
    m_seriesSignalsBlock = block;
}

QModelIndex QXYModelMapperPrivate::xModelIndex(qsizetype xIndex)
{
    if (m_count != -1 && xIndex >= m_count)
        return QModelIndex(); // invalid
    if (m_orientation == Qt::Vertical)
        return m_model->index(int(xIndex) + m_first, m_xSection);
    else
        return m_model->index(m_xSection, int(xIndex) + m_first);
}

QModelIndex QXYModelMapperPrivate::yModelIndex(qsizetype yIndex)
{
    if (m_count != -1 && yIndex >= m_count)
        return QModelIndex(); // invalid
    if (m_orientation == Qt::Vertical)
        return m_model->index(int(yIndex) + m_first, m_ySection);
    else
        return m_model->index(m_ySection, int(yIndex) + m_first);
}

qreal QXYModelMapperPrivate::valueFromModel(QModelIndex index)
{
    QVariant value = m_model->data(index, Qt::DisplayRole);
    switch (value.metaType().id()) {
    case QMetaType::QDateTime:
        return value.toDateTime().toMSecsSinceEpoch();
    case QMetaType::QDate:
        return value.toDate().startOfDay().toMSecsSinceEpoch();
    default:
        return value.toReal();
    }
}

void QXYModelMapperPrivate::setValueToModel(QModelIndex index, qreal value)
{
    QVariant oldValue = m_model->data(index, Qt::DisplayRole);
    switch (oldValue.metaType().id()) {
    case QMetaType::QDateTime:
        m_model->setData(index, QDateTime::fromMSecsSinceEpoch(value));
        break;
    case QMetaType::QDate:
        m_model->setData(index, QDateTime::fromMSecsSinceEpoch(value).date());
        break;
    default:
        m_model->setData(index, value);
    }
}

void QXYModelMapperPrivate::onPointAdded(qsizetype pointIndex)
{
    if (m_seriesSignalsBlock)
        return;

    if (m_count != -1)
        m_count += 1;

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->insertRows(int(pointIndex) + m_first, 1);
    else
        m_model->insertColumns(int(pointIndex) + m_first, 1);

    setValueToModel(xModelIndex(pointIndex), m_series->points().at(pointIndex).x());
    setValueToModel(yModelIndex(pointIndex), m_series->points().at(pointIndex).y());
    blockModelSignals(false);
}

void QXYModelMapperPrivate::onPointRemoved(qsizetype pointIndex)
{
    if (m_seriesSignalsBlock)
        return;

    if (m_count != -1)
        m_count -= 1;

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->removeRow(int(pointIndex) + m_first);
    else
        m_model->removeColumn(int(pointIndex) + m_first);
    blockModelSignals(false);
}

void QXYModelMapperPrivate::onPointsRemoved(qsizetype pointIndex, qsizetype count)
{
    if (m_seriesSignalsBlock)
        return;

    m_count -= count;

    if (m_count < -1)
        m_count = -1;

    blockModelSignals();
    if (m_orientation == Qt::Vertical)
        m_model->removeRows(int(pointIndex) + m_first, int(count));
    else
        m_model->removeColumns(int(pointIndex) + m_first, int(count));
    blockModelSignals(false);
}

void QXYModelMapperPrivate::onPointReplaced(qsizetype pointIndex)
{
    if (m_seriesSignalsBlock)
        return;

    blockModelSignals();
    setValueToModel(xModelIndex(pointIndex), m_series->points().at(pointIndex).x());
    setValueToModel(yModelIndex(pointIndex), m_series->points().at(pointIndex).y());
    blockModelSignals(false);
}

void QXYModelMapperPrivate::handleSeriesDestroyed()
{
    m_series = 0;
}

void QXYModelMapperPrivate::onModelUpdated(QModelIndex topLeft, QModelIndex bottomRight)
{
    if (m_model == 0 || m_series == 0)
        return;

    if (m_modelSignalsBlock)
        return;

    Q_TRACE_SCOPE(QGraphs2DXYModelMapperOnModelUpdated);
    blockSeriesSignals();
    QModelIndex index;
    QPointF newPoint;
    int indexColumn = 0;
    int indexRow = 0;
    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        for (int column = topLeft.column(); column <= bottomRight.column(); column++) {
            index = topLeft.sibling(row, column);
            indexColumn = index.column();
            indexRow = index.row();
            if (m_orientation == Qt::Vertical
                && (indexColumn == m_xSection || indexColumn == m_ySection)) {
                if (indexRow >= m_first && (m_count == -1 || indexRow < m_first + m_count)) {
                    QModelIndex xIndex = xModelIndex(indexRow - m_first);
                    QModelIndex yIndex = yModelIndex(indexRow - m_first);
                    if (xIndex.isValid() && yIndex.isValid()) {
                        newPoint.setX(valueFromModel(xIndex));
                        newPoint.setY(valueFromModel(yIndex));
                        m_series->replace(indexRow - m_first, newPoint);
                    }
                }
            } else if (m_orientation == Qt::Horizontal
                       && (indexRow == m_xSection || indexRow == m_ySection)) {
                if (indexColumn >= m_first && (m_count == -1 || indexColumn < m_first + m_count)) {
                    QModelIndex xIndex = xModelIndex(indexColumn - m_first);
                    QModelIndex yIndex = yModelIndex(indexColumn - m_first);
                    if (xIndex.isValid() && yIndex.isValid()) {
                        newPoint.setX(valueFromModel(xIndex));
                        newPoint.setY(valueFromModel(yIndex));
                        m_series->replace(indexColumn - m_first, newPoint);
                    }
                }
            }
        }
    }
    blockSeriesSignals(false);
}

void QXYModelMapperPrivate::onModelRowsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        insertData(start, end);
    } else if (start <= m_xSection || start <= m_ySection) {
        // if the changes affect the map - reinitialize the xy
        initializeXYFromModel();
    }
    blockSeriesSignals(false);
}

void QXYModelMapperPrivate::onModelRowsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Vertical) {
        removeData(start, end);
    } else if (start <= m_xSection || start <= m_ySection) {
        // if the changes affect the map - reinitialize the xy
        initializeXYFromModel();
    }
    blockSeriesSignals(false);
}

void QXYModelMapperPrivate::onModelColumnsAdded(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        insertData(start, end);
    } else if (start <= m_xSection || start <= m_ySection) {
        // if the changes affect the map - reinitialize the xy
        initializeXYFromModel();
    }
    blockSeriesSignals(false);
}

void QXYModelMapperPrivate::onModelColumnsRemoved(QModelIndex parent, qsizetype start, qsizetype end)
{
    Q_UNUSED(parent);
    if (m_modelSignalsBlock)
        return;

    blockSeriesSignals();
    if (m_orientation == Qt::Horizontal) {
        removeData(start, end);
    } else if (start <= m_xSection || start <= m_ySection) {
        // if the changes affect the map - reinitialize the xy
        initializeXYFromModel();
    }
    blockSeriesSignals(false);
}

void QXYModelMapperPrivate::handleModelDestroyed()
{
    m_model = 0;
}

void QXYModelMapperPrivate::insertData(int start, int end)
{
    if (m_model == 0 || m_series == 0)
        return;

    if (m_count != -1 && start >= m_first + m_count) {
        return;
    } else {
        Q_TRACE_SCOPE(QGraphs2DXYModelMappeInsertData, start, end);
        int addedCount = end - start + 1;
        if (m_count != -1 && addedCount > m_count)
            addedCount = m_count;
        int first = qMax(start, m_first);
        int last = qMin(first + addedCount - 1,
                        m_orientation == Qt::Vertical ? m_model->rowCount() - 1
                                                      : m_model->columnCount() - 1);
        for (int i = first; i <= last; i++) {
            QPointF point;
            QModelIndex xIndex = xModelIndex(i - m_first);
            QModelIndex yIndex = yModelIndex(i - m_first);
            if (xIndex.isValid() && yIndex.isValid()) {
                point.setX(valueFromModel(xIndex));
                point.setY(valueFromModel(yIndex));
                m_series->insert(i - m_first, point);
            }
        }

        // remove excess of points (above m_count)
        if (m_count != -1 && m_series->points().size() > m_count) {
            for (qsizetype i = m_series->points().size() - 1; i >= m_count; i--)
                m_series->remove(m_series->points().at(i));
        }
    }
}

void QXYModelMapperPrivate::removeData(int start, int end)
{
    if (m_model == 0 || m_series == 0)
        return;

    int removedCount = end - start + 1;
    if (m_count != -1 && start >= m_first + m_count) {
        return;
    } else {
        Q_TRACE_SCOPE(QGraphs2DXYModelMappeRemoveData, start, end);
        int toRemove = qMin(int(m_series->count()),
                            removedCount); // first find how many items can actually be removed
        int first = qMax(start, m_first);  // get the index of the first item that will be removed.
        int last = qMin(first + toRemove - 1,
                        int(m_series->count()) + m_first
                            - 1); // get the index of the last item that will be removed.
        for (int i = last; i >= first; i--)
            m_series->remove(m_series->points().at(i - m_first));

        if (m_count != -1) {
            qsizetype itemsAvailable; // check how many are available to be added
            if (m_orientation == Qt::Vertical)
                itemsAvailable = m_model->rowCount() - m_first - m_series->count();
            else
                itemsAvailable = m_model->columnCount() - m_first - m_series->count();
            int toBeAdded = qMin(
                int(itemsAvailable),
                m_count
                    - int(m_series->count())); // add not more items than there is space left to be filled.
            qsizetype currentSize = m_series->count();
            if (toBeAdded > 0) {
                for (qsizetype i = m_series->count(); i < currentSize + toBeAdded; i++) {
                    QPointF point;
                    QModelIndex xIndex = xModelIndex(i);
                    QModelIndex yIndex = yModelIndex(i);
                    if (xIndex.isValid() && yIndex.isValid()) {
                        point.setX(valueFromModel(xIndex));
                        point.setY(valueFromModel(yIndex));
                        m_series->insert(i, point);
                    }
                }
            }
        }
    }
}

void QXYModelMapperPrivate::initializeXYFromModel()
{
    if (m_model == 0 || m_series == 0)
        return;

    Q_TRACE_SCOPE(QGraphs2DXYModelMappeInitXYFromModel);

    blockSeriesSignals();
    // clear current content
    m_series->clear();

    // create the initial points set
    int pointPos = 0;
    QModelIndex xIndex = xModelIndex(pointPos);
    QModelIndex yIndex = yModelIndex(pointPos);
    if (xIndex.isValid() && yIndex.isValid()) {
        QList<QPointF> temp;

        while (xIndex.isValid() && yIndex.isValid()) {
            QPointF point;
            point.setX(valueFromModel(xIndex));
            point.setY(valueFromModel(yIndex));
            temp.append(point);
            pointPos++;
            xIndex = xModelIndex(pointPos);
            yIndex = yModelIndex(pointPos);
            // Don't warn about invalid index after the first, those are valid and used to
            // determine when we should end looping.
        }
        QXYSeriesPrivate::get(m_series)->append(temp);
    } else {
        // Invalid index right off the bat means series will be left empty, so output a warning,
        // unless model is also empty
        int count = m_orientation == Qt::Vertical ? m_model->rowCount() : m_model->columnCount();
        if (count > 0) {
            if (!xIndex.isValid()) {
                qCWarning(lcModelMapper2D, "%ls Invalid X coordinate index in model mapper.",
                         qUtf16Printable(QString::fromUtf8(__func__)));
            } else if (!yIndex.isValid()) {
                qCWarning(lcModelMapper2D, "%ls Invalid Y coordinate index in model mapper.",
                         qUtf16Printable(QString::fromUtf8(__func__)));
            }
        }
    }

    blockSeriesSignals(false);
}

QT_END_NAMESPACE

#include "moc_qxymodelmapper.cpp"
