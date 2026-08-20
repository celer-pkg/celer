// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcategory3daxis.h"
#include "qquickgraphsscatter_p.h"
#include "qscatter3dseries_p.h"
#include "qvalue3daxis.h"
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QScatter3DSeries
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QScatter3DSeries class represents a data series in a 3D scatter
 * graph.
 *
 * This class manages the series-specific visual elements, as well as the series
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
 * QScatter3DSeries supports the following format tags for QAbstract3DSeries::setItemLabelFormat():
 * \table
 *   \row
 *     \li @xTitle    \li Title from x-axis
 *   \row
 *     \li @yTitle    \li Title from y-axis
 *   \row
 *     \li @zTitle    \li Title from z-axis
 *   \row
 *     \li @xLabel    \li Item value formatted using the format of the x-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @yLabel    \li Item value formatted using the format of the y-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @zLabel    \li Item value formatted using the format of the z-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}.
 *   \row
 *     \li @seriesName \li Name of the series
 * \endtable
 *
 * For example:
 * \snippet doc_src_qtgraphs.cpp labelformat-scatter
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmltype Scatter3DSeries
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \nativetype QScatter3DSeries
 * \inherits Abstract3DSeries
 * \brief Represents a data series in a 3D scatter graph.
 *
 * This type manages the series specific visual elements, as well as the series
 * data (via a data proxy).
 *
 * Scatter3DSeries supports the following format tags for itemLabelFormat:
 * \table
 *   \row
 *     \li @xTitle    \li Title from x-axis
 *   \row
 *     \li @yTitle    \li Title from y-axis
 *   \row
 *     \li @zTitle    \li Title from z-axis
 *   \row
 *     \li @xLabel    \li Item value formatted using the format of the x-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @yLabel    \li Item value formatted using the format of the y-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @zLabel    \li Item value formatted using the format of the z-axis.
 *                        For more information, see
 *                        \l{QValue3DAxis::labelFormat}{labelFormat}.
 *   \row
 *     \li @seriesName \li Name of the series
 * \endtable
 *
 * For a more complete description, see QScatter3DSeries.
 *
 * \sa {Qt Graphs Data Handling with 3D}
 */

/*!
 * \qmlproperty ScatterDataProxy Scatter3DSeries::dataProxy
 *
 * Sets the active data proxy. The series assumes ownership of any proxy set to
 * it and deletes any previously set proxy when a new one is added. The proxy
 * cannot be null or set to another series.
 */

/*!
 * \qmlproperty qsizetype Scatter3DSeries::selectedItem
 *
 * The item that is selected at the index in the data array of the series.
 * Only one item can be selected at a time.
 * To clear the selection from this series, the invalidSelectionIndex is set as the
 * index. If this series is added to a graph, the graph can adjust the selection
 * according to user interaction or if it becomes invalid. Selecting an item on
 * another added series will also clear the selection. Removing items from or
 * inserting items into the series before the selected item will adjust the
 * selection so that the same item will stay selected.
 *
 * \sa GraphsItem3D::clearSelection()
 */

/*!
 * \qmlproperty real Scatter3DSeries::itemSize
 *
 * Sets the item size for the series. The size must be between \c 0.0 and
 * \c 1.0. Setting the size to \c 0.0 causes the item size to be automatically
 * scaled based on the total number of items in all the series for the graph.
 * The preset default is \c 0.0.
 */

/*!
 * \qmlproperty qsizetype Scatter3DSeries::invalidSelectionIndex
 * A constant property providing an invalid index for selection. This index is
 * set to the selectedItem property to clear the selection from this series.
 *
 * \sa GraphsItem3D::clearSelection()
 */

/*!
 * \qmlproperty ScatterDataArray Scatter3DSeries::dataArray
 *
 * Holds the reference to the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the data array, a series must be created for
 * the relevant proxy.
 */

/*!
    \qmlsignal Scatter3DSeries::dataProxyChanged(ScatterDataProxy proxy)

    This signal is emitted when dataProxy changes to \a proxy.
*/

/*!
    \qmlsignal Scatter3DSeries::selectedItemChanged(qsizetype index)

    This signal is emitted when selectedItem changes to \a index.
*/

