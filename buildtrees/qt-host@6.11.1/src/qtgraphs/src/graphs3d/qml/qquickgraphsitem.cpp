// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qbar3dseries.h"
#include "qquickgraphsitem_p.h"

#include "q3dscene_p.h"
#include "qabstract3daxis_p.h"
#include "qabstract3dseries.h"
#include "qabstract3dseries_p.h"
#include "qcategory3daxis.h"
#include "qcategory3daxis_p.h"
#include "qcustom3ditem.h"
#include "qcustom3ditem_p.h"
#include "qcustom3dlabel.h"
#include "qcustom3dvolume.h"
#include "qgraphsinputhandler_p.h"
#include "qgraphstheme.h"
#include "qscatter3dseries.h"
#include "qsurface3dseries.h"
#include "qvalue3daxis.h"
#include "qvalue3daxis_p.h"
#include "utils_p.h"
#include "qgraphs3dlogging_p.h"

#include <QtGui/QGuiApplication>

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dloader_p.h>
#include <QtQuick3D/private/qquick3dorthographiccamera_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>

#include <QtGui/qquaternion.h>

#include <qtgraphs_tracepoints_p.h>

#if defined(Q_OS_IOS)
#include <QtCore/QTimer>
#endif

#if defined(Q_OS_MACOS)
#include <qpa/qplatformnativeinterface.h>
#endif

QT_BEGIN_NAMESPACE

Q_TRACE_PREFIX(qtgraphs,
                   "QT_BEGIN_NAMESPACE" \
                   "class QQuickGraphsItem;" \
                   "QT_END_NAMESPACE"
               )

Q_TRACE_POINT(qtgraphs, QGraphs3DItemCtor_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemCtor_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemInit_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemInit_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemSynch_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemSynch_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateGrid_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateGrid_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCamera_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCamera_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemDoPicking_entry, float posX, float posY);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemDoPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemDoRayPicking_entry, float originX, float originY,
                            float originZ, float directionX, float directionY, float directionZ);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemDoRayPicking_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemCreateSliceView_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemCreateSliceView_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemCreateOffscreenSliceView_entry, int sliceType);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemCreateOffscreenSliceView_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateSliceGrid_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateSliceGrid_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateSliceLabels_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateSliceLabels_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCustomData_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCustomData_exit);

Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCustomVolumes_entry);
Q_TRACE_POINT(qtgraphs, QGraphs3DItemUpdateCustomVolumes_exit);


constexpr float doublePi = static_cast<float>(M_PI) * 2.0f;
constexpr float polarRoundness = 64.0f;

/*!
 * \qmltype GraphsItem3D
 * \qmlabstract
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief Base type for 3D graphs.
 *
 * The uncreatable base type for all 3D graphs in QtGraphs.
 *
 * \sa Bars3D, Scatter3D, Surface3D, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Graphs3D.SelectionMode GraphsItem3D::selectionMode
 * The active selection mode in the graph.
 * One of the \l Graphs3D.SelectionFlag enum values.
 */

/*!
 * \qmlproperty Graphs3D.ShadowQuality GraphsItem3D::shadowQuality
 * The quality of shadows. One of the \l Graphs3D.ShadowQuality enum
 * values.
 */

/*!
 * \qmlproperty Graphs3D.CameraPreset GraphsItem3D::cameraPreset
 *
 * The currently active camera preset, which is one of
 * \l{Graphs3D.CameraPreset}. If no
 * preset is active, the value is \c {Graphs3D.CameraPreset.NoPreset}.
 */

/*!
 * \qmlproperty real GraphsItem3D::cameraXRotation
 *
 * The X-rotation angle of the camera around the target point in degrees
 * starting from the current base position.
 */

/*!
 * \qmlproperty real GraphsItem3D::cameraYRotation
 *
 * The Y-rotation angle of the camera around the target point in degrees
 * starting from the current base position.
 */

/*!
 * \qmlproperty real GraphsItem3D::minCameraXRotation
 * \since 6.9
 *
 * \brief The minimum X-rotation angle of the camera around the target point in degrees.
 * The default value is \c{-180.0}
 */

/*!
 * \qmlproperty real GraphsItem3D::maxCameraXRotation
 * \since 6.9
 *
 * \brief The maximum X-rotation angle of the camera around the target point in degrees.
 * The default value is \c{180.0}
 */

/*!
 * \qmlproperty real GraphsItem3D::minCameraYRotation
 * \since 6.9
 *
 * \brief The minimum Y-rotation angle of the camera around the target point in degrees.
 * The default value is \c{0.0}
 */

/*!
 * \qmlproperty real GraphsItem3D::maxCameraYRotation
 * \since 6.9
 *
 * \brief The maximum Y-rotation angle of the camera around the target point in degrees.
 * The default value is \c{90.0}
 */

/*!
 * \qmlproperty bool GraphsItem3D::zoomAtTargetEnabled
 *
 * Whether zooming should change the camera target so that the zoomed point
 * of the graph stays at the same location after the zoom.
 *
 * Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool GraphsItem3D::zoomEnabled
 *
 * Whether this input handler allows graph zooming.
 *
 * Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool GraphsItem3D::selectionEnabled
 *
 * Whether this input handler allows selection from the graph.
 *
 * Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool GraphsItem3D::rotationEnabled
 *
 * Whether this input handler allows graph rotation.
 *
 * Defaults to \c{true}.
 */

/*!
 * \qmlproperty real GraphsItem3D::cameraZoomLevel
 *
 * The camera zoom level in percentage. The default value of \c{100.0}
 * means there is no zoom in or out set in the camera.
 * The value is limited by the minCameraZoomLevel and maxCameraZoomLevel
 * properties.
 *
 * \sa minCameraZoomLevel, maxCameraZoomLevel
 */

/*!
 * \qmlproperty real GraphsItem3D::minCameraZoomLevel
 *
 * Sets the minimum allowed camera zoom level.
 * If the new minimum level is higher than the existing maximum level, the
 * maximum level is adjusted to the new minimum as well.
 * If the current cameraZoomLevel is outside the new bounds, it is adjusted as
 * well. The minCameraZoomLevel cannot be set below \c{1.0}.
 * Defaults to \c{10.0}.
 *
 * \sa cameraZoomLevel, maxCameraZoomLevel
 */

/*!
 * \qmlproperty real GraphsItem3D::maxCameraZoomLevel
 *
 * Sets the maximum allowed camera zoom level.
 * If the new maximum level is lower than the existing minimum level, the
 * minimum level is adjusted to the new maximum as well.
 * If the current cameraZoomLevel is outside the new bounds, it is adjusted as
 * well. Defaults to \c{500.0f}.
 *
 * \sa cameraZoomLevel, minCameraZoomLevel
 */

/*!
 * \qmlproperty bool GraphsItem3D::wrapCameraXRotation
 *
 * The behavior of the minimum and maximum limits in the X-rotation.
 * By default, the X-rotation wraps from minimum value to maximum and from
 * maximum to minimum.
 *
 * If set to \c true, the X-rotation of the camera is wrapped from minimum to
 * maximum and from maximum to minimum. If set to \c false, the X-rotation of
 * the camera is limited to the sector determined by the minimum and maximum
 * values.
 */

/*!
 * \qmlproperty bool GraphsItem3D::wrapCameraYRotation
 *
 * The behavior of the minimum and maximum limits in the Y-rotation.
 * By default, the Y-rotation is limited between the minimum and maximum values
 * without any wrapping.
 *
 * If \c true, the Y-rotation of the camera is wrapped from minimum to maximum
 * and from maximum to minimum. If \c false, the Y-rotation of the camera is
 * limited to the sector determined by the minimum and maximum values.
 */

/*!
 * \qmlproperty vector3d GraphsItem3D::cameraTargetPosition
 *
 * The camera target as a vector3d. Defaults to \c {vector3d(0.0, 0.0, 0.0)}.
 *
 * Valid coordinate values are between \c{-1.0...1.0}, where the edge values
 * indicate the edges of the corresponding axis range. Any values outside this
 * range are clamped to the edge.
 *
 */

/*!
 * \qmlproperty Node GraphsItem3D::rootNode
 * \readonly
 * \since 6.9
 *
 * Returns a pointer to the root node of the 3D graph. Use this property
 * for injecting a 3D graph into a separate \l {View3D} using
 * \l {View3D::}{importScene}:
 *
 * \code
 * Bars3D {
 *   id: bars
 * }
 * View3D {
 *   importScene: bars.rootNode
 * }
 * \endcode
 *
 * \sa {View3D}
 */

/*!
 * \qmlproperty Scene3D GraphsItem3D::scene
 * \readonly
 *
 * The Scene3D pointer that can be used to manipulate the scene and access the
 * scene elements.
 *
 * This property is read-only.
 */

/*!
 * \qmlproperty GraphsTheme GraphsItem3D::theme
 * The active theme of the graph.
 *
 * \sa GraphsTheme
 */

/*!
 * \qmlproperty Graphs3D.RenderingMode GraphsItem3D::renderingMode
 *
 * How the graph will be rendered. Defaults to \c{Indirect}.
 *
 * \note Setting the \c antialiasing property of the graph does not do anything.
 * However, it is set by the graph itself if the current rendering mode uses
 * antialiasing.
 *
 * \sa msaaSamples, Graphs3D.RenderingMode
 */

/*!
 * \qmlproperty Graphs3D.TransparencyTechnique GraphsItem3D::transparencyTechnique
 * \since 6.9
 *
 * Specifies which transparency technique to use. The Default value is \c{Default}.
 * When rendering transparent surface graphs, use \c{Approximate} or \c{Accurate}.
 * \c Default should be used for other graph types.
 *
 * \value Default
 *        Indicates that order-independent transparency techniques are not used.
 *        Offers the best performance. Use when graphs don't contain
 *        transparency or when a bar or scatter graph is also using instancing,
 *        that is \l optimizationHint is {QtGraphs3D::OptimizationHint::Default}.
 *
 * \value Approximate
 *        Indicates that a graph attempts an approximation of order-independent
 *        transparency. This method is faster than \c Accurate and works on older
 *        hardware but may yield inaccurate results. Use when the order-independent
 *        transparency is needed, but the performance cost has to be lower than
 *        when using accurate order-independent transparency.
 *
 * \value Accurate
 *        Indicates that accurate order-independent transparency is used.
 *        Use when perfect transparency rendering is needed.
 */

/*!
 * \qmlproperty int GraphsItem3D::msaaSamples
 * The number of samples used in multisample antialiasing when renderingMode
 * is \c Indirect. When renderingMode is \c DirectToBackground, this property
 * value is read-only and returns the number of samples specified by the window
 * surface format.
 * Defaults to \c{4}.
 *
 * \sa renderingMode
 */

/*!
 * \qmlproperty bool GraphsItem3D::measureFps
 *
 * If \c {true}, the rendering is done continuously instead of on demand, and
 * the value of the currentFps property is updated. Defaults to \c{false}.
 *
 * \sa currentFps
 */

/*!
 * \qmlproperty int GraphsItem3D::currentFps
 *
 * When FPS measuring is enabled, the results for the last second are stored in
 * this read-only property. It takes at least a second before this value updates
 * after measuring is activated.
 *
 * \sa measureFps
 */

/*!
 * \qmlproperty list<Custom3DItem> GraphsItem3D::customItemList
 *
 * The list of \l{Custom3DItem} items added to the graph. The graph takes
 * ownership of the added items.
 */

/*!
 * \qmlproperty bool GraphsItem3D::polar
 *
 * If \c {true}, the horizontal axes are changed into polar axes. The x-axis
 * becomes the angular axis and the z-axis becomes the radial axis.
 * Polar mode is not available for bar graphs.
 *
 * Defaults to \c{false}.
 *
 * \sa orthoProjection, radialLabelOffset
 */

/*!
 * \qmlproperty real GraphsItem3D::labelMargin
 *
 * \brief This property specifies the margin for the placement of the axis labels.
 *
 * Negative values place the labels inside the plot-area while positive values
 * place them outside the plot-area. Label automatic rotation is disabled when
 * the value is negative. Defaults to \c 0.1
 *
 * \sa QAbstract3DAxis::labelAutoAngle
 *
 */

/*!
 * \qmlproperty real GraphsItem3D::radialLabelOffset
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
 * \qmlmethod void GraphsItem3D::setDefaultInputHandler()
 * @brief Restores the default input handling mechanism.
 *
 * This method reactivates the internal, pre-defined input handlers.
 * Use this to revert to default behavior after previously calling
 * \l{unsetDefaultInputHandler}.
 * \sa unsetDefaultInputHandler()
 */

 /*!
 * \qmlmethod void GraphsItem3D::unsetDefaultInputHandler()
 * @brief Disables the default input handling mechanism.
 *
 * This method deactivates the pre-defined all default input handlers.
 * Call this method when you intend to override the default behaviors.
 * \sa unsetDefaultTapHandler(), unsetDefaultDragHandler(), unsetDefaultWheelHandler(), unsetDefaultPinchHandler()
 */

 /*!
 * \qmlmethod void GraphsItem3D::unsetDefaultTapHandler()
 * @brief Disables the default tap event handler.
 *
 * This specifically unsets the built-in response to tap or click events,
 * allowing for custom tap handling logic.
 * \sa unsetDefaultInputHandler()
 */

 /*!
 * \qmlmethod void GraphsItem3D::unsetDefaultDragHandler()
 * @brief Disables the default drag event handler.
 *
 * This specifically unsets the built-in response to drag gestures.
 * Use this if you need to implement a custom handler for dragging.
 * \sa unsetDefaultInputHandler()
 */

 /*!
 * \qmlmethod void GraphsItem3D::unsetDefaultWheelHandler()
 * @brief Disables the default mouse wheel event handler.
 *
 * This specifically unsets the built-in response to mouse wheel scrolling.
 * Use this when implementing custom wheel behavior.
 * \sa unsetDefaultInputHandler()
 */

 /*!
 * \qmlmethod void GraphsItem3D::unsetDefaultPinchHandler()
 * @brief Disables the default pinch gesture handler.
 *
 * This specifically unsets the built-in response to pinch gestures.
 * Use this to implement custom pinch gesture handler.
 * \sa unsetDefaultInputHandler()
 */

  /*!
 * \qmlmethod void GraphsItem3D::setDragButton(Qt::MouseButtons button)
 * @brief Sets the mouse button used to trigger drag events.
 *
 * This method allows changing the mouse button required to initiate
 * a drag operation to \a button.
 * Defaults to \c {Qt::RightButton}.
 */

/*!
 * \qmlmethod void GraphsItem3D::clearSelection()
 * Clears selection from all attached series.
 */

/*!
 * \qmlmethod bool GraphsItem3D::hasSeries(Abstract3DSeries series)
 * Returns whether the \a series has already been added to the graph.
 */

/*!
 * \qmlmethod qsizetype GraphsItem3D::addCustomItem(Custom3DItem item)
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
 * \qmlmethod void GraphsItem3D::removeCustomItems()
 *
 * Removes all custom items. Deletes the resources allocated to them.
 */

/*!
 * \qmlmethod void GraphsItem3D::removeCustomItem(Custom3DItem item)
 *
 * Removes the custom \a {item}. Deletes the resources allocated to it.
 */

/*!
 * \qmlmethod void GraphsItem3D::removeCustomItemAt(vector3d position)
 *
 * Removes all custom items at \a {position}. Deletes the resources allocated to them.
 */

/*!
 * \qmlmethod void GraphsItem3D::releaseCustomItem(Custom3DItem item)
 *
 * Gets ownership of \a item back and removes the \a item from the graph.
 *
 * \note If the same item is added back to the graph, the texture file needs to
 * be re-set.
 *
 * \sa Custom3DItem::textureFile
 */

/*!
 * \qmlmethod void GraphsItem3D::doPicking(QPoint point)
 *
 * Performs picking using view coordinates from \a point
 * on the elements of the graph, selecting the first item hit.
 * Default input handling performs this upon receiving the onTapped event.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod void GraphsItem3D::doRayPicking(QVector3D origin, QVector3D direction)
 *
 * Performs picking starting from \a origin and in \a direction
 * on the elements of the graph, selecting the first item hit.
 *
 * \sa selectedElement
 */

/*!
 * \qmlmethod int GraphsItem3D::selectedLabelIndex()
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
 * \qmlmethod Abstract3DAxis GraphsItem3D::selectedAxis()
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
 * \qmlmethod qsizetype GraphsItem3D::selectedCustomItemIndex()
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
 * \qmlmethod Custom3DItem GraphsItem3D::selectedCustomItem()
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
 * \qmlproperty Graphs3D.ElementType GraphsItem3D::selectedElement
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
 * selectedCustomItem(), Bars3D::selectedSeries, Scatter3D::selectedSeries,
 * Scene3D::selectionQueryPosition, Graphs3D.ElementType
 */

/*!
 * \qmlproperty bool GraphsItem3D::orthoProjection
 *
 * If \c {true}, orthographic projection will be used for displaying the graph.
 * Defaults to \c{false}.
 * \note Shadows will be disabled when set to \c{true}.
 */

/*!
 * \qmlproperty real GraphsItem3D::aspectRatio
 *
 * The ratio of the graph scaling between the longest axis on the horizontal
 * plane and the y-axis. Defaults to \c{2.0}.
 *
 * \note Has no effect on Bars3D.
 *
 * \sa horizontalAspectRatio
 */

/*!
 * \qmlproperty real GraphsItem3D::horizontalAspectRatio
 *
 * The ratio of the graph scaling between the x-axis and z-axis.
 * The value of \c 0.0 indicates automatic scaling according to axis ranges.
 * Defaults to \c{0.0}.
 *
 * \note Has no effect on Bars3D, which handles scaling on the horizontal plane
 * via the \l{Bars3D::barThickness}{barThickness} and
 * \l{Bars3D::barSpacing}{barSpacing} properties. Polar graphs also ignore this
 * property.
 *
 * \sa aspectRatio, polar, Bars3D::barThickness, Bars3D::barSpacing
 */

/*!
 * \qmlproperty Graphs3D.OptimizationHint GraphsItem3D::optimizationHint
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
 * \qmlproperty locale GraphsItem3D::locale
 *
 * Sets the locale used for formatting various numeric labels.
 * Defaults to the \c{"C"} locale.
 *
 * \sa Value3DAxis::labelFormat
 */

/*!
 * \qmlproperty vector3d GraphsItem3D::queriedGraphPosition
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
 * \qmlproperty real GraphsItem3D::margin
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
 * \qmlproperty real GraphsItem3D::cutoffMargin
 * \since 6.11
 *
 * The value used for the difference between the axis limits
 * and the position at which graph elements are culled.
 *
 * \note Does not affect bar graphs.
 */

/*!
 * \qmlproperty Graphs3D.GridLineType GraphsItem3D::gridLineType
 *
 * Defines whether the grid lines type is \c Graphs3D.GridLineType.Shader or
 * \c Graphs3D.GridLineType.Geometry.
 *
 * This value affects all grid lines.
 *
 * \sa Graphs3D.GridLineType
 */

/*!
 * \qmlproperty real GraphsItem3D::shadowStrength
 *
 * The shadow strength for the whole graph. The higher the number, the darker
 * the shadows will be. The value must be between \c 0.0 and \c 100.0.
 *
 * This value affects the light specified in Scene3D.
 */

/*!
 * \qmlproperty real GraphsItem3D::lightStrength
 *
 * The specular light strength for the whole graph. The value must be between
 * \c 0.0 and \c 10.0.
 *
 * This value affects the light specified in Scene3D.
 */

/*!
 * \qmlproperty real GraphsItem3D::ambientLightStrength
 *
 * The ambient light strength for the whole graph. This value determines how
 * evenly and brightly the colors are shown throughout the graph regardless of
 * the light position. The value must be between \c 0.0 and \c 1.0.
 */

/*!
 * \qmlproperty color GraphsItem3D::lightColor
 *
 * The color of the ambient and specular light defined in Scene3D.
 */

/*!
 * \qmlsignal GraphsItem3D::tapped(QEventPoint eventPoint, Qt::MouseButton button)
 *
 * This signal is emitted when the graph item is tapped once. The \a eventPoint
 * signal parameter contains information from the release event about the point
 * that was tapped, and \a button is the \l {Qt::MouseButton}{mouse button} that was clicked,
 * or \c NoButton on a touchscreen.
 *
 * \sa QEventPoint, Qt::MouseButtons, TapHandler::singleTapped
 */

/*!
 * \qmlsignal GraphsItem3D::doubleTapped(QEventPoint eventPoint, Qt::MouseButton button)
 *
 * This signal is emitted when the graph item is tapped twice within a short span of time.
 * The \a eventPoint signal parameter contains information from the release event about the
 * point that was tapped, and \a button is the \l {Qt::MouseButton}{mouse button} that was
 * clicked, or \c NoButton on a touchscreen.
 *
 * \sa QEventPoint, Qt::MouseButtons, TapHandler::doubleTapped
 */

/*!
 * \qmlsignal GraphsItem3D::longPressed()
 *
 * This signal is emitted when the \c parent Item is pressed and held for a
 * time period greater than \l TapHandler::longPressThreshold.
 *
 * \sa TapHandler::longPressed
 */

/*!
 * \qmlsignal GraphsItem3D::dragged(QVector2D delta)
 *
 * This signal is emitted when the translation of the cluster of points
 * on the graph is changed while the pinch gesture is being performed.
 *  The \a delta vector gives the change in translation.
 *
 * \sa PinchHandler::translationChanged
 */

/*!
 * \qmlsignal GraphsItem3D::wheel(QQuickWheelEvent *event)
 *
 * This signal is emitted every time the graph receives an \a event
 * of type \l QWheelEvent: that is, every time the wheel is moved or the
 * scrolling gesture is updated.
 *
 * \sa WheelEvent, WheelHandler::wheel
 */

/*!
 * \qmlsignal GraphsItem3D::pinch(qreal delta)
 *
 * This signal is emitted when the scale factor on the graph
 * changes while the pinch gesture is being performed.
 * The \a delta value gives the multiplicative change in scale.
 *
 * \sa PinchHandler::scaleChanged
 */

/*!
 * \qmlsignal GraphsItem3D::mouseMove(QPoint mousePos)
 *
 * This signal is emitted when the graph receives a mouseMove event.
 * \a mousePos value gives the position of mouse while mouse is moving.
 *
 * \sa QQuickItem::mouseMoveEvent
 */

QQuickGraphsItem::QQuickGraphsItem(QQuickItem *parent)
    : QQuick3DViewport(parent)
    , m_locale(QLocale::c())
{
    Q_TRACE(QGraphs3DItemCtor_entry);
    if (!m_scene)
        m_scene = new Q3DScene;
    m_scene->setParent(this);

    m_qml = this;

    // Set initial theme
    QGraphsTheme *theme = new QGraphsTheme(m_scene);
    setTheme(theme);
    QGraphsLine grid = theme->grid();
    grid.setMainWidth(0.25);
    theme->setGrid(grid);
    m_themes.append(theme);

    m_scene->d_func()->setViewport(boundingRect().toRect());

    connect(m_scene, &Q3DScene::needRender, this, &QQuickGraphsItem::emitNeedRender);
    connect(m_scene,
            &Q3DScene::graphPositionQueryChanged,
            this,
            &QQuickGraphsItem::handleGraphQueryPositionChanged);
    connect(m_scene,
            &Q3DScene::selectionQueryPositionChanged,
            this,
            &QQuickGraphsItem::handleSelectionQueryPositionChanged);
    connect(m_scene, &Q3DScene::primarySubViewportChanged,
            this,
            &QQuickGraphsItem::handlePrimarySubViewportChanged);
    connect(m_scene, &Q3DScene::secondarySubViewportChanged,
            this,
            &QQuickGraphsItem::handleSecondarySubViewportChanged);

    QQuick3DSceneEnvironment *scene = environment();
    scene->setBackgroundMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes::Color);
    scene->setClearColor(Qt::transparent);

    auto sceneManager = QQuick3DObjectPrivate::get(rootNode())->sceneManager;
    connect(sceneManager.data(),
            &QQuick3DSceneManager::windowChanged,
            this,
            &QQuickGraphsItem::handleWindowChanged);
    // Set contents to false in case we are in qml designer to make component look
    // nice
    m_runningInDesigner = QGuiApplication::applicationDisplayName() == QLatin1String("Qml2Puppet");
    setFlag(ItemHasContents /*, !m_runningInDesigner*/); // Is this relevant anymore?

    // Set 4x MSAA by default
    setRenderingMode(QtGraphs3D::RenderingMode::Indirect);
    setMsaaSamples(4);

    setTransparencyTechnique(QtGraphs3D::TransparencyTechnique::Default);

    // Accept touchevents
    setAcceptTouchEvents(true);

    m_inputHandler = new QGraphsInputHandler(this);
    m_inputHandler->bindableHeight().setBinding([&] { return height(); });
    m_inputHandler->bindableWidth().setBinding([&] { return width(); });
    Q_TRACE(QGraphs3DItemCtor_exit);
}

QQuickGraphsItem::~QQuickGraphsItem()
{
    disconnect(this, 0, this, 0);
    checkWindowList(0);

    m_repeaterX->model().clear();
    m_repeaterY->model().clear();
    m_repeaterZ->model().clear();
    m_repeaterX->deleteLater();
    m_repeaterY->deleteLater();
    m_repeaterZ->deleteLater();

    for (MultiAxis axis : m_multiAxesX)
        releaseMultiAxis(QAbstract3DAxis::AxisOrientation::X, axis.seriesIndex);
    for (MultiAxis axis : m_multiAxesX)
        releaseMultiAxis(QAbstract3DAxis::AxisOrientation::Y, axis.seriesIndex);
    for (MultiAxis axis : m_multiAxesX)
        releaseMultiAxis(QAbstract3DAxis::AxisOrientation::Z, axis.seriesIndex);

    delete m_gridGeometryModel;
    delete m_subgridGeometryModel;
    delete m_sliceGridGeometryModel;
}

