// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qmath.h>
#include "qabstract3daxis_p.h"
#include "qbar3dseries_p.h"
#include "qcategory3daxis_p.h"
#include "qquickgraphsbars_p.h"
#include "qvalue3daxis_p.h"
#include "qgraphs3dlogging_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
 * \class QBar3DSeries
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QBar3DSeries class represents a data series in a 3D bar graph.
 *
 * This class manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * Regarding the proxy-series relationship, it is crucial to highlight
 * a couple of key points. In this context, data is stored in series and
 * users can access the dataset through the series. This series is controlled
 * or represented by a proxy object. Thus, the proxy can be used to manage various
 * operations on the data and update the actual dataset. However, it is necessary
 * to create a series associated with this proxy to edit the dataset.
 *
 * If no data proxy is set explicitly for the series, the series creates a
 * default proxy. Setting another proxy will destroy the existing proxy and all
 * data added to the series.
 *
 * QBar3DSeries supports the following format tags for QAbstract3DSeries::setItemLabelFormat():
 * \table
 *   \row
 *     \li @rowTitle      \li Title from row axis
 *   \row
 *     \li @colTitle      \li Title from column axis
 *   \row
 *     \li @valueTitle    \li Title from value axis
 *   \row
 *     \li @rowIdx        \li Visible row index. Localized using the graph locale.
 *   \row
 *     \li @colIdx        \li Visible column index. Localized using the graph locale.
 *   \row
 *     \li @rowLabel      \li Label from row axis
 *   \row
 *     \li @colLabel      \li Label from column axis
 *   \row
 *     \li @valueLabel    \li Item value formatted using the format of the value
 *                            axis attached to the graph. For more information,
 *                            see \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @seriesName    \li Name of the series
 *   \row
 *     \li %<format spec> \li Item value in the specified format. Formatted
 *                            using the same rules as \l{QValue3DAxis::labelFormat}.
 * \endtable
 *
 * For example:
 * \snippet doc_src_qtgraphs.cpp labelformat
 *
 * \sa {Qt Graphs Data Handling with 3D}, Q3DGraphsWidgetItem::locale
 */

/*!
 * \qmltype Bar3DSeries
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QBar3DSeries
 * \inherits Abstract3DSeries
 * \brief Represents a data series in a 3D bar graph.
 *
 * This type manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * Bar3DSeries supports the following format tags for itemLabelFormat:
 * \table
 *   \row
 *     \li @rowTitle      \li Title from row axis
 *   \row
 *     \li @colTitle      \li Title from column axis
 *   \row
 *     \li @valueTitle    \li Title from value axis
 *   \row
 *     \li @rowIdx        \li Visible row index. Localized using the graph locale.
 *   \row
 *     \li @colIdx        \li Visible column index. Localized using the graph locale.
 *   \row
 *     \li @rowLabel      \li Label from row axis
 *   \row
 *     \li @colLabel      \li Label from column axis
 *   \row
 *     \li @valueLabel    \li Item value formatted using the format of the value
 *                            axis attached to the graph. For more information,
 *                            see \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @seriesName    \li Name of the series
 *   \row
 *     \li %<format spec> \li Item value in the specified format. Formatted
 *                            using the same rules as \l{QValue3DAxis::labelFormat}{labelFormat}.
 * \endtable
 *
 * For a more complete description, see QBar3DSeries.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty BarDataProxy Bar3DSeries::dataProxy
 *
 * The active data proxy. The series assumes ownership of any proxy set to
 * it and deletes any previously set proxy when a new one is added. The proxy
 * cannot be null or set to another series.
 */

/*!
 * \qmlproperty point Bar3DSeries::selectedBar
 *
 * The bar in the series that is selected.
 *
 * The position of the selected bar is specified as a row and column in the
 * data array of the series.
 *
 * Only one bar can be selected at a time.
 *
 * To clear the selection from this series, assign invalidSelectionPosition as the
 * position.
 *
 * If this series is added to a graph, the graph can adjust the selection
 * according to user interaction or if it becomes invalid. Selecting a bar on
 * another added series will also clear the selection.
 *
 * Removing rows from or inserting rows into the series before the row of the
 * selected bar will adjust the selection so that the same bar will stay
 * selected.
 *
 * \sa {GraphsItem3D::clearSelection()}{GraphsItem3D.clearSelection()}
 */

