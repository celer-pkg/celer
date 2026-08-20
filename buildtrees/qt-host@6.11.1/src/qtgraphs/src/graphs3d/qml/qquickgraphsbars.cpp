// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs3d/utils/qgraphs3dlogging_p.h"
#include "q3dscene.h"
#include "qbar3dseries.h"
#include "qbar3dseries_p.h"
#include "qbardataproxy_p.h"
#include "qcategory3daxis_p.h"
#include "qgraphsinputhandler_p.h"
#include "qgraphstheme.h"
#include "qquickgraphsbars_p.h"
#include "qquickgraphstexturedata_p.h"
#include "qvalue3daxis_p.h"
#include "qgraphs3dlogging_p.h"

#include <QColor>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>
#include <QtQuick/qquickitemgrabresult.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

#include <qtgraphs_tracepoints_p.h>

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "#include <graphs3d/utils/qgraphs3dnamespace.h>" \
                   "class qquickgraphsbars;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsSynch_entry_beforeGraphsItem);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsSynch_exit_beforeGraphsItem);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsSynch_entry_afterGraphsItem);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsSynch_exit_afterGraphsItem);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsUpdateGraph_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsUpdateGraph_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBars_entry, int seriesCount);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBars_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsLegacy_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsLegacy_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsDefault_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsGenerateBarsDefault_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsUpdateBarPositionsLegacy_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsUpdateBarPositionsLegacy_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsDoPicking_entry, float posX, float posY);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsDoPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsDoRayPicking_entry, float originX, float originY,
              float originZ, float directionX, float directionY, float directionZ);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsDoRayPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsCreateSliceView_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsCreateSliceView_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DBarsCreateOffscreenSliceView_entry, int requestedIndex, int sliceType);
Q_TRACE_POINT(qtgraphs, QGraphs3DBarsCreateOffscreenSliceView_exit);

/*!
 * \qmltype Bars3D
 * \inherits GraphsItem3D
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief 3D bar graph.
 *
 * This type enables developers to render bar graphs in 3D with Qt Quick.
 *
 * You will need to import Qt Graphs module to use this type:
 *
 * \snippet doc_src_qmlgraphs.cpp 0
 *
 * After that you can use Bars3D in your qml files:
 *
 * \snippet doc_src_qmlgraphs.cpp 1
 *
 * See \l{Simple Bar Graph} for more thorough usage example.
 *
 * \sa Bar3DSeries, ItemModelBarDataProxy, Scatter3D, Surface3D, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Category3DAxis Bars3D::rowAxis
 * The active row axis.
 *
 * If an axis is not given, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty ValueAxis3D Bars3D::valueAxis
 * The active value axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Category3DAxis Bars3D::columnAxis
 * The active column axis.
 *
 * If an axis is not given, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty bool Bars3D::multiSeriesUniform
 * Defines whether bars are to be scaled with proportions set to a single series bar even
 * if there are multiple series displayed. If set to \c {true}, \l{barSpacing}{bar spacing} will
 * be correctly applied only to the X-axis. Preset to \c false by default.
 */

/*!
 * \qmlproperty real Bars3D::barThickness
 * The bar thickness ratio between the X and Z dimensions. The value \c 1.0
 * means that the bars are as wide as they are deep, whereas \c 0.5
 * makes them twice as deep as they are wide.
 */

/*!
 * \qmlproperty size Bars3D::barSpacing
 * Bar spacing in X and Z dimensions.
 *
 * Preset to \c {(1.0, 1.0)} by default. Spacing is affected by the
 * barSpacingRelative property.
 */

/*!
 * \qmlproperty bool Bars3D::barSpacingRelative
 * Whether spacing is absolute or relative to bar thickness.
 *
 * If \c true, the value of \c 0.0 means that the bars are placed
 * side-to-side, \c 1.0 means that a space as wide as the thickness of one bar
 * is left between the bars, and so on. Preset to \c true.
 */

/*!
 * \qmlproperty size Bars3D::barSeriesMargin
 *
 * Margin between series columns in X and Z dimensions. Preset to \c {(0.0, 0.0)} by default.
 * Sensible values are on the range [0,1).
 */

/*!
 * \qmlproperty Bar3DSeries Bars3D::selectedSeries
 * \readonly
 *
 * The selected series or \c null. If \l {GraphsItem3D::selectionMode}{selectionMode} has
 * the \c SelectionMultiSeries flag set, this property holds the series that
 * owns the selected bar.
 */

/*!
 * \qmlproperty list<Bar3DSeries> Bars3D::seriesList
 * \qmldefault
 * The series of the graph.
 * By default, this property contains an empty list.
 * To set the series, either use the addSeries() function or define them as children of the graph.
 */

/*!
 * \qmlproperty Bar3DSeries Bars3D::primarySeries
 * The primary series of the graph. It
 * is used to determine the row and column axis labels when the labels are not explicitly
 * set to the axes.
 *
 * If the specified series is not yet added to the graph, setting it as the
 * primary series will also implicitly add it to the graph.
 *
 * If the primary series itself is removed from the graph, this property
 * resets to default.
 *
 * If the series is null, this property resets to default.
 * Defaults to the first added series or zero if no series are added to the graph.
 */

/*!
 * \qmlproperty real Bars3D::floorLevel
 *
 * The floor level for the bar graph in Y-axis data coordinates.
 *
 * The actual floor level will be restricted by the Y-axis minimum and maximum
 * values.
 * Defaults to zero.
 */

/*!
 * \qmlmethod void Bars3D::addSeries(Bar3DSeries series)
 * Adds the \a series to the graph. A graph can contain multiple series, but only one set of axes,
 * so the rows and columns of all series must match for the visualized data to be meaningful.
 * If the graph has multiple visible series, only the first one added will
 * generate the row or column labels on the axes in cases where the labels are not explicitly set
 * to the axes. If the newly added series has specified a selected bar, it will be highlighted and
 * any existing selection will be cleared. Only one added series can have an active selection.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Bars3D::removeSeries(Bar3DSeries series)
 * Remove the \a series from the graph.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Bars3D::insertSeries(int index, Bar3DSeries series)
 * Inserts the \a series into the position \a index in the series list.
 * If the \a series has already been added to the list, it is moved to the
 * new \a index.
 * \note When moving a series to a new \a index that is after its old index,
 * the new position in list is calculated as if the series was still in its old
 * index, so the final index is actually the \a index decremented by one.
 * \sa GraphsItem3D::hasSeries()
 */

/*!
 * \qmlmethod void Bars3D::renderSliceToImage(int requestedIndex, QtGraphs3D::SliceCaptureType sliceType, QUrl filePath)
 * \since 6.10
 *
 * Exports a 2d slice from series at \a requestedIndex and saves the result to an image
 * at a specified \a filePath.
 * The exported slice includes bars of row or column, which is defined by
 * \a sliceType.
 */

/*!
 * \qmlsignal Bars3D::multiSeriesUniformChanged(bool uniform)
 *
 * This signal is emitted when multiSeriesUniform changes to \a uniform.
 */

/*!
 * \qmlsignal Bars3D::barThicknessChanged(real thicknessRatio)
 *
 * This signal is emitted when barThickness changes to \a thicknessRatio.
 */

/*!
 * \qmlsignal Bars3D::barSpacingChanged(size spacing)
 *
 * This signal is emitted when barSpacing changes to \a spacing.
 */

/*!
 * \qmlsignal Bars3D::barSpacingRelativeChanged(bool relative)
 *
 * This signal is emitted when barSpacingRelative changes to \a relative.
 */

/*!
 * \qmlsignal Bars3D::barSeriesMarginChanged(size margin)
 *
 * This signal is emitted when barSeriesMargin changes to \a margin.
 */

/*!
 * \qmlsignal Bars3D::rowAxisChanged(Category3DAxis axis)
 *
 * This signal is emitted when rowAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3D::columnAxisChanged(Category3DAxis axis)
 *
 * This signal is emitted when columnAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3D::valueAxisChanged(ValueAxis3D axis)
 *
 * This signal is emitted when valueAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3D::primarySeriesChanged(Bar3DSeries series)
 *
 * This signal is emitted when primarySeries changes to \a series.
 */

/*!
 * \qmlsignal Bars3D::selectedSeriesChanged(Bar3DSeries series)
 *
 * This signal is emitted when selectedSeries changes to \a series.
 */

/*!
 * \qmlsignal Bars3D::floorLevelChanged(real level)
 *
 * This signal is emitted when floorLevel changes to \a level.
 */

QQuickGraphsBars::QQuickGraphsBars(QQuickItem *parent)
    : QQuickGraphsItem(parent)
{
    m_graphType = QAbstract3DSeries::SeriesType::Bar;
    setAxisX(0);
    setAxisY(0);
    setAxisZ(0);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFlags(ItemHasContents);
    clearSelection();
}

QQuickGraphsBars::~QQuickGraphsBars()
{
    removeBarModels();
    removeSlicedBarModels();
    if (m_grabresult)
        delete m_grabresult;
}

QCategory3DAxis *QQuickGraphsBars::rowAxis() const
{
    return static_cast<QCategory3DAxis *>(axisZ());
}

void QQuickGraphsBars::setRowAxis(QCategory3DAxis *axis)
{
    QQuickGraphsItem::setAxisZ(axis);
}

QValue3DAxis *QQuickGraphsBars::valueAxis() const
{
    return static_cast<QValue3DAxis *>(axisY());
}

void QQuickGraphsBars::setValueAxis(QValue3DAxis *axis)
{
    QQuickGraphsItem::setAxisY(axis);
}

QCategory3DAxis *QQuickGraphsBars::columnAxis() const
{
    return static_cast<QCategory3DAxis *>(axisX());
}

void QQuickGraphsBars::setColumnAxis(QCategory3DAxis *axis)
{
    QQuickGraphsItem::setAxisX(axis);
}

void QQuickGraphsBars::setMultiSeriesScaling(bool uniform)
{
    m_isMultiSeriesUniform = uniform;

    m_changeTracker.multiSeriesScalingChanged = true;
    emitNeedRender();
}

bool QQuickGraphsBars::multiSeriesScaling() const
{
    return m_isMultiSeriesUniform;
}

void QQuickGraphsBars::setMultiSeriesUniform(bool uniform)
{
    if (uniform == isMultiSeriesUniform()) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << uniform;
        return;
    }
    setMultiSeriesScaling(uniform);
    emit multiSeriesUniformChanged(uniform);
}

bool QQuickGraphsBars::isMultiSeriesUniform() const
{
    return multiSeriesScaling();
}

void QQuickGraphsBars::setBarSpecs(float thicknessRatio, QSizeF spacing, bool relative)
{
    m_barThicknessRatio = thicknessRatio;
    m_barSpacing = spacing;
    m_isBarSpecRelative = relative;

    m_changeTracker.barSpecsChanged = true;
    emitNeedRender();
}

void QQuickGraphsBars::setBarThickness(float thicknessRatio)
{
    if (thicknessRatio == barThickness() || thicknessRatio <= 0.f) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), thicknessRatio);
        return;
    }
    setBarSpecs(thicknessRatio, barSpacing(), isBarSpacingRelative());
    emit barThicknessChanged(thicknessRatio);
}

float QQuickGraphsBars::barThickness() const
{
    return m_barThicknessRatio;
}

void QQuickGraphsBars::setBarSpacing(QSizeF spacing)
{
    if (spacing == barSpacing()) {
        qCDebug(lcProperties3D) << __FUNCTION__
                << "value is already set to:" << spacing;
        return;
    }
    setBarSpecs(barThickness(), spacing, isBarSpacingRelative());
    emit barSpacingChanged(spacing);
}

QSizeF QQuickGraphsBars::barSpacing() const
{
    return m_barSpacing;
}

void QQuickGraphsBars::setBarSpacingRelative(bool relative)
{
    if (relative == isBarSpacingRelative()) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << relative;
        return;
    }
    setBarSpecs(barThickness(), barSpacing(), relative);
    emit barSpacingRelativeChanged(relative);
}

bool QQuickGraphsBars::isBarSpacingRelative() const
{
    return m_isBarSpecRelative;
}

void QQuickGraphsBars::setBarSeriesMargin(QSizeF margin)
{
    if (margin == barSeriesMargin()) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << margin;
        return;
    }
    m_barSeriesMargin = margin;
    m_changeTracker.barSeriesMarginChanged = true;
    emitNeedRender();
    emit barSeriesMarginChanged(barSeriesMargin());
}

QSizeF QQuickGraphsBars::barSeriesMargin() const
{
    return m_barSeriesMargin;
}

QList<QBar3DSeries *> QQuickGraphsBars::barSeriesList()
{
    QList<QBar3DSeries *> barSeriesList;
    for (QAbstract3DSeries *abstractSeries : std::as_const(m_seriesList)) {
        QBar3DSeries *barSeries = qobject_cast<QBar3DSeries *>(abstractSeries);
        if (barSeries)
            barSeriesList.append(barSeries);
    }

    return barSeriesList;
}

QQmlListProperty<QBar3DSeries> QQuickGraphsBars::seriesList()
{
    return QQmlListProperty<QBar3DSeries>(this,
                                          this,
                                          &QQuickGraphsBars::appendSeriesFunc,
                                          &QQuickGraphsBars::countSeriesFunc,
                                          &QQuickGraphsBars::atSeriesFunc,
                                          &QQuickGraphsBars::clearSeriesFunc);
}

void QQuickGraphsBars::appendSeriesFunc(QQmlListProperty<QBar3DSeries> *list, QBar3DSeries *series)
{
    reinterpret_cast<QQuickGraphsBars *>(list->data)->addSeries(series);
}

qsizetype QQuickGraphsBars::countSeriesFunc(QQmlListProperty<QBar3DSeries> *list)
{
    return reinterpret_cast<QQuickGraphsBars *>(list->data)->barSeriesList().size();
}