/*!
    \qmlsignal Scatter3DSeries::itemSizeChanged(float size)

    This signal is emitted when itemSize changes to \a size.
*/

/*!
    \qmlsignal Scatter3DSeries::dataArrayChanged(ScatterDataArray array)

    This signal is emitted when dataArray changes to \a array.
*/

/*!
 * Constructs a scatter 3D series with the parent \a parent.
 */
QScatter3DSeries::QScatter3DSeries(QObject *parent)
    : QAbstract3DSeries(*(new QScatter3DSeriesPrivate()), parent)
{
    Q_D(QScatter3DSeries);
    // Default proxy
    d->setDataProxy(new QScatterDataProxy);
}

/*!
 * Constructs a scatter 3D series with the data proxy \a dataProxy and the
 * parent \a parent.
 */
QScatter3DSeries::QScatter3DSeries(QScatterDataProxy *dataProxy, QObject *parent)
    : QAbstract3DSeries(*(new QScatter3DSeriesPrivate()), parent)
{
    Q_D(QScatter3DSeries);
    d->setDataProxy(dataProxy);
}

/*!
 * \property QScatter3DSeries::axisX
 * \since 6.11
 *
 * \brief Holds an additional X-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QScatter3DSeries::setAxisX(QValue3DAxis *axis) {
    Q_D(QScatter3DSeries);
    if (d->m_axisX == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisX(axis);
    emit axisXChanged(axis);
}

QValue3DAxis *QScatter3DSeries::axisX() const {
    Q_D(const QScatter3DSeries);
    return d->m_axisX;
}

void QScatter3DSeries::resetAxisX()
{
    Q_D(QScatter3DSeries);
    if (!d->m_axisX) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis X is not set";
        return;
    }
    d->resetAxisX();
    emit axisXChanged(nullptr);
}

/*!
 * \property QScatter3DSeries::axisY
 * \since 6.11
 *
 * \brief Holds an additional Y-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QScatter3DSeries::setAxisY(QValue3DAxis *axis) {
    Q_D(QScatter3DSeries);
    if (d->m_axisY == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisY(axis);
    emit axisYChanged(axis);
}

QValue3DAxis *QScatter3DSeries::axisY() const {
    Q_D(const QScatter3DSeries);
    return d->m_axisY;
}

void QScatter3DSeries::resetAxisY()
{
    Q_D(QScatter3DSeries);
    if (!d->m_axisY) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis Y is not set";
        return;
    }
    d->resetAxisY();
    emit axisYChanged(nullptr);
}

/*!
 * \property QScatter3DSeries::axisZ
 * \since 6.11
 *
 * \brief Holds an additional Z-axis for the series
 *  If an axis is given, the series will be fitted
 *  to the minimum and maximum values of the axis.
 */
void QScatter3DSeries::setAxisZ(QValue3DAxis *axis) {
    Q_D(QScatter3DSeries);
    if (d->m_axisZ == axis) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
        return;
    }
    d->setAxisZ(axis);
    emit axisZChanged(axis);
}

QValue3DAxis *QScatter3DSeries::axisZ() const {
    Q_D(const QScatter3DSeries);
    return d->m_axisZ;
}

void QScatter3DSeries::resetAxisZ()
{
    Q_D(QScatter3DSeries);
    if (!d->m_axisZ) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "axis Z is not set";
        return;
    }
    d->resetAxisZ();
    emit axisZChanged(nullptr);
}

/*!
 * \internal
 */
QScatter3DSeries::QScatter3DSeries(QScatter3DSeriesPrivate &d, QObject *parent)
    : QAbstract3DSeries(d, parent)
{}

/*!
 * Deletes the scatter 3D series.
 */
QScatter3DSeries::~QScatter3DSeries() {}

/*!
 * \property QScatter3DSeries::dataProxy
 *
 * \brief The active data proxy.
 *
 * Sets the active data proxy for the series to \a proxy. The series assumes
 * ownership of any proxy set to it and deletes any previously set proxy when
 * a new one is added. The \a proxy argument cannot be null or set to another
 * series.
 */