void QQuickGraphsItem::handleAxisTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    handleAxisTitleChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisTitleChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXTitleChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYTitleChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZTitleChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    markSeriesItemLabelsDirty();
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisLabelsChanged()
{
    handleAxisLabelsChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisLabelsChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXLabelsChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYLabelsChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZLabelsChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    markSeriesItemLabelsDirty();
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisRangeChanged(float min, float max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    handleAxisRangeChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisRangeChangedBySender(QObject *sender)
{
    if (sender == m_axisX) {
        m_isSeriesVisualsDirty = true;
        m_changeTracker.axisXRangeChanged = true;
    } else if (sender == m_axisY) {
        m_isSeriesVisualsDirty = true;
        m_changeTracker.axisYRangeChanged = true;
    } else if (sender == m_axisZ) {
        m_isSeriesVisualsDirty = true;
        m_changeTracker.axisZRangeChanged = true;
    } else {
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    }
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisSegmentCountChanged(qsizetype count)
{
    Q_UNUSED(count);
    handleAxisSegmentCountChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisSegmentCountChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXSegmentCountChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYSegmentCountChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZSegmentCountChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisSubSegmentCountChanged(qsizetype count)
{
    Q_UNUSED(count);
    handleAxisSubSegmentCountChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisSubSegmentCountChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXSubSegmentCountChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYSubSegmentCountChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZSubSegmentCountChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisAutoAdjustRangeChanged(bool autoAdjust)
{
    QObject *sender = QObject::sender();
    if (sender != m_axisX && sender != m_axisY && sender != m_axisZ)
        return;

    QAbstract3DAxis *axis = static_cast<QAbstract3DAxis *>(sender);
    handleAxisAutoAdjustRangeChangedInOrientation(axis->orientation(), autoAdjust);
}

void QQuickGraphsItem::handleAxisLabelFormatChanged(const QString &format)
{
    Q_UNUSED(format);
    handleAxisLabelFormatChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisReversedChanged(bool enable)
{
    Q_UNUSED(enable);
    handleAxisReversedChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisFormatterDirty()
{
    handleAxisFormatterDirtyBySender(sender());
}

void QQuickGraphsItem::handleAxisLabelAutoRotationChanged(float angle)
{
    Q_UNUSED(angle);
    handleAxisLabelAutoRotationChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisScaleLabelsByCountChanged(bool adjust)
{
    Q_UNUSED(adjust);
    handleAxisScaleLabelsByCountChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisLabelSizeChanged(qreal size)
{
    Q_UNUSED(size);
    handleAxisLabelSizeChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisTitleVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    handleAxisTitleVisibilityChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisLabelVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    handleAxisLabelVisibilityChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisTitleFixedChanged(bool fixed)
{
    Q_UNUSED(fixed);
    handleAxisTitleFixedChangedBySender(sender());
}

void QQuickGraphsItem::handleAxisTitleOffsetChanged(float offset)
{
    Q_UNUSED(offset);
    handleAxisTitleFixedChangedBySender(sender());
}

void QQuickGraphsItem::handleInputPositionChanged(QPoint position)
{
    Q_UNUSED(position);
    emitNeedRender();
}

void QQuickGraphsItem::handleSeriesVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);

    handleSeriesVisibilityChangedBySender(sender());
}

void QQuickGraphsItem::handleItemLabelVisibleChanged(bool visible)
{
    handleItemLabelVisibleChangedBySender(visible, sender());
}

void QQuickGraphsItem::handleRequestShadowQuality(QtGraphs3D::ShadowQuality quality)
{
    setShadowQuality(quality);
}

void QQuickGraphsItem::handleGraphQueryPositionChanged(QPoint position)
{
    QVector3D data = graphPositionAt(position);
    setGraphPositionQueryPending(false);
    setQueriedGraphPosition(data);
    emit queriedGraphPositionChanged(data);
}

void QQuickGraphsItem::handleSelectionQueryPositionChanged(QPoint position)
{
    doPicking(position);
}

void QQuickGraphsItem::handlePrimarySubViewportChanged(const QRect rect)
{
    m_primarySubView = rect;
    updateSubViews();
}

void QQuickGraphsItem::handleSecondarySubViewportChanged(const QRect rect)
{
    m_secondarySubView = rect;
    updateSubViews();
}

void QQuickGraphsItem::handleAxisLabelFormatChangedBySender(QObject *sender)
{
    // Label format changing needs to dirty the data so that labels are reset.
    if (sender == m_axisX) {
        m_isDataDirty = true;
        m_changeTracker.axisXLabelFormatChanged = true;
    } else if (sender == m_axisY) {
        m_isDataDirty = true;
        m_changeTracker.axisYLabelFormatChanged = true;
    } else if (sender == m_axisZ) {
        m_isDataDirty = true;
        m_changeTracker.axisZLabelFormatChanged = true;
    } else {
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    }
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisReversedChangedBySender(QObject *sender)
{
    // Reversing change needs to dirty the data so item positions are recalculated
    if (sender == m_axisX) {
        m_isDataDirty = true;
        m_isCustomDataDirty = true;
        m_changeTracker.axisXReversedChanged = true;
    } else if (sender == m_axisY) {
        m_isDataDirty = true;
        m_isCustomDataDirty = true;
        m_changeTracker.axisYReversedChanged = true;
    } else if (sender == m_axisZ) {
        m_isDataDirty = true;
        m_isCustomDataDirty = true;
        m_changeTracker.axisZReversedChanged = true;
    } else {
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    }
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisFormatterDirtyBySender(QObject *sender)
{
    // Sender is QValue3DAxisPrivate
    QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(sender);
    if (valueAxis == m_axisX) {
        m_isDataDirty = true;
        m_changeTracker.axisXFormatterChanged = true;
    } else if (valueAxis == m_axisY) {
        m_isDataDirty = true;
        m_changeTracker.axisYFormatterChanged = true;
    } else if (valueAxis == m_axisZ) {
        m_isDataDirty = true;
        m_changeTracker.axisZFormatterChanged = true;
    } else {
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));
    }
    emitNeedRender();
}

void QQuickGraphsItem::handleAxisLabelAutoRotationChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXLabelAutoRotationChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYLabelAutoRotationChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZLabelAutoRotationChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisScaleLabelsByCountChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXScaleLabelsByCountChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYScaleLabelsByCountChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZScaleLabelsByCountChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisLabelSizeChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXLabelSizeChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYLabelSizeChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZLabelSizeChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisTitleVisibilityChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXTitleVisibilityChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYTitleVisibilityChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZTitleVisibilityChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisLabelVisibilityChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXLabelVisibilityChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYLabelVisibilityChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZLabelVisibilityChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisTitleFixedChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXTitleFixedChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYTitleFixedChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZTitleFixedChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleAxisTitleOffsetChangedBySender(QObject *sender)
{
    if (sender == m_axisX)
        m_changeTracker.axisXTitleOffsetChanged = true;
    else if (sender == m_axisY)
        m_changeTracker.axisYTitleOffsetChanged = true;
    else if (sender == m_axisZ)
        m_changeTracker.axisZTitleOffsetChanged = true;
    else
        qCWarning(lcGraphs3D, "%ls invoked for invalid axis",
                qUtf16Printable(QString::fromUtf8(__func__)));

    emitNeedRender();
}

void QQuickGraphsItem::handleMultiAxisChanged(QAbstract3DAxis *axis)
{
    m_changeTracker.multiAxisChanged = true;
    if (axis)
        connectMultiAxis(axis);
}

void QQuickGraphsItem::handleMultiAxisDirty()
{
    m_changeTracker.multiAxisChanged = true;
}

void QQuickGraphsItem::handleSeriesVisibilityChangedBySender(QObject *sender)
{
    QAbstract3DSeries *series = static_cast<QAbstract3DSeries *>(sender);
    series->d_func()->m_changeTracker.visibilityChanged = true;

    m_isDataDirty = true;
    m_isSeriesVisualsDirty = true;

    adjustAxisRanges();

    emitNeedRender();
}

void QQuickGraphsItem::markDataDirty()
{
    m_isDataDirty = true;

    markSeriesItemLabelsDirty();
    emitNeedRender();
}

void QQuickGraphsItem::markSeriesVisualsDirty()
{
    m_isSeriesVisualsDirty = true;
    emitNeedRender();
}

void QQuickGraphsItem::markSeriesItemLabelsDirty()
{
    for (int i = 0; i < m_seriesList.size(); i++)
        m_seriesList.at(i)->d_func()->markItemLabelDirty();
}

QAbstract3DAxis *QQuickGraphsItem::createDefaultAxis(QAbstract3DAxis::AxisOrientation orientation)
{
    Q_UNUSED(orientation);

    // The default default axis is a value axis. If the graph type has a different
    // default axis for some orientation, this function needs to be overridden.
    QAbstract3DAxis *defaultAxis = createDefaultValueAxis();
    return defaultAxis;
}

QValue3DAxis *QQuickGraphsItem::createDefaultValueAxis()
{
    // Default value axis has single segment, empty label format, and auto scaling
    QValue3DAxis *defaultAxis = new QValue3DAxis;
    defaultAxis->d_func()->setDefaultAxis(true);

    return defaultAxis;
}

QCategory3DAxis *QQuickGraphsItem::createDefaultCategoryAxis()
{
    // Default category axis has no labels
    QCategory3DAxis *defaultAxis = new QCategory3DAxis;
    defaultAxis->d_func()->setDefaultAxis(true);
    return defaultAxis;
}

void QQuickGraphsItem::setAxisHelper(QAbstract3DAxis::AxisOrientation orientation,
                                     QAbstract3DAxis *axis,
                                     QAbstract3DAxis **axisPtr)
{
    // Setting null axis indicates using default axis
    if (!axis)
        axis = createDefaultAxis(orientation);

    // If old axis is default axis, delete it
    QAbstract3DAxis *oldAxis = *axisPtr;
    if (oldAxis) {
        if (oldAxis->d_func()->isDefaultAxis()) {
            m_axes.removeAll(oldAxis);
            delete oldAxis;
            oldAxis = 0;
        } else {
            // Disconnect the old axis from use
            QObject::disconnect(oldAxis, 0, this, 0);
            oldAxis->d_func()->setOrientation(QAbstract3DAxis::AxisOrientation::None);
        }
    }

    // Assume ownership
    addAxis(axis);

    // Connect the new axis
    *axisPtr = axis;

    axis->d_func()->setOrientation(orientation);

    QObject::connect(axis,
                     &QAbstract3DAxis::titleChanged,
                     this,
                     &QQuickGraphsItem::handleAxisTitleChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelsChanged,
                     this,
                     &QQuickGraphsItem::handleAxisLabelsChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::rangeChanged,
                     this,
                     &QQuickGraphsItem::handleAxisRangeChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::autoAdjustRangeChanged,
                     this,
                     &QQuickGraphsItem::handleAxisAutoAdjustRangeChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelAutoAngleChanged,
                     this,
                     &QQuickGraphsItem::handleAxisLabelAutoRotationChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelAutoAngleChanged,
                     this,
                     &QQuickGraphsItem::handleAxisLabelAutoRotationChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::scaleLabelsByCountChanged,
                     this,
                     &QQuickGraphsItem::handleAxisScaleLabelsByCountChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelSizeChanged,
                     this,
                     &QQuickGraphsItem::handleAxisLabelSizeChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleVisibleChanged,
                     this,
                     &QQuickGraphsItem::handleAxisTitleVisibilityChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelVisibleChanged,
                     this,
                     &QQuickGraphsItem::handleAxisLabelVisibilityChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleFixedChanged,
                     this,
                     &QQuickGraphsItem::handleAxisTitleFixedChanged);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleOffsetChanged,
                     this,
                     &QQuickGraphsItem::handleAxisTitleOffsetChanged);

    if (orientation == QAbstract3DAxis::AxisOrientation::X)
        m_changeTracker.axisXTypeChanged = true;
    else if (orientation == QAbstract3DAxis::AxisOrientation::Y)
        m_changeTracker.axisYTypeChanged = true;
    else if (orientation == QAbstract3DAxis::AxisOrientation::Z)
        m_changeTracker.axisZTypeChanged = true;

    handleAxisTitleChangedBySender(axis);
    handleAxisLabelsChangedBySender(axis);
    handleAxisRangeChangedBySender(axis);
    handleAxisAutoAdjustRangeChangedInOrientation(axis->orientation(), axis->isAutoAdjustRange());
    handleAxisLabelAutoRotationChangedBySender(axis);
    handleAxisTitleVisibilityChangedBySender(axis);
    handleAxisLabelVisibilityChangedBySender(axis);
    handleAxisTitleFixedChangedBySender(axis);
    handleAxisTitleOffsetChangedBySender(axis);

    if (axis->type() == QAbstract3DAxis::AxisType::Value) {
        QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(axis);
        QObject::connect(valueAxis,
                         &QValue3DAxis::segmentCountChanged,
                         this,
                         &QQuickGraphsItem::handleAxisSegmentCountChanged);
        QObject::connect(valueAxis,
                         &QValue3DAxis::subSegmentCountChanged,
                         this,
                         &QQuickGraphsItem::handleAxisSubSegmentCountChanged);
        QObject::connect(valueAxis,
                         &QValue3DAxis::labelFormatChanged,
                         this,
                         &QQuickGraphsItem::handleAxisLabelFormatChanged);
        QObject::connect(valueAxis,
                         &QValue3DAxis::reversedChanged,
                         this,
                         &QQuickGraphsItem::handleAxisReversedChanged);
        // TODO: Handle this somehow (add API to QValue3DAxis?)
        //        QObject::connect(valueAxis->d_func(), &QValue3DAxisPrivate::formatterDirty,
        //                         this, &Abstract3DController::handleAxisFormatterDirty);

        handleAxisSegmentCountChangedBySender(valueAxis);
        handleAxisSubSegmentCountChangedBySender(valueAxis);
        handleAxisLabelFormatChangedBySender(valueAxis);
        handleAxisReversedChangedBySender(valueAxis);
        // TODO: Handle this somehow (add API to QValue3DAxis?)
        //        handleAxisFormatterDirtyBySender(valueAxis->d_func());

        valueAxis->formatter()->setLocale(m_locale);
    }
}

void QQuickGraphsItem::startRecordingRemovesAndInserts()
{
    // Default implementation does nothing
}

int QQuickGraphsItem::horizontalFlipFactor() const
{
    return m_horizontalFlipFactor;
}

void QQuickGraphsItem::setHorizontalFlipFactor(int newHorizontalFlipFactor)
{
    m_gridUpdate = true;
    m_horizontalFlipFactor = newHorizontalFlipFactor;
}

void QQuickGraphsItem::emitNeedRender()
{
    if (!m_renderPending) {
        emit needRender();
        m_renderPending = true;
    }
}

void QQuickGraphsItem::handleThemeColorStyleChanged(QGraphsTheme::ColorStyle style)
{
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.colorStyleOverride) {
            series->setColorStyle(style);
            series->d_func()->m_themeTracker.colorStyleOverride = false;
        }
    }
    theme()->dirtyBits()->colorStyleDirty = false;
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeBaseColorsChanged(const QList<QColor> &colors)
{
    int colorIdx = 0;
    // Set value for series that have not explicitly set this value
    if (!colors.size())
        return;

    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.baseColorOverride) {
            series->setBaseColor(colors.at(colorIdx));
            series->d_func()->m_themeTracker.baseColorOverride = false;
        }
        if (++colorIdx >= colors.size())
            colorIdx = 0;
    }

    theme()->dirtyBits()->seriesColorsDirty = false;
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeBaseGradientsChanged(const QList<QLinearGradient> &gradients)
{
    int gradientIdx = 0;
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.baseGradientOverride) {
            series->setBaseGradient(gradients.at(gradientIdx));
            series->d_func()->m_themeTracker.baseGradientOverride = false;
        }
        if (++gradientIdx >= gradients.size())
            gradientIdx = 0;
    }
    theme()->dirtyBits()->seriesGradientDirty = false;
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeSingleHighlightColorChanged(QColor color)
{
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.singleHighlightColorOverride) {
            series->setSingleHighlightColor(color);
            series->d_func()->m_themeTracker.singleHighlightColorOverride = false;
        }
    }
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeSingleHighlightGradientChanged(const QLinearGradient &gradient)
{
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.singleHighlightGradientOverride) {
            series->setSingleHighlightGradient(gradient);
            series->d_func()->m_themeTracker.singleHighlightGradientOverride = false;
        }
    }
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeMultiHighlightColorChanged(QColor color)
{
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.multiHighlightColorOverride) {
            series->setMultiHighlightColor(color);
            series->d_func()->m_themeTracker.multiHighlightColorOverride = false;
        }
    }
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeMultiHighlightGradientChanged(const QLinearGradient &gradient)
{
    // Set value for series that have not explicitly set this value
    for (QAbstract3DSeries *series : std::as_const(m_seriesList)) {
        if (!series->d_func()->m_themeTracker.multiHighlightGradientOverride) {
            series->setMultiHighlightGradient(gradient);
            series->d_func()->m_themeTracker.multiHighlightGradientOverride = false;
        }
    }
    markSeriesVisualsDirty();
}

void QQuickGraphsItem::handleThemeTypeChanged(QGraphsTheme::Theme theme)
{
    Q_UNUSED(theme);

    // Changing theme type is logically equivalent of changing the entire theme
    // object, so reset all attached series to the new theme.
    bool force = m_qml->isReady();
    QGraphsTheme *activeTheme = this->theme();
    for (int i = 0; i < m_seriesList.size(); i++)
        m_seriesList.at(i)->d_func()->resetToTheme(*activeTheme, i, force);

    markSeriesVisualsDirty();

    emit themeTypeChanged();
}

void QQuickGraphsItem::addSeriesInternal(QAbstract3DSeries *series)
{
    insertSeries(m_seriesList.size(), series);
}

void QQuickGraphsItem::insertSeries(qsizetype index, QAbstract3DSeries *series)
{
    if (series) {
        if (m_seriesList.contains(series)) {
            qsizetype oldIndex = m_seriesList.indexOf(series);
            if (index != oldIndex) {
                m_seriesList.removeOne(series);
                if (oldIndex < index)
                    index--;
                m_seriesList.insert(index, series);
                qCDebug(lcSeries3D) << __FUNCTION__
                    << series << "already exists at index of:" << oldIndex
                    << "removing it and inserting to index of:" << index;
            }
        } else {
            qsizetype oldSize = m_seriesList.size();
            m_seriesList.insert(index, series);
            series->d_func()->setGraph(this);
            QObject::connect(series,
                             &QAbstract3DSeries::visibleChanged,
                             this,
                             &QQuickGraphsItem::handleSeriesVisibilityChanged);
            QObject::connect(series,
                    &QAbstract3DSeries::lightingModeChanged,
                    this,
                    &QQuickGraphsItem::handleLightingModeChanged);
            QObject::connect(series,
                             &QAbstract3DSeries::itemLabelVisibleChanged,
                             this,
                             &QQuickGraphsItem::handleItemLabelVisibleChanged);
            series->d_func()->resetToTheme(*theme(), oldSize, false);
            qCDebug(lcSeries3D) << __FUNCTION__
                << "insert" << series << "at index of:" << index;
        }
        if (series->isVisible())
            handleSeriesVisibilityChangedBySender(series);
    }
}

void QQuickGraphsItem::removeSeriesInternal(QAbstract3DSeries *series)
{
    if (series && series->d_func()->m_graph == this) {
        m_seriesList.removeAll(series);
        QObject::disconnect(series,
                            &QAbstract3DSeries::visibleChanged,
                            this,
                            &QQuickGraphsItem::handleSeriesVisibilityChanged);
        QObject::disconnect(series,
                            &QAbstract3DSeries::lightingModeChanged,
                            this,
                            &QQuickGraphsItem::handleLightingModeChanged);
        series->d_func()->setGraph(0);
        m_isDataDirty = true;
        m_isSeriesVisualsDirty = true;
        qCDebug(lcSeries3D) << __FUNCTION__ << "removed" << series << "from seriesList";
        emitNeedRender();
    }
}

QList<QAbstract3DSeries *> QQuickGraphsItem::seriesList()
{
    return m_seriesList;
}

void QQuickGraphsItem::setAxisX(QAbstract3DAxis *axis)
{
    // Setting null axis will always create new default axis
    if (!axis || axis != m_axisX) {
        setAxisHelper(QAbstract3DAxis::AxisOrientation::X, axis, &m_axisX);
        emit axisXChanged(m_axisX);
    } else {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
    }
}

QAbstract3DAxis *QQuickGraphsItem::axisX() const
{
    return m_axisX;
}

void QQuickGraphsItem::setAxisY(QAbstract3DAxis *axis)
{
    // Setting null axis will always create new default axis
    if (!axis || axis != m_axisY) {
        setAxisHelper(QAbstract3DAxis::AxisOrientation::Y, axis, &m_axisY);
        emit axisYChanged(m_axisY);
    } else {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
    }
}

QAbstract3DAxis *QQuickGraphsItem::axisY() const
{
    return m_axisY;
}

void QQuickGraphsItem::setAxisZ(QAbstract3DAxis *axis)
{
    // Setting null axis will always create new default axis
    if (!axis || axis != m_axisZ) {
        setAxisHelper(QAbstract3DAxis::AxisOrientation::Z, axis, &m_axisZ);
        emit axisZChanged(m_axisZ);
    } else {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << axis;
    }
}

QAbstract3DAxis *QQuickGraphsItem::axisZ() const
{
    return m_axisZ;
}

void QQuickGraphsItem::addAxis(QAbstract3DAxis *axis)
{
    Q_ASSERT(axis);
    QQuickGraphsItem *owner = qobject_cast<QQuickGraphsItem *>(axis->parent());
    if (owner != this) {
        Q_ASSERT_X(!owner, "addAxis", "Axis already attached to a graph.");
        axis->setParent(this);
    }
    if (!m_axes.contains(axis))
        m_axes.append(axis);
}

void QQuickGraphsItem::releaseAxis(QAbstract3DAxis *axis)
{
    if (axis && m_axes.contains(axis)) {
        // Clear the default status from released default axes
        if (axis->d_func()->isDefaultAxis())
            axis->d_func()->setDefaultAxis(false);

        // If the axis is in use, replace it with a temporary one
        switch (axis->orientation()) {
        case QAbstract3DAxis::AxisOrientation::X:
            setAxisX(0);
            break;
        case QAbstract3DAxis::AxisOrientation::Y:
            setAxisY(0);
            break;
        case QAbstract3DAxis::AxisOrientation::Z:
            setAxisZ(0);
            break;
        default:
            break;
        }

        m_axes.removeAll(axis);
        axis->setParent(0);
    }
}

QList<QAbstract3DAxis *> QQuickGraphsItem::axes() const
{
    return m_axes;
}

void QQuickGraphsItem::setRenderingMode(QtGraphs3D::RenderingMode mode)
{
    if (mode == m_renderMode || mode < QtGraphs3D::RenderingMode::DirectToBackground
            || mode > QtGraphs3D::RenderingMode::Indirect) {
        qCWarning(lcProperties3D, "%s invalid rendering mode used",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    QtGraphs3D::RenderingMode previousMode = m_renderMode;

    m_renderMode = mode;

    m_initialisedSize = QSize(0, 0);
    setFlag(ItemHasContents /*, !m_runningInDesigner*/);

    // TODO - Need to check if the mode is set properly
    switch (mode) {
    case QtGraphs3D::RenderingMode::DirectToBackground:
        update();
        setRenderMode(QQuick3DViewport::Underlay);
        if (previousMode == QtGraphs3D::RenderingMode::Indirect) {
            checkWindowList(window());
            setAntialiasing(m_windowSamples > 0);
            if (m_windowSamples != m_samples)
                emit msaaSamplesChanged(m_windowSamples);
        }
        break;
    case QtGraphs3D::RenderingMode::Indirect:
        update();
        setRenderMode(QQuick3DViewport::Offscreen);
        break;
    }
    if (m_sliceView)
        m_sliceView->setRenderMode(renderMode());

    updateWindowParameters();

    emit renderingModeChanged(mode);
}

QtGraphs3D::RenderingMode QQuickGraphsItem::renderingMode() const
{
    return m_renderMode;
}

void QQuickGraphsItem::setTransparencyTechnique(QtGraphs3D::TransparencyTechnique technique)
{
    if (technique == m_transparencyTechnique) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << technique;
        return;
    }

    switch (technique) {
    case QtGraphs3D::TransparencyTechnique::Default:
        environment()->setOitMethod(QQuick3DSceneEnvironment::OITNone);
        break;
    case QtGraphs3D::TransparencyTechnique::Approximate:
        environment()->setOitMethod(QQuick3DSceneEnvironment::OITWeightedBlended);
        break;
    case QtGraphs3D::TransparencyTechnique::Accurate:
        environment()->setOitMethod(QQuick3DSceneEnvironment::OITLinkedList);
        break;
    }
    m_transparencyTechnique = technique;

    emit transparencyTechniqueChanged(technique);
}

QtGraphs3D::TransparencyTechnique QQuickGraphsItem::transparencyTechnique() const
{
    return m_transparencyTechnique;
}

void QQuickGraphsItem::keyPressEvent(QKeyEvent *ev)
{
    ev->ignore();
    setFlag(ItemHasContents);
    update();
}

void QQuickGraphsItem::checkSliceEnabled()
{
    if (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Slice)
        && (selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column)
            != selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))) {
        m_sliceEnabled = true;
    } else {
        m_sliceEnabled = false;
    }
}

QtGraphs3D::GridLineType QQuickGraphsItem::gridLineType() const
{
    return m_gridLineType;
}

void QQuickGraphsItem::setGridLineType(const QtGraphs3D::GridLineType &gridLineType)
{
    m_gridLineTypeDirty = true;
    if (m_gridLineType == gridLineType) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << gridLineType;
        return;
    }
    m_gridLineType = gridLineType;
    emit gridLineTypeChanged();
    emitNeedRender();
}

void QQuickGraphsItem::handleThemeTypeChange() {}

void QQuickGraphsItem::handleFpsChanged()
{
    int fps = renderStats()->fps();
    if (m_currentFps != fps) {
        m_currentFps = fps;
        emit currentFpsChanged(fps);
    }
}

void QQuickGraphsItem::handleParentWidthChange()
{
    m_cachedGeometry = parentItem()->boundingRect();
    updateWindowParameters();
    updateSubViews();
}

void QQuickGraphsItem::handleParentHeightChange()
{
    m_cachedGeometry = parentItem()->boundingRect();
    updateWindowParameters();
    updateSubViews();
}

void QQuickGraphsItem::componentComplete()
{
    QQuick3DViewport::componentComplete();
    Q_TRACE(QGraphs3DItemInit_entry);
    rootNode()->setScale(QVector3D(100,100,100));

    auto url = QUrl(QStringLiteral("defaultMeshes/backgroundMesh"));
    m_background = new QQuick3DModel();
    m_backgroundScale = new QQuick3DNode();
    m_backgroundRotation = new QQuick3DNode();
    m_graphNode = new QQuick3DNode();

    m_backgroundScale->setParent(graphNode());
    m_backgroundScale->setParentItem(graphNode());

    m_backgroundRotation->setParent(m_backgroundScale);
    m_backgroundRotation->setParentItem(m_backgroundScale);

    m_background->setObjectName("Background");
    m_background->setParent(m_backgroundRotation);
    m_background->setParentItem(m_backgroundRotation);

    m_background->setSource(url);

    m_backgroundBB = new QQuick3DModel();
    m_backgroundBB->setObjectName("BackgroundBB");
    m_backgroundBB->setParent(m_background);
    m_backgroundBB->setParentItem(m_background);
    m_backgroundBB->setSource(QUrl(QStringLiteral("defaultMeshes/barMeshFull")));
    m_backgroundBB->setPickable(true);

    m_graphNode->setParent(rootNode());
    m_graphNode->setParentItem(rootNode());

    setUpCamera();
    setUpLight();

    // Create repeaters for each axis X, Y, Z
    m_repeaterX = createRepeater();
    m_repeaterY = createRepeater();
    m_repeaterZ = createRepeater();

    m_delegateModelX.reset(new QQmlComponent(qmlEngine(this), (QStringLiteral(":/axis/AxisLabel"))));
    m_delegateModelY.reset(new QQmlComponent(qmlEngine(this), (QStringLiteral(":/axis/AxisLabel"))));
    m_delegateModelZ.reset(new QQmlComponent(qmlEngine(this), (QStringLiteral(":/axis/AxisLabel"))));

    m_repeaterX->setDelegate(m_delegateModelX.get());
    m_repeaterY->setDelegate(m_delegateModelY.get());
    m_repeaterZ->setDelegate(m_delegateModelZ.get());

    // title labels for axes
    m_titleLabelX = createTitleLabel();
    m_titleLabelX->setVisible(axisX()->isTitleVisible());
    m_titleLabelX->setProperty("labelText", axisX()->title());

    m_titleLabelY = createTitleLabel();
    m_titleLabelY->setVisible(axisY()->isTitleVisible());
    m_titleLabelY->setProperty("labelText", axisY()->title());

    m_titleLabelZ = createTitleLabel();
    m_titleLabelZ->setVisible(axisZ()->isTitleVisible());
    m_titleLabelZ->setProperty("labelText", axisZ()->title());

    // Grid with geometry
    m_gridGeometryModel = new QQuick3DModel(m_graphNode);
    m_gridGeometryModel->setCastsShadows(false);
    m_gridGeometryModel->setReceivesShadows(false);
    auto gridGeometry = new QQuick3DGeometry(m_gridGeometryModel);
    gridGeometry->setStride(sizeof(QVector3D));
    gridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                               0,
                               QQuick3DGeometry::Attribute::F32Type);
    m_gridGeometryModel->setGeometry(gridGeometry);
    QQmlListReference gridMaterialRef(m_gridGeometryModel, "materials");
    auto gridMaterial = new QQuick3DPrincipledMaterial(m_gridGeometryModel);
    gridMaterial->setLighting(QQuick3DPrincipledMaterial::Lighting::NoLighting);
    gridMaterial->setCullMode(QQuick3DMaterial::CullMode::BackFaceCulling);
    gridMaterial->setBaseColor(theme()->grid().mainColor());
    gridMaterialRef.append(gridMaterial);

    // subgrid with geometry
    m_subgridGeometryModel = new QQuick3DModel(m_graphNode);
    m_subgridGeometryModel->setCastsShadows(false);
    m_subgridGeometryModel->setReceivesShadows(false);
    auto subgridGeometry = new QQuick3DGeometry(m_subgridGeometryModel);
    subgridGeometry->setStride(sizeof(QVector3D));
    subgridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    subgridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                               0,
                               QQuick3DGeometry::Attribute::F32Type);
    m_subgridGeometryModel->setGeometry(subgridGeometry);

    QQmlListReference subgridMaterialRef(m_subgridGeometryModel, "materials");
    auto subgridMaterial = new QQuick3DPrincipledMaterial(m_subgridGeometryModel);
    subgridMaterial->setLighting(QQuick3DPrincipledMaterial::Lighting::NoLighting);
    subgridMaterial->setCullMode(QQuick3DMaterial::CullMode::BackFaceCulling);
    subgridMaterialRef.append(subgridMaterial);

    createItemLabel();

    auto axis = axisX();
    m_repeaterX->setModel(axis->labels().size());
    handleAxisLabelsChangedBySender(axisX());

    axis = axisY();
    m_repeaterY->setModel(2 * axis->labels().size());
    handleAxisLabelsChangedBySender(axisY());

    axis = axisZ();
    m_repeaterZ->setModel(axis->labels().size());
    handleAxisLabelsChangedBySender(axisZ());

    if (!m_pendingCustomItemList.isEmpty()) {
        for (const auto &item : std::as_const(m_pendingCustomItemList))
            addCustomItem(item);
    }
    qCDebug(lcGraphs3D, "QQuickGraphsItem::componentComplete.");
    Q_TRACE(QGraphs3DItemInit_exit);
}

QQuick3DDirectionalLight *QQuickGraphsItem::light() const
{
    return m_light;
}

bool QQuickGraphsItem::isSlicingActive() const
{
    return m_scene->isSlicingActive();
}

void QQuickGraphsItem::setSlicingActive(bool isSlicing)
{
    m_scene->setSlicingActive(isSlicing);
}

bool QQuickGraphsItem::isCustomLabelItem(QCustom3DItem *item) const
{
    return item->d_func()->m_isLabelItem;
}

bool QQuickGraphsItem::isCustomVolumeItem(QCustom3DItem *item) const
{
    return item->d_func()->m_isVolumeItem;
}

QImage QQuickGraphsItem::customTextureImage(QCustom3DItem *item)
{
    return item->d_func()->textureImage();
}

Q3DScene *QQuickGraphsItem::scene()
{
    return m_scene;
}

void QQuickGraphsItem::addTheme(QGraphsTheme *theme)
{
    Q_ASSERT(theme);
    QQuickGraphsItem *owner = qobject_cast<QQuickGraphsItem *>(theme->parent());
    if (owner != this) {
        Q_ASSERT_X(!owner, "addTheme", "Theme already attached to a graph.");
        theme->setParent(this);
    }
    if (!m_themes.contains(theme))
        m_themes.append(theme);
}

void QQuickGraphsItem::releaseTheme(QGraphsTheme *theme)
{
    QGraphsTheme *oldTheme = m_activeTheme;

    if (theme && m_themes.contains(theme)) {
        // If the theme is in use, replace it with a temporary one
        if (theme == m_activeTheme) {
            m_activeTheme = nullptr;
            disconnect(theme, &QGraphsTheme::themeChanged, this, &QQuickGraphsItem::handleThemeTypeChanged);
            disconnect(theme, &QGraphsTheme::colorStyleChanged, this, &QQuickGraphsItem::handleThemeColorStyleChanged);
            disconnect(theme, &QGraphsTheme::seriesColorsChanged, this, &QQuickGraphsItem::handleThemeBaseColorsChanged);
            disconnect(theme, &QGraphsTheme::seriesGradientsChanged, this, &QQuickGraphsItem::handleThemeBaseGradientsChanged);
            disconnect(theme, &QGraphsTheme::singleHighlightColorChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightColorChanged);
            disconnect(theme, &QGraphsTheme::singleHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightGradientChanged);
            disconnect(theme, &QGraphsTheme::multiHighlightColorChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightColorChanged);
            disconnect(theme, &QGraphsTheme::multiHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightGradientChanged);
            disconnect(theme, &QGraphsTheme::update, this, &QQuickGraphsItem::emitNeedRender);
        }
        m_themes.removeAll(theme);
        theme->setParent(nullptr);
    }

    if (oldTheme != m_activeTheme)
        emit activeThemeChanged(m_activeTheme);
}

QList<QGraphsTheme *> QQuickGraphsItem::themes() const
{
    return m_themes;
}