QBar3DSeries *QQuickGraphsBars::atSeriesFunc(QQmlListProperty<QBar3DSeries> *list, qsizetype index)
{
    return reinterpret_cast<QQuickGraphsBars *>(list->data)->barSeriesList().at(index);
}

void QQuickGraphsBars::clearSeriesFunc(QQmlListProperty<QBar3DSeries> *list)
{
    QQuickGraphsBars *declBars = reinterpret_cast<QQuickGraphsBars *>(list->data);
    QList<QBar3DSeries *> realList = declBars->barSeriesList();
    qsizetype count = realList.size();
    for (int i = 0; i < count; i++)
        declBars->removeSeries(realList.at(i));
}

void QQuickGraphsBars::addSeries(QBar3DSeries *series)
{
    insertSeries(m_seriesList.size(), series);
    connectSeries(series);
    if (series->selectedBar() != invalidSelectionPosition())
        updateSelectedBar();

    if (series->rowAxis())
        handleMultiAxisChanged(series->rowAxis());
    if (series->valueAxis())
        handleMultiAxisChanged(series->valueAxis());
    if (series->columnAxis())
        handleMultiAxisChanged(series->columnAxis());
}

void QQuickGraphsBars::removeSeries(QBar3DSeries *series)
{
    if (!series)
        return;

    bool wasVisible = (series && series->d_func()->m_graph == this && series->isVisible());

    QQuickGraphsItem::removeSeriesInternal(series);

    if (m_selectedBarSeries == series)
        setSelectedBar(invalidSelectionPosition(), 0, false);

    if (wasVisible)
        adjustAxisRanges();

    // If primary series is removed, reset it to default
    if (series == m_primarySeries) {
        if (m_seriesList.size())
            m_primarySeries = static_cast<QBar3DSeries *>(m_seriesList.at(0));
        else
            m_primarySeries = 0;

        handleDataRowLabelsChanged();
        handleDataColumnLabelsChanged();

        emit primarySeriesChanged(m_primarySeries);
    }

    for (auto it = m_slicedBarModels.begin(); it != m_slicedBarModels.end(); it++) {
        // Remove series also from slice bar list
        if (it.key() == series)
            m_slicedBarModels.remove(it.key());
    }

    removeBarModels();
    if (m_selectedBarSeries == series)
        resetClickedStatus();
    series->setParent(this); // Reparent as removing will leave series parentless
    disconnectSeries(series);
}

void QQuickGraphsBars::insertSeries(qsizetype index, QBar3DSeries *series)
{
    Q_ASSERT(series && series->type() == QAbstract3DSeries::SeriesType::Bar);

    qsizetype oldSize = m_seriesList.size();

    QQuickGraphsItem::insertSeries(index, series);

    if (oldSize != m_seriesList.size()) {
        QBar3DSeries *barSeries = static_cast<QBar3DSeries *>(series);
        if (!oldSize) {
            m_primarySeries = barSeries;
            handleDataRowLabelsChanged();
            handleDataColumnLabelsChanged();
        }

        if (barSeries->selectedBar() != invalidSelectionPosition())
            setSelectedBar(barSeries->selectedBar(), barSeries, false);

        if (!oldSize)
            emit primarySeriesChanged(m_primarySeries);
    }
}

void QQuickGraphsBars::clearSelection()
{
    setSelectedBar(invalidSelectionPosition(), 0, false);
}

void QQuickGraphsBars::setPrimarySeries(QBar3DSeries *series)
{
    if (!series) {
        if (m_seriesList.size())
            series = static_cast<QBar3DSeries *>(m_seriesList.at(0));
    } else if (!m_seriesList.contains(series)) {
        // Add nonexistent series.
        addSeries(series);
    }

    if (m_primarySeries != series) {
        m_primarySeries = series;
        handleDataRowLabelsChanged();
        handleDataColumnLabelsChanged();
        emit primarySeriesChanged(m_primarySeries);
    }
}

QBar3DSeries *QQuickGraphsBars::primarySeries() const
{
    return m_primarySeries;
}

QBar3DSeries *QQuickGraphsBars::selectedSeries() const
{
    return m_selectedBarSeries;
}

void QQuickGraphsBars::setSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    if (mode.testFlag(QtGraphs3D::SelectionFlag::Slice)
        && (mode.testFlag(QtGraphs3D::SelectionFlag::Row)
            == mode.testFlag(QtGraphs3D::SelectionFlag::Column))) {
        qCWarning(lcProperties3D, "%s must specify one of either row or column selection mode"
                 "in conjunction with slicing mode.",
                 qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        QtGraphs3D::SelectionFlags oldMode = selectionMode();

        QQuickGraphsItem::setSelectionMode(mode);

        if (mode != oldMode) {
            // Refresh selection upon mode change to ensure slicing is correctly
            // updated according to series the visibility.
            setSelectedBar(m_selectedBar, m_selectedBarSeries, true);

            // Special case: Always deactivate slicing when changing away from slice
            // automanagement, as this can't be handled in setSelectedBar.
            if (!mode.testFlag(QtGraphs3D::SelectionFlag::Slice)
                && oldMode.testFlag(QtGraphs3D::SelectionFlag::Slice)) {
                scene()->setSlicingActive(false);
            }
        }
    }
}

void QQuickGraphsBars::handleAxisAutoAdjustRangeChangedInOrientation(
    QAbstract3DAxis::AxisOrientation orientation, bool autoAdjust)
{
    Q_UNUSED(orientation);
    Q_UNUSED(autoAdjust);
    adjustAxisRanges();
}

void QQuickGraphsBars::handleSeriesVisibilityChangedBySender(QObject *sender)
{
    QQuickGraphsItem::handleSeriesVisibilityChangedBySender(sender);

    // Visibility changes may require disabling slicing,
    // so just reset selection to ensure everything is still valid.
    setSelectedBar(m_selectedBar, m_selectedBarSeries, false);
}

void QQuickGraphsBars::handleItemLabelVisibleChangedBySender(bool visible, QObject *sender)
{
    auto series = static_cast<QBar3DSeries *>(sender);
    if (series == m_selectedBarSeries)
    {
        itemLabel()->setVisible(visible);
        if (auto label = sliceItemLabel(); label && isSlicingActive())
            label->setVisible(visible);
    }
}

void QQuickGraphsBars::handleAxisRangeChangedBySender(QObject *sender)
{
    // Data window changed
    if (sender == m_axisX || sender == m_axisZ) {
        if (sender == m_axisX)
            handleDataColumnLabelsChanged();
        if (sender == m_axisZ)
            handleDataRowLabelsChanged();
    }

    QQuickGraphsItem::handleAxisRangeChangedBySender(sender);

    setDataDirty(true);

    // Update selected bar - may be moved offscreen
    setSelectedBar(m_selectedBar, m_selectedBarSeries, false);
}

void QQuickGraphsBars::adjustAxisRanges()
{
    QCategory3DAxis *categoryAxisZ = static_cast<QCategory3DAxis *>(m_axisZ);
    QCategory3DAxis *categoryAxisX = static_cast<QCategory3DAxis *>(m_axisX);
    QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(m_axisY);

    bool adjustZ = (categoryAxisZ && categoryAxisZ->isAutoAdjustRange());
    bool adjustX = (categoryAxisX && categoryAxisX->isAutoAdjustRange());
    bool adjustY = (valueAxis && categoryAxisX && categoryAxisZ && valueAxis->isAutoAdjustRange());

    if (adjustZ || adjustX || adjustY) {
        qsizetype maxRowCount = 0;
        qsizetype maxColumnCount = 0;
        float minValue = 0.0f;
        float maxValue = 0.0f;

        // First figure out row and column counts
        qsizetype seriesCount = m_seriesList.size();
        if (adjustZ || adjustX) {
            for (qsizetype series = 0; series < seriesCount; series++) {
                const QBar3DSeries *barSeries = static_cast<QBar3DSeries *>(m_seriesList.at(series));
                if (barSeries->isVisible()) {
                    const QBarDataProxy *proxy = barSeries->dataProxy();

                    if (adjustZ && proxy) {
                        qsizetype rowCount = proxy->rowCount();
                        if (rowCount)
                            rowCount--;

                        maxRowCount = qMax(maxRowCount, rowCount);
                    }

                    if (adjustX && proxy) {
                        const QBarDataArray &array = barSeries->dataArray();
                        qsizetype columnCount = 0;
                        for (int i = 0; i < array.size(); i++) {
                            if (columnCount < array.at(i).size())
                                columnCount = array.at(i).size();
                        }
                        if (columnCount)
                            columnCount--;

                        maxColumnCount = qMax(maxColumnCount, columnCount);
                    }
                }
            }
            // Call private implementations of setRange to avoid unsetting auto adjust
            // flag
            if (adjustZ)
                categoryAxisZ->d_func()->setRange(0.0f, float(maxRowCount), true);
            if (adjustX)
                categoryAxisX->d_func()->setRange(0.0f, float(maxColumnCount), true);
        }

        // Now that we know the row and column ranges, figure out the value axis
        // range
        if (adjustY) {
            for (int series = 0; series < seriesCount; series++) {
                const QBar3DSeries *barSeries = static_cast<QBar3DSeries *>(m_seriesList.at(series));
                if (barSeries->isVisible()) {
                    const QBarDataProxy *proxy = barSeries->dataProxy();
                    if (adjustY && proxy) {
                        QPair<float, float> limits = proxy->d_func()
                                                         ->limitValues(categoryAxisZ->min(),
                                                                       categoryAxisZ->max(),
                                                                       categoryAxisX->min(),
                                                                       categoryAxisX->max());
                        if (!series) {
                            // First series initializes the values
                            minValue = limits.first;
                            maxValue = limits.second;
                        } else {
                            minValue = qMin(minValue, limits.first);
                            maxValue = qMax(maxValue, limits.second);
                        }
                    }
                }
            }

            if (maxValue < 0.0f)
                maxValue = 0.0f;
            if (minValue > 0.0f)
                minValue = 0.0f;
            if (minValue == 0.0f && maxValue == 0.0f) {
                // Only zero value values in data set, set range to something.
                minValue = 0.0f;
                maxValue = 1.0f;
            }
            valueAxis->d_func()->setRange(minValue, maxValue, true);
        }
    }
}

void QQuickGraphsBars::setFloorLevel(float level)
{
    if (level == floorLevel()) {
        qCDebug(lcProperties3D, "%s value is already set to: %.f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), level);
        return;
    }
    m_floorLevel = level;
    setDataDirty(true);
    m_changeTracker.floorLevelChanged = true;
    emitNeedRender();
    emit floorLevelChanged(level);
}

float QQuickGraphsBars::floorLevel() const
{
    return m_floorLevel;
}

QQuick3DViewport *QQuickGraphsBars::createOffscreenSliceView(int requestedIndex,
                                                             QtGraphs3D::SliceCaptureType sliceType)
{
    QQuick3DViewport *sliceView = QQuickGraphsItem::createOffscreenSliceView(sliceType);

    Q_TRACE_SCOPE(QGraphs3DBarsCreateOffscreenSliceView, requestedIndex,
                  static_cast<int>(sliceType));

    const bool isRow = (sliceType == QtGraphs3D::SliceCaptureType::RowImage);
    const bool isColumn = (sliceType == QtGraphs3D::SliceCaptureType::ColumnImage);

    QList<QBar3DSeries *> barSeriesList = this->barSeriesList();
    for (const auto &barSeries : std::as_const(barSeriesList)) {
        qsizetype newRowSize = qMin(barSeries->dataProxy()->rowCount() - m_minRow, m_newRows);
        qsizetype newColSize = 0;
        if (newRowSize) {
            const QBarDataRow *dataRow = &barSeries->dataProxy()->rowAt(m_minRow);
            if (dataRow) {
                qsizetype dataColIndex = m_minCol;
                newColSize = qMin(dataRow->size() - dataColIndex, m_newCols);
            }
        }

        if (!barSeries->isVisible())
            continue;

        if (requestedIndex < 0 || requestedIndex >= newRowSize || requestedIndex >= newColSize) {
            qCWarning(lcEvents3D, "%s the index is out of range. The render stops.",
                      qUtf8Printable(QLatin1String(__FUNCTION__)));
            sliceView->setVisible(false);
            sliceView->deleteLater();
            return nullptr;
        }

        qsizetype slicedBarListSize = -1;

        if (isRow)
            slicedBarListSize = newColSize;
        else if (isColumn)
            slicedBarListSize = newRowSize;

        if (slicedBarListSize < 0)
            return nullptr;

        QList<BarModel *> barList = *m_barModelsMap.value(barSeries);
        bool useGradient = barSeries->d_func()->isUsingGradient();
        bool rangeGradient =
                (useGradient
                 && barSeries->d_func()->m_colorStyle == QGraphsTheme::ColorStyle::RangeGradient);

        QQuick3DModel *model = nullptr;
        QList<BarItemHolder *> barItemHolderList;
        QList<BarItemHolder *> barItemList;
        QList<BarItemHolder *> selectedItems;
        if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
            model = new QQuick3DModel();
            model->setParent(sliceView->scene());
            model->setParentItem(sliceView->scene());
            model->setObjectName(QStringLiteral("BarModel"));
            QString fileName = getMeshFileName();
            if (fileName.isEmpty())
                fileName = barSeries->userDefinedMesh();

            model->setSource(QUrl(fileName));

            auto barInstancing = new BarInstancing;
            barInstancing->setParent(barSeries);
            model->setInstancing(barInstancing);

            BarModel *barListItem = barList.at(0);
            updateItemMaterial(model, useGradient, rangeGradient,
                               QStringLiteral(":/materials/BarsMaterialInstancing"));
            updateMaterialProperties(model, false, false, barListItem->texture,
                                     barSeries->baseColor());

            barItemList = barListItem->instancing->dataArray();
            for (const auto bih : std::as_const(barItemList)) {
                if (!((isRow && bih->coord.x() == requestedIndex)
                      || (isColumn && bih->coord.y() == requestedIndex)))
                    continue;

                BarItemHolder *selectedBih = new BarItemHolder();
                selectedBih->selectedBar = false;
                selectedBih->coord = bih->coord;
                selectedBih->rotation = bih->rotation;
                selectedBih->heightValue = bih->heightValue;
                selectedBih->position = bih->position;
                selectedBih->scale = bih->scale;

                selectedItems.push_back(selectedBih);
            }
            if (selectedItems.size() == 0)
                continue;
        }

        qsizetype index = 0;
        for (int ind = 0; ind < slicedBarListSize; ++ind) {
            if (isRow)
                index = (requestedIndex * barSeries->dataProxy()->colCount()) + ind;
            else
                index = requestedIndex + (ind * barSeries->dataProxy()->colCount());

            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                if (index > barList.size())
                    return nullptr;
                model = createDataItem(sliceView->scene(), barSeries);
                BarModel *barModel = barList.at(index);
                if (isRow) {
                    model->setPosition(QVector3D(barModel->model->x(), barModel->model->y(), 0.0f));
                } else {
                    model->setX(barModel->model->z() - (barList.at(0)->visualIndex * .1f));
                    model->setY(barModel->model->y());
                    model->setZ(0.0f);
                }
                model->setScale(barModel->model->scale());

                updateItemMaterial(model, useGradient, rangeGradient,
                                   QStringLiteral(":/materials/BarsMaterial"));

                updateMaterialProperties(model, false, false, barList.at(index)->texture,
                                         barSeries->baseColor());
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                BarModel *barModel = barList.at(0);
                BarItemHolder *itemHolder = new BarItemHolder();
                itemHolder->selectedBar = false;
                itemHolder->color = barSeries->baseColor();
                itemHolder->coord = barModel->coord;
                itemHolder->rotation = selectedItems.at(ind)->rotation;
                itemHolder->heightValue = barModel->heightValue;
                itemHolder->position = selectedItems.at(ind)->position;
                itemHolder->scale = selectedItems.at(ind)->scale;

                if (isRow) {
                    itemHolder->position.setZ(.0f);
                } else {
                    itemHolder->position.setX(itemHolder->position.z()
                                               - (barModel->visualIndex * .1f));
                    itemHolder->position.setZ(.0f);
                }

                barItemHolderList.push_back(itemHolder);
            }
        }

        if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
            BarInstancing *instancing = static_cast<BarInstancing *>(model->instancing());
            instancing->setDataArray(barItemHolderList);
        }
    }

    return sliceView;
}

