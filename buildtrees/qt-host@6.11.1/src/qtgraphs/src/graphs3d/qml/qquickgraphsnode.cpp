// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcustom3ditem.h"
#include "qquickgraphsnode_p.h"
#include <qtconfigmacros.h>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype GraphsNode
 * \inherits Node
 * \qmlabstract
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief Base type for 3D graphs.
 *
 * The uncreatable base type for all 3D graph nodes in Qt Graphs.
 *
 * \sa Bars3DNode, Scatter3DNode, Surface3DNode, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Graphs3D.SelectionMode GraphsNode::selectionMode
 * The active selection mode in the graph.
 * One of the \l graphs3d.selectionflag enum values.
 */

/*!
 * \qmlproperty GraphsTheme GraphsNode::theme
 * The active theme of the graph.
 *
 * \sa GraphsTheme
 */

/*!
 * \qmlproperty list<Custom3DItem> GraphsNode::customItemList
 *
 * The list of \l{Custom3DItem} items added to the graph. The graph takes
 * ownership of the added items.
 */

/*!
 * \qmlproperty bool GraphsNode::polar
 *
 * If \c {true}, the horizontal axes are changed into polar axes. The x-axis
 * becomes the angular axis and the z-axis becomes the radial axis.
 * Polar mode is not available for bar graphs.
 *
 * Defaults to \c{false}.
 *
 * \sa radialLabelOffset
 */

/*!
 * \qmlproperty real GraphsNode::labelMargin
 *
 * This property specifies the margin for the placement of the axis labels.
 *
 * Negative values place the labels inside the plot-area while positive values
 * place them outside the plot-area. Label automatic rotation is disabled when
 * the value is negative. Defaults to \c 0.1
 *
 * \sa QAbstract3DAxis::labelAutoAngle
 *
 */

/*!
 * \qmlproperty real GraphsNode::radialLabelOffset
 *
 * This property specifies the normalized horizontal offset for the axis labels
 * of the radial polar axis. The value \c 0.0 indicates that the labels should
 * be drawn next to the 0-angle angular axis grid line. The value \c 1.0
 * indicates that the labels are drawn in their usual place at the edge of the
 * graph background. This property is ignored if the polar property value is
 * \c{false}. Defaults to \c 1.0.
 *
 * \sa polar
 */

/*!
 * \qmlmethod void GraphsNode::clearSelection()
 * Clears selection from all attached series.
 */

/*!
 * \qmlmethod bool GraphsNode::hasSeries(Abstract3DSeries series)
 * Returns whether the \a series has already been added to the graph.
 */

/*!
 * \qmlmethod qsizetype GraphsNode::addCustomItem(Custom3DItem item)
 *
 * Adds a Custom3DItem \a item to the graph. Graph takes ownership of the added
 * item.
 *
 * \return index to the added item if add was successful, -1 if trying to add a
 * null item, and index of the item if trying to add an already added item.
 *
 * \sa removeCustomItems(), removeCustomItem(), removeCustomItemAt()
 */

/*!
 * \qmlmethod void GraphsNode::removeCustomItems()
 *
 * Removes all custom items. Deletes the resources allocated to them.
 */

/*!
 * \qmlmethod void GraphsNode::removeCustomItem(Custom3DItem item)
 *
 * Removes the custom \a {item}. Deletes the resources allocated to it.
 */

/*!
 * \qmlmethod void GraphsNode::removeCustomItemAt(vector3d position)
 *
 * Removes all custom items at \a {position}. Deletes the resources allocated to them.
 */

/*!
 * \qmlmethod void GraphsNode::releaseCustomItem(Custom3DItem item)
 *
 * Gets ownership of \a item back and removes the \a item from the graph.
 *
 * \note If the same item is added back to the graph, the texture file needs to
 * be re-set.
 *
 * \sa Custom3DItem::textureFile
 */