void QScatter3DSeries::setDataProxy(QScatterDataProxy *proxy)
{
    Q_D(QScatter3DSeries);
    d->setDataProxy(proxy);
}

QScatterDataProxy *QScatter3DSeries::dataProxy() const
{
    Q_D(const QScatter3DSeries);
    return static_cast<QScatterDataProxy *>(d->dataProxy());
}

/*!
 * \property QScatter3DSeries::selectedItem
 *
 * \brief The item that is selected in the series.
 *
 * Selects the item at the index \a index in the data array of the series.
 * Only one item can be selected at a time.
 *
 * To clear the selection from this series, invalidSelectionIndex() is set as \a
 * index. If this series is added to a graph, the graph can adjust the selection
 * according to user interaction or if it becomes invalid. Selecting an item on
 * another added series will also clear the selection.
 *
 * Removing items from or inserting items into the series before the selected item
 * will adjust the selection so that the same item will stay selected.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
void QScatter3DSeries::setSelectedItem(qsizetype index)
{
    Q_D(QScatter3DSeries);
    // Don't do this in private to avoid loops, as that is used for callback from
    // graph.
    if (d->m_graph)
        static_cast<QQuickGraphsScatter *>(d->m_graph)->setSelectedItem(index, this);
    else
        d->setSelectedItem(index);
}

qsizetype QScatter3DSeries::selectedItem() const
{
    Q_D(const QScatter3DSeries);
    return d->m_selectedItem;
}

/*!
 * \property QScatter3DSeries::itemSize
 *
 * \brief Item size for the series.
 *
 * The size must be between \c 0.0f and \c 1.0f. Setting the size to \c 0.0f
 * causes the item size to be automatically scaled based on the total number of
 * items in all the series for the graph.
 *
 * The preset default is \c 0.0f.
 */
void QScatter3DSeries::setItemSize(float size)
{
    Q_D(QScatter3DSeries);
    if (size < 0.0f || size > 1.0f) {
        qCWarning(lcProperties3D, "%s invalid size. Valid range for itemSize is 0.0f...1.0f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (size == d->m_itemSize) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), size);
        return;
    }
    d->setItemSize(size);
    emit itemSizeChanged(size);
}

float QScatter3DSeries::itemSize() const
{
    Q_D(const QScatter3DSeries);
    return d->m_itemSize;
}

/*!
 * \property QScatter3DSeries::dataArray
 *
 * \brief Data array for the series.
 *
 * Holds the reference to the data array.
 *
 * dataArrayChanged signal is emitted when data array is set, unless \a newDataArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the data array, a series must be created for
 * the relevant proxy.
 *
 * \sa clearArray()
 */
void QScatter3DSeries::setDataArray(const QScatterDataArray &newDataArray)
{
    Q_D(QScatter3DSeries);
    if (d->m_dataArray.isSharedWith(newDataArray)) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "newDataArray is the same than the old one";
        return;
    }
    d->setDataArray(newDataArray);
    emit dataArrayChanged(newDataArray);
}

/*!
 * Clears the data array.
 */
void QScatter3DSeries::clearArray()
{
    Q_D(QScatter3DSeries);
    d->clearArray();
}

const QScatterDataArray &QScatter3DSeries::dataArray() const &
{
    Q_D(const QScatter3DSeries);
    return d->m_dataArray;
}

QScatterDataArray QScatter3DSeries::dataArray() &&
{
    Q_D(QScatter3DSeries);
    return std::move(d->m_dataArray);
}

/*!
 * \property QScatter3DSeries::scaleArray
 *
 * \brief Scale array for the series.
 *
 * Holds the reference to the scale array.
 *
 * scaleArrayChanged signal is emitted when scale array is set, unless \a newScaleArray
 * is identical to the previous one.
 *
 * \note Before doing anything regarding the scale array, a series must be created for
 * the relevant proxy.
 *
 * \note If a scale is not specified for the data, a default value of (1.0,1.0,1.0)
 * will be used.
 *
 * \sa clearScaleArray()
 */
