// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include <private/q3dbarswidgetitem_p.h>
#include <private/qquickgraphsbars_p.h>

QT_BEGIN_NAMESPACE

/*!
 * \class Q3DBarsWidgetItem
 * \inmodule QtGraphsWidgets
 * \ingroup graphs_3D_widgets
 * \brief The Q3DBarsWidgetItem class provides methods for rendering 3D bar graphs.
 *
 * This class enables developers to render 3D bar graphs and view them by
 * freely rotating the scene. Rotation is achieved by holding down the right
 * mouse button and moving the mouse, while zooming is accomplished using
 * the mouse wheel. If enabled, selection is performed with the left mouse
 * button. The scene can be reset to the default camera view by clicking the
 * mouse wheel. On touch devices, rotation is achieved by tap-and-move,
 * selection by tap-and-hold, and zooming by pinch.
 *
 * If no axes are set explicitly for Q3DBarsWidgetItem, temporary default axes without
 * labels are created. These default axes can be modified via axis accessors,
 * but as soon as any axis is set explicitly for the orientation, the default
 * axis for that orientation is destroyed.
 *
 * Q3DBarsWidgetItem supports more than one visible series at the same time. All series
 * don't need to have the same number of rows and columns. Row and column
 * labels are taken from the first added series unless explicitly defined
 * for row and column axes.
 *
 * Q3DBarsWidgetItem has transparency support. This feature allows you to adjust the
 * opacity of the bars, making them partially see-through, fully transparent,
 * or opaque.
 *
 * \section1 How to construct a minimal Q3DBarsWidgetItem graph
 *
 * First, construct an instance of Q3DBarsWidgetItem:
 *
 * \snippet doc_src_q3dbars_construction.cpp 4
 *
 * After constructing Q3DBarsWidgetItem, you can set the data window by changing the range
 * on the row and column axes. It is not mandatory, as the data window will default
 * to showing all of the data in the series. If the amount of data is large, it
 * is usually preferable to show just a portion of it. For example, let's set
 * the data window to display the first five rows and columns:
 *
 * \snippet doc_src_q3dbars_construction.cpp 0
 *
 * Now, Q3DBarsWidgetItem is ready to receive data to be rendered. Create a series with one
 * row of 5 values:
 *
 * \snippet doc_src_q3dbars_construction.cpp 1
 *
 * \note We set the data window to 5 x 5, but we are adding only one row of
 * data. This is okay; the rest of the rows will just be blank.
 *
 * Finally you will need to set it visible:
 *
 * \snippet doc_src_q3dbars_construction.cpp 2
 *
 * The complete code needed to create and display this graph is:
 *
 * \snippet doc_src_q3dbars_construction.cpp 3
 *
 * And this is what those few lines of code produce:
 *
 * \image q3dbars-minimal.png
 *
 * The scene can be rotated, zoomed into, and a bar can be selected to view its
 * value, but no other interactions are included in this minimal code example.
 * You can learn more by familiarizing yourself with the examples provided, like
 * the \l{Simple Bar Graph}.
 *
 * \sa Q3DScatterWidgetItem, Q3DSurfaceWidgetItem, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * Constructs a new 3D bar graph with the optional \a parent.
 */
Q3DBarsWidgetItem::Q3DBarsWidgetItem(QObject *parent)
    : Q3DGraphsWidgetItem(*(new Q3DBarsWidgetItemPrivate()), parent, QStringLiteral("Bars3D"))
{}

/*!
 * Destroys the 3D bar graph.
 */
Q3DBarsWidgetItem::~Q3DBarsWidgetItem() {}

/*!
 * \property Q3DBarsWidgetItem::primarySeries
 *
 * \brief The primary series of the graph.
 *
 * Sets \a series as the primary series of the graph. The primary series
 * determines the row and column axis labels when the labels are not explicitly
 * set to the axes.
 *
 * If the specified series is not yet added to the graph, setting it as the
 * primary series will also implicitly add it to the graph.
 *
 * If the primary series itself is removed from the graph, this property
 * resets to default.
 *
 * If \a series is null, this property resets to default.
 * Defaults to the first added series or zero if no series are added to the
 * graph.
 */