void QQuickGraphsBars::renderSliceToImage(int requestedIndex,
                                          QtGraphs3D::SliceCaptureType sliceType)
{
    QQuick3DViewport *sliceView = createOffscreenSliceView(requestedIndex, sliceType);

    if (!m_grabresult)
        m_grabresult = new QImage();

    if (sliceView) {
        QSharedPointer<QQuickItemGrabResult> grabbed = sliceView->grabToImage();
        connect(grabbed.data(), &QQuickItemGrabResult::ready, this, [&, grabbed, sliceView]() {
            sliceView->setVisible(false);
            sliceView->deleteLater();
            *m_grabresult = grabbed.data()->image();
            emit sliceImageChanged(*m_grabresult);
        });
    }
}

void QQuickGraphsBars::renderSliceToImage(int requestedIndex,
                                          QtGraphs3D::SliceCaptureType sliceType,
                                          const QUrl &filePath)
{
    QQuick3DViewport *sliceView = createOffscreenSliceView(requestedIndex, sliceType);

    if (!sliceView)
        return;

    if (filePath.isEmpty()) {
        qCWarning(lcGraphsBars3D, "Save path is not defined.");
        sliceView->setVisible(false);
        sliceView->deleteLater();
        return;
    }

    QSharedPointer<QQuickItemGrabResult> grabbed = sliceView->grabToImage();
    connect(grabbed.data(), &QQuickItemGrabResult::ready, this, [grabbed, sliceView, filePath]() {
        if (!grabbed.data()->saveToFile(filePath))
            qCWarning(lcGraphsBars3D, "Saving requested slice view to image failed");
        sliceView->setVisible(false);
        sliceView->deleteLater();
    });

    return;
}

void QQuickGraphsBars::componentComplete()
{
    QQuickGraphsItem::componentComplete();

    auto wallBackground = background();
    QUrl wallUrl = QUrl(QStringLiteral("defaultMeshes/backgroundNoFloorMesh"));
    wallBackground->setSource(wallUrl);
    setBackground(wallBackground);

    QUrl floorUrl = QUrl(QStringLiteral(":/defaultMeshes/barMeshFull"));
    m_floorBackground = new QQuick3DModel();
    m_floorBackgroundScale = new QQuick3DNode();
    m_floorBackgroundRotation = new QQuick3DNode();

    m_floorBackgroundScale->setParent(graphNode());
    m_floorBackgroundScale->setParentItem(graphNode());

    m_floorBackgroundRotation->setParent(m_floorBackgroundScale);
    m_floorBackgroundRotation->setParentItem(m_floorBackgroundScale);

    m_floorBackground->setObjectName("Floor Background");
    m_floorBackground->setParent(m_floorBackgroundRotation);
    m_floorBackground->setParentItem(m_floorBackgroundRotation);

    m_floorBackground->setSource(floorUrl);
    m_floorBackground->setPickable(false);

    setFloorGridInRange(true);
    setVerticalSegmentLine(false);

    QObject::connect(cameraTarget(),
                     &QQuick3DNode::rotationChanged,
                     this,
                     &QQuickGraphsBars::handleCameraRotationChanged);

    graphsInputHandler()->setGraphsItem(this);
}

void QQuickGraphsBars::synchData()
{
    qCDebug(lcGraphs3D, "%s start of sync", qUtf8Printable(QLatin1String(__FUNCTION__)));

    Q_TRACE(QGraphs3DBarsSynch_entry_beforeGraphsItem);

    if (m_changeTracker.barSpecsChanged || !m_cachedBarThickness.isValid()) {
        updateBarSpecs(m_barThicknessRatio, m_barSpacing, m_isBarSpecRelative);
        m_changeTracker.barSpecsChanged = false;
    }

    // Floor level update requires data update, so do before qquickgraphicsitem sync
    if (m_changeTracker.floorLevelChanged) {
        updateFloorLevel(m_floorLevel);
        // Update labels and grid as well, unless we are in shader grid mode
        updateLabels();
        if (gridLineType() != QtGraphs3D::GridLineType::Shader)
            updateGrid();
        m_changeTracker.floorLevelChanged = false;
    }

    if (m_changeTracker.multiSeriesScalingChanged) {
        m_keepSeriesUniform = m_isMultiSeriesUniform;
        setSeriesVisualsDirty(true);
        m_changeTracker.multiSeriesScalingChanged = false;
    }

    // Do not clear dirty flag, we need to react to it in qquickgraphicsitem as well
    if (theme()->dirtyBits()->plotAreaBackgroundVisibilityDirty) {
        setSeriesVisualsDirty(true);
        for (auto it = m_barModelsMap.begin(); it != m_barModelsMap.end(); it++)
            it.key()->d_func()->m_changeTracker.meshChanged = true;
    }

    if (m_changeTracker.barSeriesMarginChanged) {
        updateBarSeriesMargin(barSeriesMargin());
        m_changeTracker.barSeriesMarginChanged = false;
    }

    if (m_changeTracker.axisRangeChanged) {
        adjustAxisRanges();
        m_changeTracker.axisRangeChanged = false;
    }
    Q_TRACE(QGraphs3DBarsSynch_exit_beforeGraphsItem);

    QQuickGraphsItem::synchData();

    Q_TRACE(QGraphs3DBarsSynch_entry_afterGraphsItem);

    if (m_selectedBarPos.isNull())
        itemLabel()->setVisible(false);

    // Draw floor
    if (m_changeTracker.floorChanged) {
        updateFloor();
        m_changeTracker.floorChanged = false;
    }

    Q_TRACE(QGraphs3DBarsSynch_exit_afterGraphsItem);
    qCDebug(lcGraphs3D, "%s end syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));
}

void QQuickGraphsBars::updateFloor()
{
    auto min = qMin(scaleWithBackground().x(), scaleWithBackground().z());
    m_floorBackgroundScale->setScale(QVector3D(
        scaleWithBackground().x() + m_hBackgroundMargin,
        min * gridOffset(),
        scaleWithBackground().z() + m_hBackgroundMargin));
    m_floorBackgroundScale->setPosition(QVector3D(0.0f, -m_backgroundAdjustment, 0.0f));

    QQuaternion m_xRightAngleRotation(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 90.0f));
    QQuaternion m_xRightAngleRotationNeg(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, -90.0f));

    if (isYFlipped())
        m_floorBackgroundRotation->setRotation(m_xRightAngleRotation);
    else
        m_floorBackgroundRotation->setRotation(m_xRightAngleRotationNeg);

    QQmlListReference materialsRefF(m_floorBackground, "materials");
    QQmlListReference bbRef(background(), "materials");
    QQuick3DCustomMaterial *bgMatFloor;
    if (!materialsRefF.size() && bbRef.size()) {
        bgMatFloor = static_cast<QQuick3DCustomMaterial *>(bbRef.at(0));
        materialsRefF.append(bgMatFloor);
    }
}

void QQuickGraphsBars::updateParameters()
{
    m_minRow = m_axisZ->min();
    m_maxRow = m_axisZ->max();
    m_minCol = m_axisX->min();
    m_maxCol = m_axisX->max();
    m_newRows = m_maxRow - m_minRow + 1;
    m_newCols = m_maxCol - m_minCol + 1;

    if (m_cachedRowCount != m_newRows || m_cachedColumnCount != m_newCols) {
        m_changeTracker.selectedBarChanged = true;
        m_cachedColumnCount = m_newCols;
        m_cachedRowCount = m_newRows;

        // Calculate max scene size
        float sceneRatio = qMin(float(m_newCols) / float(m_newRows),
                                float(m_newRows) / float(m_newCols));
        m_maxSceneSize = 2.0f * qSqrt(sceneRatio * m_newCols * m_newRows);

        if (m_cachedBarThickness.isValid())
            calculateSceneScalingFactors();
    }
    m_changeTracker.axisRangeChanged = true;
    setDataDirty(true);
}

void QQuickGraphsBars::updateFloorLevel(float level)
{
    setFloorLevel(level);
    calculateHeightAdjustment();
}

void QQuickGraphsBars::updateGraph()
{
    Q_TRACE_SCOPE(QGraphs3DBarsGenerateBarsUpdateGraph);

    QList<QBar3DSeries *> barSeriesAsList = barSeriesList();
    calculateSceneScalingFactors();

    for (const auto &series : std::as_const(barSeriesAsList)) {
        QAbstract3DSeriesChangeBitField &changeTracker = series->d_func()->m_changeTracker;
        if (changeTracker.meshChanged || changeTracker.meshSmoothChanged) {
            changeTracker.meshChanged = false;
            changeTracker.meshSmoothChanged = false;
            setDataDirty(true);
        }
    }

    if (isDataDirty()) {
        removeBarModels();
        generateBars(barSeriesAsList);
    }

    if (isSeriesVisualsDirty()) {
        int visualIndex = 0;
        for (const auto &barSeries : std::as_const(barSeriesAsList)) {
            if (barSeries->isVisible()) {
                updateBarVisuality(barSeries, visualIndex);
                updateBarPositions(barSeries);
                updateBarVisuals(barSeries);
                ++visualIndex;
            } else {
                updateBarVisuality(barSeries, -1);
            }
        }
    }

    // Needs to be done after data is set, as it needs to know the visual array.
    if (m_changeTracker.selectedBarChanged) {
        updateSelectedBar();
        setItemSelected(m_selectedBar != invalidSelectionPosition());
        if (isSliceEnabled()) {
            createSliceView();
            for (const auto &series : std::as_const(barSeriesAsList)) {
                bool visible = (m_selectedBarSeries == series) && series->isVisible();
                if (sliceView()->isVisible()) {
                    if (visible) {
                        removeSlicedBarModels();
                        createSliceView();
                        setSliceActivatedChanged(false);
                        toggleSliceGraph();
                        break;
                    } else {
                        setSliceActivatedChanged(true);
                    }
                } else {
                    if (visible) {
                        m_selectionDirty = true;
                        setSliceActivatedChanged(true);
                    }
                }
            }
        }
        m_changeTracker.selectedBarChanged = false;
    }

    setDataDirty(false);
    setSeriesVisualsDirty(false);
}

void QQuickGraphsBars::updateAxisRange(float min, float max)
{
    QQuickGraphsItem::updateAxisRange(min, max);
    calculateHeightAdjustment();
}

void QQuickGraphsBars::updateAxisReversed(bool enable)
{
    Q_UNUSED(enable);
    setSeriesVisualsDirty(true);
    calculateHeightAdjustment();
}

void QQuickGraphsBars::updateLightStrength()
{
    for (const auto list : std::as_const(m_barModelsMap)) {
        for (auto barModel : std::as_const(*list)) {
            QQmlListReference materialRef(barModel->model, "materials");
            if (materialRef.size()) {
                QQuick3DCustomMaterial *material = qobject_cast<QQuick3DCustomMaterial *>(
                    materialRef.at(0));
                material->setProperty("specularBrightness", lightStrength() * 0.05);
            }
        }
    }
}

