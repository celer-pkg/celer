// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickgraphsbarsnode_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \qmltype Bars3DNode
 * \inherits GraphsNode
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief 3D bar graph node.
 *
 * This type enables developers to render a bar graph node in 3D with Qt Quick.
 *
 * You will need to import Qt Graphs module to use this type:
 *
 * \snippet doc_src_qmlnodes.qml 0
 *
 * After that you can use Bars3DNode in your qml files:
 *
 * \snippet doc_src_qmlnodes.qml 1
 *
 * \sa Bar3DSeries, ItemModelBarDataProxy, Scatter3DNode, Surface3DNode, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Category3DAxis Bars3DNode::rowAxis
 * The active row axis.
 *
 * If an axis is not given, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty ValueAxis3D Bars3DNode::valueAxis
 * The active value axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Category3DAxis Bars3DNode::columnAxis
 * The active column axis.
 *
 * If an axis is not given, a temporary default axis with no labels is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty bool Bars3DNode::multiSeriesUniform
 * Defines whether bars are to be scaled with proportions set to a single series bar even
 * if there are multiple series displayed. If set to \c {true}, \l{barSpacing}{bar spacing} will
 * be correctly applied only to the X-axis. Preset to \c false by default.
 */

/*!
 * \qmlproperty real Bars3DNode::barThickness
 * The bar thickness ratio between the X and Z dimensions. The value \c 1.0
 * means that the bars are as wide as they are deep, whereas \c 0.5
 * makes them twice as deep as they are wide.
 */

/*!
 * \qmlproperty size Bars3DNode::barSpacing
 * Bar spacing in X and Z dimensions.
 *
 * Preset to \c {(1.0, 1.0)} by default. Spacing is affected by the
 * barSpacingRelative property.
 */

/*!
 * \qmlproperty bool Bars3DNode::barSpacingRelative
 * Whether spacing is absolute or relative to bar thickness.
 *
 * If \c true, the value of \c 0.0 means that the bars are placed
 * side-to-side, \c 1.0 means that a space as wide as the thickness of one bar
 * is left between the bars, and so on. Preset to \c true.
 */

/*!
 * \qmlproperty size Bars3DNode::barSeriesMargin
 *
 * Margin between series columns in X and Z dimensions. Preset to \c {(0.0, 0.0)} by default.
 * Sensible values are on the range [0,1).
 */

/*!
 * \qmlproperty Bar3DSeries Bars3DNode::selectedSeries
 * \readonly
 *
 * The selected series or \c null. If \l {GraphsNode::selectionMode}{selectionMode} has
 * the \c SelectionMultiSeries flag set, this property holds the series that
 * owns the selected bar.
 */

/*!
 * \qmlproperty list<Bar3DSeries> Bars3DNode::seriesList
 * \qmldefault
 * The series of the graph.
 * By default, this property contains an empty list.
 * To set the series, either use the addSeries() function or define them as children of the graph.
 */

/*!
 * \qmlproperty Bar3DSeries Bars3DNode::primarySeries
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
 * \qmlproperty real Bars3DNode::floorLevel
 *
 * The floor level for the bar graph in Y-axis data coordinates.
 *
 * The actual floor level will be restricted by the Y-axis minimum and maximum
 * values.
 * Defaults to zero.
 */

/*!
 * \qmlmethod void Bars3DNode::addSeries(Bar3DSeries series)
 * Adds the \a series to the graph. A graph can contain multiple series, but only one set of axes,
 * so the rows and columns of all series must match for the visualized data to be meaningful.
 * If the graph has multiple visible series, only the first one added will
 * generate the row or column labels on the axes in cases where the labels are not explicitly set
 * to the axes. If the newly added series has specified a selected bar, it will be highlighted and
 * any existing selection will be cleared. Only one added series can have an active selection.
 * \sa GraphsNode::hasSeries()
 */

/*!
 * \qmlmethod void Bars3DNode::removeSeries(Bar3DSeries series)
 * Remove the \a series from the graph.
 * \sa GraphsNode::hasSeries()
 */

/*!
 * \qmlmethod void Bars3DNode::insertSeries(int index, Bar3DSeries series)
 * Inserts the \a series into the position \a index in the series list.
 * If the \a series has already been added to the list, it is moved to the
 * new \a index.
 * \note When moving a series to a new \a index that is after its old index,
 * the new position in list is calculated as if the series was still in its old
 * index, so the final index is actually the \a index decremented by one.
 * \sa GraphsNode::hasSeries()
 */

/*!
 * \qmlsignal Bars3DNode::multiSeriesUniformChanged(bool uniform)
 *
 * This signal is emitted when multiSeriesUniform changes to \a uniform.
 */