/*!
 * \qmlmethod void GraphsNode::doPicking(QPoint point)
 *
 * Performs picking using view coordinates from \a point
 * on the elements of the graph, selecting the first item hit.
 * Default input handling performs this upon receiving the onTapped event.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod void GraphsNode::doRayPicking(QVector3D origin, QVector3D direction)
 *
 * Performs picking starting from \a origin and in \a direction
 * on the elements of the graph, selecting the first item hit.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod int GraphsNode::selectedLabelIndex()
 *
 * Can be used to query the index of the selected label after receiving
 * \c selectedElementChanged signal with any label type. Selection is valid
 * until the next \c selectedElementChanged signal.
 *
 * \return index of the selected label, or -1.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod Abstract3DAxis GraphsNode::selectedAxis()
 *
 * Can be used to get the selected axis after receiving \c selectedElementChanged
 * signal with any label type. Selection is valid until the next
 * \c selectedElementChanged signal.
 *
 * \return the selected axis, or null.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod qsizetype GraphsNode::selectedCustomItemIndex()
 *
 * Can be used to query the index of the selected custom item after receiving
 * \c selectedElementChanged signal with
 * \l{QtGraphs3D::ElementType::CustomItem}{ElementType.CustomItem} type.
 * Selection is valid until the next \c selectedElementChanged signal.
 *
 * \return index of the selected custom item, or -1.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod Custom3DItem GraphsNode::selectedCustomItem()
 *
 * Can be used to get the selected custom item after receiving
 * \c selectedElementChanged signal with
 * \l{QtGraphs3D::ElementType::CustomItem}{ElementType.CustomItem} type.
 * Ownership of the item remains with the graph.
 * Selection is valid until the next \c selectedElementChanged signal.
 *
 * \return the selected custom item, or null.
 *
 * \sa selectedElement
 */

/*!
 * \qmlproperty Graphs3D.ElementType GraphsNode::selectedElement
 * \readonly
 *
 * The element selected in the graph.
 *
 * This property can be used to query the selected element type.
 * The type is valid until a new selection is made in the graph and the
 * \c selectedElementChanged signal is emitted.
 *
 * The signal can be used for example for implementing customized input
 * handling, as demonstrated by the \l {Axis Handling} example.
 *
 * \sa selectedLabelIndex(), selectedAxis(), selectedCustomItemIndex(),
 * selectedCustomItem(), Bars3DNode::selectedSeries, Scatter3DNode::selectedSeries,
 * Scene3D::selectionQueryPosition, Graphs3D.ElementType
 */

/*!
 * \qmlproperty real GraphsNode::aspectRatio
 *
 * The ratio of the graph scaling between the longest axis on the horizontal
 * plane and the y-axis. Defaults to \c{2.0}.
 *
 * \note Has no effect on Bars3D.
 *
 * \sa horizontalAspectRatio
 */

/*!
 * \qmlproperty real GraphsNode::horizontalAspectRatio
 *
 * The ratio of the graph scaling between the x-axis and z-axis.
 * The value of \c 0.0 indicates automatic scaling according to axis ranges.
 * Defaults to \c{0.0}.
 *
 * \note Has no effect on Bars3DNode, which handles scaling on the horizontal plane
 * via the \l{Bars3DNode::barThickness}{barThickness} and
 * \l{Bars3DNode::barSpacing}{barSpacing} properties. Polar graphs also ignore this
 * property.
 *
 * \sa aspectRatio, polar, Bars3DNode::barThickness, Bars3DNode::barSpacing
 */

/*!
 * \qmlproperty Graphs3D.OptimizationHint GraphsNode::optimizationHint
 *
 * \brief Specifies whether the default or legacy mode is used for rendering optimization.
 *
 * The default mode uses instanced rendering, and provides the full feature set
 * at the best level of performance on most systems. The static mode optimizes
 * graph rendering and is ideal for large non-changing data sets. It is slower
 * with dynamic data changes and item rotations. Selection is not optimized, so
 * using the static mode with massive data sets is not advisable. Legacy mode
 * renders all items in th graph individually, without instancing. It should be
 * used only if default mode does not work, that is the same as if the target
 * system does not support instancing. Defaults to
 * \l{QtGraphs3D::OptimizationHint::Default}{Default}.
 *
 * \note On some environments, large graphs using static optimization may not
 * render, because all of the items are rendered using a single draw call, and
 * different graphics drivers support different maximum vertice counts per call.
 * This is mostly an issue on 32bit and OpenGL ES2 platforms. To work around
 * this issue, choose an item mesh with a low vertex count or use the point mesh.
 *
 * \sa Abstract3DSeries::mesh, Graphs3D.OptimizationHint
 */

/*!
 * \qmlproperty locale GraphsNode::locale
 *
 * Sets the locale used for formatting various numeric labels.
 * Defaults to the \c{"C"} locale.
 *
 * \sa Value3DAxis::labelFormat
 */