void QQuickGraphsBars::calculateSceneScalingFactors()
{
    m_rowWidth = (m_cachedColumnCount * m_cachedBarSpacing.width()) * 0.5f;
    m_columnDepth = (m_cachedRowCount * m_cachedBarSpacing.height()) * 0.5f;
    m_maxDimension = qMax(m_rowWidth, m_columnDepth);
    m_scaleFactor = qMin((m_cachedColumnCount * (m_maxDimension / m_maxSceneSize)),
                         (m_cachedRowCount * (m_maxDimension / m_maxSceneSize)));

    // Single bar scaling
    m_xScale = m_cachedBarThickness.width() / m_scaleFactor;
    m_zScale = m_cachedBarThickness.height() / m_scaleFactor;

    // Adjust scaling according to margin
    m_xScale = m_xScale - m_xScale * m_cachedBarSeriesMargin.width();
    m_zScale = m_zScale - m_zScale * m_cachedBarSeriesMargin.height();

    // Whole graph scale factors
    m_xScaleFactor = m_rowWidth / m_scaleFactor;
    m_zScaleFactor = m_columnDepth / m_scaleFactor;

    if (margin() < 0.0f) {
        m_hBackgroundMargin = 0.0f;
        m_vBackgroundMargin = 0.0f;
    } else {
        m_hBackgroundMargin = margin();
        m_vBackgroundMargin = margin();
    }

    // Margin for a line to be fully visible on the edge in the grid shader
    const float halfLineWidth = 50.0;
    const float gridTextureSize = 4096.0;
    const float gridMargin = halfLineWidth / gridTextureSize;
    m_hBackgroundMargin += gridMargin;
    m_vBackgroundMargin += gridMargin;

    auto scale = QVector3D(m_xScaleFactor, 1.0f, m_zScaleFactor);
    setScaleWithBackground(scale);
    setBackgroundScaleMargin({m_hBackgroundMargin, m_vBackgroundMargin, m_hBackgroundMargin});
    setScale(scale);
    m_changeTracker.floorChanged = true;
}

void QQuickGraphsBars::calculateHeightAdjustment()
{
    float newAdjustment = 1.0f;
    m_actualFloorLevel = qBound(valueAxis()->min(), floorLevel(), valueAxis()->max());
    float maxAbs = qFabs(valueAxis()->max() - m_actualFloorLevel);

    // Check if we have negative values
    if (valueAxis()->min() < m_actualFloorLevel)
        m_hasNegativeValues = true;
    else if (valueAxis()->min() >= m_actualFloorLevel)
        m_hasNegativeValues = false;

    // Check first if user has set the ranges
    if (!isUserCameraRotationRange()) {
        // If not, adjust them automatically based on possibly existing negative values
        if (!m_noZeroInRange) {
            setMinCameraYRotation(-90.0f);
            setMaxCameraYRotation(90.0f);
        } else {
            if ((m_hasNegativeValues && !valueAxis()->reversed())
                || (!m_hasNegativeValues && valueAxis()->reversed())) {
                setMinCameraYRotation(-90.0f);
                setMaxCameraYRotation(0.0f);
            } else {
                setMinCameraYRotation(0.0f);
                setMaxCameraYRotation(90.0f);
            }
        }
        setUserCameraRotationRange(false);
    }

    if (valueAxis()->max() < m_actualFloorLevel) {
        m_heightNormalizer = float(qFabs(valueAxis()->min()) - qFabs(valueAxis()->max()));
        maxAbs = qFabs(valueAxis()->max()) - qFabs(valueAxis()->min());
    } else {
        m_heightNormalizer = float(valueAxis()->max() - valueAxis()->min());
    }

    // Height fractions are used in gradient calculations and are therefore
    // doubled Note that if max or min is exactly zero, we still consider it
    // outside the range
    if (valueAxis()->max() <= m_actualFloorLevel || valueAxis()->min() >= m_actualFloorLevel)
        m_noZeroInRange = true;
    else
        m_noZeroInRange = false;

    // Calculate translation adjustment for background floor
    newAdjustment = (qBound(0.0f, (maxAbs / m_heightNormalizer), 1.0f) - 0.5f) * 2.0f;
    if (valueAxis()->reversed())
        newAdjustment = -newAdjustment;

    if (newAdjustment != m_backgroundAdjustment)
        m_backgroundAdjustment = newAdjustment;
}

void QQuickGraphsBars::calculateSeriesStartPosition()
{
    m_seriesStart = -((float(m_visibleSeriesCount) - 1.0f) * 0.5f)
                    * (m_seriesStep - (m_seriesStep * m_cachedBarSeriesMargin.width()));
}

QVector3D QQuickGraphsBars::calculateCategoryLabelPosition(QAbstract3DAxis *axis,
                                                           QVector3D labelPosition,
                                                           int index)
{
    QVector3D ret = labelPosition;
    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::X) {
        float xPos = (index + 0.5f) * m_cachedBarSpacing.width();
        ret.setX((xPos - m_rowWidth) / m_scaleFactor);
    }
    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Z) {
        float zPos = (index + 0.5f) * m_cachedBarSpacing.height();
        ret.setZ((m_columnDepth - zPos) / m_scaleFactor);
    }
    ret.setY(-m_backgroundAdjustment);
    return ret;
}

float QQuickGraphsBars::calculateCategoryGridLinePosition(QAbstract3DAxis *axis, int index)
{
    float ret = 0.0f;
    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Z) {
        float colPos = index * -(m_cachedBarSpacing.height() / m_scaleFactor);
        ret = colPos + scale().z();
    }
    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::X) {
        float rowPos = index * (m_cachedBarSpacing.width() / m_scaleFactor);
        ret = rowPos - scale().x();
    }
    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Y)
        ret = -m_backgroundAdjustment;
    return ret;
}

void QQuickGraphsBars::handleAxisXChanged(QAbstract3DAxis *axis)
{
    emit columnAxisChanged(static_cast<QCategory3DAxis *>(axis));
}

void QQuickGraphsBars::handleAxisYChanged(QAbstract3DAxis *axis)
{
    emit valueAxisChanged(static_cast<QValue3DAxis *>(axis));
}

void QQuickGraphsBars::handleAxisZChanged(QAbstract3DAxis *axis)
{
    emit rowAxisChanged(static_cast<QCategory3DAxis *>(axis));
}

void QQuickGraphsBars::handleMultiAxisChanged(QAbstract3DAxis *axis)
{
    QQuickGraphsItem::handleMultiAxisChanged(axis);
}

QAbstract3DAxis *QQuickGraphsBars::getSeriesMultiAxis(QAbstract3DSeries *series,
                   QAbstract3DAxis::AxisOrientation orientation)
{
    QBar3DSeries *barSeries = qobject_cast<QBar3DSeries *>(series);

    if (!barSeries)
        return nullptr;

    QAbstract3DAxis *axis = nullptr;
    switch (orientation)  {
    case QAbstract3DAxis::AxisOrientation::X:
        axis = barSeries->rowAxis();
    break;
    case QAbstract3DAxis::AxisOrientation::Y:
        axis = barSeries->valueAxis();
    break;
    case QAbstract3DAxis::AxisOrientation::Z:
        axis = barSeries->columnAxis();
    break;
    case QAbstract3DAxis::AxisOrientation::None:
    break;
    }

    if (axis)
        axis->d_func()->setOrientation(orientation);

    return axis;
}

void QQuickGraphsBars::handleSeriesMeshChanged(QAbstract3DSeries::Mesh mesh)
{
    m_meshType = mesh;
}

void QQuickGraphsBars::handleMeshSmoothChanged(bool enable)
{
    m_smooth = enable;
}

void QQuickGraphsBars::handleCameraRotationChanged()
{
    updateLabels();
}

void QQuickGraphsBars::handleArrayReset()
{
    QBar3DSeries *series;
    if (qobject_cast<QBarDataProxy *>(sender()))
        series = static_cast<QBarDataProxy *>(sender())->series();
    else
        series = static_cast<QBar3DSeries *>(sender());

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
        series->d_func()->markItemLabelDirty();
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);
    // Clear selection unless still valid
    setSelectedBar(m_selectedBar, m_selectedBarSeries, false);
    series->d_func()->markItemLabelDirty();
    emitNeedRender();
}

void QQuickGraphsBars::handleRowsAdded(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QBar3DSeries *series = static_cast<QBarDataProxy *>(sender())->series();
    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);
    emitNeedRender();
}

void QQuickGraphsBars::handleRowsChanged(qsizetype startIndex, qsizetype count)
{
    QBar3DSeries *series = static_cast<QBarDataProxy *>(sender())->series();
    qsizetype oldChangeCount = m_changedRows.size();
    if (!oldChangeCount)
        m_changedRows.reserve(count);

    for (int i = 0; i < count; i++) {
        bool newItem = true;
        qsizetype candidate = startIndex + i;
        for (qsizetype j = 0; j < oldChangeCount; j++) {
            const ChangeRow &oldChangeItem = m_changedRows.at(j);
            if (oldChangeItem.row == candidate && series == oldChangeItem.series) {
                newItem = false;
                break;
            }
        }
        if (newItem) {
            ChangeRow newChangeItem = {series, candidate};
            m_changedRows.append(newChangeItem);
            if (series == m_selectedBarSeries && m_selectedBar.x() == candidate)
                series->d_func()->markItemLabelDirty();
        }
    }
    if (count) {
        m_changeTracker.rowsChanged = true;

        if (series->isVisible())
            adjustAxisRanges();

        // Clear selection unless still valid (row length might have changed)
        setSelectedBar(m_selectedBar, m_selectedBarSeries, false);
        emitNeedRender();
    }
}

void QQuickGraphsBars::handleRowsRemoved(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);

    QBar3DSeries *series = static_cast<QBarDataProxy *>(sender())->series();
    if (series == m_selectedBarSeries) {
        // If rows removed from selected series before the selection, adjust the
        // selection
        int selectedRow = m_selectedBar.x();
        if (startIndex <= selectedRow) {
            if ((startIndex + count) > selectedRow)
                selectedRow = -1; // Selected row removed
            else
                selectedRow -= count; // Move selected row down by amount of rows removed

            setSelectedBar(QPoint(selectedRow, m_selectedBar.y()), m_selectedBarSeries, false);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    emitNeedRender();
}

void QQuickGraphsBars::handleRowsInserted(qsizetype startIndex, qsizetype count)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(count);
    QBar3DSeries *series = static_cast<QBarDataProxy *>(sender())->series();
    if (series == m_selectedBarSeries) {
        // If rows inserted to selected series before the selection, adjust the
        // selection
        int selectedRow = m_selectedBar.x();
        if (startIndex <= selectedRow) {
            selectedRow += count;
            setSelectedBar(QPoint(selectedRow, m_selectedBar.y()), m_selectedBarSeries, false);
        }
    }

    if (series->isVisible()) {
        adjustAxisRanges();
        setDataDirty(true);
    }
    if (!m_changedSeriesList.contains(series))
        m_changedSeriesList.append(series);

    emitNeedRender();
}

void QQuickGraphsBars::handleItemChanged(qsizetype rowIndex, qsizetype columnIndex)
{
    QBar3DSeries *series = static_cast<QBarDataProxy *>(sender())->series();

    bool newItem = true;
    QPoint candidate((int(rowIndex)), (int(columnIndex)));
    for (ChangeItem item : std::as_const(m_changedItems)) {
        if (item.point == candidate && item.series == series) {
            newItem = false;
            setDataDirty(true);
            break;
        }
    }

    if (newItem) {
        ChangeItem newItem = {series, candidate};
        m_changedItems.append(newItem);
        m_changeTracker.itemChanged = true;

        if (series == m_selectedBarSeries && m_selectedBar == candidate)
            series->d_func()->markItemLabelDirty();
        if (series->isVisible())
            adjustAxisRanges();
    }
    emitNeedRender();
}

void QQuickGraphsBars::handleDataRowLabelsChanged()
{
    if (m_axisZ) {
        // Grab a sublist equal to data window (no need to have more labels in axis)
        int min = int(m_axisZ->min());
        int count = int(m_axisZ->max()) - min + 1;
        QStringList subList;
        if (m_primarySeries && m_primarySeries->dataProxy())
            subList = m_primarySeries->rowLabels().mid(min, count);
        static_cast<QCategory3DAxis *>(m_axisZ)->d_func()->setDataLabels(subList);
    }

    if (repeaterZ()) {
        updateParameters();
        repeaterZ()->setModel(m_axisZ->labels().size());
    }
}

void QQuickGraphsBars::handleDataColumnLabelsChanged()
{
    if (m_axisX) {
        // Grab a sublist equal to data window (no need to have more labels in axis)
        int min = int(m_axisX->min());
        int count = int(m_axisX->max()) - min + 1;
        QStringList subList;
        if (m_primarySeries && m_primarySeries->dataProxy())
            subList = m_primarySeries->columnLabels().mid(min, count);
        static_cast<QCategory3DAxis *>(m_axisX)->d_func()->setDataLabels(subList);
    }

    if (repeaterX()) {
        updateParameters();
        repeaterX()->setModel(m_axisX->labels().size());
    }
}

void QQuickGraphsBars::handleRowColorsChanged()
{
    setSeriesVisualsDirty(true);
    emitNeedRender();
}

void QQuickGraphsBars::handleValueColoringChanged()
{
    setSeriesVisualsDirty(true);
    emitNeedRender();
}

void QQuickGraphsBars::handleLightingModeChanged()
{
    setSeriesVisualsDirty(true);
    emitNeedRender();
}