void Q3DBarsWidgetItem::setPrimarySeries(QBar3DSeries *series)
{
    graphBars()->setPrimarySeries(series);
    emit primarySeriesChanged(series);
}

QBar3DSeries *Q3DBarsWidgetItem::primarySeries() const
{
    return graphBars()->primarySeries();
}

/*!
 * Adds the \a series to the graph. A graph can contain multiple series, but
 * only one set of axes, so the rows and columns of all series must match for
 * the visualized data to be meaningful. If the graph has multiple visible
 * series, only the primary series will generate the row or column labels on the
 * axes in cases where the labels are not explicitly set for the axes. If the
 * newly added series has specified a selected bar, it will be highlighted and
 * any existing selection will be cleared. Only one added series can have an
 * active selection.
 *
 * \sa seriesList(), primarySeries, Q3DGraphsWidgetItem::hasSeries()
 */
void Q3DBarsWidgetItem::addSeries(QBar3DSeries *series)
{
    graphBars()->addSeries(series);
}

/*!
 * Removes the \a series from the graph.
 *
 * \sa Q3DGraphsWidgetItem::hasSeries()
 */
void Q3DBarsWidgetItem::removeSeries(QBar3DSeries *series)
{
    graphBars()->removeSeries(series);
}

/*!
 * Inserts the \a series into the position \a index in the series list.
 * If the \a series has already been added to the list, it is moved to the
 * new \a index.
 * \note When moving a series to a new \a index that is after its old index,
 * the new position in the list is calculated as if the series was still in its
 * old index, so the final index is actually the \a index decremented by one.
 *
 * \sa addSeries(), seriesList(), Q3DGraphsWidgetItem::hasSeries()
 */
void Q3DBarsWidgetItem::insertSeries(int index, QBar3DSeries *series)
{
    graphBars()->insertSeries(index, series);
}

/*!
 * Returns the list of series added to this graph.
 *
 * \sa Q3DGraphsWidgetItem::hasSeries()
 */
QList<QBar3DSeries *> Q3DBarsWidgetItem::seriesList() const
{
    QList<QBar3DSeries *> barSeriesList;
    for (QAbstract3DSeries *abstractSeries : graphBars()->m_seriesList) {
        QBar3DSeries *barSeries = qobject_cast<QBar3DSeries *>(abstractSeries);
        if (barSeries)
            barSeriesList.append(barSeries);
    }

    return barSeriesList;
}

/*!
 * \property Q3DBarsWidgetItem::multiSeriesUniform
 *
 * \brief Whether bars are to be scaled with proportions set to a single series
 * bar even if there are multiple series displayed.
 *
 * If set to \c {true}, \l{barSpacing}{bar spacing} will be correctly applied
 * only to the X-axis. Preset to \c false by default.
 */
void Q3DBarsWidgetItem::setMultiSeriesUniform(bool uniform)
{
    graphBars()->setMultiSeriesUniform(uniform);
    emit multiSeriesUniformChanged(uniform);
}

bool Q3DBarsWidgetItem::isMultiSeriesUniform() const
{
    return graphBars()->isMultiSeriesUniform();
}

/*!
 * \property Q3DBarsWidgetItem::barThickness
 *
 * \brief The bar thickness ratio between the X and Z dimensions.
 *
 * The value \c 1.0 means that the bars are as wide as they are deep, whereas
 *\c 0.5 makes them twice as deep as they are wide. Preset to \c 1.0 by default.
 */
void Q3DBarsWidgetItem::setBarThickness(float thicknessRatio)
{
    graphBars()->setBarThickness(thicknessRatio);
    emit barThicknessChanged(thicknessRatio);
}

float Q3DBarsWidgetItem::barThickness() const
{
    return graphBars()->barThickness();
}