/*!
 * \qmlproperty point Bar3DSeries::invalidSelectionPosition
 * \readonly
 *
 * A constant property providing an invalid position for selection. This
 * position is assigned to the selectedBar property to clear the selection from this
 * series.
 *
 * \sa {GraphsItem3D::clearSelection()}{GraphsItem3D.clearSelection()}
 */

/*!
 * \qmlproperty real Bar3DSeries::meshAngle
 *
 * A convenience property for defining the series rotation angle in degrees.
 *
 * \note When reading this property, it is calculated from the
 * \l{Abstract3DSeries::meshRotation}{Abstract3DSeries.meshRotation} value
 * using floating point precision and always returns a value from zero to 360
 * degrees.
 *
 * \sa {Abstract3DSeries::meshRotation}{Abstract3DSeries.meshRotation}
 */

/*!
 * \qmlproperty list<Color> Bar3DSeries::rowColors
 * This property can be used to draw the rows of the series in different colors.
 * The \l{QGraphsTheme::colorStyle}{GraphsTheme.colorStyle} must be set to
 * \c Uniform to use this property.
 * \note If the property is set and the theme is changed,
 * the rowColors list is not cleared automatically.
 *
 * \sa QGraphsTheme::ColorStyle::Uniform
 */

/*!
 * \qmlproperty list Bar3DSeries::rowLabels
 *
 * The optional row labels for the array. Indexes in this array match the row
 * indexes in the data array.
 * If the list is shorter than the number of rows, all rows will not get labels.
 */

/*!
 * \qmlproperty list Bar3DSeries::columnLabels
 *
 * The optional column labels for the array. Indexes in this array match column
 * indexes in rows. If the list is shorter than the longest row, all columns
 * will not get labels.
 */

/*!
 * \qmlproperty BarDataArray Bar3DSeries::dataArray
 *
 * Holds the reference of the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the dataArray, a series must be created for
 * the relevant proxy.
 */

/*!
    \qmlsignal Bar3DSeries::dataProxyChanged(BarDataProxy proxy)

    This signal is emitted when dataProxy changes to \a proxy.
*/

/*!
    \qmlsignal Bar3DSeries::selectedBarChanged(point position)

    This signal is emitted when selectedBar changes to \a position.
*/

/*!
    \qmlsignal Bar3DSeries::meshAngleChanged(real angle)

    This signal is emitted when meshAngle changes to \a angle.
*/

/*!
    \qmlsignal Bar3DSeries::rowColorsChanged(list<color> rowcolors)

    This signal is emitted when rowColors changes to \a rowcolors.
*/

/*!
    \qmlsignal Bar3DSeries::rowLabelsChanged()

    This signal is emitted when row labels change.
*/

/*!
    \qmlsignal Bar3DSeries::columnLabelsChanged()

    This signal is emitted when column labels change.
*/

/*!
    \qmlsignal Bar3DSeries::dataArrayChanged(BarDataArray array)

    This signal is emitted when dataArray changes to \a array.
*/

/*!
 * Constructs a bar 3D series with the parent \a parent.
 */
QBar3DSeries::QBar3DSeries(QObject *parent)
    : QAbstract3DSeries(*(new QBar3DSeriesPrivate()), parent)
{
    Q_D(QBar3DSeries);
    // Default proxy
    d->setDataProxy(new QBarDataProxy);
    connectSignals();
}

/*!
 * Constructs a bar 3D series with the data proxy \a dataProxy and the parent
 * \a parent.
 */
QBar3DSeries::QBar3DSeries(QBarDataProxy *dataProxy, QObject *parent)
    : QAbstract3DSeries(*(new QBar3DSeriesPrivate()), parent)
{
    Q_D(QBar3DSeries);
    d->setDataProxy(dataProxy);
    connectSignals();
}

/*!
 * Deletes a bar 3D series.
 */
QBar3DSeries::~QBar3DSeries()
{
    clearArray();
}

/*!
 * \property QBar3DSeries::dataProxy
 *
 * \brief The active data proxy.
 *
 * The series assumes ownership of any proxy set to it and deletes any
 * previously set proxy when a new one is added. The proxy cannot be null or
 * set to another series.
 */
void QBar3DSeries::setDataProxy(QBarDataProxy *proxy)
{
    Q_D(QBar3DSeries);
    d->setDataProxy(proxy);
}