void QQuickGraphsBars::connectSeries(QBar3DSeries *series)
{
    m_meshType = series->mesh();
    m_smooth = series->isMeshSmooth();

    QObject::connect(series,
                     &QBar3DSeries::meshChanged,
                     this,
                     &QQuickGraphsBars::handleSeriesMeshChanged);
    QObject::connect(series,
                     &QBar3DSeries::meshSmoothChanged,
                     this,
                     &QQuickGraphsBars::handleMeshSmoothChanged);
    QObject::connect(series->dataProxy(),
                     &QBarDataProxy::colCountChanged,
                     this,
                     &QQuickGraphsBars::handleDataColumnLabelsChanged);
    QObject::connect(series->dataProxy(),
                     &QBarDataProxy::rowCountChanged,
                     this,
                     &QQuickGraphsBars::handleDataRowLabelsChanged);
    QObject::connect(series,
                     &QBar3DSeries::rowColorsChanged,
                     this,
                     &QQuickGraphsBars::handleRowColorsChanged);
    QObject::connect(series,
                     &QBar3DSeries::valueColoringEnabledChanged,
                     this,
                     &QQuickGraphsBars::handleValueColoringChanged);
    QObject::connect(series,
                     &QBar3DSeries::rowAxisChanged,
                     this,
                     &QQuickGraphsBars::handleMultiAxisChanged);
    QObject::connect(series,
                     &QBar3DSeries::valueAxisChanged,
                     this,
                     &QQuickGraphsBars::handleMultiAxisChanged);
    QObject::connect(series,
                     &QBar3DSeries::columnAxisChanged,
                     this,
                     &QQuickGraphsBars::handleMultiAxisChanged);

}

void QQuickGraphsBars::disconnectSeries(QBar3DSeries *series)
{
    QObject::disconnect(series, 0, this, 0);
}

void QQuickGraphsBars::generateBars(QList<QBar3DSeries *> &barSeriesList)
{
    m_visibleSeriesCount = 0;
    Q_TRACE_SCOPE(QGraphs3DBarsGenerateBars, barSeriesList.size());

    for (const auto &barSeries : std::as_const(barSeriesList)) {
        QQuick3DTexture *texture = createTexture();
        texture->setParent(this);
        auto gradient = barSeries->baseGradient();
        auto textureData = static_cast<QQuickGraphsTextureData *>(texture->textureData());
        textureData->createGradient(gradient);

        bool visible = barSeries->isVisible();

        QList<BarModel *> *barList = m_barModelsMap.value(barSeries);

        if (!barList) {
            barList = new QList<BarModel *>;
            m_barModelsMap[barSeries] = barList;
        }

        if (barList->isEmpty()) {
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                QBarDataProxy *dataProxy = barSeries->dataProxy();
                qsizetype dataRowIndex = m_minRow;
                qsizetype newRowSize = qMin(dataProxy->rowCount() - dataRowIndex, m_newRows);
                Q_TRACE_SCOPE(QGraphs3DBarsGenerateBarsLegacy);

                for (int row = 0; row < newRowSize; ++row) {
                    const QBarDataRow &dataRow = dataProxy->rowAt(dataRowIndex);
                    if (!dataRow.isEmpty()) {
                        qsizetype dataColIndex = m_minCol;
                        qsizetype newColSize = qMin(dataRow.size() - dataColIndex, m_newCols);
                        for (int col = 0; col < newColSize; ++col) {
                            QBarDataItem &dataItem = const_cast<QBarDataItem &>(
                                dataRow.at(dataColIndex));
                            auto scene = graphNode();
                            QQuick3DModel *model = createDataItem(scene, barSeries);
                            model->setVisible(visible);

                            BarModel *barModel = new BarModel();
                            barModel->model = model;
                            barModel->barItem = &dataItem;
                            barModel->coord = QPoint(int(dataRowIndex), col);
                            barModel->texture = texture;

                            if (!barList->contains(barModel))
                                barList->append(barModel);

                            ++dataColIndex;
                        }
                        ++dataRowIndex;
                    }
                }
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                Q_TRACE_SCOPE(QGraphs3DBarsGenerateBarsDefault);

                auto scene = graphNode();
                BarModel *barInstancing = new BarModel();
                barInstancing->texture = texture;

                if (barInstancing->instancing == nullptr) {
                    barInstancing->instancing = new BarInstancing;
                    barInstancing->instancing->setParent(barSeries);
                    barInstancing->selectionInstancing = new BarInstancing;
                    barInstancing->selectionInstancing->setParent(barSeries);
                    barInstancing->multiSelectionInstancing = new BarInstancing;
                    barInstancing->multiSelectionInstancing->setParent(barSeries);
                }

                if (barInstancing->model == nullptr) {
                    barInstancing->model = createDataItem(scene, barSeries);
                    barInstancing->model->setInstancing(barInstancing->instancing);
                    barInstancing->model->setVisible(visible);
                    barInstancing->model->setPickable(true);
                    barInstancing->selectedModel = createDataItem(scene, barSeries);
                    barInstancing->selectedModel->setInstancing(barInstancing->selectionInstancing);
                    barInstancing->selectedModel->setVisible(visible);
                    barInstancing->selectedModel->setPickable(true);
                    barInstancing->multiSelectedModel = createDataItem(scene, barSeries);
                    barInstancing->multiSelectedModel->setInstancing(
                        barInstancing->multiSelectionInstancing);
                    barInstancing->multiSelectedModel->setVisible(visible);
                    barInstancing->multiSelectedModel->setPickable(true);
                }

                if (!barList->contains(barInstancing)) {
                    barList->append(barInstancing);
                    qCDebug(lcGraphs3D) << "bar info:"
                        << "\n model:" << barInstancing->model
                        << "\n series:" << barSeries
                        << "\n textureSize:" << barInstancing->texture->textureData()->size()
                        << "\n rowCount: " << barSeries->dataProxy()->rowCount()
                        << "\n columnCount:" << barSeries->dataProxy()->colCount()
                        << "\n baseColor" << barSeries->baseColor()
                        << "\n mesh:" << barSeries->mesh();
                }
            }

            markSeriesVisualsDirty();
        }

        if (barSeries->isVisible())
            m_visibleSeriesCount++;
    }
}

QQuick3DModel *QQuickGraphsBars::createDataItem(QQuick3DNode *scene, QAbstract3DSeries *series)
{
    auto model = new QQuick3DModel();
    model->setParent(scene);
    model->setParentItem(scene);
    model->setObjectName(QStringLiteral("BarModel"));
    QString fileName = getMeshFileName();
    if (fileName.isEmpty())
        fileName = series->userDefinedMesh();

    model->setSource(QUrl(fileName));
    return model;
}

QString QQuickGraphsBars::getMeshFileName()
{
    QString fileName = {};
    switch (m_meshType) {
    case QAbstract3DSeries::Mesh::Sphere:
        fileName = QStringLiteral("defaultMeshes/sphereMesh");
        break;
    case QAbstract3DSeries::Mesh::Bar:
    case QAbstract3DSeries::Mesh::Cube:
        fileName = QStringLiteral("defaultMeshes/barMesh");
        break;
    case QAbstract3DSeries::Mesh::Pyramid:
        fileName = QStringLiteral("defaultMeshes/pyramidMesh");
        break;
    case QAbstract3DSeries::Mesh::Cone:
        fileName = QStringLiteral("defaultMeshes/coneMesh");
        break;
    case QAbstract3DSeries::Mesh::Cylinder:
        fileName = QStringLiteral("defaultMeshes/cylinderMesh");
        break;
    case QAbstract3DSeries::Mesh::BevelBar:
    case QAbstract3DSeries::Mesh::BevelCube:
        fileName = QStringLiteral("defaultMeshes/bevelBarMesh");
        break;
    case QAbstract3DSeries::Mesh::UserDefined:
        break;
    default:
        fileName = QStringLiteral("defaultMeshes/sphereMesh");
    }

    fixMeshFileName(fileName, m_meshType);

    return fileName;
}

void QQuickGraphsBars::fixMeshFileName(QString &fileName, QAbstract3DSeries::Mesh meshType)
{
    // Should it be smooth?
    if (m_smooth && meshType != QAbstract3DSeries::Mesh::Point
        && meshType != QAbstract3DSeries::Mesh::UserDefined) {
        fileName += QStringLiteral("Smooth");
    }

    // Should it be filled?
    if (!theme()->isPlotAreaBackgroundVisible() && meshType != QAbstract3DSeries::Mesh::Sphere
        && meshType != QAbstract3DSeries::Mesh::Point
        && meshType != QAbstract3DSeries::Mesh::UserDefined) {
        fileName.append(QStringLiteral("Full"));
    }
}

void QQuickGraphsBars::updateBarVisuality(QBar3DSeries *series, int visualIndex)
{
    QList<BarModel *> barList = *m_barModelsMap.value(series);
    for (int i = 0; i < barList.count(); i++) {
        barList.at(i)->visualIndex = visualIndex;
        barList.at(i)->model->setVisible(series->isVisible());
        if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
            barList.at(0)->selectedModel->setVisible(false);
            barList.at(0)->multiSelectedModel->setVisible(false);
        }
    }

    m_changeTracker.selectedBarChanged = true;
    itemLabel()->setVisible(false);
}

void QQuickGraphsBars::updateBarPositions(QBar3DSeries *series)
{
    QBarDataProxy *dataProxy = series->dataProxy();

    m_seriesScaleX = 1.0f / float(m_visibleSeriesCount);
    m_seriesStep = 1.0f / float(m_visibleSeriesCount);
    m_seriesStart = -((float(m_visibleSeriesCount) - 1.0f) * 0.5f)
                    * (m_seriesStep - (m_seriesStep * m_cachedBarSeriesMargin.width()));

    if (m_keepSeriesUniform)
        m_seriesScaleZ = m_seriesScaleX;
    else
        m_seriesScaleZ = 1.0f;

    m_zeroPosition = valueAxis()->positionAt(m_actualFloorLevel);

    QList<BarModel *> barList = *m_barModelsMap.value(series);

    qsizetype dataRowIndex = m_minRow;
    qsizetype newRowSize = qMin(dataProxy->rowCount() - dataRowIndex, m_newRows);
    qsizetype row = 0;
    qsizetype dataColIndex = m_minCol;
    qsizetype newColSize = qMin(dataProxy->colCount() - dataColIndex, m_newCols);
    qsizetype col = 0;
    for (int i = 0; i < barList.count(); i++) {
        float seriesPos = m_seriesStart + 0.5f
                          + (m_seriesStep
                             * (barList.at(i)->visualIndex
                                - (barList.at(i)->visualIndex * m_cachedBarSeriesMargin.width())));
        if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
            Q_TRACE_SCOPE(QGraphs3DBarsUpdateBarPositionsLegacy);

            QBarDataItem *item = barList.at(i)->barItem;
            QQuick3DModel *model = barList.at(i)->model;
            float heightValue = updateBarHeightParameters(item, series->valueAxis());
            float angle = item->rotation();

            if (angle) {
                model->setRotation(QQuaternion::fromAxisAndAngle(upVector, angle)
                                   + series->meshRotation());
            } else {
                model->setRotation(QQuaternion() + series->meshRotation());
            }

            if (heightValue < 0.f) {
                const QVector3D rot = model->eulerRotation();
                model->setEulerRotation(QVector3D(-180.f, rot.y(), rot.z()));
            }

            float colPos = (col + seriesPos) * m_cachedBarSpacing.width();
            float xPos = (colPos - m_rowWidth) / m_scaleFactor;
            float rowPos = (row + 0.5f) * (m_cachedBarSpacing.height());
            float zPos = (m_columnDepth - rowPos) / m_scaleFactor;
            float yPos;

            if (heightValue < 0.f)
                yPos = heightValue - m_backgroundAdjustment - 0.015f;
            else
                yPos = heightValue - m_backgroundAdjustment + 0.015f;

            barList.at(i)->heightValue = heightValue;
            model->setPosition(QVector3D(xPos, yPos, zPos));

            if (heightValue == 0) {
                model->setScale(QVector3D(.0f, .0f, .0f));
                model->setPickable(false);
            } else {
                model->setScale(QVector3D(m_xScale * m_seriesScaleX,
                                          qAbs(heightValue),
                                          m_zScale * m_seriesScaleZ));
                model->setPickable(true);
            }

            if (col < newColSize - 1) {
                ++col;
            } else {
                col = 0;
                if (row < newRowSize - 1)
                    ++row;
                else
                    row = 0;
            }
        } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
            Q_TRACE_SCOPE(QGraphs3DBarsGenerateBarsDefault);

            deleteBarItemHolders(barList.at(i)->instancing);
            QList<BarItemHolder *> positions;
            for (int row = 0; row < newRowSize; ++row) {
                const QBarDataRow &dataRow = dataProxy->rowAt(dataRowIndex);
                if (!dataRow.isEmpty()) {
                    dataColIndex = m_minCol;
                    newColSize = qMin(dataRow.size() - dataColIndex, m_newCols);
                    for (int col = 0; col < newColSize; col++) {
                        const QBarDataItem &item = dataRow.at(dataColIndex);
                        float heightValue = updateBarHeightParameters(&item, series->valueAxis());
                        BarItemHolder *bih = new BarItemHolder();

                        if (barList.at(i)->model->eulerRotation() != QVector3D()
                            || series->meshRotation() != QQuaternion()) {
                            const QQuaternion rotation =
                                QQuaternion(barList.at(i)->model->eulerRotation().toVector4D())
                                + series->meshRotation();

                            bih->rotation = rotation;
                        } else {
                            bih->rotation = QQuaternion::fromEulerAngles(
                                QVector3D(.0f, item.rotation(), .0f));
                        }
                        if (heightValue < 0.f) {
                            // if bars need to be rendered negative(under the floor)
                            // construct a temporary quaternion(normalized) that rotates through
                            // angle (in degrees) around x axis and multiply rotation with it.
                            QQuaternion  rot = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 180.0f);
                            bih->rotation *= rot;
                        }

                        float colPos = (col + seriesPos) * m_cachedBarSpacing.width();
                        float xPos = (colPos - m_rowWidth) / m_scaleFactor;
                        float rowPos = (row + 0.5f) * (m_cachedBarSpacing.height());
                        float zPos = (m_columnDepth - rowPos) / m_scaleFactor;
                        float yPos;

                        if (heightValue < 0.f)
                            yPos = heightValue - m_backgroundAdjustment - 0.015f;
                        else
                            yPos = heightValue - m_backgroundAdjustment + 0.015f;

                        bih->position = {xPos, yPos, zPos};
                        bih->coord = QPoint(row, col);

                        if (heightValue == 0) {
                            bih->scale = {.0f, .0f, .0f};
                        } else {
                            bih->scale = {m_xScale * m_seriesScaleX,
                                          qAbs(heightValue),
                                          m_zScale * m_seriesScaleZ};
                        }

                        bih->heightValue = heightValue;
                        bih->selectedBar = false;

                        bool colorStyleIsUniform = (series->colorStyle()
                                                    == QGraphsTheme::ColorStyle::Uniform);
                        if (colorStyleIsUniform) {
                            QList<QColor> rowColors = series->rowColors();
                            if (rowColors.size() == 0) {
                                bih->color = series->baseColor();
                            } else {
                                int rowColorIndex = bih->coord.x() % rowColors.size();
                                bih->color = rowColors[rowColorIndex];
                            }
                        }

                        positions.push_back(bih);
                        dataColIndex++;
                    }
                }
                dataRowIndex++;
            }
            barList.at(i)->instancing->setDataArray(positions);
        }
    }
}