void QQuickGraphsItem::setTheme(QGraphsTheme *theme)
{
    if (theme != m_activeTheme) {
        if (m_activeTheme) {
            disconnect(m_activeTheme, &QGraphsTheme::themeChanged, this, &QQuickGraphsItem::handleThemeTypeChanged);
            disconnect(m_activeTheme, &QGraphsTheme::colorStyleChanged, this, &QQuickGraphsItem::handleThemeColorStyleChanged);
            disconnect(m_activeTheme, &QGraphsTheme::seriesColorsChanged, this, &QQuickGraphsItem::handleThemeBaseColorsChanged);
            disconnect(m_activeTheme, &QGraphsTheme::seriesGradientsChanged, this, &QQuickGraphsItem::handleThemeBaseGradientsChanged);
            disconnect(m_activeTheme, &QGraphsTheme::singleHighlightColorChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightColorChanged);
            disconnect(m_activeTheme, &QGraphsTheme::singleHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightGradientChanged);
            disconnect(m_activeTheme, &QGraphsTheme::multiHighlightColorChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightColorChanged);
            disconnect(m_activeTheme, &QGraphsTheme::multiHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightGradientChanged);
            disconnect(m_activeTheme, &QGraphsTheme::update, this, &QQuickGraphsItem::emitNeedRender);
        }

        connect(theme, &QGraphsTheme::themeChanged, this, &QQuickGraphsItem::handleThemeTypeChanged);
        connect(theme, &QGraphsTheme::colorStyleChanged, this, &QQuickGraphsItem::handleThemeColorStyleChanged);
        connect(theme, &QGraphsTheme::seriesColorsChanged, this, &QQuickGraphsItem::handleThemeBaseColorsChanged);
        connect(theme, &QGraphsTheme::seriesGradientsChanged, this, &QQuickGraphsItem::handleThemeBaseGradientsChanged);
        connect(theme, &QGraphsTheme::singleHighlightColorChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightColorChanged);
        connect(theme, &QGraphsTheme::singleHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeSingleHighlightGradientChanged);
        connect(theme, &QGraphsTheme::multiHighlightColorChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightColorChanged);
        connect(theme, &QGraphsTheme::multiHighlightGradientChanged, this, &QQuickGraphsItem::handleThemeMultiHighlightGradientChanged);
        connect(theme, &QGraphsTheme::update, this, &QQuickGraphsItem::emitNeedRender);

        m_activeTheme = theme;

        connect(theme, &QGraphsTheme::colorSchemeChanged, this, [this](){
            m_changeTracker.themeBackgroundColorChanged = true;
            m_changeTracker.themePlotAreaBackgroundColorChanged = true;
            m_changeTracker.themeLabelBackgroundColorChanged = true;
            m_changeTracker.themeLabelTextColorChanged = true;
        });
        connect(theme, &QGraphsTheme::axisXChanged, this, [this](){
            m_changeTracker.themeAxisXChanged = true;
        });
        connect(theme, &QGraphsTheme::axisYChanged, this, [this](){
            m_changeTracker.themeAxisYChanged = true;
        });
        connect(theme, &QGraphsTheme::axisZChanged, this, [this](){
            m_changeTracker.themeAxisZChanged = true;
        });
        connect(theme, &QGraphsTheme::backgroundColorChanged, this, [this](){
            m_changeTracker.themeBackgroundColorChanged = true;
        });
        connect(theme, &QGraphsTheme::backgroundVisibleChanged, this, [this](){
            m_changeTracker.themeBackgroundVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::plotAreaBackgroundColorChanged, this, [this](){
            m_changeTracker.themePlotAreaBackgroundColorChanged = true;
        });
        connect(theme, &QGraphsTheme::plotAreaBackgroundVisibleChanged, this, [this](){
            m_changeTracker.themePlotAreaBackgroundVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::gridVisibleChanged, this, [this](){
            m_changeTracker.themeGridVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::labelsVisibleChanged, this, [this](){
            m_changeTracker.themeLabelsVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::labelBackgroundColorChanged, this, [this](){
            m_changeTracker.themeLabelBackgroundColorChanged = true;
        });
        connect(theme, &QGraphsTheme::labelTextColorChanged, this, [this](){
            m_changeTracker.themeLabelTextColorChanged = true;
        });
        connect(theme, &QGraphsTheme::labelFontChanged, this, [this](){
            m_changeTracker.themeLabelFontChanged = true;
        });
        connect(theme, &QGraphsTheme::labelBackgroundVisibleChanged, this, [this](){
            m_changeTracker.themeLabelBackgroundVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::labelBorderVisibleChanged, this, [this](){
            m_changeTracker.themeLabelBorderVisibilityChanged = true;
        });
        connect(theme, &QGraphsTheme::gridChanged, this, [this](){
            m_changeTracker.themeGridChanged = true;
        });

        m_changeTracker.themeChanged = true;
        // Default theme can be created by theme manager, so ensure we have correct theme
        QGraphsTheme *newActiveTheme = m_activeTheme;
        // Reset all attached series to the new theme
        for (int i = 0; i < m_seriesList.size(); i++)
            m_seriesList.at(i)->d_func()->resetToTheme(*newActiveTheme, i, isComponentComplete());
        markSeriesVisualsDirty();
        emit activeThemeChanged(newActiveTheme);
    } else {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "theme is already set to:" << theme;
    }
}

QGraphsTheme *QQuickGraphsItem::theme() const
{
    return m_activeTheme;
}

bool QQuickGraphsItem::hasSeries(QAbstract3DSeries *series)
{
    return m_seriesList.contains(series);
}

void QQuickGraphsItem::setSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    if (mode == m_selectionMode) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << mode.toInt();
        return;
    }
    m_selectionMode = mode;
    m_changeTracker.selectionModeChanged = true;
    emit selectionModeChanged(mode);
    emitNeedRender();
}

QtGraphs3D::SelectionFlags QQuickGraphsItem::selectionMode() const
{
    return m_selectionMode;
}

void QQuickGraphsItem::doSetShadowQuality(QtGraphs3D::ShadowQuality quality)
{
    if (quality == m_shadowQuality) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << quality;
        return;
    }
    m_shadowQuality = quality;
    m_changeTracker.shadowQualityChanged = true;
    emit shadowQualityChanged(m_shadowQuality);
    emitNeedRender();
}

void QQuickGraphsItem::setShadowQuality(QtGraphs3D::ShadowQuality quality)
{
    if (!m_useOrthoProjection)
        doSetShadowQuality(quality);
}

QtGraphs3D::ShadowQuality QQuickGraphsItem::shadowQuality() const
{
    return m_shadowQuality;
}

qsizetype QQuickGraphsItem::addCustomItem(QCustom3DItem *item)
{
    if (!item) {
        qCWarning(lcProperties3D, "%s invalid item", qUtf8Printable(QLatin1String(__FUNCTION__)));
        return -1;
    }

    if (isComponentComplete()) {
        if (isCustomLabelItem(item)) {
            QQuick3DNode *label = createTitleLabel();
            QCustom3DLabel *key = static_cast<QCustom3DLabel *>(item);
            m_customLabelList.insert(key, label);
        } else if (isCustomVolumeItem(item)) {
            QQuick3DModel *model = new QQuick3DModel();
            model->setParent(graphNode());
            model->setParentItem(graphNode());
            m_customItemList.insert(item, model);
        } else {
            QQuick3DModel *model = new QQuick3DModel();
            model->setParent(graphNode());
            model->setParentItem(graphNode());
            QQmlListReference materialsRef(model, "materials");
            QQuick3DPrincipledMaterial *material = new QQuick3DPrincipledMaterial();
            material->setParent(model);
            material->setParentItem(model);
            materialsRef.append(material);
            if (!selectionMode().testFlag(QtGraphs3D::SelectionFlag::None))
                model->setPickable(true);
            m_customItemList.insert(item, model);
        }
    } else {
        m_pendingCustomItemList.append(item);
    }

    qsizetype index = m_customItems.indexOf(item);

    if (index != -1) {
        qCWarning(lcProperties3D, "%s tried to access customItems list at invalid index.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return index;
    }

    item->setParent(this);
    connect(item, &QCustom3DItem::needUpdate, this, &QQuickGraphsItem::updateCustomItem);
    m_customItems.append(item);
    item->d_func()->resetDirtyBits();
    m_isCustomDataDirty = true;
    emitNeedRender();
    return m_customItems.size() - 1;
}

void QQuickGraphsItem::deleteCustomItems()
{
    for (QCustom3DItem *item : std::as_const(m_customItems)) {
        if (m_customItemList.contains(item))
            m_customItemList[item]->deleteLater();
        delete item;
    }
    m_customItems.clear();
    m_isCustomDataDirty = true;
    emitNeedRender();
}

void QQuickGraphsItem::deleteCustomItem(QCustom3DItem *item)
{
    if (!item)
        return;

    m_customItems.removeOne(item);
    delete item;
    item = 0;
    m_isCustomDataDirty = true;
    emitNeedRender();
}

void QQuickGraphsItem::deleteCustomItem(QVector3D position)
{
    // Get the item for the position
    for (QCustom3DItem *item : std::as_const(m_customItems)) {
        if (item->position() == position)
            deleteCustomItem(item);
    }
}

QList<QCustom3DItem *> QQuickGraphsItem::customItems() const
{
    return m_customItems;
}

void QQuickGraphsItem::updateCustomItem()
{
    m_isCustomItemDirty = true;
    m_isCustomDataDirty = true;
    emitNeedRender();
}

void QQuickGraphsItem::removeCustomItems()
{
    deleteCustomItems();
    m_customItemList.clear();
    m_customLabelList.clear();
}

void QQuickGraphsItem::removeCustomItem(QCustom3DItem *item)
{
    if (!item) {
        qCWarning(lcProperties3D, "%s invalid item", qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    if (isCustomLabelItem(item)) {
        m_customLabelList.remove(static_cast<QCustom3DLabel *>(item));
    } else if (isCustomVolumeItem(item)) {
        m_customItemList.remove(item);
        auto volume = static_cast<QCustom3DVolume *>(item);
        if (m_customVolumes.contains(volume)) {
            m_customVolumes[volume].model->deleteLater();
            m_customVolumes.remove(volume);
        }
    } else {
        if (m_customItemList.contains(item)) {
            m_customItemList[item]->deleteLater();
            m_customItemList.remove(item);
        }
    }
    deleteCustomItem(item);
}

void QQuickGraphsItem::removeCustomItemAt(QVector3D position)
{
    auto labelIterator = m_customLabelList.begin();
    while (labelIterator != m_customLabelList.end()) {
        QCustom3DLabel *label = labelIterator.key();
        if (label->position() == position) {
            labelIterator.value()->setVisible(false);
            labelIterator = m_customLabelList.erase(labelIterator);
        } else {
            ++labelIterator;
        }
    }

    auto itemIterator = m_customItemList.begin();
    while (itemIterator != m_customItemList.end()) {
        QCustom3DItem *item = itemIterator.key();
        if (item->position() == position) {
            m_customItemList[item]->deleteLater();
            itemIterator = m_customItemList.erase(itemIterator);
            if (isCustomVolumeItem(item)) {
                auto volume = static_cast<QCustom3DVolume *>(item);
                if (m_customVolumes.contains(volume)) {
                    m_customVolumes[volume].model->deleteLater();
                    m_customVolumes.remove(volume);
                }
            }
        } else {
            ++itemIterator;
        }
    }
    deleteCustomItem(position);
}

void QQuickGraphsItem::releaseCustomItem(QCustom3DItem *item)
{
    if (isCustomLabelItem(item)) {
        m_customLabelList.remove(static_cast<QCustom3DLabel *>(item));
    } else if (isCustomVolumeItem(item)) {
        m_customItemList.remove(item);
        auto volume = static_cast<QCustom3DVolume *>(item);
        if (m_customVolumes.contains(volume)) {
            m_customVolumes[volume].model->deleteLater();
            m_customVolumes.remove(volume);
        }
    } else {
        m_customItemList.remove(item);
    }

    if (item && m_customItems.contains(item)) {
        disconnect(item, &QCustom3DItem::needUpdate, this, &QQuickGraphsItem::updateCustomItem);
        m_customItems.removeOne(item);
        item->setParent(0);
        m_isCustomDataDirty = true;
        emitNeedRender();
    }
}

int QQuickGraphsItem::selectedLabelIndex() const
{
    int index = m_selectedLabelIndex;
    QAbstract3DAxis *axis = selectedAxis();
    if (axis && axis->labels().size() <= index)
        index = -1;
    return index;
}

QAbstract3DAxis *QQuickGraphsItem::selectedAxis() const
{
    QAbstract3DAxis *axis = 0;
    QtGraphs3D::ElementType type = m_clickedType;
    switch (type) {
    case QtGraphs3D::ElementType::AxisXLabel:
        axis = axisX();
        break;
    case QtGraphs3D::ElementType::AxisYLabel:
        axis = axisY();
        break;
    case QtGraphs3D::ElementType::AxisZLabel:
        axis = axisZ();
        break;
    default:
        axis = 0;
        break;
    }

    return axis;
}

qsizetype QQuickGraphsItem::selectedCustomItemIndex() const
{
    qsizetype index = m_selectedCustomItemIndex;
    if (m_customItems.size() <= index)
        index = -1;
    return index;
}

QCustom3DItem *QQuickGraphsItem::selectedCustomItem() const
{
    QCustom3DItem *item = 0;
    qsizetype index = selectedCustomItemIndex();
    if (index >= 0)
        item = m_customItems[index];
    return item;
}

QQmlListProperty<QCustom3DItem> QQuickGraphsItem::customItemList()
{
    return QQmlListProperty<QCustom3DItem>(this,
                                           this,
                                           &QQuickGraphsItem::appendCustomItemFunc,
                                           &QQuickGraphsItem::countCustomItemFunc,
                                           &QQuickGraphsItem::atCustomItemFunc,
                                           &QQuickGraphsItem::clearCustomItemFunc);
}

void QQuickGraphsItem::appendCustomItemFunc(QQmlListProperty<QCustom3DItem> *list,
                                            QCustom3DItem *item)
{
    QQuickGraphsItem *decl = reinterpret_cast<QQuickGraphsItem *>(list->data);
    decl->addCustomItem(item);
}

qsizetype QQuickGraphsItem::countCustomItemFunc(QQmlListProperty<QCustom3DItem> *list)
{
    Q_UNUSED(list);
    return reinterpret_cast<QQuickGraphsItem *>(list->data)->m_customItems.size();
}

QCustom3DItem *QQuickGraphsItem::atCustomItemFunc(QQmlListProperty<QCustom3DItem> *list,
                                                  qsizetype index)
{
    Q_UNUSED(list);
    Q_UNUSED(index);
    return reinterpret_cast<QQuickGraphsItem *>(list->data)->m_customItems.at(index);
}

void QQuickGraphsItem::clearCustomItemFunc(QQmlListProperty<QCustom3DItem> *list)
{
    QQuickGraphsItem *decl = reinterpret_cast<QQuickGraphsItem *>(list->data);
    decl->removeCustomItems();
}

void QQuickGraphsItem::synchData()
{
    qCDebug(lcGraphs3D, "%s start sync", qUtf8Printable(QLatin1String(__FUNCTION__)));
    if (!isVisible())
        return;

    Q_TRACE(QGraphs3DItemSynch_entry);

    m_renderPending = false;

    if (m_changeTracker.selectionModeChanged) {
        updateSelectionMode(selectionMode());
        m_changeTracker.selectionModeChanged = false;
    }

    bool recalculateScale = false;
    if (m_changeTracker.aspectRatioChanged) {
        recalculateScale = true;
        m_changeTracker.aspectRatioChanged = false;
    }

    if (m_changeTracker.horizontalAspectRatioChanged) {
        recalculateScale = true;
        m_changeTracker.horizontalAspectRatioChanged = false;
    }

    if (m_changeTracker.marginChanged) {
        recalculateScale = true;
        m_changeTracker.marginChanged = false;
    }

    if (m_changeTracker.polarChanged) {
        recalculateScale = true;
        m_changeTracker.polarChanged = false;
    }

    if (recalculateScale)
        calculateSceneScalingFactors();

    bool axisDirty = recalculateScale;
    if (m_changeTracker.axisXFormatterChanged) {
        m_changeTracker.axisXFormatterChanged = false;
        if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(axisX());
            valueAxisX->recalculate();
            repeaterX()->setModel(valueAxisX->formatter()->labelPositions().size());
        }
        axisDirty = true;
    }

    if (m_changeTracker.axisYFormatterChanged) {
        m_changeTracker.axisYFormatterChanged = false;
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(axisY());
            valueAxisY->recalculate();
            repeaterY()->setModel(2 * valueAxisY->formatter()->labelPositions().size());
        }
        axisDirty = true;
    }

    if (m_changeTracker.axisZFormatterChanged) {
        m_changeTracker.axisZFormatterChanged = false;
        if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(axisZ());
            valueAxisZ->recalculate();
            repeaterZ()->setModel(valueAxisZ->formatter()->labelPositions().size());
        }
        axisDirty = true;
    }

    if (m_changeTracker.axisXSegmentCountChanged) {
        if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(axisX());
            valueAxisX->recalculate();
        }
        m_changeTracker.axisXSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisYSegmentCountChanged) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(axisY());
            valueAxisY->recalculate();
        }
        m_changeTracker.axisYSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisZSegmentCountChanged) {
        if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(axisZ());
            valueAxisZ->recalculate();
        }
        m_changeTracker.axisZSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisXSubSegmentCountChanged) {
        if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(axisX());
            valueAxisX->recalculate();
        }
        m_changeTracker.axisXSubSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisYSubSegmentCountChanged) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(axisY());
            valueAxisY->recalculate();
        }
        m_changeTracker.axisYSubSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisZSubSegmentCountChanged) {
        if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(axisZ());
            valueAxisZ->recalculate();
        }
        m_changeTracker.axisZSubSegmentCountChanged = false;
        axisDirty = true;
    }

    if (m_changeTracker.axisXLabelsChanged) {
        if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
            auto valueAxisX = static_cast<QValue3DAxis *>(axisX());
            valueAxisX->recalculate();
            repeaterX()->setModel(valueAxisX->formatter()->labelPositions().size());
        } else if (axisX()->type() == QAbstract3DAxis::AxisType::Category) {
            auto categoryAxis = static_cast<QCategory3DAxis *>(axisX());
            repeaterX()->setModel(categoryAxis->labels().size());
        }

        m_changeTracker.axisXLabelsChanged = false;
        handleLabelCountChanged(m_repeaterX, theme()->axisX().labelTextColor());
        axisDirty = true;
    }

    if (m_changeTracker.axisYLabelsChanged) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            auto valueAxisY = static_cast<QValue3DAxis *>(axisY());
            valueAxisY->recalculate();
            repeaterY()->setModel(2 * valueAxisY->formatter()->labelPositions().size());
        } else if (axisY()->type() == QAbstract3DAxis::AxisType::Category) {
            auto categoryAxis = static_cast<QCategory3DAxis *>(axisY());
            repeaterY()->setModel(2 * categoryAxis->labels().size());
        }

        m_changeTracker.axisYLabelsChanged = false;
        handleLabelCountChanged(m_repeaterY, theme()->axisY().labelTextColor());
        axisDirty = true;
    }

    if (m_changeTracker.axisZLabelsChanged) {
        if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
            auto valueAxisZ = static_cast<QValue3DAxis *>(axisZ());
            valueAxisZ->recalculate();
            repeaterZ()->setModel(valueAxisZ->formatter()->labelPositions().size());
        } else if (axisZ()->type() == QAbstract3DAxis::AxisType::Category) {
            auto categoryAxis = static_cast<QCategory3DAxis *>(axisZ());
            repeaterZ()->setModel(categoryAxis->labels().size());
        }

        m_changeTracker.axisZLabelsChanged = false;
        handleLabelCountChanged(m_repeaterZ, theme()->axisZ().labelTextColor());
        axisDirty = true;
    }

    if (m_changeTracker.themeLabelsVisibilityChanged) {
        repeaterX()->setVisible(theme()->labelsVisible());
        repeaterY()->setVisible(theme()->labelsVisible());
        repeaterZ()->setVisible(theme()->labelsVisible());
    }

    if (m_changeTracker.axisXLabelVisibilityChanged) {
        repeaterX()->setVisible(axisX()->labelsVisible());
        m_changeTracker.axisXLabelVisibilityChanged = false;
    }

    if (m_changeTracker.axisYLabelVisibilityChanged) {
        repeaterY()->setVisible(axisY()->labelsVisible());
        m_changeTracker.axisYLabelVisibilityChanged = false;
    }

    if (m_changeTracker.axisZLabelVisibilityChanged) {
        repeaterZ()->setVisible(axisZ()->labelsVisible());
        m_changeTracker.axisZLabelVisibilityChanged = false;
    }
    updateTitleLabels();

    if (m_changeTracker.shadowQualityChanged) {
        updateShadowQuality(shadowQuality());
        m_changeTracker.shadowQualityChanged = false;
    }

    if (m_changeTracker.axisXRangeChanged) {
        axisDirty = true;
        calculateSceneScalingFactors();
        m_changeTracker.axisXRangeChanged = false;
    }

    if (m_changeTracker.axisYRangeChanged) {
        axisDirty = true;
        QAbstract3DAxis *axis = axisY();
        updateAxisRange(axis->min(), axis->max());
        calculateSceneScalingFactors();
        m_changeTracker.axisYRangeChanged = false;
    }

    if (m_changeTracker.axisZRangeChanged) {
        axisDirty = true;
        calculateSceneScalingFactors();
        m_changeTracker.axisZRangeChanged = false;
    }

    if (m_changeTracker.axisXReversedChanged) {
        m_changeTracker.axisXReversedChanged = false;
        if (m_axisX->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisX = static_cast<QValue3DAxis *>(m_axisX);
            updateAxisReversed(valueAxisX->reversed());
            m_labelsNeedupdate = true;
        }
    }

    if (m_changeTracker.axisYReversedChanged) {
        m_changeTracker.axisYReversedChanged = false;
        if (m_axisY->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisY = static_cast<QValue3DAxis *>(m_axisY);
            updateAxisReversed(valueAxisY->reversed());
            m_labelsNeedupdate = true;
        }
    }

    if (m_changeTracker.axisZReversedChanged) {
        m_changeTracker.axisZReversedChanged = false;
        if (m_axisZ->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxisZ = static_cast<QValue3DAxis *>(m_axisZ);
            updateAxisReversed(valueAxisZ->reversed());
            m_labelsNeedupdate = true;
        }
    }

    if (m_changeTracker.axisXLabelAutoRotationChanged) {
        axisDirty = true;
        m_changeTracker.axisXLabelAutoRotationChanged = false;
    }

    if (m_changeTracker.axisYLabelAutoRotationChanged) {
        axisDirty = true;
        m_changeTracker.axisYLabelAutoRotationChanged = false;
    }

    if (m_changeTracker.axisZLabelAutoRotationChanged) {
        axisDirty = true;
        m_changeTracker.axisZLabelAutoRotationChanged = false;
    }
    if (m_changeTracker.axisXScaleLabelsByCountChanged) {
        axisDirty = true;
        m_changeTracker.axisXScaleLabelsByCountChanged = false;
    }

    if (m_changeTracker.axisYScaleLabelsByCountChanged) {
        axisDirty = true;
        m_changeTracker.axisYScaleLabelsByCountChanged = false;
    }

    if (m_changeTracker.axisZScaleLabelsByCountChanged) {
        axisDirty = true;
        m_changeTracker.axisZScaleLabelsByCountChanged = false;
    }

    if (m_changeTracker.axisXLabelSizeChanged) {
        axisDirty = true;
        m_changeTracker.axisXLabelSizeChanged = false;
    }

    if (m_changeTracker.axisYLabelSizeChanged) {
        axisDirty = true;
        m_changeTracker.axisYLabelSizeChanged = false;
    }

    if (m_changeTracker.axisZLabelSizeChanged) {
        axisDirty = true;
        m_changeTracker.axisZLabelSizeChanged = false;
    }

    if (m_changeTracker.axisXTitleFixedChanged) {
        axisDirty = true;
        m_changeTracker.axisXTitleFixedChanged = false;
    }

    if (m_changeTracker.axisYTitleFixedChanged) {
        axisDirty = true;
        m_changeTracker.axisYTitleFixedChanged = false;
    }

    if (m_changeTracker.axisZTitleFixedChanged) {
        axisDirty = true;
        m_changeTracker.axisZTitleFixedChanged = false;
    }

    if (m_changeTracker.axisXTitleOffsetChanged) {
        axisDirty = true;
        m_changeTracker.axisXTitleOffsetChanged = false;
    }
    if (m_changeTracker.axisYTitleOffsetChanged) {
        axisDirty = true;
        m_changeTracker.axisYTitleOffsetChanged = false;
    }
    if (m_changeTracker.axisZTitleOffsetChanged) {
        axisDirty = true;
        m_changeTracker.axisZTitleOffsetChanged = false;
    }
    if (m_changeTracker.multiAxisChanged) {
        updateMultiAxis();
        m_changeTracker.multiAxisChanged = false;
    }

    if (m_changeTracker.cameraChanged) {
        updateCamera();
        m_changeTracker.cameraChanged = false;
    }

    QVector3D forward = camera()->forward();
    auto targetRotation = cameraTarget()->eulerRotation();
    if (m_yFlipped != (targetRotation.x() > 0)) {
        m_yFlipped = (targetRotation.x() > 0);
        axisDirty = true;
    }
    if (m_xFlipped != (forward.x() > 0)) {
        m_xFlipped = (forward.x() > 0);
        axisDirty = true;
    }
    if (m_zFlipped != ((forward.z() > .1f))) {
        m_zFlipped = ((forward.z() > .1f));
        axisDirty = true;
    }

    if (axisDirty) {
        QQmlListReference materialsRef(m_background, "materials");
        if (!materialsRef.size()) {
            QQuick3DCustomMaterial *bgMat
                    = createQmlCustomMaterial(QStringLiteral(":/materials/BackgroundMaterial"));
            bgMat->setParent(m_background);
            materialsRef.append(bgMat);
        }
        if (m_gridLineType == QtGraphs3D::GridLineType::Shader)
            updateGridLineType();
        else
            updateGrid();
        m_labelsNeedupdate = true;
        updateCustomData();
        if (m_sliceView && isSliceEnabled()) {
            updateSliceGrid();
            updateSliceLabels();
        }
        m_gridUpdated = true;
    }

    if (m_changeTracker.radialLabelOffsetChanged) {
        updateRadialLabelOffset();
        m_changeTracker.radialLabelOffsetChanged = false;
    }
    if (m_changeTracker.labelMarginChanged) {
        m_labelsNeedupdate = true;
        m_changeTracker.labelMarginChanged = false;
    }

    QMatrix4x4 modelMatrix;
    m_backgroundScale->setScale(m_scaleWithBackground + m_backgroundScaleMargin);

    QVector3D rotVec;
    if (!m_yFlipped) {
        rotVec = QVector3D(0, 270, 0);
        if (m_xFlipped && m_zFlipped)
            rotVec.setY(90);
        else if (!m_xFlipped && m_zFlipped)
            rotVec.setY(0);
        else if (m_xFlipped && !m_zFlipped)
            rotVec.setY(180);
    } else {
        rotVec = QVector3D(0, 180, 180);
        if (m_xFlipped && m_zFlipped)
            rotVec.setY(0);
        else if (!m_xFlipped && m_zFlipped)
            rotVec.setY(270);
        else if (m_xFlipped && !m_zFlipped)
            rotVec.setY(90);
    }

    auto rotation = Utils::calculateRotation(rotVec);

    if (rotation != m_backgroundRotation->rotation()) {
        if (m_yFlipped) {
            m_backgroundRotation->setRotation(rotation);
            if (m_axisX->labelAutoAngle() > 0.0f ||
                    m_axisY->labelAutoAngle() > 0.0f ||
                    m_axisZ->labelAutoAngle() > 0.0f) {
                m_labelsNeedupdate = true;
            }
        } else {
            modelMatrix.rotate(rotation);
            m_backgroundRotation->setRotation(rotation);
            if (m_axisX->labelAutoAngle() > 0.0f ||
                    m_axisY->labelAutoAngle() > 0.0f ||
                    m_axisZ->labelAutoAngle() > 0.0f) {
                m_labelsNeedupdate = true;
            }
        }
    }

    bool forceUpdateCustomVolumes = false;
    if (m_changeTracker.projectionChanged) {
        forceUpdateCustomVolumes = true;
        bool useOrtho = isOrthoProjection();
        if (useOrtho)
            setCamera(m_oCamera);
        else
            setCamera(m_pCamera);
        m_changeTracker.projectionChanged = false;
    }

    if (m_changeTracker.themeChanged) {
        theme()->resetDirtyBits();
        m_changeTracker.themeChanged = false;
    }

    if (m_lightStrengthDirty) {
        light()->setBrightness(lightStrength() * .2f);
        if (qFuzzyIsNull(light()->brightness()))
            light()->setBrightness(.0000001f);
        updateLightStrength();
        m_lightStrengthDirty = false;
    }

    if (m_ambientLightStrengthDirty) {
        float ambientStrength = m_ambientLightStrength;
        QColor ambientColor = QColor::fromRgbF(ambientStrength, ambientStrength, ambientStrength);
        light()->setAmbientColor(ambientColor);
        if (qFuzzyIsNull(light()->brightness()))
            light()->setBrightness(.0000001f);
        m_ambientLightStrengthDirty = false;
    }

    if (m_lightColorDirty) {
        light()->setColor(lightColor());
        m_lightColorDirty = false;
    }

    if (m_shadowStrengthDirty) {
        light()->setShadowFactor(shadowStrength());
        m_shadowStrengthDirty = false;
    }

    if (theme()->dirtyBits()->gridDirty || m_changeTracker.themeGridChanged) {
        QQmlListReference materialRef(m_background, "materials");
        Q_ASSERT(materialRef.size());
        float mainWidth = theme()->grid().mainWidth();
        if ((m_gridLineType == QtGraphs3D::GridLineType::Shader) && mainWidth > 10.0f) {
            qCWarning(lcProperties3D, "%s invalid value for shader grid. Valid range for grid width is between"
                     " 0.0 and 1.0. Value exceeds 10.0. Set it to 10.0", qUtf8Printable(QLatin1String(__FUNCTION__)));
            mainWidth = 4.0f;
        }

        if ((m_gridLineType == QtGraphs3D::GridLineType::Shader) && mainWidth < 0.0f) {
            qCWarning(lcProperties3D, "%s invalid value for shader grid. Valid range for grid width is between"
                     " 0.0 and 10.0. Value is smaller than 0.0. Set it to 0.0", qUtf8Printable(QLatin1String(__FUNCTION__)));
            mainWidth = 0.0f;
        }
        auto *material = static_cast<QQuick3DCustomMaterial *>(materialRef.at(0));
        material->setProperty("gridWidth", mainWidth);

        QColor gridMainColor = theme()->grid().mainColor();
        QQmlListReference backgroundRef(m_background, "materials");
        auto *backgroundMaterial = static_cast<QQuick3DCustomMaterial *>(backgroundRef.at(0));
        backgroundMaterial->setProperty("gridLineColor", gridMainColor);
        QQmlListReference mainGridRef(m_gridGeometryModel, "materials");
        auto *gridMaterial = static_cast<QQuick3DPrincipledMaterial *>(mainGridRef.at(0));
        gridMaterial->setBaseColor(gridMainColor);

        QColor gridSubColor = theme()->grid().subColor();
        backgroundMaterial->setProperty("subgridLineColor", gridSubColor);

        QQmlListReference subGridRef(m_subgridGeometryModel, "materials");
        auto *subgridMaterial = static_cast<QQuick3DPrincipledMaterial *>(subGridRef.at(0));
        subgridMaterial->setBaseColor(gridSubColor);

        theme()->dirtyBits()->gridDirty = false;
        m_changeTracker.themeGridChanged = false;
    }

    // label Adjustments
    if (theme()->dirtyBits()->labelBackgroundColorDirty
            || m_changeTracker.themeLabelBackgroundColorChanged) {
        QColor labelBackgroundColor = theme()->labelBackgroundColor();
        changeLabelBackgroundColor(m_repeaterX, labelBackgroundColor);
        changeLabelBackgroundColor(m_repeaterY, labelBackgroundColor);
        changeLabelBackgroundColor(m_repeaterZ, labelBackgroundColor);
        m_titleLabelX->setProperty("backgroundColor", labelBackgroundColor);
        m_titleLabelY->setProperty("backgroundColor", labelBackgroundColor);
        m_titleLabelZ->setProperty("backgroundColor", labelBackgroundColor);
        m_itemLabel->setProperty("backgroundColor", labelBackgroundColor);

        if (m_sliceView) {
            changeLabelBackgroundColor(m_sliceHorizontalLabelRepeater, labelBackgroundColor);
            changeLabelBackgroundColor(m_sliceVerticalLabelRepeater, labelBackgroundColor);
            m_sliceItemLabel->setProperty("backgroundColor", labelBackgroundColor);
            m_sliceHorizontalTitleLabel->setProperty("backgroundColor", labelBackgroundColor);
            m_sliceVerticalTitleLabel->setProperty("backgroundColor", labelBackgroundColor);
        }
        theme()->dirtyBits()->labelBackgroundColorDirty = false;
        m_changeTracker.themeLabelBackgroundColorChanged = false;
    }

    if (theme()->dirtyBits()->labelBackgroundVisibilityDirty
            || m_changeTracker.themeLabelBackgroundVisibilityChanged) {
        bool visible = theme()->isLabelBackgroundVisible();
        changeLabelBackgroundVisible(m_repeaterX, visible);
        changeLabelBackgroundVisible(m_repeaterY, visible);
        changeLabelBackgroundVisible(m_repeaterZ, visible);
        m_titleLabelX->setProperty("backgroundVisible", visible);
        m_titleLabelY->setProperty("backgroundVisible", visible);
        m_titleLabelZ->setProperty("backgroundVisible", visible);
        m_itemLabel->setProperty("backgroundVisible", visible);

        if (m_sliceView) {
            changeLabelBackgroundVisible(m_sliceHorizontalLabelRepeater, visible);
            changeLabelBackgroundVisible(m_sliceVerticalLabelRepeater, visible);
            m_sliceItemLabel->setProperty("backgroundVisible", visible);
            m_sliceHorizontalTitleLabel->setProperty("backgroundVisible", visible);
            m_sliceVerticalTitleLabel->setProperty("backgroundVisible", visible);
        }
        theme()->dirtyBits()->labelBackgroundVisibilityDirty = false;
        m_changeTracker.themeLabelBackgroundVisibilityChanged = false;
    }

    if (theme()->dirtyBits()->labelBorderVisibilityDirty
            || m_changeTracker.themeLabelBorderVisibilityChanged) {
        bool visible = theme()->isLabelBorderVisible();
        changeLabelBorderVisible(m_repeaterX, visible);
        changeLabelBorderVisible(m_repeaterY, visible);
        changeLabelBorderVisible(m_repeaterZ, visible);
        m_titleLabelX->setProperty("borderVisible", visible);
        m_titleLabelY->setProperty("borderVisible", visible);
        m_titleLabelZ->setProperty("borderVisible", visible);
        m_itemLabel->setProperty("borderVisible", visible);

        if (m_sliceView) {
            changeLabelBorderVisible(m_sliceHorizontalLabelRepeater, visible);
            changeLabelBorderVisible(m_sliceVerticalLabelRepeater, visible);
            m_sliceItemLabel->setProperty("borderVisible", visible);
            m_sliceHorizontalTitleLabel->setProperty("borderVisible", visible);
            m_sliceVerticalTitleLabel->setProperty("borderVisible", visible);
        }
        theme()->dirtyBits()->labelBorderVisibilityDirty = false;
        m_changeTracker.themeLabelBorderVisibilityChanged = false;
    }

    if (theme()->dirtyBits()->labelTextColorDirty || m_changeTracker.themeLabelTextColorChanged) {
        QColor labelTextColor = theme()->labelTextColor();
        m_itemLabel->setProperty("labelTextColor", labelTextColor);

        if (m_sliceView && isSliceEnabled())
            m_sliceItemLabel->setProperty("labelTextColor", labelTextColor);
        theme()->dirtyBits()->labelTextColorDirty = false;
        m_changeTracker.themeLabelTextColorChanged = false;
    }

    if (theme()->dirtyBits()->axisXDirty || m_changeTracker.themeAxisXChanged) {
        QColor labelTextColor = theme()->axisX().labelTextColor();
        changeLabelTextColor(m_repeaterX, labelTextColor);
        m_titleLabelX->setProperty("labelTextColor", labelTextColor);
        if (m_sliceView && isSliceEnabled()) {
            if (m_selectionMode == QtGraphs3D::SelectionFlag::Row)
                changeLabelTextColor(m_sliceHorizontalLabelRepeater, labelTextColor);
            m_sliceHorizontalTitleLabel->setProperty("labelTextColor", labelTextColor);
        }
        theme()->dirtyBits()->axisXDirty = false;
        m_changeTracker.themeAxisXChanged = false;
    }

    if (theme()->dirtyBits()->axisYDirty || m_changeTracker.themeAxisYChanged) {
        QColor labelTextColor = theme()->axisY().labelTextColor();
        changeLabelTextColor(m_repeaterY, labelTextColor);
        m_titleLabelY->setProperty("labelTextColor", labelTextColor);
        if (m_sliceView && isSliceEnabled()) {
            changeLabelTextColor(m_sliceVerticalLabelRepeater, labelTextColor);
            m_sliceVerticalTitleLabel->setProperty("labelTextColor", labelTextColor);
        }
        theme()->dirtyBits()->axisYDirty = false;
        m_changeTracker.themeAxisYChanged = false;
    }

    if (theme()->dirtyBits()->axisZDirty || m_changeTracker.themeAxisZChanged) {
        QColor labelTextColor = theme()->axisZ().labelTextColor();
        changeLabelTextColor(m_repeaterZ, labelTextColor);
        m_titleLabelZ->setProperty("labelTextColor", labelTextColor);
        if (m_sliceView && isSliceEnabled()) {
            if (m_selectionMode == QtGraphs3D::SelectionFlag::Column)
                changeLabelTextColor(m_sliceHorizontalLabelRepeater, labelTextColor);
            m_sliceHorizontalTitleLabel->setProperty("labelTextColor", labelTextColor);
        }
        theme()->dirtyBits()->axisZDirty = false;
        m_changeTracker.themeAxisZChanged = false;
    }

    if (theme()->dirtyBits()->labelFontDirty || m_changeTracker.themeLabelFontChanged) {
        auto font = theme()->labelFont();
        changeLabelFont(m_repeaterX, font);
        changeLabelFont(m_repeaterY, font);
        changeLabelFont(m_repeaterZ, font);
        m_titleLabelX->setProperty("labelFont", font);
        m_titleLabelY->setProperty("labelFont", font);
        m_titleLabelZ->setProperty("labelFont", font);
        m_itemLabel->setProperty("labelFont", font);
        m_labelsNeedupdate = true;

        if (m_sliceView && isSliceEnabled()) {
            changeLabelFont(m_sliceHorizontalLabelRepeater, font);
            changeLabelFont(m_sliceVerticalLabelRepeater, font);
            m_sliceItemLabel->setProperty("labelFont", font);
            m_sliceHorizontalTitleLabel->setProperty("labelFont", font);
            m_sliceVerticalTitleLabel->setProperty("labelFont", font);
            updateSliceLabels();
        }
        theme()->dirtyBits()->labelFontDirty = false;
        m_changeTracker.themeLabelFontChanged = false;
        m_isSeriesVisualsDirty = true;
    }

    if (theme()->dirtyBits()->labelsVisibilityDirty
            || m_changeTracker.themeLabelsVisibilityChanged) {
        bool visible = theme()->labelsVisible();
        changeLabelsVisible(m_repeaterX, visible);
        changeLabelsVisible(m_repeaterY, visible);
        changeLabelsVisible(m_repeaterZ, visible);
        m_titleLabelX->setProperty("visible", visible && axisX()->isTitleVisible());
        m_titleLabelY->setProperty("visible", visible && axisY()->isTitleVisible());
        m_titleLabelZ->setProperty("visible", visible && axisZ()->isTitleVisible());
        m_itemLabel->setProperty("visible", visible && m_itemSelected);

        if (m_sliceView) {
            changeLabelsVisible(m_sliceHorizontalLabelRepeater, visible);
            changeLabelsVisible(m_sliceVerticalLabelRepeater, visible);
            m_sliceItemLabel->setProperty("visible", visible && selectionMode()
                                          .testFlag(QtGraphs3D::SelectionFlag::Item));
            m_sliceHorizontalTitleLabel->setProperty("visible", visible);
            m_sliceVerticalTitleLabel->setProperty("visible", visible);
        }
        theme()->dirtyBits()->labelsVisibilityDirty = false;
        m_changeTracker.themeLabelsVisibilityChanged = false;
    }

    // Grid and background adjustments
    if (theme()->dirtyBits()->plotAreaBackgroundColorDirty
            || m_changeTracker.themePlotAreaBackgroundColorChanged) {
        QQmlListReference materialRef(m_background, "materials");
        Q_ASSERT(materialRef.size());
        auto material = static_cast<QQuick3DCustomMaterial *>(materialRef.at(0));
        material->setProperty("baseColor", theme()->plotAreaBackgroundColor());
        theme()->dirtyBits()->plotAreaBackgroundColorDirty = false;
        m_changeTracker.themePlotAreaBackgroundColorChanged = false;
    }

    if (theme()->dirtyBits()->plotAreaBackgroundVisibilityDirty
            || m_changeTracker.themePlotAreaBackgroundVisibilityChanged) {
        QQmlListReference materialRef(m_background, "materials");
        Q_ASSERT(materialRef.size());
        auto *material = static_cast<QQuick3DCustomMaterial *>(materialRef.at(0));
        material->setProperty("baseVisible", theme()->isPlotAreaBackgroundVisible());
        theme()->dirtyBits()->plotAreaBackgroundVisibilityDirty = false;
        m_changeTracker.themePlotAreaBackgroundVisibilityChanged = false;
    }

    if (m_gridLineTypeDirty) {
        m_gridLineType = gridLineType();
        theme()->dirtyBits()->gridVisibilityDirty = true;
        theme()->dirtyBits()->gridDirty = true;
        m_gridUpdate = true;
        m_gridLineTypeDirty = false;
    }

    if (theme()->dirtyBits()->gridVisibilityDirty || m_changeTracker.themeGridVisibilityChanged) {
        bool visible = theme()->isGridVisible();
        QQmlListReference materialRef(m_background, "materials");
        Q_ASSERT(materialRef.size());
        auto *material = static_cast<QQuick3DCustomMaterial *>(materialRef.at(0));
        material->setProperty("gridVisible", visible && (m_gridLineType == QtGraphs3D::GridLineType::Shader));
        m_gridGeometryModel->setVisible(visible &! (m_gridLineType == QtGraphs3D::GridLineType::Shader));
        m_subgridGeometryModel->setVisible(visible &! (m_gridLineType == QtGraphs3D::GridLineType::Shader));

        if (m_sliceView && isSliceEnabled())
            m_sliceGridGeometryModel->setVisible(visible);

        theme()->dirtyBits()->gridVisibilityDirty = false;
        m_changeTracker.themeGridVisibilityChanged = false;
    }

    if (theme()->dirtyBits()->singleHighlightColorDirty) {
        updateSingleHighlightColor();
        theme()->dirtyBits()->singleHighlightColorDirty = false;
    }

    // Other adjustments
    if (theme()->dirtyBits()->backgroundColorDirty
            || theme()->dirtyBits()->backgroundVisibilityDirty
            || m_changeTracker.themeBackgroundColorChanged
            || m_changeTracker.themeBackgroundVisibilityChanged) {
        updateBackgroundColor();
        theme()->dirtyBits()->backgroundColorDirty = false;
        theme()->dirtyBits()->backgroundVisibilityDirty = false;
        m_changeTracker.themeBackgroundColorChanged = false;
        m_changeTracker.themeBackgroundVisibilityChanged = false;
    }

    if (isCustomDataDirty()) {
        forceUpdateCustomVolumes = true;
        updateCustomData();
        setCustomDataDirty(false);
    }

    if (m_changedSeriesList.size()) {
        forceUpdateCustomVolumes = true;
        updateGraph();
        m_changedSeriesList.clear();
    }

    if (m_isSeriesVisualsDirty) {
        forceUpdateCustomVolumes = true;
        if (m_gridLineType == QtGraphs3D::GridLineType::Shader)
            updateGridLineType();
        else
            updateGrid();
        m_labelsNeedupdate = true;
        if (m_sliceView && isSliceEnabled()) {
            updateSliceGrid();
            updateSliceLabels();
        }
        updateGraph();
        m_isSeriesVisualsDirty = false;
    }

    if (m_gridUpdate) {
        if (m_gridLineType == QtGraphs3D::GridLineType::Shader)
            updateGridLineType();
        else
            updateGrid();
    }

    if (m_isDataDirty) {
        forceUpdateCustomVolumes = true;
        updateGraph();
        m_isDataDirty = false;
    }

    if (m_sliceActivatedChanged)
        toggleSliceGraph();

    if (isCustomItemDirty() || forceUpdateCustomVolumes)
        updateCustomVolumes();

    if (m_measureFps)
        QQuickItem::update();

    if (m_labelsNeedupdate) {
        updateLabels();
        updateMultiAxis();
    }

    Q_TRACE(QGraphs3DItemSynch_exit);

    qCDebug(lcGraphs3D, "%s end syncing", qUtf8Printable(QLatin1String(__FUNCTION__)));
}