/*!
 * \qmlproperty vector3d GraphsNode::queriedGraphPosition
 * \readonly
 *
 * This read-only property contains the latest graph position values along each
 * axis queried using Scene3D::graphPositionQuery. The values are normalized to
 * range \c{[-1, 1]}. If the queried position was outside the graph bounds, the
 * values will not reflect the real position, but will instead be some undefined
 * position outside the range \c{[-1, 1]}. The value will be undefined until a
 * query is made.
 *
 * There is no single correct 3D coordinate to match a particular screen
 * position, so to be consistent, the queries are always done against the inner
 * sides of an invisible box surrounding the graph.
 *
 * \note Bar graphs only allow querying graph position at the graph floor level,
 * so the y-value is always zero for bar graphs and valid queries can be only
 * made at screen positions that contain the floor of the graph.
 *
 * \sa Scene3D::graphPositionQuery
 */

/*!
 * \qmlproperty real GraphsNode::margin
 *
 * The absolute value used for the space left between the edge of the
 * plottable graph area and the edge of the graph background.
 *
 * If the margin value is negative, the margins are determined automatically and
 * can vary according to the size of the items in the series and the type of the
 * graph. The value is interpreted as a fraction of the y-axis range if the
 * graph aspect ratios have not been changed from the default values.
 * Defaults to \c{-1.0}.
 *
 * \note Setting a smaller margin for a scatter graph than the automatically
 * determined margin can cause the scatter items at the edges of the graph to
 * overlap with the graph background.
 *
 * \note On scatter and surface graphs, if the margin is small in comparison to
 * the axis label size, the positions of the edge labels of the axes are
 * adjusted to avoid overlap with the edge labels of the neighboring axes.
 */

/*!
 * \qmlproperty Graphs3D.GridLineType GraphsNode::gridLineType
 *
 * Defines whether the grid lines type is \c Graphs3D.GridLineType.Shader or
 * \c Graphs3D.GridLineType.Geometry.
 *
 * This value affects all grid lines.
 *
 * \sa Graphs3D.GridLineType
 */

QQuickGraphsNode::QQuickGraphsNode(QQuick3DNode *parent)
    : QQuick3DNode(parent)
    , m_selectionMode(QtGraphs3D::SelectionFlag::Item)
    , m_aspectRatio(2.0)
    , m_optimizationHint(QtGraphs3D::OptimizationHint::Default)
    , m_polar(false)
    , m_labelMargin(.1f)
    , m_radialLabelOffset(1.0f)
    , m_horizontalAspectRatio(0.0)
    , m_locale(QLocale::c())
    , m_margin(-1.0)
    , m_gridLineType(QtGraphs3D::GridLineType::Geometry)
{
    QGraphsTheme *theme = new QGraphsTheme(this);
    setTheme(theme);
}

QQuickGraphsNode::~QQuickGraphsNode() {}

void QQuickGraphsNode::componentComplete()
{
    QQuick3DNode::componentComplete();

    //initialize properties
    m_graph->setTheme(m_theme);
    m_graph->setAspectRatio(m_aspectRatio);
    m_graph->setOptimizationHint(m_optimizationHint);
    m_graph->setPolar(m_polar);
    m_graph->setLabelMargin(m_labelMargin);
    m_graph->setRadialLabelOffset(m_radialLabelOffset);
    m_graph->setHorizontalAspectRatio(m_horizontalAspectRatio);
    m_graph->setLocale(m_locale);
    m_graph->setMargin(m_margin);
    m_graph->setGridLineType(m_gridLineType);

    for (auto item : std::as_const(m_customItemList))
        m_graph->addCustomItem(item);

    //connect signals
    connect(m_graph.get(),
            &QQuickGraphsItem::selectionModeChanged,
            this,
            &QQuickGraphsNode::selectionModeChanged);
    connect(m_graph.get(), &QQuickGraphsItem::themeChanged, this, &QQuickGraphsNode::themeChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::aspectRatioChanged,
            this,
            &QQuickGraphsNode::aspectRatioChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::optimizationHintChanged,
            this,
            &QQuickGraphsNode::optimizationHintChanged);
    connect(m_graph.get(), &QQuickGraphsItem::polarChanged, this, &QQuickGraphsNode::polarChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::labelMarginChanged,
            this,
            &QQuickGraphsNode::labelMarginChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::radialLabelOffsetChanged,
            this,
            &QQuickGraphsNode::radialLabelOffsetChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::horizontalAspectRatioChanged,
            this,
            &QQuickGraphsNode::horizontalAspectRatioChanged);
    connect(m_graph.get(), &QQuickGraphsItem::localeChanged, this, &QQuickGraphsNode::localeChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::queriedGraphPositionChanged,
            this,
            &QQuickGraphsNode::queriedGraphPositionChanged);
    connect(m_graph.get(), &QQuickGraphsItem::marginChanged, this, &QQuickGraphsNode::marginChanged);
    connect(m_graph.get(),
            &QQuickGraphsItem::gridLineTypeChanged,
            this,
            &QQuickGraphsNode::gridLineTypeChanged);
}