QBarDataProxy *QBar3DSeries::dataProxy() const
{
    Q_D(const QBar3DSeries);
    return static_cast<QBarDataProxy *>(d->dataProxy());
}

/*!
 * \property QBar3DSeries::selectedBar
 *
 * \brief The bar in the series that is selected.
 *
 */

/*!
 * Selects the bar at the \a position position, specified as a row and column in
 * the data array of the series.
 *
 * Only one bar can be selected at a time.
 *
 * To clear the selection from this series, invalidSelectionPosition() is set as
 * \a position.
 *
 * If this series is added to a graph, the graph can adjust the selection
 * according to user interaction or if it becomes invalid. Selecting a bar on
 * another added series will also clear the selection.
 *
 * Removing rows from or inserting rows into the series before the row of the
 * selected bar will adjust the selection so that the same bar will stay
 * selected.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
void QBar3DSeries::setSelectedBar(QPoint position)
{
    Q_D(QBar3DSeries);
    // Don't do this in private to avoid loops, as that is used for callback from
    // graph.
    if (d->m_graph)
        static_cast<QQuickGraphsBars *>(d->m_graph)->setSelectedBar(position, this, true);
    else
        d->setSelectedBar(position);
}

QPoint QBar3DSeries::selectedBar() const
{
    Q_D(const QBar3DSeries);
    return d->m_selectedBar;
}

/*!
 * Returns an invalid position for selection. This position is set to the
 * selectedBar property to clear the selection from this series.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
QPoint QBar3DSeries::invalidSelectionPosition()
{
    return QQuickGraphsBars::invalidSelectionPosition();
}

static inline float quaternionAngle(const QQuaternion &rotation)
{
    return qRadiansToDegrees(qAcos(rotation.scalar())) * 2.f;
}

/*!
  \property QBar3DSeries::meshAngle

  \brief The series rotation angle in degrees.

  Setting this property is equivalent to the following call:

  \code
  setMeshRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angle))
  \endcode

  \note When reading this property, it is calculated from the
        QAbstract3DSeries::meshRotation value using floating point precision
        and always returns a value from zero to 360 degrees.

  \sa QAbstract3DSeries::meshRotation
 */
void QBar3DSeries::setMeshAngle(float angle)
{
    setMeshRotation(QQuaternion::fromAxisAndAngle(upVector, angle));
}

float QBar3DSeries::meshAngle() const
{
    QQuaternion rotation = meshRotation();

    if (rotation.isIdentity() || rotation.x() != 0.0f || rotation.z() != 0.0f)
        return 0.0f;
    else
        return quaternionAngle(rotation);
}

/*!
 * \property QBar3DSeries::rowColors
 *
 * \brief The list of row colors in the series.
 *
 * This property can be used to color
 * the rows of the series in different colors.
 * The QGraphsTheme::ColorStyle must be set to
 * QGraphsTheme::ColorStyle::Uniform to use this property.
 *
 * \sa QGraphsTheme::ColorStyle::Uniform
 */
void QBar3DSeries::setRowColors(const QList<QColor> &colors)
{
    Q_D(QBar3DSeries);
    d->setRowColors(colors);
}

/*!
 * \property QBar3DSeries::valueColoringEnabled
 * \since 6.9
 *
 * \brief Use the given value to color the whole bar based on the range gradient
 *
 * This property can be used to color each
 * bar separately based on its value and given range gradient.
 * The QGraphsTheme::ColorStyle must be set to
 * QGraphsTheme::ColorStyle::RangeGradient to use this property.
 *
 * \sa QGraphsTheme::ColorStyle::RangeGradient
 */
void QBar3DSeries::setValueColoringEnabled(bool enabled)
{
    Q_D(QBar3DSeries);
    d->setValueColoringEnabled(enabled);
}

bool QBar3DSeries::isValueColoringEnabled() const
{
    Q_D(const QBar3DSeries);
    return d->m_valueColoring;
}

/*!
 * \property QBar3DSeries::dataArray
 *
 * \brief Data array for the series.
 *
 * Holds the reference of the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the dataArray, a series must be created for
 * the relevant proxy.
 *
 *\sa clearRow(qsizetype rowIndex)
 *
 *\sa clearArray()
 */