void QScatter3DSeries::setScaleArray(const QList<QVector3D> &newScaleArray)
{
    Q_D(QScatter3DSeries);
    if (d->m_scaleArray.isSharedWith(newScaleArray)) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "newScaleArray is the same than the old one";
        return;
    }
    d->setScaleArray(newScaleArray);
    emit scaleArrayChanged(newScaleArray);
}

/*!
 * Clears the scale array.
 */
void QScatter3DSeries::clearScaleArray()
{
    Q_D(QScatter3DSeries);
    d->clearScaleArray();
}

const QList<QVector3D> &QScatter3DSeries::scaleArray() const &
{
    Q_D(const QScatter3DSeries);
    return d->m_scaleArray;
}

QList<QVector3D> QScatter3DSeries::scaleArray() &&
{
    Q_D(QScatter3DSeries);
    return std::move(d->m_scaleArray);
}

/*!
 * Returns an invalid index for selection. This index is set to the selectedItem
 * property to clear the selection from this series.
 *
 * \sa Q3DGraphsWidgetItem::clearSelection()
 */
qsizetype QScatter3DSeries::invalidSelectionIndex()
{
    return QQuickGraphsScatter::invalidSelectionIndex();
}

// QScatter3DSeriesPrivate

QScatter3DSeriesPrivate::QScatter3DSeriesPrivate()
    : QAbstract3DSeriesPrivate(QAbstract3DSeries::SeriesType::Scatter)
    , m_selectedItem(QQuickGraphsScatter::invalidSelectionIndex())
    , m_itemSize(0.0f)
{
    m_itemLabelFormat = QStringLiteral("@xLabel, @yLabel, @zLabel");
    m_mesh = QAbstract3DSeries::Mesh::Sphere;
}

QScatter3DSeriesPrivate::~QScatter3DSeriesPrivate()
{
    clearArray();
}

void QScatter3DSeriesPrivate::setDataProxy(QAbstractDataProxy *proxy)
{
    Q_Q(QScatter3DSeries);
    Q_ASSERT(proxy->type() == QAbstractDataProxy::DataType::Scatter);

    QAbstract3DSeriesPrivate::setDataProxy(proxy);

    emit q->dataProxyChanged(static_cast<QScatterDataProxy *>(proxy));
}

void QScatter3DSeriesPrivate::connectGraphAndProxy(QQuickGraphsItem *newGraph)
{
    Q_Q(QScatter3DSeries);
    QScatterDataProxy *scatterDataProxy = static_cast<QScatterDataProxy *>(m_dataProxy);

    if (m_graph && scatterDataProxy) {
        // Disconnect old graph/old proxy
        QObject::disconnect(scatterDataProxy, 0, m_graph, 0);
        QObject::disconnect(q, 0, m_graph, 0);
    }

    if (newGraph && scatterDataProxy) {
        QQuickGraphsScatter *graph = static_cast<QQuickGraphsScatter *>(newGraph);
        QObject::connect(scatterDataProxy,
                         &QScatterDataProxy::arrayReset,
                         graph,
                         &QQuickGraphsScatter::handleArrayReset);
        QObject::connect(scatterDataProxy,
                         &QScatterDataProxy::itemsAdded,
                         graph,
                         &QQuickGraphsScatter::handleItemsAdded);
        QObject::connect(scatterDataProxy,
                         &QScatterDataProxy::itemsChanged,
                         graph,
                         &QQuickGraphsScatter::handleItemsChanged);
        QObject::connect(scatterDataProxy,
                         &QScatterDataProxy::itemsRemoved,
                         graph,
                         &QQuickGraphsScatter::handleItemsRemoved);
        QObject::connect(scatterDataProxy,
                         &QScatterDataProxy::itemsInserted,
                         graph,
                         &QQuickGraphsScatter::handleItemsInserted);
        QObject::connect(q,
                         &QScatter3DSeries::dataProxyChanged,
                         graph,
                         &QQuickGraphsScatter::handleArrayReset);
    }
}