void QQuickGraphsNode::setGraphParent()
{
    if (m_graph)
        m_graph->setParentNode(this);
}

void QQuickGraphsNode::setSelectionMode(QtGraphs3D::SelectionFlags selectionMode)
{
    if (m_selectionMode == selectionMode)
        return;

    m_selectionMode = selectionMode;
    if (m_graph)
        m_graph->setSelectionMode(m_selectionMode);
}

QtGraphs3D::SelectionFlags QQuickGraphsNode::selectionMode() const
{
    if (m_graph)
        return m_graph->selectionMode();
    else
        return m_selectionMode;
}

void QQuickGraphsNode::setTheme(QGraphsTheme *theme)
{
    if (m_theme == theme)
        return;

    m_theme = theme;
    if (m_graph)
        m_graph->setTheme(m_theme);
}

QGraphsTheme *QQuickGraphsNode::theme() const
{
    if (m_graph)
        return m_graph->theme();
    else
        return m_theme;
}

QQmlListProperty<QCustom3DItem> QQuickGraphsNode::customItemList() const
{
    if (m_graph)
        return m_graph->customItemList();
    else
        return QQmlListProperty<QCustom3DItem>();
}


QtGraphs3D::ElementType QQuickGraphsNode::selectedElement() const
{
    if (m_graph)
        return m_graph->selectedElement();
    else
        return QtGraphs3D::ElementType::None;
}

void QQuickGraphsNode::setAspectRatio(qreal aspectRatio)
{
    if (m_aspectRatio == aspectRatio)
        return;

    m_aspectRatio = aspectRatio;
    if (m_graph)
        m_graph->setAspectRatio(aspectRatio);
}

qreal QQuickGraphsNode::aspectRatio() const
{
    if (m_graph)
        return m_graph->aspectRatio();
    else
        return m_aspectRatio;
}

void QQuickGraphsNode::setOptimizationHint(QtGraphs3D::OptimizationHint optimizationHint)
{
    if (m_optimizationHint == optimizationHint)
        return;

    m_optimizationHint = optimizationHint;
    if (m_graph)
        m_graph->setOptimizationHint(optimizationHint);
}
QtGraphs3D::OptimizationHint QQuickGraphsNode::optimizationHint() const
{
    if (m_graph)
        return m_graph->optimizationHint();
    else
        return m_optimizationHint;
}
void QQuickGraphsNode::setPolar(bool enabled)
{
    if (m_polar == enabled)
        return;

    m_polar = enabled;
    if (m_graph)
        m_graph->setPolar(enabled);
}

bool QQuickGraphsNode::isPolar() const
{
    if (m_graph)
        return m_graph->isPolar();
    else
        return m_polar;
}

void QQuickGraphsNode::setLabelMargin(float labelMargin)
{
    if (m_labelMargin == labelMargin)
        return;

    m_labelMargin = labelMargin;
    if (m_graph)
        m_graph->setLabelMargin(labelMargin);
}

float QQuickGraphsNode::labelMargin() const
{
    if (m_graph)
        return m_graph->labelMargin();
    else
        return m_labelMargin;
}

void QQuickGraphsNode::setRadialLabelOffset(float radialLabelOffset)
{
    if (m_radialLabelOffset == radialLabelOffset)
        return;

    m_radialLabelOffset = radialLabelOffset;
    if (m_graph)
        m_graph->setRadialLabelOffset(radialLabelOffset);
}

float QQuickGraphsNode::radialLabelOffset() const
{
    if (m_graph)
        return m_graph->radialLabelOffset();
    else
        return m_radialLabelOffset;
}

void QQuickGraphsNode::setHorizontalAspectRatio(qreal horizontalAspectRatio)
{
    if (m_horizontalAspectRatio == horizontalAspectRatio)
        return;

    m_horizontalAspectRatio = horizontalAspectRatio;
    if (m_graph)
        m_graph->setHorizontalAspectRatio(horizontalAspectRatio);
}