/*!
 * \qmlsignal Bars3DNode::barThicknessChanged(real thicknessRatio)
 *
 * This signal is emitted when barThickness changes to \a thicknessRatio.
 */

/*!
 * \qmlsignal Bars3DNode::barSpacingChanged(size spacing)
 *
 * This signal is emitted when barSpacing changes to \a spacing.
 */

/*!
 * \qmlsignal Bars3DNode::barSpacingRelativeChanged(bool relative)
 *
 * This signal is emitted when barSpacingRelative changes to \a relative.
 */

/*!
 * \qmlsignal Bars3DNode::barSeriesMarginChanged(size margin)
 *
 * This signal is emitted when barSeriesMargin changes to \a margin.
 */

/*!
 * \qmlsignal Bars3DNode::rowAxisChanged(Category3DAxis axis)
 *
 * This signal is emitted when rowAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3DNode::columnAxisChanged(Category3DAxis axis)
 *
 * This signal is emitted when columnAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3DNode::valueAxisChanged(ValueAxis3D axis)
 *
 * This signal is emitted when valueAxis changes to \a axis.
 */

/*!
 * \qmlsignal Bars3DNode::primarySeriesChanged(Bar3DSeries series)
 *
 * This signal is emitted when primarySeries changes to \a series.
 */

/*!
 * \qmlsignal Bars3DNode::selectedSeriesChanged(Bar3DSeries series)
 *
 * This signal is emitted when selectedSeries changes to \a series.
 */

/*!
 * \qmlsignal Bars3DNode::floorLevelChanged(real level)
 *
 * This signal is emitted when floorLevel changes to \a level.
 */

QQuickGraphsBarsNode::QQuickGraphsBarsNode(QQuick3DNode *parent)
    : QQuickGraphsNode(parent)
    , m_multiSeriesUniform(false)
    , m_barSpacing(QSizeF(1.0f, 1.0f))
    , m_barThickness(1.0f)
    , m_barSpacingRelative(false)
    , m_floorLevel(0.0f)
{}

QQuickGraphsBarsNode::~QQuickGraphsBarsNode() {}

void QQuickGraphsBarsNode::componentComplete()
{
    const QString qmlData = QLatin1StringView(R"QML(
        import QtQuick;
        import QtGraphs;

        Bars3D{}
    )QML");

    QQmlComponent *component = new QQmlComponent(qmlEngine(this), this);
    component->setData(qmlData.toUtf8(), QUrl());
    m_graph.reset(qobject_cast<QQuickGraphsItem *>(component->create()));
    setGraphParent();
    QQuickGraphsNode::componentComplete();

    //initialize components

    if (m_axisX)
        graphBars()->setAxisX(static_cast<QCategory3DAxis *>(m_axisX));
    if (m_axisY)
        graphBars()->setAxisY(static_cast<QValue3DAxis *>(m_axisY));
    if (m_axisZ)
        graphBars()->setAxisZ(static_cast<QCategory3DAxis *>(m_axisZ));

    graphBars()->setSelectionMode(m_selectionMode);
    graphBars()->setMultiSeriesUniform(m_multiSeriesUniform);
    graphBars()->setBarSpacing(m_barSpacing);
    graphBars()->setBarThickness(m_barThickness);
    graphBars()->setBarSpacingRelative(m_barSpacingRelative);
    graphBars()->setFloorLevel(m_floorLevel);

    for (auto series : std::as_const(m_seriesList))
        graphBars()->addSeries(static_cast<QBar3DSeries *>(series));

    //connect signals
    connect(graphBars(),
            &QQuickGraphsBars::rowAxisChanged,
            this,
            &QQuickGraphsBarsNode::rowAxisChanged);
    connect(graphBars(),
            &QQuickGraphsBars::valueAxisChanged,
            this,
            &QQuickGraphsBarsNode::valueAxisChanged);
    connect(graphBars(),
            &QQuickGraphsBars::columnAxisChanged,
            this,
            &QQuickGraphsBarsNode::columnAxisChanged);
    connect(graphBars(),
            &QQuickGraphsBars::multiSeriesUniformChanged,
            this,
            &QQuickGraphsBarsNode::multiSeriesUniformChanged);
    connect(graphBars(),
            &QQuickGraphsBars::barThicknessChanged,
            this,
            &QQuickGraphsBarsNode::barThicknessChanged);
    connect(graphBars(),
            &QQuickGraphsBars::barSpacingChanged,
            this,
            &QQuickGraphsBarsNode::barSpacingChanged);
    connect(graphBars(),
            &QQuickGraphsBars::barSpacingRelativeChanged,
            this,
            &QQuickGraphsBarsNode::barSpacingRelativeChanged);
    connect(graphBars(),
            &QQuickGraphsBars::barSeriesMarginChanged,
            this,
            &QQuickGraphsBarsNode::barSeriesMarginChanged);
    connect(graphBars(),
            &QQuickGraphsBars::primarySeriesChanged,
            this,
            &QQuickGraphsBarsNode::primarySeriesChanged);
    connect(graphBars(),
            &QQuickGraphsBars::selectedSeriesChanged,
            this,
            &QQuickGraphsBarsNode::selectedSeriesChanged);
    connect(graphBars(),
            &QQuickGraphsBars::floorLevelChanged,
            this,
            &QQuickGraphsBarsNode::floorLevelChanged);
}