void QScatter3DSeriesPrivate::createItemLabel()
{
    Q_Q(QScatter3DSeries);
    static const QString xTitleTag(QStringLiteral("@xTitle"));
    static const QString yTitleTag(QStringLiteral("@yTitle"));
    static const QString zTitleTag(QStringLiteral("@zTitle"));
    static const QString xLabelTag(QStringLiteral("@xLabel"));
    static const QString yLabelTag(QStringLiteral("@yLabel"));
    static const QString zLabelTag(QStringLiteral("@zLabel"));
    static const QString seriesNameTag(QStringLiteral("@seriesName"));

    if (m_selectedItem == QScatter3DSeries::invalidSelectionIndex()) {
        m_itemLabel = QString(hiddenLabelTag);
        return;
    }

    QValue3DAxis *axisX = static_cast<QValue3DAxis *>(m_graph->axisX());
    QValue3DAxis *axisY = static_cast<QValue3DAxis *>(m_graph->axisY());
    QValue3DAxis *axisZ = static_cast<QValue3DAxis *>(m_graph->axisZ());
    QVector3D selectedPosition = q->dataProxy()->itemAt(m_selectedItem).position();

    m_itemLabel = m_itemLabelFormat;

    m_itemLabel.replace(xTitleTag, axisX->title());
    m_itemLabel.replace(yTitleTag, axisY->title());
    m_itemLabel.replace(zTitleTag, axisZ->title());

    if (m_itemLabel.contains(xLabelTag)) {
        QString valueLabelText = axisX->formatter()->stringForValue(qreal(selectedPosition.x()),
                                                                    axisX->labelFormat());
        m_itemLabel.replace(xLabelTag, valueLabelText);
    }
    if (m_itemLabel.contains(yLabelTag)) {
        QString valueLabelText = axisY->formatter()->stringForValue(qreal(selectedPosition.y()),
                                                                    axisY->labelFormat());
        m_itemLabel.replace(yLabelTag, valueLabelText);
    }
    if (m_itemLabel.contains(zLabelTag)) {
        QString valueLabelText = axisZ->formatter()->stringForValue(qreal(selectedPosition.z()),
                                                                    axisZ->labelFormat());
        m_itemLabel.replace(zLabelTag, valueLabelText);
    }
    m_itemLabel.replace(seriesNameTag, m_name);
}

void QScatter3DSeriesPrivate::setSelectedItem(qsizetype index)
{
    Q_Q(QScatter3DSeries);
    if (index == m_selectedItem) {
        qCDebug(lcProperties3D, "%s Value is already set to: %" PRIdQSIZETYPE,
                qUtf8Printable(QLatin1String(__FUNCTION__)), index);
        return;
    }
    markItemLabelDirty();
    m_selectedItem = index;
    emit q->selectedItemChanged(m_selectedItem);
}

void QScatter3DSeriesPrivate::setItemSize(float size)
{
    m_itemSize = size;
    if (m_graph)
        m_graph->markSeriesVisualsDirty();
}

void QScatter3DSeriesPrivate::setDataArray(const QScatterDataArray &newDataArray)
{
    m_dataArray = newDataArray;
}

void QScatter3DSeriesPrivate::clearArray()
{
    m_dataArray.clear();
}

void QScatter3DSeriesPrivate::setScaleArray(const QList<QVector3D> &newScaleArray)
{
    m_scaleArray = newScaleArray;
}

void QScatter3DSeriesPrivate::clearScaleArray()
{
    m_scaleArray.clear();
}

void QScatter3DSeriesPrivate::setAxisX(QValue3DAxis *axis)
{
    m_axisX = axis;
}

void QScatter3DSeriesPrivate::resetAxisX()
{
    m_axisX = nullptr;
}

void QScatter3DSeriesPrivate::setAxisY(QValue3DAxis *axis)
{
    m_axisY = axis;
}

void QScatter3DSeriesPrivate::resetAxisY()
{
    m_axisY = nullptr;
}

void QScatter3DSeriesPrivate::setAxisZ(QValue3DAxis *axis)
{
    m_axisZ = axis;
}

void QScatter3DSeriesPrivate::resetAxisZ()
{
    m_axisZ = nullptr;
}

QT_END_NAMESPACE

#include "moc_qscatter3dseries.cpp"