float QQuickGraphsBars::updateBarHeightParameters(const QBarDataItem *item, QValue3DAxis *axis)
{

    if (!axis)
        axis = valueAxis();
    float value = item->value();
    float heightValue = axis->positionAt(value);

    if (qIsNaN(heightValue))
        return 0.0f;

    if (m_noZeroInRange) {
        if (m_hasNegativeValues) {
            heightValue = -1.0f + heightValue;
            if (heightValue > 0.0f)
                heightValue = 0.0f;
        } else {
            if (heightValue < 0.0f)
                heightValue = 0.0f;
        }
    } else {
        heightValue -= m_zeroPosition;
    }

    if (axis->reversed())
        heightValue = -heightValue;

    return heightValue;
}

void QQuickGraphsBars::updateBarVisuals(QBar3DSeries *series)
{
    QList<BarModel *> barList = *m_barModelsMap.value(series);
    bool useGradient = series->d_func()->isUsingGradient();

    if (useGradient) {
        if (!m_hasHighlightTexture) {
            m_highlightTexture = createTexture();
            m_highlightTexture->setParent(this);
            m_multiHighlightTexture = createTexture();
            m_multiHighlightTexture->setParent(this);
            m_hasHighlightTexture = true;
        }
        auto highlightGradient = series->singleHighlightGradient();
        auto highlightTextureData = static_cast<QQuickGraphsTextureData *>(
            m_highlightTexture->textureData());
        highlightTextureData->createGradient(highlightGradient);
        auto multiHighlightGradient = series->multiHighlightGradient();
        auto multiHighlightTextureData = static_cast<QQuickGraphsTextureData *>(
            m_multiHighlightTexture->textureData());
        multiHighlightTextureData->createGradient(multiHighlightGradient);
    } else {
        if (m_hasHighlightTexture) {
            m_highlightTexture->deleteLater();
            m_multiHighlightTexture->deleteLater();
            m_hasHighlightTexture = false;
        }
    }

    bool rangeGradient = (useGradient
                          && series->d_func()->m_colorStyle == QGraphsTheme::ColorStyle::RangeGradient);
    QColor baseColor = series->baseColor();
    QColor barColor;
    QLinearGradient gradient = series->baseGradient();

    if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
        // Release resources that might not have been deleted even though deleteLater had been set

        if (m_customView)
            m_customView->window()->releaseResources();
        else
            window()->releaseResources();

        for (int i = 0; i < barList.count(); i++) {
            QQuick3DModel *model = barList.at(i)->model;
            auto textureData = static_cast<QQuickGraphsTextureData *>(
                barList.at(i)->texture->textureData());
            textureData->createGradient(series->baseGradient());
            const bool transparency = textureData->hasTransparency();
            updateItemMaterial(model,
                               useGradient,
                               rangeGradient,
                               QStringLiteral(":/materials/BarsMaterial"));
            if (useGradient) {
                updateMaterialProperties(model,
                                         false,
                                         false,
                                         barList.at(i)->texture,
                                         QColor(Qt::white),
                                         transparency);
            } else {
                QList<QColor> rowColors = series->rowColors();
                if (rowColors.size() == 0) {
                    barColor = baseColor;
                } else {
                    int rowColorIndex = barList.at(i)->coord.x() % rowColors.size();
                    barColor = rowColors[rowColorIndex];
                }
                const bool transparency = barColor.alphaF() < 1.0;
                updateMaterialProperties(model,
                                         false,
                                         false,
                                         barList.at(i)->texture,
                                         barColor,
                                         transparency);
            }
            QQmlListReference materialsRef(model, "materials");
            auto customMaterial = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
            customMaterial->setProperty("valueColoring", series->isValueColoringEnabled());
            customMaterial->setProperty("heightValue", barList.at(i)->heightValue);
            customMaterial->setProperty("shaded",
                                        series->lightingMode()
                                            == QAbstract3DSeries::LightingMode::Shaded);
        }
    } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        for (int i = 0; i < barList.count(); i++) {
            auto textureData = static_cast<QQuickGraphsTextureData *>(
                barList.at(i)->texture->textureData());
            textureData->createGradient(series->baseGradient());
            barList.at(i)->instancing->setTransparency(textureData->hasTransparency());
            updateItemMaterial(barList.at(i)->model,
                               useGradient,
                               rangeGradient,
                               QStringLiteral(":/materials/BarsMaterialInstancing"));
            if (useGradient) {
                updateMaterialProperties(barList.at(i)->model,
                                         false,
                                         false,
                                         barList.at(i)->texture,
                                         QColor(Qt::white),
                                         textureData->hasTransparency());
            } else {
                if (!barList.at(i)->instancing->dataArray().isEmpty()) {
                    const bool transparency
                        = barList.at(i)->instancing->dataArray().at(0)->color.alphaF() < 1.0;
                    updateMaterialProperties(barList.at(i)->model,
                                             false,
                                             false,
                                             barList.at(i)->texture,
                                             QColor(Qt::white),
                                             transparency);
                }
            }
            QQmlListReference materialsRef(barList.at(i)->model, "materials");
            auto customMaterial = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
            customMaterial->setProperty("valueColoring", series->isValueColoringEnabled());
            customMaterial->setProperty("rootScale", rootNode()->scale().y());
            customMaterial->setProperty("shaded",
                                        series->lightingMode()
                                            == QAbstract3DSeries::LightingMode::Shaded);
        }
    }
}

void QQuickGraphsBars::updateItemMaterial(QQuick3DModel *item,
                                          bool useGradient,
                                          bool rangeGradient,
                                          const QString &materialName)
{
    QQmlListReference materialsRef(item, "materials");

    bool needNewMaterial = false;
    if (!materialsRef.size())
        needNewMaterial = true;
    else if (materialsRef.at(0)->objectName().contains(QStringLiteral("Instancing"))
             == materialName.contains(QStringLiteral("Instancing"))) {
        needNewMaterial = true;
    }

    if (needNewMaterial) {
        materialsRef.clear();
        auto *material = createQmlCustomMaterial(materialName);
        material->setObjectName(materialName);
        material->setParent(item);
        materialsRef.append(material);
    }
    int colorStyle;
    if (!useGradient)
        colorStyle = 0; // style is uniform
    else if (!rangeGradient)
        colorStyle = 1; // style is objectGradient
    else
        colorStyle = 2; // style is rangeGradient

    auto *material = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    material->setProperty("colorStyle", colorStyle);
}

void QQuickGraphsBars::updateMaterialProperties(QQuick3DModel *item,
                                                const bool isHighlight,
                                                const bool isMultiHighlight,
                                                QQuick3DTexture *texture,
                                                QColor color,
                                                const bool transparency)
{
    QQmlListReference materialsRef(item, "materials");
    auto customMaterial = qobject_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    if (!customMaterial)
        return;
    customMaterial->setProperty("transparency", transparency);
    QVariant textureInputAsVariant = customMaterial->property("custex");
    QQuick3DShaderUtilsTextureInput *textureInput = textureInputAsVariant
                                                        .value<QQuick3DShaderUtilsTextureInput *>();

    int colorStyle = customMaterial->property("colorStyle").value<int>();
    if (colorStyle == 0) {
        customMaterial->setProperty("uniformColor", color);
    } else {
        if (!isHighlight && !isMultiHighlight)
            textureInput->setTexture(texture);
        else
            textureInput->setTexture(isHighlight ? m_highlightTexture : m_multiHighlightTexture);
        customMaterial->setProperty("isHighlight", isHighlight || isMultiHighlight);
    }
    customMaterial->setProperty("specularBrightness", lightStrength() * 0.05);
    customMaterial->setProperty("rootScale", rootNode()->scale().y());
}

void QQuickGraphsBars::removeBarModels()
{
    for (const auto list : std::as_const(m_barModelsMap)) {
        for (auto barModel : std::as_const(*list)) {
            deleteBarModels(barModel->model);
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                deleteBarItemHolders(barModel->instancing);
                deleteBarItemHolders(barModel->selectionInstancing);
                deleteBarItemHolders(barModel->multiSelectionInstancing);
                deleteBarModels(barModel->selectedModel);
                deleteBarModels(barModel->multiSelectedModel);
            }
            delete barModel;
        }
        delete list;
    }

    m_barModelsMap.clear();
    setSelectedBar(m_selectedBar, m_selectedBarSeries, false);
}

void QQuickGraphsBars::deleteBarModels(QQuick3DModel *model)
{
    if (model) {
        model->setPickable(false);
        model->setVisible(false);
        QQmlListReference materialsRef(model, "materials");
        if (materialsRef.size()) {
            auto material = materialsRef.at(0);
            delete material;
        }
        delete model;
    }
}

void QQuickGraphsBars::deleteBarItemHolders(BarInstancing *instancing)
{
    if (instancing) {
        const QList<BarItemHolder *> barItemList = instancing->dataArray();
        for (const auto bih : barItemList)
            delete bih;
        instancing->clearDataArray();
    }
}

QQuick3DTexture *QQuickGraphsBars::createTexture()
{
    QQuick3DTexture *texture = new QQuick3DTexture();
    texture->setParent(this);
    texture->setRotationUV(-90.0f);
    texture->setHorizontalTiling(QQuick3DTexture::ClampToEdge);
    texture->setVerticalTiling(QQuick3DTexture::ClampToEdge);
    QQuickGraphsTextureData *textureData = new QQuickGraphsTextureData();
    textureData->setParent(texture);
    textureData->setParentItem(texture);
    texture->setTextureData(textureData);

    return texture;
}

bool QQuickGraphsBars::doPicking(QPointF position)
{
    if (!QQuickGraphsItem::doPicking(position))
        return false;

    Q_TRACE_SCOPE(QGraphs3DBarsDoPicking, position.x(), position.y());

    m_selectionDirty = true;
    QList<QQuick3DPickResult> pickResults;
    if (m_customView)
        pickResults = m_customView->pickAll(position.x(), position.y());
    else
        pickResults = pickAll(position.x(), position.y());
    QQuick3DModel *selectedModel = nullptr;
    QVector3D instancePos = {.0f, .0f, .0f};
    if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::None)) {
        if (!pickResults.isEmpty()) {
            for (const auto &picked : std::as_const(pickResults)) {
                if (const auto &hit = picked.objectHit()) {
                    if (hit == backgroundBB() || hit == background()) {
                        resetClickedStatus();
                        continue;
                    } else if (hit->objectName().contains(QStringLiteral("BarModel"))) {
                        if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                            selectedModel = hit;
                            for (const auto barlist : std::as_const(m_barModelsMap)) {
                                for (const auto barModel : std::as_const(*barlist)) {
                                    if (barModel->model == selectedModel) {
                                        setSelectedBar(barModel->coord,
                                                       m_barModelsMap.key(barlist),
                                                       false);
                                    }
                                }
                            }
                            break;
                        } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                            BarInstancing *barIns = static_cast<BarInstancing *>(hit->instancing());
                            // Prevents to select bars with a height of 0 which affect picking.
                            if (!barIns->dataArray().isEmpty()
                                && barIns->dataArray().at(picked.instanceIndex())->heightValue
                                       != 0) {
                                selectedModel = hit;
                                instancePos = selectedModel->instancing()->instancePosition(
                                    picked.instanceIndex());
                                for (const auto barlist : std::as_const(m_barModelsMap)) {
                                    for (const auto barModel : std::as_const(*barlist)) {
                                        QList<BarItemHolder *> barItemList = barModel->instancing
                                                                                 ->dataArray();
                                        for (const auto bih : std::as_const(barItemList)) {
                                            if (bih->position == instancePos) {
                                                setSelectedBar(bih->coord,
                                                               m_barModelsMap.key(barlist),
                                                               false);
                                                qCDebug(lcInput3D) << "picking result:"
                                                    << "\n instance position:" << bih->position
                                                    << "\n picked item column and row position:"
                                                    << bih->coord
                                                    << "\n picked item value:"
                                                    << m_barModelsMap.key(barlist)->dataProxy()
                                                                   ->itemAt(bih->coord).value();
                                                if (isSliceEnabled())
                                                    setSliceActivatedChanged(true);
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    } else if (hit->objectName().contains(QStringLiteral("ElementAxis"))) {
                        QPoint coord = invalidSelectionPosition();
                        if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column)
                            && selectedAxis() == axisX()) {
                            // Use row from previous selection in case of row + column mode
                            int previousRow = qMax(0, m_selectedBar.x());
                            coord = QPoint(previousRow, selectedLabelIndex());
                        } else if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row)
                                   && selectedAxis() == axisZ()) {
                            // Use column from previous selection in case of row + column mode
                            int previousCol = qMax(0, m_selectedBar.y());
                            coord = QPoint(selectedLabelIndex(), previousCol);
                        }
                        for (auto it = m_barModelsMap.begin(); it != m_barModelsMap.end(); it++) {
                            if (it.key()->isVisible())
                                setSelectedBar(coord, it.key(), false);
                        }
                        break;
                    }
                }
            }
        } else {
            resetClickedStatus();
        }
    }
    return true;
}