void QBar3DSeries::setDataArray(const QBarDataArray &newDataArray)
{
    Q_D(QBar3DSeries);
    if (d->m_dataArray.isSharedWith(newDataArray)) {
        qCDebug(lcProperties3D, "%s newDataArray is the same than the old one",
                qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }
    d->m_dataArray = newDataArray;
}

/*!
 * Clears the existing row in the array according to given \a rowIndex.
 */
void QBar3DSeries::clearRow(qsizetype rowIndex)
{
    Q_D(QBar3DSeries);
    d->clearRow(rowIndex);
}

/*!
 * Clears the existing array.
 */
void QBar3DSeries::clearArray()
{
    Q_D(QBar3DSeries);
    d->clearArray();
}

const QBarDataArray &QBar3DSeries::dataArray() const &
{
    Q_D(const QBar3DSeries);
    return d->m_dataArray;
}

QBarDataArray QBar3DSeries::dataArray() &&
{
    Q_D(QBar3DSeries);
    return std::move(d->m_dataArray);
}

/*!
 * \property QBar3DSeries::rowLabels
 *
 * \brief The optional row labels for the array.
 *
 * Indexes in this array match the row indexes in the data array.
 * If the list is shorter than the number of rows, all rows will not get labels.
 */
QStringList QBar3DSeries::rowLabels() const
{
    Q_D(const QBar3DSeries);
    return d->m_rowLabels;
}

void QBar3DSeries::setRowLabels(const QStringList &labels)
{
    Q_D(QBar3DSeries);
    if (rowLabels() == labels) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << labels;
        return;
    }
    d->setRowLabels(labels);
    emit rowLabelsChanged();
}

/*!
 * \property QBar3DSeries::columnLabels
 *
 * \brief The optional column labels for the array.
 *
 * Indexes in this array match column indexes in rows.
 * If the list is shorter than the longest row, all columns will not get labels.
 */
QStringList QBar3DSeries::columnLabels() const
{
    Q_D(const QBar3DSeries);
    return d->m_columnLabels;
}

void QBar3DSeries::setColumnLabels(const QStringList &labels)
{
    Q_D(QBar3DSeries);
    if (columnLabels() == labels) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << labels;
        return;
    }
    d->setColumnLabels(labels);
    emit columnLabelsChanged();
}

/*!
 * \property QBar3DSeries::customRowLabels
 * \since 6.11
 *
 * \brief The optional custom row labels for the array.
 *
 * These labels will override any labels generated by a proxy.
 * Indexes in this array match the row indexes in the data array.
 * If the list is shorter than the number of rows, all rows will not get labels.
 */
QStringList QBar3DSeries::customRowLabels() const
{
    Q_D(const QBar3DSeries);
    return d->m_customRowLabels;
}

void QBar3DSeries::setCustomRowLabels(const QStringList &labels)
{
    Q_D(QBar3DSeries);
    if (customRowLabels() == labels) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << labels;
        return;
    }
    d->setCustomRowLabels(labels);
    emit customRowLabelsChanged(labels);
}

/*!
 * \property QBar3DSeries::customColumnLabels
 * \since 6.11
 *
 * \brief The optional custom column labels for the array.
 *
 * These labels will override any labels generated by a proxy.
 * Indexes in this array match column indexes in rows.
 * If the list is shorter than the longest row, all columns will not get labels.
 */
QStringList QBar3DSeries::customColumnLabels() const
{
    Q_D(const QBar3DSeries);
    return d->m_customColumnLabels;
}

void QBar3DSeries::setCustomColumnLabels(const QStringList &labels)
{
    Q_D(QBar3DSeries);
    if (customColumnLabels() == labels) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << labels;
        return;
    }
    d->setCustomColumnLabels(labels);
    emit customColumnLabelsChanged(labels);
}

QList<QColor> QBar3DSeries::rowColors() const
{
    Q_D(const QBar3DSeries);
    return d->m_rowColors;
}

/*!
 * \property QBar3DSeries::rowAxis
 * \since 6.11
 *
 * \brief Holds an additional row axis for the series
 * \note The additional axis does not adjust the
 * fitting of the graph, but allows for an extra set of
 * labels and an axis grid.
 */