void QQuickGraphsItem::updateGrid()
{
    Q_TRACE(QGraphs3DItemUpdateGrid_entry);

    QQmlListReference materialsRef(m_background, "materials");
    if (!materialsRef.count())
        return;
    auto *bgMat = static_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    bgMat->setProperty("scale", m_scaleWithBackground);
    qsizetype gridLineCountX = 0;
    qsizetype subGridLineCountX = 0;
    gridLineCountHelper(axisX(), gridLineCountX, subGridLineCountX);

    qsizetype gridLineCountY = 0;
    qsizetype subGridLineCountY = 0;
    gridLineCountHelper(axisY(), gridLineCountY, subGridLineCountY);

    qsizetype gridLineCountZ = 0;
    qsizetype subGridLineCountZ = 0;
    gridLineCountHelper(axisZ(), gridLineCountZ, subGridLineCountZ);

    auto backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    QVector3D scaleX(backgroundScale.x() * lineLengthScaleFactor(),
                     lineWidthScaleFactor(),
                     lineWidthScaleFactor());
    QVector3D scaleY(lineWidthScaleFactor(),
                     backgroundScale.y() * lineLengthScaleFactor(),
                     lineWidthScaleFactor());
    const QVector3D scaleZ(backgroundScale.z() * lineLengthScaleFactor(),
                           lineWidthScaleFactor(),
                           lineWidthScaleFactor());

    const bool xFlipped = isXFlipped();
    const bool yFlipped = isYFlipped();
    const bool zFlipped = isZFlipped();

    const float lineOffset = 0.01f;
    const float backOffsetAdjustment = 0.005f;

    QQuaternion lineRotation(.0f, .0f, .0f, .0f);
    QVector3D rotation(90.0f, 0.0f, 0.0f);

    QByteArray vertices;
    qsizetype calculatedSize = 0;

    QByteArray subvertices;
    qsizetype subCalculatedSize = 0;

    bool usePolar = isPolar() && (m_graphType != QAbstract3DSeries::SeriesType::Bar);

    if (!usePolar) {
        int factor = m_hasVerticalSegmentLine ? 2 : 1;
        calculatedSize = (factor * gridLineCountX + factor * gridLineCountZ + 2 * gridLineCountY)
                         * 2 * sizeof(QVector3D);
        subCalculatedSize = (factor * subGridLineCountX + factor * subGridLineCountZ + 2 * subGridLineCountY)
                         * 2 * sizeof(QVector3D);
    } else {
        int radialMainGridSize = static_cast<QValue3DAxis *>(axisZ())->gridSize() * polarRoundness;
        int radialSubGridSize = static_cast<QValue3DAxis *>(axisZ())->subGridSize()
                                * polarRoundness;

        qsizetype angularMainGridsize = static_cast<QValue3DAxis *>(axisX())->gridSize();
        qsizetype angularSubGridsize = static_cast<QValue3DAxis *>(axisX())->subGridSize();

        calculatedSize = (radialMainGridSize + angularMainGridsize + (2 * gridLineCountY) - 1)
                         * 2 * sizeof(QVector3D);
        subCalculatedSize = (radialSubGridSize + + angularSubGridsize + (2 * subGridLineCountY))

                         * 2 * sizeof(QVector3D);
    }
    vertices.resize(calculatedSize);
    QVector3D *data = reinterpret_cast<QVector3D *>(vertices.data());

    subvertices.resize(subCalculatedSize);
    QVector3D *subdata = reinterpret_cast<QVector3D *>(subvertices.data());

    // Floor horizontal line
    float linePosX = 0.0f;
    float linePosY = backgroundScale.y();
    float linePosZ = 0.0f;
    float scale = m_scaleWithBackground.z();

    float x0 = backgroundScale.x();
    float x1 = -backgroundScale.x();

    float tempLineOffset = -lineOffset;
    if (!yFlipped) {
        linePosY *= -1.0f;
        rotation.setZ(180.0f);
        tempLineOffset *= -1.0f;
    }
    lineRotation = Utils::calculateRotation(rotation);
    linePosY *= m_horizontalFlipFactor;
    tempLineOffset *= m_horizontalFlipFactor;
    if (!usePolar) {
        for (int i = 0; i < subGridLineCountZ; i++) {
            if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosZ = static_cast<QValue3DAxis *>(axisZ())->subGridPositionAt(i) * -scale
                               * 2.0f
                           + scale;
            } else if (axisZ()->type() == QAbstract3DAxis::AxisType::Category) {
                linePosZ = calculateCategoryGridLinePosition(axisZ(), i);
                linePosY = calculateCategoryGridLinePosition(axisY(), i);
            }

            *subdata++ = QVector3D(x0, linePosY + tempLineOffset, linePosZ);
            *subdata++ = QVector3D(x1, linePosY + tempLineOffset, linePosZ);
        }

        for (int i = 0; i < gridLineCountZ; i++) {
            if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosZ = static_cast<QValue3DAxis *>(axisZ())->gridPositionAt(i) * -scale * 2.0f
                           + scale;
            } else if (axisZ()->type() == QAbstract3DAxis::AxisType::Category) {
                linePosZ = calculateCategoryGridLinePosition(axisZ(), i);
                linePosY = calculateCategoryGridLinePosition(axisY(), i);
            }

            *data++ = QVector3D(x0, linePosY + tempLineOffset, linePosZ);
            *data++ = QVector3D(x1, linePosY + tempLineOffset, linePosZ);
        }
    } else {
        auto valueAxisZ = static_cast<QValue3DAxis *>(axisZ());

        for (int k = 0; k < subGridLineCountZ; k++) {
            float degrees = 0.0f;
            const float r = (m_polarRadius) *valueAxisZ->subGridPositionAt(k);
            QVector3D lastPoint(r * qCos(degrees), linePosY + tempLineOffset, r * qSin(degrees));
            for (int i = 1; i <= polarRoundness; i++) {
                degrees = doublePi * i / polarRoundness;
                const float xPos = qCos(degrees);
                const float zPos = qSin(degrees);

                const QVector3D pos(r * xPos, linePosY + tempLineOffset, r * zPos);
                *subdata++ = lastPoint;
                *subdata++ = pos;
                lastPoint = pos;
            }
        }

        for (int k = 0; k < gridLineCountZ; k++) {
            float degrees = 0.0f;
            const float r = (m_polarRadius) *valueAxisZ->gridPositionAt(k);
            QVector3D lastPoint(r * qCos(degrees), linePosY + tempLineOffset, r * qSin(degrees));

            for (int i = 1; i <= polarRoundness; i++) {
                degrees = doublePi * i / polarRoundness;
                const float xPos = qCos(degrees);
                const float zPos = qSin(degrees);

                const QVector3D pos(r * xPos, linePosY + tempLineOffset, r * zPos);
                *data++ = lastPoint;
                *data++ = pos;
                lastPoint = pos;
            }
        }
    }

    // Side vertical line
    linePosX = -backgroundScale.x();
    linePosY = 0.0f;
    rotation = QVector3D(0.0f, 90.0f, 0.0f);

    float y0 = -backgroundScale.y();
    float y1 = backgroundScale.y();

    x0 = -backgroundScale.x();
    x1 = -backgroundScale.x();

    tempLineOffset = lineOffset;

    if (xFlipped) {
        linePosX *= -1.0f;
        rotation.setY(-90.0f);
        tempLineOffset *= -1.0f;
        x0 *= -1.0f;
        x1 *= -1.0f;
    }
    lineRotation = Utils::calculateRotation(rotation);
    if (m_hasVerticalSegmentLine) {
        for (int i = 0; i < subGridLineCountZ; i++) {
            if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosZ = static_cast<QValue3DAxis *>(axisZ())->subGridPositionAt(i) * scale * 2.0f
                           - scale;
            }

            *subdata++ = QVector3D(x0 + tempLineOffset, y0, linePosZ);
            *subdata++ = QVector3D(x1 + tempLineOffset, y1, linePosZ);
        }

        for (int i = 0; i < gridLineCountZ; i++) {
            if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosZ = static_cast<QValue3DAxis *>(axisZ())->gridPositionAt(i) * scale * 2.0f
                           - scale;
            }

            *data++ = QVector3D(x0 + tempLineOffset, y0, linePosZ);
            *data++ = QVector3D(x1 + tempLineOffset, y1, linePosZ);
        }
    }

    // Side horizontal line
    scale = m_scaleWithBackground.y();
    rotation = QVector3D(180.0f, -90.0f, 0.0f);

    float z0 = backgroundScale.z();
    float z1 = -backgroundScale.z();

    x0 = -backgroundScale.x();
    x1 = -backgroundScale.x();

    tempLineOffset = lineOffset;

    if (xFlipped) {
        rotation.setY(90.0f);
        tempLineOffset *= -1.0f;
        x0 *= -1.0f;
        x1 *= -1.0f;
    }
    lineRotation = Utils::calculateRotation(rotation);
    for (int i = 0; i < gridLineCountY; i++) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            linePosY = static_cast<QValue3DAxis *>(axisY())->gridPositionAt(i) * scale * 2.0f
                       - scale;
        } else if (axisY()->type() == QAbstract3DAxis::AxisType::Category) {
            linePosY = calculateCategoryGridLinePosition(axisY(), i);
        }

        *data++ = QVector3D(x0 + tempLineOffset, linePosY, z0);
        *data++ = QVector3D(x1 + tempLineOffset, linePosY, z1);
    }

    for (int i = 0; i < subGridLineCountY; i++) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            linePosY = static_cast<QValue3DAxis *>(axisY())->subGridPositionAt(i) * scale * 2.0f
                       - scale;
        } else if (axisY()->type() == QAbstract3DAxis::AxisType::Category) {
            linePosY = calculateCategoryGridLinePosition(axisY(), i);
        }

        *subdata++ = QVector3D(x0 + tempLineOffset, linePosY, z0);
        *subdata++ = QVector3D(x1 + tempLineOffset, linePosY, z1);
    }

    // Floor vertical line
    linePosY = -backgroundScale.y();
    rotation = QVector3D(-90.0f, 90.0f, 0.0f);

    tempLineOffset = lineOffset;
    z0 = backgroundScale.z();
    z1 = -backgroundScale.z();

    if (yFlipped) {
        linePosY *= -1.0f;
        rotation.setZ(180.0f);
        tempLineOffset *= -1.0f;
    }
    scale = m_scaleWithBackground.x();
    linePosY *= m_horizontalFlipFactor;
    tempLineOffset *= m_horizontalFlipFactor;

    if (!usePolar) {
        for (int i = 0; i < subGridLineCountX; i++) {
            if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosX = static_cast<QValue3DAxis *>(axisX())->subGridPositionAt(i) * scale * 2.0f
                           - scale;
            } else if (axisX()->type() == QAbstract3DAxis::AxisType::Category) {
                linePosX = calculateCategoryGridLinePosition(axisX(), i);
                linePosY = calculateCategoryGridLinePosition(axisY(), i);
            }

            *subdata++ = QVector3D(linePosX, linePosY + tempLineOffset, z0);
            *subdata++ = QVector3D(linePosX, linePosY + tempLineOffset, z1);
        }

        for (int i = 0; i < gridLineCountX; i++) {
            if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosX = static_cast<QValue3DAxis *>(axisX())->gridPositionAt(i) * scale * 2.0f
                           - scale;
            } else if (axisX()->type() == QAbstract3DAxis::AxisType::Category) {
                linePosX = calculateCategoryGridLinePosition(axisX(), i);
                linePosY = calculateCategoryGridLinePosition(axisY(), i);
            }

            *data++ = QVector3D(linePosX, linePosY + tempLineOffset, backgroundScale.z());
            *data++ = QVector3D(linePosX, linePosY + tempLineOffset, -backgroundScale.z());
        }
    } else {
        auto valueAxisX = static_cast<QValue3DAxis *>(axisX());
        const QVector3D center(0.0f, linePosY + tempLineOffset, 0.0f);
        const float halfRatio = ((m_polarRadius) + (m_labelMargin * 0.5f));

        for (int i = 0; i < subGridLineCountX; i++) {
            float angle = valueAxisX->subGridPositionAt(i) * 360.0f - rotationOffset;
            float posX = halfRatio * qCos(qDegreesToRadians(angle));
            float posZ = halfRatio * qSin(qDegreesToRadians(angle));
            *subdata++ = center;
            *subdata++ = QVector3D(posX, linePosY + tempLineOffset, posZ);
        }

        for (int i = 0; i < gridLineCountX - 1; i++) {
            float angle = valueAxisX->gridPositionAt(i) * 360.0f - rotationOffset;
            float posX = halfRatio * qCos(qDegreesToRadians(angle));
            float posZ = halfRatio * qSin(qDegreesToRadians(angle));
            *data++ = center;
            *data++ = QVector3D(posX, linePosY + tempLineOffset, posZ);
        }
    }

    // Back horizontal line
    linePosX = 0.0f;
    rotation = QVector3D(0.0f, 0.0f, 0.0f);

    x0 = -backgroundScale.x();
    x1 = backgroundScale.x();

    z0 = -backgroundScale.z();
    z1 = -backgroundScale.z();

    tempLineOffset = lineOffset;
    float tempBackOffsetAdjustment = backOffsetAdjustment;

    if (zFlipped) {
        rotation.setX(180.0f);
        z0 *= -1.0f;
        z1 *= -1.0f;
        tempLineOffset *= -1.0f;
        tempBackOffsetAdjustment *= -1.0f;
    }
    lineRotation = Utils::calculateRotation(rotation);
    scale = m_scaleWithBackground.y();
    for (int i = 0; i < subGridLineCountY; i++) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            linePosY = static_cast<QValue3DAxis *>(axisY())->subGridPositionAt(i) * scale * 2.0f
                       - scale;
        } else if (axisY()->type() == QAbstract3DAxis::AxisType::Category) {
            linePosY = calculateCategoryGridLinePosition(axisY(), i);
        }
        *subdata++ = QVector3D(x0, linePosY, z0 + tempLineOffset + tempBackOffsetAdjustment);
        *subdata++ = QVector3D(x1, linePosY, z1 + tempLineOffset + tempBackOffsetAdjustment);
    }

    for (int i = 0; i < gridLineCountY; i++) {
        if (axisY()->type() == QAbstract3DAxis::AxisType::Value) {
            linePosY = static_cast<QValue3DAxis *>(axisY())->gridPositionAt(i) * scale * 2.0f
                       - scale;
        } else if (axisY()->type() == QAbstract3DAxis::AxisType::Category) {
            linePosY = calculateCategoryGridLinePosition(axisY(), i);
        }
        *data++ = QVector3D(x0, linePosY, z0 + tempLineOffset + tempBackOffsetAdjustment);
        *data++ = QVector3D(x1, linePosY, z1 + tempLineOffset + tempBackOffsetAdjustment);
    }

    // Back vertical line
    scale = m_scaleWithBackground.x();
    rotation = QVector3D(0.0f, 0.0f, 0.0f);

    y0 = -backgroundScale.y();
    y1 = backgroundScale.y();

    z0 = -backgroundScale.z();
    z1 = -backgroundScale.z();

    tempLineOffset = lineOffset;
    tempBackOffsetAdjustment = backOffsetAdjustment;

    if (zFlipped) {
        rotation.setY(180.0f);
        z0 *= -1.0f;
        z1 *= -1.0f;
        tempLineOffset *= -1.0f;
        tempBackOffsetAdjustment *= -1.0f;
    }
    lineRotation = Utils::calculateRotation(rotation);
    if (m_hasVerticalSegmentLine) {
        for (int i = 0; i < gridLineCountX; i++) {
            if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosX = static_cast<QValue3DAxis *>(axisX())->gridPositionAt(i) * scale * 2.0f
                           - scale;
            }
            *data++ = QVector3D(linePosX, y0, z0 + tempLineOffset + tempBackOffsetAdjustment);
            *data++ = QVector3D(linePosX, y1, z1 + tempLineOffset + tempBackOffsetAdjustment);
        }

        for (int i = 0; i < subGridLineCountX; i++) {
            if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
                linePosX = static_cast<QValue3DAxis *>(axisX())->subGridPositionAt(i) * scale * 2.0f
                           - scale;
            }
            *subdata++ = QVector3D(linePosX, y0, z0 + tempLineOffset + tempBackOffsetAdjustment);
            *subdata++ = QVector3D(linePosX, y1, z1 + tempLineOffset + tempBackOffsetAdjustment);
        }
    }
    QQuick3DGeometry *gridGeometry = m_gridGeometryModel->geometry();
    gridGeometry->setVertexData(vertices);
    gridGeometry->update();
    QQuick3DGeometry *subgridGeometry = m_subgridGeometryModel->geometry();
    subgridGeometry->setVertexData(subvertices);
    subgridGeometry->update();
    m_gridUpdate = false;
    Q_TRACE(QGraphs3DItemUpdateGrid_exit);
}

void QQuickGraphsItem::updateGridLineType()
{
    const int textureSize = 4096;
    QList<QVector4D> grid(textureSize * 2, QVector4D(0, 0, 0, 0));
    QQmlListReference materialsRef(m_background, "materials");
    QQuick3DCustomMaterial *bgMat;
    if (!materialsRef.size()) {
        bgMat = createQmlCustomMaterial(QStringLiteral(":/materials/BackgroundMaterial"));
        bgMat->setParent(m_background);
        materialsRef.append(bgMat);
    } else {
        bgMat = static_cast<QQuick3DCustomMaterial *>(materialsRef.at(0));
    }

    QVariant texAsVariant = bgMat->property("gridTex");
    auto *texinput = texAsVariant.value<QQuick3DShaderUtilsTextureInput *>();
    QQuick3DTexture *texMap = texinput->texture();
    QQuick3DTextureData *mapData = nullptr;
    if (!texMap) {
        texMap = new QQuick3DTexture();
        texMap->setParent(this);
        texMap->setHorizontalTiling(QQuick3DTexture::MirroredRepeat);
        texMap->setVerticalTiling(QQuick3DTexture::MirroredRepeat);
        texMap->setMinFilter(QQuick3DTexture::Linear);
        texMap->setMagFilter(QQuick3DTexture::Nearest);
        mapData = new QQuick3DTextureData();
        mapData->setSize(QSize(textureSize, 2));
        mapData->setFormat(QQuick3DTextureData::RGBA32F);
        mapData->setParent(texMap);
        mapData->setParentItem(texMap);
    } else {
        mapData = texMap->textureData();
    }

    QList<qsizetype> lineCounts(6);
    gridLineCountHelper(axisX(), lineCounts[0], lineCounts[3]);
    gridLineCountHelper(axisY(), lineCounts[1], lineCounts[4]);
    gridLineCountHelper(axisZ(), lineCounts[2], lineCounts[5]);

    float baseWidth = 100;
    QList<int> lineWidths(3);
    lineWidths[0] = baseWidth / m_scaleWithBackground.x();
    lineWidths[1] = baseWidth / m_scaleWithBackground.y();
    lineWidths[2] = baseWidth / m_scaleWithBackground.z();

    QList<QVector4D> axisMask = {QVector4D(1, 0, 0, 1),
                                 QVector4D(0, 1, 0, 1),
                                 QVector4D(0, 0, 1, 1)};

    bgMat->setProperty("scale", m_scaleWithBackground);
    bgMat->setProperty("rootScale", rootNode()->scale());
    bgMat->setProperty("polar", isPolar());
    bool xCat = axisX()->type() == QAbstract3DAxis::AxisType::Category;
    bool zCat = axisZ()->type() == QAbstract3DAxis::AxisType::Category;
    bgMat->setProperty("xCategory", xCat);
    bgMat->setProperty("zCategory", zCat);
    bgMat->setProperty("margin", backgroundScaleMargin());

    for (int i = 0; i < lineCounts.size(); i++) {
        qsizetype lineCount = lineCounts[i];
        int axis = i % 3;
        int subGridOffset = textureSize * float(i > 2);
        QVector4D mask = axisMask.at(axis);
        QVector4D revMask = QVector4D(1, 1, 1, 1) - mask;
        for (int j = 0; j < lineCount; j++) {
            float linePos = -1;
            switch (i) {
            case 0:
                if (!xCat)
                    linePos = static_cast<QValue3DAxis *>(axisX())->gridPositionAt(j);
                else
                    linePos = float(j) / float(lineCount);
                break;
            case 1:
                if (axisY()->type() == QAbstract3DAxis::AxisType::Value)
                    linePos = static_cast<QValue3DAxis *>(axisY())->gridPositionAt(j);
                else
                    linePos = float(j) / float(lineCount);
                break;
            case 2:
                if (!zCat)
                    linePos = static_cast<QValue3DAxis *>(axisZ())->gridPositionAt(j);
                else
                    linePos = float(j) / float(lineCount);
                break;
            case 3:
                if (!xCat)
                    linePos = static_cast<QValue3DAxis *>(axisX())->subGridPositionAt(j);
                break;
            case 4:
                if (axisY()->type() == QAbstract3DAxis::AxisType::Value)
                    linePos = static_cast<QValue3DAxis *>(axisY())->subGridPositionAt(j);
                break;
            case 5:
                if (!zCat)
                    linePos = static_cast<QValue3DAxis *>(axisZ())->subGridPositionAt(j);
                break;
            }
            if (linePos < 0)
                continue;

            int index = ((textureSize - 1) * linePos) + subGridOffset;
            for (int k = 0; k < lineWidths[axis]; k++) {
                float nextIdx = qMin(index + k, textureSize * 2 - 1);
                float prevIdx = qMax(index - k, 0);

                float dist = float(lineWidths[axis] - k) / float(lineWidths[axis]);
                float curDist = (grid[nextIdx] * mask).toVector3D().length();

                if (dist > curDist)
                    grid[nextIdx] = grid[nextIdx] * revMask + dist * mask;

                curDist = (grid[prevIdx] * mask).toVector3D().length();
                if (dist > curDist)
                    grid[prevIdx] = grid[prevIdx] * revMask + dist * mask;
            }
        }
    }

    QByteArray data = QByteArray(reinterpret_cast<char *>(grid.data()),
                                 grid.size() * sizeof(QVector4D));
    mapData->setTextureData(data);
    texMap->setTextureData(mapData);
    texinput->setTexture(texMap);
    m_gridUpdate = false;
}

float QQuickGraphsItem::fontScaleFactor(float pointSize)
{
    return 0.00007f + pointSize / (500000.0f * pointSize);
}

float QQuickGraphsItem::labelAdjustment(float width)
{
    float a = -2.43761e-13f;
    float b = 4.23579e-10f;
    float c = 0.00414881f;

    float factor = a * qPow(width, 3) + b * qPow(width, 2) + c;
#if defined(Q_OS_WIN)
    factor *= .8f;
#endif
    float ret = width * .5f * factor;
    return ret;
}

void QQuickGraphsItem::gridLineCountHelper(QAbstract3DAxis *axis, qsizetype &lineCount, qsizetype &sublineCount)
{
    if (axis->type() == QAbstract3DAxis::AxisType::Value) {
        auto valueAxis = static_cast<QValue3DAxis *>(axis);
        lineCount = valueAxis->gridSize();
        sublineCount = valueAxis->subGridSize();
    } else if (axis->type() == QAbstract3DAxis::AxisType::Category) {
        lineCount = axis->labels().size();
        sublineCount = 0;
    }
}

QVector3D QQuickGraphsItem::graphPosToAbsolute(QVector3D position)
{
    QVector3D pos = position;
    const int maxX = axisX()->max();
    const int minX = axisX()->min();
    const int maxY = axisY()->max();
    const int minY = axisY()->min();
    const int maxZ = axisZ()->max();
    const int minZ = axisZ()->min();

    float xAdjust = 1.0f;
    float yAdjust = 1.0f;
    float zAdjust = 1.0f;

    auto xValueAxis = qobject_cast<QValue3DAxis *>(axisX());
    auto yValueAxis = qobject_cast<QValue3DAxis *>(axisY());
    auto zValueAxis = qobject_cast<QValue3DAxis *>(axisZ());

    if (xValueAxis)
        xAdjust = xValueAxis->reversed()? -1.0f: 1.0f;
    if (yValueAxis)
        yAdjust = yValueAxis->reversed()? -1.0f: 1.0f;
    if (zValueAxis)
        zAdjust = zValueAxis->reversed()? 1.0f: -1.0f;

    const QVector3D adjustment = m_scaleWithBackground * QVector3D(xAdjust, yAdjust, zAdjust);

    float xNormalizer = maxX - minX;
    float xPos = (pos.x() - minX) / xNormalizer;
    float yNormalizer = maxY - minY;
    float yPos = (pos.y() - minY) / yNormalizer;
    float zNormalizer = maxZ - minZ;
    float zPos = (pos.z() - minZ) / zNormalizer;
    pos = QVector3D(xPos, yPos, zPos);
    if (isPolar()) {
        float angle = xPos * M_PI * 2.0f;
        float radius = zPos;
        xPos = radius * qSin(angle) * 1.0f;
        zPos = -(radius * qCos(angle)) * 1.0f;
        yPos = yPos * adjustment.y() * 2.0f - adjustment.y();
        pos = QVector3D(xPos, yPos, zPos);
    } else {
        pos = pos * adjustment * 2.0f - adjustment;
    }
    return pos;
}

void QQuickGraphsItem::updateLabels()
{
    Q_TRACE(QGraphs3DItemUpdateLabels_entry);

    auto labels = axisX()->labels();
    qsizetype labelCount = labels.size();
    float labelAutoAngle = m_labelMargin >= 0? axisX()->labelAutoAngle() : 0;
    float labelAngleFraction = labelAutoAngle / 90.0f;
    float fractionCamX = m_xRotation * labelAngleFraction;
    float fractionCamY = m_yRotation * labelAngleFraction;

    const bool xFlipped = isXFlipped();
    const bool yFlipped = isYFlipped();
    const bool zFlipped = isZFlipped();

    QVector3D labelRotation = QVector3D(0.0f, 0.0f, 0.0f);

    float xPos = 0.0f;
    float yPos = 0.0f;
    float zPos = 0.0f;

    auto backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;

    labelRotation = calculateLabelRotation(labelAutoAngle,
                                           fractionCamX,
                                           fractionCamY,
                                           Qt::Axis::XAxis);

    QQuaternion totalRotation = Utils::calculateRotation(labelRotation);

    float scale = backgroundScale.x() - m_backgroundScaleMargin.x();

    float relativeScale = scale / m_axisX->labels().count();

    float pointSize = theme()->labelFont().pointSizeF();

    float textPadding = pointSize * .5f;

    float labelsMaxWidth = float(findLabelsMaxWidth(axisX()->labels())) + textPadding;

    QFontMetrics fm(theme()->labelFont());
    float labelHeight = fm.height() + textPadding;

    // set base size to some reasonable value
    float fontRatio = labelsMaxWidth / labelHeight;

    float adjustment;
    const float baseSize = 25.0f;
    const float relativePointSize = theme()->labelFont().pointSizeF() / baseSize;
    const float scaleFactor = fontScaleFactor(pointSize) * pointSize;

    if (axisX()->isScaleLabelsByCount()) {
        m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                 0.01f * relativePointSize * relativeScale,
                                 0.01f) * axisX()->labelSize();
        adjustment = m_fontScaled.y() * 110.0f;
    } else {
        m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axisX()->labelSize();
        adjustment = labelAdjustment(labelsMaxWidth * axisX()->labelSize());
    }

    zPos = backgroundScale.z() + adjustment + m_labelMargin;

    adjustment *= qAbs(qSin(qDegreesToRadians(labelRotation.z())));
    const float labelDepthMargin = 0.03f; //margin to prevent z-fighting
    yPos = backgroundScale.y() + adjustment - labelDepthMargin;

    float yOffset = -0.1f;
    if (!yFlipped) {
        yPos *= -1.0f;
        yOffset *= -1.0f;
    }

    if (zFlipped)
        zPos *= -1.0f;

    yPos *= m_horizontalFlipFactor;
    auto labelTrans = QVector3D(0.0f, yPos, zPos);
    float angularLabelZPos = 0.0f;

    const float angularAdjustment{1.1f};
    if (axisX()->type() == QAbstract3DAxis::AxisType::Value) {
        auto valueAxisX = static_cast<QValue3DAxis *>(axisX());
        for (int i = 0; i < repeaterX()->count(); i++) {
            if (labelCount <= i)
                break;
            auto obj = static_cast<QQuick3DNode *>(repeaterX()->objectAt(i));
            if (isPolar()) {
                if (i == repeaterX()->count() - 1) {
                    obj->setVisible(false);
                    break;
                }
                float rad = qDegreesToRadians(valueAxisX->labelPositionAt(i) * 360.0f);
                labelTrans.setX((-qSin(rad) * -scale + qSin(rad) * m_labelMargin * m_polarRadius)
                                * angularAdjustment);
                labelTrans.setY(yPos + yOffset);
                labelTrans.setZ((qCos(rad) * -scale - qCos(rad) * m_labelMargin * m_polarRadius)
                                * angularAdjustment);
                if (i == 0) {
                    angularLabelZPos = labelTrans.z();
                    rad = qDegreesToRadians(valueAxisX->labelPositionAt(i) * 360.0f);
                    labelTrans.setX(
                        (-qSin(rad) * -scale + qSin(rad) * m_labelMargin * m_polarRadius));
                    labelTrans.setY(yPos + yOffset);
                    labelTrans.setZ(
                        (qCos(rad) * -scale - qCos(rad) * m_labelMargin * m_polarRadius));
                }
            } else {
                labelTrans.setX(valueAxisX->labelPositionAt(i) * scale * 2.0f - scale);
            }
            obj->setObjectName(QStringLiteral("ElementAxisXLabel"));
            obj->setScale(m_fontScaled);
            obj->setPosition(labelTrans);
            obj->setRotation(totalRotation);
            qsizetype labelIndex =
                    valueAxisX->reversed() ? labelCount - 1 - i : i;
            obj->setProperty("labelText", labels[labelIndex]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            if (!labels[i].compare(QString(hiddenLabelTag)))
                obj->setVisible(false);
        }
    } else if (axisX()->type() == QAbstract3DAxis::AxisType::Category) {
        for (int i = 0; i < repeaterX()->count(); i++) {
            if (labelCount <= i)
                break;
            labelTrans = calculateCategoryLabelPosition(axisX(), labelTrans, i);
            auto obj = static_cast<QQuick3DNode *>(repeaterX()->objectAt(i));
            obj->setObjectName(QStringLiteral("ElementAxisXLabel"));
            obj->setScale(m_fontScaled);
            obj->setPosition(labelTrans);
            obj->setRotation(totalRotation);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
        }
    }

    float x = labelTrans.x();
    labelTrans.setX(0.0f);
    updateXTitle(labelRotation, labelTrans, totalRotation, labelsMaxWidth, m_fontScaled);
    if (isPolar()) {
        m_titleLabelX->setZ(angularLabelZPos - m_labelMargin * 2.0f);
        m_titleLabelX->setRotation(totalRotation);
    }
    labelTrans.setX(x);

    labels = axisY()->labels();
    labelCount = labels.size();
    labelAutoAngle = m_labelMargin >= 0 ? axisY()->labelAutoAngle() : 0;
    labelAngleFraction = labelAutoAngle / 90.0f;
    fractionCamX = m_xRotation * labelAngleFraction;
    fractionCamY = m_yRotation * labelAngleFraction;
    QVector3D sideLabelRotation(0.0f, -90.0f, 0.0f);
    QVector3D backLabelRotation(0.0f, 0.0f, 0.0f);

    if (labelAutoAngle == 0.0f) {
        if (!xFlipped)
            sideLabelRotation.setY(90.0f);
        if (zFlipped)
            backLabelRotation.setY(180.f);
    } else {
        // Orient side labels somewhat towards the camera
        if (xFlipped) {
            if (zFlipped)
                backLabelRotation.setY(180.0f + (2.0f * labelAutoAngle) - fractionCamX);
            else
                backLabelRotation.setY(-fractionCamX);
            sideLabelRotation.setY(-90.0f + labelAutoAngle - fractionCamX);
        } else {
            if (zFlipped)
                backLabelRotation.setY(180.0f - (2.0f * labelAutoAngle) - fractionCamX);
            else
                backLabelRotation.setY(-fractionCamX);
            sideLabelRotation.setY(90.0f - labelAutoAngle - fractionCamX);
        }
    }

    backLabelRotation.setX(-fractionCamY);
    sideLabelRotation.setX(-fractionCamY);

    totalRotation = Utils::calculateRotation(sideLabelRotation);
    scale = backgroundScale.y() - m_backgroundScaleMargin.y();
    labelsMaxWidth = float(findLabelsMaxWidth(axisY()->labels())) + textPadding;
    fontRatio = labelsMaxWidth / labelHeight;
    relativeScale = scale / m_axisY->labels().count();
    if (axisY()->isScaleLabelsByCount()) {
        m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                 0.01f * relativePointSize * relativeScale,
                                 0.01f) * axisY()->labelSize();
        adjustment = m_fontScaled.y() * 190.0f;
    } else {
        m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axisY()->labelSize();
        adjustment = labelAdjustment(labelsMaxWidth * axisY()->labelSize());
    }

    xPos = backgroundScale.x() - labelDepthMargin;
    if (!xFlipped)
        xPos *= -1.0f;
    labelTrans.setX(xPos);

    zPos = backgroundScale.z() + adjustment + m_labelMargin;
    if (zFlipped)
        zPos *= -1.0f;
    labelTrans.setZ(zPos);
    for (int i = 0; i < repeaterY()->count() / 2; i++) {
        if (labelCount <= i)
            break;
        auto obj = static_cast<QQuick3DNode *>(repeaterY()->objectAt(i));
        auto valueAxisY = static_cast<QValue3DAxis *>(axisY());
        labelTrans.setY(valueAxisY->labelPositionAt(i) * scale * 2.0f - scale);

        obj->setObjectName(QStringLiteral("ElementAxisYLabel"));
        obj->setScale(m_fontScaled);
        obj->setPosition(labelTrans);
        obj->setRotation(totalRotation);
        qsizetype labelIndex = valueAxisY->reversed() ? labelCount - 1 - i : i;
        obj->setProperty("labelText", labels[labelIndex]);
        obj->setProperty("labelWidth", labelsMaxWidth);
        obj->setProperty("labelHeight", labelHeight);
        if (!labels[i].compare(QString(hiddenLabelTag)))
            obj->setVisible(false);
    }

    auto sideLabelTrans = labelTrans;
    auto totalSideLabelRotation = totalRotation;

    labels = axisZ()->labels();
    labelCount = labels.size();
    labelAutoAngle = m_labelMargin >= 0 ? axisZ()->labelAutoAngle() : 0;
    labelAngleFraction = labelAutoAngle / 90.0f;
    fractionCamX = m_xRotation * labelAngleFraction;
    fractionCamY = m_yRotation * labelAngleFraction;

    labelRotation = calculateLabelRotation(labelAutoAngle,
                                           fractionCamX,
                                           fractionCamY,
                                           Qt::Axis::ZAxis);

    totalRotation = Utils::calculateRotation(labelRotation);

    scale = backgroundScale.z() - m_backgroundScaleMargin.z();
    labelsMaxWidth = float(findLabelsMaxWidth(axisZ()->labels())) + textPadding;
    fontRatio = labelsMaxWidth / labelHeight;
    relativeScale = scale / m_axisZ->labels().count();

    if (axisZ()->isScaleLabelsByCount()) {
        m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                 0.01f * relativePointSize * relativeScale,
                                 0.01f) * axisZ()->labelSize();
        adjustment = m_fontScaled.y() * 110.0f;
    } else {
        m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axisZ()->labelSize();
        adjustment = labelAdjustment(labelsMaxWidth * axisZ()->labelSize());
    }

    xPos = backgroundScale.x() + adjustment + m_labelMargin;
    if (xFlipped)
        xPos *= -1.0f;

    adjustment *= qAbs(qSin(qDegreesToRadians(labelRotation.z())));
    yPos = backgroundScale.y() + adjustment - labelDepthMargin;
    if (!yFlipped)
        yPos *= -1.0f;

    yPos *= m_horizontalFlipFactor;
    labelTrans = QVector3D(xPos, yPos, 0.0f);
    if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
        auto valueAxisZ = static_cast<QValue3DAxis *>(axisZ());
        float offsetAdjustment = 0.05f;
        float offset = radialLabelOffset() + offsetAdjustment;
        for (int i = 0; i < repeaterZ()->count(); i++) {
            if (labelCount <= i)
                break;

            auto obj = static_cast<QQuick3DNode *>(repeaterZ()->objectAt(i));
            if (isPolar()) {
                // RADIAL LABELS
                float polarX = backgroundScale.x() * offset + m_labelMargin * 2.0f;
                if (xFlipped)
                    polarX *= -1;
                labelTrans.setX(polarX);
                labelTrans.setY(yPos + yOffset);

                labelTrans.setZ(-valueAxisZ->labelPositionAt(i) * m_polarRadius);
            } else {
                labelTrans.setZ(valueAxisZ->labelPositionAt(i) * scale * -2.0f + scale);
            }
            obj->setObjectName(QStringLiteral("ElementAxisZLabel"));
            obj->setScale(m_fontScaled);
            obj->setPosition(labelTrans);
            obj->setRotation(totalRotation);
            qsizetype labelIndex = valueAxisZ->reversed() ? labelCount - 1 - i : i;
            obj->setProperty("labelText", labels[labelIndex]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            if (!labels[i].compare(QString(hiddenLabelTag)))
                obj->setVisible(false);
        }
    } else if (axisZ()->type() == QAbstract3DAxis::AxisType::Category) {
        for (int i = 0; i < repeaterZ()->count(); i++) {
            if (labelCount <= i)
                break;
            labelTrans = calculateCategoryLabelPosition(axisZ(), labelTrans, i);
            auto obj = static_cast<QQuick3DNode *>(repeaterZ()->objectAt(i));
            obj->setObjectName(QStringLiteral("ElementAxisZLabel"));
            obj->setScale(m_fontScaled);
            obj->setPosition(labelTrans);
            obj->setRotation(totalRotation);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
        }
    }

    float z = labelTrans.z();
    labelTrans.setZ(0.0f);
    updateZTitle(labelRotation, labelTrans, totalRotation, labelsMaxWidth, m_fontScaled);
    labelTrans.setZ(z);

    labels = axisY()->labels();
    labelCount = labels.size();
    totalRotation = Utils::calculateRotation(backLabelRotation);
    scale = backgroundScale.y() - m_backgroundScaleMargin.y();
    labelsMaxWidth = float(findLabelsMaxWidth(axisY()->labels())) + textPadding;
    fontRatio = labelsMaxWidth / labelHeight;
    relativeScale = scale / m_axisY->labels().count();
    if (axisY()->isScaleLabelsByCount()) {
        m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                 0.01f * relativePointSize * relativeScale,
                                 0.01f) * axisY()->labelSize();
        adjustment = m_fontScaled.y() * 190.0f;
    } else {
        m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axisY()->labelSize();
        adjustment = labelAdjustment(labelsMaxWidth * axisY()->labelSize());
    }

    xPos = backgroundScale.x() + adjustment + m_labelMargin;
    if (xFlipped)
        xPos *= -1.0f;
    labelTrans.setX(xPos);

    zPos = -backgroundScale.z() + labelDepthMargin;
    if (zFlipped)
        zPos *= -1.0f;
    labelTrans.setZ(zPos);

    for (int i = 0; i < repeaterY()->count() / 2; i++) {
        if (labelCount <= i)
            break;
        auto obj = static_cast<QQuick3DNode *>(
            repeaterY()->objectAt(i + (repeaterY()->count() / 2)));
        auto valueAxisY = static_cast<QValue3DAxis *>(axisY());
        labelTrans.setY(valueAxisY->labelPositionAt(i) * scale * 2.0f
                        - scale);
        obj->setObjectName(QStringLiteral("ElementAxisYLabel"));
        obj->setScale(m_fontScaled);
        obj->setPosition(labelTrans);
        obj->setRotation(totalRotation);
        qsizetype labelIndex = valueAxisY->reversed() ? labelCount - 1 - i : i;
        obj->setProperty("labelText", labels[labelIndex]);
        obj->setProperty("labelWidth", labelsMaxWidth);
        obj->setProperty("labelHeight", labelHeight);
        if (!labels[i].compare(QString(hiddenLabelTag)))
            obj->setVisible(false);
    }

    QVector3D backLabelTrans = labelTrans;
    QQuaternion totalBackLabelRotation = totalRotation;
    updateYTitle(sideLabelRotation,
                 backLabelRotation,
                 sideLabelTrans,
                 backLabelTrans,
                 totalSideLabelRotation,
                 totalBackLabelRotation,
                 labelsMaxWidth,
                 m_fontScaled);
    m_labelsNeedupdate = false;
    Q_TRACE(QGraphs3DItemUpdateLabels_exit);
}