bool QQuickGraphsBars::doRayPicking(QVector3D origin, QVector3D direction)
{
    if (!QQuickGraphsItem::doRayPicking(origin, direction))
        return false;

    Q_TRACE_SCOPE(QGraphs3DBarsDoRayPicking, origin.x(), origin.y(), origin.z(), direction.x(),
            direction.y(), direction.z());

    m_selectionDirty = true;
    QList<QQuick3DPickResult> pickResults;
    if (m_customView)
        pickResults = m_customView->rayPickAll(origin, direction);
    else
        pickResults = rayPickAll(origin, direction);
    QQuick3DModel *selectedModel = nullptr;
    QVector3D instancePos = {.0f, .0f, .0f};
    if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::None)) {
        if (!pickResults.isEmpty()) {
            for (const auto &picked : std::as_const(pickResults)) {
                if (const auto &hit = picked.objectHit()) {
                    if (hit == backgroundBB() || hit == background()) {
                        resetClickedStatus();
                        continue;
                    } else if (hit->objectName().contains(QStringLiteral("BarModel"))) {
                        if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                            selectedModel = hit;
                            for (const auto barlist : std::as_const(m_barModelsMap)) {
                                for (const auto barModel : std::as_const(*barlist)) {
                                    if (barModel->model == selectedModel) {
                                        setSelectedBar(barModel->coord,
                                                       m_barModelsMap.key(barlist),
                                                       false);
                                    }
                                }
                            }
                            break;
                        } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                            BarInstancing *barIns = static_cast<BarInstancing *>(hit->instancing());
                            // Prevents to select bars with a height of 0 which affect picking.
                            if (!barIns->dataArray().isEmpty()
                                && barIns->dataArray().at(picked.instanceIndex())->heightValue
                                       != 0) {
                                selectedModel = hit;
                                instancePos = selectedModel->instancing()->instancePosition(
                                    picked.instanceIndex());
                                for (const auto barlist : std::as_const(m_barModelsMap)) {
                                    for (const auto barModel : std::as_const(*barlist)) {
                                        QList<BarItemHolder *> barItemList = barModel->instancing
                                                                                 ->dataArray();
                                        for (const auto bih : std::as_const(barItemList)) {
                                            if (bih->position == instancePos) {
                                                setSelectedBar(bih->coord,
                                                               m_barModelsMap.key(barlist),
                                                               false);
                                                qCDebug(lcInput3D) << "picking result:"
                                                    << "\n instance position:" << bih->position
                                                    << "\n picked item column and row position:"
                                                    << bih->coord
                                                    << "\n picked item value:"
                                                    << m_barModelsMap.key(barlist)->dataProxy()
                                                                   ->itemAt(bih->coord).value();
                                                if (isSliceEnabled())
                                                    setSliceActivatedChanged(true);
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    } else if (hit->objectName().contains(QStringLiteral("ElementAxis"))) {
                        QPoint coord = invalidSelectionPosition();
                        if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column)
                            && selectedAxis() == axisX()) {
                            // Use row from previous selection in case of row + column mode
                            int previousRow = qMax(0, m_selectedBar.x());
                            coord = QPoint(previousRow, selectedLabelIndex());
                        } else if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row)
                                   && selectedAxis() == axisZ()) {
                            // Use column from previous selection in case of row + column mode
                            int previousCol = qMax(0, m_selectedBar.y());
                            coord = QPoint(selectedLabelIndex(), previousCol);
                        }
                        for (auto it = m_barModelsMap.begin(); it != m_barModelsMap.end(); it++) {
                            if (it.key()->isVisible())
                                setSelectedBar(coord, it.key(), false);
                        }
                        break;
                    }
                }
            }
        } else {
            resetClickedStatus();
        }
    }
    return true;
}

QAbstract3DAxis *QQuickGraphsBars::createDefaultAxis(QAbstract3DAxis::AxisOrientation orientation)
{
    QAbstract3DAxis *defaultAxis = 0;

    if (orientation == QAbstract3DAxis::AxisOrientation::Y)
        defaultAxis = createDefaultValueAxis();
    else
        defaultAxis = createDefaultCategoryAxis();

    return defaultAxis;
}

void QQuickGraphsBars::adjustSelectionPosition(QPoint &pos, const QBar3DSeries *series)
{
    const QBarDataProxy *proxy = 0;
    if (series)
        proxy = series->dataProxy();

    if (!proxy)
        pos = invalidSelectionPosition();

    if (pos != invalidSelectionPosition()) {
        qsizetype maxRow = proxy->rowCount() - 1;
        qsizetype maxCol = (pos.x() <= maxRow && pos.x() >= 0 && !proxy->rowAt(pos.x()).isEmpty())
                ? proxy->rowAt(pos.x()).size() - 1
                : -1;

        if (pos.x() < 0 || pos.x() > maxRow || pos.y() < 0 || pos.y() > maxCol)
            pos = invalidSelectionPosition();
    }
}

void QQuickGraphsBars::setSelectedBar(QPoint coord, QBar3DSeries *series, bool enterSlice)
{
    // If the selection targets non-existent bar, clear selection instead.
    QPoint pos = coord;

    // Series may already have been removed, so check it before setting the
    // selection.
    if (!m_seriesList.contains(series))
        series = nullptr;

    adjustSelectionPosition(pos, series);

    if (series && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Slice)) {
        // If the selected bar is outside data window, or there is no visible
        // selected bar, disable slicing.
        if (pos.x() < m_axisZ->min() || pos.x() > m_axisZ->max() || pos.y() < m_axisX->min()
            || pos.y() > m_axisX->max() || !series->isVisible()) {
            scene()->setSlicingActive(false);
        } else if (enterSlice) {
            scene()->setSlicingActive(true);
        }
        emitNeedRender();
    }

    if (pos != m_selectedBar || series != m_selectedBarSeries) {
        bool seriesChanged = (series != m_selectedBarSeries);
        m_selectedBar = pos;
        m_selectedBarSeries = series;
        m_changeTracker.selectedBarChanged = true;
        checkSliceEnabled();

        // Clear selection from other series and finally set new selection to the
        // specified series
        for (QAbstract3DSeries *otherSeries : std::as_const(m_seriesList)) {
            QBar3DSeries *barSeries = static_cast<QBar3DSeries *>(otherSeries);
            if (barSeries != m_selectedBarSeries)
                barSeries->d_func()->setSelectedBar(invalidSelectionPosition());
        }
        if (m_selectedBarSeries)
            m_selectedBarSeries->d_func()->setSelectedBar(m_selectedBar);

        if (seriesChanged)
            emit selectedSeriesChanged(m_selectedBarSeries);

        if (pos == invalidSelectionPosition())
            resetClickedStatus();

        setSeriesVisualsDirty(true);
        emitNeedRender();
    }
}

void QQuickGraphsBars::updateSelectedBar()
{
    for (auto it = m_barModelsMap.begin(); it != m_barModelsMap.end(); it++) {
        if (m_selectedBarSeries && it.key()->isVisible()) {
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                for (auto barList : std::as_const(*it.value())) {
                    QQuickGraphsBars::SelectionType selectionType = isSelected(barList->coord.x(),
                                                                               barList->coord.y(),
                                                                               it.key());
                    switch (selectionType) {
                    case QQuickGraphsBars::SelectionItem: {
                        updateMaterialProperties(barList->model,
                                                 true,
                                                 false,
                                                 barList->texture,
                                                 it.key()->singleHighlightColor());

                        m_selectedBarPos = barList->model->position();
                        QString label = (m_selectedBarSeries->d_func()->itemLabel());

                        if (barList->heightValue >= 0.0f) {
                            m_selectedBarPos.setY(m_selectedBarPos.y() + barList->heightValue
                                                  + 0.2f);
                        } else {
                            m_selectedBarPos.setY(m_selectedBarPos.y() + barList->heightValue
                                                  - 0.2f);
                        }

                        updateItemLabel(m_selectedBarPos);
                        itemLabel()->setVisible(m_selectedBarSeries->isItemLabelVisible()
                                                && theme()->labelsVisible());
                        itemLabel()->setProperty("labelText", label);
                        if (!label.compare(QString(hiddenLabelTag)))
                            itemLabel()->setVisible(false);
                        if (isSliceEnabled())
                            updateSliceItemLabel(label, m_selectedBarPos);
                        break;
                    }
                    case QQuickGraphsBars::SelectionRow:
                    case QQuickGraphsBars::SelectionColumn: {
                        updateMaterialProperties(barList->model,
                                                 false,
                                                 true,
                                                 barList->texture,
                                                 it.key()->multiHighlightColor());
                        break;
                    }
                    default:
                        break;
                    }
                }
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                QList<BarModel *> barList = *m_barModelsMap.value(it.key());
                deleteBarItemHolders(barList.at(0)->selectionInstancing);
                deleteBarItemHolders(barList.at(0)->multiSelectionInstancing);
                createBarItemHolders(it.key(), barList, false);
            }
        }
    }
}

QQuickGraphsBars::SelectionType QQuickGraphsBars::isSelected(int row, int bar, QBar3DSeries *series)
{
    QQuickGraphsBars::SelectionType isSelectedType = QQuickGraphsBars::SelectionNone;
    if ((selectionMode().testFlag(QtGraphs3D::SelectionFlag::MultiSeries) && m_selectedBarSeries)
        || series == m_selectedBarSeries) {
        if (row == m_selectedBar.x() && bar == m_selectedBar.y()
            && (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Item))) {
            isSelectedType = QQuickGraphsBars::SelectionItem;
        } else if (row == m_selectedBar.x()
                   && (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))) {
            isSelectedType = QQuickGraphsBars::SelectionRow;
        } else if (bar == m_selectedBar.y()
                   && (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))) {
            isSelectedType = QQuickGraphsBars::SelectionColumn;
        }
    }

    return isSelectedType;
}

void QQuickGraphsBars::updateSliceItemLabel(const QString &label, QVector3D position)
{
    QQuickGraphsItem::updateSliceItemLabel(label, position);

    QFontMetrics fm(theme()->labelFont());
    float textPadding = theme()->labelFont().pointSizeF() * .7f;
    float labelHeight = fm.height() + textPadding;
    float labelWidth = fm.horizontalAdvance(label) + textPadding;
    sliceItemLabel()->setProperty("labelWidth", labelWidth);
    sliceItemLabel()->setProperty("labelHeight", labelHeight);
    QVector3D slicePos = position;
    if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))
        slicePos.setX(slicePos.z());
    else if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))
        slicePos.setX(slicePos.x());
    QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(axisY());
    if (valueAxis->reversed())
        slicePos.setY(slicePos.y() - (textPadding * .06f));
    else
        slicePos.setY(slicePos.y() + (textPadding * .06f));
    slicePos.setZ(.1f);
    sliceItemLabel()->setPosition(slicePos);
    sliceItemLabel()->setProperty("labelText", label);
    if (!label.compare(QString(hiddenLabelTag)))
        sliceItemLabel()->setVisible(false);
    sliceItemLabel()->setEulerRotation(QVector3D(0.0f, 0.0f, 90.0f));
    sliceItemLabel()->setVisible(m_selectedBarSeries->isItemLabelVisible()
                                 && theme()->labelsVisible());
}

void QQuickGraphsBars::resetClickedStatus()
{
    m_selectedBarPos = QVector3D(0.0f, 0.0f, 0.0f);
    m_selectedBar = QQuickGraphsBars::invalidSelectionPosition();
    m_selectedBarSeries = 0;
    clearSelection();

    if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
        for (const auto barList : std::as_const(m_barModelsMap)) {
            QList<BarItemHolder *> barItemList = barList->at(0)->instancing->dataArray();
            for (auto bih : std::as_const(barItemList))
                bih->selectedBar = false;
        }
    }

    if (sliceView() && sliceView()->isVisible()) {
        setSliceActivatedChanged(true);
        m_selectionDirty = true;
    }
    setSeriesVisualsDirty(true);
}

void QQuickGraphsBars::createSliceView()
{
    setSliceOrthoProjection(false);
    QQuickGraphsItem::createSliceView();

    Q_TRACE_SCOPE(QGraphs3DBarsCreateSliceView);

    QList<QBar3DSeries *> barSeries = barSeriesList();
    for (const auto &barSeries : std::as_const(barSeries)) {
        QList<BarModel *> &slicedBarList = m_slicedBarModels[barSeries];

        if (slicedBarList.isEmpty()) {
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                qsizetype dataRowIndex = m_minRow;
                qsizetype newRowSize = qMin(barSeries->dataProxy()->rowCount() - dataRowIndex,
                                            m_newRows);
                qsizetype newColSize = 0;
                if (newRowSize) {
                    const QBarDataRow *dataRow = &barSeries->dataProxy()->rowAt(dataRowIndex);
                    if (dataRow) {
                        qsizetype dataColIndex = m_minCol;
                        newColSize = qMin(dataRow->size() - dataColIndex, m_newCols);
                    }
                }
                qsizetype slicedBarListSize = 0;

                if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))
                    slicedBarListSize = newColSize;
                else if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))
                    slicedBarListSize = newRowSize;

                for (int ind = 0; ind < slicedBarListSize; ++ind) {
                    QQuick3DModel *model = createDataItem(sliceView()->scene(), barSeries);
                    model->setVisible(false);
                    BarModel *barModel = new BarModel();
                    barModel->model = model;

                    if (!slicedBarList.contains(barModel))
                        slicedBarList.append(barModel);
                }
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                BarModel *barInstancing = new BarModel();

                if (barInstancing->selectionInstancing == nullptr) {
                    barInstancing->selectionInstancing = new BarInstancing;
                    barInstancing->selectionInstancing->setParent(barSeries);
                    barInstancing->multiSelectionInstancing = new BarInstancing;
                    barInstancing->multiSelectionInstancing->setParent(barSeries);
                }

                if (barInstancing->selectedModel == nullptr) {
                    barInstancing->selectedModel = createDataItem(sliceView()->scene(), barSeries);
                    barInstancing->selectedModel->setInstancing(barInstancing->selectionInstancing);
                    barInstancing->selectedModel->setVisible(false);
                    barInstancing->multiSelectedModel = createDataItem(sliceView()->scene(),
                                                                       barSeries);
                    barInstancing->multiSelectedModel->setInstancing(
                        barInstancing->multiSelectionInstancing);
                    barInstancing->multiSelectedModel->setVisible(false);
                }

                if (!slicedBarList.contains(barInstancing))
                    slicedBarList.append(barInstancing);
            }
        }
    }
}