void QQuickGraphsBarsNode::setRowAxis(QCategory3DAxis *axis)
{
    if (m_axisZ == axis)
        return;

    m_axisZ = axis;
    if (graphBars())
        graphBars()->setRowAxis(axis);
}

QCategory3DAxis *QQuickGraphsBarsNode::rowAxis() const
{
    if (graphBars())
        return graphBars()->rowAxis();
    else
        return static_cast<QCategory3DAxis *>(m_axisZ);
}

void QQuickGraphsBarsNode::setValueAxis(QValue3DAxis *axis)
{
    if (m_axisY == axis)
        return;

    m_axisY = axis;
    if (graphBars())
        graphBars()->setValueAxis(axis);
}

QValue3DAxis *QQuickGraphsBarsNode::valueAxis() const
{
    if (graphBars())
        return graphBars()->valueAxis();
    else
        return static_cast<QValue3DAxis *>(m_axisY);
}

void QQuickGraphsBarsNode::setColumnAxis(QCategory3DAxis *axis)
{
    if (m_axisX == axis)
        return;

    m_axisX = axis;
    if (graphBars())
        graphBars()->setColumnAxis(axis);
}

QCategory3DAxis *QQuickGraphsBarsNode::columnAxis() const
{
    if (graphBars())
        return graphBars()->columnAxis();
    else
        return static_cast<QCategory3DAxis *>(m_axisX);
}

void QQuickGraphsBarsNode::setMultiSeriesUniform(bool uniform)
{
    if (m_multiSeriesUniform == uniform)
        return;
    m_multiSeriesUniform = uniform;
    if (graphBars())
        graphBars()->setMultiSeriesUniform(uniform);
}

bool QQuickGraphsBarsNode::isMultiSeriesUniform() const
{
    if (graphBars())
        return graphBars()->isMultiSeriesUniform();
    else
        return m_multiSeriesUniform;
}

void QQuickGraphsBarsNode::setBarThickness(float thickness)
{
    if (m_barThickness == thickness)
        return;
    m_barThickness = thickness;
    if (graphBars())
        graphBars()->setBarThickness(thickness);
}

float QQuickGraphsBarsNode::barThickness() const
{
    if (graphBars())
        return graphBars()->barThickness();
    else
        return m_barThickness;
}

void QQuickGraphsBarsNode::setBarSpacing(QSizeF spacing)
{
    if (m_barSpacing == spacing)
        return;
    m_barSpacing = spacing;
    if (graphBars())
        graphBars()->setBarSpacing(spacing);
}

QSizeF QQuickGraphsBarsNode::barSpacing() const
{
    if (graphBars())
        return graphBars()->barSpacing();
    else
        return m_barSpacing;
}

void QQuickGraphsBarsNode::setBarSpacingRelative(bool relative)
{
    if (m_barSpacingRelative == relative)
        return;
    m_barSpacingRelative = relative;

    if (graphBars())
        graphBars()->setBarSpacingRelative(relative);
}

bool QQuickGraphsBarsNode::isBarSpacingRelative() const
{
    if (graphBars())
        return graphBars()->isBarSpacingRelative();
    else
        return m_barSpacingRelative;
}

void QQuickGraphsBarsNode::setBarSeriesMargin(QSizeF margin)
{
    if (m_barSeriesMargin == margin)
        return;

    m_barSeriesMargin = margin;
    if (graphBars())
        graphBars()->setBarSeriesMargin(margin);
}

QSizeF QQuickGraphsBarsNode::barSeriesMargin() const
{
    if (graphBars())
        return graphBars()->barSeriesMargin();
    else
        return m_barSeriesMargin;
}

QBar3DSeries *QQuickGraphsBarsNode::selectedSeries() const
{
    if (graphBars())
        return graphBars()->selectedSeries();
    else
        return nullptr;
}

void QQuickGraphsBarsNode::setPrimarySeries(QBar3DSeries *series)
{
    if (m_primarySeries == series)
        return;

    m_primarySeries = series;
    if (graphBars())
        graphBars()->setPrimarySeries(series);
}