QVector3D QQuickGraphsItem::calculateLabelRotation(float labelAutoAngle,
                                                   float fractionCamX,
                                                   float fractionCamY,
                                                   Qt::Axis axis)
{

    QVector3D labelRotation;
    const bool xFlipped = isXFlipped();
    const bool yFlipped = isYFlipped();
    const bool zFlipped = isZFlipped();

    if (axis == Qt::Axis::XAxis) {
        if (labelAutoAngle == 0.0f) {
            labelRotation = QVector3D(-90.0f, 90.0f, 0.0f);
            if (xFlipped)
                labelRotation.setY(-90.0f);
            if (yFlipped) {
                if (xFlipped)
                    labelRotation.setY(-90.0f);
                else
                    labelRotation.setY(90.0f);
                labelRotation.setX(90.0f);
            }
        } else {
            if (xFlipped)
                labelRotation.setY(-90.0f);
            else
                labelRotation.setY(90.0f);
            if (yFlipped) {
                if (zFlipped) {
                    if (xFlipped) {
                        labelRotation.setX(90.0f
                                           - (2.0f * labelAutoAngle - fractionCamX)
                                                 * (labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle - fractionCamY);
                    } else {
                        labelRotation.setX(90.0f
                                           - (2.0f * labelAutoAngle + fractionCamX)
                                                 * (labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle + fractionCamY);
                    }
                } else {
                    if (xFlipped) {
                        labelRotation.setX(90.0f
                                           + fractionCamX * -(labelAutoAngle + fractionCamY)
                                                 / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle + fractionCamY);
                    } else {
                        labelRotation.setX(90.0f
                                           - fractionCamX * (-labelAutoAngle - fractionCamY)
                                                 / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle - fractionCamY);
                    }
                }
            } else {
                if (zFlipped) {
                    if (xFlipped) {
                        labelRotation.setX(-90.0f
                                           + (2.0f * labelAutoAngle - fractionCamX)
                                                 * (labelAutoAngle - fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle - fractionCamY);
                    } else {
                        labelRotation.setX(-90.0f
                                           + (2.0f * labelAutoAngle + fractionCamX)
                                                 * (labelAutoAngle - fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle + fractionCamY);
                    }
                } else {
                    if (xFlipped) {
                        labelRotation.setX(-90.0f
                                           - fractionCamX * (-labelAutoAngle + fractionCamY)
                                                 / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle + fractionCamY);
                    } else {
                        labelRotation.setX(-90.0f
                                           + fractionCamX * -(labelAutoAngle - fractionCamY)
                                                 / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle - fractionCamY);
                    }
                }
            }
        }
        if (isPolar())
            labelRotation.setY(0.0f);
    } else if (axis == Qt::Axis::ZAxis) {
        if (labelAutoAngle == 0.0f) {
            labelRotation = QVector3D(90.0f, 0.0f, 0.0f);
            if (zFlipped)
                labelRotation.setY(180.0f);
            if (yFlipped) {
                if (zFlipped)
                    labelRotation.setY(180.0f);
                else
                    labelRotation.setY(0.0f);
                labelRotation.setX(90.0f);
            } else {
                labelRotation.setX(-90.0f);
            }
        } else {
            if (zFlipped)
                labelRotation.setY(180.0f);
            else
                labelRotation.setY(0.0f);
            if (yFlipped) {
                if (zFlipped) {
                    if (xFlipped) {
                        labelRotation.setX(90.0f
                                - (labelAutoAngle - fractionCamX)
                                * (-labelAutoAngle - fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle + fractionCamY);
                    } else {
                        labelRotation.setX(90.0f
                                + (labelAutoAngle + fractionCamX)
                                * (labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle - fractionCamY);
                    }
                } else {
                    if (xFlipped) {
                        labelRotation.setX(90.0f
                                + (labelAutoAngle - fractionCamX)
                                * -(labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle - fractionCamY);
                    } else {
                        labelRotation.setX(90.0f
                                - (labelAutoAngle + fractionCamX)
                                * (labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle + fractionCamY);
                    }
                }
            } else {
                if (zFlipped) {
                    if (xFlipped) {
                        labelRotation.setX(-90.0f
                                + (labelAutoAngle - fractionCamX)
                                * (-labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle + fractionCamY);
                    } else {
                        labelRotation.setX(-90.0f
                                - (labelAutoAngle + fractionCamX)
                                * (labelAutoAngle - fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle - fractionCamY);
                    }
                } else {
                    if (xFlipped) {
                        labelRotation.setX(-90.0f
                                - (labelAutoAngle - fractionCamX)
                                * (-labelAutoAngle + fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(labelAutoAngle - fractionCamY);
                    } else {
                        labelRotation.setX(-90.0f
                                + (labelAutoAngle + fractionCamX)
                                * (labelAutoAngle - fractionCamY) / labelAutoAngle);
                        labelRotation.setZ(-labelAutoAngle + fractionCamY);
                    }
                }
            }
        }
    }
    return labelRotation;
}

void QQuickGraphsItem::updateRadialLabelOffset()
{
    if (!isPolar())
        return;

    QVector3D backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    float offset = radialLabelOffset();
    float scale = backgroundScale.x() + (m_backgroundScaleMargin.x());
    float polarX = scale * offset + m_labelMargin * 2.0f;
    if (isXFlipped())
        polarX *= -1;
    if (axisZ()->type() == QAbstract3DAxis::AxisType::Value) {
        for (int i = 0; i < repeaterZ()->count(); i++) {
            QQuick3DNode *obj = static_cast<QQuick3DNode *>(repeaterZ()->objectAt(i));
            QVector3D pos = obj->position();
            pos.setX(polarX);
            obj->setPosition(pos);
        }
    }

    polarX += m_labelMargin * 2.5f;
    QVector3D pos = m_titleLabelZ->position();
    pos.setX(polarX);
    m_titleLabelZ->setPosition(pos);
}

void QQuickGraphsItem::releaseMultiAxis(QAbstract3DAxis::AxisOrientation orientation, qsizetype index)
{
    QList<MultiAxis> *multiAxes = nullptr;
    switch (orientation) {
        case QAbstract3DAxis::AxisOrientation::X:
            multiAxes = &m_multiAxesX;
            break;
        case QAbstract3DAxis::AxisOrientation::Y:
            multiAxes = &m_multiAxesY;
            break;
        case QAbstract3DAxis::AxisOrientation::Z:
            multiAxes = &m_multiAxesZ;
            break;
        case QAbstract3DAxis::AxisOrientation::None:
            return;
    }

    auto indexExists =  [index](MultiAxis axis) { return axis.seriesIndex == index; };
    const auto it = std::find_if(multiAxes->begin(), multiAxes->end(), indexExists);
    if (it != multiAxes->end()) {
        it->repeater->setModel(0);
        it->repeater->deleteLater();
        it->grid->deleteLater();
        it->titleLabel->deleteLater();
    }
    multiAxes->erase(std::remove_if(multiAxes->begin(), multiAxes->end(), indexExists), multiAxes->end());

    return;
}

void QQuickGraphsItem::connectMultiAxis(QAbstract3DAxis *axis)
{
    QObject::connect(axis,
                     &QAbstract3DAxis::titleChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelsChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::rangeChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::autoAdjustRangeChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelAutoAngleChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelAutoAngleChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::scaleLabelsByCountChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelSizeChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleVisibleChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::labelVisibleChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleFixedChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);
    QObject::connect(axis,
                     &QAbstract3DAxis::titleOffsetChanged,
                     this,
                     &QQuickGraphsItem::handleMultiAxisDirty);

    if (axis->type() == QAbstract3DAxis::AxisType::Value) {
        QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(axis);
        QObject::connect(valueAxis,
                         &QValue3DAxis::segmentCountChanged,
                         this,
                         &QQuickGraphsItem::handleMultiAxisDirty);
        QObject::connect(valueAxis,
                         &QValue3DAxis::subSegmentCountChanged,
                         this,
                         &QQuickGraphsItem::handleMultiAxisDirty);
        QObject::connect(valueAxis,
                         &QValue3DAxis::labelFormatChanged,
                         this,
                         &QQuickGraphsItem::handleMultiAxisDirty);
        QObject::connect(valueAxis,
                         &QValue3DAxis::reversedChanged,
                         this,
                         &QQuickGraphsItem::handleMultiAxisDirty);
    }
}

void QQuickGraphsItem::updateMultiAxis()
{
    const QList<QAbstract3DAxis::AxisOrientation> orientations = {
        QAbstract3DAxis::AxisOrientation::X,
        QAbstract3DAxis::AxisOrientation::Y,
        QAbstract3DAxis::AxisOrientation::Z};

    for (qsizetype i = 0; i < m_seriesList.count(); i++) {
        auto series = m_seriesList.at(i);

        for (auto orientation : orientations) {
            QAbstract3DAxis *axis = getSeriesMultiAxis(series, orientation);
            if (axis) {
                updateMultiAxisLabels(i, axis);
                updateMultiAxisGrid(i, axis);
            } else {
                releaseMultiAxis(orientation, i);
            }
        }
    }

    m_isDataDirty = true;
}

void QQuickGraphsItem::updateMultiAxisLabels(qsizetype axisIndex, QAbstract3DAxis *axis) {

    auto labels = axis->labels();
    qsizetype labelCount = labels.size();
    const bool xFlipped = isXFlipped();
    const bool yFlipped = isYFlipped();
    const bool zFlipped = isZFlipped();

    const float pointSize = theme()->labelFont().pointSizeF();
    const float textPadding = pointSize * .5f;
    const auto backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    QFontMetrics fm(theme()->labelFont());
    const float labelHeight = fm.height() + textPadding;

    const float baseSize = 25.0f;
    const float relativePointSize = theme()->labelFont().pointSizeF() / baseSize;
    const float scaleFactor = fontScaleFactor(pointSize) * pointSize;
    const float labelDepthMargin = 0.03f; //margin to prevent z-fighting

    float xPos = 0.0f;
    float yPos = 0.0f;
    float zPos = 0.0f;
    QVector3D labelTrans;

    MultiAxis multiAxis = getMultiAxis(axisIndex, axis);
    if (multiAxis.seriesIndex == -1) // uninitalized axis
        return;

    QQuick3DRepeater *repeater = multiAxis.repeater;
    repeater->setModel(axis->orientation() == QAbstract3DAxis::AxisOrientation::Y
                           ? 2 * labelCount
                           : labelCount);
    repeater->setVisible(axis->labelsVisible());

    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::X) {
        float labelAutoAngle = m_labelMargin >= 0? axis->labelAutoAngle() : 0;
        float labelAngleFraction = labelAutoAngle / 90.0f;
        float fractionCamX = m_xRotation * labelAngleFraction;
        float fractionCamY = m_yRotation * labelAngleFraction;

        QVector3D labelRotation = QVector3D(0.0f, 0.0f, 0.0f);

        labelRotation = calculateLabelRotation(labelAutoAngle,
                                               fractionCamX,
                                               fractionCamY,
                                               Qt::Axis::XAxis);

        QQuaternion totalRotation = Utils::calculateRotation(labelRotation);

        float scale = backgroundScale.x() - m_backgroundScaleMargin.x();
        float relativeScale = scale / axis->labels().count();
        float labelsMaxWidth = float(findLabelsMaxWidth(axis->labels())) + textPadding;

        // set base size to some reasonable value
        float fontRatio = labelsMaxWidth / labelHeight;

        float adjustment;

        if (axis->isScaleLabelsByCount()) {
            m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                     0.01f * relativePointSize * relativeScale,
                                     0.01f) * axis->labelSize();
            adjustment = m_fontScaled.y() * 110.0f;
        } else {
            m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axis->labelSize();
            adjustment = labelAdjustment(labelsMaxWidth * axis->labelSize());
        }

        zPos = backgroundScale.z() + adjustment + m_labelMargin;

        zPos += (axisIndex + 1) * 0.3f;

        yPos = backgroundScale.y() - labelDepthMargin;

        float yOffset = -0.1f;
        if (!yFlipped) {
            yPos *= -1.0f;
            yOffset *= -1.0f;
        }

        if (zFlipped)
            zPos *= -1.0f;

        labelTrans = QVector3D(0.0f, yPos, zPos);
        float angularLabelZPos = 0.0f;

        const float angularAdjustment{1.1f};

        if (axis->type() == QAbstract3DAxis::AxisType::Value) {
            auto valueAxisX = static_cast<QValue3DAxis *>(axis);
            for (int i = 0; i < repeater->count(); i++) {
                if (labelCount <= i)
                    break;
                auto obj = static_cast<QQuick3DModel *>(repeater->objectAt(i));
                if (isPolar()) {
                    if (i == repeater->count() - 1) {
                        obj->setVisible(false);
                        break;
                    }
                    float polarRadius = m_polarRadius + (axisIndex) * 2.0f;
                    float rad = qDegreesToRadians(valueAxisX->labelPositionAt(i) * 360.0f);
                    labelTrans.setX((-qSin(rad) * -scale + qSin(rad) * m_labelMargin * polarRadius)
                                    * angularAdjustment);
                    labelTrans.setY(yPos + yOffset);
                    labelTrans.setZ((qCos(rad) * -scale - qCos(rad) * m_labelMargin * polarRadius)
                                    * angularAdjustment);
                } else {
                    labelTrans.setX(valueAxisX->labelPositionAt(i) * scale * 2.0f - scale);
                }
                obj->setObjectName(QStringLiteral("ElementAxisXLabel"));
                obj->setScale(m_fontScaled);
                obj->setPosition(labelTrans);
                obj->setRotation(totalRotation);
                qsizetype labelIndex =
                    valueAxisX->reversed() ? labelCount - 1 - i : i;
                obj->setProperty("labelText", labels[labelIndex]);
                obj->setProperty("labelWidth", labelsMaxWidth);
                obj->setProperty("labelHeight", labelHeight);
                if (!labels[i].compare(QString(hiddenLabelTag)))
                    obj->setVisible(false);

                obj->setProperty("labelTextColor", theme()->axisX().labelTextColor());
                obj->setProperty("labelFont", theme()->labelFont());
                obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
                obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
                obj->setProperty("borderVisible", theme()->isLabelBorderVisible());

            }
        } else if (axis->type() == QAbstract3DAxis::AxisType::Category) {
            float stepSize = backgroundScale.x() * 2.0f / repeater->count();
            float startPos = -scaleWithBackground().x();
            for (int i = 0; i < repeater->count(); i++) {
                if (labelCount <= i)
                    break;
                QVector3D pos = labelTrans;
                float xPos = startPos + (stepSize * i) + stepSize * 0.5;
                pos.setX(xPos);
                auto obj = static_cast<QQuick3DNode *>(repeater->objectAt(i));
                obj->setObjectName(QStringLiteral("ElementAxisXLabel"));
                obj->setScale(m_fontScaled);
                obj->setPosition(pos);
                obj->setRotation(totalRotation);
                obj->setProperty("labelText", labels[i]);
                obj->setProperty("labelWidth", labelsMaxWidth);
                obj->setProperty("labelHeight", labelHeight);

                obj->setProperty("labelTextColor", theme()->axisX().labelTextColor());
                obj->setProperty("labelFont", theme()->labelFont());
                obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
                obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
                obj->setProperty("borderVisible", theme()->isLabelBorderVisible());
            }
        }

        labelTrans.setX(0.0f);
        QQuick3DNode *xLabel = multiAxis.titleLabel;

        xLabel->setProperty("labelText", axis->title());
        xLabel->setVisible(axis->isTitleVisible());
        xLabel->setProperty("backgroundColor", theme()->labelBackgroundColor());
        xLabel->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
        xLabel->setProperty("borderVisible", theme()->isLabelBorderVisible());
        xLabel->setProperty("labelTextColor", theme()->axisX().labelTextColor());
        xLabel->setProperty("labelFont", theme()->labelFont());

        updateXTitle(labelRotation, labelTrans, totalRotation, labelsMaxWidth, m_fontScaled, axis, xLabel);
        if (isPolar()) {
            xLabel->setZ(angularLabelZPos - m_labelMargin * 2.0f);
            xLabel->setRotation(totalRotation);
        }
    } else if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Y) {

        float labelAutoAngle = m_labelMargin >= 0 ? axis->labelAutoAngle() : 0;
        float labelAngleFraction = labelAutoAngle / 90.0f;
        float fractionCamX = m_xRotation * labelAngleFraction;
        float fractionCamY = m_yRotation * labelAngleFraction;
        QVector3D sideLabelRotation(0.0f, -90.0f, 0.0f);
        QVector3D backLabelRotation(0.0f, 0.0f, 0.0f);

        if (labelAutoAngle == 0.0f) {
            if (!xFlipped)
                sideLabelRotation.setY(90.0f);
            if (zFlipped)
                backLabelRotation.setY(180.f);
        } else {
            // Orient side labels somewhat towards the camera
            if (xFlipped) {
                if (zFlipped)
                    backLabelRotation.setY(180.0f + (2.0f * labelAutoAngle) - fractionCamX);
                else
                    backLabelRotation.setY(-fractionCamX);
                sideLabelRotation.setY(-90.0f + labelAutoAngle - fractionCamX);
            } else {
                if (zFlipped)
                    backLabelRotation.setY(180.0f - (2.0f * labelAutoAngle) - fractionCamX);
                else
                    backLabelRotation.setY(-fractionCamX);
                sideLabelRotation.setY(90.0f - labelAutoAngle - fractionCamX);
            }
        }

        backLabelRotation.setX(-fractionCamY);
        sideLabelRotation.setX(-fractionCamY);

        float adjustment;

        QQuaternion totalRotation = Utils::calculateRotation(sideLabelRotation);
        float scale = backgroundScale.y() - m_backgroundScaleMargin.y();
        float labelsMaxWidth = float(findLabelsMaxWidth(axis->labels())) + textPadding;
        float fontRatio = labelsMaxWidth / labelHeight;
        float relativeScale = scale / axis->labels().count();
        if (axis->isScaleLabelsByCount()) {
            m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                     0.01f * relativePointSize * relativeScale,
                                     0.01f) * axis->labelSize();
            adjustment = m_fontScaled.y() * 190.0f;
        } else {
            m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axis->labelSize();
            adjustment = labelAdjustment(labelsMaxWidth * axis->labelSize());
        }

        xPos = backgroundScale.x() - labelDepthMargin;
        if (!xFlipped)
            xPos *= -1.0f;

        labelTrans.setX(xPos);

        zPos = backgroundScale.z() + adjustment + m_labelMargin;
        zPos += (axisIndex + 1) * 0.3f;
        if (zFlipped)
            zPos *= -1.0f;
        labelTrans.setZ(zPos);

        for (int i = 0; i < repeater->count() / 2; i++) {
            if (labelCount <= i)
                break;
            auto obj = static_cast<QQuick3DNode *>(repeater->objectAt(i));
            auto valueAxisY = static_cast<QValue3DAxis *>(axis);
            labelTrans.setY(valueAxisY->labelPositionAt(i) * scale * 2.0f - scale);

            obj->setObjectName(QStringLiteral("ElementAxisYLabel"));
            obj->setScale(m_fontScaled);
            obj->setPosition(labelTrans);
            obj->setRotation(totalRotation);
            qsizetype labelIndex = valueAxisY->reversed() ? labelCount - 1 - i : i;
            obj->setProperty("labelText", labels[labelIndex]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            if (!labels[i].compare(QString(hiddenLabelTag)))
                obj->setVisible(false);

            obj->setProperty("labelTextColor", theme()->axisY().labelTextColor());
            obj->setProperty("labelFont", theme()->labelFont());
            obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
            obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
            obj->setProperty("borderVisible", theme()->isLabelBorderVisible());
        }

        auto sideLabelTrans = labelTrans;
        auto totalSideLabelRotation = totalRotation;

    labelCount = labels.size();
    totalRotation = Utils::calculateRotation(backLabelRotation);
    scale = backgroundScale.y() - m_backgroundScaleMargin.y();
    labelsMaxWidth = float(findLabelsMaxWidth(axis->labels())) + textPadding;
    fontRatio = labelsMaxWidth / labelHeight;
    relativeScale = scale / axis->labels().count();
    if (axis->isScaleLabelsByCount()) {
        m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                 0.01f * relativePointSize * relativeScale,
                                 0.01f) * axis->labelSize();
        adjustment = m_fontScaled.y() * 190.0f;
    } else {
        m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axis->labelSize();
        adjustment = labelAdjustment(labelsMaxWidth * axis->labelSize());
    }

    xPos = backgroundScale.x() + adjustment + m_labelMargin;
    xPos += (axisIndex + 1) * 0.3f;
    if (xFlipped)
        xPos *= -1.0f;
    labelTrans.setX(xPos);

    zPos = -backgroundScale.z() + labelDepthMargin;
    if (zFlipped)
        zPos *= -1.0f;
    labelTrans.setZ(zPos);

    for (int i = 0; i < repeater->count() / 2; i++) {
        if (labelCount <= i)
            break;
        auto obj = static_cast<QQuick3DNode *>(
            repeater->objectAt(i + (repeater->count() / 2)));
        auto valueAxisY = static_cast<QValue3DAxis *>(axis);
        labelTrans.setY(valueAxisY->labelPositionAt(i) * scale * 2.0f
                        - scale);
        obj->setObjectName(QStringLiteral("ElementAxisYLabel"));
        obj->setScale(m_fontScaled);
        obj->setPosition(labelTrans);
        obj->setRotation(totalRotation);
        qsizetype labelIndex = valueAxisY->reversed() ? labelCount - 1 - i : i;
        obj->setProperty("labelText", labels[labelIndex]);
        obj->setProperty("labelWidth", labelsMaxWidth);
        obj->setProperty("labelHeight", labelHeight);
        if (!labels[i].compare(QString(hiddenLabelTag)))
            obj->setVisible(false);

        obj->setProperty("labelTextColor", theme()->axisY().labelTextColor());
        obj->setProperty("labelFont", theme()->labelFont());
        obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
        obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
        obj->setProperty("borderVisible", theme()->isLabelBorderVisible());
    }

    QVector3D backLabelTrans = labelTrans;
    QQuaternion totalBackLabelRotation = totalRotation;

    QQuick3DNode *yLabel = multiAxis.titleLabel;
    yLabel->setProperty("labelText", axis->title());
    yLabel->setVisible(axis->isTitleVisible());
    yLabel->setProperty("backgroundColor", theme()->labelBackgroundColor());
    yLabel->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
    yLabel->setProperty("borderVisible", theme()->isLabelBorderVisible());
    yLabel->setProperty("labelTextColor", theme()->axisX().labelTextColor());
    yLabel->setProperty("labelFont", theme()->labelFont());

    updateYTitle(sideLabelRotation,
                 backLabelRotation,
                 sideLabelTrans,
                 backLabelTrans,
                 totalSideLabelRotation,
                 totalBackLabelRotation,
                 labelsMaxWidth,
                 m_fontScaled,
                 axis,
                 yLabel);

    } else if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Z) {

        float labelAutoAngle = m_labelMargin >= 0 ? axis->labelAutoAngle() : 0;
        float labelAngleFraction = labelAutoAngle / 90.0f;
        float fractionCamX = m_xRotation * labelAngleFraction;
        float fractionCamY = m_yRotation * labelAngleFraction;

        QVector3D labelRotation = calculateLabelRotation(labelAutoAngle,
                                                         fractionCamX,
                                                         fractionCamY,
                                                         Qt::Axis::ZAxis);

        QQuaternion totalRotation = Utils::calculateRotation(labelRotation);

        float scale = backgroundScale.z() - m_backgroundScaleMargin.z();
        float labelsMaxWidth = float(findLabelsMaxWidth(axis->labels())) + textPadding;
        float fontRatio = labelsMaxWidth / labelHeight;
        float relativeScale = scale / axis->labels().count();

        float adjustment;
        if (axis->isScaleLabelsByCount()) {
            m_fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                     0.01f * relativePointSize * relativeScale,
                                     0.01f) * axis->labelSize();
            adjustment = m_fontScaled.y() * 110.0f;
        } else {
            m_fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f)
                * axis->labelSize();
            adjustment = labelAdjustment(labelsMaxWidth * axis->labelSize());
        }

        xPos = backgroundScale.x() + adjustment + m_labelMargin;
        xPos += (axisIndex +1) * 0.3f;
        if (xFlipped)
            xPos *= -1.0f;


        yPos = backgroundScale.y() - labelDepthMargin;
        float yOffset = -0.1f;
        if (!yFlipped) {
            yPos *= -1.0f;
            yOffset *= -1.0f;
        }

        labelTrans = QVector3D(xPos, yPos, 0.0f);
        if (axis->type() == QAbstract3DAxis::AxisType::Value) {
            auto valueAxisZ = static_cast<QValue3DAxis *>(axis);
            float offsetAdjustment = 0.05f;
            float offset = radialLabelOffset() + offsetAdjustment;
            for (int i = 0; i < repeater->count(); i++) {
                if (labelCount <= i)
                    break;

                auto obj = static_cast<QQuick3DNode *>(repeater->objectAt(i));
                if (isPolar()) {
                    // RADIAL LABELS
                    float polarX = backgroundScale.x() * offset + m_labelMargin * 2.0f;
                    polarX += (axisIndex + 1) * 0.3f;
                    if (xFlipped)
                        polarX *= -1;
                    labelTrans.setX(polarX);
                    labelTrans.setY(yPos + yOffset);

                    labelTrans.setZ(-valueAxisZ->labelPositionAt(i) * m_polarRadius);
                } else {
                    labelTrans.setZ(valueAxisZ->labelPositionAt(i) * scale * -2.0f + scale);
                }
                obj->setObjectName(QStringLiteral("ElementAxisZLabel"));
                obj->setScale(m_fontScaled);
                obj->setPosition(labelTrans);
                obj->setRotation(totalRotation);
                qsizetype labelIndex = valueAxisZ->reversed() ? labelCount - 1 - i : i;
                obj->setProperty("labelText", labels[labelIndex]);
                obj->setProperty("labelWidth", labelsMaxWidth);
                obj->setProperty("labelHeight", labelHeight);
                if (!labels[i].compare(QString(hiddenLabelTag)))
                    obj->setVisible(false);

                obj->setProperty("labelTextColor", theme()->axisZ().labelTextColor());
                obj->setProperty("labelFont", theme()->labelFont());
                obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
                obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
                obj->setProperty("borderVisible", theme()->isLabelBorderVisible());
            }
        } else if (axis->type() == QAbstract3DAxis::AxisType::Category) {
            float stepSize = backgroundScale.z() * 2.0f / repeater->count();
            float startPos = -scaleWithBackground().z();
            for (int i = 0; i < repeater->count(); i++) {
                if (labelCount <= i)
                    break;
                QVector3D pos = labelTrans;
                float zPos = startPos + (stepSize * i) + stepSize * 0.5;
                pos.setZ(zPos);
                auto obj = static_cast<QQuick3DNode *>(repeater->objectAt(i));
                obj->setObjectName(QStringLiteral("ElementAxisZLabel"));
                obj->setScale(m_fontScaled);
                obj->setPosition(pos);
                obj->setRotation(totalRotation);
                obj->setProperty("labelText", labels[i]);
                obj->setProperty("labelWidth", labelsMaxWidth);
                obj->setProperty("labelHeight", labelHeight);

                obj->setProperty("labelTextColor", theme()->axisZ().labelTextColor());
                obj->setProperty("labelFont", theme()->labelFont());
                obj->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
                obj->setProperty("backgroundColor", theme()->labelBackgroundColor());
                obj->setProperty("borderVisible", theme()->isLabelBorderVisible());
            }
        }

        labelTrans.setZ(0.0f);
        QQuick3DNode *zLabel = multiAxis.titleLabel;
        zLabel->setProperty("labelText", axis->title());
        zLabel->setVisible(axis->isTitleVisible());
        zLabel->setProperty("backgroundColor", theme()->labelBackgroundColor());
        zLabel->setProperty("backgroundVisible", theme()->isLabelBackgroundVisible());
        zLabel->setProperty("borderVisible", theme()->isLabelBorderVisible());
        zLabel->setProperty("labelTextColor", theme()->axisX().labelTextColor());
        zLabel->setProperty("labelFont", theme()->labelFont());
        updateZTitle(labelRotation,
                     labelTrans,
                     totalRotation,
                     labelsMaxWidth,
                     m_fontScaled,
                     axis,
                     zLabel);
    }
}

QQuickGraphsItem::MultiAxis QQuickGraphsItem::getMultiAxis(qsizetype axisIndex, QAbstract3DAxis *axis)
{

    QList<MultiAxis> *multiAxes = nullptr;
    switch (axis->orientation()) {
        case QAbstract3DAxis::AxisOrientation::X:
            multiAxes = &m_multiAxesX;
            break;
        case QAbstract3DAxis::AxisOrientation::Y:
            multiAxes = &m_multiAxesY;
            break;
        case QAbstract3DAxis::AxisOrientation::Z:
            multiAxes = &m_multiAxesZ;
            break;
        case QAbstract3DAxis::AxisOrientation::None:
            return MultiAxis{};
            break;
    }

    MultiAxis multiAxis;
    bool found = false;
    for (qsizetype i = 0; i < multiAxes->count(); i++) {
        MultiAxis axis = multiAxes->at(i);
        if (axis.seriesIndex == axisIndex) {
            multiAxis = axis;
            found = true;
        }
    }

    if (!found) {
        multiAxis = createMultiAxis(axisIndex);
        multiAxes->append(multiAxis);
    }

    return multiAxis;
}

QQuickGraphsItem::MultiAxis QQuickGraphsItem::createMultiAxis(qsizetype axisIndex)
{
    MultiAxis axis;

    axis.repeater = createRepeater(graphNode());
    axis.repeater->setParent(graphNode());
    axis.repeater->setParentItem(graphNode());
    axis.repeater->setVisible(true);

    axis.delegateModel = new QQmlComponent(qmlEngine(this), (QStringLiteral(":/axis/AxisLabel")));
    axis.delegateModel->setParent(this);
    axis.repeater->setDelegate(axis.delegateModel);

    axis.titleLabel = createTitleLabel(graphNode());
    axis.titleLabel->setVisible(true);

    axis.grid = new QQuick3DModel(graphNode());
    axis.grid->setCastsShadows(false);
    axis.grid->setReceivesShadows(false);
    auto gridGeometry = new QQuick3DGeometry(axis.grid);
    gridGeometry->setStride(sizeof(QVector3D));
    gridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    gridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                               0,
                               QQuick3DGeometry::Attribute::F32Type);
    axis.grid->setGeometry(gridGeometry);
    QQmlListReference gridMaterialRef(axis.grid, "materials");
    auto gridMaterial = new QQuick3DPrincipledMaterial(axis.grid);
    gridMaterial->setLighting(QQuick3DPrincipledMaterial::Lighting::NoLighting);
    gridMaterial->setCullMode(QQuick3DMaterial::CullMode::BackFaceCulling);
    gridMaterial->setBaseColor(theme()->grid().mainColor());
    gridMaterialRef.append(gridMaterial);

    axis.seriesIndex = axisIndex;

    return axis;
}

void QQuickGraphsItem::updateMultiAxisGrid(qsizetype axisIndex, QAbstract3DAxis *axis)
{
    MultiAxis multiAxis = getMultiAxis(axisIndex, axis);
    if (multiAxis.seriesIndex == -1) // uninitalized axis
        return;
    QQuick3DModel *model = multiAxis.grid;
    QQuick3DRepeater *repeater = multiAxis.repeater;

    QValue3DAxis *valueAxis = qobject_cast<QValue3DAxis *>(axis);
    QVector3D backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    float tickLength = 0.1f;

    const float pointSize = theme()->labelFont().pointSizeF();
    const float textPadding = pointSize * .5f;
    float labelsMaxWidth = float(findLabelsMaxWidth(axis->labels())) + textPadding;
    const float baseSize = 25.0f;
    const float relativePointSize = theme()->labelFont().pointSizeF() / baseSize;

    float labelOffset;
    if (axis->isScaleLabelsByCount()) {
        QFontMetrics fm(theme()->labelFont());
        const float labelHeight = fm.height() + textPadding;
        float scale = backgroundScale.x() - m_backgroundScaleMargin.x();
        float relativeScale = scale / axis->labels().count();
        float fontRatio = labelsMaxWidth / labelHeight;
        QVector3D fontScaled = QVector3D(0.01f * fontRatio * relativePointSize * relativeScale,
                                     0.01f * relativePointSize * relativeScale,
                                     0.01f) * axis->labelSize();
        labelOffset = fontScaled.y() * 110.0f;
    } else {
        labelOffset = labelAdjustment(labelsMaxWidth * axis->labelSize());
    }

    QList<QVector3D> vertices;

    if (axis->orientation() == QAbstract3DAxis::AxisOrientation::X) {
        if (axis->type() == QAbstract3DAxis::AxisType::Value) {
            if (m_isPolar) {
                //Use one label less since the graph is cyclical
                int labelCount = repeater->count() - 1;
                float yPos = -backgroundScale.y() + 0.01;

                QVector3D firstLabelPos = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();
                firstLabelPos.setY(0);
                float polarRadius = firstLabelPos.length() - 0.1f;

                const int ringSegments = 64;
                float angleStep = (float(M_PI) * 2.0f / float(ringSegments));
                for (int i = 0; i < ringSegments; i++) {
                    QVector3D pos0 = QVector3D(polarRadius * qCos(angleStep * i),
                        yPos,
                        polarRadius * qSin(angleStep * i));

                    QVector3D pos1 = QVector3D(polarRadius * qCos(angleStep * (i + 1)),
                        yPos,
                        polarRadius * qSin(angleStep * (i + 1)));

                    vertices.append(pos0);
                    vertices.append(pos1);
                }

                for (int i = 0; i < labelCount; i++) {
                    float tickAngle = M_PI + (M_PI * 2 / labelCount) * i;
                    QVector3D tickStart = QVector3D(qSin(tickAngle) * polarRadius, yPos, qCos(tickAngle) * polarRadius);
                    QVector3D tickEnd = QVector3D(
                        qSin(tickAngle) * (polarRadius - tickLength), yPos,
                        qCos(tickAngle) * (polarRadius - tickLength));
                    vertices.append(tickStart);
                    vertices.append(tickEnd);

                    int subSegments = valueAxis->subSegmentCount();
                    float angleStep = (M_PI * 2 / labelCount) / (subSegments + 1);

                    for (int j = 1; j <= subSegments; j++) {
                        QVector3D pos0 = QVector3D(
                            polarRadius * qSin(tickAngle + angleStep * j), yPos,
                            polarRadius * qCos(tickAngle + angleStep * j));
                        QVector3D pos1 = QVector3D(
                            (polarRadius - tickLength * 0.5f) * qSin(tickAngle + angleStep * j), yPos,
                            (polarRadius - tickLength * 0.5f) * qCos(tickAngle + angleStep * j));

                        vertices.append(pos0);
                        vertices.append(pos1);
                    }
                }
            } else {
                //Use label positions as reference
                QVector3D posStart = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();
                QVector3D posEnd = static_cast<QQuick3DModel *>(repeater->objectAt(repeater->count() - 1))->position();

                QVector3D zOffset = m_zFlipped? QVector3D(0.0f,0.0f,labelOffset) : QVector3D(0.0f,0.0f,-labelOffset);

                posStart += zOffset;
                posEnd += zOffset;

                vertices.append(posStart);
                vertices.append(posEnd);

                for (int i = 0; i < repeater->count(); i++) {
                    QVector3D pos0 = static_cast<QQuick3DModel *>(repeater->objectAt(i))->position();
                    pos0 += zOffset;
                    float tick = m_zFlipped? tickLength : -tickLength;
                    QVector3D pos1 = pos0 + QVector3D(0, 0, tick);

                    vertices.append(pos0);
                    vertices.append(pos1);

                    if (i < (repeater->count() - 1)) {
                        QVector3D nextPos = static_cast<QQuick3DModel *>(repeater->objectAt(i + 1))->position();
                        float tickRange =  nextPos.x() - pos0.x();
                        float nextTick = tickRange / (valueAxis->subSegmentCount() + 1);
                        for (int j = 0; j < valueAxis->subSegmentCount(); j++) {
                            QVector3D subPos0 = pos0 + QVector3D((nextTick * (j + 1)), 0, 0);
                            QVector3D subPos1 = subPos0 + QVector3D(0,0, tick * 0.5f);
                            vertices.append(subPos0);
                            vertices.append(subPos1);
                        }
                    }
                }
            }
        } else if (axis->type() == QAbstract3DAxis::AxisType::Category) {
            if (repeater->count() < 1)
                return;
            float stepSize = backgroundScale.x() * 2.0f / repeater->count();
            float startPosX = -scaleWithBackground().x();

            QVector3D startPos = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();

            QVector3D zOffset = m_zFlipped? QVector3D(0.0f,0.0f,labelOffset) : QVector3D(0.0f,0.0f,-labelOffset);
            startPos += zOffset;

            startPos.setX(startPosX);
            QVector3D endPos = startPos + QVector3D(backgroundScale.x() * 2.0f, 0.0f, 0.0f);
            vertices.append(startPos);
            vertices.append(endPos);
            float tick = m_zFlipped? tickLength : -tickLength;

            for (int i = 0; i < repeater->count() + 1; i++) {
                QVector3D pos0 = startPos + QVector3D(stepSize * i, 0.0f, 0.0f);
                QVector3D pos1 = pos0 + QVector3D(0.0f, 0.0f, tick);
                vertices.append(pos0);
                vertices.append(pos1);
            }
        }
    } else if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Y) {

        QVector3D posStart1 = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();
        QVector3D posEnd1 = static_cast<QQuick3DModel *>(repeater->objectAt(repeater->count() / 2 - 1))->position();

        QVector3D posStart2 = static_cast<QQuick3DModel *>(repeater->objectAt(repeater->count() / 2))->position();
        QVector3D posEnd2 = static_cast<QQuick3DModel *>(repeater->objectAt(repeater->count() - 1))->position();

        QVector3D xOffset = m_xFlipped? QVector3D(labelOffset,0.0f,0.0f) : QVector3D(-labelOffset,0.0f,0.0f);
        QVector3D zOffset = m_zFlipped? QVector3D(0.0f,0.0f,labelOffset) : QVector3D(0.0f,0.0f,-labelOffset);

        posStart1 += zOffset;
        posEnd1 += zOffset;

        posStart2 += xOffset;
        posEnd2 += xOffset;

        vertices.append(posStart1);
        vertices.append(posEnd1);

        vertices.append(posStart2);
        vertices.append(posEnd2);

        for (int i = 0; i < repeater->count(); i++) {
            QVector3D pos0 = static_cast<QQuick3DModel *>(repeater->objectAt(i))->position();

            QVector3D pos1;
            float tick;
            if (i < repeater->count() / 2) {
                pos0 += zOffset;
                tick = m_zFlipped? tickLength : -tickLength;
                pos1 = pos0 + QVector3D(0.0f, 0.0f, tick);
            }
            else {
                tick = m_xFlipped? tickLength : -tickLength;
                pos0 += xOffset;
                pos1 = pos0 + QVector3D(tick, 0.0f, 0.0f);
            }

            vertices.append(pos0);
            vertices.append(pos1);

            if (i < (repeater->count() - 1) ) {
                QVector3D nextPos = static_cast<QQuick3DModel *>(repeater->objectAt(i + 1))->position();
                float tickRange =  nextPos.y() - pos0.y();
                float nextTick = tickRange / (valueAxis->subSegmentCount() + 1);
                for (int j = 0; j < valueAxis->subSegmentCount(); j++) {
                    QVector3D subPos0 = pos0 + QVector3D(0, (nextTick * (j + 1)), 0);
                    QVector3D subPos1;
                    if (i < repeater->count() / 2)
                        subPos1 = subPos0 + QVector3D(0,0, tick * 0.5f);
                    else
                        subPos1 = subPos0 + QVector3D(tick * 0.5f, 0, 0);
                    vertices.append(subPos0);
                    vertices.append(subPos1);
                }
            }
        }
    } else if (axis->orientation() == QAbstract3DAxis::AxisOrientation::Z) {
        //Use label positions as reference
        if (axis->type() == QAbstract3DAxis::AxisType::Value) {
            QVector3D posStart = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();
            QVector3D posEnd = static_cast<QQuick3DModel *>(repeater->objectAt(repeater->count() - 1))->position();

            QVector3D xOffset = m_xFlipped? QVector3D(labelOffset,0.0f,0.0f) : QVector3D(-labelOffset,0.0f,0.0f);

            posStart += xOffset;
            posEnd += xOffset;

            vertices.append(posStart);
            vertices.append(posEnd);

            for (int i = 0; i < repeater->count(); i++) {
                QVector3D pos0 = static_cast<QQuick3DModel *>(repeater->objectAt(i))->position();
                pos0 += xOffset;
                float tick = m_xFlipped? tickLength : -tickLength;
                QVector3D pos1 = pos0 + QVector3D(tick, 0, 0);

                vertices.append(pos0);
                vertices.append(pos1);

                if (i < (repeater->count() - 1)) {
                    QVector3D nextPos = static_cast<QQuick3DModel *>(repeater->objectAt(i + 1))->position();
                    float tickRange =  nextPos.z() - pos0.z();
                    float nextTick = tickRange / (valueAxis->subSegmentCount() + 1);
                    for (int j = 0; j < valueAxis->subSegmentCount(); j++) {
                        QVector3D subPos0 = pos0 + QVector3D(0, 0, (nextTick * (j + 1)));
                        QVector3D subPos1 = subPos0 + QVector3D(tick * 0.5f, 0, 0);
                        vertices.append(subPos0);
                        vertices.append(subPos1);
                    }
                }
            }
        } else if (axis->type() == QAbstract3DAxis::AxisType::Category) {
            if (repeater->count() < 1)
                return;
            float stepSize = backgroundScale.z() * 2.0f / repeater->count();
            float startPosZ = -scaleWithBackground().z();

            QVector3D startPos = static_cast<QQuick3DModel *>(repeater->objectAt(0))->position();
            QVector3D xOffset = m_xFlipped? QVector3D(labelOffset,0.0f,0.0f) : QVector3D(-labelOffset,0.0f,0.0f);
            startPos += xOffset;
            startPos.setZ(startPosZ);
            QVector3D endPos = startPos + QVector3D(0.0f, 0.0f, backgroundScale.z() * 2.0f);
            vertices.append(startPos);
            vertices.append(endPos);
            float tick = m_xFlipped? tickLength : -tickLength;

            for (int i = 0; i < repeater->count() + 1; i++) {
                QVector3D pos0 = startPos + QVector3D(0.0f, 0.0f, stepSize * i);
                QVector3D pos1 = pos0 + QVector3D(tick, 0.0f, 0.0f);
                vertices.append(pos0);
                vertices.append(pos1);
            }
        }
    }


    auto geometry = model->geometry();
    QByteArray vertexBuffer(reinterpret_cast<char *>(vertices.data()),
                            vertices.size() * sizeof(QVector3D));
    geometry->setVertexData(vertexBuffer);
    geometry->update();
}

void QQuickGraphsItem::positionAndScaleLine(QQuick3DNode *lineNode,
                                            QVector3D scale,
                                            QVector3D position)
{
    lineNode->setScale(scale);
    lineNode->setPosition(position);
}

QVector3D QQuickGraphsItem::graphPositionAt(const QPoint point)
{
    auto result = pick(point.x(), point.y());
    QVector3D position = QVector3D();
    if (result.objectHit())
        position = result.scenePosition();

    return position;
}

void QQuickGraphsItem::updateShadowQuality(QtGraphs3D::ShadowQuality quality)
{
    if (quality != QtGraphs3D::ShadowQuality::None) {
        light()->setCastsShadow(true);
        light()->setShadowFactor(25.f);

        QQuick3DAbstractLight::QSSGShadowMapQuality shadowMapQuality;
        switch (quality) {
        case QtGraphs3D::ShadowQuality::Low:
        case QtGraphs3D::ShadowQuality::SoftLow:
            shadowMapQuality = QQuick3DAbstractLight::QSSGShadowMapQuality::ShadowMapQualityMedium;
            break;
        case QtGraphs3D::ShadowQuality::Medium:
        case QtGraphs3D::ShadowQuality::SoftMedium:
            shadowMapQuality = QQuick3DAbstractLight::QSSGShadowMapQuality::ShadowMapQualityHigh;
            break;
        case QtGraphs3D::ShadowQuality::High:
        case QtGraphs3D::ShadowQuality::SoftHigh:
            shadowMapQuality = QQuick3DAbstractLight::QSSGShadowMapQuality::ShadowMapQualityVeryHigh;
            break;
        default:
            shadowMapQuality = QQuick3DAbstractLight::QSSGShadowMapQuality::ShadowMapQualityHigh;
            break;
        }
        light()->setShadowMapQuality(shadowMapQuality);
        if (quality >= QtGraphs3D::ShadowQuality::SoftLow)
            light()->setShadowFilter(10.f);
        else
            light()->setShadowFilter(2.f);
    } else {
        light()->setCastsShadow(false);
        light()->setShadowFactor(0.f);
    }
}

void QQuickGraphsItem::updateItemLabel(QVector3D position)
{
    if (m_customView)
        m_itemLabel->setParentItem(m_customView);

    if (m_labelPosition != position)
        m_labelPosition = position;

    // QVector3D pos2d = mapFrom3DScene(m_labelPosition * rootNode()->scale().z());
    QVector3D scenePos = rootNode()->mapPositionToScene(m_labelPosition);
    QVector3D pos2d = mapFrom3DScene(scenePos);
    if (m_customView)
        pos2d = m_customView->mapFrom3DScene(scenePos);

    int pointSize = theme()->labelFont().pointSize();
    float scale = m_labelScale.x() * ((-10.0f * pointSize) + 650.0f)
                  / (pos2d.z() / rootNode()->scale().z());
    scale = scale < 0 ? -scale : scale;
    if (m_sliceView && m_sliceView->isVisible())
        m_itemLabel->setScale(scale * .2f);
    else
        m_itemLabel->setScale(scale);
    pos2d.setX(pos2d.x() - (m_itemLabel->width() / 2.f));
    pos2d.setY(pos2d.y() - (m_itemLabel->height() / 2.f)
               - (m_itemLabel->height() * m_itemLabel->scale()));
    m_itemLabel->setPosition(pos2d.toPointF());
}

void QQuickGraphsItem::updateSliceItemLabel(const QString &label, QVector3D position)
{
    Q_UNUSED(position);

    QFontMetrics fm(theme()->labelFont());
    float textPadding = theme()->labelFont().pointSizeF() * .7f;
    float labelHeight = fm.height() + textPadding;
    float labelWidth = fm.horizontalAdvance(label) + textPadding;

    float pointSize = theme()->labelFont().pointSizeF();
    float scaleFactor = fontScaleFactor(pointSize) * pointSize;
    float fontRatio = labelWidth / labelHeight;

    QVector3D fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f);
    m_sliceItemLabel->setScale(fontScaled);
}

void QQuickGraphsItem::createVolumeMaterial(QCustom3DVolume *volume, Volume &volumeItem)
{
    if (volumeItem.texture)
        volumeItem.texture->deleteLater();
    volumeItem.texture = new QQuick3DTexture();
    auto texture = volumeItem.texture;

    texture->setParent(this);
    texture->setMinFilter(QQuick3DTexture::Filter::Nearest);
    texture->setMagFilter(QQuick3DTexture::Filter::Nearest);
    texture->setHorizontalTiling(QQuick3DTexture::TilingMode::ClampToEdge);
    texture->setVerticalTiling(QQuick3DTexture::TilingMode::ClampToEdge);

    if (volumeItem.textureData)
        volumeItem.textureData->deleteLater();
    volumeItem.textureData = new QQuick3DTextureData();
    auto textureData = volumeItem.textureData;

    int color8Bit = (volume->textureFormat() == QImage::Format_Indexed8) ? 1 : 0;

    textureData->setParent(texture);
    textureData->setParentItem(texture);
    textureData->setSize(QSize(volume->textureWidth(), volume->textureHeight()));
    textureData->setDepth(volume->textureDepth());
    if (color8Bit)
        textureData->setFormat(QQuick3DTextureData::R8);
    else
        textureData->setFormat(QQuick3DTextureData::RGBA8);
    textureData->setTextureData(
        QByteArray::fromRawData(reinterpret_cast<const char *>(volume->textureData()->constData()),
                                volume->textureData()->size()));
    texture->setTextureData(textureData);

    QObject::connect(volume, &QCustom3DVolume::textureDataChanged, this, [this, volume] {
        m_customVolumes[volume].updateTextureData = true;
    });

    if (color8Bit) {
        if (volumeItem.colorTexture)
            volumeItem.colorTexture->deleteLater();
        volumeItem.colorTexture = new QQuick3DTexture();
        auto colorTexture = volumeItem.colorTexture;

        colorTexture->setParent(this);
        colorTexture->setMinFilter(QQuick3DTexture::Filter::Nearest);
        colorTexture->setMagFilter(QQuick3DTexture::Filter::Nearest);
        colorTexture->setHorizontalTiling(QQuick3DTexture::TilingMode::ClampToEdge);
        colorTexture->setVerticalTiling(QQuick3DTexture::TilingMode::ClampToEdge);

        QByteArray colorTableBytes;
        const QList<QRgb> &colorTable = volume->colorTable();
        for (int i = 0; i < colorTable.size(); i++) {
            QRgb shifted = qRgba(qBlue(colorTable[i]),
                                 qGreen(colorTable[i]),
                                 qRed(colorTable[i]),
                                 qAlpha(colorTable[i]));
            colorTableBytes.append(
                QByteArray::fromRawData(reinterpret_cast<const char *>(&shifted), sizeof(shifted)));
        }

        if (volumeItem.colorTextureData)
            volumeItem.colorTextureData->deleteLater();
        volumeItem.colorTextureData = new QQuick3DTextureData();
        auto colorTextureData = volumeItem.colorTextureData;

        colorTextureData->setParent(colorTexture);
        colorTextureData->setParentItem(colorTexture);
        colorTextureData->setSize(QSize(int(volume->colorTable().size()), 1));
        colorTextureData->setFormat(QQuick3DTextureData::RGBA8);
        colorTextureData->setTextureData(colorTableBytes);
        colorTexture->setTextureData(colorTextureData);

        QObject::connect(volume, &QCustom3DVolume::colorTableChanged, this, [this, volume] {
            m_customVolumes[volume].updateColorTextureData = true;
        });
    }

    auto model = volumeItem.model;
    QQmlListReference materialsRef(model, "materials");

    QQuick3DCustomMaterial *material = nullptr;

    if (volume->drawSlices() && m_validVolumeSlice)
        material = createQmlCustomMaterial(QStringLiteral(":/materials/VolumeSliceMaterial"));
    else if (volume->useHighDefShader())
        material = createQmlCustomMaterial(QStringLiteral(":/materials/VolumeMaterial"));
    else
        material = createQmlCustomMaterial(QStringLiteral(":/materials/VolumeLowDefMaterial"));

    auto textureSamplerVariant = material->property("textureSampler");
    auto textureSampler = textureSamplerVariant.value<QQuick3DShaderUtilsTextureInput *>();
    textureSampler->setTexture(volumeItem.texture);

    if (color8Bit) {
        auto colorSamplerVariant = material->property("colorSampler");
        auto colorSampler = colorSamplerVariant.value<QQuick3DShaderUtilsTextureInput *>();
        colorSampler->setTexture(volumeItem.colorTexture);
    }

    material->setProperty("textureDimensions",
                          QVector3D(1.0f / float(volume->textureWidth()),
                                    1.0f / float(volume->textureHeight()),
                                    1.0f / float(volume->textureDepth())));

    materialsRef.append(material);

    volumeItem.useHighDefShader = volume->useHighDefShader();
    volumeItem.drawSlices = volume->drawSlices() && m_validVolumeSlice;
}

QQuick3DModel *QQuickGraphsItem::createSliceFrame(Volume &volumeItem)
{
    QQuick3DModel *model = new QQuick3DModel();
    model->setParent(volumeItem.model);
    model->setParentItem(volumeItem.model);
    model->setSource(QUrl(QStringLiteral("defaultMeshes/barMeshFull")));
    model->setScale(QVector3D(1, 1, 0.01f));
    model->setDepthBias(-100.0f);

    QQmlListReference materialsRef(model, "materials");
    QQuick3DCustomMaterial *material = createQmlCustomMaterial(
        QStringLiteral(":/materials/VolumeFrameMaterial"));
    material->setParent(model);
    material->setParentItem(model);
    material->setCullMode(QQuick3DMaterial::NoCulling);
    materialsRef.append(material);

    return model;
}

void QQuickGraphsItem::updateSliceFrameMaterials(QCustom3DVolume *volume, Volume &volumeItem)
{
    QQmlListReference materialsRefX(volumeItem.sliceFrameX, "materials");
    QQmlListReference materialsRefY(volumeItem.sliceFrameY, "materials");
    QQmlListReference materialsRefZ(volumeItem.sliceFrameZ, "materials");

    QVector2D frameWidth;
    QVector3D frameScaling;

    frameScaling = QVector3D(volume->scaling().z()
                                 + (volume->scaling().z() * volume->sliceFrameGaps().z())
                                 + (volume->scaling().z() * volume->sliceFrameWidths().z()),
                             volume->scaling().y()
                                 + (volume->scaling().y() * volume->sliceFrameGaps().y())
                                 + (volume->scaling().y() * volume->sliceFrameWidths().y()),
                             volume->scaling().x() * volume->sliceFrameThicknesses().x());

    frameWidth = QVector2D(volume->scaling().z() * volume->sliceFrameWidths().z(),
                           volume->scaling().y() * volume->sliceFrameWidths().y());

    frameWidth.setX(1.0f - (frameWidth.x() / frameScaling.x()));
    frameWidth.setY(1.0f - (frameWidth.y() / frameScaling.y()));

    auto material = materialsRefX.at(0);
    material->setProperty("color", volume->sliceFrameColor());
    material->setProperty("sliceFrameWidth", frameWidth);

    frameScaling = QVector3D(volume->scaling().x()
                                 + (volume->scaling().x() * volume->sliceFrameGaps().x())
                                 + (volume->scaling().x() * volume->sliceFrameWidths().x()),
                             volume->scaling().z()
                                 + (volume->scaling().z() * volume->sliceFrameGaps().z())
                                 + (volume->scaling().z() * volume->sliceFrameWidths().z()),
                             volume->scaling().y() * volume->sliceFrameThicknesses().y());
    frameWidth = QVector2D(volume->scaling().x() * volume->sliceFrameWidths().x(),
                           volume->scaling().z() * volume->sliceFrameWidths().z());

    frameWidth.setX(1.0f - (frameWidth.x() / frameScaling.x()));
    frameWidth.setY(1.0f - (frameWidth.y() / frameScaling.y()));

    material = materialsRefY.at(0);
    material->setProperty("color", volume->sliceFrameColor());
    material->setProperty("sliceFrameWidth", frameWidth);

    frameScaling = QVector3D(volume->scaling().x()
                                 + (volume->scaling().x() * volume->sliceFrameGaps().x())
                                 + (volume->scaling().x() * volume->sliceFrameWidths().x()),
                             volume->scaling().y()
                                 + (volume->scaling().y() * volume->sliceFrameGaps().y())
                                 + (volume->scaling().y() * volume->sliceFrameWidths().y()),
                             volume->scaling().z() * volume->sliceFrameThicknesses().z());
    frameWidth = QVector2D(volume->scaling().x() * volume->sliceFrameWidths().x(),
                           volume->scaling().y() * volume->sliceFrameWidths().y());

    frameWidth.setX(1.0f - (frameWidth.x() / frameScaling.x()));
    frameWidth.setY(1.0f - (frameWidth.y() / frameScaling.y()));

    material = materialsRefZ.at(0);
    material->setProperty("color", volume->sliceFrameColor());
    material->setProperty("sliceFrameWidth", frameWidth);
}

void QQuickGraphsItem::updateCustomVolumes()
{
    Q_TRACE_SCOPE(QGraphs3DItemUpdateCustomVolumes);

    auto itemIterator = m_customItemList.constBegin();
    while (itemIterator != m_customItemList.constEnd()) {
        QCustom3DItem *item = itemIterator.key();
        QQuick3DModel *model = itemIterator.value();

        if (auto volume = qobject_cast<QCustom3DVolume *>(item)) {
            auto &&volumeItem = m_customVolumes[volume];

            QQmlListReference materialsRef(model, "materials");
            if (volumeItem.useHighDefShader != volume->useHighDefShader()) {
                materialsRef.clear();
                createVolumeMaterial(volume, volumeItem);
            }

            m_validVolumeSlice = volume->sliceIndexX() >= 0
                    || volume->sliceIndexY() >= 0
                    || volume->sliceIndexZ() >= 0;

            if (volumeItem.drawSlices != (volume->drawSlices() && m_validVolumeSlice)) {
                materialsRef.clear();
                createVolumeMaterial(volume, volumeItem);
            }

            QVector3D sliceIndices(
                (float(volume->sliceIndexX()) + 0.5f) / float(volume->textureWidth()) * 2.0 - 1.0,
                (float(volume->sliceIndexY()) + 0.5f) / float(volume->textureHeight()) * 2.0 - 1.0,
                (float(volume->sliceIndexZ()) + 0.5f) / float(volume->textureDepth()) * 2.0 - 1.0);

            if (volumeItem.drawSliceFrames != volume->drawSliceFrames()) {
                if (volume->drawSliceFrames()) {
                    volumeItem.sliceFrameX->setVisible(true);
                    volumeItem.sliceFrameY->setVisible(true);
                    volumeItem.sliceFrameZ->setVisible(true);

                    volumeItem.sliceFrameX->setRotation(QQuaternion::fromEulerAngles(0, 90, 0));
                    volumeItem.sliceFrameY->setRotation(QQuaternion::fromEulerAngles(90, 0, 0));

                    updateSliceFrameMaterials(volume, volumeItem);
                } else {
                    volumeItem.sliceFrameX->setVisible(false);
                    volumeItem.sliceFrameY->setVisible(false);
                    volumeItem.sliceFrameZ->setVisible(false);
                }
                volumeItem.drawSliceFrames = volume->drawSliceFrames();
            }

            auto material = materialsRef.at(0);
            QVector3D minBounds(-1, 1, 1);
            QVector3D maxBounds(1, -1, -1);
            QVector3D translation(0, 0, 0);
            QVector3D scaling(1, 1, 1);

            model->setVisible(volume->isVisible());
            if (!volume->isScalingAbsolute() && !volume->isPositionAbsolute()) {

                QVector3D pos = volume->position();
                QVector3D scale = volume->scaling() / 2;

                QVector3D minGraphBounds(pos.x() - scale.x(),
                                    pos.y() - scale.y(),
                                    pos.z() + scale.z());
                QVector3D maxGraphBounds(pos.x() + scale.x(),
                                    pos.y() + scale.y(),
                                    pos.z() - scale.z());

                QVector3D minCorner = graphPosToAbsolute(minGraphBounds);
                QVector3D maxCorner = graphPosToAbsolute(maxGraphBounds);

                scale = QVector3D(qAbs(maxCorner.x() - minCorner.x()),
                                  qAbs(maxCorner.y() - minCorner.y()),
                                  qAbs(maxCorner.z() - minCorner.z())) / 2.0f;

                const QVector3D mScale = scaleWithBackground();
                const QVector3D itemRange = maxCorner - minCorner;
                if (minCorner.x() < -mScale.x())
                    minBounds.setX(-1.0f + (2.0f * qAbs(minCorner.x() + mScale.x()) / itemRange.x()));
                if (minCorner.y() < -mScale.y())
                    minBounds.setY(-(-1.0f + (2.0f * qAbs(minCorner.y() + mScale.y()) / itemRange.y())));
                if (minCorner.z() < -mScale.z())
                    minBounds.setZ(-(-1.0f + (2.0f * qAbs(minCorner.z() + mScale.z()) / itemRange.z())));

                if (maxCorner.x() > mScale.x())
                    maxBounds.setX(1.0f - (2.0f * qAbs(maxCorner.x() - mScale.x()) / itemRange.x()));
                if (maxCorner.y() > mScale.y())
                    maxBounds.setY(-(1.0f - (2.0f * qAbs(maxCorner.y() - mScale.y()) / itemRange.y())));
                if (maxCorner.z() > mScale.z())
                    maxBounds.setZ(-(1.0f - (2.0f * qAbs(maxCorner.z() - mScale.z()) / itemRange.z())));

                QVector3D minBoundsNorm = minBounds;
                QVector3D maxBoundsNorm = maxBounds;

                minBoundsNorm.setY(-minBoundsNorm.y());
                minBoundsNorm.setZ(-minBoundsNorm.z());
                minBoundsNorm = 0.5f * (minBoundsNorm + QVector3D(1,1,1));

                maxBoundsNorm.setY(-maxBoundsNorm.y());
                maxBoundsNorm.setZ(-maxBoundsNorm.z());
                maxBoundsNorm = 0.5f * (maxBoundsNorm + QVector3D(1,1,1));

                QVector3D adjScaling = scale * (maxBoundsNorm - minBoundsNorm);
                model->setScale(adjScaling);

                QVector3D adjPos = volume->position();
                QVector3D dataExtents = (maxGraphBounds - minGraphBounds) / 2.0f;

                adjPos = adjPos + (dataExtents * minBoundsNorm)
                        - (dataExtents * (QVector3D(1,1,1) - maxBoundsNorm));
                adjPos = graphPosToAbsolute(adjPos);
                model->setPosition(adjPos);
            } else {
                model->setScale(volume->scaling());
            }
            model->setRotation(volume->rotation());

            material->setProperty("minBounds", minBounds);
            material->setProperty("maxBounds", maxBounds);

            if (volume->drawSlices())
                material->setProperty("volumeSliceIndices", sliceIndices);

            if (volume->drawSliceFrames()) {
                float sliceFrameX = sliceIndices.x();
                float sliceFrameY = sliceIndices.y();
                float sliceFrameZ = sliceIndices.z();
                if (volume->sliceIndexX() >= 0 && scaling.x() > 0)
                    sliceFrameX = (sliceFrameX + translation.x()) / scaling.x();
                if (volume->sliceIndexY() >= 0 && scaling.y() > 0)
                    sliceFrameY = (sliceFrameY - translation.y()) / scaling.y();
                if (volume->sliceIndexZ() >= 0 && scaling.z() > 0)
                    sliceFrameZ = (sliceFrameZ + translation.z()) / scaling.z();

                if (sliceFrameX < -1 || sliceFrameX > 1)
                    volumeItem.sliceFrameX->setVisible(false);
                else
                    volumeItem.sliceFrameX->setVisible(true);

                if (sliceFrameY < -1 || sliceFrameY > 1)
                    volumeItem.sliceFrameY->setVisible(false);
                else
                    volumeItem.sliceFrameY->setVisible(true);

                if (sliceFrameZ < -1 || sliceFrameZ > 1)
                    volumeItem.sliceFrameZ->setVisible(false);
                else
                    volumeItem.sliceFrameZ->setVisible(true);

                volumeItem.sliceFrameX->setX(sliceFrameX);
                volumeItem.sliceFrameY->setY(-sliceFrameY);
                volumeItem.sliceFrameZ->setZ(-sliceFrameZ);
            }

            material->setProperty("alphaMultiplier", volume->alphaMultiplier());
            material->setProperty("preserveOpacity", volume->preserveOpacity());
            material->setProperty("useOrtho", isOrthoProjection());

            int sampleCount = volume->textureWidth() + volume->textureHeight()
                              + volume->textureDepth();
            material->setProperty("sampleCount", sampleCount);

            int color8Bit = (volume->textureFormat() == QImage::Format_Indexed8) ? 1 : 0;
            material->setProperty("color8Bit", color8Bit);

            if (volumeItem.updateTextureData) {
                auto textureData = volumeItem.textureData;
                textureData->setSize(QSize(volume->textureWidth(), volume->textureHeight()));
                textureData->setDepth(volume->textureDepth());

                if (color8Bit)
                    textureData->setFormat(QQuick3DTextureData::R8);
                else
                    textureData->setFormat(QQuick3DTextureData::RGBA8);

                textureData->setTextureData(
                    QByteArray::fromRawData(reinterpret_cast<const char *>(
                                                volume->textureData()->constData()),
                                            volume->textureData()->size()));

                material->setProperty("textureDimensions",
                                      QVector3D(1.0f / float(volume->textureWidth()),
                                                1.0f / float(volume->textureHeight()),
                                                1.0f / float(volume->textureDepth())));

                volumeItem.updateTextureData = false;
            }

            if (volumeItem.updateColorTextureData) {
                auto colorTextureData = volumeItem.colorTextureData;
                QByteArray colorTableBytes;
                const QList<QRgb> &colorTable = volume->colorTable();
                for (int i = 0; i < colorTable.size(); i++) {
                    QRgb shifted = qRgba(qBlue(colorTable[i]),
                                         qGreen(colorTable[i]),
                                         qRed(colorTable[i]),
                                         qAlpha(colorTable[i]));
                    colorTableBytes.append(
                        QByteArray::fromRawData(reinterpret_cast<const char *>(&shifted),
                                                sizeof(shifted)));
                }
                colorTextureData->setTextureData(colorTableBytes);
            }
        }
        ++itemIterator;
    }
}

void QQuickGraphsItem::updateAxisRange(float min, float max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
}

void QQuickGraphsItem::updateAxisReversed(bool enable)
{
    Q_UNUSED(enable);
}

int QQuickGraphsItem::findLabelsMaxWidth(const QStringList &labels)
{
    int labelWidth = 0;
    QFontMetrics labelFM(theme()->labelFont());

    for (const auto &label : std::as_const(labels)) {
        auto width = labelFM.horizontalAdvance(label);
        if (labelWidth < width)
            labelWidth = width;
    }
    return labelWidth;
}

QVector3D QQuickGraphsItem::calculateCategoryLabelPosition(QAbstract3DAxis *axis,
                                                           QVector3D labelPosition,
                                                           int index)
{
    Q_UNUSED(axis);
    Q_UNUSED(index);
    return labelPosition;
}

float QQuickGraphsItem::calculateCategoryGridLinePosition(QAbstract3DAxis *axis, int index)
{
    Q_UNUSED(axis);
    Q_UNUSED(index);
    return 0.0f;
}

float QQuickGraphsItem::calculatePolarBackgroundMargin()
{
    // Check each extents of each angular label
    // Calculate angular position
    auto valueAxisX = static_cast<QValue3DAxis *>(axisX());
    auto labelPositions = const_cast<QList<float> &>(valueAxisX->formatter()->labelPositions());
    float actualLabelHeight = m_fontScaled.y() * 2.0f; // All labels are same height
    float maxNeededMargin = 0.0f;

    // Axis title needs to be accounted for
    if (valueAxisX->isTitleVisible())
        maxNeededMargin = 2.0f * actualLabelHeight + 3.0f * labelMargin();

    for (int label = 0; label < labelPositions.size(); label++) {
        QSizeF labelSize{m_fontScaled.x(), m_fontScaled.z()};
        float actualLabelWidth = actualLabelHeight / labelSize.height() * labelSize.width();
        float labelPosition = labelPositions.at(label);
        qreal angle = labelPosition * M_PI * 2.0;
        float x = qAbs((m_polarRadius + labelMargin()) * float(qSin(angle))) + actualLabelWidth
                  - m_polarRadius + labelMargin();
        float z = qAbs(-(m_polarRadius + labelMargin()) * float(qCos(angle))) + actualLabelHeight
                  - m_polarRadius + labelMargin();
        float neededMargin = qMax(x, z);
        maxNeededMargin = qMax(maxNeededMargin, neededMargin);
    }

    maxNeededMargin *= 0.2f;
    return maxNeededMargin;
}

void QQuickGraphsItem::updateXTitle(QVector3D labelRotation,
                                    QVector3D labelTrans,
                                    const QQuaternion &totalRotation,
                                    float labelsMaxWidth,
                                    QVector3D scale,
                                    QAbstract3DAxis *axis,
                                    QQuick3DNode *label)
{

    if (axis == nullptr)
        axis = axisX();

    QFont font = theme()->axisXLabelFont() == QFont() ? theme()->labelFont() : theme()->axisXLabelFont();
    float pointSize = font.pointSizeF();
    float textPadding = pointSize * .5f;
    QFontMetrics fm(font);
    float height = fm.height() + textPadding;
    float width = fm.horizontalAdvance(axis->title()) + textPadding;

    float titleOffset;

    bool radial = false;
    if (radial)
        titleOffset = -2.0f * (m_labelMargin + scale.y());
    else
        titleOffset = 2.0f * m_labelMargin + (labelsMaxWidth * scale.y());

    float zRotation = 0.0f;
    float yRotation = 0.0f;
    float xRotation = -90.0f + labelRotation.z();
    float offsetRotation = labelRotation.z();
    float extraRotation = -90.0f;
    if (m_yFlipped) {
        zRotation = 180.0f;
        if (m_zFlipped) {
            titleOffset = -titleOffset;
            if (m_xFlipped) {
                offsetRotation = -offsetRotation;
                extraRotation = -extraRotation;
            } else {
                xRotation = -90.0f - labelRotation.z();
            }
        } else {
            yRotation = 180.0f;
            if (m_xFlipped) {
                offsetRotation = -offsetRotation;
                xRotation = -90.0f - labelRotation.z();
            } else {
                extraRotation = -extraRotation;
            }
        }
    } else {
        if (m_zFlipped) {
            titleOffset = -titleOffset;
            if (m_xFlipped) {
                offsetRotation = -offsetRotation;
            } else {
                xRotation = -90.0f - labelRotation.z();
                extraRotation = -extraRotation;
            }
            yRotation = 180.0f;
            if (m_yFlipped) {
                extraRotation = -extraRotation;
                if (m_xFlipped)
                    xRotation = 90.0f + labelRotation.z();
                else
                    xRotation = 90.0f - labelRotation.z();
            }
        } else {
            if (m_xFlipped) {
                offsetRotation = -offsetRotation;
                xRotation = -90.0f - labelRotation.z();
                extraRotation = -extraRotation;
            }
            if (m_yFlipped) {
                xRotation = 90.0f + labelRotation.z();
                extraRotation = -extraRotation;
                if (m_xFlipped)
                    xRotation = 90.0f - labelRotation.z();
            }
        }
    }

    if (offsetRotation == 180.0f || offsetRotation == -180.0f)
        offsetRotation = 0.0f;

    QQuaternion offsetRotator = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, offsetRotation);
    QVector3D titleOffsetVector = offsetRotator.rotatedVector(QVector3D(0.0f, 0.0f, titleOffset));
    titleOffsetVector.setX(axis->titleOffset() * scaleWithBackground().x());

    QQuaternion titleRotation;
    if (axis->isTitleFixed()) {
        titleRotation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, zRotation)
                        * QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, yRotation)
                        * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, xRotation);
    } else {
        titleRotation = totalRotation
                        * QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, extraRotation);
    }

    QVector3D titleScale = scale;
    titleScale.setX(titleScale.y() * width / height);


    if (label == nullptr)
        label = m_titleLabelX;

    label->setScale(titleScale);
    label->setPosition(labelTrans + titleOffsetVector);
    label->setRotation(titleRotation);
    label->setProperty("labelWidth", width);
    label->setProperty("labelHeight", height);
}