void QQuickGraphsBars::toggleSliceGraph()
{
    if (m_selectionDirty)
        QQuickGraphsItem::toggleSliceGraph();

    if (sliceView() && !sliceView()->isVisible()) {
        removeSlicedBarModels();
        m_changeTracker.selectedBarChanged = false;
        return;
    }

    qsizetype index = 0;
    bool rowMode = selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row);
    for (auto it = m_slicedBarModels.begin(); it != m_slicedBarModels.end(); it++) {
        bool useGradient = it.key()->d_func()->isUsingGradient();
        bool rangeGradient = (useGradient
                              && it.key()->d_func()->m_colorStyle
                                      == QGraphsTheme::ColorStyle::RangeGradient);
        QList<BarModel *> barList = *m_barModelsMap.value(it.key());
        QList<BarModel *> sliceList = it.value();
        if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
            for (int ind = 0; ind < it.value().count(); ++ind) {
                if (rowMode)
                    index = (m_selectedBar.x() * it.key()->dataProxy()->colCount()) + ind;
                else
                    index = m_selectedBar.y() + (ind * it.key()->dataProxy()->colCount());
                bool visible = ((m_selectedBarSeries == it.key()
                                 || selectionMode().testFlag(QtGraphs3D::SelectionFlag::MultiSeries))
                                && it.key()->isVisible());
                if (index < barList.size() && m_selectedBar != invalidSelectionPosition()) {
                    BarModel *sliceBarModel = sliceList.at(ind);
                    BarModel *barModel = barList.at(index);

                    sliceBarModel->model->setVisible(visible);
                    if (rowMode) {
                        sliceBarModel->model->setPosition(
                            QVector3D(barModel->model->x(), barModel->model->y(), 0.0f));
                    } else {
                        sliceBarModel->model->setX(barModel->model->z()
                                                   - (barList.at(0)->visualIndex * .1f));
                        sliceBarModel->model->setY(barModel->model->y());
                        sliceBarModel->model->setZ(0.0f);
                    }
                    sliceBarModel->model->setScale(barModel->model->scale());

                    updateItemMaterial(sliceBarModel->model,
                                       useGradient,
                                       rangeGradient,
                                       QStringLiteral(":/materials/BarsMaterial"));

                    if (barModel->coord == m_selectedBar
                        && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Item)) {
                        updateMaterialProperties(sliceBarModel->model,
                                                 true,
                                                 false,
                                                 barList.at(index)->texture,
                                                 it.key()->singleHighlightColor());
                        updateSliceItemLabel(m_selectedBarSeries->itemLabel(), m_selectedBarPos);
                    } else {
                        updateMaterialProperties(sliceBarModel->model,
                                                 false,
                                                 false,
                                                 barList.at(index)->texture,
                                                 it.key()->baseColor());
                    }
                } else {
                    setSliceActivatedChanged(true);
                    QQuickGraphsItem::toggleSliceGraph();
                    setSliceEnabled(false);
                    return;
                }
            }
            setSliceActivatedChanged(false);
        } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
            deleteBarItemHolders(sliceList.at(0)->selectionInstancing);
            deleteBarItemHolders(sliceList.at(0)->multiSelectionInstancing);
            createBarItemHolders(it.key(), barList, true);
            setSliceActivatedChanged(false);
        }
    }
}

void QQuickGraphsBars::handleLabelCountChanged(QQuick3DRepeater *repeater, QColor axisLabelColor)
{
    QQuickGraphsItem::handleLabelCountChanged(repeater, axisLabelColor);

    if (repeater == repeaterX())
        handleDataColumnLabelsChanged();
    if (repeater == repeaterZ())
        handleDataRowLabelsChanged();
}

void QQuickGraphsBars::removeSlicedBarModels()
{
    for (auto &list : std::as_const(m_slicedBarModels)) {
        for (auto barModel : list) {
            if (optimizationHint() == QtGraphs3D::OptimizationHint::Legacy) {
                deleteBarModels(barModel->model);
            } else if (optimizationHint() == QtGraphs3D::OptimizationHint::Default) {
                deleteBarItemHolders(barModel->selectionInstancing);
                deleteBarItemHolders(barModel->multiSelectionInstancing);
                deleteBarModels(barModel->selectedModel);
                deleteBarModels(barModel->multiSelectedModel);
            }
            delete barModel;
        }
    }

    m_slicedBarModels.clear();
}

void QQuickGraphsBars::createBarItemHolders(QBar3DSeries *series,
                                            QList<BarModel *> barList,
                                            bool slice)
{
    bool useGradient = series->d_func()->isUsingGradient();
    bool rangeGradient = (useGradient
                          && series->d_func()->m_colorStyle == QGraphsTheme::ColorStyle::RangeGradient);
    bool visible = ((m_selectedBarSeries == series
                     || selectionMode().testFlag(QtGraphs3D::SelectionFlag::MultiSeries))
                    && series->isVisible());
    QList<BarItemHolder *> barItemList;
    QList<BarItemHolder *> multiBarItemList;
    QQuick3DModel *selectedModel = nullptr;
    QQuick3DModel *multiSelectedModel = nullptr;
    QList<BarItemHolder *> selectedItem;
    QList<BarItemHolder *> multiSelectedItems;
    BarInstancing *instancing;
    BarInstancing *multiInstancing;

    if (slice) {
        QList<BarModel *> sliceBarList = m_slicedBarModels.value(series);
        barItemList = barList.at(0)->selectionInstancing->dataArray();
        multiBarItemList = barList.at(0)->multiSelectionInstancing->dataArray();
        selectedModel = sliceBarList.at(0)->selectedModel;
        multiSelectedModel = sliceBarList.at(0)->multiSelectedModel;
        instancing = sliceBarList.at(0)->selectionInstancing;
        multiInstancing = sliceBarList.at(0)->multiSelectionInstancing;
    } else {
        barItemList = barList.at(0)->instancing->dataArray();
        selectedModel = barList.at(0)->selectedModel;
        multiSelectedModel = barList.at(0)->multiSelectedModel;
        instancing = barList.at(0)->selectionInstancing;
        multiInstancing = barList.at(0)->multiSelectionInstancing;
    }

    for (const auto bih : std::as_const(barItemList)) {
        QQuickGraphsBars::SelectionType selectionType = isSelected(bih->coord.x(),
                                                                   bih->coord.y(),
                                                                   series);
        switch (selectionType) {
        case QQuickGraphsBars::SelectionItem: {
            updateItemMaterial(selectedModel,
                               useGradient,
                               rangeGradient,
                               QStringLiteral(":/materials/BarsMaterialInstancing"));
            updateMaterialProperties(selectedModel,
                                     true,
                                     false,
                                     barList.at(0)->texture,
                                     QColor(Qt::white));
            if (!slice)
                bih->selectedBar = true;
            selectedModel->setVisible(visible);
            BarItemHolder *selectedBih = new BarItemHolder();
            selectedBih->selectedBar = false;
            selectedBih->color = series->singleHighlightColor();
            selectedBih->coord = bih->coord;
            selectedBih->rotation = bih->rotation;
            selectedBih->heightValue = bih->heightValue;
            selectedBih->position = bih->position;
            selectedBih->scale = bih->scale;

            QString label = m_selectedBarSeries->itemLabel();
            if (slice) {
                if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row)) {
                    selectedBih->position.setZ(.0f);
                } else {
                    selectedBih->position.setX(selectedBih->position.z()
                                               - (barList.at(0)->visualIndex * .1f));
                    selectedBih->position.setZ(.0f);
                }
                updateSliceItemLabel(label, m_selectedBarPos);
            }

            selectedItem.push_back(selectedBih);
            instancing->setDataArray(selectedItem);

            m_selectedBarPos = bih->position;

            if (bih->heightValue >= 0.0f)
                m_selectedBarPos.setY(m_selectedBarPos.y() + bih->heightValue + 0.2f);
            else
                m_selectedBarPos.setY(m_selectedBarPos.y() + bih->heightValue - 0.2f);

            updateItemLabel(m_selectedBarPos);
            itemLabel()->setVisible(m_selectedBarSeries->isItemLabelVisible()
                                    && theme()->labelsVisible());
            itemLabel()->setProperty("labelText", label);
            if (!label.compare(QString(hiddenLabelTag)))
                itemLabel()->setVisible(false);
            if (isSliceEnabled())
                updateSliceItemLabel(label, m_selectedBarPos);
            break;
        }
        case QQuickGraphsBars::SelectionRow:
        case QQuickGraphsBars::SelectionColumn: {
            updateItemMaterial(multiSelectedModel,
                               useGradient,
                               rangeGradient,
                               QStringLiteral(":/materials/BarsMaterialInstancing"));
            updateMaterialProperties(multiSelectedModel,
                                     false,
                                     true,
                                     barList.at(0)->texture,
                                     QColor(Qt::white));
            if (!slice)
                bih->selectedBar = true;
            multiSelectedModel->setVisible(visible);
            BarItemHolder *selectedBih = new BarItemHolder();
            selectedBih->selectedBar = false;
            selectedBih->color = series->multiHighlightColor();
            selectedBih->coord = bih->coord;
            selectedBih->rotation = bih->rotation;
            selectedBih->heightValue = bih->heightValue;
            selectedBih->position = bih->position;
            selectedBih->scale = bih->scale;

            multiSelectedItems.push_back(selectedBih);
            multiInstancing->setDataArray(multiSelectedItems);
            break;
        }
        default:
            break;
        }
    }

    if (slice) {
        for (const auto bih : std::as_const(multiBarItemList)) {
            updateItemMaterial(multiSelectedModel,
                               useGradient,
                               rangeGradient,
                               QStringLiteral(":/materials/BarsMaterialInstancing"));
            updateMaterialProperties(multiSelectedModel,
                                     false,
                                     false,
                                     barList.at(0)->texture,
                                     QColor(Qt::white));

            multiSelectedModel->setVisible(visible);
            BarItemHolder *selectedBih = new BarItemHolder();
            selectedBih->selectedBar = false;
            selectedBih->color = series->baseColor();
            selectedBih->coord = bih->coord;
            selectedBih->rotation = bih->rotation;
            selectedBih->heightValue = bih->heightValue;
            selectedBih->position = bih->position;
            selectedBih->scale = bih->scale;

            if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row)) {
                selectedBih->position.setZ(.0f);
            } else {
                selectedBih->position.setX(selectedBih->position.z()
                                           - (barList.at(0)->visualIndex * .1f));
                selectedBih->position.setZ(.0f);
            }

            multiSelectedItems.push_back(selectedBih);
            multiInstancing->setDataArray(multiSelectedItems);
        }
    }
}

void QQuickGraphsBars::updateSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    checkSliceEnabled();
    if (!sliceView())
        createSliceView();

    bool validSlice = mode.testFlag(QtGraphs3D::SelectionFlag::Slice) && m_selectedBarSeries;
    if (sliceView() && sliceView()->isVisible()) {
        if (validSlice) {
            removeSlicedBarModels();
            createSliceView();
            toggleSliceGraph();
        } else {
            m_selectionDirty = true;
            setSliceActivatedChanged(true);
        }
    } else if (validSlice) {
        m_selectionDirty = true;
        setSliceActivatedChanged(true);
    }

    setSeriesVisualsDirty(true);
    itemLabel()->setVisible(false);
    if (sliceView() && !mode.testFlag(QtGraphs3D::SelectionFlag::Item))
        sliceItemLabel()->setVisible(false);
}

void QQuickGraphsBars::updateBarSpecs(float thicknessRatio, QSizeF spacing, bool relative)
{
    // Convert ratio to QSizeF, as we need it in that format for autoscaling
    // calculations
    m_cachedBarThickness.setWidth(1.0);
    m_cachedBarThickness.setHeight(1.0f / thicknessRatio);

    if (relative) {
        m_cachedBarSpacing.setWidth((m_cachedBarThickness.width() * 2) * (spacing.width() + 1.0f));
        m_cachedBarSpacing.setHeight((m_cachedBarThickness.height() * 2)
                                     * (spacing.height() + 1.0f));
    } else {
        m_cachedBarSpacing = m_cachedBarThickness * 2 + spacing * 2;
    }

    m_changeTracker.axisRangeChanged = true;
    m_changeTracker.selectedBarChanged = true;

    // Calculate here and at setting sample space
    calculateSceneScalingFactors();
    setSeriesVisualsDirty(true);
}

void QQuickGraphsBars::updateBarSeriesMargin(QSizeF margin)
{
    m_cachedBarSeriesMargin = margin;
    calculateSeriesStartPosition();
    calculateSceneScalingFactors();
    setSeriesVisualsDirty(true);
}

QT_END_NAMESPACE

#include "moc_qquickgraphsbars_p.cpp"