void QBar3DSeries::setRowAxis(QCategory3DAxis *axis)
{
    Q_D(QBar3DSeries);
    if (rowAxis() == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setRowAxis(axis);
    emit rowAxisChanged(axis);
}

QCategory3DAxis *QBar3DSeries::rowAxis() const
{
    Q_D(const QBar3DSeries);
    return d->m_rowAxis;
}

void QBar3DSeries::resetRowAxis()
{
    Q_D(QBar3DSeries);
    if (!d->m_rowAxis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "row axis not set";
        return;
    }
    d->resetRowAxis();
    emit rowAxisChanged(nullptr);
}


/*!
 * \property QBar3DSeries::valueAxis
 * \since 6.11
 *
 * \brief Holds an additional value axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QBar3DSeries::setValueAxis(QValue3DAxis *axis)
{
    Q_D(QBar3DSeries);
    if (valueAxis() == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setValueAxis(axis);
    emit valueAxisChanged(axis);
}

QValue3DAxis *QBar3DSeries::valueAxis() const
{
    Q_D(const QBar3DSeries);
    return d->m_valueAxis;
}

void QBar3DSeries::resetValueAxis()
{
    Q_D(QBar3DSeries);
    if (!d->m_valueAxis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value axis not set";
        return;
    }
    d->resetValueAxis();
    emit valueAxisChanged(nullptr);
}

/*!
 * \property QBar3DSeries::columnAxis
 * \since 6.11
 *
 * \brief Holds an additional column axis for the series
 * \note The additional axis does not adjust the
 * fitting of the graph, but allows for an extra set of
 * labels and an axis grid.
 */
void QBar3DSeries::setColumnAxis(QCategory3DAxis *axis)
{
    Q_D(QBar3DSeries);
    if (columnAxis() == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setColumnAxis(axis);
    emit columnAxisChanged(axis);
}

QCategory3DAxis *QBar3DSeries::columnAxis() const
{
    Q_D(const QBar3DSeries);
    return d->m_columnAxis;
}

void QBar3DSeries::resetColumnAxis()
{
    Q_D(QBar3DSeries);
    if (!d->m_columnAxis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "column axis not set";
        return;
    }
    d->resetColumnAxis();
    emit columnAxisChanged(nullptr);
}

/*!
 * \internal
 */
void QBar3DSeries::connectSignals()
{
    QObject::connect(this,
                     &QAbstract3DSeries::meshRotationChanged,
                     this,
                     &QBar3DSeries::handleMeshRotationChanged);
}

/*!
 * \internal
 */
void QBar3DSeries::handleMeshRotationChanged(const QQuaternion &rotation)
{
    emit meshAngleChanged(quaternionAngle(rotation));
}

// QBar3DSeriesPrivate

QBar3DSeriesPrivate::QBar3DSeriesPrivate()
    : QAbstract3DSeriesPrivate(QAbstract3DSeries::SeriesType::Bar)
    , m_selectedBar(QQuickGraphsBars::invalidSelectionPosition())
{
    m_itemLabelFormat = QStringLiteral("@valueLabel");
    m_mesh = QAbstract3DSeries::Mesh::BevelBar;
    m_valueColoring = false;
}

QBar3DSeriesPrivate::~QBar3DSeriesPrivate() {}

void QBar3DSeriesPrivate::fixRowLabels(qsizetype startIndex,
                                       qsizetype count,
                                       const QStringList &newLabels,
                                       bool isInsert)
{
    bool changed = false;
    qsizetype currentSize = m_rowLabels.size();

    qsizetype newSize = newLabels.size();
    if (startIndex >= currentSize) {
        // Adding labels past old label array, create empty strings to fill
        // intervening space
        if (newSize) {
            for (qsizetype i = currentSize; i < startIndex; i++)
                m_rowLabels << QString();
            // Doesn't matter if insert, append, or just change when there were no
            // existing strings, just append new strings.
            m_rowLabels << newLabels;
            changed = true;
        }
    } else {
        if (isInsert) {
            qsizetype insertIndex = startIndex;
            if (count)
                changed = true;
            for (qsizetype i = 0; i < count; i++) {
                if (i < newSize)
                    m_rowLabels.insert(insertIndex++, newLabels.at(i));
                else
                    m_rowLabels.insert(insertIndex++, QString());
            }
        } else {
            // Either append or change, replace labels up to array end and then add
            // new ones
            qsizetype lastChangeIndex = count + startIndex;
            qsizetype newIndex = 0;
            for (qsizetype i = startIndex; i < lastChangeIndex; i++) {
                if (i >= currentSize) {
                    // Label past the current size, so just append the new label
                    if (newSize < newIndex) {
                        changed = true;
                        m_rowLabels << newLabels.at(newIndex);
                    } else {
                        break; // No point appending empty strings, so just exit
                    }
                } else if (newSize > newIndex) {
                    // Replace existing label
                    if (m_rowLabels.at(i) != newLabels.at(newIndex)) {
                        changed = true;
                        m_rowLabels[i] = newLabels.at(newIndex);
                    }
                } else {
                    // No more new labels, so clear existing label
                    if (!m_rowLabels.at(i).isEmpty()) {
                        changed = true;
                        m_rowLabels[i] = QString();
                    }
                }
                newIndex++;
            }
        }
    }

    if (changed) {
        Q_Q(QBar3DSeries);
        emit q->rowLabelsChanged();
    }
}

void QBar3DSeriesPrivate::setDataProxy(QAbstractDataProxy *proxy)
{
    Q_ASSERT(proxy->type() == QAbstractDataProxy::DataType::Bar);
    Q_Q(QBar3DSeries);

    QAbstract3DSeriesPrivate::setDataProxy(proxy);

    emit q->dataProxyChanged(static_cast<QBarDataProxy *>(proxy));
}

void QBar3DSeriesPrivate::connectGraphAndProxy(QQuickGraphsItem *newGraph)
{
    Q_Q(QBar3DSeries);
    QBarDataProxy *barDataProxy = static_cast<QBarDataProxy *>(m_dataProxy);

    if (m_graph && barDataProxy) {
        // Disconnect old graph/old proxy
        QObject::disconnect(barDataProxy, 0, m_graph, 0);
        QObject::disconnect(q, 0, m_graph, 0);
    }

    if (newGraph && barDataProxy) {
        QQuickGraphsBars *graph = static_cast<QQuickGraphsBars *>(newGraph);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::arrayReset,
                         graph,
                         &QQuickGraphsBars::handleArrayReset);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::rowsAdded,
                         graph,
                         &QQuickGraphsBars::handleRowsAdded);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::rowsChanged,
                         graph,
                         &QQuickGraphsBars::handleRowsChanged);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::rowsRemoved,
                         graph,
                         &QQuickGraphsBars::handleRowsRemoved);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::rowsInserted,
                         graph,
                         &QQuickGraphsBars::handleRowsInserted);
        QObject::connect(barDataProxy,
                         &QBarDataProxy::itemChanged,
                         graph,
                         &QQuickGraphsBars::handleItemChanged);
        QObject::connect(q,
                         &QBar3DSeries::rowLabelsChanged,
                         graph,
                         &QQuickGraphsBars::handleDataRowLabelsChanged);
        QObject::connect(q,
                         &QBar3DSeries::columnLabelsChanged,
                         graph,
                         &QQuickGraphsBars::handleDataColumnLabelsChanged);
        QObject::connect(q,
                         &QBar3DSeries::dataProxyChanged,
                         graph,
                         &QQuickGraphsBars::handleArrayReset);
        QObject::connect(q,
                         &QBar3DSeries::rowColorsChanged,
                         graph,
                         &QQuickGraphsBars::handleRowColorsChanged);
    }
}