qreal QQuickGraphsNode::horizontalAspectRatio() const
{
    if (m_graph)
        return m_graph->horizontalAspectRatio();
    else
        return m_horizontalAspectRatio;
}

void QQuickGraphsNode::setLocale(QLocale locale)
{
    if (m_locale == locale)
        return;

    m_locale = locale;
    if (m_graph)
        m_graph->setLocale(locale);
}

QLocale QQuickGraphsNode::locale() const
{
    if (m_graph)
        return m_graph->locale();
    else
        return m_locale;
}

QVector3D QQuickGraphsNode::queriedGraphPosition() const
{
    if (m_graph)
        return m_graph->queriedGraphPosition();
    else
        return QVector3D();
}

qreal QQuickGraphsNode::margin() const
{
    if (m_graph)
        return m_graph->margin();
    else
        return m_margin;
}

void QQuickGraphsNode::setMargin(qreal margin)
{
    if (m_margin == margin)
        return;

    m_margin = margin;
    if (m_graph)
        m_graph->setMargin(margin);
}

QtGraphs3D::GridLineType QQuickGraphsNode::gridLineType() const
{
    if (m_graph)
        return m_graph->gridLineType();
    else
        return m_gridLineType;
}
void QQuickGraphsNode::setGridLineType(const QtGraphs3D::GridLineType &gridLineType)
{
    if (m_gridLineType == gridLineType)
        return;

    m_gridLineType = gridLineType;
    if (m_graph)
        m_graph->setGridLineType(gridLineType);
}

bool QQuickGraphsNode::hasSeries(QAbstract3DSeries *series)
{
    if (m_graph)
        return m_graph->hasSeries(series);
    else
        return m_seriesList.contains(series);
}
void QQuickGraphsNode::addSeriesInternal(QAbstract3DSeries *series)
{
    insertSeries(m_seriesList.size(), series);
}

void QQuickGraphsNode::insertSeries(qsizetype index, QAbstract3DSeries *series)
{
    if (series) {
        if (m_seriesList.contains(series)) {
            qsizetype oldIndex = m_seriesList.indexOf(series);
            if (index != oldIndex) {
                m_seriesList.removeOne(series);
                if (oldIndex < index)
                    index--;
                m_seriesList.insert(index, series);
            }
        } else {
            m_seriesList.insert(index, series);
        }
    }
}

void QQuickGraphsNode::removeSeriesInternal(QAbstract3DSeries *series)
{
    if (series)
        m_seriesList.removeAll(series);
}

QList<QAbstract3DSeries *> QQuickGraphsNode::seriesList()
{
    return m_seriesList;
}

qsizetype QQuickGraphsNode::addCustomItem(QCustom3DItem *item)
{
    if (m_graph)
        m_graph->addCustomItem(item);

    m_customItemList.append(item);
    return m_customItemList.size() - 1;
}

void QQuickGraphsNode::removeCustomItems()
{
    if (m_graph)
        m_graph->removeCustomItems();

    m_customItemList.clear();
}

void QQuickGraphsNode::removeCustomItem(QCustom3DItem *item)
{
    if (m_graph)
        m_graph->deleteCustomItem(item);

    m_customItemList.removeOne(item);
}

void QQuickGraphsNode::removeCustomItemAt(QVector3D position)
{
    if (m_graph)
        m_graph->removeCustomItemAt(position);

    for (QCustom3DItem *item : std::as_const(m_customItemList)) {
        if (item->position() == position)
            m_customItemList.removeOne(item);
    }
}

void QQuickGraphsNode::releaseCustomItem(QCustom3DItem *item)
{
    if (m_graph)
        m_graph->releaseCustomItem(item);

    m_customItemList.removeOne(item);
}

int QQuickGraphsNode::selectedLabelIndex() const
{
    if (m_graph)
        return selectedLabelIndex();
    else
        return -1;
}

QAbstract3DAxis *QQuickGraphsNode::selectedAxis() const
{
    if (m_graph)
        return m_graph->selectedAxis();
    return nullptr;
}

qsizetype QQuickGraphsNode::selectedCustomItemIndex() const
{
    if (m_graph)
        return m_graph->selectedCustomItemIndex();
    else
        return -1;
}

QCustom3DItem *QQuickGraphsNode::selectedCustomItem() const
{
    if (m_graph)
        return m_graph->selectedCustomItem();
    else
        return nullptr;
}

QT_END_NAMESPACE

#include "moc_qquickgraphsnode_p.cpp"