/*!
 * \property Q3DBarsWidgetItem::barSpacing
 *
 * \brief Bar spacing in the X and Z dimensions.
 *
 * Preset to \c {(1.0, 1.0)} by default. Spacing is affected by the
 * barSpacingRelative property.
 *
 * \sa barSpacingRelative, multiSeriesUniform, barSeriesMargin
 */
void Q3DBarsWidgetItem::setBarSpacing(QSizeF spacing)
{
    graphBars()->setBarSpacing(spacing);
    emit barSpacingChanged(spacing);
}

QSizeF Q3DBarsWidgetItem::barSpacing() const
{
    return graphBars()->barSpacing();
}

/*!
 * \property Q3DBarsWidgetItem::barSpacingRelative
 *
 * \brief Whether spacing is absolute or relative to bar thickness.
 *
 * If it is \c true, the value of \c 0.0 means that the bars are placed
 * side-to-side, \c 1.0 means that a space as wide as the thickness of one bar
 * is left between the bars, and so on. Preset to \c true.
 */
void Q3DBarsWidgetItem::setBarSpacingRelative(bool relative)
{
    graphBars()->setBarSpacingRelative(relative);
    emit barSpacingRelativeChanged(relative);
}

bool Q3DBarsWidgetItem::isBarSpacingRelative() const
{
    return graphBars()->isBarSpacingRelative();
}

/*!
 * \property Q3DBarsWidgetItem::barSeriesMargin
 *
 * \brief Margin between series columns in X and Z dimensions.
 * Sensible values are on the range [0,1).
 *
 * Preset to \c {(0.0, 0.0)} by default. This property enables
 * showing bars from different series side by side, but with space between
 * columns.
 *
 * \sa barSpacing
 */
void Q3DBarsWidgetItem::setBarSeriesMargin(QSizeF margin)
{
    graphBars()->setBarSeriesMargin(margin);
    emit barSeriesMarginChanged(margin);
}

QSizeF Q3DBarsWidgetItem::barSeriesMargin() const
{
    return graphBars()->barSeriesMargin();
}

/*!
 * \property Q3DBarsWidgetItem::rowAxis
 *
 * \brief The axis attached to the active row.
 *
 * Sets the axis of the active row to \a axis. Implicitly calls addAxis() to
 * transfer the ownership of the axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is set explicitly to the
 * same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DBarsWidgetItem::setRowAxis(QCategory3DAxis *axis)
{
    graphBars()->setRowAxis(axis);
    emit rowAxisChanged(rowAxis());
}

QCategory3DAxis *Q3DBarsWidgetItem::rowAxis() const
{
    return graphBars()->rowAxis();
}

/*!
 * \property Q3DBarsWidgetItem::columnAxis
 *
 * \brief The axis attached to the active column.
 *
 * Sets the axis of the active column to \a axis. Implicitly calls addAxis() to
 * transfer the ownership of the axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is set explicitly to the
 * same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DBarsWidgetItem::setColumnAxis(QCategory3DAxis *axis)
{
    graphBars()->setColumnAxis(axis);
    emit columnAxisChanged(columnAxis());
}

QCategory3DAxis *Q3DBarsWidgetItem::columnAxis() const
{
    return graphBars()->columnAxis();
}

/*!
 * \property Q3DBarsWidgetItem::valueAxis
 *
 * Sets the active value axis (the Y-axis) to \a axis. Implicitly calls
 * addAxis() to transfer the ownership of \a axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels and
 * an automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is set explicitly to the
 * same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DBarsWidgetItem::setValueAxis(QValue3DAxis *axis)
{
    graphBars()->setValueAxis(axis);
    emit valueAxisChanged(valueAxis());
}

QValue3DAxis *Q3DBarsWidgetItem::valueAxis() const
{
    return graphBars()->valueAxis();
}

/*!
 * \property Q3DBarsWidgetItem::selectedSeries
 * \readonly
 *
 * \brief The selected series or a null value.
 *
 * If selectionMode has the \c SelectionMultiSeries flag set, this
 * property holds the series that owns the selected bar.
 */