void QBar3DSeriesPrivate::createItemLabel()
{
    Q_Q(QBar3DSeries);
    static const QString rowIndexTag(QStringLiteral("@rowIdx"));
    static const QString rowLabelTag(QStringLiteral("@rowLabel"));
    static const QString rowTitleTag(QStringLiteral("@rowTitle"));
    static const QString colIndexTag(QStringLiteral("@colIdx"));
    static const QString colLabelTag(QStringLiteral("@colLabel"));
    static const QString colTitleTag(QStringLiteral("@colTitle"));
    static const QString valueTitleTag(QStringLiteral("@valueTitle"));
    static const QString valueLabelTag(QStringLiteral("@valueLabel"));
    static const QString seriesNameTag(QStringLiteral("@seriesName"));

    if (m_selectedBar == QBar3DSeries::invalidSelectionPosition()) {
        m_itemLabel = QString(hiddenLabelTag);
        return;
    }

    QLocale locale(QLocale::c());
    if (m_graph)
        locale = m_graph->locale();
    else
        return;

    QCategory3DAxis *categoryAxisZ = static_cast<QCategory3DAxis *>(m_graph->axisZ());
    QCategory3DAxis *categoryAxisX = static_cast<QCategory3DAxis *>(m_graph->axisX());
    QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(m_graph->axisY());
    qreal selectedBarValue = qreal(q->dataProxy()->itemAt(m_selectedBar).value());

    // Custom format expects printf format specifier. There is no tag for it.
    m_itemLabel = valueAxis->formatter()->stringForValue(selectedBarValue, m_itemLabelFormat);

    int selBarPosRow = m_selectedBar.x();
    int selBarPosCol = m_selectedBar.y();
    m_itemLabel.replace(rowIndexTag, locale.toString(selBarPosRow));
    if (categoryAxisZ->labels().size() > selBarPosRow)
        m_itemLabel.replace(rowLabelTag, categoryAxisZ->labels().at(selBarPosRow));
    else
        m_itemLabel.replace(rowLabelTag, QString());
    m_itemLabel.replace(rowTitleTag, categoryAxisZ->title());
    m_itemLabel.replace(colIndexTag, locale.toString(selBarPosCol));
    if (categoryAxisX->labels().size() > selBarPosCol)
        m_itemLabel.replace(colLabelTag, categoryAxisX->labels().at(selBarPosCol));
    else
        m_itemLabel.replace(colLabelTag, QString());
    m_itemLabel.replace(colTitleTag, categoryAxisX->title());
    m_itemLabel.replace(valueTitleTag, valueAxis->title());

    if (m_itemLabel.contains(valueLabelTag)) {
        QString valueLabelText = valueAxis->formatter()->stringForValue(selectedBarValue,
                                                                        valueAxis->labelFormat());
        m_itemLabel.replace(valueLabelTag, valueLabelText);
    }

    m_itemLabel.replace(seriesNameTag, m_name);
}