void QQuickGraphsItem::updateYTitle(QVector3D sideLabelRotation,
                                    QVector3D backLabelRotation,
                                    QVector3D sideLabelTrans,
                                    QVector3D backLabelTrans,
                                    const QQuaternion &totalSideRotation,
                                    const QQuaternion &totalBackRotation,
                                    float labelsMaxWidth,
                                    QVector3D scale,
                                    QAbstract3DAxis *axis,
                                    QQuick3DNode *label)
{
    if (axis == nullptr)
        axis = axisY();
    QFont font = theme()->axisYLabelFont() == QFont() ? theme()->labelFont() : theme()->axisYLabelFont();
    float pointSize = font.pointSizeF();
    float textPadding = pointSize * .5f;
    QFontMetrics fm(font);
    float height = fm.height() + textPadding;
    float width = fm.horizontalAdvance(axis->title()) + textPadding;

    float titleOffset = m_labelMargin + (labelsMaxWidth * scale.x());

    QQuaternion zRightAngleRotation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, 90.0f);
    float yRotation;
    QVector3D titleTrans;
    QQuaternion totalRotation;
    if (m_xFlipped != m_zFlipped) {
        yRotation = backLabelRotation.y();
        titleTrans = backLabelTrans;
        totalRotation = totalBackRotation;
    } else {
        yRotation = sideLabelRotation.y();
        titleTrans = sideLabelTrans;
        totalRotation = totalSideRotation;
    }
    titleTrans.setY(.0f);

    QQuaternion offsetRotator = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, yRotation);
    QVector3D titleOffsetVector = offsetRotator.rotatedVector(QVector3D(-titleOffset, 0.0f, 0.0f));
    titleOffsetVector.setY(axis->titleOffset() * scaleWithBackground().y());

    QQuaternion titleRotation;
    if (axis->isTitleFixed()) {
        titleRotation = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, yRotation)
                        * zRightAngleRotation;
    } else {
        titleRotation = totalRotation * zRightAngleRotation;
    }

    QVector3D titleScale = scale;
    titleScale.setX(titleScale.y() * width / height);

    if (label == nullptr)
        label = m_titleLabelY;

    label->setScale(titleScale);
    label->setPosition(titleTrans + titleOffsetVector);
    label->setRotation(titleRotation);
    label->setProperty("labelWidth", width);
    label->setProperty("labelHeight", height);
}

void QQuickGraphsItem::updateZTitle(QVector3D labelRotation,
                                    QVector3D labelTrans,
                                    const QQuaternion &totalRotation,
                                    float labelsMaxWidth,
                                    QVector3D scale,
                                    QAbstract3DAxis *axis,
                                    QQuick3DNode *label)
{

    if (axis == nullptr)
        axis = axisZ();
    QFont font = theme()->axisZLabelFont() == QFont() ? theme()->labelFont() : theme()->axisZLabelFont();
    float pointSize = font.pointSizeF();
    float textPadding = pointSize * .5f;
    QFontMetrics fm(font);
    float height = fm.height() + textPadding;
    float width = fm.horizontalAdvance(axis->title()) + textPadding;

    float titleOffset = m_labelMargin + (labelsMaxWidth * scale.x());

    float zRotation = labelRotation.z();
    float yRotation = -90.0f;
    float xRotation = -90.0f;
    float extraRotation = 90.0f;

    if (m_yFlipped) {
        xRotation = -xRotation;
        if (m_zFlipped) {
            if (m_xFlipped) {
                titleOffset = -titleOffset;
                zRotation = -zRotation;
                extraRotation = -extraRotation;
            } else {
                zRotation = -zRotation;
                yRotation = -yRotation;
            }
        } else {
            if (m_xFlipped) {
                titleOffset = -titleOffset;
            } else {
                extraRotation = -extraRotation;
                yRotation = -yRotation;
            }
        }
    } else {
        if (m_zFlipped) {
            zRotation = -zRotation;
            if (m_xFlipped) {
                titleOffset = -titleOffset;
            } else {
                extraRotation = -extraRotation;
                yRotation = -yRotation;
            }
        } else {
            if (m_xFlipped) {
                titleOffset = -titleOffset;
                extraRotation = -extraRotation;
            } else {
                yRotation = -yRotation;
            }
        }
        if (m_yFlipped) {
            xRotation = -xRotation;
            extraRotation = -extraRotation;
        }
    }

    float offsetRotation = zRotation;
    if (offsetRotation == 180.0f || offsetRotation == -180.0f)
        offsetRotation = 0.0f;

    QQuaternion offsetRotator = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, offsetRotation);
    QVector3D titleOffsetVector = offsetRotator.rotatedVector(QVector3D(titleOffset, 0.0f, 0.0f));
    titleOffsetVector.setZ(axis->titleOffset() * scaleWithBackground().z());

    QQuaternion titleRotation;
    if (axis->isTitleFixed()) {
        titleRotation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, zRotation)
                        * QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, yRotation)
                        * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, xRotation);
    } else {
        titleRotation = totalRotation
                        * QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, extraRotation);
    }

    if (label == nullptr)
        label = m_titleLabelZ;
    QVector3D titleScale = scale;
    titleScale.setX(titleScale.y() * width / height);

    label->setScale(titleScale);
    label->setPosition(labelTrans + titleOffsetVector);
    label->setRotation(titleRotation);
    label->setProperty("labelWidth", width);
    label->setProperty("labelHeight", height);
}

void QQuickGraphsItem::updateCamera()
{
    Q_TRACE(QGraphs3DItemUpdateCamera_entry);
    QVector3D lookingPosition = m_requestedTarget;

    const float scale = qMin(width(), height() * 1.6f);
    const float magnificationScaleFactor = 1.0f / 640.0f;
    const float magnification = scale * magnificationScaleFactor / rootNode()->scale().x();

    auto useOrtho = isOrthoProjection();
    if (useOrtho) {
        if (m_sliceView && m_sliceView->isVisible()) {
            m_oCamera->setVerticalMagnification(m_zoomLevel * .4f);
            m_oCamera->setHorizontalMagnification(m_zoomLevel * .4f);
        } else {
            m_oCamera->setVerticalMagnification(m_zoomLevel * magnification);
            m_oCamera->setHorizontalMagnification(m_zoomLevel * magnification);
        }
    }
    cameraTarget()->setPosition(lookingPosition);
    auto rotation = QVector3D(-m_yRotation, -m_xRotation, 0);
    cameraTarget()->setEulerRotation(rotation);
    float zoom = 720.f / m_zoomLevel;
    m_pCamera->setZ(zoom);
    updateCustomLabelsRotation();
    updateItemLabel(m_labelPosition);
    updateMultiAxis();
    Q_TRACE(QGraphs3DItemUpdateCamera_exit);
}

void QQuickGraphsItem::handleLabelCountChanged(QQuick3DRepeater *repeater, QColor axisLabelColor)
{
    changeLabelBackgroundColor(repeater, theme()->labelBackgroundColor());
    changeLabelBackgroundVisible(repeater, theme()->isLabelBackgroundVisible());
    changeLabelBorderVisible(repeater, theme()->isLabelBorderVisible());
    changeLabelTextColor(repeater, axisLabelColor);
    changeLabelFont(repeater, theme()->labelFont());

    if (m_sliceView) {
        changeLabelBackgroundColor(m_sliceHorizontalLabelRepeater, theme()->labelBackgroundColor());
        changeLabelBackgroundColor(m_sliceVerticalLabelRepeater, theme()->labelBackgroundColor());
        changeLabelBackgroundVisible(m_sliceHorizontalLabelRepeater,
                                     theme()->isLabelBackgroundVisible());
        changeLabelBackgroundVisible(m_sliceVerticalLabelRepeater,
                                     theme()->isLabelBackgroundVisible());
        changeLabelBorderVisible(m_sliceHorizontalLabelRepeater, theme()->isLabelBorderVisible());
        changeLabelBorderVisible(m_sliceVerticalLabelRepeater, theme()->isLabelBorderVisible());
        if (m_selectionMode == QtGraphs3D::SelectionFlag::Row)
            changeLabelTextColor(m_sliceHorizontalLabelRepeater, theme()->axisX().labelTextColor());
        else if (m_selectionMode == QtGraphs3D::SelectionFlag::Column)
            changeLabelTextColor(m_sliceHorizontalLabelRepeater, theme()->axisZ().labelTextColor());
        changeLabelTextColor(m_sliceVerticalLabelRepeater, theme()->axisY().labelTextColor());
        changeLabelFont(m_sliceHorizontalLabelRepeater, theme()->labelFont());
        changeLabelFont(m_sliceVerticalLabelRepeater, theme()->labelFont());
    }
}