QBar3DSeries *Q3DBarsWidgetItem::selectedSeries() const
{
    return graphBars()->selectedSeries();
}

/*!
 * \property Q3DBarsWidgetItem::floorLevel
 *
 * \brief The floor level for the bar graph in Y-axis data coordinates.
 *
 * The actual floor level will be restricted by the Y-axis minimum and maximum
 * values.
 * Defaults to zero.
 */
void Q3DBarsWidgetItem::setFloorLevel(float level)
{
    graphBars()->setFloorLevel(level);
    emit floorLevelChanged(level);
}

float Q3DBarsWidgetItem::floorLevel() const
{
    return graphBars()->floorLevel();
}

bool Q3DBarsWidgetItem::event(QEvent *event)
{
    return Q3DGraphsWidgetItem::event(event);
}

/*!
 * Adds \a axis to the graph. The axes added via addAxis are not yet taken to
 * use, addAxis is simply used to give the ownership of the \a axis to the
 * graph. The \a axis must not be null or added to another graph.
 *
 * \sa releaseAxis(), setValueAxis(), setRowAxis(), setColumnAxis()
 */
void Q3DBarsWidgetItem::addAxis(QAbstract3DAxis *axis)
{
    graphBars()->addAxis(axis);
}

/*!
 * Releases the ownership of the \a axis back to the caller, if it is added to
 * this graph. If the released \a axis is in use, a new default axis will be
 * created and set active.
 *
 * If the default axis is released and added back later, it behaves as any other
 * axis would.
 *
 * \sa addAxis(), setValueAxis(), setRowAxis(), setColumnAxis()
 */
void Q3DBarsWidgetItem::releaseAxis(QAbstract3DAxis *axis)
{
    graphBars()->releaseAxis(axis);
}

/*!
 * Returns the list of all added axes.
 *
 * \sa addAxis()
 */
QList<QAbstract3DAxis *> Q3DBarsWidgetItem::axes() const
{
    return graphBars()->axes();
}

/*!
 * \fn Q3DBarsWidgetItem::sliceImageChanged(const QImage &image)
 * \since 6.10
 * Emitted when \l renderSliceToImage has prepared the \a{image}.
 */

/*!
 * \internal
 */
QQuickGraphsBars *Q3DBarsWidgetItem::graphBars()
{
    Q_D(Q3DBarsWidgetItem);
    return static_cast<QQuickGraphsBars *>(d->m_graphsItem.get());
}

/*!
 * \internal
 */
const QQuickGraphsBars *Q3DBarsWidgetItem::graphBars() const
{
    Q_D(const Q3DBarsWidgetItem);
    return static_cast<const QQuickGraphsBars *>(d->m_graphsItem.get());
}

/*!
 * Exports the requested slice view to an image.
 * The exported slice is bars of row or column, which is defined by \a sliceType,
 * at a given \a requestedIndex.
 *
 * The \l sliceImageChanged signal is emitted when the image is ready, and can
 * be captured as follows:
 *
 * \code
 * connect(item, &Q3DBarsWidgetItem::sliceImageChanged, this, [](const QImage &image) {
 *     // ~~~
 * });
 *
 * item->renderSliceToImage(sliceType, index);
 * \endcode
 *
 * The image is rendered with the current antialiasing settings.
 *
 * \sa QQuickItem::grabToImage(), sliceImageChanged()
 *
 * \since 6.10
 */
void Q3DBarsWidgetItem::renderSliceToImage(int requestedIndex,
                                           QtGraphs3D::SliceCaptureType sliceType)
{
    auto graph = graphBars();
    disconnect(graph,
               &QQuickGraphsBars::sliceImageChanged,
               this,
               &Q3DBarsWidgetItem::sliceImageChanged);
    connect(graph,
            &QQuickGraphsBars::sliceImageChanged,
            this,
            &Q3DBarsWidgetItem::sliceImageChanged);
    graphBars()->renderSliceToImage(requestedIndex, sliceType);
}

QT_END_NAMESPACE