void QBar3DSeriesPrivate::setSelectedBar(QPoint position)
{
    Q_Q(QBar3DSeries);
    if (position == m_selectedBar) {
        qCDebug(lcProperties3D, "%s value is already set to: %d %d",
                qUtf8Printable(QLatin1String(__FUNCTION__)), position.x(), position.y());
        return;
    }

    markItemLabelDirty();
    m_selectedBar = position;
    emit q->selectedBarChanged(m_selectedBar);
}

void QBar3DSeriesPrivate::setRowColors(const QList<QColor> &colors)
{
    Q_Q(QBar3DSeries);
    if (m_rowColors == colors) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << colors;
        return;
    }

    m_rowColors = colors;
    emit q->rowColorsChanged(m_rowColors);
}

void QBar3DSeriesPrivate::setValueColoringEnabled(bool enabled)
{
    Q_Q(QBar3DSeries);
    if (m_valueColoring == enabled) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enabled;
        return;
    }
    m_valueColoring = enabled;
    emit q->valueColoringEnabledChanged(enabled);
}

void QBar3DSeriesPrivate::setDataArray(const QBarDataArray &newDataArray)
{
    m_dataArray = newDataArray;
}

void QBar3DSeriesPrivate::clearRow(qsizetype rowIndex)
{
    m_dataArray[rowIndex].clear();
}

void QBar3DSeriesPrivate::clearArray()
{
    m_dataArray.clear();
}

void QBar3DSeriesPrivate::setRowLabels(const QStringList &labels)
{
    m_rowLabels = labels;
}

void QBar3DSeriesPrivate::setColumnLabels(const QStringList &labels)
{
    m_columnLabels = labels;
}

void QBar3DSeriesPrivate::setCustomRowLabels(const QStringList &labels)
{
    m_customRowLabels = labels;
}

void QBar3DSeriesPrivate::setCustomColumnLabels(const QStringList &labels)
{
    m_customColumnLabels = labels;
}

void QBar3DSeriesPrivate::setRowAxis(QCategory3DAxis *axis)
{
    m_rowAxis = axis;
}

void QBar3DSeriesPrivate::resetRowAxis()
{
    m_rowAxis = nullptr;
}

void QBar3DSeriesPrivate::setValueAxis(QValue3DAxis *axis)
{
    m_valueAxis = axis;
}

void QBar3DSeriesPrivate::resetValueAxis()
{
    m_valueAxis = nullptr;
}

void QBar3DSeriesPrivate::setColumnAxis(QCategory3DAxis *axis)
{
    m_columnAxis = axis;
}

void QBar3DSeriesPrivate::resetColumnAxis()
{
    m_columnAxis = nullptr;
}

QT_END_NAMESPACE

#include "moc_qbar3dseries.cpp"