QBar3DSeries *QQuickGraphsBarsNode::primarySeries() const
{
    if (graphBars())
        return graphBars()->primarySeries();
    else
        return m_primarySeries;
}

void QQuickGraphsBarsNode::setFloorLevel(float floorLevel)
{
    if (m_floorLevel == floorLevel)
        return;

    m_floorLevel = floorLevel;
    if (graphBars())
        graphBars()->setFloorLevel(floorLevel);
}

float QQuickGraphsBarsNode::floorLevel() const
{
    if (graphBars())
        return graphBars()->floorLevel();
    else
        return m_floorLevel;
}

QList<QBar3DSeries *> QQuickGraphsBarsNode::barSeriesList()
{
    if (graphBars()) {
        return graphBars()->barSeriesList();
    } else {
        QList<QBar3DSeries *> barSeriesList;
        for (QAbstract3DSeries *abstractSeries : std::as_const(m_seriesList)) {
            QBar3DSeries *barSeries = qobject_cast<QBar3DSeries *>(abstractSeries);
            if (barSeries)
                barSeriesList.append(barSeries);
        }
        return barSeriesList;
    }
}

QQmlListProperty<QBar3DSeries> QQuickGraphsBarsNode::seriesList()
{
    if (graphBars()) {
        return graphBars()->seriesList();
    } else {
        return QQmlListProperty<QBar3DSeries>(this,
                                              this,
                                              &QQuickGraphsBarsNode::appendSeriesFunc,
                                              &QQuickGraphsBarsNode::countSeriesFunc,
                                              &QQuickGraphsBarsNode::atSeriesFunc,
                                              &QQuickGraphsBarsNode::clearSeriesFunc);
    }
}

void QQuickGraphsBarsNode::appendSeriesFunc(QQmlListProperty<QBar3DSeries> *list,
                                            QBar3DSeries *series)
{
    reinterpret_cast<QQuickGraphsBarsNode *>(list->data)->addSeries(series);
}

qsizetype QQuickGraphsBarsNode::countSeriesFunc(QQmlListProperty<QBar3DSeries> *list)
{
    return reinterpret_cast<QQuickGraphsBarsNode *>(list->data)->barSeriesList().size();
}

QBar3DSeries *QQuickGraphsBarsNode::atSeriesFunc(QQmlListProperty<QBar3DSeries> *list,
                                                 qsizetype index)
{
    return reinterpret_cast<QQuickGraphsBarsNode *>(list->data)->barSeriesList().at(index);
}

void QQuickGraphsBarsNode::clearSeriesFunc(QQmlListProperty<QBar3DSeries> *list)
{
    QQuickGraphsBarsNode *declBarsNode = reinterpret_cast<QQuickGraphsBarsNode *>(list->data);
    QList<QBar3DSeries *> realList = declBarsNode->barSeriesList();
    qsizetype count = realList.size();
    for (qsizetype i = 0; i < count; i++)
        declBarsNode->removeSeries(realList.at(i));
}

void QQuickGraphsBarsNode::addSeries(QBar3DSeries *series)
{
    Q_ASSERT(series && series->type() == QAbstract3DSeries::SeriesType::Bar);

    if (graphBars())
        graphBars()->addSeries(series);

    QQuickGraphsNode::addSeriesInternal(series);
}

void QQuickGraphsBarsNode::removeSeries(QBar3DSeries *series)
{
    if (graphBars())
        graphBars()->removeSeries(series);
    QQuickGraphsNode::removeSeriesInternal(series);
}

void QQuickGraphsBarsNode::insertSeries(qsizetype index, QBar3DSeries *series)
{
    if (graphBars())
        graphBars()->insertSeries(index, series);

    QQuickGraphsNode::insertSeries(index, series);
}

void QQuickGraphsBarsNode::clearSelection()
{
    if (graphBars())
        graphBars()->clearSelection();
}

bool QQuickGraphsBarsNode::doPicking(QPointF point)
{
    if (graphBars())
        return graphBars()->doPicking(point);
    else
        return false;
}

bool QQuickGraphsBarsNode::doRayPicking(const QVector3D &origin, const QVector3D &direction)
{
    if (graphBars())
        return graphBars()->doRayPicking(origin, direction);
    else
        return false;
}


/*!
 * \internal
 */
QQuickGraphsBars *QQuickGraphsBarsNode::graphBars()
{
    return static_cast<QQuickGraphsBars *>(m_graph.get());
}

/*!
 * \internal
 */
const QQuickGraphsBars *QQuickGraphsBarsNode::graphBars() const
{
    return static_cast<QQuickGraphsBars *>(m_graph.get());
}

QT_END_NAMESPACE

#include "moc_qquickgraphsbarsnode_p.cpp"