void QQuickGraphsItem::updateCustomData()
{
    Q_TRACE_SCOPE(QGraphs3DItemUpdateCustomData);

    int maxX = axisX()->max();
    int minX = axisX()->min();
    int maxY = axisY()->max();
    int minY = axisY()->min();
    int maxZ = axisZ()->max();
    int minZ = axisZ()->min();

    auto labelIterator = m_customLabelList.constBegin();
    while (labelIterator != m_customLabelList.constEnd()) {
        QCustom3DLabel *label = labelIterator.key();
        QQuick3DNode *customLabel = labelIterator.value();

        QVector3D pos = label->position();
        // We incorrectly assumed label position to be normalized by default, when it in
        // reality is -1...1. Because of this we need to multiply the x and z by 2.
        // (QTBUG-131138)
        pos.setX(pos.x() * 2.f);
        pos.setZ(pos.z() * 2.f);
        if (!label->isPositionAbsolute()) {
            if (pos.x() < minX || pos.x() > maxX
                || pos.y() < minY || pos.y() > maxY
                || pos.z() < minZ || pos.z() > maxZ) {
                customLabel->setVisible(false);
                ++labelIterator;
                continue;
            }
            pos = graphPosToAbsolute(pos);
        }

        QFontMetrics fm(label->font());
        int width = fm.horizontalAdvance(label->text());
        int height = fm.height();
        customLabel->setProperty("labelWidth", width);
        customLabel->setProperty("labelHeight", height);
        customLabel->setPosition(pos);
        QQuaternion rotation = label->rotation();
        if (label->isFacingCamera())
            rotation = Utils::calculateRotation(QVector3D(-m_yRotation, -m_xRotation, 0));
        customLabel->setRotation(rotation);
        float pointSize = theme()->labelFont().pointSizeF();
        float scaleFactor = fontScaleFactor(pointSize) * pointSize;
        float fontRatio = float(height) / float(width);
        QVector3D fontScaled = QVector3D(scaleFactor / fontRatio, scaleFactor, 0.0f);
        customLabel->setScale(fontScaled);
        customLabel->setProperty("labelText", label->text());
        customLabel->setProperty("labelTextColor", label->textColor());
        customLabel->setProperty("labelFont", label->font());
        customLabel->setProperty("backgroundVisible", label->isBackgroundVisible());
        customLabel->setProperty("backgroundColor", label->backgroundColor());
        customLabel->setProperty("borderVisible", label->isBorderVisible());
        customLabel->setVisible(label->isVisible());

        ++labelIterator;
    }

    auto itemIterator = m_customItemList.constBegin();
    while (itemIterator != m_customItemList.constEnd()) {
        QCustom3DItem *item = itemIterator.key();
        QQuick3DModel *model = itemIterator.value();

        QVector3D pos = item->position();
        QList<QAbstract3DAxis *> axes{axisX(), axisY(), axisZ()};
        QList<float> bScales{scaleWithBackground().x(),
                    scaleWithBackground().y(),
                    scaleWithBackground().z()};
        if (!item->isPositionAbsolute()) {
            if (item->position().x() < minX || item->position().x() > maxX
                || item->position().y() < minY || item->position().y() > maxY
                || item->position().z() < minZ || item->position().z() > maxZ) {
                model->setVisible(false);
                ++itemIterator;
                continue;
            }
            pos = graphPosToAbsolute(pos);
        }
        model->setPosition(pos);

        if (!item->isScalingAbsolute()) {
            QList<float> iScales{item->scaling().x(), item->scaling().y(), item->scaling().z()};
            for (int i = 0; i < axes.count(); i++) {
                if (auto vAxis = static_cast<QValue3DAxis *>(axes.at(i))) {
                    float axisRange = vAxis->max() - vAxis->min();
                    float realRange = bScales.at(i);
                    float ratio = realRange / axisRange;
                    iScales[i] *= ratio;
                }
            }
            // We incorrectly assumed models to be scaled to 0...1 by default, when they in
            // reality are scaled to -1...1. Because of this we need to multiply the scale by 2
            // (QTBUG-126611)
            model->setScale(QVector3D(iScales.at(0), iScales.at(1), iScales.at(2)) * 2.f);
        } else {
            // We incorrectly assumed models to be scaled to 0...1 by default, when they in
            // reality are scaled to -1...1. Because of this we need to multiply the scale by 2
            // (QTBUG-126611)
            model->setScale(item->scaling() * 2.f);
        }

        if (auto volume = qobject_cast<QCustom3DVolume *>(item)) {
            if (!m_customVolumes.contains(volume)) {
                auto &&volumeItem = m_customVolumes[volume];

                volumeItem.model = model;
                model->setSource(QUrl(volume->meshFile()));

                volumeItem.useHighDefShader = volume->useHighDefShader();

                m_validVolumeSlice = volume->sliceIndexX() >= 0
                        || volume->sliceIndexY() >= 0
                        || volume->sliceIndexZ() >= 0;

                volumeItem.drawSlices = volume->drawSlices() && m_validVolumeSlice;

                createVolumeMaterial(volume, volumeItem);

                volumeItem.sliceFrameX = createSliceFrame(volumeItem);
                volumeItem.sliceFrameY = createSliceFrame(volumeItem);
                volumeItem.sliceFrameZ = createSliceFrame(volumeItem);

                if (volume->drawSliceFrames()) {
                    volumeItem.sliceFrameX->setVisible(true);
                    volumeItem.sliceFrameY->setVisible(true);
                    volumeItem.sliceFrameZ->setVisible(true);

                    QVector3D sliceIndices((float(volume->sliceIndexX()) + 0.5f)
                                                   / float(volume->textureWidth()) * 2.0
                                               - 1.0,
                                           (float(volume->sliceIndexY()) + 0.5f)
                                                   / float(volume->textureHeight()) * 2.0
                                               - 1.0,
                                           (float(volume->sliceIndexZ()) + 0.5f)
                                                   / float(volume->textureDepth()) * 2.0
                                               - 1.0);

                    volumeItem.sliceFrameX->setX(sliceIndices.x());
                    volumeItem.sliceFrameY->setY(-sliceIndices.y());
                    volumeItem.sliceFrameZ->setZ(-sliceIndices.z());

                    volumeItem.sliceFrameX->setRotation(QQuaternion::fromEulerAngles(0, 90, 0));
                    volumeItem.sliceFrameY->setRotation(QQuaternion::fromEulerAngles(90, 0, 0));

                    updateSliceFrameMaterials(volume, volumeItem);
                } else {
                    volumeItem.sliceFrameX->setVisible(false);
                    volumeItem.sliceFrameY->setVisible(false);
                    volumeItem.sliceFrameZ->setVisible(false);
                }
                volumeItem.drawSliceFrames = volume->drawSliceFrames();
                m_customItemList.insert(item, model);
            }
        } else {
            model->setSource(QUrl::fromLocalFile(item->meshFile()));
            QQmlListReference materialsRef(model, "materials");
            QQuick3DPrincipledMaterial *material = static_cast<QQuick3DPrincipledMaterial *>(
                materialsRef.at(0));
            QQuick3DTexture *texture = material->baseColorMap();
            if (!texture) {
                texture = new QQuick3DTexture();
                texture->setParent(model);
                texture->setParentItem(model);
                material->setBaseColorMap(texture);
            }
            if (!item->textureFile().isEmpty()) {
                texture->setSource(QUrl::fromLocalFile(item->textureFile()));
            } else {
                QImage textureImage = customTextureImage(item);
                textureImage.convertTo(QImage::Format_RGBA32FPx4);
                QQuick3DTextureData *textureData = texture->textureData();
                if (!textureData) {
                    textureData = new QQuick3DTextureData();
                    textureData->setParent(texture);
                    textureData->setParentItem(texture);
                    textureData->setFormat(QQuick3DTextureData::RGBA32F);
                    texture->setTextureData(textureData);
                }
                textureData->setSize(textureImage.size());
                textureData->setTextureData(
                    QByteArray(reinterpret_cast<const char *>(textureImage.bits()),
                               textureImage.sizeInBytes()));
            }

            if (!item->isRotationAbsolute()) {
                auto xValueAxis = qobject_cast<QValue3DAxis *>(axisX());
                auto yValueAxis = qobject_cast<QValue3DAxis *>(axisY());
                auto zValueAxis = qobject_cast<QValue3DAxis *>(axisZ());

                if (xValueAxis && yValueAxis && zValueAxis) {
                    QQuaternion itemRotation(item->rotation());

                    /*
                      the z axis is initally reversed in qtgraphs,
                      so we have to reverse the axis in charge
                      of it initally
                    */
                    itemRotation.setX(-itemRotation.x());

                    float sX = xValueAxis->reversed()? -1.0 : 1.0;
                    float sY = yValueAxis->reversed()? -1.0 : 1.0;
                    float sZ = zValueAxis->reversed()? -1.0 : 1.0;

                    float handedness = sX * sY * sZ;

                    if (handedness == 1.0f) {
                        itemRotation = {
                            itemRotation.scalar(),
                            itemRotation.x() * sX,
                            itemRotation.y() * sY,
                            itemRotation.z() * sZ,
                        };
                    } else {
                        itemRotation = {
                            item->rotation().scalar(),
                            -itemRotation.x() * sX,
                            -itemRotation.y() * sY,
                            -itemRotation.z() * sZ,
                        };
                    }

                    model->setRotation(itemRotation);
                } else {
                    model->setRotation(item->rotation());
                }
            } else {
                model->setRotation(item->rotation());
            }
            model->setVisible(item->isVisible());
        }
        ++itemIterator;
    }
}

void QQuickGraphsItem::updateCustomLabelsRotation()
{
    auto labelIterator = m_customLabelList.constBegin();
    while (labelIterator != m_customLabelList.constEnd()) {
        QCustom3DLabel *label = labelIterator.key();
        QQuick3DNode *customLabel = labelIterator.value();
        QQuaternion rotation = label->rotation();
        if (label->isFacingCamera())
            rotation = Utils::calculateRotation(QVector3D(-m_yRotation, -m_xRotation, 0));
        customLabel->setRotation(rotation);
        ++labelIterator;
    }
}

int QQuickGraphsItem::msaaSamples() const
{
    if (m_renderMode == QtGraphs3D::RenderingMode::Indirect)
        return m_samples;
    else
        return m_windowSamples;
}

void QQuickGraphsItem::setMsaaSamples(int samples)
{
    if (m_renderMode != QtGraphs3D::RenderingMode::Indirect) {
        qCWarning(lcProperties3D, "%s multisampling cannot be adjusted in this render mode",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else if (m_samples != samples) {
        m_samples = samples;
        setAntialiasing(m_samples > 0);
        auto sceneEnv = environment();
        sceneEnv->setAntialiasingMode(
            m_samples > 0 ? QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues::MSAA
                          : QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues::NoAA);
        switch (m_samples) {
        case 0:
            // no-op
            break;
        case 2:
            sceneEnv->setAntialiasingQuality(
                QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues::Medium);
            break;
        case 4:
            sceneEnv->setAntialiasingQuality(
                QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues::High);
            break;
        case 8:
            sceneEnv->setAntialiasingQuality(
                QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues::VeryHigh);
            break;
        default:
            qCWarning(lcProperties3D, "%s invalid multisampling sample number, using 4x instead",
                      qUtf8Printable(QLatin1String(__FUNCTION__)));
            sceneEnv->setAntialiasingQuality(
                QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues::High);
            m_samples = 4;
            break;
        }
        emit msaaSamplesChanged(m_samples);
        update();
    }
}

void QQuickGraphsItem::setParentNode(QQuick3DNode *node) {
    if (node) {
        m_parentNode = node;

        // find active sceneManager
        auto *p = node->parent();
        QQuick3DViewport *view = nullptr;
        while (p && !view) {
            view = qobject_cast<QQuick3DViewport *>(p);
            p = p->parent();
        }

        if (view) {
            m_customView = view;
            auto sceneManager = QQuick3DObjectPrivate::get(view->scene())->sceneManager;
            setParent(view);

            if (graphNode()) {
                graphNode()->setParent(view->parent());
                graphNode()->setParentItem(node);
            }

            connect(sceneManager.data(),
                    &QQuick3DSceneManager::windowChanged,
                    this,
                    &QQuickGraphsItem::handleWindowChanged);

            handleWindowChanged();
        }

    }
}

void QQuickGraphsItem::handleWindowChanged(/*QQuickWindow *window*/)
{

    QQuick3DSceneManager *manager = nullptr;
    if (m_customView)
        manager = QQuick3DObjectPrivate::get(m_customView->scene())->sceneManager;
    else
        manager = QQuick3DObjectPrivate::get(rootNode())->sceneManager;

    auto window = manager->window();
    checkWindowList(window);

    if (!window)
        return;

#if defined(Q_OS_MACOS)
    // Enable touch events for Mac touchpads
    typedef void (*EnableTouch)(QWindow *, bool);
    EnableTouch enableTouch = reinterpret_cast<EnableTouch>(
                QFunctionPointer(QGuiApplication::platformNativeInterface()
                                 ->nativeResourceFunctionForIntegration("registertouchwindow")));
    if (enableTouch)
        enableTouch(window, true);
#endif

    connect(window, &QObject::destroyed, this, &QQuickGraphsItem::windowDestroyed);

    int oldWindowSamples = m_windowSamples;
    m_windowSamples = window->format().samples();
    if (m_windowSamples < 0)
        m_windowSamples = 0;

    connect(window, &QQuickWindow::beforeSynchronizing, this, &QQuickGraphsItem::synchData);

    if (m_renderMode == QtGraphs3D::RenderingMode::DirectToBackground) {
        setAntialiasing(m_windowSamples > 0);
        if (m_windowSamples != oldWindowSamples)
            emit msaaSamplesChanged(m_windowSamples);
    }

    connect(this, &QQuickGraphsItem::needRender, window, &QQuickWindow::update);
    // Force camera update before rendering the first frame
    // to workaround a Quick3D device pixel ratio bug
    connect(window, &QQuickWindow::beforeRendering, this, [this, window]() {
        m_oCamera->setClipNear(0.001f);
        disconnect(window, &QQuickWindow::beforeRendering, this, nullptr);
    });
    updateWindowParameters();

#if defined(Q_OS_IOS)
    // Scenegraph render cycle in iOS sometimes misses update after
    // beforeSynchronizing signal. This ensures we don't end up displaying the
    // graph without any data, in case update is skipped after synchData.
    QTimer::singleShot(0, window, SLOT(update()));
#endif
}

void QQuickGraphsItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    // Do not cache primary subviewport geometry, as that will mess up window size
    if (!parentItem())
        return;
    m_cachedGeometry = parentItem()->boundingRect();
    updateWindowParameters();
}

void QQuickGraphsItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuick3DViewport::itemChange(change, value);
    updateWindowParameters();
}

void QQuickGraphsItem::updateWindowParameters()
{
    // Update the device pixel ratio, window size and bounding box
    QQuickWindow *win = window();
    if (win) {
        if (win->devicePixelRatio() != scene()->devicePixelRatio()) {
            scene()->setDevicePixelRatio(win->devicePixelRatio());
            win->update();
        }

        QSize windowSize;

        if (m_renderMode == QtGraphs3D::RenderingMode::DirectToBackground)
            windowSize = win->size();
        else
            windowSize = m_cachedGeometry.size().toSize();

        if (windowSize != scene()->d_func()->windowSize()) {
            scene()->d_func()->setWindowSize(windowSize);
            win->update();
        }

        resizeViewports(m_cachedGeometry.size());
    }
}

void QQuickGraphsItem::handleSelectionModeChange(QtGraphs3D::SelectionFlags mode)
{
    emit selectionModeChanged(mode);
}

void QQuickGraphsItem::handleShadowQualityChange(QtGraphs3D::ShadowQuality quality)
{
    emit shadowQualityChanged(quality);
}

void QQuickGraphsItem::handleSelectedElementChange(QtGraphs3D::ElementType type)
{
    m_clickedType = type;
    emit selectedElementChanged(type);
}

void QQuickGraphsItem::handleOptimizationHintChange(QtGraphs3D::OptimizationHint hint)
{
    Q_UNUSED(hint)
}

void QQuickGraphsItem::resizeViewports(QSizeF viewportSize)
{
    if (!viewportSize.isEmpty()) {
        scene()->d_func()->setViewport(
                    QRect(0.0f, 0.0f, viewportSize.width() + 0.5f, viewportSize.height() + 0.5f));
    }
}

void QQuickGraphsItem::checkWindowList(QQuickWindow *window)
{
    QQuickWindow *oldWindow = m_graphWindowList.value(this);
    m_graphWindowList[this] = window;

    if (oldWindow != window && oldWindow) {
        QObject::disconnect(oldWindow,
                            &QObject::destroyed,
                            this,
                            &QQuickGraphsItem::windowDestroyed);
        QObject::disconnect(oldWindow,
                            &QQuickWindow::beforeSynchronizing,
                            this,
                            &QQuickGraphsItem::synchData);
        QObject::disconnect(this, &QQuickGraphsItem::needRender, oldWindow, &QQuickWindow::update);
    }

    QList<QQuickWindow *> windowList;

    const auto keys = m_graphWindowList.keys();
    for (const auto &graph : keys) {
        if (graph->m_renderMode == QtGraphs3D::RenderingMode::DirectToBackground)
            windowList.append(m_graphWindowList.value(graph));
    }

    if (!window) {
        m_graphWindowList.remove(this);
        return;
    }
}

void QQuickGraphsItem::setMeasureFps(bool enable)
{
    if (m_measureFps == enable) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }

    m_measureFps = enable;
    if (enable) {
        QObject::connect(renderStats(),
                         &QQuick3DRenderStats::fpsChanged,
                         this,
                         &QQuickGraphsItem::handleFpsChanged);
        emitNeedRender();
    } else {
        QObject::disconnect(renderStats(), 0, this, 0);
    }
    emit measureFpsChanged(enable);
}

bool QQuickGraphsItem::measureFps() const
{
    return m_measureFps;
}

int QQuickGraphsItem::currentFps() const
{
    return m_currentFps;
}

void QQuickGraphsItem::setOrthoProjection(bool enable)
{
    if (enable == m_useOrthoProjection) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "graph is already using orthoProjection.";
        return;
    }
    m_useOrthoProjection = enable;
    m_changeTracker.projectionChanged = true;
    emit orthoProjectionChanged(m_useOrthoProjection);
    // If changed to ortho, disable shadows
    if (m_useOrthoProjection)
        doSetShadowQuality(QtGraphs3D::ShadowQuality::None);
    emitNeedRender();
}

bool QQuickGraphsItem::isOrthoProjection() const
{
    return m_useOrthoProjection;
}

QtGraphs3D::ElementType QQuickGraphsItem::selectedElement() const
{
    return m_clickedType;
}

void QQuickGraphsItem::setAspectRatio(qreal ratio)
{
    if (m_aspectRatio == ratio || ratio <= 0.0) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f or aspect ratio is 0.0f or smaller",
                qUtf8Printable(QLatin1String(__FUNCTION__)), ratio);
        return;
    }
    m_aspectRatio = ratio;
    m_changeTracker.aspectRatioChanged = true;
    emit aspectRatioChanged(m_aspectRatio);
    m_isDataDirty = true;
    emitNeedRender();
}

qreal QQuickGraphsItem::aspectRatio() const
{
    return m_aspectRatio;
}

void QQuickGraphsItem::setOptimizationHint(QtGraphs3D::OptimizationHint hint)
{
    if (hint != m_optimizationHint) {
        m_optimizationHint = hint;
        m_changeTracker.optimizationHintChanged = true;
        m_isDataDirty = true;
        handleOptimizationHintChange(m_optimizationHint);
        emit optimizationHintChanged(hint);
        emitNeedRender();
    } else {
        qCDebug(lcProperties3D) << qUtf8Printable(QLatin1String(__FUNCTION__))
            << "Value is already set to:" << hint;
    }
}

QtGraphs3D::OptimizationHint QQuickGraphsItem::optimizationHint() const
{
    return m_optimizationHint;
}

void QQuickGraphsItem::setPolar(bool enable)
{
    if (enable == m_isPolar) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << enable;
        return;
    }
    if (m_graphType == QAbstract3DSeries::SeriesType::Bar) {
        qCWarning(lcProperties3D, "%s polar type with bars is not supported.",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
    }
    m_isPolar = enable;
    m_changeTracker.polarChanged = true;
    setVerticalSegmentLine(!m_isPolar);
    m_isDataDirty = true;
    emit polarChanged(m_isPolar);
    emitNeedRender();
}

bool QQuickGraphsItem::isPolar() const
{
    return m_isPolar;
}

void QQuickGraphsItem::setLabelMargin(float margin)
{
    if (QtPrivate::fuzzyCompare(m_labelMargin, margin)) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), margin);
        return;
    }
    m_labelMargin = margin;
    m_changeTracker.labelMarginChanged = true;
    emit labelMarginChanged(m_labelMargin);
    emitNeedRender();
}

float QQuickGraphsItem::labelMargin() const
{
    return m_labelMargin;
}

void QQuickGraphsItem::setRadialLabelOffset(float offset)
{
    if (QtPrivate::fuzzyCompare(m_radialLabelOffset, offset)) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), offset);
        return;
    }
    m_radialLabelOffset = offset;
    m_changeTracker.radialLabelOffsetChanged = true;
    emit radialLabelOffsetChanged(m_radialLabelOffset);
    emitNeedRender();
}

float QQuickGraphsItem::radialLabelOffset() const
{
    return m_radialLabelOffset;
}

void QQuickGraphsItem::setHorizontalAspectRatio(qreal ratio)
{
    if (qFuzzyCompare(1 + m_horizontalAspectRatio, 1 + ratio) || ratio <= 0.0) {
        qCDebug(lcProperties3D, "%s invalid value or value is already set to: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), ratio);
        return;
    }
    m_horizontalAspectRatio = ratio;
    m_changeTracker.horizontalAspectRatioChanged = true;
    emit horizontalAspectRatioChanged(m_horizontalAspectRatio);
    m_isDataDirty = true;
    emitNeedRender();
}

qreal QQuickGraphsItem::horizontalAspectRatio() const
{
    return m_horizontalAspectRatio;
}

void QQuickGraphsItem::setLocale(const QLocale &locale)
{
    if (m_locale == locale) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << locale;
        return;
    }
    m_locale = locale;
    // Value axis formatters need to be updated
    QValue3DAxis *axis = qobject_cast<QValue3DAxis *>(m_axisX);
    if (axis)
        axis->formatter()->setLocale(m_locale);
    axis = qobject_cast<QValue3DAxis *>(m_axisY);
    if (axis)
        axis->formatter()->setLocale(m_locale);
    axis = qobject_cast<QValue3DAxis *>(m_axisZ);
    if (axis)
        axis->formatter()->setLocale(m_locale);
    emit localeChanged(m_locale);
}

QLocale QQuickGraphsItem::locale() const
{
    return m_locale;
}

QVector3D QQuickGraphsItem::queriedGraphPosition() const
{
    return m_queriedGraphPosition;
}

void QQuickGraphsItem::setMargin(qreal margin)
{
    if (m_margin == margin) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), margin);
        return;
    }
    m_margin = margin;
    m_changeTracker.marginChanged = true;
    emit marginChanged(margin);
    emitNeedRender();
}

qreal QQuickGraphsItem::margin() const
{
    return m_margin;
}

void QQuickGraphsItem::setCutoffMargin(qreal margin)
{
    if (m_cutoffMargin == margin) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), margin);
        return;
    }
    m_cutoffMargin = margin;
    m_isSeriesVisualsDirty = true;
    emit cutoffMarginChanged(margin);
    emitNeedRender();
}

qreal QQuickGraphsItem::cutoffMargin() const
{
    return m_cutoffMargin;
}

QQuick3DNode *QQuickGraphsItem::rootNode() const
{
    if (m_parentNode)
        return m_parentNode;
    else
        return QQuick3DViewport::scene();
}

void QQuickGraphsItem::changeLabelBackgroundColor(QQuick3DRepeater *repeater, QColor color)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("backgroundColor", color);
    }
}

void QQuickGraphsItem::changeLabelBackgroundVisible(QQuick3DRepeater *repeater, const bool &visible)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("backgroundVisible", visible);
    }
}

void QQuickGraphsItem::changeLabelBorderVisible(QQuick3DRepeater *repeater, const bool &visible)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("borderVisible", visible);
    }
}

void QQuickGraphsItem::changeLabelTextColor(QQuick3DRepeater *repeater, QColor color)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("labelTextColor", color);
    }
}

void QQuickGraphsItem::changeLabelFont(QQuick3DRepeater *repeater, const QFont &font)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("labelFont", font);
    }
}

void QQuickGraphsItem::changeLabelsVisible(QQuick3DRepeater *repeater, const bool &visible)
{
    int count = repeater->count();
    for (int i = 0; i < count; i++) {
        auto label = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        label->setProperty("visible", visible);
    }
}

void QQuickGraphsItem::changeGridLineColor(QQuick3DRepeater *repeater, QColor color)
{
    for (int i = 0; i < repeater->count(); i++) {
        auto lineNode = static_cast<QQuick3DNode *>(repeater->objectAt(i));
        lineNode->setProperty("lineColor", color);
    }
}

void QQuickGraphsItem::updateTitleLabels()
{
    if (m_changeTracker.axisXTitleVisibilityChanged) {
        m_titleLabelX->setVisible(axisX()->isTitleVisible());
        m_changeTracker.axisXTitleVisibilityChanged = false;
    }

    if (m_changeTracker.axisYTitleVisibilityChanged) {
        m_titleLabelY->setVisible(axisY()->isTitleVisible());
        m_changeTracker.axisYTitleVisibilityChanged = false;
    }

    if (m_changeTracker.axisZTitleVisibilityChanged) {
        m_titleLabelZ->setVisible(axisZ()->isTitleVisible());
        m_changeTracker.axisZTitleVisibilityChanged = false;
    }

    if (m_changeTracker.axisXTitleChanged) {
        m_titleLabelX->setProperty("labelText", axisX()->title());
        m_changeTracker.axisXTitleChanged = false;
    }

    if (m_changeTracker.axisYTitleChanged) {
        m_titleLabelY->setProperty("labelText", axisY()->title());
        m_changeTracker.axisYTitleChanged = false;
    }

    if (m_changeTracker.axisZTitleChanged) {
        m_titleLabelZ->setProperty("labelText", axisZ()->title());
        m_changeTracker.axisZTitleChanged = false;
    }
}


void QQuickGraphsItem::updateSelectionMode(QtGraphs3D::SelectionFlags newMode)
{
    Q_UNUSED(newMode);

    if (m_sliceView && m_sliceView->isVisible())
        toggleSliceGraph();
}

bool QQuickGraphsItem::doPicking(QPointF point)
{
    checkSliceEnabled();

    Q_TRACE_SCOPE(QGraphs3DItemDoPicking, point.x(), point.y());

    QList<QQuick3DPickResult> results = pickAll(point.x(), point.y());
    if (!m_customItemList.isEmpty()) {
        // Try to pick custom item only
        for (const auto &result : std::as_const(results)) {
            QCustom3DItem *customItem = m_customItemList.key(result.objectHit(), nullptr);

            if (customItem) {
                qsizetype selectedIndex = m_customItems.indexOf(customItem);
                m_selectedCustomItemIndex = selectedIndex;
                handleSelectedElementChange(QtGraphs3D::ElementType::CustomItem);
                // Don't allow picking in subclasses if custom item is picked
                return false;
            }
        }
    }

    for (const auto &result : std::as_const(results)) {
        if (!result.objectHit())
            continue;
        QString objName = result.objectHit()->objectName();
        if (objName.contains(QStringLiteral("ElementAxisXLabel"))) {
            for (int i = 0; i < repeaterX()->count(); i++) {
                auto obj = static_cast<QQuick3DNode *>(repeaterX()->objectAt(i));
                if (result.objectHit() == obj)
                    m_selectedLabelIndex = i;
            }
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisXLabel);
            break;
        } else if (objName.contains(QStringLiteral("ElementAxisYLabel"))) {
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisYLabel);
            break;
        } else if (objName.contains(QStringLiteral("ElementAxisZLabel"))) {
            for (int i = 0; i < repeaterX()->count(); i++) {
                auto obj = static_cast<QQuick3DNode *>(repeaterZ()->objectAt(i));
                if (result.objectHit() == obj)
                    m_selectedLabelIndex = i;
            }
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisZLabel);
            break;
        } else {
            continue;
        }
    }
    return true;
}

bool QQuickGraphsItem::doRayPicking(QVector3D origin, QVector3D direction)
{
    checkSliceEnabled();

    Q_TRACE_SCOPE(QGraphs3DItemDoRayPicking, origin.x(), origin.y(), origin.z(), direction.x(),
                  direction.y(), direction.z());
    QList<QQuick3DPickResult> results = rayPickAll(origin, direction);
    if (!m_customItemList.isEmpty()) {
        // Try to pick custom item only
        for (const auto &result : std::as_const(results)) {
            QCustom3DItem *customItem = m_customItemList.key(result.objectHit(), nullptr);

            if (customItem) {
                qsizetype selectedIndex = m_customItems.indexOf(customItem);
                m_selectedCustomItemIndex = selectedIndex;
                handleSelectedElementChange(QtGraphs3D::ElementType::CustomItem);
                // Don't allow picking in subclasses if custom item is picked
                return false;
            }
        }
    }

    for (const auto &result : std::as_const(results)) {
        if (!result.objectHit())
            continue;
        QString objName = result.objectHit()->objectName();
        if (objName.contains(QStringLiteral("ElementAxisXLabel"))) {
            for (int i = 0; i < repeaterX()->count(); i++) {
                auto obj = static_cast<QQuick3DNode *>(repeaterX()->objectAt(i));
                if (result.objectHit() == obj)
                    m_selectedLabelIndex = i;
            }
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisXLabel);
            break;
        } else if (objName.contains(QStringLiteral("ElementAxisYLabel"))) {
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisYLabel);
            break;
        } else if (objName.contains(QStringLiteral("ElementAxisZLabel"))) {
            for (int i = 0; i < repeaterX()->count(); i++) {
                auto obj = static_cast<QQuick3DNode *>(repeaterZ()->objectAt(i));
                if (result.objectHit() == obj)
                    m_selectedLabelIndex = i;
            }
            handleSelectedElementChange(QtGraphs3D::ElementType::AxisZLabel);
            break;
        } else {
            continue;
        }
    }
    return true;
}

void QQuickGraphsItem::minimizeMainGraph()
{
    QQuickItem *anchor = QQuickItemPrivate::get(this)->anchors()->fill();
    if (anchor)
        QQuickItemPrivate::get(this)->anchors()->resetFill();

    m_inputHandler->setX(x());
    m_inputHandler->setY(y());
}

void QQuickGraphsItem::toggleSliceGraph()
{
    if (!m_sliceView || !m_sliceActivatedChanged)
        return;

    if (m_sliceView->isVisible()) {
        // Maximize main view
        m_sliceView->setVisible(false);
        m_sliceView->update();
        setSlicingActive(false);
        updateSubViews();
        qCDebug(lcEvents3D, "%s exit sliceView", qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        // Minimize main view
        setSlicingActive(true);
        m_sliceView->setVisible(true);
        minimizeMainGraph();
        updateSubViews();
        updateSliceGrid();
        updateSliceLabels();
        qCDebug(lcEvents3D, "%s enter sliceView", qUtf8Printable(QLatin1String(__FUNCTION__)));
    }

    m_sliceActivatedChanged = false;
}

void QQuickGraphsItem::updateSubViews()
{
    QRect newMainView = isSlicingActive() ? scene()->primarySubViewport() : scene()->viewport();
    QRect newSliceView = scene()->secondarySubViewport();

    if (newMainView.isValid() && newMainView.toRectF() != boundingRect()) {
        // Set main view dimensions and position
        setX(newMainView.x());
        setY(newMainView.y());
        setSize(newMainView.size());
        update();
    }

    if (sliceView()) {
        if (newSliceView.isValid() && m_sliceView->boundingRect() != newSliceView.toRectF()) {
            // Set slice view dimensions and position
            m_sliceView->setX(newSliceView.x());
            m_sliceView->setY(newSliceView.y());
            m_sliceView->setSize(newSliceView.size());
            m_sliceView->update();
        }

        if (isSliceOrthoProjection()) {
            const float scale = qMin(m_sliceView->width(), m_sliceView->height());
            QQuick3DOrthographicCamera *camera = static_cast<QQuick3DOrthographicCamera *>(
                        m_sliceView->camera());
            const float magnificationScaleFactor = .16f; // this controls the size of the slice view
            const float magnification = scale * magnificationScaleFactor;
            camera->setHorizontalMagnification(magnification);
            camera->setVerticalMagnification(magnification);
        }
    }
}

void QQuickGraphsItem::windowDestroyed(QObject *obj)
{
    // Remove destroyed window from window lists
    QQuickWindow *win = static_cast<QQuickWindow *>(obj);
    QQuickWindow *oldWindow = m_graphWindowList.value(this);

    if (win == oldWindow)
        m_graphWindowList.remove(this);
}

QQmlComponent *QQuickGraphsItem::createRepeaterDelegateComponent(const QString &fileName)
{
    QQmlComponent component(qmlEngine(this), fileName);
    return qobject_cast<QQmlComponent *>(component.create());
}

QQuick3DRepeater *QQuickGraphsItem::createRepeater(QQuick3DNode *parent)
{
    auto engine = qmlEngine(this);
    QQmlComponent repeaterComponent(engine);
    repeaterComponent.setData("import QtQuick3D; Repeater3D{}", QUrl());
    auto repeater = qobject_cast<QQuick3DRepeater *>(repeaterComponent.create());
    repeater->setParent(parent ? parent : graphNode());
    repeater->setParentItem(parent ? parent : graphNode());
    return repeater;
}

QQuick3DNode *QQuickGraphsItem::createTitleLabel(QQuick3DNode *parent)
{
    auto engine = qmlEngine(this);
    QQmlComponent comp(engine, QStringLiteral(":/axis/TitleLabel"));
    auto titleLabel = qobject_cast<QQuick3DNode *>(comp.create());
    titleLabel->setParent(parent ? parent : graphNode());
    titleLabel->setParentItem(parent ? parent : graphNode());
    titleLabel->setVisible(false);
    titleLabel->setScale(m_labelScale);
    return titleLabel;
}

void QQuickGraphsItem::createItemLabel()
{
    auto engine = qmlEngine(this);
    QQmlComponent comp(engine, QStringLiteral(":/axis/ItemLabel"));
    m_itemLabel = qobject_cast<QQuickItem *>(comp.create());
    m_itemLabel->setParent(this);
    m_itemLabel->setParentItem(this);
    m_itemLabel->setVisible(false);
}

QQuick3DCustomMaterial *QQuickGraphsItem::createQmlCustomMaterial(const QString &fileName)
{
    QQmlComponent component(qmlEngine(this), fileName);
    QQuick3DCustomMaterial *material = qobject_cast<QQuick3DCustomMaterial *>(component.create());
    return material;
}

QQuick3DPrincipledMaterial *QQuickGraphsItem::createPrincipledMaterial()
{
    QQmlComponent component(qmlEngine(this));
    component.setData("import QtQuick3D; PrincipledMaterial{}", QUrl());
    return qobject_cast<QQuick3DPrincipledMaterial *>(component.create());
}

QtGraphs3D::CameraPreset QQuickGraphsItem::cameraPreset() const
{
    return m_activePreset;
}

void QQuickGraphsItem::setCameraPreset(QtGraphs3D::CameraPreset preset)
{
    switch (preset) {
    case QtGraphs3D::CameraPreset::FrontLow: {
        m_xRotation = 0.0f;
        m_yRotation = 0.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::Front: {
        m_xRotation = 0.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::FrontHigh: {
        m_xRotation = 0.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::LeftLow: {
        m_xRotation = 90.0f;
        m_yRotation = 0.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::Left: {
        m_xRotation = 90.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::LeftHigh: {
        m_xRotation = 90.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::RightLow: {
        m_xRotation = -90.0f;
        m_yRotation = 0.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::Right: {
        m_xRotation = -90.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::RightHigh: {
        m_xRotation = -90.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::BehindLow: {
        m_xRotation = 180.0f;
        m_yRotation = 0.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::Behind: {
        m_xRotation = 180.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::BehindHigh: {
        m_xRotation = 180.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::IsometricLeft: {
        m_xRotation = 45.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::IsometricLeftHigh: {
        m_xRotation = 45.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::IsometricRight: {
        m_xRotation = -45.0f;
        m_yRotation = 22.5f;
        break;
    }
    case QtGraphs3D::CameraPreset::IsometricRightHigh: {
        m_xRotation = -45.0f;
        m_yRotation = 45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::DirectlyAbove: {
        m_xRotation = 0.0f;
        m_yRotation = 90.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::DirectlyAboveCW45: {
        m_xRotation = -45.0f;
        m_yRotation = 90.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::DirectlyAboveCCW45: {
        m_xRotation = 45.0f;
        m_yRotation = 90.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::FrontBelow: {
        m_xRotation = 0.0f;
        m_yRotation = -45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::LeftBelow: {
        m_xRotation = 90.0f;
        m_yRotation = -45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::RightBelow: {
        m_xRotation = -90.0f;
        m_yRotation = -45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::BehindBelow: {
        m_xRotation = 180.0f;
        m_yRotation = -45.0f;
        break;
    }
    case QtGraphs3D::CameraPreset::DirectlyBelow: {
        m_xRotation = 0.0f;
        m_yRotation = -90.0f;
        break;
    }
    default:
        preset = QtGraphs3D::CameraPreset::NoPreset;
        break;
    }

    // All presets target the center of the graph
    setCameraTargetPosition(QVector3D());

    if (m_activePreset != preset) {
        m_activePreset = preset;
        emit cameraPresetChanged(preset);
    }
    if (camera()) {
        updateCamera();
        connect(this, &QQuickGraphsItem::cameraXRotationChanged, m_scene, &Q3DScene::needRender);
        connect(this, &QQuickGraphsItem::cameraYRotationChanged, m_scene, &Q3DScene::needRender);
        connect(this, &QQuickGraphsItem::cameraZoomLevelChanged, m_scene, &Q3DScene::needRender);
    }
    m_changeTracker.cameraChanged = true;
}

void QQuickGraphsItem::setCameraXRotation(float rotation)
{
    if (m_wrapXRotation)
        rotation = Utils::wrapValue(rotation, m_minXRotation, m_maxXRotation);
    else
        rotation = qBound(m_minXRotation, rotation, m_maxXRotation);
    if (rotation != m_xRotation) {
        m_xRotation = rotation;
        qCDebug(lcEvents3D, "%s x rotation: %.1f ", qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        m_changeTracker.cameraChanged = true;
        emit cameraXRotationChanged(m_xRotation);
    }
}

void QQuickGraphsItem::setCameraYRotation(float rotation)
{
    if (m_wrapYRotation)
        rotation = Utils::wrapValue(rotation, m_minYRotation, m_maxYRotation);
    else
        rotation = qBound(m_minYRotation, rotation, m_maxYRotation);
    if (rotation != m_yRotation) {
        m_yRotation = rotation;
        qCDebug(lcEvents3D, "%s y rotation: %.1f ", qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        m_changeTracker.cameraChanged = true;
        emit cameraYRotationChanged(m_yRotation);
    }
}

void QQuickGraphsItem::setMinCameraXRotation(float rotation)
{
    if (m_minXRotation == rotation) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        return;
    }

    m_minXRotation = rotation;
    setUserCameraRotationRange(true);
    emit minCameraXRotationChanged(rotation);
}

void QQuickGraphsItem::setMaxCameraXRotation(float rotation)
{
    if (m_maxXRotation == rotation) {
        qCDebug(lcProperties3D,"%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        return;
    }

    m_maxXRotation = rotation;
    setUserCameraRotationRange(true);
    emit maxCameraXRotationChanged(rotation);
}

void QQuickGraphsItem::setMinCameraYRotation(float rotation)
{
    if (m_minYRotation == rotation) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        return;
    }

    m_minYRotation = rotation;
    setUserCameraRotationRange(true);
    emit minCameraYRotationChanged(rotation);
}

void QQuickGraphsItem::setMaxCameraYRotation(float rotation)
{
    if (m_maxYRotation == rotation) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), rotation);
        return;
    }

    m_maxYRotation = rotation;
    setUserCameraRotationRange(true);
    emit maxCameraYRotationChanged(rotation);
}

void QQuickGraphsItem::setZoomAtTargetEnabled(bool enable)
{
    m_inputHandler->setZoomAtTargetEnabled(enable);
}

bool QQuickGraphsItem::zoomAtTargetEnabled()
{
    return m_inputHandler->isZoomAtTargetEnabled();
}

void QQuickGraphsItem::setZoomEnabled(bool enable)
{
    m_inputHandler->setZoomEnabled(enable);
}

bool QQuickGraphsItem::zoomEnabled()
{
    return m_inputHandler->isZoomEnabled();
}

void QQuickGraphsItem::setSelectionEnabled(bool enable)
{
    m_inputHandler->setSelectionEnabled(enable);
}

bool QQuickGraphsItem::selectionEnabled()
{
    return m_inputHandler->isSelectionEnabled();
}

void QQuickGraphsItem::setRotationEnabled(bool enable)
{
    m_inputHandler->setRotationEnabled(enable);
}

bool QQuickGraphsItem::rotationEnabled()
{
    return m_inputHandler->isRotationEnabled();
}

void QQuickGraphsItem::unsetDefaultInputHandler()
{
    m_inputHandler->unsetDefaultInputHandler();
}

void QQuickGraphsItem::unsetDefaultTapHandler()
{
    m_inputHandler->unsetDefaultTapHandler();
}

void QQuickGraphsItem::unsetDefaultDragHandler()
{
    m_inputHandler->unsetDefaultDragHandler();
}

void QQuickGraphsItem::unsetDefaultWheelHandler()
{
    m_inputHandler->unsetDefaultWheelHandler();
}

void QQuickGraphsItem::unsetDefaultPinchHandler()
{
    m_inputHandler->unsetDefaultPinchHandler();
}

void QQuickGraphsItem::setDragButton(Qt::MouseButtons button)
{
    m_inputHandler->setDragButton(button);
}

void QQuickGraphsItem::setDefaultInputHandler()
{
    m_inputHandler->setDefaultInputHandler();
}

void QQuickGraphsItem::setCameraZoomLevel(float level)
{
    if (m_zoomLevel == level)
        return;

    m_zoomLevel = level;
    qCDebug(lcEvents3D, "%s zoom level: %.1f", qUtf8Printable(QLatin1String(__FUNCTION__)), level);
    m_changeTracker.cameraChanged = true;
    emit cameraZoomLevelChanged(level);
}

void QQuickGraphsItem::setMinCameraZoomLevel(float level)
{
    if (m_minZoomLevel == level || level < 1.f) {
        qCDebug(lcProperties3D, "%s value: %.1f is either same or it is lower than 1.0f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), level);
        return;
    }

    m_minZoomLevel = level;
    emit minCameraZoomLevelChanged(level);

    setMaxCameraZoomLevel(std::max(m_minZoomLevel, m_maxZoomLevel));

    if (cameraZoomLevel() < level)
        setCameraZoomLevel(level);
}

void QQuickGraphsItem::setMaxCameraZoomLevel(float level)
{
    if (m_maxZoomLevel == level) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), level);
        return;
    }

    m_maxZoomLevel = level;
    emit maxCameraZoomLevelChanged(level);

    setMinCameraZoomLevel(std::min(m_minZoomLevel, m_maxZoomLevel));

    if (cameraZoomLevel() > level)
        setCameraZoomLevel(level);
}

void QQuickGraphsItem::setCameraTargetPosition(QVector3D target)
{
    if (m_requestedTarget == target) {
        qCDebug(lcProperties3D) << qUtf8Printable(QLatin1String(__FUNCTION__))
            << "position is already set to:" << target;
        return;
    }

    m_requestedTarget.setX(std::clamp(target.x(), -1.0f, 1.0f));
    m_requestedTarget.setY(std::clamp(target.y(), -1.0f, 1.0f));
    m_requestedTarget.setZ(std::clamp(target.z(), -1.0f, 1.0f));
    m_changeTracker.cameraChanged = true;
    emit cameraTargetPositionChanged(target);
    emitNeedRender();
}

void QQuickGraphsItem::setCameraPosition(float horizontal, float vertical, float zoom)
{
    setCameraZoomLevel(zoom);
    setCameraXRotation(horizontal);
    setCameraYRotation(vertical);
}

bool QQuickGraphsItem::event(QEvent *event)
{
    return QQuickItem::event(event);
}

void QQuickGraphsItem::createSliceView()
{
    if (m_sliceView)
        return;
    Q_TRACE_SCOPE(QGraphs3DItemCreateSliceView);

    connect(parentItem(),
            &QQuickItem::widthChanged,
            this,
            &QQuickGraphsItem::handleParentWidthChange);
    connect(parentItem(),
            &QQuickItem::heightChanged,
            this,
            &QQuickGraphsItem::handleParentHeightChange);
    connect(this, &QQuickItem::heightChanged,
            this,
            &QQuickGraphsItem::handleParentHeightChange);
    connect(this, &QQuickItem::widthChanged,
            this,
            &QQuickGraphsItem::handleParentWidthChange);

    m_sliceView = new QQuick3DViewport();
    m_sliceView->setParent(parent());
    m_sliceView->setParentItem(parentItem());
    m_sliceView->setVisible(false);
    if (!m_parentNode) {
        m_sliceView->setHeight(parentItem()->height());
        m_sliceView->setWidth(parentItem()->width());
    }
    m_sliceView->setZ(-1);
    m_sliceView->environment()->setBackgroundMode(QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes::Color);
    m_sliceView->environment()->setClearColor(environment()->clearColor());
    m_sliceView->setRenderMode(renderMode());

    auto scene = m_sliceView->scene();

    createSliceCamera(m_sliceView);

    m_labelDelegate.reset(new QQmlComponent(qmlEngine(this), QStringLiteral(":/axis/AxisLabel")));

    m_sliceGridGeometryModel = new QQuick3DModel(scene);

    auto sliceGridGeometry = new QQuick3DGeometry(m_sliceGridGeometryModel);
    sliceGridGeometry->setStride(sizeof(QVector3D));
    sliceGridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    sliceGridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                                    0,
                                    QQuick3DGeometry::Attribute::F32Type);
    m_sliceGridGeometryModel->setGeometry(sliceGridGeometry);

    QQmlListReference gridMaterialRef(m_sliceGridGeometryModel, "materials");
    auto gridMaterial = new QQuick3DPrincipledMaterial(m_sliceGridGeometryModel);
    gridMaterial->setLighting(QQuick3DPrincipledMaterial::Lighting::NoLighting);
    gridMaterial->setCullMode(QQuick3DMaterial::CullMode::BackFaceCulling);
    gridMaterial->setBaseColor(Qt::red);
    gridMaterialRef.append(gridMaterial);

    m_sliceHorizontalLabelRepeater = createRepeater(scene);
    m_sliceHorizontalLabelRepeater->setDelegate(m_labelDelegate.get());

    m_sliceVerticalLabelRepeater = createRepeater(scene);
    m_sliceVerticalLabelRepeater->setDelegate(m_labelDelegate.get());

    m_sliceHorizontalTitleLabel = createTitleLabel(scene);
    m_sliceHorizontalTitleLabel->setVisible(true);

    m_sliceVerticalTitleLabel = createTitleLabel(scene);
    m_sliceVerticalTitleLabel->setVisible(true);

    m_sliceItemLabel = createTitleLabel(scene);
    m_sliceItemLabel->setVisible(false);
}

QQuick3DViewport *QQuickGraphsItem::createOffscreenSliceView(QtGraphs3D::SliceCaptureType sliceType)
{
    Q_TRACE_SCOPE(QGraphs3DItemCreateOffscreenSliceView, static_cast<int>(sliceType));

    auto sliceView = new QQuick3DViewport();
    sliceView->setParent(this);
    sliceView->setParentItem(this);
    sliceView->setWidth(parentItem()->width() * .5);
    sliceView->setHeight(parentItem()->height() * .5);
    sliceView->setX(sliceView->width() * -1);
    sliceView->environment()->setBackgroundMode(
            QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes::Color);
    sliceView->environment()->setClearColor(environment()->clearColor());
    sliceView->setRenderMode(renderMode());

    auto scene = sliceView->scene();

    createSliceCamera(sliceView);

    std::unique_ptr<QQmlComponent> labelDelegate;
    labelDelegate.reset(new QQmlComponent(qmlEngine(this), QStringLiteral(":/axis/AxisLabel")));

    auto sliceGridGeometryModel = new QQuick3DModel(scene);

    auto sliceGridGeometry = new QQuick3DGeometry(sliceGridGeometryModel);
    sliceGridGeometry->setStride(sizeof(QVector3D));
    sliceGridGeometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    sliceGridGeometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                                    0,
                                    QQuick3DGeometry::Attribute::F32Type);
    sliceGridGeometryModel->setGeometry(sliceGridGeometry);

    QQmlListReference gridMaterialRef(sliceGridGeometryModel, "materials");
    auto gridMaterial = new QQuick3DPrincipledMaterial(sliceGridGeometryModel);
    gridMaterial->setLighting(QQuick3DPrincipledMaterial::Lighting::NoLighting);
    gridMaterial->setCullMode(QQuick3DMaterial::CullMode::BackFaceCulling);
    gridMaterial->setBaseColor(Qt::red);
    gridMaterialRef.append(gridMaterial);

    updateSliceGrid(sliceGridGeometryModel, sliceType);

    auto sliceHorizontalLabelRepeater = createRepeater(scene);
    sliceHorizontalLabelRepeater->setDelegate(labelDelegate.get());

    auto sliceVerticalLabelRepeater = createRepeater(scene);
    sliceVerticalLabelRepeater->setDelegate(labelDelegate.get());

    auto sliceHorizontalTitleLabel = createTitleLabel(scene);
    sliceHorizontalTitleLabel->setVisible(true);

    auto sliceVerticalTitleLabel = createTitleLabel(scene);
    sliceVerticalTitleLabel->setVisible(true);

    auto sliceItemLabel = createTitleLabel(scene);
    sliceItemLabel->setVisible(false);

    updateSliceLabels(sliceHorizontalLabelRepeater, sliceVerticalLabelRepeater,
                      sliceHorizontalTitleLabel, sliceVerticalTitleLabel, sliceItemLabel,
                      sliceType);

    return sliceView;
}

void QQuickGraphsItem::createSliceCamera(QQuick3DViewport *sliceView)
{
    if (isSliceOrthoProjection()) {
        auto camera = new QQuick3DOrthographicCamera(sliceView->scene());
        camera->setPosition(QVector3D(.0f, .0f, 20.0f));
        const float scale = qMin(sliceView->width(), sliceView->height());
        const float magnificationScaleFactor = 2 * window()->devicePixelRatio()
                                               * .08f; // this controls the size of the slice view
        const float magnification = scale * magnificationScaleFactor;
        camera->setHorizontalMagnification(magnification);
        camera->setVerticalMagnification(magnification);
        sliceView->setCamera(camera);

        auto light = new QQuick3DDirectionalLight(sliceView->scene());
        light->setParent(camera);
        light->setParentItem(camera);
    } else {
        auto camera = new QQuick3DPerspectiveCamera(sliceView->scene());
        camera->setFieldOfViewOrientation(
            QQuick3DPerspectiveCamera::FieldOfViewOrientation::Vertical);
        camera->setClipNear(5.f);
        camera->setClipFar(15.f);
        camera->setFieldOfView(35.f);
        camera->setPosition(QVector3D(.0f, .0f, 10.f));
        sliceView->setCamera(camera);

        auto light = new QQuick3DDirectionalLight(sliceView->scene());
        light->setParent(camera);
        light->setParentItem(camera);
        light->setAmbientColor(QColor::fromRgbF(1.f, 1.f, 1.f));
    }
}

void QQuickGraphsItem::updateSliceGrid(QQuick3DModel *sliceGridGeometryModel,
                                       QtGraphs3D::SliceCaptureType sliceType)
{
    Q_TRACE_SCOPE(QGraphs3DItemUpdateSliceGrid);
    QAbstract3DAxis *horizontalAxis = nullptr;
    QAbstract3DAxis *verticalAxis = axisY();
    auto backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    float scale;
    float translate;

    float horizontalScale = 0.0f;

    const bool isRow = ((sliceType == QtGraphs3D::SliceCaptureType::NoImage
                         && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))
                        || sliceType == QtGraphs3D::SliceCaptureType::RowImage);
    const bool isColumn = ((sliceType == QtGraphs3D::SliceCaptureType::NoImage
                            && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))
                           || sliceType == QtGraphs3D::SliceCaptureType::ColumnImage);

    if (isRow) {
        horizontalAxis = axisX();
        horizontalScale = backgroundScale.x();
        scale = m_scaleWithBackground.x();
        translate = m_scaleWithBackground.x();
    } else if (isColumn) {
        horizontalAxis = axisZ();
        horizontalScale = backgroundScale.z();
        scale = m_scaleWithBackground.z();
        translate = m_scaleWithBackground.z();
    }

    if (horizontalAxis == nullptr) {
        qCWarning(lcGraphs3D, "%s invalid axis type",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }
    int lineCount = 0;
    if (m_hasVerticalSegmentLine || isPolar()) {
        if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Value) {
            QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(horizontalAxis);
            lineCount += valueAxis->gridSize() + valueAxis->subGridSize();
        } else if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Category) {
            lineCount += horizontalAxis->labels().size();
        }
    }

    if (verticalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(verticalAxis);
        lineCount += valueAxis->gridSize() + valueAxis->subGridSize();
    } else if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        lineCount += verticalAxis->labels().size();
    }

    QByteArray vertices;
    vertices.resize(lineCount * 2 * sizeof(QVector3D));
    auto data = reinterpret_cast<QVector3D *>(vertices.data());
    float linePosX = .0f;
    float linePosY = .0f;
    const float linePosZ = -1.f; // Draw grid lines behind slice (especially for surface)

    float x0, x1;
    float y0, y1;
    y0 = -backgroundScale.y();
    y1 = backgroundScale.y();
    if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        auto axis = static_cast<QValue3DAxis *>(horizontalAxis);
        for (int i = 0; i < axis->subGridSize(); i++) {
            linePosX = axis->subGridPositionAt(i) * scale * 2.0f - translate;
            *data++ = QVector3D(linePosX, y0, linePosZ);
            *data++ = QVector3D(linePosX, y1, linePosZ);
        }
        for (int i = 0; i < axis->gridSize(); i++) {
            linePosX = axis->gridPositionAt(i) * scale * 2.0f - translate;
            *data++ = QVector3D(linePosX, y0, linePosZ);
            *data++ = QVector3D(linePosX, y1, linePosZ);
        }
    }

    scale = m_scaleWithBackground.y();
    translate = m_scaleWithBackground.y();

    x0 = horizontalScale * 1.1f;
    x1 = -horizontalScale * 1.1f;
    if (verticalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        auto axis = static_cast<QValue3DAxis *>(verticalAxis);
        for (int i = 0; i < axis->gridSize(); i++) {
            linePosY = axis->gridPositionAt(i) * scale * 2.0f - translate;
            *data++ = QVector3D(x0, linePosY, linePosZ);
            *data++ = QVector3D(x1, linePosY, linePosZ);
        }
        for (int i = 0; i < axis->subGridSize(); i++) {
            linePosY = axis->subGridPositionAt(i) * scale * 2.0f - translate;
            *data++ = QVector3D(x0, linePosY, linePosZ);
            *data++ = QVector3D(x1, linePosY, linePosZ);
        }
    } else if (verticalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        for (int i = 0; i < verticalAxis->labels().size(); i++) {
            linePosY = calculateCategoryGridLinePosition(verticalAxis, i);
            *data++ = QVector3D(x0, linePosY, linePosZ);
            *data++ = QVector3D(x1, linePosY, linePosZ);
        }
    }

    QQuick3DModel *sliceModel = nullptr;
    if (sliceGridGeometryModel)
        sliceModel = sliceGridGeometryModel;
    else
        sliceModel = m_sliceGridGeometryModel;
    QQuick3DGeometry *geometry = sliceModel->geometry();
    geometry->setVertexData(vertices);
    geometry->update();

    QQmlListReference materialRef(sliceModel, "materials");
    auto material = static_cast<QQuick3DPrincipledMaterial *>(materialRef.at(0));
    material->setBaseColor(theme()->grid().mainColor());
}

void QQuickGraphsItem::updateSliceLabels(QQuick3DRepeater *horizontalLabel,
                                         QQuick3DRepeater *verticalLabel,
                                         QQuick3DNode *horizontalTitle,
                                         QQuick3DNode *verticalTitle,
                                         QQuick3DNode *itemLabel,
                                         QtGraphs3D::SliceCaptureType sliceType)
{
    Q_TRACE_SCOPE(QGraphs3DItemUpdateSliceLabels);
    QAbstract3DAxis *horizontalAxis = nullptr;
    QAbstract3DAxis *verticalAxis = axisY();
    auto backgroundScale = m_scaleWithBackground + m_backgroundScaleMargin;
    float scale;
    float translate;
    QColor horizontalLabelTextColor;

    QQuick3DRepeater *sliceHorizontalLabelRepeater = nullptr;
    if (horizontalLabel)
        sliceHorizontalLabelRepeater = horizontalLabel;
    else
        sliceHorizontalLabelRepeater = m_sliceHorizontalLabelRepeater;

    QQuick3DRepeater *sliceVerticalLabelRepeater = nullptr;
    if (verticalLabel)
        sliceVerticalLabelRepeater = verticalLabel;
    else
        sliceVerticalLabelRepeater = m_sliceVerticalLabelRepeater;

    const bool isRow = ((sliceType == QtGraphs3D::SliceCaptureType::NoImage
                         && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Row))
                        || sliceType == QtGraphs3D::SliceCaptureType::RowImage);
    const bool isColumn = ((sliceType == QtGraphs3D::SliceCaptureType::NoImage
                            && selectionMode().testFlag(QtGraphs3D::SelectionFlag::Column))
                           || sliceType == QtGraphs3D::SliceCaptureType::ColumnImage);

    if (isRow) {
        horizontalAxis = axisX();
        scale = backgroundScale.x() - m_backgroundScaleMargin.x();
        translate = backgroundScale.x() - m_backgroundScaleMargin.x();
        horizontalLabelTextColor = theme()->axisX().labelTextColor();
    } else if (isColumn) {
        horizontalAxis = axisZ();
        scale = backgroundScale.z() - m_backgroundScaleMargin.z();
        translate = backgroundScale.z() - m_backgroundScaleMargin.z();
        horizontalLabelTextColor = theme()->axisZ().labelTextColor();
    }

    if (horizontalAxis == nullptr) {
        qCWarning(lcProperties3D, "%s invalid selection mode",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    }

    if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(horizontalAxis);
        sliceHorizontalLabelRepeater->model().clear();
        sliceHorizontalLabelRepeater->setModel(valueAxis->labels().size());
    } else if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        sliceHorizontalLabelRepeater->model().clear();
        sliceHorizontalLabelRepeater->setModel(horizontalAxis->labels().size());
    }

    if (verticalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        QValue3DAxis *valueAxis = static_cast<QValue3DAxis *>(verticalAxis);
        sliceVerticalLabelRepeater->model().clear();
        sliceVerticalLabelRepeater->setModel(valueAxis->labels().size());
    } else if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        sliceVerticalLabelRepeater->model().clear();
        sliceVerticalLabelRepeater->setModel(verticalAxis->labels().size());
    }

    float textPadding = 12.0f;
    float labelsMaxWidth = float(findLabelsMaxWidth(horizontalAxis->labels())) + textPadding;
    QFontMetrics fm(theme()->labelFont());
    float labelHeight = fm.height() + textPadding;

    float pointSize = theme()->labelFont().pointSizeF();
    float scaleFactor = fontScaleFactor(pointSize) * pointSize;
    float fontRatio = labelsMaxWidth / labelHeight;
    QVector3D fontScaled = QVector3D(scaleFactor * fontRatio, scaleFactor, 0.00001f);

    float adjustment = labelsMaxWidth * scaleFactor;
    float yPos = backgroundScale.y() + adjustment;

    QVector3D labelTrans = QVector3D(0.0f, -yPos, 0.0f);
    QStringList labels = horizontalAxis->labels();
    QFont font = theme()->labelFont();
    bool borderVisible = theme()->isLabelBorderVisible();

    bool backgroundVisible = theme()->isLabelBackgroundVisible();
    QColor backgroundColor = theme()->labelBackgroundColor();

    if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        for (int i = 0; i < sliceHorizontalLabelRepeater->count(); i++) {
            auto obj = static_cast<QQuick3DNode *>(sliceHorizontalLabelRepeater->objectAt(i));
            // It is important to use the position of vertical grids so that they can be in the same
            // position when col/row ranges are updated.
            float linePosX = static_cast<QValue3DAxis *>(horizontalAxis)->gridPositionAt(i) * scale
                                 * 2.0f
                             - translate;
            labelTrans.setX(linePosX);
            labelTrans.setY(-yPos - adjustment);
            obj->setScale(fontScaled);
            obj->setPosition(labelTrans);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            obj->setProperty("labelFont", font);
            obj->setProperty("borderVisible", borderVisible);
            obj->setProperty("labelTextColor", horizontalLabelTextColor);
            obj->setProperty("backgroundVisible", backgroundVisible);
            obj->setProperty("backgroundColor", backgroundColor);
            obj->setEulerRotation(QVector3D(.0f, .0f, -45.0f));
            if (!labels[i].compare(QString(hiddenLabelTag)))
                obj->setVisible(false);
        }
    } else if (horizontalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        for (int i = 0; i < sliceHorizontalLabelRepeater->count(); i++) {
            labelTrans = calculateCategoryLabelPosition(horizontalAxis, labelTrans, i);
            labelTrans.setY(-yPos /*- (adjustment / 2.f)*/);
            if (isColumn)
                labelTrans.setX(labelTrans.z());
            labelTrans.setZ(1.0f); // Bring the labels on top of bars and grid
            auto obj = static_cast<QQuick3DNode *>(sliceHorizontalLabelRepeater->objectAt(i));
            obj->setScale(fontScaled);
            obj->setPosition(labelTrans);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            obj->setProperty("labelFont", font);
            obj->setProperty("borderVisible", borderVisible);
            obj->setProperty("labelTextColor", horizontalLabelTextColor);
            obj->setProperty("backgroundVisible", backgroundVisible);
            obj->setProperty("backgroundColor", backgroundColor);
            obj->setEulerRotation(QVector3D(0.0f, 0.0f, -60.0f));
        }
    }

    scale = backgroundScale.y() - m_backgroundScaleMargin.y();
    translate = backgroundScale.y() - m_backgroundScaleMargin.y();
    labels = verticalAxis->labels();
    labelsMaxWidth = float(findLabelsMaxWidth(labels)) + textPadding;
    // Since labelsMaxWidth changes for each axis, these needs to be recalculated for scaling.
    fontRatio = labelsMaxWidth / labelHeight;
    fontScaled.setX(scaleFactor * fontRatio);
    adjustment = labelsMaxWidth * scaleFactor;
    float xPos = 0.0f;
    if (isRow)
        xPos = backgroundScale.x() + (adjustment * 1.5f);
    else if (isColumn)
        xPos = backgroundScale.z() + (adjustment * 1.5f);
    labelTrans = QVector3D(xPos, 0.0f, 0.0f);
    QColor verticalLabelTextColor = theme()->axisY().labelTextColor();

    if (verticalAxis->type() == QAbstract3DAxis::AxisType::Value) {
        auto valueAxis = static_cast<QValue3DAxis *>(verticalAxis);
        for (int i = 0; i < sliceVerticalLabelRepeater->count(); i++) {
            auto obj = static_cast<QQuick3DNode *>(sliceVerticalLabelRepeater->objectAt(i));
            labelTrans.setY(valueAxis->labelPositionAt(i) * scale * 2.0f - translate);
            obj->setScale(fontScaled);
            obj->setPosition(labelTrans);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            obj->setProperty("labelFont", font);
            obj->setProperty("borderVisible", borderVisible);
            obj->setProperty("labelTextColor", verticalLabelTextColor);
            obj->setProperty("backgroundVisible", backgroundVisible);
            obj->setProperty("backgroundColor", backgroundColor);
            if (!labels[i].compare(QString(hiddenLabelTag)))
                obj->setVisible(false);
        }
    } else if (verticalAxis->type() == QAbstract3DAxis::AxisType::Category) {
        for (int i = 0; i < sliceVerticalLabelRepeater->count(); i++) {
            labelTrans = calculateCategoryLabelPosition(verticalAxis, labelTrans, i);
            auto obj = static_cast<QQuick3DNode *>(sliceVerticalLabelRepeater->objectAt(i));
            obj->setScale(fontScaled);
            obj->setPosition(labelTrans);
            obj->setProperty("labelText", labels[i]);
            obj->setProperty("labelWidth", labelsMaxWidth);
            obj->setProperty("labelHeight", labelHeight);
            obj->setProperty("labelFont", font);
            obj->setProperty("borderVisible", borderVisible);
            obj->setProperty("labelTextColor", verticalLabelTextColor);
            obj->setProperty("backgroundVisible", backgroundVisible);
            obj->setProperty("backgroundColor", backgroundColor);
        }
    }

    labelHeight = fm.height() + textPadding;
    float labelWidth = fm.horizontalAdvance(verticalAxis->title()) + textPadding;
    QVector3D vTitleScale = fontScaled;
    vTitleScale.setX(fontScaled.y() * labelWidth / labelHeight);
    adjustment = labelHeight * scaleFactor;
    if (isRow)
        xPos = backgroundScale.x() + adjustment;
    else if (isColumn)
        xPos = backgroundScale.z() + adjustment;
    labelTrans = QVector3D(-(xPos + adjustment), 0.0f, 0.0f);

    QQuick3DNode *sliceVerticalTitleLabel = nullptr;
    if (verticalTitle)
        sliceVerticalTitleLabel = verticalTitle;
    else
        sliceVerticalTitleLabel = m_sliceVerticalTitleLabel;
    if (!verticalAxis->title().isEmpty()) {
        sliceVerticalTitleLabel->setScale(vTitleScale);
        sliceVerticalTitleLabel->setPosition(labelTrans);
        sliceVerticalTitleLabel->setProperty("labelWidth", labelWidth);
        sliceVerticalTitleLabel->setProperty("labelHeight", labelHeight);
        sliceVerticalTitleLabel->setProperty("labelText", verticalAxis->title());
        sliceVerticalTitleLabel->setProperty("labelFont", font);
        sliceVerticalTitleLabel->setProperty("borderVisible", borderVisible);
        sliceVerticalTitleLabel->setProperty("labelTextColor", verticalLabelTextColor);
        sliceVerticalTitleLabel->setProperty("backgroundVisible", backgroundVisible);
        sliceVerticalTitleLabel->setProperty("backgroundColor", backgroundColor);
        sliceVerticalTitleLabel->setEulerRotation(QVector3D(.0f, .0f, 90.0f));
    } else {
        sliceVerticalTitleLabel->setVisible(false);
    }

    labelHeight = fm.height() + textPadding;
    labelWidth = fm.horizontalAdvance(horizontalAxis->title()) + textPadding;
    QVector3D hTitleScale = fontScaled;
    hTitleScale.setX(fontScaled.y() * labelWidth / labelHeight);
    adjustment = labelHeight * scaleFactor;
    yPos = backgroundScale.y() * 1.5f + (adjustment * 6.f);
    labelTrans = QVector3D(0.0f, -yPos, 0.0f);

    QQuick3DNode *sliceHorizontalTitleLabel = nullptr;
    if (horizontalTitle)
        sliceHorizontalTitleLabel = horizontalTitle;
    else
        sliceHorizontalTitleLabel = m_sliceHorizontalTitleLabel;
    if (!horizontalAxis->title().isEmpty()) {
        sliceHorizontalTitleLabel->setScale(hTitleScale);
        sliceHorizontalTitleLabel->setPosition(labelTrans);
        sliceHorizontalTitleLabel->setProperty("labelWidth", labelWidth);
        sliceHorizontalTitleLabel->setProperty("labelHeight", labelHeight);
        sliceHorizontalTitleLabel->setProperty("labelText", horizontalAxis->title());
        sliceHorizontalTitleLabel->setProperty("labelFont", font);
        sliceHorizontalTitleLabel->setProperty("borderVisible", borderVisible);
        sliceHorizontalTitleLabel->setProperty("labelTextColor", horizontalLabelTextColor);
        sliceHorizontalTitleLabel->setProperty("backgroundVisible", backgroundVisible);
        sliceHorizontalTitleLabel->setProperty("backgroundColor", backgroundColor);
    } else {
        sliceHorizontalTitleLabel->setVisible(false);
    }

    QQuick3DNode *sliceItemLabel = nullptr;
    if (itemLabel)
        sliceItemLabel = itemLabel;
    else
        sliceItemLabel = m_sliceItemLabel;
    sliceItemLabel->setProperty("labelFont", font);
    sliceItemLabel->setProperty("borderVisible", borderVisible);
    sliceItemLabel->setProperty("labelTextColor", theme()->labelTextColor());
    sliceItemLabel->setProperty("backgroundVisible", backgroundVisible);
    sliceItemLabel->setProperty("backgroundColor", backgroundColor);
}

void QQuickGraphsItem::setUpCamera()
{
    // By default we could get away with a value of 10 or 15, but as camera zoom is implemented
    // by moving it, we have to take into account the maximum zoom out level. The other
    // option would be to adjust far clip whenever zoom level changes.
    const float farclip = 7000.f;

    m_pCamera = new QQuick3DPerspectiveCamera(rootNode());
    m_pCamera->setClipNear(0.1f);
    m_pCamera->setClipFar(farclip);
    m_pCamera->setFieldOfView(45.0f);
    m_pCamera->setPosition(QVector3D(.0f, .0f, 5.f));

    auto cameraTarget = new QQuick3DNode(rootNode());
    cameraTarget->setParentItem(rootNode());

    setCameraTarget(cameraTarget);
    cameraTarget->setPosition(QVector3D(0, 0, 0));
    QQuick3DObjectPrivate::get(cameraTarget)
        ->refSceneManager(*QQuick3DObjectPrivate::get(rootNode())->sceneManager);

    m_pCamera->lookAt(cameraTarget);
    m_pCamera->setParent(cameraTarget);
    m_pCamera->setParentItem(cameraTarget);

    m_oCamera = new QQuick3DOrthographicCamera(rootNode());
    // Set clip near 0.0001f so that it can be set correct value to workaround
    // a Quick3D device pixel ratio bug
    m_oCamera->setClipNear(0.0001f);
    m_oCamera->setClipFar(farclip);
    m_oCamera->setPosition(QVector3D(0.f, 0.f, 5.f));
    m_oCamera->setParent(cameraTarget);
    m_oCamera->setParentItem(cameraTarget);
    m_oCamera->lookAt(cameraTarget);

    auto useOrtho = isOrthoProjection();
    if (useOrtho)
        setCamera(m_oCamera);
    else
        setCamera(m_pCamera);
    m_changeTracker.cameraChanged = true;
}

void QQuickGraphsItem::setUpLight()
{
    auto light = new QQuick3DDirectionalLight(rootNode());
    QQuick3DObjectPrivate::get(light)->refSceneManager(
        *QQuick3DObjectPrivate::get(rootNode())->sceneManager);
    light->setParent(camera());
    light->setParentItem(camera());
    light->setSoftShadowQuality(QQuick3DAbstractLight::QSSGSoftShadowQuality::Hard);
    m_light = light;
}

void QQuickGraphsItem::setWrapCameraXRotation(bool wrap)
{
    if (m_wrapXRotation == wrap) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << wrap;
        return;
    }
    m_wrapXRotation = wrap;
    emit wrapCameraXRotationChanged(wrap);
}

void QQuickGraphsItem::setWrapCameraYRotation(bool wrap)
{
    if (m_wrapYRotation == wrap) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << wrap;
        return;
    }
    m_wrapYRotation = wrap;
    emit wrapCameraYRotationChanged(wrap);
}

float QQuickGraphsItem::ambientLightStrength() const
{
    return m_ambientLightStrength;
}

void QQuickGraphsItem::setAmbientLightStrength(float newAmbientLightStrength)
{
    if (QtPrivate::fuzzyCompare(m_ambientLightStrength, newAmbientLightStrength)) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newAmbientLightStrength);
        return;
    }

    if (newAmbientLightStrength < 0.0f || newAmbientLightStrength > 1.0f) {
        qCWarning(lcProperties3D, "%s invalid value. Valid range for ambientLightStrength is between "
                 "0.0f and 1.0f", qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        m_ambientLightStrengthDirty = true;
        m_ambientLightStrength = newAmbientLightStrength;
        emit ambientLightStrengthChanged();
        emitNeedRender();
    }
}

float QQuickGraphsItem::lightStrength() const
{
    return m_lightStrength;
}

void QQuickGraphsItem::setLightStrength(float newLightStrength)
{
    if (QtPrivate::fuzzyCompare(m_lightStrength, newLightStrength)) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newLightStrength);
        return;
    }

    if (newLightStrength < 0.0f || newLightStrength > 10.0f) {
        qCWarning(lcProperties3D, "%s invalid value. Valid range for lightStrength is between 0.0f and "
                 "10.0f", qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        m_lightStrengthDirty = true;
        m_lightStrength = newLightStrength;
        emit lightStrengthChanged();
        emitNeedRender();
    }
}

float QQuickGraphsItem::shadowStrength() const
{
    return m_shadowStrength;
}

void QQuickGraphsItem::setShadowStrength(float newShadowStrength)
{
    if (QtPrivate::fuzzyCompare(m_shadowStrength, newShadowStrength)) {
        qCDebug(lcProperties3D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newShadowStrength);
        return;
    }

    if (newShadowStrength < 0.0f || newShadowStrength > 100.0f) {
        qCWarning(lcProperties3D, "%s invalid value. Valid range for shadowStrength is between 0.0f "
                 "and 100.0f", qUtf8Printable(QLatin1String(__FUNCTION__)));
    } else {
        m_shadowStrengthDirty = true;
        m_shadowStrength = newShadowStrength;
        emit shadowStrengthChanged();
        emitNeedRender();
    }
}

QColor QQuickGraphsItem::lightColor() const
{
    return m_lightColor;
}

void QQuickGraphsItem::setLightColor(QColor newLightColor)
{
    if (m_lightColor == newLightColor) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << newLightColor;
        return;
    }
    m_lightColorDirty = true;
    m_lightColor = newLightColor;
    emit lightColorChanged();
    emitNeedRender();
}

void QQuickGraphsItem::updateBackgroundColor()
{
    if (theme()->isBackgroundVisible())
        environment()->setClearColor(theme()->backgroundColor());
    else
        environment()->setClearColor(Qt::transparent);

    if (m_sliceView)
        m_sliceView->environment()->setClearColor(environment()->clearColor());

}

void QQuickGraphsItem::setItemSelected(bool selected)
{
    m_itemSelected = selected;
}

QT_END_NAMESPACE

#include "moc_qquickgraphsitem_p.cpp"
